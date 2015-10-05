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


uint32_t ItemWorthAdjustedForCasterLevel(objHndl objHndItem, uint32_t slotLevelNew);
int32_t CreateItemResourceCheck(objHndl ObjHnd, objHndl ObjHndItem);
void CraftScrollWandPotionSetItemSpellData(objHndl objHndItem, objHndl objHndCrafter);
void CreateItemDebitXPGP(objHndl objHndCrafter, objHndl objHndItem);
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

uint32_t CraftWandRadialMenu(DispatcherCallbackArgs args);
uint32_t CraftWandOnAdd(DispatcherCallbackArgs args);