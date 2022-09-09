#pragma once

class CombinedImgFile;
struct LgcyWindow;
struct LgcyScrollBar;
struct TigTextStyle;
struct UiResizeArgs;
struct UiSystemConf;

#include "ui_system.h"

#define CRAFT_EFFECT_INVALID -1

enum ItemCreationType : uint32_t {
	IC_Alchemy = 0,
	BrewPotion,
	ScribeScroll,
	CraftWand,
	CraftRod,
	CraftWondrous,
	CraftStaff, // was Arms And Armor in vanilla, hacked by Moebius to expand the list
	ForgeRing,
	CraftMagicArmsAndArmor,
	Inactive // indicates that no crafting is going on
};

enum ItemEnhancementSpecFlags{
	IESF_ENABLED = 0x1,
	IESF_WEAPON = 0x2,
	IESF_ARMOR = 0x4,
	IESF_SHIELD = 0x8,
	IESF_RANGED = 0x10, // applies to ranged weapons only
	IESF_MELEE = 0x20, // applies to melee weapons only
	IESF_THROWN = 0x40, // applies to thrown weapons only
	IESF_UNK100 = 0x100, // only used in Keen
	IESF_ENH_BONUS = 0x200, // special casing for the enhancement bonus (the +X for weapons/armors)
	IESF_INCREMENTAL = 0x400, // indicates that there are multiple progressive versions of this that supercede each other
	IESF_NONCORE = 0x800, // enhancement based on non-core rules material (e.g. splatbook/fanmade), enabled only when non-core materials config is set 
	IESF_LIGHT_ONLY = 0x1000, // only for light armor 
	IESF_TWO_HANDED = 0x2000 // two handed weapon only
};

struct ItemEnhancementSpec {
	std::string condName;
	int condId;
	uint32_t flags; // see ItemEnhancementSpecFlags
	int effectiveBonus;
	union {
		int enhBonus; // for the +X item bonuses, or spell resistance
	} data;

	struct EnhReqs{
		uint32_t minLevel;
		Alignment alignment;
		Stat classReq;
		std::vector<feat_enums> featReq;
		std::map<int, std::vector<uint32_t>> spells; // each entry in the map is considered a sufficient condition
		EnhReqs(){
			minLevel = 0;
			alignment = Alignment::ALIGNMENT_NEUTRAL;
			classReq = static_cast<Stat>(-1);
		}
	} reqs;
	int upgradesTo;
	int downgradesTo;
	ItemEnhancementSpec() {
		flags = 0;
		effectiveBonus = 1;
		data.enhBonus = 0;
		condId = 0;
		downgradesTo = upgradesTo = CRAFT_EFFECT_INVALID;
	}
	ItemEnhancementSpec(const std::string &condName, uint32_t Flags, int EffcBonus, int enhBonus = 0);
};

struct TigMsg;

class UiItemCreation : public UiSystem {
	friend class D20System;
	friend class ItemCreationHooks;
public:
	static constexpr auto Name = "ItemCreation-UI";
	UiItemCreation(const UiSystemConf &config);
	~UiItemCreation();
	void Reset() override;
	void ResizeViewport(const UiResizeArgs &resizeArgs) override;
	const std::string &GetName() const override;

	BOOL IsActive();
	BOOL ItemCreationShow(objHndl crafter, ItemCreationType icType); // shows the item creation UI for the chosen IC type

	BOOL ItemCreationWndMsg(int widId, TigMsg* msg);
	
	void ItemCreationWndRender(int widId);
		void ItemCreationEntryRender(int widId);
		void ItemCreationCraftingCostTexts(int widId, objHndl objHndItem, int spellEnum);
		BOOL ItemCreationEntryMsg(int widId, TigMsg* msg);
		void ItemCreationCreateBtnRender(int widId) const;
		void ItemCreationCancelBtnRender(int widId) const;

