
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
#include <mod_support.h>
#include <float_line.h>
#include <ui/ui_dialog.h>
#include "ui_legacysystems.h"
#include "ui_systems.h"
#include <infrastructure/tabparser.h>
#include "ui_assets.h"
#include "d20_race.h"
#include "gamesystems/d20/d20stats.h"
#include "bonus.h"

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
	{"iesf_two_handed", IESF_TWO_HANDED },

	{"iesf_melee",IESF_MELEE },
	{"iesf_thrown",IESF_THROWN },
	{"iesf_unk100",IESF_UNK100 },
	{"iesf_plus_bonus",IESF_ENH_BONUS },
	{"iesf_incremental", IESF_INCREMENTAL },
	{"iesf_noncore", IESF_NONCORE },
	{"iesf_light_only", IESF_LIGHT_ONLY },
};

int WandCraftCostCp=0;

static inline UiItemCreation &itemCreation() {
	return uiSystems->GetItemCreation();
}

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

	static int HookedStatLevelGetForItemCreationPrereq(objHndl handle);

	static BOOL HookedIsSpellKnown(objHndl handle, int spellEnum); // for Lax Rules overriding of spell requirements in crafting Wondrous Item

	static char* GetCraftingPrereqString(objHndl crafter, objHndl item);

	void apply() override {
		// auto system = UiSystem::getUiSystem("ItemCreation-UI");		
		// system->init = systemInit;
		
		replaceFunction<char*(__cdecl)(objHndl, objHndl)>(0x10152410, GetCraftingPrereqString);

		// UiItemCreationIsActive
		replaceFunction<BOOL(__cdecl)()>(0x1014F180, [](){
			return itemCreation().IsActive();
		});

		// System Funcs
		replaceFunction<void(__cdecl)(UiResizeArgs&)>(0x10154E90, [](UiResizeArgs& arg){
			itemCreation().UiItemCreationResize(arg);
		});


		redirectCall(0x1015068D, HookedStatLevelGetForItemCreationPrereq);

		// Show
		replaceFunction<BOOL(__cdecl)(objHndl, ItemCreationType)>(0x101536C0, [](objHndl crafter, ItemCreationType icTypeNew){
			return itemCreation().ItemCreationShow(crafter, icTypeNew);
		});

		replaceFunction<int(__cdecl)(objHndl, objHndl)>(0x10152690, [](objHndl crafter, objHndl item) {
			return itemCreation().CreateItemResourceCheck(crafter, item) ? 1 : 0;
		});


		// MAA Window Message Handler
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10153110, [](int widId, TigMsg* msg)	{
			return itemCreation().MaaWndMsg(widId, msg);
		});

		// MAA Textbox
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10151890, [](int widId, TigMsg* msg) {
			return itemCreation().MaaTextboxMsg(widId, msg);
		});
		replaceFunction<bool(__cdecl)(int , objHndl)>(0x10151C10, [](int widId, objHndl item)
		{
			return itemCreation().MaaWndRenderText(widId, item);
		});

		// MAA selected item for crafting
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10152E40, [](int widId, TigMsg* msg) {
			return itemCreation().MaaItemMsg(widId, msg); }
		);

		// MAA Effect "buttons"
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10153250, [](int widId, TigMsg* msg) {
			return itemCreation().MaaEffectMsg(widId, msg); }
		);
		replaceFunction<void(__cdecl)(int)>(0x10153990, [](int widId) {
			return itemCreation().MaaEffectRender(widId); }
		);
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10152ED0, [](int widId, TigMsg* msg) {
			return itemCreation().MaaEffectAddMsg(widId, msg);
		});
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10152FE0, [](int widId, TigMsg* msg) {
			return itemCreation().MaaEffectRemoveMsg(widId, msg);
		});
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x10151EF0, [](int widId, TigMsg* msg) {
			return itemCreation().MaaAppliedBtnMsg(widId, msg);
		});

		// CreateBtnMsg
		replaceFunction<BOOL(int, TigMsg * )>(0x10153F60, [](int widId, TigMsg* msg){
			return itemCreation().CreateBtnMsg(widId, msg);
		});

		// CancelBtnMsg
		replaceFunction<BOOL(int, TigMsg *)>(0x10153820, [](int widId, TigMsg* msg) {
			return itemCreation().CancelBtnMsg(widId, msg);
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

		redirectCall(0x10150806, HookedIsSpellKnown);
	}

} itemCreationHooks;

//*****************************************************************************
//* ItemCreation-UI
//*****************************************************************************

UiItemCreation::UiItemCreation(const UiSystemConf &config) {

	for (int i = 0; i < 30; i++) {
		GoldCraftCostVsEffectiveBonus[i] = 1000 * i*i;
		GoldBaseWorthVsEffectiveBonus[i] = GoldCraftCostVsEffectiveBonus[i] * 2;
	}

	craftedItemExistingEffectiveBonus = -1; // stores the crafted item existing (pre-crafting) effective bonus
											//craftingItemIdx = -1;
	craftedItemExtraGold = 0;  //stores the crafted item existing (pre-crafting) extra gold cost

	memset(numItemsCrafting, 0, sizeof(numItemsCrafting));
	memset(craftedItemHandles, 0, sizeof(craftedItemHandles));
	craftedItemNamePos = 0;
	craftingWidgetId = -1;
	scribeScrollSpells.clear();
	mScribedScrollSpell = 0;

	LoadMaaSpecs();

	mCreateBtnRect = TigRect(133, 339, 112, 22);
	mMaaCancelBtnRect = TigRect(256, 339, 112, 22);
	mMaaCraftedItemIconDestRect = TigRect(215, 62, 64, 64);
	mItemCreationScrollbar = new LgcyScrollBar;
	mEnhBonusDnRect = TigRect(450, 156, 15, 9);

	if (!mesFuncs.Open("tpmes\\item_creation.mes", &mItemCreationMes))
		throw TempleException("Unable to open item_creation.mes");
	temple::GetRef<MesHandle>(0x10BEDFD0) = mItemCreationMes;
	if (!mesFuncs.Open("rules\\item_creation.mes", temple::GetPointer<MesHandle>(0x10BEDA90)))
		throw TempleException("Unable to open item_creation.mes");
	if (!mesFuncs.Open("mes\\item_creation_names.mes", temple::GetPointer<MesHandle>(0x10BEDB4C)))
		throw TempleException("Unable to open item_creation_names.mes");

	if (!InitItemCreationRules())
		throw TempleException("Unable to initialize item creation rules");

	uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptNormal, temple::GetRef<int>(0x10BED9F0));
	uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptHover, temple::GetRef<int>(0x10BEDA48));
	uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptPressed, temple::GetRef<int>(0x10BED9EC));
	uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::DisabledNormal, temple::GetRef<int>(0x10BEDB48));
	uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineNormal, temple::GetRef<int>(0x10BEDA5C));
	uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineHover, temple::GetRef<int>(0x10BEE2D4));
	uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::DeclinePressed, temple::GetRef<int>(0x10BED6D0));

	bkgImage = new CombinedImgFile("art\\interface\\item_creation_ui\\item_creation.img");

	if (temple::Dll::GetInstance().HasCo8Hooks() && !modSupport.IsKotB()) {
		mUseCo8Ui = true;
		if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\ITEM_CREATION_WIDENED_0_0.tga", &mItemCreationWidenedTexture00)
			|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\ITEM_CREATION_WIDENED_1_0.tga", &mItemCreationWidenedTexture10)
			|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\ITEM_CREATION_WIDENED_0_1.tga", &mItemCreationWidenedTexture01)
			|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\ITEM_CREATION_WIDENED_1_1.tga", &mItemCreationWidenedTexture11))
			throw TempleException("Unable to register Co8 textures");
	}

	if (textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\craftarms_0.tga", temple::GetPointer<int>(0x10BEE38C))
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\craftarms_1.tga", temple::GetPointer<int>(0x10BECEE8))
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\craftarms_2.tga", temple::GetPointer<int>(0x10BED988))
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\craftarms_3.tga", temple::GetPointer<int>(0x10BECEEC))
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\invslot_selected.tga", temple::GetPointer<int>(0x10BECDAC))
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\invslot.tga", temple::GetPointer<int>(0x10BEE038))
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\add_button.tga", temple::GetPointer<int>(0x10BEE334))
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\add_button_grey.tga", temple::GetPointer<int>(0x10BED990))
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\add_button_hover.tga", temple::GetPointer<int>(0x10BEE2D8))
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\add_button_press.tga", temple::GetPointer<int>(0x10BED79C))
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\down_arrow.tga", &mDownArrowTga)
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\down_arrow_click.tga", &mDownArrowClickTga)
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\down_arrow_disabled.tga", &mDownArrowDisabledTga)
		|| textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\down_arrow_hovered.tga", &mDownArrowHoveredTga)) {
		throw TempleException("Missing textures for Item Creation UI!");
	}

	/*
	Init Widgets
	*/
	UiItemCreationWidgetsInit(config.width, config.height);
	MaaWidgetsInit(config.width, config.height);

	if (!temple::GetRef<bool(__cdecl)()>(0x1014F4D0)()) //   ItemCreationStringsGet()
		throw TempleException("Unable to initialize item creation strings");


	auto& icWnd = temple::GetRef<LgcyWindow>(0x10BEE040);
	auto& maaWnd = temple::GetRef<LgcyWindow>(0x10BEDB58);

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
	mMaaWnd = temple::GetPointer<LgcyWindow>(0x10BEDB58);
	// mMaaItemsScrollbarId = temple::GetRef<int>(0x10BED8A0);
	// mMaaApplicableEffectsScrollbarId = temple::GetRef<int>(0x10BECD78);

	mItemCreationWnd = temple::GetPointer<LgcyWindow>(0x10BEE040);
	// mItemCreationScrollbarId = temple::GetRef<int>(0x10BED9F4);
	// mMaaItemsScrollbarY = temple::GetRef<int>(0x10BECDA4);

}
UiItemCreation::~UiItemCreation() {
	auto shutdown = temple::GetPointer<void()>(0x10150eb0);
	shutdown();
}
void UiItemCreation::ResizeViewport(const UiResizeArgs& resizeArg) {
	auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10154e90);
	resize(&resizeArg);
}
void UiItemCreation::Reset() {
	auto reset = temple::GetPointer<void()>(0x1014f170);
	reset();
}
const std::string &UiItemCreation::GetName() const {
	static std::string name("ItemCreation-UI");
	return name;
}

int UiItemCreation::CraftedWandSpellLevel(objHndl objHndItem)
{
	SpellStoreData wandSpell;
	if (!CraftedWandSpellGet(objHndItem, wandSpell))
		wandSpell.spellLevel = -1;
	return wandSpell.spellLevel;
}

int UiItemCreation::CraftedWandCasterLevel(objHndl item)
{
	// int spellLvl = CraftedWandSpellLevel(item);
	SpellStoreData wandSpell;
	if (!CraftedWandSpellGet(item, wandSpell))
		wandSpell.spellLevel = -1;
	if (wandSpell.spellLevel <= 1)
		return 1;

	auto spellLvl = (int)wandSpell.spellLevel;
	auto castingClass = spellSys.GetCastingClass(wandSpell.classCode);
	auto minCasterLvl = (int)d20ClassSys.GetMinCasterLevelForSpellLevel(castingClass, spellLvl);
	if ((minCasterLvl % 2) == 0) {
		minCasterLvl--; // because toee encodes spell level rather than caster level...
	}
	auto casterLvl = (spellLvl * 2) - 1; // TODO get this right for rangers/paladins... bleh

	if (minCasterLvl >= 1 && casterLvl < minCasterLvl ){
		return minCasterLvl;
	}
	return casterLvl;
}

bool UiItemCreation::CreateItemResourceCheck(objHndl crafter, objHndl objHndItem, int spellEnum){
	bool canCraft = 1;
	bool xpCheck = 0;
	auto insuffXp = itemCreationAddresses.craftInsufficientXP;
	auto insuffCp = itemCreationAddresses.craftInsufficientFunds;
	auto insuffSkill = itemCreationAddresses.craftSkillReqNotMet;
	auto insuffPrereqs = itemCreationAddresses.insuffPrereqs;
	auto surplusXP = d20LevelSys.GetSurplusXp(crafter);
	uint32_t craftingCostCP = 0;
	auto partyMoney = party.GetMoney();

	

	*insuffXp = 0;
	*insuffCp = 0;
	*insuffSkill = 0;
	*insuffPrereqs = 0;

	// Check GP Section
	int itemWorth = 0;
	if (itemCreationType == ItemCreationType::ScribeScroll){
		itemWorth = ScribedScrollWorth(spellEnum, ScribedScrollCasterLevel(spellEnum));
		craftingCostCP = itemWorth / 2;
	}
	else {
		auto itemObj = objSystem->GetObject(objHndItem);
		itemWorth = itemObj->GetInt32(obj_f_item_worth);

		// MAA
		if (itemCreationType == ItemCreationType::CraftMagicArmsAndArmor) {
			craftingCostCP = MaaCpCost(CRAFT_EFFECT_INVALID);
		}
		// Wands
		else if (itemCreationType == ItemCreationType::CraftWand) {
			itemWorth = CraftedWandWorth(objHndItem, CraftedWandCasterLevel(objHndItem)); //ItemWorthAdjustedForCasterLevel(objHndItem, CraftedWandCasterLevel(objHndItem));
			craftingCostCP = itemWorth / 2;
		}
		// Potions etc
		else {
			// current method for crafting stuff:
			craftingCostCP = itemWorth / 2;
		};
	}
	
	if ( ( (uint32_t)partyMoney ) < craftingCostCP){
		*insuffCp = 1;
		canCraft = 0;
	};


	// Check XP & prerequisites section

	// Scrolls, Wands and Potions:
	if (itemCreationType == ItemCreationType::ScribeScroll) {
		if (!ScribeScrollCheck(crafter, spellEnum)) {
			*insuffPrereqs = 1;
			canCraft = 0;
		}
		
		// check XP
		int itemXPCost = itemWorth / 2500;
		xpCheck = surplusXP >= itemXPCost;
	}
	else if (itemCreationType == CraftMagicArmsAndArmor) {
		int magicArmsAndArmorXPCost = MaaXpCost(CRAFT_EFFECT_INVALID);
		xpCheck = surplusXP >= magicArmsAndArmorXPCost;
	}
	else {
		// check requirements from rules\\item_creation.mes
		if (!ItemCreationParseMesfileEntry(crafter, objHndItem)){
			*insuffPrereqs = 1;
			canCraft = 0;
		}

		// check XP
		int itemXPCost = itemWorth / 2500;
		xpCheck = surplusXP >= itemXPCost;
	} 
		
	if (xpCheck){
		return canCraft;
	} else
	{
		*insuffXp = 1;
		return 0;
	};

}

