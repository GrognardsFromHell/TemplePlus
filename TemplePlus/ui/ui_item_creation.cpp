
#include "stdafx.h"
#include "ui.h"
#include "config/config.h"
#include "tig/tig_mes.h"
#include "tig/tig_font.h"
#include "temple_functions.h"
#include "tig/tig_tokenizer.h"
#include "ui_item_creation.h"
#include "combat.h"
#include "obj.h"
#include "radialmenu.h"
#include "common.h"
#include "ui_render.h"
#include <python/python_integration_obj.h>
#include <python/python_object.h>
#include <condition.h>
#include <critter.h>
#include <util/fixes.h>
//#include <condition.h>
#include "gamesystems/objects/objsystem.h"
#include <gamesystems/gamesystems.h>
#include <infrastructure/keyboard.h>
#include <infrastructure/elfhash.h>
#include <tig/tig_tabparser.h>
#include <d20_level.h>
#include <party.h>
#include <graphics/rectangle.h>
#include <graphics/render_hooks.h>
#include <tig/tig_texture.h>
#include <regex>
#include "graphics/imgfile.h"
#include "ui_tooltip.h"

#define NUM_ITEM_ENHANCEMENT_SPECS 41
#define NUM_APPLIED_BONUSES_MAX 9 // number of bonuses that can be applied on item creation

#define MAA_EFFECT_BUTTONS_COUNT 10
#define MAA_TEXTBOX_MAX_LENGTH 60
#define NUM_DISPLAYED_CRAFTABLE_ITEMS_MAX 21
#define MAA_NUM_ENCHANTABLE_ITEM_WIDGETS 5
#define NUM_ITEM_CREATION_ENTRY_WIDGETS 21 // number of "buttons" (clickable text items) in the normal crafting menu
#define MAA_MAX_ENHANCEMENT_BONUS 5
#define MAA_MAX_EFFECTIVE_BONUS 10 // maximum craftable effective bonus

const std::unordered_map<std::string, uint32_t> ItemEnhSpecFlagDict = { 
	{"iesf_enabled", IESF_ENABLED},
	{"iesf_weapon",IESF_WEAPON } ,
	{"iesf_armor",IESF_ARMOR },
	{"iesf_shield",IESF_SHIELD },
	{"iesf_ranged",IESF_RANGED },

	{"iesf_melee",IESF_MELEE },
	{"iesf_thrown",IESF_THROWN },
	{"iesf_unk100",IESF_UNK100 },
	{"iesf_plus_bonus",IESF_ENH_BONUS },
	{"iesf_incremental", IESF_INCREMENTAL }
};

int WandCraftCostCp=0;

ItemCreation itemCreation;

struct UiItemCreationAddresses : temple::AddressTable
{
	int (__cdecl* Sub_100F0200)(objHndl a1, RadialMenuEntry *entry);
	ItemCreationType * itemCreationType;
	objHndl* crafter;
	int *craftInsufficientXP;
	int * craftInsufficientFunds;
	int*craftSkillReqNotMet;
	int*insuffPrereqs;
	char**itemCreationUIStringSkillRequired;
	char**itemCreationUIStringItemCost;
	char**itemCreationUIStringXPCost;
	char**itemCreationUIStringValue;
	TigTextStyle*itemCreationTextStyle;
	TigTextStyle*itemCreationTextStyle2;
	ItemEnhancementSpec *itemEnhSpecs; // size 41
	UiItemCreationAddresses()
	{
		rebase(Sub_100F0200, 0x100F0200);
		rebase(itemCreationType, 0x10BEDF50);
		rebase(crafter, 0x10BECEE0);
		rebase(craftInsufficientXP,0x10BEE3A4);
		rebase(craftInsufficientFunds, 0x10BEE3A8);
		rebase(craftSkillReqNotMet, 0x10BEE3AC);
		rebase(insuffPrereqs, 0x10BEE3B0);

		rebase(itemCreationUIStringSkillRequired, 0x10BED6D4);
		rebase(itemCreationUIStringItemCost, 0x10BEDB50);
		rebase(itemCreationUIStringXPCost, 0x10BED8A4);
		rebase(itemCreationUIStringValue, 0x10BED8A8);

		rebase(itemCreationTextStyle, 0x10BEE338);
		rebase(itemCreationTextStyle2, 0x10BED938);
		rebase(itemEnhSpecs, 0x102967C8);
	}
	
} itemCreationAddresses;




static MesHandle mesItemCreationText;
static MesHandle mesItemCreationRules;
static MesHandle mesItemCreationNamesText;
static ImgFile *background = nullptr;
//static ButtonStateTextures acceptBtnTextures;
//static ButtonStateTextures declineBtnTextures;
static int disabledBtnTexture;

class ItemCreationHooks : public TempleFix {
public:
	

	static void HookedGetLineForMaaAppend(MesHandle, MesLine*); // ensures the crafted item name doesn't overflow

	void apply() override {
		// auto system = UiSystem::getUiSystem("ItemCreation-UI");		
		// system->init = systemInit;
		
		// System Funcs
		replaceFunction<int(__cdecl)(GameSystemConf&)>(0x10154BA0, [](GameSystemConf& conf) {
			return itemCreation.UiItemCreationInit(conf);
		});
		replaceFunction<void(__cdecl)(UiResizeArgs&)>(0x10154E90, [](UiResizeArgs& arg){
			itemCreation.UiItemCreationResize(arg);
		});


		// Show
		replaceFunction<BOOL(__cdecl)(objHndl, ItemCreationType)>(0x101536C0, [](objHndl crafter, ItemCreationType icTypeNew){
			return itemCreation.ItemCreationShow(crafter, icTypeNew);
		});

		replaceFunction<int(__cdecl)(objHndl, objHndl)>(0x10152690, [](objHndl crafter, objHndl item) {
			return itemCreation.CreateItemResourceCheck(crafter, item) ? 1 : 0;
		});


		// MAA Window Message Handler
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10153110, [](int widId, TigMsg* msg)	{
			return itemCreation.MaaWndMsg(widId, msg);
		});

		// MAA Textbox
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10151890, [](int widId, TigMsg* msg) {
			return itemCreation.MaaTextboxMsg(widId, msg);
		});
		replaceFunction<bool(__cdecl)(int , objHndl)>(0x10151C10, [](int widId, objHndl item)
		{
			return itemCreation.MaaWndRenderText(widId, item);
		});

		// MAA selected item for crafting
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10152E40, [](int widId, TigMsg* msg) {
			return itemCreation.MaaItemMsg(widId, msg); }
		);

		// MAA Effect "buttons"
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10153250, [](int widId, TigMsg* msg) {
			return itemCreation.MaaEffectMsg(widId, msg); }
		);
		replaceFunction<void(__cdecl)(int)>(0x10153990, [](int widId) {
			return itemCreation.MaaEffectRender(widId); }
		);
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10152ED0, [](int widId, TigMsg* msg) {
			return itemCreation.MaaEffectAddMsg(widId, msg);
		});
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10152FE0, [](int widId, TigMsg* msg) {
			return itemCreation.MaaEffectRemoveMsg(widId, msg);
		});
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10151EF0, [](int widId, TigMsg* msg) {
			return itemCreation.MaaAppliedBtnMsg(widId, msg);
		});

		// CreateBtnMsg
		replaceFunction<BOOL(int, TigMsg * )>(0x10153F60, [](int widId, TigMsg* msg){
			return itemCreation.CreateBtnMsg(widId, msg);
		});

		// CancelBtnMsg
		replaceFunction<BOOL(int, TigMsg *)>(0x10153820, [](int widId, TigMsg* msg) {
			return itemCreation.CancelBtnMsg(widId, msg);
		});

		
		/*auto writeval = &UiItemCreationInit;
		write(0x102F6C10 + 9 * 4 * 26 + 4, &writeval, sizeof(void*));*/

		redirectCall(0x1015221B, HookedGetLineForMaaAppend);
		replaceFunction<void(_cdecl)(char)>(0x10150C10, [](char newChar) {
			auto craftedName = temple::GetPointer<char>(0x10BED758);
			auto& craftedNameCurPos = temple::GetRef<int>(0x10BECE7C);
			craftedName[63] = 0; // ensure string termination


			int currStrLen = strlen(craftedName) + 1;
			if (currStrLen < 62) {
				for (int i = craftedNameCurPos; i < currStrLen; currStrLen--)
				{
					craftedName[currStrLen] = craftedName[currStrLen - 1];
				}
				craftedName[craftedNameCurPos] = newChar;
				craftedNameCurPos++;
			}


		});
	}

} itemCreationHooks;



int ItemCreation::CraftedWandSpellLevel(objHndl objHndItem)
{
	auto spellData = objSystem->GetObject(objHndItem)->GetSpell(obj_f_item_spell_idx, 0);
	uint32_t spellLevelBasic = spellData.spellLevel;
	uint32_t spellLevelFinal = spellData.spellLevel;


	auto casterLevelSet = (uint32_t) d20Sys.d20QueryReturnData(itemCreationCrafter, DK_QUE_Craft_Wand_Spell_Level);
	casterLevelSet = 2 * ((casterLevelSet + 1) / 2) - 1;
	if (casterLevelSet < 1)
		casterLevelSet = 1;

	auto slotLevelSet = 1 + (casterLevelSet - 1)/ 2;
	if (spellLevelBasic == 0 && casterLevelSet <= 1)
		slotLevelSet = 0;
		
	

	// get data from caster - make this optional!

	uint32_t classCodes[SPELL_ENUM_MAX] = { 0, };
	uint32_t spellLevels[SPELL_ENUM_MAX] = { 0, };
	uint32_t spellFoundNum = 0;
	int casterKnowsSpell = spellSys.spellKnownQueryGetData(itemCreationCrafter, spellData.spellEnum, classCodes, spellLevels, &spellFoundNum);
	if (casterKnowsSpell){
		uint32_t spellClassFinal = classCodes[0];
		spellLevelFinal = 0;
		uint32_t isClassSpell = classCodes[0] & (0x80);

		if (isClassSpell){
			spellLevelFinal = spellSys.GetMaxSpellSlotLevel(itemCreationCrafter, static_cast<Stat>(classCodes[0] & 0x7F), 0);
		};
		if (spellFoundNum > 1){
			for (uint32_t i = 1; i < spellFoundNum; i++){
				if (spellLevels[i] > spellLevelFinal){
					spellData.classCode = classCodes[i];
					spellLevelFinal = spellLevels[i];
				}
			}
			spellData.spellLevel = spellLevelFinal;

		}
		spellData.spellLevel = spellLevelFinal; // that's the max possible at this point
		if (slotLevelSet && slotLevelSet <= spellLevelFinal && slotLevelSet >= spellLevelBasic)
			spellData.spellLevel = slotLevelSet;
		else if (slotLevelSet  > spellLevelFinal)
			spellData.spellLevel = spellLevelFinal;
		else if (slotLevelSet < spellLevelBasic)
			spellData.spellLevel = spellLevelBasic;
		else if (spellLevelBasic == 0)
		{
			spellData.spellLevel = spellLevelBasic;
		} 

		//templeFuncs.Obj_Set_IdxField_byPtr(objHndItem, obj_f_item_spell_idx, 0, &spellData);
		spellLevelFinal = spellData.spellLevel;

	}
	return spellLevelFinal;
}

int ItemCreation::CraftedWandCasterLevel(objHndl item)
{
	int result = CraftedWandSpellLevel(item);
	if (result <= 1)
		return 1;
	return (result * 2) - 1;
}

bool ItemCreation::CreateItemResourceCheck(objHndl obj, objHndl objHndItem){
	bool canCraft = 1;
	bool xpCheck = 0;
	auto insuffXp = itemCreationAddresses.craftInsufficientXP;
	auto insuffCp = itemCreationAddresses.craftInsufficientFunds;
	auto insuffSkill = itemCreationAddresses.craftSkillReqNotMet;
	auto insuffPrereqs = itemCreationAddresses.insuffPrereqs;
	auto surplusXP = d20LevelSys.GetSurplusXp(obj);
	uint32_t craftingCostCP;
	auto partyMoney = party.GetMoney();

	*insuffXp = 0;
	*insuffCp = 0;
	*insuffSkill = 0;
	*insuffPrereqs = 0;
	int itemWorth = objects.getInt32(objHndItem, obj_f_item_worth);

	// Check GP Section
	if (itemCreationType == ItemCreationType::CraftMagicArmsAndArmor){
		craftingCostCP = MaaCpCost( CRAFT_EFFECT_INVALID );
	}
	else
	{
		// current method for crafting stuff:
		craftingCostCP =  itemWorth / 2;

		// TODO: create new function
		if (itemCreationType == ItemCreationType::CraftWand)
		{

			itemWorth = ItemWorthAdjustedForCasterLevel(objHndItem, CraftedWandCasterLevel(objHndItem));
			craftingCostCP = itemWorth / 2;
		}
			
	};

	if ( ( (uint32_t)partyMoney ) < craftingCostCP){
		*insuffCp = 1;
		canCraft = 0;
	};


	// Check XP section (and maybe spell prerequisites too? explore sub_10152280)
	if ( itemCreationType != CraftMagicArmsAndArmor){
		// check requirements from rules\\item_creation.mes
		if ( temple::GetRef<int(__cdecl)(objHndl, objHndl)>(0x10152280)(obj, objHndItem) == 0){ 
			*insuffPrereqs = 1;
			canCraft = 0;
		};
		// check XP
		// TODO make XP cost calculation take applied caster level into account
		int itemXPCost = itemWorth / 2500;
		xpCheck = surplusXP >= itemXPCost;
	} else 
	{
		int magicArmsAndArmorXPCost = MaaXpCost(CRAFT_EFFECT_INVALID);
		xpCheck = surplusXP >= magicArmsAndArmorXPCost;
	}
		
	if (xpCheck){
		return canCraft;
	} else
	{
		*insuffXp = 1;
		return 0;
	};

}

const char* ItemCreation::GetItemCreationMesLine(int lineId){
	MesLine line;
	line.key = lineId;

	mesFuncs.GetLine_Safe(mItemCreationMes, &line);
	return line.value;
}

char const* ItemCreation::ItemCreationGetItemName(objHndl itemHandle) const
{
	if (!itemHandle)
		return nullptr;
	auto itemObj = gameSystems->GetObj().GetObject(itemHandle);

	int protoNum = 0;
	if (itemObj->IsProto()) {
		protoNum = itemObj->id.GetPrototypeId();
	}
	else
		protoNum = itemObj->protoId.GetPrototypeId();

	auto itemCreationNames = temple::GetRef<MesHandle>(0x10BEDB4C);
	MesLine line;
	line.key = protoNum;
	if (mesFuncs.GetLine(itemCreationNames, &line))
		return line.value;
	else
		return description.getDisplayName(itemHandle, itemHandle);
}

objHndl ItemCreation::MaaGetItemHandle(){
	if (craftingItemIdx < 0 || (uint32_t) craftingItemIdx >= mMaaCraftableItemList.size()) {
		return objHndl::null;
	}
	return mMaaCraftableItemList[craftingItemIdx];
}

bool ItemCreation::IsWeaponBonus(int effIdx)
{
	if (effIdx < 0)
		return false;

	if ( (itemEnhSpecs[effIdx].flags & IESF_ENH_BONUS) 
		&& (itemEnhSpecs[effIdx].flags & IESF_WEAPON)){
		return true;
	}
	return false;
}

bool ItemCreation::IsOutmoded(int effIdx){

	auto itEnh = &itemEnhSpecs[effIdx];
	while ( itEnh->upgradesTo != CRAFT_EFFECT_INVALID){
		for (auto it: appliedBonusIndices){
			if (it == itEnh->upgradesTo)
				return true;
		}
		itEnh = &itemEnhSpecs[itEnh->upgradesTo];
	}
	return false;
}

bool ItemCreation::MaaEffectIsApplicable(int effIdx){

	auto& itEnh = itemEnhSpecs[effIdx];
	if (!(itEnh.flags & IESF_ENABLED))
		return false;

	if (itEnh.flags & IESF_INCREMENTAL){
		// not the root and hasn't got the prerequisite effect level
		if (itEnh.downgradesTo != CRAFT_EFFECT_INVALID && !HasNecessaryEffects(effIdx)  )
			return false;
		if (IsOutmoded(effIdx))
			return false;
	}

	if (craftingItemIdx >= 0 && (uint32_t) craftingItemIdx < mMaaCraftableItemList.size())
	{
		auto itemHandle = mMaaCraftableItemList[craftingItemIdx];
		if (itemHandle){
			auto itemObj = gameSystems->GetObj().GetObject(itemHandle);
			if (itemObj->type == obj_t_weapon) {
				if ( !(itEnh.flags & IESF_WEAPON) )
				return false;

				if (itEnh.flags & IESF_RANGED)	{
					auto weapFlags = static_cast<WeaponFlags>(gameSystems->GetObj().GetObject(itemHandle)->GetInt32(obj_f_weapon_flags));
					if (!(weapFlags & WeaponFlags::OWF_RANGED_WEAPON))
						return false;
				}
				if (itEnh.flags & IESF_THROWN) {
					auto weapFlags = static_cast<WeaponFlags>(gameSystems->GetObj().GetObject(itemHandle)->GetInt32(obj_f_weapon_flags));
					if (!(weapFlags & WeaponFlags::OWF_THROWABLE))
						return false;
				}

			}
			if (itemObj->type == obj_t_armor)
			{
				auto armorFlags = itemObj->GetInt32(obj_f_armor_flags);
				auto armorType = inventory.GetArmorType(armorFlags);
				if (armorType == ArmorType::ARMOR_TYPE_SHIELD)	{

					if (!(itEnh.flags & IESF_SHIELD))
						return false;
				} 
				else{

					if (!(itEnh.flags & IESF_ARMOR))
						return false;
				}
			}
		}
	}


	return true;
}