	//MAA
	BOOL MaaWndMsg(int widId, TigMsg* msg); // message handler for the item creation window
	void MaaWndRender(int widId);
	void MaaItemRender(int widId);
	void MaaAppliedBtnRender(int widId);
	void MaaEnhBonusDnRender(int widId);
	void MaaEnhBonusUpRender(int widId);

	BOOL CreateBtnMsg(int widId, TigMsg* msg);
	bool MaaShouldJustModifyArg(int effIdx, objHndl item);
	void CreateItemFinalize(objHndl crafter, objHndl item);
	void MaaCreateBtnRender(int widId) const;
	BOOL CancelBtnMsg(int widId, TigMsg* msg);
	void MaaCancelBtnRender(int widId) const;

	

	BOOL MaaTextboxMsg(int widId, TigMsg* msg);
	bool MaaWndRenderText(int widId, objHndl item);
	BOOL MaaItemMsg(int widId, TigMsg* msg);
	BOOL MaaEffectMsg(int widId, TigMsg* msg);
	void MaaEffectRender(int widId); 
	int MaaEffectTooltip(int x, int y, int* widId);
		void MaaEffectGetTextStyle(int effIdx, objHndl crafter, TigTextStyle* & style);
	BOOL MaaEffectAddMsg(int widId, TigMsg* msg);
	int MaaGetTotalEffectiveBonus(int effIdx);
	int MaaGetTotalExtraCost(int effIdx);
	void MaaAppendEnhancement(int effIdx);
	BOOL MaaEffectRemoveMsg(int widId, TigMsg* msg);
	BOOL MaaAppliedBtnMsg(int widId, TigMsg* msg);
	BOOL MaaEnhBonusUpMsg(int widId, TigMsg* msg);
	BOOL MaaEnhBonusDnMsg(int widId, TigMsg* msg);
		


	// Widget utilities
	int GetEffIdxFromWidgetIdx(int widIdx); // gets the effect index (ID) from widget INDEX
	int GetEffIdxFromWidgetId(int widId);  // gets the effect index (ID) from widget ID


	
	int UiItemCreationInit(GameSystemConf& conf);
		bool InitItemCreationRules();


	
	void UiItemCreationWidgetsInit(int width, int height); // Item creation (scribe scroll etc.) widgets
	void MaaWidgetsInit(int width, int height);
	void MaaWidgetsExit(int widId);
	void ItemCreationWidgetsExit(int widId);
	void UiItemCreationResize(UiResizeArgs& resizeArgs);
		
	// general item creation utilities
	bool MaaCrafterMeetsReqs(int effIdx, objHndl crafter); // checks if the crafter meets the requirements specified in maa_craft_specs.tab
	bool MaaEffectIsInAppliedList(int effIdx);

	int MaaCpCost(int appliedEffectIdx);
	int MaaXpCost(int effIdx); // calculates XP cost for crafting effects in MAA
	bool CreateItemResourceCheck(objHndl crafter, objHndl item, int spellEnum = 0);
	bool ScribeScrollCheck(objHndl crafter, int spellEnum);
	const char* GetItemCreationMesLine(int lineId);
	char const* ItemCreationGetItemName(objHndl itemHandle) const;
	objHndl MaaGetItemHandle();

	void CreateItemDebitXPGP(objHndl crafter, objHndl item, int spellEnum = 0);
	bool CraftedWandSpellGet(objHndl item, SpellStoreData& spellData, int * spellLevelBase = nullptr);
	int CraftedWandSpellLevel(objHndl item);
	int CraftedWandCasterLevel(objHndl item);
	uint32_t ItemWorthAdjustedForCasterLevel(objHndl objHndItem, uint32_t casterLevelNew);
	uint32_t CraftedWandWorth(objHndl item, int casterLevelNew);
	bool ScribedScrollSpellGet(int spellEnum, SpellStoreData& spellData, int * spellLevelBase = nullptr);
	int ScribedScrollSpellLevel(int spellEnum);
	int ScribedScrollCasterLevel(int spellEnum);
	uint32_t ScribedScrollWorth(int spellEnum, int casterLevelNew);
	int GetMaxSpellLevelFromCasterLevel(int cl);

