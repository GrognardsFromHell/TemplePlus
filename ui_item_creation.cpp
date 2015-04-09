
#include "stdafx.h"
#include "ui.h"
#include "fixes.h"
#include "tig_mes.h"
#include "tig_font.h"
#include "temple_functions.h"
#include "tig_tokenizer.h"
#include "ui_item_creation.h"
#include "combat.h"
#include "obj.h"
#include "radial_menu.h"
#include "common.h"
#include "ui_render.h"

GlobalPrimitive<ItemCreationType, 0x10BEDF50> itemCreationType;
GlobalPrimitive<objHndl, 0x10BECEE0> globObjHndCrafter;

GlobalPrimitive<int32_t, 0x10BEE3A4> craftInsufficientXP;
GlobalPrimitive<int32_t, 0x10BEE3A8> craftInsufficientFunds;
GlobalPrimitive<int32_t, 0x10BEE3AC> craftSkillReqNotMet;
GlobalPrimitive<int32_t, 0x10BEE3B0> dword_10BEE3B0;

GlobalPrimitive<char *, 0x10BED6D4> itemCreationUIStringSkillRequired;
GlobalPrimitive<char *, 0x10BEDB50> itemCreationUIStringItemCost;
GlobalPrimitive<char *, 0x10BED8A4> itemCreationUIStringXPCost;
GlobalPrimitive<char *, 0x10BED8A8> itemCreationUIStringValue;
GlobalPrimitive<TigTextStyle, 0x10BEE338> itemCreationTextStyle; // so far used by "Item Cost: %d" and "Experience Cost: %d"
GlobalPrimitive<TigTextStyle, 0x10BED938> itemCreationTextStyle2; // so far used by "Value: %d"




static MesHandle mesItemCreationText;
static MesHandle mesItemCreationRules;
static MesHandle mesItemCreationNamesText;
static ImgFile *background = nullptr;
static ButtonStateTextures acceptBtnTextures;
static ButtonStateTextures declineBtnTextures;
static int disabledBtnTexture;



int32_t CreateItemResourceCheck(objHndl ObjHnd, objHndl ObjHndItem){
	bool canCraft = 1;
	bool xpCheck = 0;
	int32_t * globInsuffXP = craftInsufficientXP.ptr();
	int32_t * globInsuffFunds = craftInsufficientFunds.ptr();
	int32_t *globSkillReqNotMet = craftSkillReqNotMet.ptr();
	int32_t *globB0 = dword_10BEE3B0.ptr();
	uint32_t crafterLevel = objects.StatLevelGet(ObjHnd, stat_level);
	uint32_t minXPForCurrentLevel = templeFuncs.XPReqForLevel(crafterLevel); 
	uint32_t crafterXP = templeFuncs.Obj_Get_Field_32bit(ObjHnd, obj_f_critter_experience);
	uint32_t surplusXP = crafterXP - minXPForCurrentLevel;
	uint32_t craftingCostCP = 0;
	uint32_t partyMoney = templeFuncs.PartyMoney();

	*globInsuffXP = 0;
	*globInsuffFunds = 0;
	*globSkillReqNotMet = 0;
	*globB0 = 0;

	
	// Check GP Section
	if (itemCreationType == ItemCreationType(8) ){ 
		craftingCostCP = templeFuncs.ItemWorthFromEnhancements( 41 );
	}
	else
	{
		// current method for crafting stuff:
		craftingCostCP =  templeFuncs.Obj_Get_Field_32bit(ObjHndItem, obj_f_item_worth) / 2;

		// TODO: create new function
		// // craftingCostCP = CraftedItemWorthDueToAppliedLevel()
	};

	if ( ( (uint32_t)partyMoney ) < craftingCostCP){
		*globInsuffFunds = 1;
		canCraft = 0;
	};


	// Check XP section (and maybe spell prerequisites too? explore sub_10152280)
	if ( itemCreationType != CraftMagicArmsAndArmor){
		if ( templeFuncs.sub_10152280(ObjHnd, ObjHndItem) == 0){ //TODO explore function
			*globB0 = 1;
			canCraft = 0;
		};

		// TODO make XP cost calculation take applied caster level into account
		uint32_t itemXPCost = templeFuncs.Obj_Get_Field_32bit(ObjHndItem, obj_f_item_worth) / 2500; 
		xpCheck = surplusXP > itemXPCost;
	} else 
	{
		uint32_t magicArmsAndArmorXPCost = templeFuncs.CraftMagicArmsAndArmorSthg(41);
		xpCheck = surplusXP > magicArmsAndArmorXPCost;
	}
		
	if (xpCheck){
		return canCraft;
	} else
	{
		*globInsuffXP = 1;
		return 0;
	};

};