bool UiItemCreation::ScribeScrollCheck(objHndl crafter, int spEnum)
{
	std::vector<int> spellClasses, spellLevels;
	if (!spellSys.SpellKnownQueryGetData(crafter, spEnum, spellClasses, spellLevels))
		return false;

	for (auto i = 0u; i < spellLevels.size(); i++) {
		auto maxSpellLvl = -1;
		auto spClass = spellClasses[i];
		if (spellSys.isDomainSpell(spClass)) {
			maxSpellLvl = spellSys.GetMaxSpellLevel(crafter, stat_level_cleric);
		}
		else {
			maxSpellLvl = spellSys.GetMaxSpellLevel(crafter, spellSys.GetCastingClass(spClass));
		}


		if (maxSpellLvl >= spellLevels[i]) {
			return true;
		}
	}
	return false;
}

const char* UiItemCreation::GetItemCreationMesLine(int lineId){
	MesLine line;
	line.key = lineId;

	mesFuncs.GetLine_Safe(mItemCreationMes, &line);
	return line.value;
}

char const* UiItemCreation::ItemCreationGetItemName(objHndl itemHandle) const
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
	const char * result = nullptr;
	if (mesFuncs.GetLine(itemCreationNames, &line)){
		result= line.value;
	}
	else {
		result = description.getDisplayName(itemHandle, itemHandle);
	}

	if (!result){
		int dummy = 1;
	}
	return result;
		
}

objHndl UiItemCreation::MaaGetItemHandle(){
	if (craftingItemIdx < 0 || (uint32_t) craftingItemIdx >= mMaaCraftableItemList.size()) {
		return objHndl::null;
	}
	return mMaaCraftableItemList[craftingItemIdx];
}

bool UiItemCreation::IsWeaponBonus(int effIdx)
{
	if (effIdx < 0)
		return false;

	if ( (itemEnhSpecs[effIdx].flags & IESF_ENH_BONUS) 
		&& (itemEnhSpecs[effIdx].flags & IESF_WEAPON)){
		return true;
	}
	return false;
}

bool UiItemCreation::IsOutmoded(int effIdx){

	auto itEnh = &itemEnhSpecs[effIdx];
	while ( itEnh->upgradesTo != CRAFT_EFFECT_INVALID){
		for (auto it: appliedBonusIndices){
			if (it == itEnh->upgradesTo)
				return true;

			//Removes the start effect after it is added (otherwise it displays the greyed start effect and the second effect)
			if ((it == effIdx) && (itEnh->downgradesTo == CRAFT_EFFECT_INVALID))
				return true;
		}
		itEnh = &itemEnhSpecs[itEnh->upgradesTo];
	}
	return false;
}

bool UiItemCreation::MaaEffectIsApplicable(int effIdx){

	auto& itEnh = itemEnhSpecs[effIdx];
	if (!(itEnh.flags & IESF_ENABLED) || ( (itEnh.flags & IESF_NONCORE) && !config.nonCoreMaterials))
		return false;

	if (itEnh.flags & IESF_INCREMENTAL){
		// not the root and hasn't got the prerequisite effect level
		if (itEnh.downgradesTo != CRAFT_EFFECT_INVALID && !HasNecessaryEffects(effIdx))
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

				if (itEnh.flags & IESF_TWO_HANDED) {
					auto wieldType = inventory.GetWieldType(mItemCreationCrafter, itemHandle);
					if (wieldType != 2) {
						return false;
					}
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

					if (itEnh.flags & IESF_LIGHT_ONLY) {
						if (armorType != ARMOR_TYPE_LIGHT) {
							return false;
						}
					}
				}
			}
		}
	}


	return true;
}

int UiItemCreation::GetEffIdxFromWidgetIdx(int widIdx){

	// auto scrollbar2Y = temple::GetRef<int>(0x10BECDA8);
	auto adjIdx = mMaaApplicableEffectsScrollbarY + widIdx; // this is the overall index for the effect
	auto validCount = 0;
	for (auto idx : itemEnhIdxSorted) {  //Using the sorted effect list so effects will display in sorted order
		if (MaaEffectIsApplicable(idx)) {
			auto effect = itemEnhSpecs[idx];
			if (!(effect.flags & IESF_ENH_BONUS)) {
				if (validCount == adjIdx) {
					return idx;
				}
				validCount++;
			}
		}
	}

	return CRAFT_EFFECT_INVALID;
}

int UiItemCreation::GetEffIdxFromWidgetId(int widId){
	auto idx = 0;
	for (idx = 0; idx < MAA_EFFECT_BUTTONS_COUNT; idx++) {
		if (maaBtnIds[idx] == widId)
			break;
	}
	if (idx >= MAA_EFFECT_BUTTONS_COUNT)
		return CRAFT_EFFECT_INVALID;

	
	return GetEffIdxFromWidgetIdx(idx);
}