	bool IsWeaponBonus(int effIdx);
	bool IsOutmoded(int effIdx);
	bool MaaEffectIsApplicable(int effIdx);
	int HasNecessaryEffects(int effIdx);
	int MaaGetCurEnhBonus();
	int MaaGetEffIdxForEnhBonus(int enhBon, objHndl item);

	/*
	checks the obj_f_item_pad_wielder_condition_array for existence of the effect (or better/equal in the case of +x effects)
	This only returns TRUE for conditions that were already previously crafted!
	*/
	bool ItemWielderCondsContainEffect(int effIdx, objHndl item);
	
	void CraftScrollWandPotionSetItemSpellData(objHndl item, objHndl crafter);

	UiItemCreation();

	int GetItemCreationType();

	std::map<int, ItemEnhancementSpec> &GetItemEnhSpecs() { return itemEnhSpecs; }
	std::string GetEffectDescription(objHndl item);
	
protected:

	void ButtonStateInit(int wndId);
	void MaaInitCraftedItem(objHndl itemHandle);
	void MaaInitCrafter(objHndl crafter);
	void MaaInitWnd(int wndId);

	void LoadMaaSpecs();
	static int GetSurplusXp(objHndl crafter);
	bool ItemWielderCondsHasAntecedent(int effIdx, objHndl item);

	bool ItemCreationParseMesfileEntry(objHndl crafter, objHndl item);
	const char* GetItemCreationRulesMesLine(int key);
	bool ItemCreationRulesParseReqText(objHndl crafter, const char* reqTxt);
	std::string PrintPrereqToken(const char* str);

	int GetScrollInventoryIconId(int spellEnum);

	int mItemCreationType = 9;
	objHndl mItemCreationCrafter;
	int mCraftingItemIdx = -1;
	int mScribedScrollSpell = 0;


	int craftingWidgetId; // denotes which item creation widget is currently active
	bool mUseCo8Ui = false;

	LgcyWindow* mItemCreationWnd = nullptr;
		
		int mItemCreationWndId;
		int mItemCreationScrollbarId;
		LgcyScrollBar* mItemCreationScrollbar;
		int mMaaItemsScrollbarY = 0;
		CombinedImgFile* bkgImage;
		int mItemCreationEntryBtnIds[21];
		int mItemCreationScrollbarY;
		int mItemCreationCreateBtnId;

		// widened UI textures for use with Co8
		int mItemCreationWidenedTexture00;
		int mItemCreationWidenedTexture10;
		int mItemCreationWidenedTexture01;
		int mItemCreationWidenedTexture11;

		LgcyWindow* mMaaWnd = nullptr;
		int mMaaWndId;
		int mMaaItemsScrollbarId;
		int mMaaApplicableEffectsScrollbarId;
		int mMaaApplicableEffectsScrollbarY = 0;
		int mMaaSelectedEffIdx = -1;
		int mMaaItemBtnIds[5];
		int maaBtnIds[10]; // widget IDs for the craftable effects
		int mMaaAppliedBtnIds[9];
		int mMaaActiveAppliedWidIdx = -1;
		int mMaaTextboxId=-1;
		int mMaaEffectAddBtnId = -1;
		int mMaaEffectRemoveBtnId = -1;
		int mMaaCancelBtnId = -1;
		int mMaaCreateBtnId = -1;
		TigRect mCreateBtnRect ;
		TigRect mMaaCancelBtnRect;
		TigRect mMaaCraftedItemIconDestRect;
		int mEnhBonusArrowUpId, mEnhBonusArrowDnId;
		int mDownArrowTga;
		int mDownArrowClickTga;
		int mDownArrowDisabledTga;
		int mDownArrowHoveredTga;
		TigRect mEnhBonusDnRect;