void CraftScrollWandPotionSetItemSpellData(objHndl objHndItem, objHndl objHndCrafter){

	// the new and improved Wands/Scroll Property Setting Function

	auto pytup = PyTuple_New(2);
	PyTuple_SetItem(pytup, 0, templeFuncs.PyObjFromObjHnd(objHndItem));
	PyTuple_SetItem(pytup, 1, templeFuncs.PyObjFromObjHnd(objHndCrafter));

	if (itemCreationType == CraftWand){
		// do wand specific stuff
		if (config.newFeatureTestMode)
		{
			char * wandNewName = PyString_AsString(templeFuncs.PyScript_Execute("crafting", "craft_wand_new_name", pytup));

			templeFuncs.Obj_Set_Field_32bit(objHndItem, obj_f_description, objects.description.CustomNameNew(wandNewName)); // TODO: create function that appends effect of caster level boost			
		}

		//templeFuncs.GetGlo
	};
	if (itemCreationType == ScribeScroll){
		// do scroll specific stuff
		// templeFuncs.Obj_Set_Field_32bit(objHndItem, obj_f_description, templeFuncs.CustomNameNew("Scroll of LOL"));
	};

	if (itemCreationType == BrewPotion){
		// do potion specific stuff
		// templeFuncs.Obj_Set_Field_32bit(objHndItem, obj_f_description, templeFuncs.CustomNameNew("Potion of Commotion"));
		// TODO: change it so it's 0xBAAD F00D just like spawned / mobbed potions
	};

	int numItemSpells = templeFuncs.Obj_Get_IdxField_NumItems(objHndItem, obj_f_item_spell_idx);

	// loop thru the item's spells (can be more than one in principle, like Keoghthem's Ointment)

	// Current code - change this at will...
	for (int n = 0; n < numItemSpells; n++){
		SpellStoreData spellData;
		templeFuncs.Obj_Get_IdxField_256bit(objHndItem, obj_f_item_spell_idx, n, &spellData);

		// get data from caster - make this optional!

		uint32_t classCodes[SPELLENUMMAX] = { 0, };
		uint32_t spellLevels[SPELLENUMMAX] = { 0, };
		uint32_t spellFoundNum = 0;
		int casterKnowsSpell = templeFuncs.ObjSpellKnownQueryGetData(objHndCrafter, spellData.spellEnum, classCodes, spellLevels, &spellFoundNum);
		if (casterKnowsSpell){
			uint32_t spellClassFinal = classCodes[0];
			uint32_t spellLevelFinal = 0;
			uint32_t isClassSpell = classCodes[0] & (0x80);

			if (isClassSpell){
				spellLevelFinal = templeFuncs.ObjGetMaxSpellSlotLevel(objHndCrafter, classCodes[0] & (0x7F), 0);
			};
			if (spellFoundNum > 1){
				for (uint32_t i = 1; i < spellFoundNum; i++){
					if (spellLevels[i] > spellLevelFinal){
						spellData.classCode = classCodes[i];
						spellLevelFinal = spellLevels[i];
					};
				}
				spellData.spellLevel = spellLevelFinal;

			};
			spellData.spellLevel = spellLevelFinal;
			templeFuncs.Obj_Set_IdxField_byPtr(objHndItem, obj_f_item_spell_idx, n, &spellData);

		};

	};
};


void CreateItemDebitXPGP(objHndl objHndCrafter, objHndl objHndItem){
	uint32_t crafterXP = templeFuncs.Obj_Get_Field_32bit(objHndCrafter, obj_f_critter_experience);
	uint32_t craftingCostCP = 0;
	uint32_t craftingCostXP = 0;

	if (itemCreationType == CraftMagicArmsAndArmor){ // magic arms and armor
		craftingCostCP = templeFuncs.ItemWorthFromEnhancements(41);
		craftingCostXP = templeFuncs.CraftMagicArmsAndArmorSthg(41);
	}
	else
	{
		// TODO make crafting costs take applied caster level into account
		// currently this is what ToEE does	
		craftingCostCP = templeFuncs.Obj_Get_Field_32bit(objHndItem, obj_f_item_worth) / 2;
		craftingCostXP = templeFuncs.Obj_Get_Field_32bit(objHndItem, obj_f_item_worth) / 2500;

	};

	templeFuncs.DebitPartyMoney(0, 0, 0, craftingCostCP);
	templeFuncs.Obj_Set_Field_32bit(objHndCrafter, obj_f_critter_experience, crafterXP - craftingCostXP);
};

