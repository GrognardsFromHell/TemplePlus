
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

	void apply() override {
		// auto system = UiSystem::getUiSystem("ItemCreation-UI");		
		// system->init = systemInit;
		
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

	memset(numItemsCrafting, 0, sizeof(numItemsCrafting));
	memset(craftedItemHandles, 0, sizeof(craftedItemHandles));
	craftedItemNamePos = 0;
	craftingWidgetId = -1;

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

	//auto spellData = objSystem->GetObject(objHndItem)->GetSpell(obj_f_item_spell_idx, 0);
	//uint32_t spellLevelBasic = spellData.spellLevel;
	//uint32_t spellLevelFinal = spellData.spellLevel;


	//int casterLevelSet = (int) d20Sys.d20QueryReturnData(itemCreationCrafter, DK_QUE_Craft_Wand_Spell_Level);
	//casterLevelSet = 2 * ((casterLevelSet + 1) / 2) - 1;
	//if (casterLevelSet < 1)
	//	casterLevelSet = 1;

	//auto slotLevelSet = 1 + (casterLevelSet - 1)/ 2;
	//if (spellLevelBasic == 0 && casterLevelSet <= 1)
	//	slotLevelSet = 0;
	//	
	//

	//// get data from caster - make this optional!

	//uint32_t spellClassCodes[SPELL_ENUM_MAX_VANILLA] = { 0, };
	//uint32_t spellLevels[SPELL_ENUM_MAX_VANILLA] = { 0, };
	//uint32_t spellFoundNum = 0;
	//int casterKnowsSpell = spellSys.spellKnownQueryGetData(itemCreationCrafter, spellData.spellEnum, spellClassCodes, spellLevels, &spellFoundNum);
	//if (casterKnowsSpell){
	//	uint32_t spellClassFinal = spellClassCodes[0];
	//	spellLevelBasic = spellLevels[0];
	//	spellLevelFinal = 0;
	//	auto isClassSpell = !spellSys.isDomainSpell(spellClassCodes[0]);
	//	
	//	if (isClassSpell){
	//		
	//		spellLevelFinal = spellSys.GetMaxSpellSlotLevel(itemCreationCrafter, spellSys.GetCastingClass(spellClassCodes[0]), 0);
	//	};

	//	if (spellFoundNum > 1){
	//		for (uint32_t i = 1; i < spellFoundNum; i++){
	//			if (spellLevels[i] > spellLevelFinal){
	//				spellData.classCode = spellClassCodes[i];
	//				spellLevelFinal = spellLevels[i];
	//			}
	//			if (spellLevels[i] < spellLevelBasic){
	//				spellLevelBasic = spellLevels[i];
	//			}
	//		}
	//		spellData.spellLevel = spellLevelFinal;

	//	}

	//	spellData.spellLevel = spellLevelFinal; // that's the max possible at this point
	//	if (slotLevelSet && slotLevelSet <= spellLevelFinal && slotLevelSet >= spellLevelBasic)
	//		spellData.spellLevel = slotLevelSet;
	//	else if (slotLevelSet  > spellLevelFinal)
	//		spellData.spellLevel = spellLevelFinal;
	//	else if (slotLevelSet < spellLevelBasic)
	//		spellData.spellLevel = spellLevelBasic;
	//	else if (spellLevelBasic == 0)
	//	{
	//		spellData.spellLevel = spellLevelBasic;
	//	} 

	//	spellLevelFinal = spellData.spellLevel;

	//}
	//return spellLevelFinal;
}

int UiItemCreation::CraftedWandCasterLevel(objHndl item)
{
	int result = CraftedWandSpellLevel(item);
	if (result <= 1)
		return 1;
	return (result * 2) - 1;
}

