#pragma once

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

	bool CreateBtnMsg(int widId, TigMsg* msg);
	void CreateItemFinalize(objHndl crafter, objHndl item);

	bool MaaEffectMsg(int widId, TigMsg* msg);


	// Widget utilities
	int GetEffIdxFromWidgetIdx(int widIdx);

	int UiItemCreationInit(GameSystemConf* conf);

	// general item creation utilities
	int ItemCraftCostFromEnhancements(int appliedEffectIdx);
	void CreateItemDebitXPGP(objHndl crafter, objHndl item);
	int CreateItemResourceCheck(objHndl crafter, objHndl item);
	bool IsWeaponBonus(int effIdx);
	bool ItemEnhancementIsApplicable(int effIdx);
	int HasPlusBonus(int effIdx);
	/*
	checks the obj_f_item_pad_wielder_condition_array for existence of the effect (or better/equal in the case of +x effects)
	*/
	bool ItemWielderCondsContainEffect(int effIdx, objHndl item);
	
	ItemCreation();
	
protected:
	std::map<int, ItemEnhancementSpec> itemEnhSpecs; // the idx has a reserved value of -1 for "none"
	int maaBtnIds[10];
	void	InitItemEnhSpecs();
	std::vector<int> appliedBonusIndices;
	int GoldBaseWorthVsEffectiveBonus[30]; // lookup table for base worth (in GP) vs. effective enhancement level
	int GoldCraftCostVsEffectiveBonus[30]; // lookup table for craft cost (in GP) vs. effective enhancement level
	int craftedItemExistingEffectiveBonus; // stores the crafted item existing (pre-crafting) effective bonus
	int& craftingItemIdx = temple::GetRef<int>(0x10BEE398);

	uint32_t numItemsCrafting[8]; // for each Item Creation type
	objHndl* craftedItemHandles[8]; // proto handles for new items, and item to modifyt for MAA
	int& itemCreationType = temple::GetRef<int>(0x10BEDF50);
	objHndl& itemCreationCrafter = temple::GetRef<objHndl>(0x10BECEE0);
	char craftedItemName[1024];
	int craftedItemNameLength;
	int craftingWidgetId;

	void GetMaaSpecs();
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
int ItemCreation::craftedItemNameLength;
int ItemCreation::craftingWidgetId;
	
	*/
};

extern ItemCreation itemCreation;



uint32_t ItemWorthAdjustedForCasterLevel(objHndl objHndItem, uint32_t slotLevelNew);
int32_t CreateItemResourceCheck(objHndl ObjHnd, objHndl ObjHndItem);
void CraftScrollWandPotionSetItemSpellData(objHndl objHndItem, objHndl objHndCrafter);
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