void __cdecl UiItemCreationCraftingCostTexts(objHndl objHndItem){
	// prolog
	int32_t widgetId;
	int32_t * globInsuffXP;
	int32_t * globInsuffFunds;
	int32_t *globSkillReqNotMet;
	int32_t *globB0;
	uint32_t craftingCostCP;
	uint32_t craftingCostXP;
	TigRect rect;
	char * prereqString;
	__asm{
		mov widgetId, ebx; // widgetId is passed in ebx
	};


	uint32_t slowLevelNew = -1; // h4x!
	if (itemCreationType == CraftWand){
		// do wand specific stuff
		// slowLevelNew = 5; // jus for testing!
	};
	

	rect.x = 212;
	rect.y = 157;
	rect.width = 159;
	rect.height = 10;

	globInsuffXP = craftInsufficientXP.ptr();
	globInsuffFunds = craftInsufficientFunds.ptr();
	globSkillReqNotMet = craftSkillReqNotMet.ptr();
	globB0 = dword_10BEE3B0.ptr();

	//old method
	/* 
	craftingCostCP = templeFuncs.Obj_Get_Field_32bit(objHndItem, obj_f_item_worth) / 2;
	craftingCostXP = templeFuncs.Obj_Get_Field_32bit(objHndItem, obj_f_item_worth) / 2500;
	*/

	craftingCostCP = ItemWorthAdjustedForCasterLevel(objHndItem, slowLevelNew) / 2;
	craftingCostXP = ItemWorthAdjustedForCasterLevel(objHndItem, slowLevelNew) / 2500;
	
	string text;
	// "Item Cost: %d"
	if (*globInsuffXP || *globInsuffFunds || *globSkillReqNotMet || *globB0){
		
		text = format("{} @{}{}", *itemCreationUIStringItemCost.ptr(), *(globInsuffFunds)+1, craftingCostCP / 100);
	} else {
		//_snprintf(text, 128, "%s @3%d", itemCreationUIStringItemCost.ptr(), craftingCostCP / 100);
		text = format("{} @3{}", *itemCreationUIStringItemCost.ptr(), craftingCostCP / 100);
	};


	UiRenderer::DrawTextInWidget(widgetId, text, rect, itemCreationTextStyle);
	rect.y += 11;



	// "Experience Cost: %d"  (or "Skill Req: " for alchemy - WIP)

	if (itemCreationType == Alchemy){ 
		// placeholder - they do similar bullshit in the code :P but I guess it can be modified easily enough!
		if (*globInsuffXP || *globInsuffFunds || *globSkillReqNotMet || *globB0){
			text = format("{} @{}{}", *itemCreationUIStringSkillRequired.ptr(), *globSkillReqNotMet + 1, craftingCostXP);
		}
		else {
			text = format("{} @3{}", *itemCreationUIStringSkillRequired.ptr(), craftingCostXP);
		};
	}
	else
	{
		if (*globInsuffXP || *globInsuffFunds || *globSkillReqNotMet || *globB0){
			text = format("{} @{}{}", *itemCreationUIStringXPCost.ptr(), *(globInsuffXP) + 1, craftingCostXP);
		}
		else {
			text = format("{} @3{}", *itemCreationUIStringXPCost.ptr(), craftingCostXP);
		};
	};

	UiRenderer::DrawTextInWidget(widgetId, text, rect, itemCreationTextStyle);
	rect.y += 11;

	// "Value: %d"
	//_snprintf(text, 128, "%s @1%d", * (itemCreationUIStringValue.ptr() ), templeFuncs.Obj_Get_Field_32bit(objHndItem, obj_f_item_worth) / 100);
	text = format("{} @1{}", *itemCreationUIStringValue.ptr(), ItemWorthAdjustedForCasterLevel(objHndItem, slowLevelNew) / 100);

	UiRenderer::DrawTextInWidget(widgetId, text, rect, itemCreationTextStyle2);

	// Prereq: %s
	rect.x = 210;
	rect.y = 200;
	rect.width = 150;
	rect.height = 105;
	prereqString = templeFuncs.ItemCreationPrereqSthg_sub_101525B0(globObjHndCrafter, objHndItem);
	if (prereqString){
		UiRenderer::DrawTextInWidget(widgetId, prereqString, rect, itemCreationTextStyle);
	}
	

	
};