	MesHandle mItemCreationMes; // tpmes\\item_creation.mes

	std::map<int, int> itemExtraGold; // Extra gold cost associated with the item
	std::map<int, ItemEnhancementSpec> itemEnhSpecs; // the idx has a reserved value of -1 for "none"
	std::vector<int> itemEnhIdxSorted;
	
	
	int& maaSelectedEffIdx = mMaaSelectedEffIdx; // currently selected craftable effect
	bool* itemCreationResourceCheckResults = nullptr; // 0x10BEE330
	
	std::vector<objHndl> mMaaCraftableItemList; // handles to enchantable inventory items
	std::vector<int> appliedBonusIndices;
	int GoldBaseWorthVsEffectiveBonus[30]; // lookup table for base worth (in GP) vs. effective enhancement level
	int GoldCraftCostVsEffectiveBonus[30]; // lookup table for craft cost (in GP) vs. effective enhancement level
	int craftedItemExistingEffectiveBonus; // stores the crafted item existing (pre-crafting) effective bonus
	int craftedItemExtraGold;  // stores the crafted item existing (pre-crafting) extra gold cost
	int& craftingItemIdx = mCraftingItemIdx; //temple::GetRef<int>(0x10BEE398);

	uint32_t numItemsCrafting[8]; // for each Item Creation type (0x11E76C7C)
	objHndl* craftedItemHandles[8]; // proto handles for new items, and item to modify for MAA (0x11E76C7C)
	std::vector<int> scribeScrollSpells; // New: special casing for automated scroll scribing

	int& itemCreationType = mItemCreationType;
	objHndl& itemCreationCrafter = mItemCreationCrafter;//temple::GetRef<objHndl>(0x10BECEE0);
	std::string craftedItemName;
	int craftedItemNamePos; // position of the text indicator ("|" character)

	

	/*
	
std::map<int, ItemEnhancementSpec> ItemCreation::itemEnhSpecs;
std::vector<int> ItemCreation::appliedBonusIndices;
int ItemCreation::GoldBaseWorthVsEffectiveBonus[30]; // lookup table for base worth (in GP) vs. effective enhancement level
int ItemCreation::GoldCraftCostVsEffectiveBonus[30]; // lookup table for craft cost (in GP) vs. effective enhancement level
int ItemCreation::craftedItemExistingEffectiveBonus; // stores the crafted item existing (pre-crafting) effective bonus
int& ItemCreation::craftingItemIdx = temple::GetRef<int>(0x10BEE398);

uint32_t ItemCreation::numItemsCrafting[8]; // for each Item Creation type
objHndl* ItemCreation::craftedItemHandles[8];
int& ItemCreation::itemCreationType = temple::GetRef<int>(0x10BEDF50);
objHndl& ItemCreation::itemCreationCrafter = temple::GetRef<objHndl>(0x10BECEE0);
char ItemCreation::craftedItemName[1024];
int ItemCreation::craftedItemNamePos;
int ItemCreation::craftingWidgetId;
	
	*/
};

//uint32_t ItemWorthAdjustedForCasterLevel(objHndl objHndItem, uint32_t slotLevelNew);
void UiItemCreationCraftingCostTexts(objHndl objHndItem);
/*
struct ButtonStateTextures {
	int normal;
	int hover;
	int pressed;
	ButtonStateTextures() : normal(-1), hover(-1), pressed(-1) {}

	void loadAccept() {
		uiManager->GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptNormal, normal);
		uiManager->GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptHover, hover);
		uiManager->GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptPressed, pressed);
	}

	void loadDecline() {
		uiManager->GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineNormal, normal);
		uiManager->GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineHover, hover);
		uiManager->GetAsset(UiAssetType::Generic, UiGenericAsset::DeclinePressed, pressed);
	}
};
*/