int UiItemCreation::HasNecessaryEffects(int effIdx){

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

int UiItemCreation::MaaGetCurEnhBonus(){

	auto curEnhBon = 0;
	for (auto it : appliedBonusIndices) {
		auto& itEnhSpec = itemEnhSpecs[it];
		if ( (itEnhSpec.flags & IESF_ENH_BONUS) && itEnhSpec.data.enhBonus > curEnhBon)
			curEnhBon = itEnhSpec.data.enhBonus;
	}

	return curEnhBon;
}

int UiItemCreation::MaaGetEffIdxForEnhBonus(int enhBon, objHndl itemHandle){

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
	
std::string UiItemCreation::GetEffectDescription(objHndl item)
{
	std::string res;

	auto obj = gameSystems->GetObj().GetObject(item);
	auto condArray = obj->GetInt32Array(obj_f_item_pad_wielder_condition_array);
	for (auto i = 0u; i < condArray.GetSize(); i++) {
		const auto condID = condArray[i];
		for (auto& itemEnh : itemEnhSpecs) {
			if (condID == itemEnh.second.condId) {

				//Some upgradables have the same condition, in that case check the bonuse also
				if (itemEnh.second.flags & (IESF_ENH_BONUS | IESF_INCREMENTAL)) {
					const auto bonus = inventory.GetItemWieldCondArg(item, condID, 0);
					if (bonus != itemEnh.second.data.enhBonus) {
						continue;
					}
				}

				//Add Enhancement to the +X for weapons, armor and shields
				std::string effName = GetItemCreationMesLine(1000 + itemEnh.first);
				if (itemEnh.second.flags & IESF_ENH_BONUS ) {
					MesLine mesLine;
					mesLine.key = 147;
					bonusSys.GetBonusMesLine(mesLine);
					std::string mesValue(mesLine.value);

					//Clip off the link that does not slow correctly in the popup
					if (!mesValue.empty() && mesValue[0] == '~') {
						mesValue = mesValue.substr(1, mesValue.size() - 1);
						const auto clipIdx = mesValue.find('~');
						mesValue = mesValue.substr(0, clipIdx);
					}
					effName += " ";
					effName += mesValue;
				}

				if (res.size() > 0) {
					res += "; ";
				}
				res += effName;
				break;
			}
		}
	}

	return res;
}
	
/* 0x1014F310 */
bool UiItemCreation::ItemWielderCondsContainEffect(int effIdx, objHndl item)
{
	if (effIdx == CRAFT_EFFECT_INVALID)
		return false;

	auto itemObj = gameSystems->GetObj().GetObject(item);


	auto &itEnh = itemEnhSpecs[effIdx];
	auto condId = itEnh.condId;
	auto condArray = itemObj->GetInt32Array(obj_f_item_pad_wielder_condition_array);

	if (condArray.GetSize() <= 0)
		return false;


	if (!IsWeaponBonus(effIdx)){  // not a +x WEAPON bonus

		for (auto i = 0u; i < condArray.GetSize(); i++){
			auto condArrayIt = condArray[i];
			if (condArrayIt  == condId)	{

				if (itemObj->type == obj_t_armor){
					const bool isArmorEnch = itEnh.flags & IESF_ARMOR;
					const bool isShieldEnch = itEnh.flags & IESF_SHIELD;

					// Ensure that shield bonuses don't get applied to normal armors and normal armor bonuses don't get applied to shields (e.g. so Armor Spell Resistance doesn't appear twice)
					if (isArmorEnch || isShieldEnch) {
						const auto armorFlags = itemObj->GetInt32(obj_f_armor_flags);
						const bool itemIsShield = inventory.GetArmorType(armorFlags) == ARMOR_TYPE_SHIELD;
						if (!itemIsShield && isShieldEnch) {
							return false;
						}
						else if (itemIsShield && isArmorEnch) {
							return false;
						}
					}
				}
				

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

void UiItemCreation::CraftScrollWandPotionSetItemSpellData(objHndl objHndItem, objHndl objHndCrafter){

	// the new and improved Wands/Scroll Property Setting Function

	auto obj = objSystem->GetObject(objHndItem);
	// auto itemCreationType = *itemCreationAddresses.itemCreationType;

	if (itemCreationType == CraftWand){
		
		auto wandSpell = obj->GetSpell(obj_f_item_spell_idx, 0);
		CraftedWandSpellGet(objHndItem, wandSpell);		
		obj->SetSpell(obj_f_item_spell_idx, 0, wandSpell);
		
		auto args = PyTuple_New(3);
			
		int casterLevelFinal = wandSpell.spellLevel * 2 - 1;
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
		auto scrollSpell = obj->GetSpell(obj_f_item_spell_idx, 0);
		ScribedScrollSpellGet(mScribedScrollSpell, scrollSpell);
		obj->SetSpell(obj_f_item_spell_idx, 0, scrollSpell);
		auto invAid = GetScrollInventoryIconId(mScribedScrollSpell);
		obj->SetInt32(obj_f_item_inv_aid, invAid);


		auto baseDescr = description.GetDescriptionString(obj->GetInt32(obj_f_description) );
		auto spellName = spellSys.GetSpellName(mScribedScrollSpell);
		int SPELL_ENUM_AID = 1;
		auto aidSpellName = std::string(spellSys.GetSpellName(SPELL_ENUM_AID));
		auto pos = std::strstr(baseDescr, aidSpellName.c_str());
		if (!pos) {
			pos = std::strstr(baseDescr, tolower(aidSpellName).c_str());
		}
		auto endPos = pos + aidSpellName.size();
		char newName[1024] = {0,};
		auto idx = 0;
		for (idx = 0; baseDescr + idx < pos; ++idx) {
			newName[idx] = baseDescr[idx];
		}
		for (auto i=0; i < strlen(spellName); ++i) {
			newName[idx+i] = spellName[i];
		}

		for (auto i = 0; endPos + i < baseDescr + strlen(baseDescr); ++i) {
			newName[idx + strlen(spellName) + i] = endPos[i];
		}
		auto newNameId = description.CustomNameNew(newName);
		obj->SetInt32(obj_f_description, newNameId);
		return;
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

		uint32_t classCodes[SPELL_ENUM_MAX_VANILLA] = { 0, };
		uint32_t spellLevels[SPELL_ENUM_MAX_VANILLA] = { 0, };
		uint32_t spellFoundNum = 0;
		int casterKnowsSpell = spellSys.spellKnownQueryGetData(objHndCrafter, spellData.spellEnum, classCodes, spellLevels, &spellFoundNum);
		if (casterKnowsSpell){
			uint32_t spellClassFinal = classCodes[0];
			uint32_t spellLevelFinal = 0;
			uint32_t isClassSpell = classCodes[0] & (0x80);

			if (isClassSpell){
				spellLevelFinal = spellSys.GetMaxSpellLevel(objHndCrafter, spellSys.GetCastingClass(classCodes[0] ), 0);
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


void UiItemCreation::CreateItemDebitXPGP(objHndl crafter, objHndl objHndItem, int spellEnum){
	uint32_t crafterXP = objects.getInt32(crafter, obj_f_critter_experience);
	uint32_t craftingCostCP = 0;
	uint32_t craftingCostXP = 0;

	if (itemCreationType == CraftMagicArmsAndArmor){ // magic arms and armor
		craftingCostCP = MaaCpCost(CRAFT_EFFECT_INVALID);
		craftingCostXP = MaaXpCost(CRAFT_EFFECT_INVALID);
	}
	else
	{
		int itemWorth;
		if (itemCreationType == ItemCreationType::CraftWand)
			itemWorth = CraftedWandWorth(objHndItem, CraftedWandCasterLevel(objHndItem));
		else if (itemCreationType == ItemCreationType::ScribeScroll){
			itemWorth = ScribedScrollWorth(spellEnum, ScribedScrollCasterLevel(spellEnum));
		}
		else
			itemWorth = objects.getInt32(objHndItem, obj_f_item_worth);
		craftingCostCP = itemWorth / 2;
		craftingCostXP = itemWorth / 2500;

	};
	party.DebitMoney(0, 0, 0, craftingCostCP);
	objects.setInt32(crafter, obj_f_critter_experience, crafterXP - craftingCostXP);
}

bool UiItemCreation::CraftedWandSpellGet(objHndl item, SpellStoreData & spellDataOut, int * spellLevelBaseOut){

	if (!item)
		return false;
	auto obj = objSystem->GetObject(item);
	if (!obj->GetSpellArray(obj_f_item_spell_idx).GetSize())
		return false;

	// get spell
	auto spellData = obj->GetSpell(obj_f_item_spell_idx, 0);
	
	// default values (shouldn't really be used...)
	int spellLevelBasic = 999;
	int spellLevelMax = -1;
	uint32_t spellLevelFinal = 0;
	auto spellClassFinal = spellData.classCode;


	// get data from caster
	uint32_t spellClassCodes[SPELL_ENUM_MAX_VANILLA] = { 0, };
	uint32_t spellLevels[SPELL_ENUM_MAX_VANILLA] = { 0, };
	uint32_t spellFoundNum = 0;
	int casterKnowsSpell = spellSys.spellKnownQueryGetData(itemCreationCrafter, spellData.spellEnum, spellClassCodes, spellLevels, &spellFoundNum);
	if (!casterKnowsSpell){
		logger->warn("CraftedWandSpellGet: Caster doesn't know spell!");
		return false;
	}

	// cycle thru known spells. Note: these can be meta-magicked to a higher level so be careful!!!
	for (auto i = 0u; i < spellFoundNum; i++){
		auto spellClassTemp = spellClassCodes[i];
		auto isDomainSpell = spellSys.isDomainSpell(spellClassTemp);
		if (isDomainSpell){
			spellClassTemp = spellSys.GetSpellClass(stat_level_cleric);
		}

		int spellLevelMaxTemp = spellSys.GetMaxSpellLevel(itemCreationCrafter, spellSys.GetCastingClass(spellClassTemp), 0);
		int spellLevelMinTemp = spellLevels[i];

		
		if (spellLevelMaxTemp > spellLevelMax){
			spellClassFinal = spellClassTemp; // take caster class from highest caster level
			spellLevelMax = spellLevelMaxTemp;
		}
		if (spellLevelMinTemp < spellLevelBasic)
			spellLevelBasic = spellLevelMinTemp;
	}

	spellData.classCode = spellClassFinal;
	spellData.spellLevel = spellLevelMax; // that's the max possible at this point


	// get Craft Wand Caster Level setting
	int wandSpellLevetSet = (int)d20Sys.d20QueryReturnData(itemCreationCrafter, DK_QUE_Craft_Wand_Spell_Level);
	int casterLevelSet = 2 * ((wandSpellLevetSet + 1) / 2) - 1; // {0,1,2} ? 1;  {3,4} ? 3 etc 
	if (casterLevelSet < 1)
		casterLevelSet = 1;

	auto slotLevelSet = 1 + (casterLevelSet - 1) / 2; // {1,2}? 1 ; {3,4} ? 2 etc
	if (spellLevelBasic == 0 && casterLevelSet <= 1)
		slotLevelSet = 0;

	// enforce max/min levels
	if (slotLevelSet > 0 && slotLevelSet <= spellLevelMax && slotLevelSet >= spellLevelBasic)
		spellData.spellLevel = slotLevelSet;
	else if (slotLevelSet  > spellLevelMax)
		spellData.spellLevel = spellLevelMax;
	else if (slotLevelSet < spellLevelBasic)
		spellData.spellLevel = spellLevelBasic;
	else if (spellLevelBasic == 0)
		spellData.spellLevel = spellLevelBasic;
	


	spellDataOut = spellData;
	if (spellLevelBaseOut)
		*spellLevelBaseOut = spellLevelBasic;
	return true;
}


void UiItemCreation::ItemCreationCraftingCostTexts(int widgetId, objHndl objHndItem, int spellEnum){
	// prolog
	int32_t * insuffXp;
	int32_t * insuffCp;
	int32_t *insuffSkill;
	int32_t *insuffPrereq;
	uint32_t craftingCostCP;
	uint32_t craftingCostXP;
	TigRect rect(212 + 108 * mUseCo8Ui, 157, 159, 10);
	char * prereqString;

	int casterLevelNew = -1;
	auto itemWorth = 0;

	if (itemCreationType == ScribeScroll) {
		casterLevelNew = ScribedScrollCasterLevel(spellEnum);
		itemWorth = ScribedScrollWorth(spellEnum, casterLevelNew);
	}
	else if (itemCreationType == CraftWand){
		casterLevelNew = CraftedWandCasterLevel(objHndItem);
		itemWorth = CraftedWandWorth(objHndItem, casterLevelNew);
	}
	else {
		auto obj = objSystem->GetObject(objHndItem);
		itemWorth = obj->GetInt32(obj_f_item_worth);
	}
	
	

	insuffXp = itemCreationAddresses.craftInsufficientXP;
	insuffCp = itemCreationAddresses.craftInsufficientFunds;
	insuffSkill = itemCreationAddresses.craftSkillReqNotMet;
	insuffPrereq = itemCreationAddresses.insuffPrereqs;

	//auto itemWorth = ItemWorthAdjustedForCasterLevel(objHndItem, casterLevelNew);
	craftingCostCP = itemWorth / 2;
	craftingCostXP = itemWorth / 2500;
	
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
	text = format("{} @1{}", *itemCreationAddresses.itemCreationUIStringValue, itemWorth / 100);

	UiRenderer::DrawTextInWidget(widgetId, text, rect, *itemCreationAddresses.itemCreationTextStyle2);

	// Prereq: %s
	rect.x = 210 + 108 * mUseCo8Ui;
	rect.y = 200;
	rect.width = 150;
	rect.height = 105;
	if (GetItemCreationType() == ItemCreationType::ScribeScroll) {
		std::string tmp;
		
		auto prereqStr = PrintPrereqToken("S");
		auto prereqMet = spellSys.IsSpellKnown(itemCreationCrafter, spellEnum);

		auto minCasterLevel = 0;
		auto clPrereqMet = false;
		if (prereqMet) {
			SpellStoreData spellData;
			ScribedScrollSpellGet(spellEnum, spellData);
			
			auto castingClass = !spellSys.isDomainSpell(spellData.classCode) ? spellSys.GetCastingClass(spellData.classCode) : stat_level_cleric;
			minCasterLevel = d20ClassSys.GetMinCasterLevelForSpellLevel(castingClass, spellData.spellLevel);
			clPrereqMet = critterSys.GetCasterLevelForClass(itemCreationCrafter, castingClass) >= minCasterLevel;
			
		}
		
		auto clPrereqStr = PrintPrereqToken( fmt::format("C{}", minCasterLevel).c_str() );
		
		
		static auto prereqFieldLabel = temple::GetRef<char*>(0x10BED98C);
		if (*itemCreationAddresses.craftInsufficientXP
			|| *itemCreationAddresses.craftInsufficientFunds
			|| *itemCreationAddresses.craftSkillReqNotMet
			|| *itemCreationAddresses.insuffPrereqs) {
			
			//_snprintf(tmp, 2000, "@0%s @%d%s", prereqFieldLabel, prereqMet ? 1 : 2, prereqStr);
			tmp.append(fmt::format("@0{} @{:d}{}\n", prereqFieldLabel, prereqMet ? 1 : 2, prereqStr));
			if (prereqMet) {
				tmp.append(fmt::format("@0{} @{:d}{}\n", prereqFieldLabel, clPrereqMet ? 1 : 2, clPrereqStr));
			}
		}
		else {
			//_snprintf(tmp, 2000, "@0%s @3%s", prereqFieldLabel, prereqStr);
			tmp.append(fmt::format("@0{} @3{}\n", prereqFieldLabel, prereqStr));
			if (prereqMet) {
				tmp.append(fmt::format("@0{} @3{}\n", prereqFieldLabel, clPrereqStr));
			}
		}
		if (tmp.size()) {
			UiRenderer::DrawTextInWidget(widgetId, tmp, rect, *itemCreationAddresses.itemCreationTextStyle);
		}
	}
	else {
		prereqString = temple::GetRef<char* (__cdecl)(objHndl, objHndl)>(0x101525B0)(itemCreationCrafter, objHndItem);
		if (prereqString) {
			UiRenderer::DrawTextInWidget(widgetId, prereqString, rect, *itemCreationAddresses.itemCreationTextStyle);
		}
	}
	
	
	if (!*insuffPrereq &&
		(itemCreationType == ItemCreationType::CraftWand || itemCreationType == ItemCreationType::ScribeScroll))
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

BOOL UiItemCreation::ItemCreationEntryMsg(int widId, TigMsg* msg){
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
	mScribedScrollSpell = 0;
	if (itemCreationType == ItemCreationType::ScribeScroll) {
		mScribedScrollSpell = scribeScrollSpells[itemIdx];
	}
	if (CreateItemResourceCheck(itemCreationCrafter, itemHandle, mScribedScrollSpell)) {
		uiManager->SetButtonState(mItemCreationCreateBtnId, LgcyButtonState::Normal);
	}
	else {
		uiManager->SetButtonState(mItemCreationCreateBtnId, LgcyButtonState::Disabled);
	}

	return true;
}

void UiItemCreation::ItemCreationCreateBtnRender(int widId) const
{
	auto buttonState = uiManager->GetButtonState(widId);

	Render2dArgs arg;
	if (buttonState == LgcyButtonState::Down)
	{
		arg.textureId = temple::GetRef<int>(0x10BED9EC);
	}
	else if (buttonState == LgcyButtonState::Hovered)
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

void UiItemCreation::ItemCreationCancelBtnRender(int widId) const
{
	auto buttonState = uiManager->GetButtonState(widId);

	Render2dArgs arg;
	if (buttonState == LgcyButtonState::Down)
	{
		arg.textureId = temple::GetRef<int>(0x10BED6D0);
	}
	else if (buttonState == LgcyButtonState::Hovered)
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


void UiItemCreation::LoadMaaSpecs()
{

	struct MaaSpecTabEntry	{
		std::string id;
		std::string condName;
		std::string flags;
		std::string effBonus;
		std::string enhBonus;
		std::string classReq; // class req
		std::string charReqs; // Character Level, Alignment
		std::string spellReqs;
		std::string featReqs;
		std::string antecedent;
		std::string extraGold;
	};

	auto maaSpecLineParser = [this](const TabFileRecord &record)
	{
		MaaSpecTabEntry tabEntry;
		tabEntry.id = record[0].AsString();
		tabEntry.condName = record[1].AsString();
		tabEntry.flags = record[2].AsString();
		tabEntry.effBonus = record[3].AsString();
		tabEntry.enhBonus = record[4].AsString();
		tabEntry.classReq = record[5].AsString();
		tabEntry.charReqs = record[6].AsString();
		tabEntry.spellReqs = record[7].AsString();
		tabEntry.featReqs = record[8].AsString();
		tabEntry.antecedent = record[9].AsString();
		tabEntry.extraGold = record[10].AsString();
		
		auto effIdx = std::stoi(tabEntry.id);
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
		
		
		auto effBonus = std::stoi(tabEntry.effBonus);
		auto enhBonus = std::stoi(tabEntry.enhBonus);

		itemEnhSpecs[effIdx] = ItemEnhancementSpec(condName, flags, effBonus, enhBonus);

		auto &itEnh = itemEnhSpecs[effIdx];
		// get class req
		if (!tabEntry.classReq.empty())
		{
			// TODO (right now only Weapon Ki Focus uses it and it's not enabled anyway)
		}

		// get charReqs
		if (!tabEntry.charReqs.empty())
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

		// get spellReqs, Format: ('A''B''C' = A or B or C) ('D'+'E'+'F' = D and E and F
		if (!tabEntry.spellReqs.empty() && !(config.laxRules && config.disableCraftingSpellReqs))
		{
			StringTokenizer spellReqTok(tabEntry.spellReqs);
			bool andSpell = false;
			int idx = 0;
			while (spellReqTok.next())
			{
				auto& tok = spellReqTok.token();
				if (tok.type == StringTokenType::Plus) {
					if (idx > 0) {
						idx--;  //Add to the same map entry
					}
				} else  if (tok.type != StringTokenType::QuotedString)
					continue;
				auto spellEnum = spellSys.GetSpellEnum(tok.text);
				if (spellEnum){
					idx++;
					itEnh.reqs.spells[idx].push_back(spellEnum);
				}
			}
		}
		
		if (!tabEntry.featReqs.empty()) {
			StringTokenizer spellReqTok(tabEntry.featReqs);
			while (spellReqTok.next())
			{
				auto& tok = spellReqTok.token();
				if (tok.type == StringTokenType::QuotedString) {
					const auto featReq = static_cast<feat_enums>(ElfHash::Hash(tok.text));  //New feat
					itEnh.reqs.featReq.push_back(featReq);
				}
				else if (tok.type == StringTokenType::Number) {
					itEnh.reqs.featReq.push_back(static_cast<feat_enums>(tok.numberInt));  //Old Feat
				}
			}
		}

		if (!tabEntry.antecedent.empty()) {
			itEnh.downgradesTo = std::stoi(tabEntry.antecedent);
		} else {
			itEnh.downgradesTo = CRAFT_EFFECT_INVALID;
		}

		//Get Extra Gold Cost (Optional Field)
		if (tabEntry.extraGold.empty()) {
			itemExtraGold[effIdx] = 0;
		} else {
			itemExtraGold[effIdx] = std::stoi(tabEntry.extraGold);
		}

		return 0;
	};

	TabFile::ParseFile("tprules/craft_maa_specs.tab", maaSpecLineParser);
	
	for (auto i = itemEnhSpecs.begin(); i != itemEnhSpecs.end(); ++i) {
		auto downgradesTo = i->second.downgradesTo;
		if (downgradesTo != CRAFT_EFFECT_INVALID){
			itemEnhSpecs[downgradesTo].upgradesTo = i->first;
		}
	}
}

uint32_t UiItemCreation::ItemWorthAdjustedForCasterLevel(objHndl objHndItem, uint32_t casterLevelNew){
	auto obj = objSystem->GetObject(objHndItem);

	auto itemWorthBase = obj->GetInt32(obj_f_item_worth);
	if (casterLevelNew == -1){
		return itemWorthBase;
	}

	auto numItemSpells = obj->GetSpellArray(obj_f_item_spell_idx).GetSize();
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

uint32_t UiItemCreation::CraftedWandWorth(objHndl item, int casterLevelNew){

	auto baseWorth = 750;
	auto obj = objSystem->GetObject(item);

	// which spell?
	auto spellData = obj->GetSpell(obj_f_item_spell_idx, 0);

	SpellEntry spEntry(spellData.spellEnum);
	int materialCost = spEntry.costGp * 50;


	// retrieve Spell Known data
	int spellLevelBase = spellData.spellLevel; // default value
	CraftedWandSpellGet(item, spellData, &spellLevelBase);
	auto casterLevelBase = max(1, spellLevelBase * 2 - 1);
	auto casterClass = (Stat)spellSys.GetCastingClass(spellData.classCode);
	auto minCasterLevel = (int)d20ClassSys.GetMinCasterLevelForSpellLevel(casterClass, spellLevelBase);
	if (minCasterLevel >= 1) {
		casterLevelBase = minCasterLevel;
	}

	// ToEE only encodes spell level in items - Caster Level is implicitly taken as Spell Level * 2 - 1
	// So in case caster level was selected as even valued, reduce it by 1 so as to not overcharge the crafter...
	if ((casterLevelBase % 2) == 0) {
		casterLevelBase--;
	}

	// get base worth by class (default to protos.tab spec)
	auto itemWorthBaseGp = (spellLevelBase == 0) ? ((baseWorth / 2) * casterLevelBase) : (baseWorth  * casterLevelBase * spellLevelBase);
	auto itemWorthBase = itemWorthBaseGp * 100; // +materialCost * 100;



	if (casterLevelNew == -1) {
		return itemWorthBase + materialCost * 100;
	}

	if (spellLevelBase == 0 && casterLevelNew > casterLevelBase) {
		return itemWorthBase * casterLevelNew + materialCost * 100;
	}
	if (casterLevelNew > casterLevelBase) {
		return (uint32_t)((double)itemWorthBase * (double)casterLevelNew / casterLevelBase) + materialCost * 100;
	}
	return itemWorthBase + materialCost * 100;

}

bool UiItemCreation::ScribedScrollSpellGet(int spellEnum, SpellStoreData & spellDataOut, int * spellLevelBaseOut){
	
	// get spell
	SpellStoreData spellData;
	spellData.spellEnum = spellEnum;

	// default values (shouldn't really be used...)
	int spellLevelBasic = 999;
	int spellLevelMax = -1;
	uint32_t spellLevelFinal = 0;
	auto spellClassFinal = spellData.classCode;


	// get data from caster
	uint32_t spellClassCodes[SPELL_ENUM_MAX_VANILLA] = { 0, };
	uint32_t spellLevels[SPELL_ENUM_MAX_VANILLA] = { 0, };
	uint32_t spellFoundNum = 0;
	int casterKnowsSpell = spellSys.spellKnownQueryGetData(itemCreationCrafter, spellData.spellEnum, spellClassCodes, spellLevels, &spellFoundNum);
	if (!casterKnowsSpell) {
		logger->warn("ScribedScrollSpellGet: Caster doesn't know spell!");
		return false;
	}

	// cycle thru known spells. Note: these can be meta-magicked to a higher level so be careful!!!
	for (auto i = 0u; i < spellFoundNum; i++) {
		auto spellClassTemp = spellClassCodes[i];
		auto isDomainSpell = spellSys.isDomainSpell(spellClassTemp);
		if (isDomainSpell) {
			spellClassTemp = spellSys.GetSpellClass(stat_level_cleric);
		}

		int spellLevelMaxTemp = spellSys.GetMaxSpellLevel(itemCreationCrafter, spellSys.GetCastingClass(spellClassTemp), 0);
		int spellLevelMinTemp = spellLevels[i];


		if (spellLevelMaxTemp > spellLevelMax) {
			spellClassFinal = spellClassTemp; // take caster class from highest caster level
			spellLevelMax = spellLevelMaxTemp;
		}
		if (spellLevelMinTemp < spellLevelBasic)
			spellLevelBasic = spellLevelMinTemp;
	}

	spellData.classCode = spellClassFinal;
	spellData.spellLevel = spellLevelMax; // that's the max possible at this point


										  // get Craft Wand Caster Level setting
	int scribeScrollLevelSet = (int)d20Sys.d20QueryReturnData(itemCreationCrafter, DK_QUE_Scribe_Scroll_Spell_Level);
	int casterLevelSet = 2 * ((scribeScrollLevelSet + 1) / 2) - 1; // {0,1,2} ? 1;  {3,4} ? 3 etc 
	if (casterLevelSet < 1)
		casterLevelSet = 1;

	auto slotLevelSet = 1 + (casterLevelSet - 1) / 2; // {1,2}? 1 ; {3,4} ? 2 etc
	if (spellLevelBasic == 0 && casterLevelSet <= 1)
		slotLevelSet = 0;

	// enforce max/min levels
	if (slotLevelSet > 0 && slotLevelSet <= spellLevelMax && slotLevelSet >= spellLevelBasic)
		spellData.spellLevel = slotLevelSet;
	else if (slotLevelSet  > spellLevelMax)
		spellData.spellLevel = spellLevelMax;
	else if (slotLevelSet < spellLevelBasic)
		spellData.spellLevel = spellLevelBasic;
	else if (spellLevelBasic == 0)
		spellData.spellLevel = spellLevelBasic;



	spellDataOut = spellData;
	if (spellLevelBaseOut)
		*spellLevelBaseOut = spellLevelBasic;
	return true;
}

int UiItemCreation::ScribedScrollSpellLevel(int spellEnum)
{
	SpellStoreData scrollSpell;
	if (!ScribedScrollSpellGet(spellEnum, scrollSpell))
		scrollSpell.spellLevel = -1;
	return scrollSpell.spellLevel;
}

int UiItemCreation::ScribedScrollCasterLevel(int spellEnum)
{
	// int result = ScribedScrollSpellLevel(item);
	/*if (result <= 1)
		return 1;
	return (result * 2) - 1;
	*/

	SpellStoreData scrollSpell;
	if (!ScribedScrollSpellGet(spellEnum, scrollSpell))
		scrollSpell.spellLevel = -1;
	if (scrollSpell.spellLevel <= 1)
		return 1;

	auto spellLvl = (int)scrollSpell.spellLevel;
	auto castingClass = spellSys.GetCastingClass(scrollSpell.classCode);
	auto minCasterLvl = (int)d20ClassSys.GetMinCasterLevelForSpellLevel(castingClass, spellLvl);
	if ( (minCasterLvl % 2 )==0){
		minCasterLvl--; // because toee encodes spell level rather than caster level...
	}
	auto casterLvl = (spellLvl * 2) - 1; // TODO get this right for rangers/paladins... bleh

	if (minCasterLvl >= 1 && casterLvl < minCasterLvl) {
		return minCasterLvl;
	}
	return casterLvl;
}

uint32_t UiItemCreation::ScribedScrollWorth(int spellEnum, int casterLevelNew)
{
	auto baseWorth = 25;
	
	// Calculate cost
	SpellEntry spEntry(spellEnum);
	int materialCost = spEntry.costGp;

	// retrieve Spell Known data (e.g. for Bards) and caster level (as modified by user selection)
	int spellLevelBase = 0; // default value
	SpellStoreData spellData;
	ScribedScrollSpellGet(spellEnum, spellData, &spellLevelBase);
	auto casterLevelBase = max(1,spellLevelBase * 2 - 1);
	auto casterClass = (Stat)spellSys.GetCastingClass(spellData.classCode);
	auto minCasterLevel = (int)d20ClassSys.GetMinCasterLevelForSpellLevel(casterClass, spellLevelBase);
	if (minCasterLevel >= 1) {
		casterLevelBase = minCasterLevel;
	}

	// ToEE only encodes spell level in items - Caster Level is implicitly taken as Spell Level * 2 - 1
	// So in case caster level was selected as even valued, reduce it by 1 so as to not overcharge the crafter...
	if ((casterLevelBase % 2 ) == 0){
		casterLevelBase--;
	}

	// get base worth by class (default to protos.tab spec)
	auto itemWorthBaseGp = (spellLevelBase == 0) ? ((baseWorth / 2) * casterLevelBase) : (baseWorth  * casterLevelBase * spellLevelBase);
	auto itemWorthBase = itemWorthBaseGp * 100; // +materialCost * 100;
	


	if (casterLevelNew == -1) {
		return itemWorthBase + materialCost * 100;
	}

	if (spellLevelBase == 0 && casterLevelNew > casterLevelBase) {
		return itemWorthBase * casterLevelNew + materialCost * 100;
	}
	if (casterLevelNew > casterLevelBase) {
		return (uint32_t)((double)itemWorthBase * (double)casterLevelNew / casterLevelBase) + materialCost * 100;
	}
	return itemWorthBase + materialCost * 100;
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
    result = objects.description.getDisplayName(protoHandle);
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


UiItemCreation::UiItemCreation(){

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
	mScribedScrollSpell = 0;
}

int UiItemCreation::GetItemCreationType(){
	return mItemCreationType;
}

int UiItemCreation::GetSurplusXp(objHndl crafter){
	auto level = critterSys.GetEffectiveLevel(crafter);
	auto xpReq = d20LevelSys.GetXpRequireForLevel(level);
	return gameSystems->GetObj().GetObject(crafter)->GetInt32(obj_f_critter_experience) - xpReq;
}

bool UiItemCreation::ItemWielderCondsHasAntecedent(int effIdx, objHndl item){
	auto &itEnh = itemEnhSpecs[effIdx];
	if (itEnh.downgradesTo != CRAFT_EFFECT_INVALID){
		if (ItemWielderCondsContainEffect(itEnh.downgradesTo, item))
			return true;
		else 
			return ItemWielderCondsHasAntecedent(itEnh.downgradesTo, item);
	}
	return false;
}

// Originally 0x10152280
bool UiItemCreation::ItemCreationParseMesfileEntry(objHndl crafter, objHndl item){
	
	auto itemObj = objSystem->GetObject(item);
	auto protoId = 0;
	if (itemObj->IsProto()) {
		protoId = itemObj->id.GetPrototypeId();
	}
	else
		protoId = itemObj->protoId.GetPrototypeId();

	auto line = GetItemCreationRulesMesLine(protoId);
	if (!line)
		return true;

	StringTokenizer tok(line);
	while (tok.next()){
		
		if (tok.token().type != StringTokenType::Identifier && tok.token().type != StringTokenType::QuotedString)
			continue;
		if (!ItemCreationRulesParseReqText(crafter, tok.token().text))
			return false;

	}
	
	return true;
}

const char * UiItemCreation::GetItemCreationRulesMesLine(int key){

	auto mesHnd = temple::GetRef<MesHandle>(0x10BEDA90);
	MesLine line(key);
	mesFuncs.GetLine(mesHnd, &line);
	return line.value;
}

// Originally 0x101505B0
bool UiItemCreation::ItemCreationRulesParseReqText(objHndl crafter, const char * reqTxt){

	if (!reqTxt)
		return true;

	auto firstChar = *reqTxt;
	firstChar = toupper(firstChar);

	// alignment
	if (firstChar == 'A'){
		if (config.laxRules && config.disableAlignmentRestrictions){
			return true;
		}

		auto algn = objects.StatLevelGet(crafter, stat_alignment);
		if (!_stricmp(reqTxt+1, "good")){
			return (algn & ALIGNMENT_GOOD) != 0;
		}
		if (!_stricmp(reqTxt + 1, "evil")) {
			return (algn & ALIGNMENT_EVIL) != 0;
		}
		if (!_stricmp(reqTxt + 1, "lawful")) {
			return (algn & ALIGNMENT_LAWFUL) != 0;
		}
		if (!_stricmp(reqTxt + 1, "chaotic")) {
			return (algn & ALIGNMENT_CHAOTIC) != 0;
		}
		return true;
	}

	// caster level
	if (firstChar == 'C'){
		auto clReq = atol(reqTxt + 1);
		return critterSys.GetCasterLevel(crafter) >= clReq;
	}

	// feat
	if (firstChar == 'F'){
		auto getFeatEnumByName = temple::GetRef<feat_enums(__cdecl)(const char*)>(0x1007BB50);
		return feats.HasFeatCountByClass(crafter, getFeatEnumByName(reqTxt + 1));
	}

	// Race
	if (firstChar == 'R'){
		auto raceEnum = d20RaceSys.GetRaceEnum(reqTxt + 1);
		return objects.StatLevelGet(crafter, stat_race) == raceEnum;
	}

	// Spell
	if (firstChar == 'S'){
		if (config.laxRules && config.disableCraftingSpellReqs){
			if (GetItemCreationType() == ItemCreationType::CraftWondrous)
				return true;
		}
		auto spEnum = spellSys.GetSpellEnum(reqTxt + 1);
		if (!spEnum)
			spEnum = atol(reqTxt + 1);
		return spellSys.IsSpellKnown(crafter, spEnum);
	}

	// OR condition
	if (firstChar == 'O'){
		StringTokenizer tok(reqTxt + 1);
		while (tok.next()){

			if (tok.token().type != StringTokenType::Identifier 
				&& tok.token().type != StringTokenType::QuotedString)
				continue;

			if (ItemCreationRulesParseReqText(crafter, tok.token().text))
				return true;
		}
		return false;
	}
	
	

	return false;
}

std::string UiItemCreation::PrintPrereqToken(const char * reqTxt)
{
	std::string result = fmt::format("");

	auto firstChar = toupper(*reqTxt);
	MesLine mesLine;
	switch (firstChar){
	case 'A':
		if (!_stricmp(reqTxt + 1, "good")) {
			mesLine.key = 20100;
		}
		else if (!_stricmp(reqTxt + 1, "lawful")) {
			mesLine.key = 20101;
		}
		if (!_stricmp(reqTxt + 1, "evil")) {
			mesLine.key = 20102;
		}
		else  { // should be chaotic...
			mesLine.key = 20103;
		}
		mesFuncs.GetLine_Safe(mItemCreationMes, &mesLine);
		result = fmt::format("{}", mesLine.value);
		break;
	case 'C': // level
		mesLine.key = 20000 + min(20l, atol(reqTxt+1) );
		mesFuncs.GetLine_Safe(mItemCreationMes, &mesLine);
		result = fmt::format("{}", mesLine.value);
		break;
	case 'F':
		result = fmt::format("{}", feats.GetFeatName(temple::GetRef<feat_enums(__cdecl)(const char*)>(0x1007BB50)(reqTxt + 1)));
		break;
	case 'R':
		result = fmt::format("{}", d20Stats.GetRaceName(d20RaceSys.GetRaceEnum(reqTxt + 1)) );
		break;
	case 'S':
	{
		if (itemCreationType == ItemCreationType::ScribeScroll) {
			result = spellSys.GetSpellName(mScribedScrollSpell);
			return result;
		}
		result = fmt::format("{}", spellSys.GetSpellName(spellSys.GetSpellEnum(reqTxt + 1)));
		break;
	}
	case 'O':
		{
			StringTokenizer tok(reqTxt + 1);
			auto isFirst = true;
			while (tok.next()) {
				if (!isFirst){
					result.append(", ");
				}
				if (tok.token().type != StringTokenType::Identifier
					&& tok.token().type != StringTokenType::QuotedString)
					continue;

				auto tmp = PrintPrereqToken(tok.token().text);
				result.append(tmp);
				isFirst = false;
			}
		}
		break;
	default:
		result = fmt::format("null");
		break;
	}
	return result;
}

int UiItemCreation::GetScrollInventoryIconId(int spellEnum)
{
	SpellEntry entry(mScribedScrollSpell);
	if (!entry.spellEnum) {
		return 0;
	}
	
	switch ((SpellSchools)entry.spellSchoolEnum) {
	case SpellSchools::School_Abjuration:
		return 384;
	case SpellSchools::School_Conjuration:
		return 379;
	case SpellSchools::School_Divination:
		return 386;
	case SpellSchools::School_Enchantment:
		return 383;
	case SpellSchools::School_Evocation:
		return 154;
	case SpellSchools::School_Illusion:
		return 381;
	case SpellSchools::School_Necromancy:
		return 380;
	case SpellSchools::School_Transmutation:
		return 382;
	default:
		return 154;
	}
}


ItemEnhancementSpec::ItemEnhancementSpec(const std::string &condName, uint32_t Flags, int EffcBonus, int enhBonus)
	:condName(condName),flags(Flags),effectiveBonus(EffcBonus){
	data.enhBonus = enhBonus;
	condId = ElfHash::Hash(condName);
	downgradesTo = upgradesTo = CRAFT_EFFECT_INVALID;
}

BOOL UiItemCreation::IsActive(){
	return itemCreationType != ItemCreationType::Inactive;
}

BOOL UiItemCreation::ItemCreationShow(objHndl crafter, ItemCreationType icType){

	if (crafter == objHndl::null){
		uiManager->SetHidden(mItemCreationWndId, true);
		uiManager->SetHidden(mMaaWndId, true);
		return FALSE;
	}

	if (icType == itemCreationType)
		return TRUE;

	if (itemCreationType <= ItemCreationType::ForgeRing && itemCreationType >= ItemCreationType::IC_Alchemy )
	{
		if (itemCreationResourceCheckResults)
			free(itemCreationResourceCheckResults);
		uiManager->SetHidden(mItemCreationWndId, true);
		*mItemCreationWnd = *uiManager->GetWindow(mItemCreationWndId);
	} 
	else if (itemCreationType == ItemCreationType::CraftMagicArmsAndArmor)	{
		uiManager->SetHidden(mMaaWndId, true);
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

BOOL UiItemCreation::ItemCreationWndMsg(int widId, TigMsg * msg){

	if (msg->type == TigMsgType::MOUSE) {
		auto _msg = (TigMsgMouse*)msg;
		if (_msg->buttonStateFlags & MSF_SCROLLWHEEL_CHANGE) {
			auto newMsg = *(TigMsgMouse*)msg;
			newMsg.buttonStateFlags = MSF_SCROLLWHEEL_CHANGE;
			*mItemCreationScrollbar = *uiManager->GetScrollBar(mItemCreationScrollbarId);
			mItemCreationScrollbar->HandleMessage((TigMsg&)newMsg);
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
			uiManager->ScrollbarGetY(mItemCreationScrollbarId, &mItemCreationScrollbarY);
		}
		return true;
	}

	return false;
}

void UiItemCreation::ItemCreationWndRender(int widId){
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


	auto shortname = uiAssets->GetStatShortName(stat_experience);
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
		objHndl itemHandle = craftedItemHandles[itemCreationType][craftingItemIdx];
		const char* itemName = nullptr;
		auto invAid = 0;
		if (itemCreationType == ItemCreationType::ScribeScroll) {
			itemName = spellSys.GetSpellName(mScribedScrollSpell);
			invAid = GetScrollInventoryIconId(mScribedScrollSpell);
		}
		else {
			itemName = ItemCreationGetItemName(itemHandle);
			invAid = gameSystems->GetObj().GetObject(itemHandle)->GetInt32(obj_f_item_inv_aid);
		}
		
		// draw icon
		int textureId;
		uiAssets->GetAsset(UiAssetType::Inventory, (UiGenericAsset)invAid, textureId);
		rect = TigRect(temple::GetRef<TigRect>(0x102FAEC4));
		rect.x += 108 * mUseCo8Ui;
		UiRenderer::DrawTexture(textureId, rect);


		
		measText = UiRenderer::MeasureTextSize(itemName, temple::GetRef<TigTextStyle>(0x10BED938));
		if (measText.width > 161) {
			measText.width = 161;
		}
		rect = TigRect((161 - measText.width )/2 + 208 + 108 * mUseCo8Ui, 132, 161, 24);
		UiRenderer::DrawTextInWidget(widId, itemName, rect, temple::GetRef<TigTextStyle>(0x10BEDFE8));
		
		ItemCreationCraftingCostTexts(widId, itemHandle, mScribedScrollSpell);
	}
	

	UiRenderer::PopFont();
}


void UiItemCreation::ItemCreationEntryRender(int widId){
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


	// special case for scrolls
	const char* text = nullptr;
	if (itemCreationType == ItemCreationType::ScribeScroll) {
		auto spellEnum = scribeScrollSpells[itemIdx];
		text = spellSys.GetSpellName(spellEnum);
	}
	else {
		auto itemHandle = craftedItemHandles[itemCreationType][itemIdx];
		text = ItemCreationGetItemName(itemHandle);
	}

	TigRect rect(32, 12 * widIdx + 55, 155 + mUseCo8Ui * 108, 12);
	auto checkRes = itemCreationResourceCheckResults[itemIdx];
	if (itemIdx == craftingItemIdx)	{
		if (checkRes)
			UiRenderer::DrawTextInWidget(mItemCreationWndId, text, rect, temple::GetRef<TigTextStyle>(0x10BEDFE8));
		else
			UiRenderer::DrawTextInWidget(mItemCreationWndId, text, rect, temple::GetRef<TigTextStyle>(0x10BECE90));
	} 
	else {
		if (checkRes)
			UiRenderer::DrawTextInWidget(mItemCreationWndId, text, rect, temple::GetRef<TigTextStyle>(0x10BED938));
		else
			UiRenderer::DrawTextInWidget(mItemCreationWndId, text, rect, temple::GetRef<TigTextStyle>(0x10BED6D8));
	}

	UiRenderer::PopFont();
}

void UiItemCreation::MaaWndRender(int widId){

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


	auto shortname = uiAssets->GetStatShortName(stat_experience);
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

void UiItemCreation::MaaItemRender(int widId){
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
	uiAssets->GetAsset(UiAssetType::Inventory, invAid, textureId);
	srcRect = TigRect(0, 0, 64, 64);
	UiRenderer::DrawTexture(textureId, rect, srcRect);

}

void UiItemCreation::MaaAppliedBtnRender(int widId){
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

void UiItemCreation::MaaEnhBonusDnRender(int widId){

	objHndl itemHandle = MaaGetItemHandle();
	if (!itemHandle)
		return;

	int texId;
	auto bs = uiManager->GetButtonState(widId);
	switch (bs) {
	case Disabled:
		texId = mDownArrowDisabledTga;
		break;
	case LgcyButtonState::Down:
		texId = mDownArrowClickTga;
		break;
	case LgcyButtonState::Hovered:
		texId = mDownArrowHoveredTga;
		break;
	case LgcyButtonState::Normal:
	default:
		texId = mDownArrowTga;
	}
	static TigRect srcRect(0,0,19,11);

	UiRenderer::DrawTextureInWidget(mMaaWndId, texId, mEnhBonusDnRect, srcRect);
}

void UiItemCreation::MaaEnhBonusUpRender(int widId){

	objHndl itemHandle = MaaGetItemHandle();
	if (!itemHandle)
		return;

	int texId;
	auto bs = uiManager->GetButtonState(widId);
	switch (bs) {
	case Disabled:
		texId = mDownArrowDisabledTga;
		break;
	case Down:
		texId = mDownArrowClickTga;
		break;
	case Hovered:
		texId = mDownArrowHoveredTga;
		break;
	case Normal:
	default:
		texId = mDownArrowTga;
	}
	static TigRect srcRect(0, 0, 19, 11);
	TigRect rect = mEnhBonusDnRect;
	rect.x -= 15;
	UiRenderer::DrawTextureInWidget(mMaaWndId,texId, rect, srcRect, 0x20);
}

void UiItemCreation::ButtonStateInit(int wndId){
	uiManager->SetHidden(wndId, false);
	*mItemCreationWnd = *uiManager->GetWindow(wndId);

	auto itemCount = numItemsCrafting[itemCreationType];

	itemCreationResourceCheckResults = new bool[itemCount];
	if (itemCreationType == ItemCreationType::ScribeScroll) {
		for (int i = 0u; i < itemCount; ++i) {
			auto spellEnum = scribeScrollSpells[i];
			itemCreationResourceCheckResults[i] = CreateItemResourceCheck(itemCreationCrafter, objHndl::null, spellEnum);
		}
	}
	else {
		for (int i = 0u; i < itemCount; i++)
		{
			auto itemHandle = craftedItemHandles[itemCreationType][i];
			if (itemHandle) {
				itemCreationResourceCheckResults[i] = CreateItemResourceCheck(itemCreationCrafter, itemHandle);
			}
		}
	}
	
	mScribedScrollSpell = 0;
	if (craftingItemIdx >= 0 && craftingItemIdx < itemCount){
		if (itemCreationType == ItemCreationType::ScribeScroll) {
			mScribedScrollSpell = scribeScrollSpells[craftingItemIdx];
		}
		if (CreateItemResourceCheck(itemCreationCrafter, craftedItemHandles[itemCreationType][craftingItemIdx], mScribedScrollSpell))
			uiManager->SetButtonState(mItemCreationCreateBtnId, LgcyButtonState::Normal);
		else
			uiManager->SetButtonState(mItemCreationCreateBtnId, LgcyButtonState::Disabled);
	}

	uiManager->ScrollbarSetYmax(mItemCreationScrollbarId, itemCount - NUM_DISPLAYED_CRAFTABLE_ITEMS_MAX  < 0 ? 0 : itemCount - NUM_DISPLAYED_CRAFTABLE_ITEMS_MAX);
	uiManager->ScrollbarSetY(mItemCreationScrollbarId, 0);
	mItemCreationScrollbarY = 0;
	uiManager->BringToFront(wndId);
}

void UiItemCreation::MaaInitCraftedItem(objHndl itemHandle){

	craftedItemName.clear();
	craftedItemExistingEffectiveBonus = 0;
	craftedItemExtraGold = 0;
	appliedBonusIndices.clear();
	if (!itemHandle){
		return;
	}

	craftedItemName.append(ItemCreationGetItemName(itemHandle));
	craftedItemNamePos = craftedItemName.size();

	for (auto it : itemEnhSpecs) {
		if (ItemWielderCondsContainEffect(it.first, itemHandle)) {

			if (it.second.flags & ItemEnhancementSpecFlags::IESF_ENH_BONUS) {
				craftedItemExistingEffectiveBonus += it.second.effectiveBonus;
				craftedItemExtraGold += itemExtraGold[it.first];
				appliedBonusIndices.push_back(it.first);
				/*if (!ItemWielderCondsContainEffect(it.second.upgradesTo, itemHandle)) {

				}*/
			}
			else if (!ItemWielderCondsContainEffect(it.second.upgradesTo, itemHandle)) {
				craftedItemExistingEffectiveBonus += it.second.effectiveBonus;
				craftedItemExtraGold += itemExtraGold[it.first];
				appliedBonusIndices.push_back(it.first);
			}
		}
	}
	

}

void UiItemCreation::MaaInitCrafter(objHndl crafter){
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

void UiItemCreation::MaaInitWnd(int wndId){
	maaSelectedEffIdx = -1;
	mMaaActiveAppliedWidIdx = -1;
	objHndl itemHandle = objHndl::null;
	if (craftingItemIdx >= 0 && mCraftingItemIdx < (int)mMaaCraftableItemList.size()) {
		itemHandle = mMaaCraftableItemList[craftingItemIdx];
	}

	MaaInitCraftedItem(itemHandle);
	uiManager->SetHidden(wndId, false);
	*mMaaWnd = *uiManager->GetWindow(wndId);
	uiManager->BringToFront(wndId);
	// auto scrollbarId =  mMaaCraftableItemsScrollbarId;// temple::GetRef<int>(0x10BED8A0);
	uiManager->ScrollbarSetYmax(mMaaItemsScrollbarId, mMaaCraftableItemList.size() < 5 ? 0 : mMaaCraftableItemList.size() - 5);
	uiManager->ScrollbarSetY(mMaaItemsScrollbarId, 0);
	mMaaItemsScrollbarY = 0;

	auto numApplicableEffects = 0;
	for (auto it : itemEnhSpecs) {
		if (MaaEffectIsApplicable(it.first)) {
			numApplicableEffects++;
		}
	}

	uiManager->ScrollbarSetYmax(mMaaApplicableEffectsScrollbarId, numApplicableEffects < MAA_EFFECT_BUTTONS_COUNT ? 0 : numApplicableEffects - MAA_EFFECT_BUTTONS_COUNT);
	uiManager->ScrollbarSetY(mMaaApplicableEffectsScrollbarId, 0);

	mMaaApplicableEffectsScrollbarY = 0;
	craftingItemIdx = -1;

	if (!mMaaCraftableItemList.size()) {
		auto title = combatSys.GetCombatMesLine(6009);
		auto helpId = ElfHash::Hash("TAG_CRAFT_MAGIC_ARMS_ARMOR_POPUP");
		auto popupType0 = temple::GetRef<int(__cdecl)(int, int(__cdecl*)(), const char*)>(0x100E6F10);
		popupType0(helpId, []() { return itemCreation().ItemCreationShow(objHndl::null, ItemCreationType::Inactive); }, title);
		itemCreationType = ItemCreationType::Inactive;  //Necessary to set when the dialog is forced closed or it will still think it is opened 
	}
	craftedItemNamePos = craftedItemName.size();
}

BOOL UiItemCreation::CreateBtnMsg(int widId, TigMsg* msg)
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
				
			if ( CreateItemResourceCheck(itemCreationCrafter, itemHandle, mScribedScrollSpell) )
			{
				CreateItemDebitXPGP(itemCreationCrafter, itemHandle, mScribedScrollSpell);
				CreateItemFinalize(itemCreationCrafter, itemHandle);
			}
		}
	}
	return false;
}

bool UiItemCreation::MaaShouldJustModifyArg(int effIdx, objHndl item){

	auto &itEnh = itemEnhSpecs[effIdx];
	if (!(itEnh.flags & (IESF_ENH_BONUS | IESF_INCREMENTAL)))
		return false;
		
	return ItemWielderCondsHasAntecedent(effIdx, item);
}

void UiItemCreation::MaaCreateBtnRender(int widId) const
{
	auto buttonState = uiManager->GetButtonState(widId);
	
	Render2dArgs arg;
	if (buttonState == LgcyButtonState::Down)
	{
		arg.textureId = temple::GetRef<int>(0x10BED9EC);
	} else if (buttonState == LgcyButtonState::Hovered)
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
void UiItemCreation::CreateItemFinalize(objHndl crafter, objHndl item){

	auto icType = itemCreationType;
	auto effBonus = 0;
	auto crafterObj = gameSystems->GetObj().GetObject(crafter);
	auto altPressed = infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) || infrastructure::gKeyboard.IsKeyPressed(VK_RMENU);

	//auto appliedBonusIndices = temple::GetRef<int[9]>(0x10BED908);

	if (icType == ItemCreationType::CraftMagicArmsAndArmor){

		effBonus = MaaGetTotalEffectiveBonus(CRAFT_EFFECT_INVALID);

		auto itemObj = gameSystems->GetObj().GetObject(item);
		for (auto it : appliedBonusIndices){
			auto effIdx = it;

			if (effIdx == CRAFT_EFFECT_INVALID)
				continue;

			auto& itEnh = itemEnhSpecs[effIdx];

			if (ItemWielderCondsContainEffect(effIdx, item)){
				
				/*if (itEnh.flags & IESF_ENH_BONUS) {
					effBonus += itEnh.effectiveBonus;
				}*/
				continue;
			}
			
			//effBonus += itEnh.effectiveBonus;
			


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
		if (itemObj->type == obj_t_weapon){
			itemWorthDelta *= GoldBaseWorthVsEffectiveBonus[effBonus] - GoldBaseWorthVsEffectiveBonus[craftedItemExistingEffectiveBonus];
		}
		else{
			itemWorthDelta *= GoldCraftCostVsEffectiveBonus[effBonus] - GoldCraftCostVsEffectiveBonus[craftedItemExistingEffectiveBonus];
		}

		const int extraCostDelta = 100 * (MaaGetTotalExtraCost(CRAFT_EFFECT_INVALID) - craftedItemExtraGold);
		itemWorthDelta += extraCostDelta;

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

		uiManager->SetHidden(mMaaWndId, true);
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

	auto insertResult = inventory.SetItemParent(newItemHandle, crafter, IIF_None);
	if (!insertResult){
		uiDialog->ShowTextBubble(crafter, crafter, fmt::format("My inventory is full!\nI dropped the item on the ground.") , -1);
	}

	if (itemCreationType != ItemCreationType::Inactive) {
		//infrastructure::gKeyboard.Update();
		// if ALT is pressed, keep the window open for more crafting!

		if (altPressed) {

			// refresh the resource checks
			if (itemCreationType == ItemCreationType::ScribeScroll) {
				for (int i = 0; i < numItemsCrafting[itemCreationType]; i++) {
					auto protoHandle = craftedItemHandles[itemCreationType][i];
					auto spellEnum = scribeScrollSpells[i];
					if (protoHandle) {
						itemCreationResourceCheckResults[i] = CreateItemResourceCheck(crafter, protoHandle, spellEnum);
					}
				}
			}
			else {
				for (int i = 0; i < numItemsCrafting[itemCreationType]; i++) {
					auto protoHandle = craftedItemHandles[itemCreationType][i];
					if (protoHandle)
						itemCreationResourceCheckResults[i] = CreateItemResourceCheck(crafter, protoHandle);
				}
			}
			
			if (craftingItemIdx >= 0 && craftingItemIdx < numItemsCrafting[itemCreationType]) {
				auto createBtnId = mItemCreationCreateBtnId; //temple::GetRef<int>(0x10BED8B0);
				if (CreateItemResourceCheck(crafter, item, mScribedScrollSpell)) {
					uiManager->SetButtonState(createBtnId, LgcyButtonState::Normal);
				}
				else {
					uiManager->SetButtonState(createBtnId, LgcyButtonState::Disabled);
				}
			}
			return;
		}

		// else close the window and reset everything
		free(itemCreationResourceCheckResults);
		uiManager->SetHidden(mItemCreationWndId, true);
		itemCreationType = ItemCreationType::Inactive;
		itemCreationCrafter = 0i64;
	}
}

BOOL UiItemCreation::CancelBtnMsg(int widId, TigMsg* msg) {
	if (msg->type == TigMsgType::WIDGET && (TigMsgWidgetEvent)msg->arg2 == TigMsgWidgetEvent::MouseReleased)
	{
		craftedItemNamePos = craftedItemName.size();
		craftingWidgetId = widId;

		if (itemCreationType != ItemCreationType::Inactive) {
			if (itemCreationType <= ItemCreationType::ForgeRing){
				free(itemCreationResourceCheckResults);
				uiManager->SetHidden(mItemCreationWndId, true);
				*mItemCreationWnd = *uiManager->GetWindow(mItemCreationWndId);
			}
			else if (itemCreationType == ItemCreationType::CraftMagicArmsAndArmor)
			{
				uiManager->SetHidden(mMaaWndId, true);
				*mMaaWnd = *uiManager->GetWindow(mItemCreationWndId);
			}

			itemCreationType = ItemCreationType::Inactive;
		}
		return true;
	}
	return false;
}

void UiItemCreation::MaaCancelBtnRender(int widId) const
{
	auto buttonState = uiManager->GetButtonState(widId);

	Render2dArgs arg;
	if (buttonState == LgcyButtonState::Down)
	{
		arg.textureId = temple::GetRef<int>(0x10BED6D0);
	}
	else if (buttonState == LgcyButtonState::Hovered)
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

bool UiItemCreation::MaaCrafterMeetsReqs(int effIdx, objHndl crafter)
{
	if (!crafter)
		return false;
	if (effIdx == CRAFT_EFFECT_INVALID)
		return false;

	auto& itEnh = itemEnhSpecs[effIdx];
	if (objects.StatLevelGet(crafter, stat_level) < (int)itEnh.reqs.minLevel)
		return false;

	if (itEnh.reqs.alignment){
		if (!(objects.StatLevelGet(crafter,stat_alignment) & itEnh.reqs.alignment))
		return false;
	}
	if (itEnh.reqs.featReq.size() > 0) {
		for (auto& feat : itEnh.reqs.featReq) {
			if (!feats.HasFeatCount(crafter, feat)) {
				return false;
			}
		}
	}
		
	if (itEnh.reqs.spells.size() > 0){
		auto hasReq = false;
		for (auto it : itEnh.reqs.spells) {
			auto spellsFound = true;
			for (auto it2 : it.second)
			{
				if (!spellSys.spellKnownQueryGetData(crafter, it2, nullptr, nullptr, nullptr)) {
					spellsFound = false;
					break;
				}
			}
			if (spellsFound) {
				hasReq = true;
				break;
			}
		}
		if (!hasReq && !(config.laxRules && config.disableCraftingSpellReqs))
			return false;
	}
	

	return true;
}

bool UiItemCreation::MaaEffectIsInAppliedList(int effIdx){
	for (auto it: appliedBonusIndices){
		if (it == effIdx)
			return true;
	}
	return false;
}

BOOL UiItemCreation::MaaWndMsg(int widId, TigMsg * msg)
{
	if (msg->type == TigMsgType::MOUSE) {

		auto _msg = (TigMsgMouse*)msg;
		if (_msg->buttonStateFlags & MSF_SCROLLWHEEL_CHANGE) {
			auto widg = uiManager->GetScrollBar(mMaaApplicableEffectsScrollbarId);
			auto newMsg = *(TigMsgMouse*)msg;
			newMsg.buttonStateFlags = MSF_SCROLLWHEEL_CHANGE;
			widg->HandleMessage((TigMsg&)newMsg);
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
			uiManager->ScrollbarGetY(mMaaItemsScrollbarId, &mMaaItemsScrollbarY);
			uiManager->ScrollbarGetY(mMaaApplicableEffectsScrollbarId, &mMaaApplicableEffectsScrollbarY);
		}
		return true;
	}
	
	return false;
}

BOOL UiItemCreation::MaaTextboxMsg(int widId, TigMsg* msg){
	auto _msg = (TigMsgWidget*)msg;
	if (msg->type != TigMsgType::WIDGET || _msg->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return false;

	craftedItemNamePos = craftedItemName.size();
	craftingWidgetId = widId;

	return false;
}

bool UiItemCreation::MaaWndRenderText(int widId, objHndl item){
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

	// draw GP cost
	auto cpCost = MaaCpCost(CRAFT_EFFECT_INVALID);
	if (insuffXp || insuffGp || insuffSkill || insuffPrereqs){
		text.append(fmt::format("{} @{}{}", enhCostLabel, insuffGp+1, cpCost/100));
	} 
	else{
		text.append(fmt::format("{} @3{}", enhCostLabel, cpCost / 100));
	}
	auto& textStyle = temple::GetRef<TigTextStyle>(0x10BEE338);
	UiRenderer::DrawTextInWidget(widId, text, rect, textStyle);

	// draw XP cost
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

	// draw Item Worth
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

BOOL UiItemCreation::MaaItemMsg(int widId, TigMsg* msg){

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

BOOL UiItemCreation::MaaEffectMsg(int widId, TigMsg* msg){

	if (msg->type != TigMsgType::WIDGET || ((TigMsgWidget*)msg)->widgetEventType != TigMsgWidgetEvent::MouseReleased)
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

	/*int surplusXp = GetSurplusXp(itemCreationCrafter);
	if (MaaXpCost(effIdx) > surplusXp)
		return true;

	auto cpCost = MaaCpCost(effIdx);
	if (cpCost > party.GetMoney())
		return true;*/

	if (HasNecessaryEffects(effIdx))
		maaSelectedEffIdx = effIdx;

	return true;
}

void UiItemCreation::MaaEffectRender(int widId){

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

int UiItemCreation::MaaEffectTooltip(int x, int y, int * widId){

	if (craftingItemIdx == -1)
		return 0;

	LgcyButton * btn = uiManager->GetButton(*widId);
	if (btn->buttonState == Down || btn->buttonState == Disabled)
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
	//std::string text(fmt::format("{}", tooltips.GetTooltipString(6049))); // Requirements:
	std::string text(fmt::format("Requirements:")); // Requirements:
	
	if (itEnh.reqs.minLevel) {
		text.append(fmt::format("\n{} {}", uiAssets->GetStatMesLine(273), itEnh.reqs.minLevel )); // Caster Level
	}
	if (itEnh.reqs.alignment) {
		if (itEnh.reqs.alignment & ALIGNMENT_GOOD)
			text.append(fmt::format("\n{} {}", uiAssets->GetStatMesLine(238), uiAssets->GetStatMesLine(8017)));
		else if (itEnh.reqs.alignment & ALIGNMENT_EVIL)
			text.append(fmt::format("\n{} {}", uiAssets->GetStatMesLine(238), uiAssets->GetStatMesLine(8011)));

		if (itEnh.reqs.alignment & ALIGNMENT_LAWFUL)
			text.append(fmt::format("\n{} {}", uiAssets->GetStatMesLine(238), uiAssets->GetStatMesLine(8022)));
		else if (itEnh.reqs.alignment & ALIGNMENT_CHAOTIC)
			text.append(fmt::format("\n{} {}", uiAssets->GetStatMesLine(238), uiAssets->GetStatMesLine(8004)));
	}

	if (itEnh.reqs.featReq.size() > 0) {
		text += "\n";
		text += GetItemCreationMesLine(20105);
		bool first = true;
		for (auto feat : itEnh.reqs.featReq) {
			if (!first) {
				text += ",";
			}
			text += feats.GetFeatName(feat);
			first = false;
		}
	}
	
	// Note:  The display might need a tweek if there is every something with really complicated requirements 
	// but this should be good enough pretty much anything published
	if (itEnh.reqs.spells.size()) {
		text.append("\nSpells: ");
		auto firstReq = true;
		for (auto it : itEnh.reqs.spells) {
			auto firstSpell = true;

			if (!firstReq) {
				text.append(",  or ");
			}
			for (auto it2 : it.second) {
				if (firstSpell) {
					text.append(fmt::format("{}", spellSys.GetSpellName(it2)));
					firstSpell = false;
				}
				else {
					text.append(fmt::format(", and {}", spellSys.GetSpellName(it2)));
				}
			}
			firstReq = false;
		}
	}

	// Get the description and cut into multiple lines at about 40 characters (max 3 in practice)
	auto desc = GetItemCreationMesLine(effIdx + 2000);
	if (desc) {
		std::string descText = desc;
		while (descText.size() > 40) {
			auto nIdx = descText.find(' ', 40);  //Find the end of the word after character 40
			if (nIdx != std::string::npos) {
				std::string descText1 = descText.substr(0, nIdx);
				text.push_back('\n');
				text += descText1;
				descText = descText.substr(nIdx + 1, descText.size() - nIdx - 1);
			}
			else {
				break;  //Don't break this line there is nothing after the end of the last word
			}
		}
		if (!descText.empty()) {
			text.push_back('\n');
			text += descText;
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

void UiItemCreation::MaaEffectGetTextStyle(int effIdx, objHndl crafter, TigTextStyle* &style){
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

	if (!HasNecessaryEffects(effIdx)) {
		style = temple::GetPointer<TigTextStyle>(0x10BEE2E0);
		return;
	}


	if (effIdx == maaSelectedEffIdx){
		style = temple::GetPointer<TigTextStyle>(0x10BEDF70);
		return;
	}

	// if it's just XP/CP, let the user pick it to see the required resources
	int surplusXp = d20LevelSys.GetSurplusXp(crafter);
	auto insuffXp = MaaXpCost(effIdx) > surplusXp;
	if (insuffXp) {
		//style = temple::GetPointer<TigTextStyle>(0x10BECDB0);
		style = temple::GetPointer<TigTextStyle>(0x10BEDDF0);
		return;
	}

	auto cpCost = MaaCpCost(effIdx);
	auto insuffCp = cpCost > party.GetMoney();
	if (insuffCp) {
		//style = temple::GetPointer<TigTextStyle>(0x10BED8B8);
		style = temple::GetPointer<TigTextStyle>(0x10BEDDF0);
		return;
	}

	style = temple::GetPointer<TigTextStyle>(0x10BEDDF0);
	return;
}


BOOL UiItemCreation::MaaEffectAddMsg(int widId, TigMsg* msg)
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

	/*int surplusXp = GetSurplusXp(itemCreationCrafter);
	if (MaaXpCost(effIdx) > surplusXp)
		return true;

	auto cpCost = MaaCpCost(effIdx);
	if (cpCost > party.GetMoney())
		return true;*/

	if (HasNecessaryEffects(effIdx))
	{
		MaaAppendEnhancement(effIdx);
		CreateItemResourceCheck(itemCreationCrafter, itemHandle);
	}
	
	return true;
}

int UiItemCreation::MaaGetTotalExtraCost(int effIdx)
{
	int extraCost = 0;
	for (auto it : appliedBonusIndices) {
		if (it != CRAFT_EFFECT_INVALID)
			extraCost += itemExtraGold[it];
	}

	if (effIdx == CRAFT_EFFECT_INVALID)
		return extraCost;

	// add the bonus level of the new effect
	auto& itEnh = itemEnhSpecs[effIdx];

	if (itEnh.flags & IESF_INCREMENTAL) {
		if (itEnh.downgradesTo != CRAFT_EFFECT_INVALID) {
			const auto downgradeIdx = itEnh.downgradesTo;
			auto downgradeCost = itemExtraGold[downgradeIdx];
			extraCost -= downgradeCost;
		}
	}

	extraCost += itemExtraGold[effIdx];

	return extraCost;
}

int UiItemCreation::MaaGetTotalEffectiveBonus(int effIdx){


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

void UiItemCreation::MaaAppendEnhancement(int effIdx){

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

BOOL UiItemCreation::MaaEffectRemoveMsg(int widId, TigMsg* msg){
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

BOOL UiItemCreation::MaaAppliedBtnMsg(int widId, TigMsg* msg){
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

BOOL UiItemCreation::MaaEnhBonusUpMsg(int widId, TigMsg * msg){
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

BOOL UiItemCreation::MaaEnhBonusDnMsg(int widId, TigMsg * msg){

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

bool UiItemCreation::InitItemCreationRules(){
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
			continue;
		}
		if (i == ItemCreationType::ScribeScroll) { // special handling now
			continue;
		}

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
				[this](objHndl first, objHndl second){
				auto firstName = ItemCreationGetItemName(first);
				auto secondName = ItemCreationGetItemName(second);
				auto comRes = _stricmp(firstName, secondName);
				return comRes < 0;
			});
		}
		
	}

	// Automated scribe scroll entries:
	{
		int i = (int)ItemCreationType::ScribeScroll;
		numItemsCrafting[i] = 0;
		spellSys.DoForSpellEntries( [&](SpellEntry& entry)->void {
			auto spellEnum = entry.spellEnum;
			if (spellSys.IsNonCore(spellEnum) && !config.nonCoreMaterials) {
				return;
			}
			if (spellSys.IsMonsterSpell(spellEnum) || spellSys.IsSpellLike(spellEnum)) {
				return;
			}
			// Todo: warlock...
			if (spellEnum >= 2300 && spellEnum <= 2400) {
				return;
			}
			if (!entry.spellLvlsNum) { // some non-spells in the 700 range are like that
				return;
			}
			if (spellEnum >= 700 && spellEnum <= 802) { // special case some snowflakes grrr
				// 733 Scorching Ray, 740 Ray of Clumsiness, 775 Resonance, 794, 795, 796, 797, 798, 799 - vigor spells, scintillating sphere, etc
				switch (spellEnum) {
				case 733:
				case 740:
				case 775:
				case 794:
				case 795:
				case 796:
				case 797:
				case 798:
				case 799:
					break;
				default:
					return;
				}
			}
			scribeScrollSpells.push_back(spellEnum);
			numItemsCrafting[i]++;
		});
		std::sort(scribeScrollSpells.begin(), scribeScrollSpells.end(),
			[this](int first, int second) {
				auto firstName = spellSys.GetSpellName(first);
				auto secondName = spellSys.GetSpellName(second);
				auto comRes = _stricmp(firstName, secondName);
				return comRes < 0;
			});
		craftedItemHandles[i] = new objHndl[numItemsCrafting[i] + 1];
		memset(craftedItemHandles[i], 0, sizeof(objHndl) * (numItemsCrafting[i] + 1));

		// Use Aid scroll as prototype
		auto SCROLL_OF_AID_PROTO = 9002, SPELL_AID_ENUM = 1;
		auto scrollProtoHandle = gameSystems->GetObj().GetProtoHandle(SCROLL_OF_AID_PROTO);
		if (!scrollProtoHandle) {
			throw TempleException("InitItemCreationRules: Cannot Find aid scroll proto!");
		}
		auto &spell = objSystem->GetObject(scrollProtoHandle)->GetSpell(obj_f_item_spell_idx, 0);
		if (spell.spellEnum != SPELL_AID_ENUM) {
			throw TempleException("InitItemCreationRules: Unexpected scroll proto - expected aid spell scroll!");
		}
		// just fill it with the same handle
		for (auto j = 0u; j < numItemsCrafting[i]; ++j) {
			craftedItemHandles[i][j] = scrollProtoHandle;
		}
		
	}

	// Create a list of indexes sorted by the Mes file name
	for (auto& itemEnh : itemEnhSpecs) {
		itemEnhIdxSorted.push_back(itemEnh.first);
	}
	std::sort(itemEnhIdxSorted.begin(), itemEnhIdxSorted.end(), [&](int n1, int n2) {
		return (_strcmpi(GetItemCreationMesLine(1000 + n1), GetItemCreationMesLine(1000 + n2)) < 0); }
	);

	mesFuncs.Close(icrules);

	return true;
}

void UiItemCreation::UiItemCreationWidgetsInit(int width, int height){
	auto& wnd = temple::GetRef<LgcyWindow>(0x10BEE040);
	wnd = LgcyWindow((width - 404 - 108*mUseCo8Ui) / 2, (height - 421) / 2, 404+ 108*mUseCo8Ui, 421);
	wnd.flags = 1;
	wnd.zIndex = -1;
	wnd.render = [](int widId) { itemCreation().ItemCreationWndRender(widId); };
	wnd.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().ItemCreationWndMsg(widId, msg); };//temple::GetRef<bool(__cdecl)(int, TigMsg*)>(0x1014FC20);

	mItemCreationWndId = uiManager->AddWindow(temple::GetRef<LgcyWindow>(0x10BEE040));

	mItemCreationScrollbar->Init(185 + mUseCo8Ui * 108, 51, 259);
	mItemCreationScrollbar->scrollQuantum = 3;
	mItemCreationScrollbar->x += wnd.x;
	mItemCreationScrollbar->y += wnd.y;

	mItemCreationScrollbarId = uiManager->AddScrollBar(*mItemCreationScrollbar, mItemCreationWndId);

	auto btnY = 55;

	for (int i = 0; i < NUM_ITEM_CREATION_ENTRY_WIDGETS; i++){
		LgcyButton btn(nullptr, mItemCreationWndId, 32, btnY, 155 + 108 * mUseCo8Ui, 12);
		btn.x += wnd.x;
		btn.y += wnd.y;
		btn.render = [](int widId) {itemCreation().ItemCreationEntryRender(widId); };
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().ItemCreationEntryMsg(widId, msg); };
		mItemCreationEntryBtnIds[i] = uiManager->AddButton(btn, mItemCreationWndId);
		btnY += 12;
	}
	// create button
	{
		LgcyButton btn(nullptr, mItemCreationWndId, 81, 373, 112, 22);
		btn.x += wnd.x;
		btn.y += wnd.y;
		btn.render = [](int widId) { itemCreation().ItemCreationCreateBtnRender(widId); };
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().CreateBtnMsg(widId, msg); };
		btn.SetDefaultSounds();

		mItemCreationCreateBtnId = uiManager->AddButton(btn, mItemCreationWndId);
	}
	// cancel button
	auto &cancelBtnId = temple::GetRef<int>(0x10BEDA68);
	
	LgcyButton btn(nullptr, mItemCreationWndId, 205 + 108*mUseCo8Ui, 373, 112, 22);
	btn.x += wnd.x;
	btn.y += wnd.y;
	btn.render = [](int widId) {itemCreation().ItemCreationCancelBtnRender(widId); };
	btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().CancelBtnMsg(widId, msg); };
	btn.SetDefaultSounds();

	cancelBtnId = uiManager->AddButton(btn, mItemCreationWndId);
}

void UiItemCreation::MaaWidgetsInit(int width, int height) {
	auto& wnd = temple::GetRef<LgcyWindow>(0x10BEDB58);
	wnd = LgcyWindow((width - 504) / 2, (height - 387) / 2, 504, 387);
	wnd.flags = 1;
	wnd.render = [](int widId) {itemCreation().MaaWndRender(widId); };
	wnd.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().MaaWndMsg(widId, msg); };
	wnd.zIndex = -1;
	mMaaWndId = uiManager->AddWindow(wnd);

	// Scrollbar for the Items
	auto &maaScrollbar = temple::GetRef<LgcyScrollBar>(0x10BEDA98);
	maaScrollbar.Init(184, 51, 225);
	maaScrollbar.x += wnd.x;
	maaScrollbar.y += wnd.y;
	mMaaItemsScrollbarId = uiManager->AddScrollBar(maaScrollbar, mMaaWndId);

	// Scrollbar for the effects
	auto appEffectsScrollbar = temple::GetRef<LgcyScrollBar>(0x10BEDE90);
	appEffectsScrollbar.Init(313, 148, 128);
	appEffectsScrollbar.x += wnd.x;
	appEffectsScrollbar.y += wnd.y;
	mMaaApplicableEffectsScrollbarId = uiManager->AddScrollBar(appEffectsScrollbar, mMaaWndId);

	// Item buttons
	auto btnY = 53;
	for (int i = 0; i < MAA_NUM_ENCHANTABLE_ITEM_WIDGETS; i++) {
		LgcyButton btn(nullptr, mMaaWndId, 28, btnY, 152, 42);
		btn.x += wnd.x;	btn.y += wnd.y;
		btn.render = [](int widId) { itemCreation().MaaItemRender(widId); };
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().MaaItemMsg(widId, msg); };
		mMaaItemBtnIds[i] = uiManager->AddButton(btn, mMaaWndId);
		btnY += 42;
	}

	// applicable effect butons
	btnY = 152;
	for (int i = 0; i < MAA_EFFECT_BUTTONS_COUNT; i++) {
		LgcyButton btn(nullptr, mMaaWndId, 207, btnY, 106, 12);
		btn.x += wnd.x;	btn.y += wnd.y;
		btn.render = [](int widId) { itemCreation().MaaEffectRender(widId); };
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().MaaEffectMsg(widId, msg); };
		btn.renderTooltip = [](int x, int y, LgcyWidgetId* widId) { itemCreation().MaaEffectTooltip(x, y, widId); };
		maaBtnIds[i] = uiManager->AddButton(btn, mMaaWndId);
		btnY += 12;
	}

	// Enhancement bonus buttons
	LgcyButton enhBonusDown(nullptr, mMaaWndId, mEnhBonusDnRect);
	enhBonusDown.x += wnd.x; enhBonusDown.y += wnd.y;
	enhBonusDown.render = [](int widId) {itemCreation().MaaEnhBonusDnRender(widId); };
	enhBonusDown.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().MaaEnhBonusDnMsg(widId, msg); };
	enhBonusDown.SetDefaultSounds();
	mEnhBonusArrowDnId = uiManager->AddButton(enhBonusDown, mMaaWndId);

	LgcyButton enhBonusUp(nullptr, mMaaWndId, mEnhBonusDnRect);
	enhBonusUp.x += wnd.x - 15; enhBonusUp.y += wnd.y;
	enhBonusUp.render = [](int widId) {itemCreation().MaaEnhBonusUpRender(widId); };
	enhBonusUp.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().MaaEnhBonusUpMsg(widId, msg); };
	enhBonusUp.SetDefaultSounds();
	mEnhBonusArrowUpId = uiManager->AddButton(enhBonusUp, mMaaWndId);

	// applied effects
	btnY = 152 + 12;
	for (int i = 0; i < NUM_APPLIED_BONUSES_MAX; i++) {
		LgcyButton btn(nullptr, mMaaWndId, 355, btnY, 106, 12);
		btn.x += wnd.x;	btn.y += wnd.y;
		btn.render = [](int widId) { itemCreation().MaaAppliedBtnRender(widId); };
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().MaaAppliedBtnMsg(widId, msg); };
		mMaaAppliedBtnIds[i] = uiManager->AddButton(btn, mMaaWndId);
		btnY += 12;
	}

	// create button
	//auto createBtnId = temple::GetPointer<int>(0x10BED8B0);
	{
		LgcyButton btn(nullptr, mMaaWndId, 132, 340, 112, 22);
		btn.x += wnd.x;
		btn.y += wnd.y;
		btn.render = [](int widId) { itemCreation().MaaCreateBtnRender(widId); };
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().CreateBtnMsg(widId, msg); };
		btn.SetDefaultSounds();

		mMaaCreateBtnId= uiManager->AddButton(btn, mMaaWndId);
	}
	// cancel button
	//auto cancelBtnId = temple::GetPointer<int>(0x10BECD70);
	{
		LgcyButton btn(nullptr, mMaaWndId, 256, 340, 112, 22);
		btn.x += wnd.x;
		btn.y += wnd.y;
		btn.render = [](int widId) { itemCreation().MaaCancelBtnRender(widId); };
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().CancelBtnMsg(widId, msg); };
		btn.SetDefaultSounds();

		mMaaCancelBtnId = uiManager->AddButton(btn, mMaaWndId);
	}


	// Add Effect button
	//auto effectAddBtnId = temple::GetPointer<int>(0x10BEE394);
	{
		LgcyButton btn(nullptr, mMaaWndId, 333, 189, temple::GetRef<int>(0x102FAF5C), temple::GetRef<int>(0x102FAF60));
		btn.x += wnd.x;
		btn.y += wnd.y;
		temple::GetRef<int>(0x102FAF54) = 333 + wnd.x;
		temple::GetRef<int>(0x102FAF58) = 189 + wnd.y;
		btn.render = temple::GetRef<void(__cdecl)(int)>(0x10150020);
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().MaaEffectAddMsg(widId, msg); };
		btn.SetDefaultSounds();

		mMaaEffectAddBtnId = uiManager->AddButton(btn, mMaaWndId);
	}

	// Remove Effect button
	//auto effectRemoveBtnId = temple::GetPointer<int>(0x10BEE394);
	{
		LgcyButton btn(nullptr, mMaaWndId, 335, 220, temple::GetRef<int>(0x102FAF6C), temple::GetRef<int>(0x102FAF70));
		btn.x += wnd.x;
		btn.y += wnd.y;
		temple::GetRef<int>(0x102FAF64) = 335 + wnd.x;
		temple::GetRef<int>(0x102FAF68) = 220 + wnd.y;
		btn.render = temple::GetRef<void(__cdecl)(int)>(0x101500B0);
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().MaaEffectRemoveMsg(widId, msg); };
		btn.SetDefaultSounds();

		mMaaEffectRemoveBtnId = uiManager->AddButton(btn, mMaaWndId);
	}

	// Textbox
	//auto maaTextboxId = temple::GetPointer<int>(0x10BED9E8); // now internal member
	auto& maaTextboxRect = temple::GetRef<TigRect>(0x102FAF74);
	maaTextboxRect.x = 296; maaTextboxRect.y = 72;
	{
		LgcyButton btn(nullptr, mMaaWndId, maaTextboxRect);
		btn.x += wnd.x;
		btn.y += wnd.y;
		maaTextboxRect.x += wnd.x;
		maaTextboxRect.y += wnd.y;
		// render is handled in the main window
		btn.handleMessage = [](int widId, TigMsg* msg) { return itemCreation().MaaTextboxMsg(widId, msg); };
		btn.SetDefaultSounds();

		mMaaTextboxId = uiManager->AddButton(btn, mMaaWndId);
	}

}

void UiItemCreation::MaaWidgetsExit(int widId){
	uiManager->RemoveChildWidget(mMaaTextboxId);
	uiManager->RemoveChildWidget(mMaaEffectRemoveBtnId);
	uiManager->RemoveChildWidget(mMaaEffectAddBtnId);
	uiManager->RemoveChildWidget(mMaaCreateBtnId);
	uiManager->RemoveChildWidget(mMaaCancelBtnId);
	for (int i = 0; i < NUM_APPLIED_BONUSES_MAX; i++){
		uiManager->RemoveChildWidget(mMaaAppliedBtnIds[i]);
	}
	for (int i = 0; i < MAA_EFFECT_BUTTONS_COUNT; i++) {
		uiManager->RemoveChildWidget(maaBtnIds[i]);
	}
	for (int i = 0; i < MAA_NUM_ENCHANTABLE_ITEM_WIDGETS; i++)	{
		uiManager->RemoveChildWidget(mMaaItemBtnIds[i]);
	}

	auto wnd = uiManager->GetWindow(widId);

	uiManager->RemoveWidget(widId);

}

void UiItemCreation::ItemCreationWidgetsExit(int widId){

	uiManager->RemoveChildWidget(mItemCreationCreateBtnId); 
	uiManager->RemoveChildWidget(temple::GetRef<int>(0x10BEDA68)); // cancel button
	
	/*auto icEntryBtnIds = temple::GetRef<int[NUM_DISPLAYED_CRAFTABLE_ITEMS_MAX]>(0x10BECE28);
	for (int i = 0; i < NUM_DISPLAYED_CRAFTABLE_ITEMS_MAX; i++){
		uiManager->RemoveChildWidget(icEntryBtnIds[i]);
	}*/

	for (int i = 0; i < NUM_DISPLAYED_CRAFTABLE_ITEMS_MAX; i++) {
		uiManager->RemoveChildWidget(mItemCreationEntryBtnIds[i]);
	}
	uiManager->RemoveChildWidget(mItemCreationScrollbarId);

	auto wnd = uiManager->GetWindow(widId);
	
	uiManager->RemoveWidget(widId);
}

void UiItemCreation::UiItemCreationResize(UiResizeArgs& resizeArgs){

	auto& icWnd = temple::GetRef<LgcyWindow>(0x10BEE040);
	auto& maaWnd = temple::GetRef<LgcyWindow>(0x10BEDB58);

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

int ItemCreationHooks::HookedStatLevelGetForItemCreationPrereq(objHndl handle){
	auto result = 0;
	if (!handle)
		return result;

	result = critterSys.GetCasterLevel(handle);
	return result;
}

BOOL ItemCreationHooks::HookedIsSpellKnown(objHndl handle, int spellEnum){

	if (config.laxRules && config.disableCraftingSpellReqs){
		auto icType = uiSystems->GetItemCreation().GetItemCreationType();
		if (icType == ItemCreationType::CraftWondrous)
			return TRUE;
	}

	return temple::GetRef<BOOL(__cdecl)(objHndl, int)>(0x10075B50)(handle, spellEnum);
}

char* ItemCreationHooks::GetCraftingPrereqString(objHndl crafter, objHndl item){
	auto prereqBuffer = temple::GetRef<char[2000]>(0x10BECEF0);
	prereqBuffer[0] = 0;

	auto itemGuid = objSystem->GetPersistableId(item);
	auto protoId = itemGuid.GetPrototypeId();
	auto prereqStrRaw = itemCreation().GetItemCreationRulesMesLine(protoId);
	if (!prereqStrRaw){
		return prereqBuffer;
	}

	std::string tmp;
	StringTokenizer tok(prereqStrRaw);
	while (tok.next()){
		if (tok.token().type != StringTokenType::QuotedString && tok.token().type != StringTokenType::Identifier){
			continue;
		}

		auto prereqStr = itemCreation().PrintPrereqToken(tok.token().text);
		static auto prereqFieldLabel = temple::GetRef<char*>(0x10BED98C);
		if (*itemCreationAddresses.craftInsufficientXP 
			|| *itemCreationAddresses.craftInsufficientFunds
			|| *itemCreationAddresses.craftSkillReqNotMet
			|| *itemCreationAddresses.insuffPrereqs){
			auto prereqMet = itemCreation().ItemCreationRulesParseReqText(crafter, tok.token().text);
			//_snprintf(tmp, 2000, "@0%s @%d%s", prereqFieldLabel, prereqMet ? 1 : 2, prereqStr);
			tmp.append( fmt::format("@0{} @{:d}{}\n", prereqFieldLabel, prereqMet ? 1 : 2, prereqStr) );
		}
		else{
			//_snprintf(tmp, 2000, "@0%s @3%s", prereqFieldLabel, prereqStr);
			tmp.append( fmt::format("@0{} @3{}\n", prereqFieldLabel, prereqStr) );
		}

	}
	if (tmp.size()){
		_snprintf(prereqBuffer, 2000, "%s", tmp.c_str());
	}
		
	return prereqBuffer;
}

int UiItemCreation::MaaCpCost(int effIdx){

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

	int TotalCost = 0;
	if (gameSystems->GetObj().GetObject(itemHandle)->type == obj_t_weapon){
		TotalCost = 50 * (GoldBaseWorthVsEffectiveBonus[effBonus] - GoldBaseWorthVsEffectiveBonus[craftedItemExistingEffectiveBonus]);
	}
	else {
		TotalCost = 50 * (GoldCraftCostVsEffectiveBonus[effBonus] - GoldCraftCostVsEffectiveBonus[craftedItemExistingEffectiveBonus]);
	}

	const int extraCost = 50 * (MaaGetTotalExtraCost(effIdx) - craftedItemExtraGold);
	TotalCost += extraCost;

	return TotalCost;
}

int UiItemCreation::MaaXpCost(int effIdx){

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

	int TotalCost = 0;
	if (gameSystems->GetObj().GetObject(itemHandle)->type == obj_t_weapon) {
		TotalCost = (GoldBaseWorthVsEffectiveBonus[effBonus] - GoldBaseWorthVsEffectiveBonus[craftedItemExistingEffectiveBonus]) / 25;
	}
	else {
		TotalCost = (GoldCraftCostVsEffectiveBonus[effBonus] - GoldCraftCostVsEffectiveBonus[craftedItemExistingEffectiveBonus]) / 25;
	}

	const int extraCost = (MaaGetTotalExtraCost(effIdx) - craftedItemExtraGold) / 25;
	TotalCost += extraCost;

	return TotalCost;
}
