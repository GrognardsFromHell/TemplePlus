
#include "stdafx.h"
#include "radialmenu.h"

RadialMenus radialMenus;

static_assert(validate_size<RadialMenuEntry, 0x48>::value, "Structure has an incorrect size.");
static_assert(validate_size<RadialMenuNode, 0x11C>::value, "Structure has an incorrect size.");
static_assert(validate_size<RadialMenu, 0x45570>::value, "Structure has an incorrect size.");

static struct RadialMenuAddresses : AddressTable {

	int *radialMenuCount;
	RadialMenu *radialMenus; // 10 consecutive
	RadialMenuEntry *selectedEntry; // Last selected radial entry
	int *standardNodeIndices; // Array of indices for standard radial menu nodes
	D20RadialMenuDef * d20RadialDefs; // Array of definitions for D20 Action entries (size 18 in ToEE)

	RadialMenu* (__cdecl *GetRadialMenu)(objHndl handle);
	void (__cdecl *SetDefaults)(RadialMenuEntry *entry);

	void (__cdecl *RadialMenuAssign)(objHndl handle); // assigns a radial menu to obj and gives it a Root node (from the global definition)
	
	// The difference of the secondary add functions is not quite clear yet.
	// Returns the index of the new node
	int (__cdecl *AddRootNode)(objHndl handle, const RadialMenuEntry &entry);
	int (__cdecl *AddRootNode2)(objHndl handle, const RadialMenuEntry &entry);

	// Returns the index of the new node
	int (__cdecl *AddChildNode)(objHndl handle, const RadialMenuEntry &entry, int parentIdx);
	int (__cdecl *AddChildNode2)(objHndl handle, const RadialMenuEntry &entry, int parentIdx);

	int(__cdecl *SetSpontaneousCastingAltNode)(objHndl handle, int parentIdx, SpellStoreData * spellData);
	int(__cdecl *AddSpell)(objHndl handle, SpellStoreData * spellData, int * idxOut, const RadialMenuEntry & entry); //adds a spell to the Radial Menu

	RadialMenuAddresses() {
		rebase(GetRadialMenu, 0x100F04D0);
		rebase(SetDefaults, 0x100F0AF0);
		
		rebase(AddChildNode, 0x100F0670);
		rebase(AddRootNode, 0x100F0710);

		rebase(AddChildNode2, 0x100F0D10);

		rebase(AddRootNode2, 0x100F0E10);
		rebase(RadialMenuAssign, 0x100F0EE0); 
		rebase(SetSpontaneousCastingAltNode, 0x100F1010);
		rebase(AddSpell, 0x100F1470);
		
		rebase(radialMenuCount, 0x10BD0230);
		rebase(radialMenus, 0x115B2060);
		rebase(selectedEntry, 0x10BD01E0);
		rebase(standardNodeIndices, 0x11E76CE4);
		rebase(d20RadialDefs, 0x102E8738);
	}
} addresses;

const RadialMenu* RadialMenus::GetForObj(objHndl handle) {
	return addresses.GetRadialMenu(handle);
}

vector<const RadialMenu*> RadialMenus::GetAll() {
	vector<const RadialMenu*> result;

	for (int i = 0; i < *addresses.radialMenuCount; ++i) {
		result.push_back(addresses.radialMenus + i);
	}

	return result;
}

const RadialMenuEntry& RadialMenus::GetLastSelected() {
	return *addresses.selectedEntry;
}

int RadialMenus::GetStandardNode(RadialMenuStandardNode node) {
	return addresses.standardNodeIndices[(int)node];
}

void RadialMenuEntry::SetDefaults() {
	addresses.SetDefaults(this);
}