int ItemCreation::GetEffIdxFromWidgetIdx(int widIdx){

	// auto scrollbar2Y = temple::GetRef<int>(0x10BECDA8);
	auto adjIdx = mMaaApplicableEffectsScrollbarY + widIdx; // this is the overall index for the effect
	auto validCount = 0;
	for (auto it: itemEnhSpecs){
		if (MaaEffectIsApplicable(it.first) && !(it.second.flags & IESF_ENH_BONUS)){
			if (validCount == adjIdx){
				return it.first;
			}
			validCount++;
		}
			
	}
	return CRAFT_EFFECT_INVALID;
}

int ItemCreation::GetEffIdxFromWidgetId(int widId){
	auto idx = 0;
	for (idx = 0; idx < MAA_EFFECT_BUTTONS_COUNT; idx++) {
		if (maaBtnIds[idx] == widId)
			break;
	}
	if (idx >= MAA_EFFECT_BUTTONS_COUNT)
		return CRAFT_EFFECT_INVALID;

	
	return GetEffIdxFromWidgetIdx(idx);
}

int ItemCreation::HasNecessaryEffects(int effIdx){

	if (effIdx == CRAFT_EFFECT_INVALID)
		return FALSE;

	
	auto &itEnh = itemEnhSpecs[effIdx];


	if (itEnh.flags & IESF_ENH_BONUS) {
		// if the effect itself is a +1 plus then ok
		if (itEnh.downgradesTo == CRAFT_EFFECT_INVALID)
			return TRUE;

		// otherwise, ensure the previous +X bonus is already applied
		//return HasNecessaryEffects(itEnh.downgradesTo);
		for (auto it : appliedBonusIndices)
		{
			auto &itEnh2 = itemEnhSpecs[it];
			if ((itEnh2.flags & IESF_ENH_BONUS)
				&& itEnh2.data.enhBonus == itEnh.data.enhBonus - 1)
				return TRUE;
		}
		return FALSE;
	}
	else {
		// for the other effects, ensure there is at least a +1 bonus
		bool hasBonus = false, 
			hasAntecedent = !(itEnh.flags & IESF_INCREMENTAL 
				&& itEnh.downgradesTo != CRAFT_EFFECT_INVALID);

		for (auto it : appliedBonusIndices)	{

			auto &itEnh2 = itemEnhSpecs[it];
			if (!hasBonus && (itEnh2.flags & IESF_ENH_BONUS) && itEnh2.data.enhBonus >= 1) {
				hasBonus = true;
				if (hasAntecedent)
					return TRUE;
			}
			if (itEnh2.upgradesTo == effIdx){
				hasAntecedent = true;
				if (hasBonus)
					return TRUE;
			}
		}
		return (hasAntecedent && hasBonus) ? TRUE : FALSE;
	}

}

int ItemCreation::MaaGetCurEnhBonus(){

	auto curEnhBon = 0;
	for (auto it : appliedBonusIndices) {
		auto& itEnhSpec = itemEnhSpecs[it];
		if ( (itEnhSpec.flags & IESF_ENH_BONUS) && itEnhSpec.data.enhBonus > curEnhBon)
			curEnhBon = itEnhSpec.data.enhBonus;
	}

	return curEnhBon;
}

int ItemCreation::MaaGetEffIdxForEnhBonus(int enhBon, objHndl itemHandle){

	auto flag = IESF_WEAPON;
	auto itemObj = gameSystems->GetObj().GetObject(itemHandle);
	if (itemObj->type == obj_t_armor){
		auto armorFlags = itemObj->GetInt32(obj_f_armor_flags);
		auto armorType = inventory.GetArmorType(armorFlags);
		if (armorType == ArmorType::ARMOR_TYPE_SHIELD) 
			flag = IESF_SHIELD;
		else 
			flag = IESF_ARMOR;
	}

	for (auto& it : itemEnhSpecs) {
		if ((it.second.flags & flag) && it.second.data.enhBonus == enhBon)
			return it.first;
	}
	return CRAFT_EFFECT_INVALID;
}
	

	

bool ItemCreation::ItemWielderCondsContainEffect(int effIdx, objHndl item)
{
	if (effIdx == CRAFT_EFFECT_INVALID)
		return false;

	auto itemObj = gameSystems->GetObj().GetObject(item);


	auto &itEnh = itemEnhSpecs[effIdx];
	auto condId = itEnh.condId;
	auto condArray = itemObj->GetInt32Array(obj_f_item_pad_wielder_condition_array);

	if (condArray.GetSize() <= 0)
		return false;


	if (!IsWeaponBonus(effIdx)){  // a +x WEAPON bonus

		for (auto i = 0u; i < condArray.GetSize(); i++){
			auto condArrayIt = condArray[i];
			if (condArrayIt  == condId)	{
				if ( itEnh.flags & (IESF_ENH_BONUS | IESF_INCREMENTAL ) ){
					return itEnh.data.enhBonus <= inventory.GetItemWieldCondArg(item, condId, 0);
				}
				else
					return true;
			}	
		}

		return false;
	}

	// else, do the weapon +x bonus

	auto toHitBonusId = ElfHash::Hash("To Hit Bonus");
	auto damageBonusId = ElfHash::Hash("Damage Bonus");
	auto damBonus = 0;
	auto toHitBonus = 0;

	for (auto i = 0u; i < condArray.GetSize();i++){

		auto wielderCondId = condArray[i];
		if (wielderCondId == condId){
			return itemEnhSpecs[effIdx].data.enhBonus <= inventory.GetItemWieldCondArg(item, condId, 0);
		}

		if (wielderCondId == toHitBonusId)
		{
			toHitBonus = inventory.GetItemWieldCondArg(item, toHitBonusId, 0);
			if (toHitBonus == damBonus)
				return itemEnhSpecs[effIdx].data.enhBonus <= toHitBonus;
		}

		if (wielderCondId == damageBonusId)
		{
			damBonus = inventory.GetItemWieldCondArg(item, toHitBonusId, 0);
			if (toHitBonus == damBonus)
			{
				return itemEnhSpecs[effIdx].data.enhBonus <= toHitBonus;
			}
		}
	}

	return false;
};

void ItemCreation::CraftScrollWandPotionSetItemSpellData(objHndl objHndItem, objHndl objHndCrafter){

	// the new and improved Wands/Scroll Property Setting Function

	auto obj = objSystem->GetObject(objHndItem);
	// auto itemCreationType = *itemCreationAddresses.itemCreationType;

	if (itemCreationType == CraftWand){
		
		auto spellData = obj->GetSpell(obj_f_item_spell_idx, 0);
		uint32_t spellLevelBasic = spellData.spellLevel;
		uint32_t spellLevelFinal = CraftedWandSpellLevel(objHndItem);
		spellData.spellLevel = spellLevelFinal;				
		obj->SetSpell(obj_f_item_spell_idx, 0, spellData);
		
		auto args = PyTuple_New(3);
			
		int casterLevelFinal = spellLevelFinal * 2 - 1;
		if (casterLevelFinal < 1)
			casterLevelFinal = 1;
		PyTuple_SET_ITEM(args, 0, PyObjHndl_Create(objHndItem));
		PyTuple_SET_ITEM(args, 1, PyObjHndl_Create(objHndCrafter));
		PyTuple_SET_ITEM(args, 2, PyInt_FromLong(casterLevelFinal));

		auto wandNameObj = pythonObjIntegration.ExecuteScript("crafting", "craft_wand_new_name", args);
		char * wandNewName = PyString_AsString(wandNameObj);
			
		objects.setInt32(objHndItem, obj_f_description, objects.description.CustomNameNew(wandNewName)); // TODO: create function that appends effect of caster level boost			
		objects.setInt32(objHndItem, obj_f_item_spell_charges_idx, 50); // as god intended it to be!
		Py_DECREF(wandNameObj);
		Py_DECREF(args);
		
		return;

	}
	if (itemCreationType == ScribeScroll){
		// do scroll specific stuff
		// templeFuncs.Obj_Set_Field_32bit(objHndItem, obj_f_description, templeFuncs.CustomNameNew("Scroll of LOL"));
	};

	if (itemCreationType == BrewPotion){
		// do potion specific stuff
		// templeFuncs.Obj_Set_Field_32bit(objHndItem, obj_f_description, templeFuncs.CustomNameNew("Potion of Commotion"));
		// TODO: change it so it's 0xBAAD F00D just like spawned / mobbed potions
	};
	
	auto numItemSpells = obj->GetSpellArray(obj_f_item_spell_idx).GetSize();

	// loop thru the item's spells (can be more than one in principle, like Keoghthem's Ointment)

	// Current code - change this at will...
	for (auto n = 0u; n < numItemSpells; n++){
		auto spellData = obj->GetSpell(obj_f_item_spell_idx, n);

		// get data from caster - make this optional!

		uint32_t classCodes[SPELL_ENUM_MAX] = { 0, };
		uint32_t spellLevels[SPELL_ENUM_MAX] = { 0, };
		uint32_t spellFoundNum = 0;
		int casterKnowsSpell = spellSys.spellKnownQueryGetData(objHndCrafter, spellData.spellEnum, classCodes, spellLevels, &spellFoundNum);
		if (casterKnowsSpell){
			uint32_t spellClassFinal = classCodes[0];
			uint32_t spellLevelFinal = 0;
			uint32_t isClassSpell = classCodes[0] & (0x80);

			if (isClassSpell){
				spellLevelFinal = spellSys.GetMaxSpellSlotLevel(objHndCrafter, static_cast<Stat>(classCodes[0] & 0x7F), 0);
				spellData.classCode = spellClassFinal;
			};
			if (spellFoundNum > 1){
				for (uint32_t i = 1; i < spellFoundNum; i++){
					if (spellLevels[i] > spellLevelFinal){
						spellData.classCode = classCodes[i];
						spellLevelFinal = spellLevels[i];
					}
				}
				spellData.spellLevel = spellLevelFinal;

			}
			spellData.spellLevel = spellLevelFinal;
			obj->SetSpell(obj_f_item_spell_idx, n, spellData);

		}

	}
};


void ItemCreation::CreateItemDebitXPGP(objHndl crafter, objHndl objHndItem){
	uint32_t crafterXP = objects.getInt32(crafter, obj_f_critter_experience);
	uint32_t craftingCostCP = 0;
	uint32_t craftingCostXP = 0;

	auto itemCreationType = *itemCreationAddresses.itemCreationType;

	if (itemCreationType == CraftMagicArmsAndArmor){ // magic arms and armor
		craftingCostCP = MaaCpCost(CRAFT_EFFECT_INVALID);
		craftingCostXP = MaaXpCost(CRAFT_EFFECT_INVALID);
	}
	else
	{
		// TODO make crafting costs take applied caster level into account
		// currently this is what ToEE does	
		int itemWorth;
		if (itemCreationType == ItemCreationType::CraftWand)
			itemWorth = ItemWorthAdjustedForCasterLevel(objHndItem, CraftedWandCasterLevel(objHndItem));
		else
			itemWorth = objects.getInt32(objHndItem, obj_f_item_worth);
		craftingCostCP = itemWorth / 2;
		craftingCostXP = itemWorth / 2500;

	};

	templeFuncs.DebitPartyMoney(0, 0, 0, craftingCostCP);
	objects.setInt32(crafter, obj_f_critter_experience, crafterXP - craftingCostXP);
};

void ItemCreation::ItemCreationCraftingCostTexts(int widgetId, objHndl objHndItem){
	// prolog
	int32_t * insuffXp;
	int32_t * insuffCp;
	int32_t *insuffSkill;
	int32_t *insuffPrereq;
	uint32_t craftingCostCP;
	uint32_t craftingCostXP;
	TigRect rect(212 + 108 * mUseCo8Ui, 157, 159, 10);
	char * prereqString;

	uint32_t casterLevelNew = -1; // h4x!
	
	if (itemCreationType == CraftWand)
	{
		casterLevelNew = CraftedWandCasterLevel(objHndItem);
	}
	

	insuffXp = itemCreationAddresses.craftInsufficientXP;
	insuffCp = itemCreationAddresses.craftInsufficientFunds;
	insuffSkill = itemCreationAddresses.craftSkillReqNotMet;
	insuffPrereq = itemCreationAddresses.insuffPrereqs;


	craftingCostCP = ItemWorthAdjustedForCasterLevel(objHndItem, casterLevelNew) / 2;
	craftingCostXP = ItemWorthAdjustedForCasterLevel(objHndItem, casterLevelNew) / 2500;
	
	string text;
	// "Item Cost: %d"
	if (*insuffXp || *insuffCp || *insuffSkill || *insuffPrereq){
		
		text = format("{} @{}{}", *itemCreationAddresses.itemCreationUIStringItemCost, *(insuffCp)+1, craftingCostCP / 100);
	} else {
		//_snprintf(text, 128, "%s @3%d", *itemCreationAddresses.itemCreationUIStringItemCost, craftingCostCP / 100);
		text = format("{} @3{}", *itemCreationAddresses.itemCreationUIStringItemCost, craftingCostCP / 100);
	};


	UiRenderer::DrawTextInWidget(widgetId, text, rect, *itemCreationAddresses.itemCreationTextStyle);
	rect.y += 11;



	// "Experience Cost: %d"  (or "Skill Req: " for alchemy - WIP)

	if (itemCreationType == IC_Alchemy){ 
		// placeholder - they do similar bullshit in the code :P but I guess it can be modified easily enough!
		if (*insuffXp || *insuffCp || *insuffSkill || *insuffPrereq){
			text = format("{} @{}{}", *itemCreationAddresses.itemCreationUIStringSkillRequired, *insuffSkill + 1, craftingCostXP);
		}
		else {
			text = format("{} @3{}", *itemCreationAddresses.itemCreationUIStringSkillRequired, craftingCostXP);
		};
	}
	else
	{
		if (*insuffXp || *insuffCp || *insuffSkill || *insuffPrereq){
			text = format("{} @{}{}", *itemCreationAddresses.itemCreationUIStringXPCost, *(insuffXp) + 1, craftingCostXP);
		}
		else {
			text = format("{} @3{}", *itemCreationAddresses.itemCreationUIStringXPCost, craftingCostXP);
		};
	};

	UiRenderer::DrawTextInWidget(widgetId, text, rect, *itemCreationAddresses.itemCreationTextStyle);
	rect.y += 11;

	// "Value: %d"
	//_snprintf(text, 128, "%s @1%d", * (itemCreationUIStringValue.ptr() ), templeFuncs.Obj_Get_Field_32bit(objHndItem, obj_f_item_worth) / 100);
	text = format("{} @1{}", *itemCreationAddresses.itemCreationUIStringValue, ItemWorthAdjustedForCasterLevel(objHndItem, casterLevelNew) / 100);

	UiRenderer::DrawTextInWidget(widgetId, text, rect, *itemCreationAddresses.itemCreationTextStyle2);

	// Prereq: %s
	rect.x = 210 + 108 * mUseCo8Ui;
	rect.y = 200;
	rect.width = 150;
	rect.height = 105;
	prereqString = templeFuncs.ItemCreationPrereqSthg_sub_101525B0(itemCreationCrafter, objHndItem);
	if (prereqString){
		UiRenderer::DrawTextInWidget(widgetId, prereqString, rect, *itemCreationAddresses.itemCreationTextStyle);
	}
	
	if (itemCreationType == ItemCreationType::CraftWand)
	{
		rect.x = 210 + 108 * mUseCo8Ui;
		rect.y = 250;
		rect.width = 150;
		rect.height = 105;
		char asdf[1000];
		sprintf(asdf, "Crafted Caster Level: " );
		
		text = format("{} @3{}", asdf, casterLevelNew);
		if (prereqString){
			UiRenderer::DrawTextInWidget(widgetId, text, rect, *itemCreationAddresses.itemCreationTextStyle);
		}
	}
	
	
}