bool UiItemCreation::CreateItemResourceCheck(objHndl crafter, objHndl objHndItem){
	bool canCraft = 1;
	bool xpCheck = 0;
	auto insuffXp = itemCreationAddresses.craftInsufficientXP;
	auto insuffCp = itemCreationAddresses.craftInsufficientFunds;
	auto insuffSkill = itemCreationAddresses.craftSkillReqNotMet;
	auto insuffPrereqs = itemCreationAddresses.insuffPrereqs;
	auto surplusXP = d20LevelSys.GetSurplusXp(crafter);
	uint32_t craftingCostCP;
	auto partyMoney = party.GetMoney();

	auto itemObj = objSystem->GetObject(objHndItem);

	*insuffXp = 0;
	*insuffCp = 0;
	*insuffSkill = 0;
	*insuffPrereqs = 0;

	// Check GP Section
	int itemWorth = itemObj->GetInt32(obj_f_item_worth);

	// Scrolls
	if (itemCreationType == ItemCreationType::ScribeScroll){
		craftingCostCP = itemWorth / 2; // todo enhance with applied level etc.
	}
	// MAA
	else if (itemCreationType == ItemCreationType::CraftMagicArmsAndArmor){
		craftingCostCP = MaaCpCost( CRAFT_EFFECT_INVALID );
	}
	// Wands & Potions
	else {
		// current method for crafting stuff:
		craftingCostCP =  itemWorth / 2;

		if (itemCreationType == ItemCreationType::CraftWand){

			itemWorth = CraftedWandWorth(objHndItem, CraftedWandCasterLevel(objHndItem)); //ItemWorthAdjustedForCasterLevel(objHndItem, CraftedWandCasterLevel(objHndItem));
			craftingCostCP = itemWorth / 2;
		}
			
	};

	if ( ( (uint32_t)partyMoney ) < craftingCostCP){
		*insuffCp = 1;
		canCraft = 0;
	};


	// Check XP & prerequisites section

	// Scrolls, Wands and Potions:
	if ( itemCreationType != CraftMagicArmsAndArmor){
		// check requirements from rules\\item_creation.mes
		//if ( temple::GetRef<int(__cdecl)(objHndl, objHndl)>(0x10152280)(crafter, objHndItem) == 0){ 
		if (!ItemCreationParseMesfileEntry(crafter, objHndItem)){
			*insuffPrereqs = 1;
			canCraft = 0;
		}

		// scrolls - ensure caster can also actually cast the spell
		else if (itemCreationType == ItemCreationType::ScribeScroll )	{

			auto spData = itemObj->GetSpell(obj_f_item_spell_idx, 0); // scrolls should only have a single spell...
			auto spEnum = spData.spellEnum;

			std::vector<int> spellClasses, spellLevels;

			auto isLevelOk = false;

			if (!spellSys.SpellKnownQueryGetData(crafter, spEnum, spellClasses, spellLevels))
				*insuffPrereqs = 1;

			for (auto i=0u; i < spellLevels.size(); i++){
				auto maxSpellLvl = -1;
				auto spClass = spellClasses[i];
				if (spellSys.isDomainSpell(spClass))
				{
					maxSpellLvl = spellSys.GetMaxSpellLevel(crafter, stat_level_cleric);

				} else
				{
					maxSpellLvl = spellSys.GetMaxSpellLevel(crafter, spellSys.GetCastingClass(spClass));
				}

				
				if (maxSpellLvl >= spellLevels[i])
				{
					isLevelOk = true;
					break;
				}		
			}

			if (!isLevelOk){
				*insuffPrereqs = 1;
				canCraft = 0;
			}

		}

		// check XP
		
		int itemXPCost = itemWorth / 2500;
		xpCheck = surplusXP >= itemXPCost;
	} 
	// MAA
	else {
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
		}
		itEnh = &itemEnhSpecs[itEnh->upgradesTo];
	}
	return false;
}