uint32_t ItemWorthAdjustedForCasterLevel(objHndl objHndItem, uint32_t slotLevelNew){
	uint32_t numItemSpells = templeFuncs.Obj_Get_IdxField_NumItems(objHndItem, obj_f_item_spell_idx);
	uint32_t itemWorthBase = templeFuncs.Obj_Get_Field_32bit(objHndItem, obj_f_item_worth);
	if (slotLevelNew == -1){
		return itemWorthBase;
	}

	uint32_t itemSlotLevelBase = 0;

	// loop thru the item's spells (can be more than one in principle, like Keoghthem's Ointment)

	for (uint32_t n = 0; n < numItemSpells; n++){
		SpellStoreData spellData;
		templeFuncs.Obj_Get_IdxField_256bit(objHndItem, obj_f_item_spell_idx, n, &spellData);
		if (spellData.spellLevel > itemSlotLevelBase){
			itemSlotLevelBase = spellData.spellLevel;
		}
	};

	if (itemSlotLevelBase == 0 && slotLevelNew > itemSlotLevelBase){
		return itemWorthBase * slotLevelNew * 2;
	}
	else if (slotLevelNew > itemSlotLevelBase)
	{
		return itemWorthBase * slotLevelNew / itemSlotLevelBase;
	};
	return itemWorthBase;

	
}

static vector<uint64_t> craftingProtoHandles[8];

const char *getProtoName(uint64_t protoHandle) {
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
    result = objects.description.GetDisplayName((objHndl)protoHandle, (objHndl)protoHandle);
  return result;
  */

	return objects.description.GetDisplayName(protoHandle, protoHandle);
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
			auto handle = templeFuncs.GetProtoHandle(tokenizer.token().numberInt);
			protoHandles.push_back(handle);
		}

		// Sort by prototype name
		sort(protoHandles.begin(), protoHandles.end(), [](uint64_t a, uint64_t b)
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
	if (IsCombatActive()) { return 0; }
	MesLine mesLine;
	RadialMenuStruct radmenu;
	mesLine.key = combatMesLine;
	mesFuncs.GetLine_Safe(*combat.combatMesfileIdx, &mesLine);
	RadialMenuStructInit(&radmenu);
	radmenu.field0 = (void*)mesLine.value;
	radmenu.field20 = 37;
	radmenu.field24 = itemCreationType;
	radmenu.field40 = templeFuncs.StringHash(helpSystemString);
	radialFuncs.RadialMenuCreateEntry(args.objHndCaller,  &radmenu , radialFuncs.RadialMenuArgMap_sub_100F12B0(3) ) ;

	return 0;
};

#pragma region ItemCreation Radial Menu Dispatcher Callbacks

uint32_t BrewPotionRadialMenu(DispatcherCallbackArgs args)
{
	return ItemCreationBuildRadialMenuEntry(args, BrewPotion, "TAG_BREW_POTION", 5066);
};

uint32_t ScribeScrollRadialMenu(DispatcherCallbackArgs args)
{
	return ItemCreationBuildRadialMenuEntry(args, ScribeScroll, "TAG_SCRIBE_SCROLL", 5067);
};

uint32_t CraftWandRadialMenu(DispatcherCallbackArgs args)
{
	return ItemCreationBuildRadialMenuEntry(args, CraftWand, "TAG_CRAFT_WAND", 5068);
};

uint32_t CraftRodRadialMenu(DispatcherCallbackArgs args)
{
	return ItemCreationBuildRadialMenuEntry(args, CraftRod, "TAG_CRAFT_ROD", 5069);
};

uint32_t CraftWondrousRadialMenu(DispatcherCallbackArgs args)
{
	return ItemCreationBuildRadialMenuEntry(args, CraftWondrous, "TAG_CRAFT_WONDROUS", 5070);
};

uint32_t CraftStaffRadialMenu(DispatcherCallbackArgs args)
{
	return ItemCreationBuildRadialMenuEntry(args, CraftStaff, "TAG_CRAFT_STAFF", 5103);
};

uint32_t ForgeRingRadialMenu(DispatcherCallbackArgs args)
{
	return ItemCreationBuildRadialMenuEntry(args, ForgeRing, "TAG_FORGE_RING", 5104);
};