BOOL ItemCreation::ItemCreationEntryMsg(int widId, TigMsg* msg){
	auto _msg = (TigMsgWidget*)msg;
	if (msg->type != TigMsgType::WIDGET || _msg->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return false;

	craftingWidgetId = widId;

	auto widIdx = 0;
	for (; widIdx< NUM_ITEM_CREATION_ENTRY_WIDGETS; widIdx++) {
		if (mItemCreationEntryBtnIds[widIdx] == widId)
			break;
	}

	if (widIdx >= NUM_ITEM_CREATION_ENTRY_WIDGETS)
		widIdx = -1;

	auto itemIdx = mItemCreationScrollbarY + widIdx;
	if (itemIdx < 0 || (uint32_t) itemIdx >= numItemsCrafting[itemCreationType])
		return true;


	craftingItemIdx = itemIdx;
	auto itemHandle = craftedItemHandles[itemCreationType][itemIdx];
	if (CreateItemResourceCheck(itemCreationCrafter, itemHandle)) {
		ui.ButtonSetButtonState(mItemCreationCreateBtnId, UiButtonState::UBS_NORMAL);
	}
	else {
		ui.ButtonSetButtonState(mItemCreationCreateBtnId, UiButtonState::UBS_DISABLED);
	}

	return true;
}

void ItemCreation::ItemCreationCreateBtnRender(int widId) const
{
	UiButtonState buttonState;
	if (ui.GetButtonState(widId, buttonState))
		return;

	Render2dArgs arg;
	if (buttonState == UiButtonState::UBS_DOWN)
	{
		arg.textureId = temple::GetRef<int>(0x10BED9EC);
	}
	else if (buttonState == UiButtonState::UBS_HOVERED)
	{
		arg.textureId = temple::GetRef<int>(0x10BEDA48);
	}
	else
	{
		arg.textureId = temple::GetRef<int>(0x10BED9F0);
	}

	arg.flags = 0;
	//arg.srcRect = &temple::GetRef<TigRect>(0x102FAEE4);
	TigRect destRect( 82 + 14*mUseCo8Ui, 165 + 209, 113, 22);
	/*arg.destRect = &destRect;
	arg.vertexColors = nullptr;*/
	UiRenderer::DrawTextureInWidget(mItemCreationWndId, arg.textureId, destRect, temple::GetRef<TigRect>(0x102FAEE4));

	auto &textStyle = temple::GetRef<TigTextStyle>(0x10BED9F8);
	auto &text = temple::GetRef<const char*>(0x10BED930);
	auto measText = UiRenderer::MeasureTextSize(text, textStyle);

	destRect.x += (112 - measText.width)/ 2;
	destRect.y += (22 - measText.height) / 2;
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	UiRenderer::DrawTextInWidget(mItemCreationWndId, text, destRect, textStyle);
	UiRenderer::PopFont();
}

void ItemCreation::ItemCreationCancelBtnRender(int widId) const
{
	UiButtonState buttonState;
	if (ui.GetButtonState(widId, buttonState))
		return;

	Render2dArgs arg;
	if (buttonState == UiButtonState::UBS_DOWN)
	{
		arg.textureId = temple::GetRef<int>(0x10BED6D0);
	}
	else if (buttonState == UiButtonState::UBS_HOVERED)
	{
		arg.textureId = temple::GetRef<int>(0x10BEE2D4);
	}
	else
	{
		arg.textureId = temple::GetRef<int>(0x10BEDA5C);
	}

	arg.flags = 0;
	arg.srcRect = nullptr;;
	TigRect destRect(207 + 108*mUseCo8Ui,  165 + 209, 112, 22);
	arg.destRect = &destRect;
	arg.vertexColors = nullptr;
	// RenderHooks::TextureRender2d(&arg);
	UiRenderer::DrawTextureInWidget(mItemCreationWndId, arg.textureId, destRect, temple::GetRef<TigRect>(0x102FAEE4));

	auto &textStyle = temple::GetRef<TigTextStyle>(0x10BED9F8);
	auto &text = temple::GetRef<const char*>(0x10BED8AC);
	auto measText = UiRenderer::MeasureTextSize(text, textStyle);

	destRect.x += (112 - measText.width) / 2;
	destRect.y += (22 - measText.height) / 2;
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	UiRenderer::DrawTextInWidget(mItemCreationWndId, text, destRect, textStyle);
	UiRenderer::PopFont();
};


void ItemCreation::GetMaaSpecs() const
{

	struct MaaSpecTabEntry	{
		char * id;
		char * condName;
		char * flags;
		char * effBonus;
		char * enhBonus;
		char * classReq; // class req
		char * charReqs; // Character Level, Alignment
		char * spellReqs;
		char * featReqs; // TODO
		char * antecedent;
	};

	auto maaSpecLineParser = [](const TigTabParser*, int lineIdx, char ** cols)
	{
		auto& tabEntry = *reinterpret_cast<MaaSpecTabEntry*>(cols);
		
		auto effIdx = atol(tabEntry.id);
		auto condName = tabEntry.condName;

		// get flags
		uint32_t flags = 0;
		{
			StringTokenizer flagTok(tabEntry.flags);
			while (flagTok.next()) {
				auto& tok = flagTok.token();
				if (tok.type != StringTokenType::Identifier)
					continue;
				
				auto flagLookup = ItemEnhSpecFlagDict.find(tok.text);
				if (flagLookup != ItemEnhSpecFlagDict.end()){
					flags |= flagLookup->second;
				}
				
				
			}
		}
		
		
		auto effBonus = atol(tabEntry.effBonus);
		auto enhBonus = atol(tabEntry.enhBonus);

		itemCreation.itemEnhSpecs[effIdx] = ItemEnhancementSpec(condName, flags, effBonus, enhBonus);

		auto &itEnh = itemCreation.itemEnhSpecs[effIdx];
		// get class req
		if (tabEntry.classReq)
		{
			// TODO (right now only Weapon Ki Focus uses it and it's not enabled anyway)
		}

		// get charReqs
		if (tabEntry.charReqs)
		{
			StringTokenizer charReqTok(tabEntry.charReqs);
			while (charReqTok.next())
			{
				auto& tok = charReqTok.token();
				if (tok.type != StringTokenType::Identifier)
					continue;
				if (toupper(tok.text[0]) == 'A')
				{
					if (!_stricmp(&tok.text[1], "good"))
						itEnh.reqs.alignment = Alignment::ALIGNMENT_GOOD;
					else if (!_stricmp(&tok.text[1], "lawful"))
						itEnh.reqs.alignment = Alignment::ALIGNMENT_LAWFUL;
					else if (!_stricmp(&tok.text[1], "evil"))
						itEnh.reqs.alignment = Alignment::ALIGNMENT_EVIL;
					else if (!_stricmp(&tok.text[1], "chaotic"))
						itEnh.reqs.alignment = Alignment::ALIGNMENT_CHAOTIC;
				} 
				else if (toupper(tok.text[0]) == 'C')
				{
					itEnh.reqs.minLevel = atol(&tok.text[1]);
				}
				
			}
		}

		// get spellReqs
		if (tabEntry.spellReqs)
		{
			StringTokenizer spellReqTok(tabEntry.spellReqs);
			while (spellReqTok.next())
			{
				auto& tok = spellReqTok.token();
				if (tok.type != StringTokenType::QuotedString)
					continue;
				auto spellEnum = spellSys.getSpellEnum(tok.text);
				if (spellEnum){
					itEnh.reqs.spells[0].push_back(spellEnum);
				}
				// TODO: implement AND/OR support (currently just Brilliant Radiance has this, not yet implemented anyway)
			}
		}
		
		if (tabEntry.antecedent && *tabEntry.antecedent){
			itEnh.downgradesTo = atol(tabEntry.antecedent);
		} 
		else{
			itEnh.downgradesTo = CRAFT_EFFECT_INVALID;
		}

		return 0;
	};

	TigTabParser maaSpecsTab;
	maaSpecsTab.Init(maaSpecLineParser);
	maaSpecsTab.Open("tprules\\craft_maa_specs.tab");
	maaSpecsTab.Process();
	maaSpecsTab.Close();

	for (auto i = itemEnhSpecs.begin(); i != itemEnhSpecs.end(); ++i) {
		auto downgradesTo = i->second.downgradesTo;
		if (downgradesTo != CRAFT_EFFECT_INVALID){
			itemCreation.itemEnhSpecs[downgradesTo].upgradesTo = i->first;
		}
	}
}

uint32_t ItemWorthAdjustedForCasterLevel(objHndl objHndItem, uint32_t casterLevelNew){
	auto obj = objSystem->GetObject(objHndItem);
	auto numItemSpells = obj->GetSpellArray(obj_f_item_spell_idx).GetSize();
	auto itemWorthBase = obj->GetInt32(obj_f_item_worth);
	if (casterLevelNew == -1){
		return itemWorthBase;
	}

	uint32_t itemSlotLevelBase = 0;

	// loop thru the item's spells (can be more than one in principle, like Keoghthem's Ointment)

	for (uint32_t n = 0; n < numItemSpells; n++){
		auto spellData = obj->GetSpell(obj_f_item_spell_idx, n);
		if (spellData.spellLevel > itemSlotLevelBase){
			itemSlotLevelBase = spellData.spellLevel;
		}
	};

	auto casterLevelOld = (uint32_t) std::max<int>(1, itemSlotLevelBase * 2 - 1);

	if (itemSlotLevelBase == 0 && casterLevelNew > casterLevelOld){
		return itemWorthBase * casterLevelNew;
	}
	if (casterLevelNew > casterLevelOld)
	{
		return itemWorthBase * casterLevelNew / casterLevelOld;
	}
	return itemWorthBase;

	
}

static vector<objHndl> craftingProtoHandles[8];

const char *getProtoName(objHndl protoHandle) {
	/*
	 // gets item creation proto id
  if ( sub_1009C950((objHndl)protoHandle) )
    v1 = sub_100392E0(protoHandle);
  else
    v1 = sub_10039320((objHndl)protoHandle);

  line.key = v1;
  if ( tig_mes_get_line(ui_itemcreation_names, &line) )
    result = line.value;
  else
    result = objects.description._getDisplayName((objHndl)protoHandle, (objHndl)protoHandle);
  return result;
  */

	return objects.description.getDisplayName(protoHandle);
}

static void loadProtoIds(MesHandle mesHandle) {

	for (uint32_t i = 0; i < 8; ++i) {
		auto protoLine = mesFuncs.GetLineById(mesHandle, i);
		if (!protoLine) {
			continue;
		}

		auto &protoHandles = craftingProtoHandles[i];

		StringTokenizer tokenizer(protoLine);
		while (tokenizer.next()) {
			auto handle = objSystem->GetProtoHandle(tokenizer.token().numberInt);
			protoHandles.push_back(handle);
		}

		// Sort by prototype name
		sort(protoHandles.begin(), protoHandles.end(), [](objHndl a, objHndl b)
		{
			auto nameA = getProtoName(a);
			auto nameB = getProtoName(b);
			return _strcmpi(nameA, nameB);
		});
		logger->info("Loaded {} prototypes for crafting type {}", craftingProtoHandles[i].size(), i);
	}
	
}




uint32_t ItemCreationBuildRadialMenuEntry(DispatcherCallbackArgs args, ItemCreationType itemCreationType, char* helpSystemString, MesHandle combatMesLine)
{
	if (combatSys.isCombatActive()) { return 0; }
	MesLine mesLine;
	RadialMenuEntryAction radEntry(combatMesLine, D20A_ITEM_CREATION, itemCreationType, helpSystemString);
	radEntry.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::Feats);
	
	return 0;
};


static void __cdecl systemReset() {
}

static void __cdecl systemExit() {
}


ItemCreation::ItemCreation(){

	for (int i = 0; i < 30; i++) {
		GoldCraftCostVsEffectiveBonus[i] = 1000 * i*i;
		GoldBaseWorthVsEffectiveBonus[i] = GoldCraftCostVsEffectiveBonus[i] * 2;
	}

	craftedItemExistingEffectiveBonus = -1; // stores the crafted item existing (pre-crafting) effective bonus
	//craftingItemIdx = -1;

	memset(numItemsCrafting, 0, sizeof(numItemsCrafting));
	memset(craftedItemHandles, 0, sizeof(craftedItemHandles));
	craftedItemNamePos = 0;
	craftingWidgetId = -1;

}

int ItemCreation::GetSurplusXp(objHndl crafter){
	auto level = objects.StatLevelGet(crafter, stat_level);
	auto xpReq = d20LevelSys.GetXpRequireForLevel(level);
	return gameSystems->GetObj().GetObject(crafter)->GetInt32(obj_f_critter_experience) - xpReq;
}

bool ItemCreation::ItemWielderCondsHasAntecedent(int effIdx, objHndl item){
	auto &itEnh = itemEnhSpecs[effIdx];
	if (itEnh.downgradesTo != CRAFT_EFFECT_INVALID){
		if (ItemWielderCondsContainEffect(itEnh.downgradesTo, item))
			return true;
		else 
			return ItemWielderCondsHasAntecedent(itEnh.downgradesTo, item);
	}
	return false;
}


ItemEnhancementSpec::ItemEnhancementSpec(const char* CondName, uint32_t Flags, int EffcBonus, int enhBonus)
	:condName(CondName),flags(Flags),effectiveBonus(EffcBonus){
	data.enhBonus = enhBonus;
	condId = ElfHash::Hash(condName);
	downgradesTo = upgradesTo = CRAFT_EFFECT_INVALID;
}

BOOL ItemCreation::ItemCreationShow(objHndl crafter, ItemCreationType icType)
{
	if (icType == itemCreationType)
		return TRUE;

	if (itemCreationType <= ItemCreationType::ForgeRing && itemCreationType >= ItemCreationType::IC_Alchemy )
	{
		if (itemCreationResourceCheckResults)
			free(itemCreationResourceCheckResults);
		ui.WidgetSetHidden(mItemCreationWndId, 1);
		ui.WidgetCopy(mItemCreationWndId, mItemCreationWnd);
	} 
	else if (itemCreationType == ItemCreationType::CraftMagicArmsAndArmor)	{
		ui.WidgetSetHidden(mMaaWndId, 1);
	}

	itemCreationType = icType;
	itemCreationCrafter = crafter;

	if (icType <= ItemCreationType::ForgeRing)	{
		ButtonStateInit(mItemCreationWndId);
	} 
	else if (icType == ItemCreationType::CraftMagicArmsAndArmor)	{
		appliedBonusIndices.clear();
		MaaInitCrafter(crafter);
		MaaInitWnd(mMaaWndId);
		return TRUE;
	}

	return TRUE;
}

BOOL ItemCreation::ItemCreationWndMsg(int widId, TigMsg * msg){

	if (msg->type == TigMsgType::MOUSE) {
		auto _msg = (TigMsgMouse*)msg;
		if (_msg->buttonStateFlags & MSF_SCROLLWHEEL_CHANGE) {
			auto newMsg = *(TigMsgMouse*)msg;
			newMsg.buttonStateFlags = MSF_SCROLLWHEEL_CHANGE;
			ui.WidgetCopy(mItemCreationScrollbarId, mItemCreationScrollbar);
			mItemCreationScrollbar->handleMessage(mItemCreationScrollbarId, (TigMsg*)&newMsg);
		}
		return true;
	}
		

	if (msg->type == TigMsgType::KEYSTATECHANGE && (msg->arg2 & 0xFF) == 1
		|| msg->type == TigMsgType::KEYDOWN) {
		auto vk = msg->arg1 & 0xFF;
		if (msg->type == TigMsgType::KEYSTATECHANGE)
			vk = infrastructure::gKeyboard.ToVirtualKey(msg->arg1);

		switch (vk) {
		case VK_RETURN:
			// TODO: Add "enter to craft"
			return true;
		default:
			return true;
		}
		return true;
	}


	if (msg->type == TigMsgType::CHAR) {
		auto key = (char)msg->arg1;
		// TODO: add jump to letter
		return true;
	}

	if (msg->type == TigMsgType::WIDGET) { // scrolling
		auto _msg = (TigMsgWidget*)msg;
		if (_msg->widgetEventType == TigMsgWidgetEvent::Scrolled) {
			ui.ScrollbarGetY(mItemCreationScrollbarId, &mItemCreationScrollbarY);
		}
		return true;
	}

	return false;
}