bool UiItemCreation::MaaEffectIsApplicable(int effIdx){

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

int UiItemCreation::GetEffIdxFromWidgetIdx(int widIdx){

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
					// ensure that shield bonuses don't get applied to normal armors (e.g. so Armor Spell Resistance doesn't appear twice)
					auto armorFlags = itemObj->GetInt32(obj_f_armor_flags);
					if ((itEnh.flags & IESF_SHIELD) && inventory.GetArmorType(armorFlags) != ARMOR_TYPE_SHIELD )
						return false;
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


void UiItemCreation::CreateItemDebitXPGP(objHndl crafter, objHndl objHndItem){
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
			itemWorth = CraftedWandWorth(objHndItem, CraftedWandCasterLevel(objHndItem)); //ItemWorthAdjustedForCasterLevel(objHndItem, CraftedWandCasterLevel(objHndItem));
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


void UiItemCreation::ItemCreationCraftingCostTexts(int widgetId, objHndl objHndItem){
	// prolog
	int32_t * insuffXp;
	int32_t * insuffCp;
	int32_t *insuffSkill;
	int32_t *insuffPrereq;
	uint32_t craftingCostCP;
	uint32_t craftingCostXP;
	TigRect rect(212 + 108 * mUseCo8Ui, 157, 159, 10);
	char * prereqString;

	int casterLevelNew = -1; // h4x!
	auto obj = objSystem->GetObject(objHndItem);
	auto itemWorth = obj->GetInt32(obj_f_item_worth);

	if (itemCreationType == CraftWand){
		casterLevelNew = CraftedWandCasterLevel(objHndItem);
		itemWorth = CraftedWandWorth(objHndItem, casterLevelNew);
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
	prereqString = temple::GetRef<char*(__cdecl)(objHndl, objHndl)>(0x101525B0)(itemCreationCrafter, objHndItem);
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
	if (CreateItemResourceCheck(itemCreationCrafter, itemHandle)) {
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
		std::string featReqs; // TODO
		std::string antecedent;
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

		// get spellReqs
		if (!tabEntry.spellReqs.empty() && !(config.laxRules && config.disableCraftingSpellReqs))
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
		
		if (!tabEntry.antecedent.empty()) {
			itEnh.downgradesTo = std::stoi(tabEntry.antecedent);
		} else {
			itEnh.downgradesTo = CRAFT_EFFECT_INVALID;
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
	auto obj = objSystem->GetObject(item);

	// which spell?
	auto spellData = obj->GetSpell(obj_f_item_spell_idx, 0);
	int spellLevelBase = (int)spellData.spellLevel;
	// retrieve Spell Known data
	CraftedWandSpellGet(item, spellData, &spellLevelBase);

	// get base worth by class (default to protos.tab spec)
	auto itemWorthBase = obj->GetInt32(obj_f_item_worth);

	auto itemWorthBaseGp = itemWorthBase / 100;

	switch (spellLevelBase){ // cost at minimum required caster level
	case 0:
		itemWorthBaseGp = 375; break;
	case 1:
		itemWorthBaseGp = 750; break;
	case 2:
		itemWorthBaseGp = 4500; break;
	case 3:
		itemWorthBaseGp = 11250; break;
	case 4:
	default:
		itemWorthBaseGp = 21000; break;
	}
	itemWorthBase = itemWorthBaseGp * 100;
	auto casterClass = (Stat)spellSys.GetCastingClass(spellData.classCode);
	//auto minCasterLevel = (int)d20ClassSys.GetMinCasterLevelForSpellLevel(casterClass, spellLevelBase);
	
	
	switch (casterClass){
	case stat_level_cleric:
	case stat_level_wizard:
	case stat_level_druid:
		switch (spellLevelBase){
			case 0:
				itemWorthBaseGp = 375; break;
			case 1:
				itemWorthBaseGp = 750; break;
			case 2:
				itemWorthBaseGp = 4500; break;
			case 3:
				itemWorthBaseGp = 11250; break;
			case 4:
			default:
				itemWorthBaseGp = 21000; break;
		}
		break;
	case stat_level_sorcerer:
		switch (spellLevelBase) {
		case 0:
			itemWorthBaseGp = 375; break;
		case 1:
			itemWorthBaseGp = 750; break;
		case 2:
			itemWorthBaseGp = 6000; break;
		case 3:
			itemWorthBaseGp = 13500; break;
		case 4:
		default:
			itemWorthBaseGp = 24000; break;
		}
		break;
	case stat_level_bard:
		switch (spellLevelBase) {
		case 0:
			itemWorthBaseGp = 375; break;
		case 1:
			itemWorthBaseGp = 1500; break;
		case 2:
			itemWorthBaseGp = 6000; break;
		case 3:
			itemWorthBaseGp = 15750; break;
		case 4:
		default:
			itemWorthBaseGp = 30000; break;
		}
		break;
	case stat_level_paladin:
	case stat_level_ranger:
		switch (spellLevelBase) {
		case 0:
			itemWorthBaseGp = 375; break;
		case 1:
			itemWorthBaseGp = 1500; break;
		case 2:
			itemWorthBaseGp = 6000; break;
		case 3:
			itemWorthBaseGp = 11250; break;
		case 4:
		default:
			itemWorthBaseGp = 21000; break;
		}
		break;
	default: // use Wizard-like
		switch (spellLevelBase) {
		case 0:
			itemWorthBaseGp = 375; break;
		case 1:
			itemWorthBaseGp = 750; break;
		case 2:
			itemWorthBaseGp = 4500; break;
		case 3:
			itemWorthBaseGp = 11250; break;
		case 4:
		default:
			itemWorthBaseGp = 21000; break;
		}
		break;
	}


	if (casterLevelNew == -1) {
		return itemWorthBase;
	}


	auto casterLevelOld = spellLevelBase * 2 - 1;
	if (casterLevelOld < 1)
		casterLevelOld = 1;

	if (spellLevelBase == 0 && casterLevelNew > casterLevelOld) {
		return itemWorthBase * casterLevelNew;
	}
	if (casterLevelNew > casterLevelOld){
		return (uint32_t)( (double)itemWorthBase * (double)casterLevelNew / casterLevelOld );
	}
	return itemWorthBase;

}

bool UiItemCreation::ScribedScrollSpellGet(objHndl item, SpellStoreData & spellDataOut, int * spellLevelBaseOut){
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

	auto characterLvl = objects.StatLevelGet(itemCreationCrafter, stat_level);

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

}

int UiItemCreation::GetItemCreationType(){
	return mItemCreationType;
}

int UiItemCreation::GetSurplusXp(objHndl crafter){
	auto level = objects.StatLevelGet(crafter, stat_level);
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
		auto raceEnum = temple::GetRef<Race(__cdecl)(const char*)>(0x10073B70)(reqTxt + 1);
		return objects.StatLevelGet(crafter, stat_race) == raceEnum;
	}

	// Spell
	if (firstChar == 'S'){
		if (config.laxRules && config.disableCraftingSpellReqs){
			if (GetItemCreationType() == ItemCreationType::CraftWondrous)
				return true;
		}
		auto spEnum = spellSys.getSpellEnum(reqTxt + 1);
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
		auto itemHandle = craftedItemHandles[itemCreationType][craftingItemIdx];

		// draw icon
		auto invAid = (UiGenericAsset)gameSystems->GetObj().GetObject(itemHandle)->GetInt32(obj_f_item_inv_aid);
		int textureId;
		uiAssets->GetAsset(UiAssetType::Inventory, invAid, textureId);
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
			uiManager->SetButtonState(mItemCreationCreateBtnId, LgcyButtonState::Normal);
		else
			uiManager->SetButtonState(mItemCreationCreateBtnId, LgcyButtonState::Disabled);
	}

	uiManager->ScrollbarSetYmax(mItemCreationScrollbarId, numItemsCrafting[itemCreationType] - NUM_DISPLAYED_CRAFTABLE_ITEMS_MAX  < 0 ? 0 : numItemsCrafting[itemCreationType] - NUM_DISPLAYED_CRAFTABLE_ITEMS_MAX);
	uiManager->ScrollbarSetY(mItemCreationScrollbarId, 0);
	mItemCreationScrollbarY = 0;
	uiManager->BringToFront(wndId);
}

void UiItemCreation::MaaInitCraftedItem(objHndl itemHandle){

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
			
			if (it.second.flags & ItemEnhancementSpecFlags::IESF_ENH_BONUS){
				craftedItemExistingEffectiveBonus += it.second.effectiveBonus;
				appliedBonusIndices.push_back(it.first);
				/*if (!ItemWielderCondsContainEffect(it.second.upgradesTo, itemHandle)) {
					
				}*/
			}
			else if (!ItemWielderCondsContainEffect(it.second.upgradesTo, itemHandle)){
				craftedItemExistingEffectiveBonus += it.second.effectiveBonus;
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
				
			if ( CreateItemResourceCheck(itemCreationCrafter, itemHandle) )
			{
				CreateItemDebitXPGP(itemCreationCrafter, itemHandle);
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
		uiDialog.ShowTextBubble(crafter, crafter, fmt::format("My inventory is full!\nI dropped the item on the ground.") , -1);
	}

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
				auto createBtnId = mItemCreationCreateBtnId; //temple::GetRef<int>(0x10BED8B0);
				if (CreateItemResourceCheck(crafter, item))
				{
					uiManager->SetButtonState(createBtnId, LgcyButtonState::Normal);
				}
				else
				{
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
		if (!spellKnown && !(config.laxRules && config.disableCraftingSpellReqs))
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
	std::string text(fmt::format("{}", tooltips.GetTooltipString(6049))); // Requirements:
	
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
					[this](objHndl first, objHndl second){
					auto firstName = ItemCreationGetItemName(first);
					auto secondName = ItemCreationGetItemName(second);
					auto comRes = _stricmp(firstName, secondName);
					return comRes < 0;
				});
			}
			

		}
		
	}

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

	if (gameSystems->GetObj().GetObject(itemHandle)->type == obj_t_weapon){
		return 50 * (GoldBaseWorthVsEffectiveBonus[effBonus] - GoldBaseWorthVsEffectiveBonus[craftedItemExistingEffectiveBonus]);
	}
	return 50 * (GoldCraftCostVsEffectiveBonus[effBonus] - GoldCraftCostVsEffectiveBonus[craftedItemExistingEffectiveBonus]);
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

	if (gameSystems->GetObj().GetObject(itemHandle)->type == obj_t_weapon) {
		return (GoldBaseWorthVsEffectiveBonus[effBonus] - GoldBaseWorthVsEffectiveBonus[craftedItemExistingEffectiveBonus]) / 25;
	}
	return (GoldCraftCostVsEffectiveBonus[effBonus] - GoldCraftCostVsEffectiveBonus[craftedItemExistingEffectiveBonus]) / 25;
}
