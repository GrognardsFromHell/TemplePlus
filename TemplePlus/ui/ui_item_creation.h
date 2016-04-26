#pragma once

struct WidgetType1;
struct TigTextStyle;
struct UiResizeArgs;

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

enum ItemEnhancementSpecFlags
{
	IESF_ENABLED = 0x1,
	IESF_WEAPON = 0x2,
	IESF_ARMOR = 0x4,
	IESF_SHIELD = 0x8,
	IESF_RANGED = 0x10, // applies to ranged weapons only
	IESF_MELEE = 0x20, // applies to melee weapons only
	IESF_THROWN = 0x40, // applies to thrown weapons only
	IESF_UNK100 = 0x100, // only used in Keen
	IESF_PLUS_BONUS = 0x200 // e.g. a +1 weapon/armor bonus
};

struct ItemEnhancementSpec {
	std::string condName;
	uint32_t flags; // see ItemEnhancementSpecFlags
	int effectiveBonus;
	union {
		int enhBonus; // for the +X item bonuses, or spell resistance
	} data;

	struct EnhReqs
	{
		int minLevel;
		Alignment alignment;
		Stat classReq;
		std::map<int, std::vector<uint32_t>> spells; // each entry in the map is considered a sufficient condition
		EnhReqs()
		{
			minLevel = 0;
			alignment = Alignment::ALIGNMENT_NEUTRAL;
			classReq = static_cast<Stat>(-1);
		}
	} reqs;

	ItemEnhancementSpec() {
		//condName = nullptr;
		flags = 0;
		effectiveBonus = 1;
		data.enhBonus = 0;
	}
	ItemEnhancementSpec(const char* CondName, uint32_t Flags, int EffcBonus, int enhBonus = 0);
};

struct TigMsg;

class ItemCreation {
	friend class D20System;
public:


	
	BOOL ItemCreationShow(objHndl crafter, ItemCreationType icType); // shows the item creation UI for the chosen IC type

	void ItemCreationWndRender(int widId);
	void MaaWndRender(int widId);
	void MaaItemRender(int widId);
	void MaaAppliedBtnRender(int widId);

	bool CreateBtnMsg(int widId, TigMsg* msg);
	void MaaCreateBtnRender(int widId) const;
		void CreateItemFinalize(objHndl crafter, objHndl item);
	bool CancelBtnMsg(int widId, TigMsg* msg);
	void MaaCancelBtnRender(int widId) const;

	bool MaaWndMsg(int widId, TigMsg* msg); // message handler for the item creation window

	bool MaaTextboxMsg(int widId, TigMsg* msg);
	bool MaaRenderText(int widId, objHndl item);
	bool MaaItemMsg(int widId, TigMsg* msg);
	bool MaaEffectMsg(int widId, TigMsg* msg);
	void MaaEffectRender(int widId); 
		void MaaEffectGetTextStyle(int effIdx, objHndl crafter, TigTextStyle* & style);
	bool MaaEffectAddMsg(int widId, TigMsg* msg);
		void MaaAppendEnhancement(int effIdx);
	bool MaaEffectRemoveMsg(int widId, TigMsg* msg);
	bool MaaAppliedBtnMsg(int widId, TigMsg* msg);
		


	// Widget utilities
	int GetEffIdxFromWidgetIdx(int widIdx);


	
	int UiItemCreationInit(GameSystemConf& conf);
		bool InitItemCreationRules();
		bool ItemCreationWidgetsInit(int width, int height);
		bool MaaWidgetsInit(int width, int height);
	void MaaWidgetsExit(int widId);
	void ItemCreationWidgetsExit(int widId);
	void UiItemCreationResize(UiResizeArgs& resizeArgs);
		
	// general item creation utilities
	bool MaaCrafterMeetsReqs(int effIdx, objHndl crafter); // checks if the crafter meets the requirements specified in maa_craft_specs.tab
	bool MaaEffectIsInAppliedList(int effIdx);

	int MaaCpCost(int appliedEffectIdx);
	int MaaXpCost(int effIdx); // calculates XP cost for crafting effects in MAA
	int CreateItemResourceCheck(objHndl crafter, objHndl item);
	const char* GetItemCreationMesLine(int lineId);
	char const* ItemCreationGetItemName(objHndl itemHandle) const;
	objHndl MaaGetItemHandle();

	void CreateItemDebitXPGP(objHndl crafter, objHndl item);
	
	bool IsWeaponBonus(int effIdx);
	bool ItemEnhancementIsApplicable(int effIdx);
	int HasPlusBonus(int effIdx);
	/*
	checks the obj_f_item_pad_wielder_condition_array for existence of the effect (or better/equal in the case of +x effects)
	*/
	bool ItemWielderCondsContainEffect(int effIdx, objHndl item);
	
	void CraftScrollWandPotionSetItemSpellData(objHndl item, objHndl crafter);

	ItemCreation();

protected:

	void ButtonStateInit(int wndId);
	void MaaInitCraftedItem(objHndl itemHandle);
	void MaaInitCrafter(objHndl crafter);
	void MaaInitWnd(int wndId);

	void GetMaaSpecs();
	static int GetSurplusXp(objHndl crafter);

	int mItemCreationType = 9;
	objHndl mItemCreationCrafter;
	int mCraftingItemIdx = -1;


	int craftingWidgetId; // denotes which item creation widget is currently active
	int mCreateBtnId;

	WidgetType1* mItemCreationWnd = nullptr;
		
		int mItemCreationWndId;
		int mItemCreationScrollbarId;
		int mMaaItemsScrollbarY = 0;
	WidgetType1* mMaaWnd = nullptr;
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

	MesHandle mItemCreationMes; // mes\\item_creation.mes

	std::map<int, ItemEnhancementSpec> itemEnhSpecs; // the idx has a reserved value of -1 for "none"
	
	
	int& maaSelectedEffIdx = mMaaSelectedEffIdx; // currently selected craftable effect
	bool* itemCreationResourceCheckResults = nullptr;
	
	void	InitItemEnhSpecs();
	std::vector<objHndl> mMaaCraftableItemList; // handles to enchantable inventory items
	std::vector<int> appliedBonusIndices;
	int GoldBaseWorthVsEffectiveBonus[30]; // lookup table for base worth (in GP) vs. effective enhancement level
	int GoldCraftCostVsEffectiveBonus[30]; // lookup table for craft cost (in GP) vs. effective enhancement level
	int craftedItemExistingEffectiveBonus; // stores the crafted item existing (pre-crafting) effective bonus
	int& craftingItemIdx = mCraftingItemIdx; //temple::GetRef<int>(0x10BEE398);

	uint32_t numItemsCrafting[8]; // for each Item Creation type
	objHndl* craftedItemHandles[8]; // proto handles for new items, and item to modifyt for MAA
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

extern ItemCreation itemCreation;



uint32_t ItemWorthAdjustedForCasterLevel(objHndl objHndItem, uint32_t slotLevelNew);
void UiItemCreationCraftingCostTexts(objHndl objHndItem);
/*
struct ButtonStateTextures {
	int normal;
	int hover;
	int pressed;
	ButtonStateTextures() : normal(-1), hover(-1), pressed(-1) {}

	void loadAccept() {
		ui.GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptNormal, normal);
		ui.GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptHover, hover);
		ui.GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptPressed, pressed);
	}

	void loadDecline() {
		ui.GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineNormal, normal);
		ui.GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineHover, hover);
		ui.GetAsset(UiAssetType::Generic, UiGenericAsset::DeclinePressed, pressed);
	}
};
*/