void ItemCreation::ItemCreationWndRender(int widId){
	// Background Image
	if (mUseCo8Ui){
		// draw background (composed of pieces)
		TigRect srcRect(0, 0, 256, 165);
		TigRect destRect(mItemCreationWnd->x, mItemCreationWnd->y, 256, 165);
		UiRenderer::DrawTexture(mItemCreationWidenedTexture01, destRect, srcRect);
		destRect.x += 256;
		UiRenderer::DrawTexture(mItemCreationWidenedTexture11, destRect, srcRect);
		
		destRect.y += 165;
		srcRect = TigRect(0, 0, 256, 256);
		destRect.height = 256;
		UiRenderer::DrawTexture(mItemCreationWidenedTexture10, destRect, srcRect);
		destRect.x -= 256;
		UiRenderer::DrawTexture(mItemCreationWidenedTexture00, destRect, srcRect);
	} 
	else	{
		bkgImage->SetX(mItemCreationWnd->x);
		bkgImage->SetY(mItemCreationWnd->y);
		bkgImage->Render();
	}
	
	

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	// title
	std::string text(GetItemCreationMesLine(itemCreationType));
	auto measText = UiRenderer::MeasureTextSize(text, temple::GetRef<TigTextStyle>(0x10BED938));
	TigRect rect((342 + 108*mUseCo8Ui - measText.width)/2 + 29, (15 - measText.height) / 2 + 10,342, 15);
	UiRenderer::DrawTextInWidget(mItemCreationWndId, text, rect, temple::GetRef<TigTextStyle>(0x10BED9F8));

	// draw crafter name
	auto crafterName = description.getDisplayName(itemCreationCrafter);
	measText = UiRenderer::MeasureTextSize(crafterName, temple::GetRef<TigTextStyle>(0x10BED938));
	TigRect crafterRect((351 + 108 * mUseCo8Ui - measText.width) / 2 + 24 , (21 - measText.height) / 2 + 322, 351, 21);
	UiRenderer::DrawTextInWidget(widId, crafterName, crafterRect, temple::GetRef<TigTextStyle>(0x10BED9F8));

	// draw XP & GP
	auto surplusXp = d20LevelSys.GetSurplusXp(itemCreationCrafter);
	text = fmt::format("{}", surplusXp);
	measText = UiRenderer::MeasureTextSize(text, temple::GetRef<TigTextStyle>(0x10BED938));
	TigRect resourceRect((66 - measText.width) / 2 + 130 + 14*mUseCo8Ui, (12 - measText.height) / 2 + 343, 66, 12);
	UiRenderer::DrawTextInWidget(widId, text, resourceRect, temple::GetRef<TigTextStyle>(0x10BED938));
	auto partyMoney = party.GetMoney();
	text = fmt::format("{}", partyMoney / 100);
	measText = UiRenderer::MeasureTextSize(text, temple::GetRef<TigTextStyle>(0x10BED938));
	resourceRect = TigRect((66 - measText.width) / 2 + 245 + 108 * mUseCo8Ui, (12 - measText.height) / 2 + 343, 66, 12);
	UiRenderer::DrawTextInWidget(widId, text, resourceRect, temple::GetRef<TigTextStyle>(0x10BED938));


	auto shortname = ui.GetStatShortName(stat_experience);
	measText = UiRenderer::MeasureTextSize(shortname, temple::GetRef<TigTextStyle>(0x10BED938));
	resourceRect = TigRect((37 - measText.width) / 2 + 89 + 14 * mUseCo8Ui, (15 - measText.height) / 2 + 341, 37, 15);
	UiRenderer::DrawTextInWidget(widId, shortname, resourceRect, temple::GetRef<TigTextStyle>(0x10BED850));


	text = GetItemCreationMesLine(10 + itemCreationType); // Item Creation Names (printed above the items to be created)
	measText = UiRenderer::MeasureTextSize(text, temple::GetRef<TigTextStyle>(0x10BED938));
	resourceRect = TigRect(29, 38, 167, 11);
	UiRenderer::DrawTextInWidget(widId, text, resourceRect, temple::GetRef<TigTextStyle>(0x10BED6D8));

	text = GetItemCreationMesLine(10000); // Item Information:
	measText = UiRenderer::MeasureTextSize(text, temple::GetRef<TigTextStyle>(0x10BED938));
	resourceRect = TigRect(206 + 108 * mUseCo8Ui, 38, 167, 11);
	UiRenderer::DrawTextInWidget(widId, text, resourceRect, temple::GetRef<TigTextStyle>(0x10BED6D8));




	// draw info pertaining to selected item
	if (craftingItemIdx >= 0 && (uint32_t) craftingItemIdx < numItemsCrafting[itemCreationType])
	{
		auto itemHandle = craftedItemHandles[itemCreationType][craftingItemIdx];

		// draw icon
		auto invAid = (UiGenericAsset)gameSystems->GetObj().GetObject(itemHandle)->GetInt32(obj_f_item_inv_aid);
		int textureId;
		ui.GetAsset(UiAssetType::Inventory, invAid, textureId);
		rect = TigRect(temple::GetRef<TigRect>(0x102FAEC4));
		rect.x += 108 * mUseCo8Ui;
		UiRenderer::DrawTexture(textureId, rect);


		auto itemName = ItemCreationGetItemName(itemHandle);
		measText = UiRenderer::MeasureTextSize(itemName, temple::GetRef<TigTextStyle>(0x10BED938));
		if (measText.width > 161) {
			measText.width = 161;
		}
		rect = TigRect((161 - measText.width )/2 + 208 + 108 * mUseCo8Ui, 132, 161, 24);
		UiRenderer::DrawTextInWidget(widId, itemName, rect, temple::GetRef<TigTextStyle>(0x10BEDFE8));
		
		ItemCreationCraftingCostTexts(widId, itemHandle);
	}
	

	UiRenderer::PopFont();
}


void ItemCreation::ItemCreationEntryRender(int widId){
	auto widIdx = 0;
	for (; widIdx< NUM_ITEM_CREATION_ENTRY_WIDGETS; widIdx++){
		if (mItemCreationEntryBtnIds[ widIdx ] == widId)
			break;
	}

	if (widIdx >= NUM_ITEM_CREATION_ENTRY_WIDGETS)
		widIdx = -1;

	auto itemIdx = mItemCreationScrollbarY + widIdx;
	if (itemIdx < 0 || (uint32_t) itemIdx >= numItemsCrafting[itemCreationType])
		return;

	if (mUseCo8Ui)
		UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	else
		UiRenderer::PushFont(PredefinedFont::ARIAL_10);


	auto itemHandle = craftedItemHandles[itemCreationType][itemIdx];
	auto itemName = ItemCreationGetItemName(itemHandle);
	TigRect rect(32, 12 * widIdx + 55, 155 + mUseCo8Ui * 108, 12);
	auto checkRes = itemCreationResourceCheckResults[itemIdx];
	if (itemIdx == craftingItemIdx)	{
		if (checkRes)
			UiRenderer::DrawTextInWidget(mItemCreationWndId, itemName, rect, temple::GetRef<TigTextStyle>(0x10BEDFE8));
		else
			UiRenderer::DrawTextInWidget(mItemCreationWndId, itemName, rect, temple::GetRef<TigTextStyle>(0x10BECE90));
	} 
	else {
		if (checkRes)
			UiRenderer::DrawTextInWidget(mItemCreationWndId, itemName, rect, temple::GetRef<TigTextStyle>(0x10BED938));
		else
			UiRenderer::DrawTextInWidget(mItemCreationWndId, itemName, rect, temple::GetRef<TigTextStyle>(0x10BED6D8));
	}

	UiRenderer::PopFont();
}

void ItemCreation::MaaWndRender(int widId){

	// draw background (composed of pieces)
	TigRect srcRect(1, 1, 254, 254);
	TigRect destRect(mMaaWnd->x, mMaaWnd->y, 254, 254);
	UiRenderer::DrawTexture(temple::GetRef<int>(0x10BEE38C), destRect, srcRect);
	destRect.x += 253;
	UiRenderer::DrawTexture(temple::GetRef<int>(0x10BECEE8), destRect, srcRect);
	destRect.y += 253;
	UiRenderer::DrawTexture(temple::GetRef<int>(0x10BECEEC), destRect, srcRect);
	destRect.x -= 253;
	UiRenderer::DrawTexture(temple::GetRef<int>(0x10BED988), destRect, srcRect);



	UiRenderer::PushFont(PredefinedFont::PRIORY_12);

	// draw title
	auto creationTypeLabel = GetItemCreationMesLine(ItemCreationType::CraftMagicArmsAndArmor);
	auto measText = UiRenderer::MeasureTextSize(creationTypeLabel, temple::GetRef<TigTextStyle>(0x10BED938));
	TigRect titleRect((440 - measText.width) / 2 + 31, (13 - measText.height) / 2 + 12, 440, 13);
	UiRenderer::DrawTextInWidget(widId, creationTypeLabel, titleRect, temple::GetRef<TigTextStyle>(0x10BED9F8));

	// draw crafter name
	auto crafterName = description.getDisplayName(itemCreationCrafter);
	measText = UiRenderer::MeasureTextSize(crafterName, temple::GetRef<TigTextStyle>(0x10BED938));
	TigRect crafterRect((347 - measText.width) / 2 + 77, (16 - measText.height) / 2 + 290, 347, 16);
	UiRenderer::DrawTextInWidget(widId, crafterName, crafterRect, temple::GetRef<TigTextStyle>(0x10BED9F8));
	
	// draw XP & GP
	auto surplusXp = d20LevelSys.GetSurplusXp(itemCreationCrafter);
	std::string text = fmt::format("{}", surplusXp);
	measText = UiRenderer::MeasureTextSize(text, temple::GetRef<TigTextStyle>(0x10BED938));
	TigRect resourceRect((66 - measText.width) / 2 + 180, (12 - measText.height) / 2 + 310, 66, 12);
	UiRenderer::DrawTextInWidget(widId, text, resourceRect, temple::GetRef<TigTextStyle>(0x10BED9F8));
	auto partyMoney = party.GetMoney();
	text = fmt::format("{}", partyMoney / 100);
	measText = UiRenderer::MeasureTextSize(text, temple::GetRef<TigTextStyle>(0x10BED938));
	resourceRect = TigRect((66 - measText.width) / 2 + 296, (12 - measText.height) / 2 + 310, 66, 12);
	UiRenderer::DrawTextInWidget(widId, text, resourceRect, temple::GetRef<TigTextStyle>(0x10BED9F8));


	auto shortname = ui.GetStatShortName(stat_experience);
	measText = UiRenderer::MeasureTextSize(shortname, temple::GetRef<TigTextStyle>(0x10BED938));
	resourceRect = TigRect((37 - measText.width) / 2 + 140, (15- measText.height) / 2 + 308, 37, 15);
	UiRenderer::DrawTextInWidget(widId, shortname, resourceRect, temple::GetRef<TigTextStyle>(0x10BED850));

	

	if (craftingItemIdx >= 0 && (uint32_t) craftingItemIdx < mMaaCraftableItemList.size())	{

		// draw item icon
		auto itemHandle = mMaaCraftableItemList[craftingItemIdx];
		auto invAid = gameSystems->GetObj().GetObject(itemHandle)->GetInt32(obj_f_item_inv_aid);
		auto GetAsset = temple::GetRef<int(__cdecl)(UiAssetType assetType, uint32_t assetIndex, int* textureIdOut, int offset) >(0x1004A360);
		int textureId;
		GetAsset(UiAssetType::Inventory, invAid, &textureId, 0);
		UiRenderer::DrawTexture(textureId, mMaaCraftedItemIconDestRect);

		// render texts
		MaaWndRenderText(widId, itemHandle);
	}

	UiRenderer::PopFont();
}

void ItemCreation::MaaItemRender(int widId){
	auto widIdx = 0;
	for (; widIdx < MAA_NUM_ENCHANTABLE_ITEM_WIDGETS; widIdx++){
		if (widId == mMaaItemBtnIds[widIdx])
			break;
	}
	if (widIdx >= MAA_NUM_ENCHANTABLE_ITEM_WIDGETS)
		widIdx = -1;

	auto itemIdx = widIdx + mMaaItemsScrollbarY;

	if (itemIdx < 0 || (size_t) itemIdx >= mMaaCraftableItemList.size())
		return;

	// bounding box I think
	TigRect rect(mMaaWnd->x + 30, mMaaWnd->y + widIdx*44+ 54, 152, 42);
	TigRect srcRect(1, 1, 152, 42);
	if (itemIdx == craftingItemIdx)
		UiRenderer::DrawTexture(temple::GetRef<int>(0x10BECDAC), rect, srcRect);
	else
		UiRenderer::DrawTexture(temple::GetRef<int>(0x10BEE038), rect, srcRect);


	// draw item name
	auto itemHandle = mMaaCraftableItemList[itemIdx];
	UiRenderer::PushFont(PredefinedFont::ARIAL_10);
	auto itemName = ItemCreationGetItemName(itemHandle);
	auto measText = UiRenderer::MeasureTextSize(itemName, temple::GetRef<TigTextStyle>(0x10BED938));
	if (measText.width > 110){
		measText.width = 110;
	}
	rect = TigRect( (110 - measText.width)/2 + 72, (42 - measText.height)/2 + 54 + 44* widIdx ,110, 42);
	if (itemIdx == craftingItemIdx)
		UiRenderer::DrawTextInWidget(mMaaWndId, itemName, rect, temple::GetRef<TigTextStyle>(0x10BEDFE8));
	else
		UiRenderer::DrawTextInWidget(mMaaWndId, itemName, rect, temple::GetRef<TigTextStyle>(0x10BED938));

	UiRenderer::PopFont();

	// draw item icon
	rect = TigRect(mMaaWnd->x + 31, mMaaWnd->y + 54 + 44 * widIdx, 40, 40);
	auto invAid = (UiGenericAsset)gameSystems->GetObj().GetObject(itemHandle)->GetInt32(obj_f_item_inv_aid);
	int textureId =0;
	ui.GetAsset(UiAssetType::Inventory, invAid, textureId);
	srcRect = TigRect(0, 0, 64, 64);
	UiRenderer::DrawTexture(textureId, rect, srcRect);

}

void ItemCreation::MaaAppliedBtnRender(int widId){
	if (craftingItemIdx < 0)
		return;

	if (appliedBonusIndices.size() == 0)
		return;

	auto idx = 0u;
	for (idx = 0; idx < NUM_APPLIED_BONUSES_MAX; idx++) {
		if (mMaaAppliedBtnIds[idx] == widId)
			break;
	}
	if (idx >= NUM_APPLIED_BONUSES_MAX || idx >= appliedBonusIndices.size())
		return;

	auto displayCount = 0;
	auto effIdx = CRAFT_EFFECT_INVALID;
	for (auto it : appliedBonusIndices) {
		if (itemEnhSpecs[it].flags & IESF_ENH_BONUS)
			continue;
		if (displayCount == idx) {
			effIdx = it;
			break;
		}
		displayCount++;
	}

	if (effIdx == CRAFT_EFFECT_INVALID)
		return;

	//if (!MaaCrafterMeetsReqs(effIdx, itemCreationCrafter)){
	//	return; // TODO is this a bug? what if someone else enchanted it first?
	//}

	auto enhName = GetItemCreationMesLine(1000 + effIdx);
	TigRect rect(355, 12 * idx + 152 + 12, 106, 12);

	auto itemHandle = mMaaCraftableItemList[craftingItemIdx];
	if (idx == mMaaActiveAppliedWidIdx)	{
		if (ItemWielderCondsContainEffect(effIdx, itemHandle)) {
			UiRenderer::DrawTextInWidget(mMaaWndId, enhName, rect, temple::GetRef<TigTextStyle>(0x10BECE90));
		}
		else
		{
			UiRenderer::DrawTextInWidget(mMaaWndId, enhName, rect, temple::GetRef<TigTextStyle>(0x10BEDFE8));
		}
	}
	else if (ItemWielderCondsContainEffect(effIdx, itemHandle)) {
		UiRenderer::DrawTextInWidget(mMaaWndId, enhName, rect, temple::GetRef<TigTextStyle>(0x10BED6D8));
	} else
	{
		UiRenderer::DrawTextInWidget(mMaaWndId, enhName, rect, temple::GetRef<TigTextStyle>(0x10BED938));
	}

}

void ItemCreation::MaaEnhBonusDnRender(int widId){

	objHndl itemHandle = MaaGetItemHandle();
	if (!itemHandle)
		return;

	int texId;
	UiButtonState bs;
	ui.GetButtonState(widId, bs);
	switch (bs) {
	case UBS_DISABLED:
		texId = mDownArrowDisabledTga;
		break;
	case UBS_DOWN:
		texId = mDownArrowClickTga;
		break;
	case UBS_HOVERED:
		texId = mDownArrowHoveredTga;
		break;
	case UBS_NORMAL:
	default:
		texId = mDownArrowTga;
	}
	static TigRect srcRect(0,0,19,11);

	UiRenderer::DrawTextureInWidget(mMaaWndId, texId, mEnhBonusDnRect, srcRect);
}

void ItemCreation::MaaEnhBonusUpRender(int widId){

	objHndl itemHandle = MaaGetItemHandle();
	if (!itemHandle)
		return;

	int texId;
	UiButtonState bs;
	ui.GetButtonState(widId, bs);
	switch (bs) {
	case UBS_DISABLED:
		texId = mDownArrowDisabledTga;
		break;
	case UBS_DOWN:
		texId = mDownArrowClickTga;
		break;
	case UBS_HOVERED:
		texId = mDownArrowHoveredTga;
		break;
	case UBS_NORMAL:
	default:
		texId = mDownArrowTga;
	}
	static TigRect srcRect(0, 0, 19, 11);
	TigRect rect = mEnhBonusDnRect;
	rect.x -= 15;
	UiRenderer::DrawTextureInWidget(mMaaWndId,texId, rect, srcRect, 0x20);
}

void ItemCreation::ButtonStateInit(int wndId){
	ui.WidgetSetHidden(wndId, 0);
	ui.WidgetCopy(wndId, mItemCreationWnd);
	itemCreationResourceCheckResults = new bool[numItemsCrafting[itemCreationType]];
	for (int i = 0; i < (int)numItemsCrafting[itemCreationType];i++)
	{
		auto itemHandle = craftedItemHandles[itemCreationType][i];
		if (itemHandle)	{
			itemCreationResourceCheckResults[i] = CreateItemResourceCheck(itemCreationCrafter, itemHandle);
		}
	}

	if (craftingItemIdx >= 0 && craftingItemIdx < (int)numItemsCrafting[itemCreationType]){
		if (CreateItemResourceCheck(itemCreationCrafter, craftedItemHandles[itemCreationType][craftingItemIdx]))
			ui.ButtonSetButtonState(mItemCreationCreateBtnId, UiButtonState::UBS_NORMAL);
		else
			ui.ButtonSetButtonState(mItemCreationCreateBtnId, UiButtonState::UBS_DISABLED);
	}

	ui.ScrollbarSetYmax(mItemCreationScrollbarId, numItemsCrafting[itemCreationType] - NUM_DISPLAYED_CRAFTABLE_ITEMS_MAX  < 0 ? 0 : numItemsCrafting[itemCreationType] - NUM_DISPLAYED_CRAFTABLE_ITEMS_MAX);
	ui.ScrollbarSetY(mItemCreationScrollbarId, 0);
	mItemCreationScrollbarY = 0;
	ui.WidgetBringToFront(wndId);
}

void ItemCreation::MaaInitCraftedItem(objHndl itemHandle){

	craftedItemName.clear();
	craftedItemExistingEffectiveBonus = 0;
	appliedBonusIndices.clear();
	if (!itemHandle){
		return;
	}

	craftedItemName.append(ItemCreationGetItemName(itemHandle));
	craftedItemNamePos = craftedItemName.size();

	for (auto it: itemEnhSpecs)	{
		if (ItemWielderCondsContainEffect(it.first,itemHandle))	{
			if (!ItemWielderCondsContainEffect(it.second.upgradesTo, itemHandle)){
				craftedItemExistingEffectiveBonus += it.second.effectiveBonus;
				appliedBonusIndices.push_back(it.first);
			}
		}
	}
	

}