uint32_t CraftMagicArmsAndArmorRadialMenu(DispatcherCallbackArgs args)
{
	return ItemCreationBuildRadialMenuEntry(args, CraftMagicArmsAndArmor, "TAG_CRAFT_MAA", 5071);
};

#pragma endregion

static int __cdecl systemInit(const GameSystemConf *conf) {

	mesFuncs.Open("mes\\item_creation.mes", &mesItemCreationText);
	mesFuncs.Open("mes\\item_creation_names.mes", &mesItemCreationNamesText);
	mesFuncs.Open("rules\\item_creation.mes", &mesItemCreationRules);
	loadProtoIds(mesItemCreationRules);

	acceptBtnTextures.loadAccept();
	declineBtnTextures.loadDecline();
	ui.GetAsset(UiAssetType::Generic, UiGenericAsset::DisabledNormal, disabledBtnTexture);

	background = ui.LoadImg("art\\interface\\item_creation_ui\\item_creation.img");

	// TODO !sub_10150F00("rules\\item_creation.mes")

	/*
	tig_texture_register("art\\interface\\item_creation_ui\\craftarms_0.tga", &dword_10BEE38C)
    || tig_texture_register("art\\interface\\item_creation_ui\\craftarms_1.tga", &dword_10BECEE8)
    || tig_texture_register("art\\interface\\item_creation_ui\\craftarms_2.tga", &dword_10BED988)
    || tig_texture_register("art\\interface\\item_creation_ui\\craftarms_3.tga", &dword_10BECEEC)
    || tig_texture_register("art\\interface\\item_creation_ui\\invslot_selected.tga", &dword_10BECDAC)
    || tig_texture_register("art\\interface\\item_creation_ui\\invslot.tga", &dword_10BEE038)
    || tig_texture_register("art\\interface\\item_creation_ui\\add_button.tga", &dword_10BEE334)
    || tig_texture_register("art\\interface\\item_creation_ui\\add_button_grey.tga", &dword_10BED990)
    || tig_texture_register("art\\interface\\item_creation_ui\\add_button_hover.tga", &dword_10BEE2D8)
    || tig_texture_register("art\\interface\\item_creation_ui\\add_button_press.tga", &dword_10BED79C) )
	*/

	return 0;
}

static void __cdecl systemReset() {
}

static void __cdecl systemExit() {
}

class ItemCreation : public TempleFix {
public:
	const char* name() override {
		return "Item Creation UI";
	}
	void apply() override {
		// auto system = UiSystem::getUiSystem("ItemCreation-UI");		
		// system->init = systemInit;
		void* ptrToBrewPotion = &BrewPotionRadialMenu;
		void* ptrToScribeScroll = &ScribeScrollRadialMenu;
		void* ptrToCraftWand = &CraftWandRadialMenu;
		void* ptrToCraftRod = &CraftRodRadialMenu;

		void* ptrToCraftWondrous = &CraftWondrousRadialMenu;
		void* ptrToCraftStaff = &CraftStaffRadialMenu;
		void* ptrToForgeRing = &ForgeRingRadialMenu;
		void* ptrToMAA = &CraftMagicArmsAndArmorRadialMenu;

		replaceFunction(0x10150DA0, CraftScrollWandPotionSetItemSpellData);
		replaceFunction(0x10152690, CreateItemResourceCheck);
		replaceFunction(0x10151F60, CreateItemDebitXPGP);
		replaceFunction(0x10152930, UiItemCreationCraftingCostTexts);


		write(0x102EE250, &ptrToBrewPotion, sizeof(ptrToBrewPotion));
		
		write(0x102EE280, &ptrToScribeScroll, sizeof(ptrToScribeScroll));
		write(0x102EE2B0, &ptrToCraftWand, sizeof(ptrToCraftWand));
		write(0x102EE2E0, &ptrToCraftRod, sizeof(ptrToCraftRod));

		write(0x102EE310, &ptrToCraftWondrous, sizeof(ptrToCraftWondrous));
		if (config.newFeatureTestMode)
		{
			char * testbuf[100];
			read(0x102EE310, testbuf, 4);
			logger->info("New Feature Test Mode: Testing Item Creation Function Replacement");
		}

		write(0x102AAE28, &ptrToCraftStaff, sizeof(ptrToCraftStaff));
		write(0x102AADF8, &ptrToForgeRing, sizeof(ptrToForgeRing));
		write(0x102EE340, &ptrToMAA, sizeof(ptrToMAA));
	}
} itemCreation;