void ItemCreation::MaaInitCrafter(objHndl crafter){
	mMaaCraftableItemList.clear();
	auto crafterObj = gameSystems->GetObj().GetObject(crafter);
	auto crafterInvenNum = crafterObj->GetInt32(obj_f_critter_inventory_num);
	auto inventoryArr = crafterObj->GetObjectIdArray(obj_f_critter_inventory_list_idx);
	
	static auto itemCanBeEnchantedWithMaa = [](objHndl crafter, objHndl item){
		static auto weaponMwId = ElfHash::Hash("Weapon Masterwork");
		static auto weaponEnhId = ElfHash::Hash("Weapon Enhancement Bonus");
		static auto armorMwId = ElfHash::Hash("Armor Masterwork");
		static auto armorEnhId = ElfHash::Hash("Armor Enhancement Bonus");
		static auto toHitId = ElfHash::Hash("To Hit Bonus");
		static auto damBonId = ElfHash::Hash("Damage Bonus");
		static auto shieldEnhId = ElfHash::Hash("Shield Enhancement Bonus");

		auto itemObj = gameSystems->GetObj().GetObject(item);
		
		if (itemObj->type == obj_t_weapon){
			auto condArr = itemObj->GetInt32Array(obj_f_item_pad_wielder_condition_array);
			bool oneFound = false;
			for (auto i = 0u; i < condArr.GetSize(); i++)
			{
				auto condId = condArr[i];
				if (condId == weaponMwId || condId == weaponEnhId)
					return true;
				if (condId == damBonId || condId == toHitId){
					if (oneFound)
						return true;
					oneFound = true;
				}
			}
		} 
		else if (itemObj->type == obj_t_armor){
			auto condArr = itemObj->GetInt32Array(obj_f_item_pad_wielder_condition_array);
			for (auto i = 0u; i < condArr.GetSize(); i++)
			{
				auto condId = condArr[i];
				auto condW = conds.GetById(condId);
				if (condId == armorMwId || condId == armorEnhId || condId == shieldEnhId)
					return true;
			}
		}
		return false;
	};  // temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x1014F190);
	
	
	for (int i = 0; i < crafterInvenNum;i++){
		auto itemHandle = inventoryArr[i].GetHandle();		
		if (itemCanBeEnchantedWithMaa(crafter, itemHandle))	{
			mMaaCraftableItemList.push_back(itemHandle);
		}
	}
	MaaInitCraftedItem(objHndl::null);
}

void ItemCreation::MaaInitWnd(int wndId){
	maaSelectedEffIdx = -1;
	mMaaActiveAppliedWidIdx = -1;
	objHndl itemHandle = objHndl::null;
	if (craftingItemIdx >= 0 && mCraftingItemIdx < (int)mMaaCraftableItemList.size()) {
		itemHandle = mMaaCraftableItemList[craftingItemIdx];
	}

	MaaInitCraftedItem(itemHandle);
	ui.WidgetSetHidden(wndId, 0);
	ui.WidgetCopy(wndId, mMaaWnd);
	ui.WidgetBringToFront(wndId);
	// auto scrollbarId =  mMaaCraftableItemsScrollbarId;// temple::GetRef<int>(0x10BED8A0);
	ui.ScrollbarSetYmax(mMaaItemsScrollbarId, mMaaCraftableItemList.size() < 5 ? 0 : mMaaCraftableItemList.size() - 5);
	ui.ScrollbarSetY(mMaaItemsScrollbarId, 0);
	mMaaItemsScrollbarY = 0;

	auto numApplicableEffects = 0;
	for (auto it : itemEnhSpecs) {
		if (MaaEffectIsApplicable(it.first)) {
			numApplicableEffects++;
		}
	}

	ui.ScrollbarSetYmax(mMaaApplicableEffectsScrollbarId, numApplicableEffects < MAA_EFFECT_BUTTONS_COUNT ? 0 : numApplicableEffects - MAA_EFFECT_BUTTONS_COUNT);
	ui.ScrollbarSetY(mMaaApplicableEffectsScrollbarId, 0);

	mMaaApplicableEffectsScrollbarY = 0;
	craftingItemIdx = -1;

	if (!mMaaCraftableItemList.size()) {
		auto title = combatSys.GetCombatMesLine(6009);
		auto helpId = ElfHash::Hash("TAG_CRAFT_MAGIC_ARMS_ARMOR_POPUP");
		auto popupType0 = temple::GetRef<int(__cdecl)(int, int(__cdecl*)(), const char*)>(0x100E6F10);
		popupType0(helpId, []() { return itemCreation.ItemCreationShow(objHndl::null, ItemCreationType::Inactive); }, title);
	}
	craftedItemNamePos = craftedItemName.size();
}

BOOL ItemCreation::CreateBtnMsg(int widId, TigMsg* msg)
{
	auto _msg = (TigMsgWidget*)msg;
	if (msg->type == TigMsgType::WIDGET && _msg->widgetEventType == TigMsgWidgetEvent::MouseReleased)
	{
		craftedItemNamePos = craftedItemName.size();
		craftingWidgetId = widId;
		if (craftingItemIdx >= 0 )
		{
			objHndl itemHandle;
			if (itemCreationType == ItemCreationType::CraftMagicArmsAndArmor){
				itemHandle = mMaaCraftableItemList[craftingItemIdx];
			} 
			else{
				itemHandle = craftedItemHandles[itemCreationType][craftingItemIdx];
			}
				
			if ( CreateItemResourceCheck(itemCreationCrafter, itemHandle) )
			{
				CreateItemDebitXPGP(itemCreationCrafter, itemHandle);
				CreateItemFinalize(itemCreationCrafter, itemHandle);
			}
		}
	}
	return false;
}

bool ItemCreation::MaaShouldJustModifyArg(int effIdx, objHndl item){

	auto &itEnh = itemEnhSpecs[effIdx];
	if (!(itEnh.flags & (IESF_ENH_BONUS | IESF_INCREMENTAL)))
		return false;
		
	return ItemWielderCondsHasAntecedent(effIdx, item);
}

void ItemCreation::MaaCreateBtnRender(int widId) const
{
	UiButtonState buttonState;
	if (ui.GetButtonState(widId,buttonState))
		return;
	
	Render2dArgs arg;
	if (buttonState == UiButtonState::UBS_DOWN)
	{
		arg.textureId = temple::GetRef<int>(0x10BED9EC);
	} else if (buttonState == UiButtonState::UBS_HOVERED)
	{
		arg.textureId = temple::GetRef<int>(0x10BEDA48);
	} else
	{
		arg.textureId = temple::GetRef<int>(0x10BED9F0);
	}
	
	arg.flags = 0;
	arg.srcRect = &temple::GetRef<TigRect>(0x102FAEE4);
	arg.destRect = &mCreateBtnRect;
	arg.vertexColors = nullptr;
	RenderHooks::TextureRender2d(&arg);

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	UiRenderer::DrawTextInWidget(mMaaWndId, temple::GetRef<const char*>(0x10BED930), temple::GetRef<TigRect>(0x10BEDA80), temple::GetRef<TigTextStyle>(0x10BED9F8));
	UiRenderer::PopFont();
}

// Item Creation UI
void ItemCreation::CreateItemFinalize(objHndl crafter, objHndl item){

	auto icType = itemCreationType;
	auto effBonus = 0;
	auto crafterObj = gameSystems->GetObj().GetObject(crafter);
	auto altPressed = infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) || infrastructure::gKeyboard.IsKeyPressed(VK_RMENU);

	//auto appliedBonusIndices = temple::GetRef<int[9]>(0x10BED908);

	if (icType == ItemCreationType::CraftMagicArmsAndArmor){

		auto itemObj = gameSystems->GetObj().GetObject(item);
		for (auto it : appliedBonusIndices){
			auto effIdx = it;

			if (effIdx == CRAFT_EFFECT_INVALID)
				continue;

			if (ItemWielderCondsContainEffect(effIdx, item))
				continue;

			auto& itEnh = itemEnhSpecs[effIdx];

			effBonus += itEnh.effectiveBonus;
			int arg0 = 0, arg1 = 0;


			auto condId = ElfHash::Hash(itEnh.condName);
			auto wielderConds = itemObj->GetInt32Array(obj_f_item_pad_wielder_condition_array);

			if ( MaaShouldJustModifyArg(effIdx, item) ) {
				arg0 = itEnh.data.enhBonus;
				arg1 = 0;

				auto condArgIdx = 0;
				for (size_t i = 0; i < wielderConds.GetSize(); i++) {
					auto wieldCondId = itemObj->GetInt32(obj_f_item_pad_wielder_condition_array, i);
					auto wieldCond = conds.GetById(wieldCondId);
					if (wieldCondId == condId) {
						itemObj->SetInt32(obj_f_item_pad_wielder_argument_array, condArgIdx, arg0);
						itemObj->SetInt32(obj_f_item_pad_wielder_argument_array, condArgIdx + 1, arg1);
					}
					if (wieldCond)
						condArgIdx += wieldCond->numArgs;
				}
			}
			else {
			
				arg0 = itEnh.data.enhBonus;
				arg1 = 0;

				auto appliedCond = conds.GetByName(itEnh.condName);
				itemObj->AppendInt32(obj_f_item_pad_wielder_condition_array, ElfHash::Hash(appliedCond->condName));

				if (appliedCond->numArgs > 0)
					itemObj->AppendInt32(obj_f_item_pad_wielder_argument_array, arg0);
				if (appliedCond->numArgs > 1)
					itemObj->AppendInt32(obj_f_item_pad_wielder_argument_array, arg1);
				for (auto i = 2u; i < appliedCond->numArgs; i++)
					itemObj->AppendInt32(obj_f_item_pad_wielder_argument_array, 0);

				// applying +1 shield/armor bonus, so remove the masterwork condition
				if ((itEnh.flags & (IESF_ARMOR | IESF_SHIELD) ) && itEnh.data.enhBonus == 1){
					auto armorMWCondId = ElfHash::Hash("Armor Masterwork");
					inventory.RemoveWielderCond(item,armorMWCondId);
				}
			} 
		}
		auto itemWorthDelta = 100;
		if (itemObj->type == obj_t_weapon)
		{
			itemWorthDelta *= GoldBaseWorthVsEffectiveBonus[effBonus] - GoldBaseWorthVsEffectiveBonus[craftedItemExistingEffectiveBonus];
		}
		else
		{
			itemWorthDelta *= GoldCraftCostVsEffectiveBonus[effBonus] - GoldCraftCostVsEffectiveBonus[craftedItemExistingEffectiveBonus];
		}

		auto itemWorthRegardIdentified = temple::GetRef<int(__cdecl)(objHndl)>(0x10067C90)(item);

		itemObj->SetInt32(obj_f_item_worth, itemWorthRegardIdentified + itemWorthDelta);

		auto itemDesc = itemObj->GetInt32(obj_f_description);
		if (description.DescriptionIsCustom(itemDesc))
		{
			description.CustomNameChange(craftedItemName.c_str(), itemDesc);
		}
		else
		{
			auto itemDescNew = description.CustomNameNew(craftedItemName.c_str());
			itemObj->SetInt32(obj_f_description, itemDescNew);
		}

		ui.WidgetSetHidden(mMaaWndId, 1);
		itemCreationType = ItemCreationType::Inactive;
		itemCreationCrafter = 0i64;

		return;
	}

	// else, a non-MAA crafting type with the simpler window

	// create the new item
	auto newItemHandle = gameSystems->GetObj().CreateObject(item, crafterObj->GetLocation());
	if (!newItemHandle)
		return;

	auto itemObj = gameSystems->GetObj().GetObject(newItemHandle);

	// make it Identified
	itemObj->SetItemFlag(ItemFlag::OIF_IDENTIFIED, true);


	// set its spell properties
	if (itemCreationType == ItemCreationType::CraftWand
		|| itemCreationType == ItemCreationType::ScribeScroll
		|| itemCreationType == ItemCreationType::BrewPotion) {
		CraftScrollWandPotionSetItemSpellData(newItemHandle, crafter);
	}

	inventory.ItemGet(newItemHandle, crafter, 8);

	if (itemCreationType != ItemCreationType::Inactive) {
		//infrastructure::gKeyboard.Update();
		// if ALT is pressed, keep the window open for more crafting!

		//auto& itemCreationResourceCheckResults = temple::GetRef<char*>(0x10BEE330);
		if (altPressed) {

			// refresh the resource checks
			auto numItemsCrafting = temple::GetRef<int[8]>(0x11E76C7C); // array containing number of protos
			static auto craftingHandles = temple::GetRef<objHndl*[8]>(0x11E76C3C); // proto handles

			for (int i = 0; i < numItemsCrafting[itemCreationType]; i++) {
				auto protoHandle = craftingHandles[itemCreationType][i];
				if (protoHandle)
					itemCreationResourceCheckResults[i] = CreateItemResourceCheck(crafter, protoHandle);
			}

			auto icItemIdx = temple::GetRef<int>(0x10BEE398);
			if (icItemIdx >= 0 && icItemIdx < numItemsCrafting[itemCreationType]) {
				auto createBtnId = temple::GetRef<int>(0x10BED8B0);
				if (CreateItemResourceCheck(crafter, item))
				{
					ui.ButtonSetButtonState(createBtnId, UiButtonState::UBS_NORMAL);
				}
				else
				{
					ui.ButtonSetButtonState(createBtnId, UiButtonState::UBS_DISABLED);
				}
			}
			return;
		}

		// else close the window and reset everything
		free(itemCreationResourceCheckResults);
		ui.WidgetSetHidden(mItemCreationWndId, 1);
		itemCreationType = ItemCreationType::Inactive;
		itemCreationCrafter = 0i64;
	}
}

BOOL ItemCreation::CancelBtnMsg(int widId, TigMsg* msg) {
	if (msg->type == TigMsgType::WIDGET && (TigMsgWidgetEvent)msg->arg2 == TigMsgWidgetEvent::MouseReleased)
	{
		craftedItemNamePos = craftedItemName.size();
		craftingWidgetId = widId;

		if (itemCreationType != ItemCreationType::Inactive) {
			if (itemCreationType <= ItemCreationType::ForgeRing){
				free(itemCreationResourceCheckResults);
				ui.WidgetSetHidden(mItemCreationWndId, 1);
				ui.WidgetCopy(mItemCreationWndId, mItemCreationWnd);
			}
			else if (itemCreationType == ItemCreationType::CraftMagicArmsAndArmor)
			{
				ui.WidgetSetHidden(mMaaWndId, 1);
				ui.WidgetCopy(mItemCreationWndId, mMaaWnd);
			}

			itemCreationType = ItemCreationType::Inactive;
		}
		return true;
	}
	return false;
}

void ItemCreation::MaaCancelBtnRender(int widId) const
{
	UiButtonState buttonState;
	if (ui.GetButtonState(widId, buttonState))
		return;

	Render2dArgs arg;
	if (buttonState == UiButtonState::UBS_DOWN)
	{
		arg.textureId = temple::GetRef<int>(0x10BED6D0);
	}
	else if (buttonState == UiButtonState::UBS_HOVERED)
	{
		arg.textureId = temple::GetRef<int>(0x10BEE2D4);
	}
	else
	{
		arg.textureId = temple::GetRef<int>(0x10BEDA5C);
	}

	arg.flags = 0;
	arg.srcRect = &temple::GetRef<TigRect>(0x102FAF04);
	arg.destRect = &mMaaCancelBtnRect;
	arg.vertexColors = nullptr;
	RenderHooks::TextureRender2d(&arg);

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	UiRenderer::DrawTextInWidget(mMaaWndId, temple::GetRef<const char*>(0x10BED8AC), temple::GetRef<TigRect>(0x10BED744), temple::GetRef<TigTextStyle>(0x10BED9F8));
	UiRenderer::PopFont();
}

bool ItemCreation::MaaCrafterMeetsReqs(int effIdx, objHndl crafter)
{
	if (!crafter)
		return false;
	if (effIdx == CRAFT_EFFECT_INVALID)
		return false;

	auto& itEnh = itemEnhSpecs[effIdx];
	if (objects.StatLevelGet(crafter, stat_level) < itEnh.reqs.minLevel)
		return false;

	if (itEnh.reqs.alignment){
		if (!(objects.StatLevelGet(crafter,stat_alignment) & itEnh.reqs.alignment))
		return false;
	}
		
	// todo: implement AND/OR conditions
	if (itEnh.reqs.spells.size() > 0){
		auto spellKnown = false;
		for (auto it : itEnh.reqs.spells) {
			for (auto it2 : it.second)
			{
				if (spellSys.spellKnownQueryGetData(crafter, it2, nullptr, nullptr, nullptr))
					spellKnown = true;
			}
		}
		if (!spellKnown)
			return false;
	}
	

	return true;
}

bool ItemCreation::MaaEffectIsInAppliedList(int effIdx){
	for (auto it: appliedBonusIndices){
		if (it == effIdx)
			return true;
	}
	return false;
}

BOOL ItemCreation::MaaWndMsg(int widId, TigMsg * msg)
{
	if (msg->type == TigMsgType::MOUSE) {

		auto _msg = (TigMsgMouse*)msg;
		if (_msg->buttonStateFlags & MSF_SCROLLWHEEL_CHANGE) {
			auto widg = ui.ScrollbarGet(mMaaApplicableEffectsScrollbarId);
			auto newMsg = *(TigMsgMouse*)msg;
			newMsg.buttonStateFlags = MSF_SCROLLWHEEL_CHANGE;
			widg->handleMessage(mMaaApplicableEffectsScrollbarId, (TigMsg*)&newMsg);
		}

		return true;
	}

	auto annulMsg = [](TigMsg* _msg){
		_msg->type = TigMsgType::MOUSE;
		memset(_msg, 0, sizeof TigMsg);
		_msg->arg1 = 0xdeadbeef;
		_msg->arg2 = 0xbeefb00f;
	};


	if (msg->type == TigMsgType::KEYSTATECHANGE && (msg->arg2 & 0xFF) == 1
		|| msg->type == TigMsgType::KEYDOWN) {
		auto vk = msg->arg1 & 0xFF;
		if (msg->type == TigMsgType::KEYSTATECHANGE)
			vk = infrastructure::gKeyboard.ToVirtualKey(msg->arg1);

		int crnl = 0;
		switch (vk) {
		case VK_LEFT:
			if (--craftedItemNamePos < 0)
				craftedItemNamePos = 0;
			annulMsg(msg);
			return true;
		case VK_RIGHT:
			craftedItemNamePos++;
			crnl = craftedItemName.size();
			if (craftedItemNamePos > crnl)
				craftedItemNamePos = crnl;
			annulMsg(msg);
			return true;
		case VK_HOME:
			craftedItemNamePos = 0;
			return true;
		case VK_DELETE:
			crnl = craftedItemName.size();
			if (craftedItemNamePos < crnl) {
				craftedItemName.erase(craftedItemName.begin() + craftedItemNamePos);
			}
			annulMsg(msg);
			return true;
		default:
			annulMsg(msg);
			return FALSE;
			
		}
		return true;
	} 
	else if (msg->type == TigMsgType::KEYSTATECHANGE && (msg->arg2 == 0)){
		annulMsg(msg);
		return FALSE;
	}
	
	
	if (msg->type == TigMsgType::CHAR) {
		auto key = (char)msg->arg1;
		if (key == '\b') {
			if (craftedItemNamePos > 0){
				if (craftedItemNamePos <= (int) craftedItemName.size()) {
					craftedItemName.erase(craftedItemName.begin() + --craftedItemNamePos);
				}
			}

		}
		else if (key == '\33'){  // escape
			TigMsg exitMsg;
			exitMsg.type = TigMsgType::WIDGET;
			exitMsg.arg2 = (int)TigMsgWidgetEvent::MouseReleased;
			annulMsg(msg);
			return CancelBtnMsg(widId, &exitMsg);
		}
		else if (key != '\r' && key >= ' ' && key <= '~'){
			auto curStrLen = craftedItemName.size() + 1;
			if (curStrLen < MAA_TEXTBOX_MAX_LENGTH){
				if (craftedItemNamePos >= (int) craftedItemName.size()) {
					craftedItemName.push_back(key);
				}
				else {
					craftedItemName.insert(craftedItemName.begin() + craftedItemNamePos, key);
				}
				craftedItemNamePos++;
			}
			
		}
		
		annulMsg(msg);
		return TRUE;
	}

	if (msg->type == TigMsgType::WIDGET) { // scrolling
		auto _msg = (TigMsgWidget*)msg;
		if (_msg->widgetEventType== TigMsgWidgetEvent::Scrolled) {
			ui.ScrollbarGetY(mMaaItemsScrollbarId, &mMaaItemsScrollbarY);
			ui.ScrollbarGetY(mMaaApplicableEffectsScrollbarId, &mMaaApplicableEffectsScrollbarY);
		}
		return true;
	}
	
	return false;
}

BOOL ItemCreation::MaaTextboxMsg(int widId, TigMsg* msg){
	auto _msg = (TigMsgWidget*)msg;
	if (msg->type != TigMsgType::WIDGET || _msg->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return false;

	craftedItemNamePos = craftedItemName.size();
	craftingWidgetId = widId;

	return false;
}

bool ItemCreation::MaaWndRenderText(int widId, objHndl item){
	std::string text;

	// draw the textbox text
	TigRect rect(296, 72,170, 10);
	if (craftedItemNamePos > (int) craftedItemName.size())
		craftedItemNamePos = craftedItemName.size();
	if (craftedItemNamePos > 0)	{
		text.append(craftedItemName,0, craftedItemNamePos);
	}
	text.append("|");
	text.append(&craftedItemName[craftedItemNamePos]);

	auto maaTextboxStyle = temple::GetRef<TigTextStyle>(0x10BEDFE8);
	auto measWidth = UiRenderer::MeasureTextSize(text, maaTextboxStyle);
	while (measWidth.width > rect.width)	{
		text.erase(text.begin());
		measWidth = UiRenderer::MeasureTextSize(text, maaTextboxStyle);
	}
	UiRenderer::DrawTextInWidget(widId, text, rect, maaTextboxStyle);

	// draw the enhancement costs
	rect = TigRect(293, 90, 159, 10);
	text.clear();

	auto insuffXp = temple::GetRef<int>(0x10BEE3A4);
	auto insuffGp = temple::GetRef<int>(0x10BEE3A8);
	auto insuffSkill = temple::GetRef<int>(0x10BEE3AC);
	auto insuffPrereqs = temple::GetRef<int>(0x10BEE3B0);
	auto enhCostLabel = temple::GetRef<const char*>(0x10BED798);
	auto cpCost = MaaCpCost(CRAFT_EFFECT_INVALID);
	if (insuffXp || insuffGp || insuffSkill || insuffPrereqs){
		text.append(fmt::format("{} @{}{}", enhCostLabel, insuffGp+1, cpCost/100));
	} 
	else{
		text.append(fmt::format("{} @3{}", enhCostLabel, cpCost / 100));
	}
	auto& textStyle = temple::GetRef<TigTextStyle>(0x10BEE338);
	UiRenderer::DrawTextInWidget(widId, text, rect, textStyle);

	// draw xp cost
	rect.y += 11;
	text.clear();
	auto xpCostLabel = temple::GetRef<const char*>(0x10BED8A4);
	auto xpCost = MaaXpCost(CRAFT_EFFECT_INVALID);
	if (insuffXp || insuffGp || insuffSkill || insuffPrereqs){
		text.append(fmt::format("{} @{}{}", xpCostLabel, insuffXp + 1, xpCost));
	} 
	else	{
		text.append(fmt::format("{} @3{}", xpCostLabel, xpCost ));
	}
	UiRenderer::DrawTextInWidget(widId, text, rect, textStyle);

	// draw GP cost
	rect.y += 11;
	auto itemWorth = gameSystems->GetObj().GetObject(item)->GetInt32(obj_f_item_worth);
	text.clear();
	auto valueLabel = temple::GetRef<const char*>(0x10BED8A8);
	text.append(fmt::format("{} @3{}", valueLabel, itemWorth / 100));

	
	UiRenderer::DrawTextInWidget(widId, text, rect, textStyle);

	rect = TigRect(355, 152, 0,0);
	rect.x = 355;
	auto enhBon = MaaGetCurEnhBonus();
	text = fmt::format("{} {}", GetItemCreationMesLine(10009),enhBon);
	measWidth = UiRenderer::MeasureTextSize(text, textStyle);
	rect.width = measWidth.width;
	rect.height = measWidth.height;
	return UiRenderer::DrawTextInWidget(widId, text, rect, textStyle);
}

BOOL ItemCreation::MaaItemMsg(int widId, TigMsg* msg){

	auto _msg =(TigMsgWidget*)(msg);
	if (msg->type != TigMsgType::WIDGET || _msg->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return false;

	craftedItemNamePos = craftedItemName.size();
	craftingWidgetId = widId;
	auto widIdx = 0;
	for (widIdx = 0; widIdx < MAA_NUM_ENCHANTABLE_ITEM_WIDGETS; widIdx++)	{
		if (mMaaItemBtnIds[widIdx] == widId)
			break;
	}
	if (widIdx >= 5)
		widIdx = -1;

	auto itemIdx = mMaaItemsScrollbarY + widIdx;


	if (itemIdx >= 0 && itemIdx < (int) mMaaCraftableItemList.size()){
		auto itemHandle = mMaaCraftableItemList[itemIdx];
		craftingItemIdx = itemIdx;
		MaaInitCraftedItem(itemHandle);
	}

	return true;
}

BOOL ItemCreation::MaaEffectMsg(int widId, TigMsg* msg){

	if (msg->type != TigMsgType::WIDGET || msg->arg2 != 1)
		return false;

	craftedItemNamePos = craftedItemName.size();
	craftingWidgetId = widId;

	auto widIdx = 0;
	for (widIdx = 0 ; widIdx < MAA_EFFECT_BUTTONS_COUNT; widIdx++){
		if (maaBtnIds[widIdx] == widId)
			break;
	}
	if (widIdx >= 10)
		return false;

	auto effIdx = GetEffIdxFromWidgetIdx(widIdx);
	if (!MaaCrafterMeetsReqs(effIdx, itemCreationCrafter))
		return true;
	
	if (MaaEffectIsInAppliedList(effIdx))
		return true;

	int surplusXp = GetSurplusXp(itemCreationCrafter);
	if (MaaXpCost(effIdx) > surplusXp)
		return true;

	auto cpCost = MaaCpCost(effIdx);
	if (cpCost > party.GetMoney())
		return true;

	if (HasNecessaryEffects(effIdx))
		maaSelectedEffIdx = effIdx;

	return true;
}

void ItemCreation::MaaEffectRender(int widId){

	if (craftingItemIdx == -1)
		return;

	auto idx = 0;
	for (idx = 0; idx < MAA_EFFECT_BUTTONS_COUNT; idx++){
		if (maaBtnIds[idx] == widId)
			break;
	}
	if (idx >= MAA_EFFECT_BUTTONS_COUNT)
		return;

	auto effIdx = GetEffIdxFromWidgetIdx(idx);
	if (effIdx == CRAFT_EFFECT_INVALID)
		return;

	TigRect rect(207, idx * 12 + 152, 106, 12);
	auto effName = GetItemCreationMesLine(1000 + effIdx);

	TigTextStyle* textstyle;
	MaaEffectGetTextStyle(effIdx, itemCreationCrafter, textstyle);

	UiRenderer::DrawTextInWidget(mMaaWndId, effName, rect, *textstyle);
}

int ItemCreation::MaaEffectTooltip(int x, int y, int * widId){

	if (craftingItemIdx == -1)
		return 0;

	WidgetType2 * btn = ui.GetButton(*widId);
	if (btn->buttonState == UBS_DOWN || btn->buttonState == UBS_DISABLED)
		return 0;

	auto effIdx = GetEffIdxFromWidgetId(*widId);
	if (effIdx == CRAFT_EFFECT_INVALID)
		return 0;

	if (!MaaEffectIsApplicable(effIdx))
		return 0;
	
	for (auto it : appliedBonusIndices){
		if (it == effIdx) {
			return 0;
		}
	}


	auto& itEnh = itemEnhSpecs[effIdx];
	std::string text(fmt::format("{}", tooltips.GetTooltipString(6049))); // Requirements:
	
	if (itEnh.reqs.minLevel) {
		text.append(fmt::format("\n{} {}", ui.GetStatMesLine(273), itEnh.reqs.minLevel )); // Caster Level
	}
	if (itEnh.reqs.alignment) {
		if (itEnh.reqs.alignment & ALIGNMENT_GOOD)
			text.append(fmt::format("\n{} {}", ui.GetStatMesLine(238), ui.GetStatMesLine(8017)));
		else if (itEnh.reqs.alignment & ALIGNMENT_EVIL)
			text.append(fmt::format("\n{} {}", ui.GetStatMesLine(238), ui.GetStatMesLine(8011)));

		if (itEnh.reqs.alignment & ALIGNMENT_LAWFUL)
			text.append(fmt::format("\n{} {}", ui.GetStatMesLine(238), ui.GetStatMesLine(8022)));
		else if (itEnh.reqs.alignment & ALIGNMENT_CHAOTIC)
			text.append(fmt::format("\n{} {}", ui.GetStatMesLine(238), ui.GetStatMesLine(8004)));
	}
	
	if (itEnh.reqs.spells.size()) {
		text.append("\nSpells: ");
		bool isFirst = true;
		for (auto it : itEnh.reqs.spells){
			for (auto it2 : it.second) {
				if (isFirst) {
					text.append(fmt::format("{}", spellSys.GetSpellName(it2)));
					isFirst = false;
				}
				else
					text.append(fmt::format(",  or {}", spellSys.GetSpellName(it2)));
				
			}
		}
	}
	

	
	

	TigTextStyle style;
	const ColorRect cRect1(XMCOLOR(0xCC111111));
	style.bgColor = const_cast<ColorRect*>(&cRect1);
	const ColorRect cRectShadow(XMCOLOR(0xFF000000));
	style.shadowColor = const_cast<ColorRect*>(&cRectShadow);
	const ColorRect cRectText[3] = { ColorRect(0xFFFFFFFF), ColorRect(0xFFFFFF66), ColorRect(0xFFF33333) };
	style.textColor = const_cast<ColorRect*>(cRectText);

	style.flags = 0xC08;
	style.kerning = 2;
	style.tracking = 3;

	style.field2c = -1;

	auto charInvTtStyleIdx = temple::GetRef<int>(0x10BEECB0);
	auto tt = tooltips.GetStyle(charInvTtStyleIdx);

	UiRenderer::PushFont(tt.fontName, tt.fontSize);



	

	auto measuredSize = UiRenderer::MeasureTextSize(text, style);
	TigRect extents(x, y - measuredSize.height, measuredSize.width, measuredSize.height);
	if (extents.y  < 0) {
		extents.y = y;
	}
	auto wftWidth = temple::GetRef<int>(0x103012C8);
	if (extents.x + measuredSize.width > wftWidth) {
		extents.x = wftWidth - measuredSize.width;
	}
	UiRenderer::RenderText(text, extents, style);

	UiRenderer::PopFont();

	return 0;
}

void ItemCreation::MaaEffectGetTextStyle(int effIdx, objHndl crafter, TigTextStyle* &style){
	if (!MaaCrafterMeetsReqs(effIdx, crafter)){
		style = temple::GetPointer<TigTextStyle>(0x10BEDE40);
		return;
	}

	for (auto it: appliedBonusIndices)
	{
		if (it == effIdx){
			style = temple::GetPointer<TigTextStyle>(0x10BED998);
			return;
		}
	}

	int surplusXp = d20LevelSys.GetSurplusXp(crafter);
	if (MaaXpCost(effIdx) > surplusXp){
		style = temple::GetPointer<TigTextStyle>(0x10BECDB0);
		return;
	}

	auto cpCost = MaaCpCost(effIdx);
	if (cpCost > party.GetMoney())
	{
		style = temple::GetPointer<TigTextStyle>(0x10BED8B8);
		return;
	}

	if (!HasNecessaryEffects(effIdx)){
		style = temple::GetPointer<TigTextStyle>(0x10BEE2E0);
		return;
	}



	if (effIdx == maaSelectedEffIdx){
		style = temple::GetPointer<TigTextStyle>(0x10BEDF70);
		return;
	}

	style = temple::GetPointer<TigTextStyle>(0x10BEDDF0);
	return;
}


BOOL ItemCreation::MaaEffectAddMsg(int widId, TigMsg* msg)
{
	if (msg->type != TigMsgType::WIDGET || msg->arg2 != 1)
		return false;

	craftedItemNamePos = craftedItemName.size();
	auto effIdx = maaSelectedEffIdx;
	craftingWidgetId = widId;

	if (effIdx < 0 || effIdx == CRAFT_EFFECT_INVALID)
		return true;

	objHndl itemHandle = MaaGetItemHandle();
	if (!itemHandle)
		return true;

	if (!MaaCrafterMeetsReqs(effIdx, itemCreationCrafter))
		return true;

	if (MaaEffectIsInAppliedList(effIdx))
		return true;

	int surplusXp = GetSurplusXp(itemCreationCrafter);
	if (MaaXpCost(effIdx) > surplusXp)
		return true;

	auto cpCost = MaaCpCost(effIdx);
	if (cpCost > party.GetMoney())
		return true;

	if (HasNecessaryEffects(effIdx))
	{
		MaaAppendEnhancement(effIdx);
		CreateItemResourceCheck(itemCreationCrafter, itemHandle);
	}
	
	return true;
}

int ItemCreation::MaaGetTotalEffectiveBonus(int effIdx){


	// calculate the effective bonus from pre-existing effects
	auto effBonus = 0;
	for (auto it : appliedBonusIndices) {
		if (it != CRAFT_EFFECT_INVALID)
			effBonus += itemEnhSpecs[it].effectiveBonus;
	}

	if (effIdx == CRAFT_EFFECT_INVALID)
		return effBonus;


	// add the bonus level of the new effect
	auto &itEnh = itemEnhSpecs[effIdx];
	
	// if it's incremenetal, subtract the bonus from the antecedent since it's going to be removed
	if (itEnh.flags & IESF_INCREMENTAL) {
		if (itEnh.downgradesTo != CRAFT_EFFECT_INVALID) {
			effBonus -= itemEnhSpecs[itEnh.downgradesTo].effectiveBonus;
		}
	}

	// add the new effect's bonus to the total
	effBonus += itemEnhSpecs[effIdx].effectiveBonus;

	return effBonus;
}

void ItemCreation::MaaAppendEnhancement(int effIdx){

	if (effIdx == CRAFT_EFFECT_INVALID)
		return;

	if (!HasNecessaryEffects(effIdx))
		return;

	auto &itEnh = itemEnhSpecs[effIdx];

	int effBonus = MaaGetTotalEffectiveBonus(effIdx);

	// check that it doesn't exceed the maximum allowed Total Effective Bonus (10)
	if (effBonus > MAA_MAX_EFFECTIVE_BONUS) {
		return;
	}

	// remove the direct antecedent
	if ( (itEnh.flags & IESF_INCREMENTAL) && itEnh.downgradesTo != CRAFT_EFFECT_INVALID){
		for (auto it = appliedBonusIndices.begin(); it != appliedBonusIndices.end(); ++it){
			if (*it == itEnh.downgradesTo){
				appliedBonusIndices.erase(it);
				break;
			}
		}
	}
	

	appliedBonusIndices.push_back(effIdx);
	const char* icMesLine = GetItemCreationMesLine(1000 + effIdx);
	
	
	auto icLineLen = strlen(icMesLine);
	if (icLineLen + craftedItemNamePos + 1 <= MAA_TEXTBOX_MAX_LENGTH){

		// +X bonus; if +1, try to strip the "Masterwork" string; otherwise try to replace the previous bonus
		if (itEnh.flags & IESF_ENH_BONUS){
			// adding a +1 bonus - remove the "Masterwork" descriptor if it is there
			if (itEnh.data.enhBonus == 1){
				auto mwPos = craftedItemName.find("Masterwork ");
				if (mwPos != std::string::npos){
					craftedItemName.erase(craftedItemName.begin() + mwPos, craftedItemName.begin() + mwPos + 11);
				}
				craftedItemName.push_back(' ');
				craftedItemName.append(icMesLine);
			} 
			// adding a higher bonus - replace pre-existing bonuses
			else	{
				std::regex rreg("(\\+\\d)");
				std::smatch subMatches;
				std::regex_search(craftedItemName,subMatches, rreg);
				if (subMatches.size() > 1) {
					auto pos = craftedItemName.find(subMatches[1]);
					craftedItemName[pos+1]=icMesLine[1];
				} 
				else{
					craftedItemName.push_back(' ');
					craftedItemName.append(icMesLine);
				}
				
			}
			
		} 
		// else, prepend the effect name
		else if (itEnh.flags & IESF_INCREMENTAL && itEnh.downgradesTo != CRAFT_EFFECT_INVALID){
			auto oldIcLine = GetItemCreationMesLine(1000 + itEnh.downgradesTo);
			std::regex rreg(fmt::format("\\({}\\)",oldIcLine));

			std::smatch subMatches;
			std::regex_search(craftedItemName, subMatches, rreg);
			if (subMatches.size() > 1) {
				auto pos = craftedItemName.find(subMatches[1]);
				craftedItemName.erase(pos, pos + strlen(oldIcLine));
				craftedItemName.insert(pos, icMesLine);
			}
			else {
				craftedItemName.insert(0,fmt::format("{} ",icMesLine));
			}
		}
		else{
			craftedItemName.insert(0, fmt::format("{} ",icMesLine));
		}
		
	}
	craftedItemNamePos = craftedItemName.size();
}

BOOL ItemCreation::MaaEffectRemoveMsg(int widId, TigMsg* msg){
	if (msg->type != TigMsgType::WIDGET || msg->arg2 != 1)
		return false;

	craftedItemNamePos = craftedItemName.size();

	auto idx = mMaaActiveAppliedWidIdx;
	craftingWidgetId = widId;

	if (mMaaActiveAppliedWidIdx < 0 || mMaaActiveAppliedWidIdx >= NUM_APPLIED_BONUSES_MAX){
		return false;
	}

	objHndl itemHandle = MaaGetItemHandle();
	if (!itemHandle)
		return false;

	if (idx >= (int) appliedBonusIndices.size())
		return false;

	//auto effIdx = appliedBonusIndices[idx];
	auto displayCount = 0;
	auto appBonIdx = 0u;
	auto effIdx = CRAFT_EFFECT_INVALID;
	for (auto it : appliedBonusIndices) {
		if (itemEnhSpecs[it].flags & IESF_ENH_BONUS){
			appBonIdx++;
			continue;
		}
		if (displayCount == idx) {
			effIdx = it;
			break;
		}
		appBonIdx++;
		displayCount++;
	}

	if (effIdx == CRAFT_EFFECT_INVALID || ItemWielderCondsContainEffect(effIdx, itemHandle)){
		CreateItemResourceCheck(itemCreationCrafter, itemHandle);
		return false;
	}

	if (appBonIdx < appliedBonusIndices.size()){
		auto &itEnh = itemEnhSpecs[appliedBonusIndices[appBonIdx]];
		if ((itEnh.flags & IESF_INCREMENTAL) && itEnh.downgradesTo != CRAFT_EFFECT_INVALID){
			appliedBonusIndices[appBonIdx] = itEnh.downgradesTo;
		}
		else
			appliedBonusIndices.erase(appliedBonusIndices.begin() + appBonIdx);
		CreateItemResourceCheck(itemCreationCrafter, itemHandle);
		return true;
	}

	return false;
}

BOOL ItemCreation::MaaAppliedBtnMsg(int widId, TigMsg* msg){
	if (msg->type != TigMsgType::WIDGET || msg->arg2 != 1)
		return false;
	craftedItemNamePos = craftedItemName.size();
	craftingWidgetId = widId;
	auto widIdx = 0;
	while (mMaaAppliedBtnIds[widIdx] != widId)
	{
		widIdx++;
		if (widIdx >= NUM_APPLIED_BONUSES_MAX)
			return false;
	}

	mMaaActiveAppliedWidIdx = widIdx;
	return true;
}

BOOL ItemCreation::MaaEnhBonusUpMsg(int widId, TigMsg * msg){
	if (msg->type != TigMsgType::WIDGET || ((TigMsgWidget*)msg)->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return false;

	// get the current enh bonus
	auto curEnhBon = MaaGetCurEnhBonus();
	if (curEnhBon >= MAA_MAX_ENHANCEMENT_BONUS)
		return true;

	craftedItemNamePos = craftedItemName.size();

	objHndl itemHandle = MaaGetItemHandle();
	if (!itemHandle)
		return true;

	auto effIdx = MaaGetEffIdxForEnhBonus(curEnhBon + 1, itemHandle);
	craftingWidgetId = widId;

	if (effIdx == CRAFT_EFFECT_INVALID)
		return true;

	

	if (!MaaCrafterMeetsReqs(effIdx, itemCreationCrafter))
		return true;

	if (MaaEffectIsInAppliedList(effIdx))
		return true;

	int surplusXp = GetSurplusXp(itemCreationCrafter);
	if (MaaXpCost(effIdx) > surplusXp)
		return true;

	auto cpCost = MaaCpCost(effIdx);
	if (cpCost > party.GetMoney())
		return true;

	if (HasNecessaryEffects(effIdx)){
		MaaAppendEnhancement(effIdx);
		CreateItemResourceCheck(itemCreationCrafter, itemHandle);
	}

	return true;
}

BOOL ItemCreation::MaaEnhBonusDnMsg(int widId, TigMsg * msg){

	if (msg->type != TigMsgType::WIDGET || ((TigMsgWidget*)msg)->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return false;

	// get the current enh bonus
	auto curEnhBon = MaaGetCurEnhBonus();
	if (curEnhBon <= 0)
		return true;

	objHndl itemHandle = MaaGetItemHandle();
	if (!itemHandle)
		return true;

	auto effIdx = MaaGetEffIdxForEnhBonus(curEnhBon , itemHandle);

	if (ItemWielderCondsContainEffect(effIdx, itemHandle))
		return true;

	craftingWidgetId = widId;

	auto& itEnh = itemEnhSpecs[effIdx];
	
	for (auto it = appliedBonusIndices.begin(); it != appliedBonusIndices.end(); ++it) {
		if ( (itemEnhSpecs[*it].flags & IESF_ENH_BONUS ) && itemEnhSpecs[*it].data.enhBonus >= curEnhBon) {
			appliedBonusIndices.erase(it);
			break;
		}
	}
	
	if (curEnhBon == 1) {
		appliedBonusIndices.clear();
		CreateItemResourceCheck(itemCreationCrafter, itemHandle);
		return false;
	}

	return false;
}

int ItemCreation::UiItemCreationInit(GameSystemConf& conf)
{
	mCreateBtnRect = TigRect(133, 339, 112, 22);
	mMaaCancelBtnRect = TigRect(256, 339, 112, 22);
	mMaaCraftedItemIconDestRect = TigRect(215, 62, 64, 64);
	mItemCreationScrollbar = new WidgetType3;
	mEnhBonusDnRect = TigRect(450, 156, 15, 9);

	if (!mesFuncs.Open("tpmes\\item_creation.mes", &mItemCreationMes))
		return 0;
	temple::GetRef<MesHandle>(0x10BEDFD0) = mItemCreationMes;
	if (!mesFuncs.Open("rules\\item_creation.mes", temple::GetPointer<MesHandle>(0x10BEDA90)))
		return 0;
	if (!mesFuncs.Open("mes\\item_creation_names.mes", temple::GetPointer<MesHandle>(0x10BEDB4C)))
		return 0;

	if (!InitItemCreationRules())
		return 0;

	ui.GetAsset(UiAssetType::Generic,  UiGenericAsset::AcceptNormal,temple::GetRef<int>(0x10BED9F0));
	ui.GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptHover, temple::GetRef<int>(0x10BEDA48));
	ui.GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptPressed, temple::GetRef<int>(0x10BED9EC));
	ui.GetAsset(UiAssetType::Generic, UiGenericAsset::DisabledNormal, temple::GetRef<int>(0x10BEDB48));
	ui.GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineNormal, temple::GetRef<int>(0x10BEDA5C));
	ui.GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineHover, temple::GetRef<int>(0x10BEE2D4));
	ui.GetAsset(UiAssetType::Generic, UiGenericAsset::DeclinePressed, temple::GetRef<int>(0x10BED6D0));

	bkgImage = new CombinedImgFile("art\\interface\\item_creation_ui\\item_creation.img");

	if (temple::Dll::GetInstance().HasCo8Hooks()){	
		mUseCo8Ui = true;
		if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\ITEM_CREATION_WIDENED_0_0.tga", &mItemCreationWidenedTexture00))
			return 0;
		if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\ITEM_CREATION_WIDENED_1_0.tga", &mItemCreationWidenedTexture10))
			return 0;
		if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\ITEM_CREATION_WIDENED_0_1.tga", &mItemCreationWidenedTexture01))
			return 0;
		if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\ITEM_CREATION_WIDENED_1_1.tga", &mItemCreationWidenedTexture11))
			return 0;
	}

	if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\craftarms_0.tga", temple::GetPointer<int>(0x10BEE38C)))
		return 0;
	if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\craftarms_1.tga", temple::GetPointer<int>(0x10BECEE8)))
		return 0;
	if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\craftarms_2.tga", temple::GetPointer<int>(0x10BED988)))
		return 0;
	if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\craftarms_3.tga", temple::GetPointer<int>(0x10BECEEC)))
		return 0;
	if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\invslot_selected.tga", temple::GetPointer<int>(0x10BECDAC)))
		return 0;
	if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\invslot.tga", temple::GetPointer<int>(0x10BEE038)))
		return 0;
	if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\add_button.tga", temple::GetPointer<int>(0x10BEE334)))
		return 0;
	if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\add_button_grey.tga", temple::GetPointer<int>(0x10BED990)))
		return 0;
	if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\add_button_hover.tga", temple::GetPointer<int>(0x10BEE2D8)))
		return 0;
	if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\add_button_press.tga", temple::GetPointer<int>(0x10BED79C)))
		return 0;
	if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\down_arrow.tga", &mDownArrowTga)
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\down_arrow_click.tga", &mDownArrowClickTga)
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\down_arrow_disabled.tga", &mDownArrowDisabledTga)
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\down_arrow_hovered.tga", &mDownArrowHoveredTga)
		)
		throw TempleException("Missing textures for Item Creation UI!");


	/*
	  Init Widgets
	*/
	if (!UiItemCreationWidgetsInit(conf.width, conf.height))
		return 0;

	if (!MaaWidgetsInit(conf.width, conf.height))
		return 0;

	if (! temple::GetRef<bool(__cdecl)()>(0x1014F4D0)()) //   ItemCreationStringsGet()
		return 0;


	auto& icWnd = temple::GetRef<WidgetType1>(0x10BEE040);
	auto& maaWnd = temple::GetRef<WidgetType1>(0x10BEDB58);

	auto& rect = temple::GetRef<TigRect>(0x102FAEC4);
	rect.x += icWnd.x;	rect.y += icWnd.y;

	mMaaCraftedItemIconDestRect.x += maaWnd.x;	mMaaCraftedItemIconDestRect.y += maaWnd.y;

	auto& rect3 = temple::GetRef<TigRect>(0x102FAEF4);
	rect3.x += icWnd.x;	rect3.y += icWnd.y;

	auto& rect4 = temple::GetRef<TigRect>(0x102FAF14);
	rect4.x += icWnd.x;	rect4.y += icWnd.y;

	mCreateBtnRect.x += maaWnd.x;	mCreateBtnRect.y += maaWnd.y;
	mMaaCancelBtnRect.x += maaWnd.x; mMaaCancelBtnRect.y += maaWnd.y;

	itemCreationType = ItemCreationType::Inactive;



	itemCreationCrafter = temple::GetRef<objHndl>(0x10BECEE0);
	// craftingItemIdx = temple::GetRef<int>(0x10BEE398);

	//MAA
	// maaSelectedEffIdx = temple::GetRef<int>(0x10BECD74);
	mMaaWnd = temple::GetPointer<WidgetType1>(0x10BEDB58);
	// mMaaItemsScrollbarId = temple::GetRef<int>(0x10BED8A0);
	// mMaaApplicableEffectsScrollbarId = temple::GetRef<int>(0x10BECD78);

	mItemCreationWnd = temple::GetPointer<WidgetType1>(0x10BEE040);
	// mItemCreationScrollbarId = temple::GetRef<int>(0x10BED9F4);
	// mMaaItemsScrollbarY = temple::GetRef<int>(0x10BECDA4);

	return 1;
}

bool ItemCreation::InitItemCreationRules(){
	auto fname = "rules\\item_creation.mes";
	MesHandle icrules;
	auto mesFile = mesFuncs.Open(fname, &icrules);
	if (!mesFile) {
		logger->error("Cannot open rules\\item_creation.mes!");
		return false;
	}
	for (auto i = (int)ItemCreationType::IC_Alchemy; i < ItemCreationType::Inactive; i++){
		if (i == ItemCreationType::CraftMagicArmsAndArmor){
			mMaaCraftableItemList.clear();
		}
		else
		{
			MesLine line(i);
			mesFuncs.GetLine_Safe(icrules, &line);
			numItemsCrafting[i] = 0;
			{
				StringTokenizer craftableTok(line.value);
				while (craftableTok.next()) {
					if (craftableTok.token().type == StringTokenType::Number)
						++numItemsCrafting[i];
				}
			}
			craftedItemHandles[i] = new objHndl[numItemsCrafting[i]+1];
			memset(craftedItemHandles[i], 0, sizeof(objHndl) *(numItemsCrafting[i] + 1));
			if (numItemsCrafting[i] > 0){
				int j = 0;
				StringTokenizer craftableTok(line.value);
				while (craftableTok.next()) {
					if (craftableTok.token().type == StringTokenType::Number){
						auto protoHandle = gameSystems->GetObj().GetProtoHandle(craftableTok.token().numberInt);
						if (protoHandle)
							craftedItemHandles[i][j++] = protoHandle;
					}
						
				}
				numItemsCrafting[i] = j; // in case there are invalid protos
				std::sort(craftedItemHandles[i], &craftedItemHandles[i][numItemsCrafting[i]],
					[](objHndl first, objHndl second){
					auto firstName = itemCreation.ItemCreationGetItemName(first);
					auto secondName = itemCreation.ItemCreationGetItemName(second);
					auto comRes = _stricmp(firstName, secondName);
					return comRes < 0;
				});
			}
			

		}
		
	}

	mesFuncs.Close(icrules);

	return true;
}

bool ItemCreation::UiItemCreationWidgetsInit(int width, int height){
	auto wndId = &mItemCreationWndId;
	auto& wnd = temple::GetRef<WidgetType1>(0x10BEE040);
	wnd.WidgetType1Init((width - 404 - 108*mUseCo8Ui) / 2, (height - 421) / 2, 404+ 108*mUseCo8Ui, 421);
	wnd.widgetFlags = 1;
	wnd.windowId = 0x7FFFFFFF;
	wnd.render = [](int widId) { itemCreation.ItemCreationWndRender(widId); };
	wnd.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.ItemCreationWndMsg(widId, msg); };//temple::GetRef<bool(__cdecl)(int, TigMsg*)>(0x1014FC20);

	if (ui.AddWindow(temple::GetPointer<WidgetType1>(0x10BEE040), sizeof(WidgetType1),
		wndId, "ui_item_creation.cpp", 2094))
		return false;

	if (mItemCreationScrollbar->Init(185 + mUseCo8Ui * 108, 51, 259))
		return false;
	mItemCreationScrollbar->scrollQuantum = 3;
	mItemCreationScrollbar->x += wnd.x;
	mItemCreationScrollbar->y += wnd.y;

	if (mItemCreationScrollbar->Add(&mItemCreationScrollbarId) || ui.BindToParent(*wndId, mItemCreationScrollbarId))
		return false;

	auto btnY = 55;

	for (int i = 0; i < NUM_ITEM_CREATION_ENTRY_WIDGETS; i++){
		WidgetType2 btn(nullptr, *wndId, 32, btnY, 155 + 108 * mUseCo8Ui, 12);
		btn.x += wnd.x;
		btn.y += wnd.y;
		btn.render = [](int widId) {itemCreation.ItemCreationEntryRender(widId); };
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.ItemCreationEntryMsg(widId, msg); };
		ui.AddButton(&btn, sizeof(WidgetType2), &mItemCreationEntryBtnIds[i], "ui_item_creation.cpp", 2115);
		if (ui.BindToParent(*wndId, mItemCreationEntryBtnIds[i]))
			return false;
		btnY += 12;
	}
	// create button
	{
		WidgetType2 btn(nullptr, *wndId, 81, 373, 112, 22);
		btn.x += wnd.x;
		btn.y += wnd.y;
		btn.render = [](int widId) { itemCreation.ItemCreationCreateBtnRender(widId); };
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.CreateBtnMsg(widId, msg); };

		ui.AddButton(&btn, sizeof(WidgetType2), &mItemCreationCreateBtnId, "ui_item_creation.cpp", 2127);
		ui.SetDefaultSounds(mItemCreationCreateBtnId);
		ui.BindToParent(*wndId, mItemCreationCreateBtnId);
	}
	// cancel button
	auto cancelBtnId = temple::GetPointer<int>(0x10BEDA68);
	
	WidgetType2 btn(nullptr, *wndId, 205 + 108*mUseCo8Ui, 373, 112, 22);
	btn.x += wnd.x;
	btn.y += wnd.y;
	btn.render = [](int widId) {itemCreation.ItemCreationCancelBtnRender(widId); };
	btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.CancelBtnMsg(widId, msg); };

	ui.AddButton(&btn, sizeof(WidgetType2), cancelBtnId, "ui_item_creation.cpp", 2142);
	ui.SetDefaultSounds(*cancelBtnId);
	return ui.BindToParent(*wndId, *cancelBtnId) == 0;
	
}

bool ItemCreation::MaaWidgetsInit(int width, int height) {
	auto& wnd = temple::GetRef<WidgetType1>(0x10BEDB58);
	auto wndId = &mMaaWndId;
	wnd.WidgetType1Init((width - 504) / 2, (height - 387) / 2, 504, 387);
	wnd.widgetFlags = 1;
	wnd.render = [](int widId) {itemCreation.MaaWndRender(widId); };
	wnd.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.MaaWndMsg(widId, msg); };
	wnd.windowId = 0x7FFFFFFF;
	if (wnd.Add(wndId))
		return false;

	// Scrollbar for the Items
	auto maaScrollbar = temple::GetPointer<WidgetType3>(0x10BEDA98);
	maaScrollbar->Init(184, 51, 225);
	maaScrollbar->x += wnd.x;
	maaScrollbar->y += wnd.y;
	maaScrollbar->Add(&mMaaItemsScrollbarId);
	if (ui.BindToParent(*wndId, mMaaItemsScrollbarId))
		return false;


	// Scrollbar for the effects
	auto appEffectsScrollbar = temple::GetPointer<WidgetType3>(0x10BEDE90);
	appEffectsScrollbar->Init(313, 148, 128);
	appEffectsScrollbar->x += wnd.x;
	appEffectsScrollbar->y += wnd.y;
	appEffectsScrollbar->Add(&mMaaApplicableEffectsScrollbarId);
	if (ui.BindToParent(*wndId, mMaaApplicableEffectsScrollbarId))
		return false;

	// Item buttons
	auto btnY = 53;
	for (int i = 0; i < MAA_NUM_ENCHANTABLE_ITEM_WIDGETS; i++) {
		WidgetType2 btn(nullptr, *wndId, 28, btnY, 152, 42);
		btn.x += wnd.x;	btn.y += wnd.y;
		btn.render = [](int widId) { itemCreation.MaaItemRender(widId); };
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.MaaItemMsg(widId, msg); };
		btn.Add(&mMaaItemBtnIds[i]);
		ui.BindToParent(*wndId, mMaaItemBtnIds[i]);
		btnY += 42;
	}

	// applicable effect butons
	btnY = 152;
	for (int i = 0; i < MAA_EFFECT_BUTTONS_COUNT; i++) {
		WidgetType2 btn(nullptr, *wndId, 207, btnY, 106, 12);
		btn.x += wnd.x;	btn.y += wnd.y;
		btn.render = [](int widId) { itemCreation.MaaEffectRender(widId); };
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.MaaEffectMsg(widId, msg); };
		btn.renderTooltip = [](int x, int y, int* widId) { return itemCreation.MaaEffectTooltip(x, y, widId); };
		btn.Add(&maaBtnIds[i]);
		ui.BindToParent(*wndId, maaBtnIds[i]);
		btnY += 12;
	}

	// Enhancement bonus buttons
	WidgetType2 enhBonusDown(nullptr, *wndId, mEnhBonusDnRect);
	enhBonusDown.x += wnd.x; enhBonusDown.y += wnd.y;
	enhBonusDown.render = [](int widId) {itemCreation.MaaEnhBonusDnRender(widId); };
	enhBonusDown.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.MaaEnhBonusDnMsg(widId, msg); };
	enhBonusDown.Add(&mEnhBonusArrowDnId);
	ui.BindToParent(*wndId, mEnhBonusArrowDnId);
	ui.SetDefaultSounds(mEnhBonusArrowDnId);

	WidgetType2 enhBonusUp(nullptr, *wndId, mEnhBonusDnRect);
	enhBonusUp.x += wnd.x - 15; enhBonusUp.y += wnd.y;
	enhBonusUp.render = [](int widId) {itemCreation.MaaEnhBonusUpRender(widId); };
	enhBonusUp.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.MaaEnhBonusUpMsg(widId, msg); };
	enhBonusUp.Add(&mEnhBonusArrowUpId);
	ui.BindToParent(*wndId, mEnhBonusArrowUpId);
	ui.SetDefaultSounds(mEnhBonusArrowUpId);

	// applied effects
	btnY = 152 + 12;
	for (int i = 0; i < NUM_APPLIED_BONUSES_MAX; i++) {
		WidgetType2 btn(nullptr, *wndId, 355, btnY, 106, 12);
		btn.x += wnd.x;	btn.y += wnd.y;
		btn.render = [](int widId) { itemCreation.MaaAppliedBtnRender(widId); };
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.MaaAppliedBtnMsg(widId, msg); };
		btn.Add(&mMaaAppliedBtnIds[i]);
		ui.BindToParent(*wndId, mMaaAppliedBtnIds[i]);
		btnY += 12;
	}

	// create button
	//auto createBtnId = temple::GetPointer<int>(0x10BED8B0);
	{
		WidgetType2 btn(nullptr, *wndId, 132, 340, 112, 22);
		btn.x += wnd.x;
		btn.y += wnd.y;
		btn.render = [](int widId) { itemCreation.MaaCreateBtnRender(widId); };
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.CreateBtnMsg(widId, msg); };

		ui.AddButton(&btn, sizeof(WidgetType2), &mMaaCreateBtnId, "ui_item_creation.cpp", 2224);
		ui.SetDefaultSounds(mMaaCreateBtnId);
		ui.BindToParent(*wndId, mMaaCreateBtnId);
	}
	// cancel button
	//auto cancelBtnId = temple::GetPointer<int>(0x10BECD70);
	{
		WidgetType2 btn(nullptr, *wndId, 256, 340, 112, 22);
		btn.x += wnd.x;
		btn.y += wnd.y;
		btn.render = [](int widId) { itemCreation.MaaCancelBtnRender(widId); };
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.CancelBtnMsg(widId, msg); };

		ui.AddButton(&btn, sizeof(WidgetType2), &mMaaCancelBtnId, "ui_item_creation.cpp", 2237);
		ui.SetDefaultSounds(mMaaCancelBtnId);
		ui.BindToParent(*wndId, mMaaCancelBtnId);
	}


	// Add Effect button
	//auto effectAddBtnId = temple::GetPointer<int>(0x10BEE394);
	{
		WidgetType2 btn(nullptr, *wndId, 333, 189, temple::GetRef<int>(0x102FAF5C), temple::GetRef<int>(0x102FAF60));
		btn.x += wnd.x;
		btn.y += wnd.y;
		temple::GetRef<int>(0x102FAF54) = 333 + wnd.x;
		temple::GetRef<int>(0x102FAF58) = 189 + wnd.y;
		btn.render = temple::GetRef<void(__cdecl)(int)>(0x10150020);
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.MaaEffectAddMsg(widId, msg); };

		ui.AddButton(&btn, sizeof(WidgetType2), &mMaaEffectAddBtnId, "ui_item_creation.cpp", 2237);
		ui.SetDefaultSounds(mMaaEffectAddBtnId);
		ui.BindToParent(*wndId, mMaaEffectAddBtnId);
	}

	// Remove Effect button
	//auto effectRemoveBtnId = temple::GetPointer<int>(0x10BEE394);
	{
		WidgetType2 btn(nullptr, *wndId, 335, 220, temple::GetRef<int>(0x102FAF6C), temple::GetRef<int>(0x102FAF70));
		btn.x += wnd.x;
		btn.y += wnd.y;
		temple::GetRef<int>(0x102FAF64) = 335 + wnd.x;
		temple::GetRef<int>(0x102FAF68) = 220 + wnd.y;
		btn.render = temple::GetRef<void(__cdecl)(int)>(0x101500B0);
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.MaaEffectRemoveMsg(widId, msg); };

		ui.AddButton(&btn, sizeof(WidgetType2), &mMaaEffectRemoveBtnId, "ui_item_creation.cpp", 2237);
		ui.SetDefaultSounds(mMaaEffectRemoveBtnId);
		ui.BindToParent(*wndId, mMaaEffectRemoveBtnId);
	}

	// Textbox
	//auto maaTextboxId = temple::GetPointer<int>(0x10BED9E8); // now internal member
	auto& maaTextboxRect = temple::GetRef<TigRect>(0x102FAF74);
	maaTextboxRect.x = 296; maaTextboxRect.y = 72;
	{
		WidgetType2 btn(nullptr, *wndId, maaTextboxRect);
		btn.x += wnd.x;
		btn.y += wnd.y;
		maaTextboxRect.x += wnd.x;
		maaTextboxRect.y += wnd.y;
		// render is handled in the main window
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation.MaaTextboxMsg(widId, msg); };

		ui.AddButton(&btn, sizeof(WidgetType2), &mMaaTextboxId, "ui_item_creation.cpp", 2286);
		ui.SetDefaultSounds(mMaaTextboxId);
		return ui.BindToParent(*wndId, mMaaTextboxId) == 0;
	}

}

void ItemCreation::MaaWidgetsExit(int widId){
	ui.WidgetRemoveRegardParent(mMaaTextboxId);
	ui.WidgetRemoveRegardParent(mMaaEffectRemoveBtnId);
	ui.WidgetRemoveRegardParent(mMaaEffectAddBtnId);
	ui.WidgetRemoveRegardParent(mMaaCreateBtnId);
	ui.WidgetRemoveRegardParent(mMaaCancelBtnId);
	for (int i = 0; i < NUM_APPLIED_BONUSES_MAX; i++){
		ui.WidgetRemoveRegardParent(mMaaAppliedBtnIds[i]);
	}
	for (int i = 0; i < MAA_EFFECT_BUTTONS_COUNT; i++) {
		ui.WidgetRemoveRegardParent(maaBtnIds[i]);
	}
	for (int i = 0; i < MAA_NUM_ENCHANTABLE_ITEM_WIDGETS; i++)	{
		ui.WidgetRemoveRegardParent(mMaaItemBtnIds[i]);
	}
	ui.WidgetAndWindowRemove(widId);

}

void ItemCreation::ItemCreationWidgetsExit(int widId){
	ui.WidgetRemoveRegardParent(temple::GetRef<int>(0x10BED8B0)); // create button
	ui.WidgetRemoveRegardParent(temple::GetRef<int>(0x10BEDA68)); // cancel button
	auto icEntryBtnIds = temple::GetRef<int[NUM_DISPLAYED_CRAFTABLE_ITEMS_MAX]>(0x10BECE28);
	for (int i = 0; i < NUM_DISPLAYED_CRAFTABLE_ITEMS_MAX; i++){
		ui.WidgetRemoveRegardParent(icEntryBtnIds[i]);
	}
	ui.WidgetAndWindowRemove(widId);
}

void ItemCreation::UiItemCreationResize(UiResizeArgs& resizeArgs){

	auto& icWnd = temple::GetRef<WidgetType1>(0x10BEE040);
	auto& maaWnd = temple::GetRef<WidgetType1>(0x10BEDB58);

	auto& rect = temple::GetRef<TigRect>(0x102FAEC4);
	rect.x -= icWnd.x;	rect.y -= icWnd.y;

	mMaaCraftedItemIconDestRect.x -= maaWnd.x;	mMaaCraftedItemIconDestRect.y -= maaWnd.y;

	auto& rect3 = temple::GetRef<TigRect>(0x102FAEF4);
	rect3.x -= icWnd.x;	rect3.y -= icWnd.y;

	auto& rect4 = temple::GetRef<TigRect>(0x102FAF14);
	rect4.x -= icWnd.x;	rect4.y -= icWnd.y;

	
	mCreateBtnRect.x -= maaWnd.x;	mCreateBtnRect.y -= maaWnd.y;
	mMaaCancelBtnRect.x -= maaWnd.x;	mMaaCancelBtnRect.y -= maaWnd.y;
	//mEnhBonusDnRect.x -= maaWnd.x; mEnhBonusDnRect.y -= maaWnd.y;
	
	MaaWidgetsExit(mMaaWndId);
	ItemCreationWidgetsExit(mItemCreationWndId);
	

	UiItemCreationWidgetsInit(resizeArgs.rect1.width, resizeArgs.rect1.height);
	MaaWidgetsInit(resizeArgs.rect1.width, resizeArgs.rect1.height);


	temple::GetRef<int(__cdecl)()>(0x1014F4D0)(); // get strings

	rect.x += icWnd.x;	rect.y += icWnd.y;

	mMaaCraftedItemIconDestRect.x += maaWnd.x;	mMaaCraftedItemIconDestRect.y += maaWnd.y;

	rect3.x += icWnd.x;	rect3.y += icWnd.y;

	rect4.x += icWnd.x;	rect4.y += icWnd.y;

	
	mCreateBtnRect.x += maaWnd.x;	mCreateBtnRect.y += maaWnd.y;
	mMaaCancelBtnRect.x += maaWnd.x;	mMaaCancelBtnRect.y += maaWnd.y;
	// mEnhBonusDnRect.x += maaWnd.x; mEnhBonusDnRect.y += maaWnd.y;
	

	return;
}

void ItemCreationHooks::HookedGetLineForMaaAppend(MesHandle handle, MesLine* line)
{
	
	auto result = mesFuncs.GetLine_Safe(handle, line);
	const char emptyString [1] = "";
	
	auto craftedName = temple::GetPointer<char>(0x10BED758);
	craftedName[62] = craftedName[63] =0; // ensure string termination (some users are naughty...)

	auto craftedNameLen = strlen(craftedName);
	auto enhancementNameLen = strlen(line->value);
	if (enhancementNameLen + craftedNameLen > 60){
		line->value = emptyString;
	}
	



}

int ItemCreation::MaaCpCost(int effIdx){

	auto icType = itemCreationType;

	if (craftingItemIdx < 0 || (uint32_t) craftingItemIdx >= numItemsCrafting[icType])
		return 0;

	objHndl itemHandle = objHndl::null;
	if (icType == ItemCreationType::CraftMagicArmsAndArmor)
	{
		if ((size_t) craftingItemIdx >= mMaaCraftableItemList.size())
			return 0;
		itemHandle = mMaaCraftableItemList[craftingItemIdx];
	} else
	{
		itemHandle = craftedItemHandles[icType][craftingItemIdx];
	}
	
	if (!itemHandle)
		return 0;

	// get the overall effective bonus level
	auto effBonus = MaaGetTotalEffectiveBonus(effIdx);

	if (effBonus > 29)
		effBonus = 29;

	if (gameSystems->GetObj().GetObject(itemHandle)->type == obj_t_weapon){
		return 50 * (GoldBaseWorthVsEffectiveBonus[effBonus] - GoldBaseWorthVsEffectiveBonus[craftedItemExistingEffectiveBonus]);
	}
	return 50 * (GoldCraftCostVsEffectiveBonus[effBonus] - GoldCraftCostVsEffectiveBonus[craftedItemExistingEffectiveBonus]);
}

int ItemCreation::MaaXpCost(int effIdx){

	auto icType = itemCreationType;

	if (craftingItemIdx < 0 || (uint32_t) craftingItemIdx >= numItemsCrafting[icType])
		return 0;

	objHndl itemHandle = objHndl::null;
	if (icType == ItemCreationType::CraftMagicArmsAndArmor)
	{
		if ((size_t) craftingItemIdx >= mMaaCraftableItemList.size())
			return 0;
		itemHandle = mMaaCraftableItemList[craftingItemIdx];
	}
	else
	{
		itemHandle = craftedItemHandles[icType][craftingItemIdx];
	}

	if (!itemHandle)
		return 0;

	// get the overall effective bonus level
	auto effBonus = MaaGetTotalEffectiveBonus(effIdx);

	if (effBonus > 29)
		effBonus = 29;

	if (gameSystems->GetObj().GetObject(itemHandle)->type == obj_t_weapon) {
		return (GoldBaseWorthVsEffectiveBonus[effBonus] - GoldBaseWorthVsEffectiveBonus[craftedItemExistingEffectiveBonus]) / 25;
	}
	return (GoldCraftCostVsEffectiveBonus[effBonus] - GoldCraftCostVsEffectiveBonus[craftedItemExistingEffectiveBonus]) / 25;
}
