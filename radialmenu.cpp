
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

	RadialMenu* (__cdecl *GetRadialMenu)(objHndl handle);
	void (__cdecl *SetDefaults)(RadialMenuEntry *entry);
	
	// The difference of the secondary add functions is not quite clear yet.
	// Returns the index of the new node
	int (__cdecl *AddRootNode)(objHndl handle, const RadialMenuEntry &entry);
	int (__cdecl *AddRootNode2)(objHndl handle, const RadialMenuEntry &entry);

	// Returns the index of the new node
	int (__cdecl *AddChildNode)(objHndl handle, const RadialMenuEntry &entry, int parentIdx);
	int (__cdecl *AddChildNode2)(objHndl handle, const RadialMenuEntry &entry, int parentIdx);

	RadialMenuAddresses() {
		rebase(GetRadialMenu, 0x100F04D0);
		rebase(SetDefaults, 0x100F0AF0);
		rebase(AddRootNode, 0x100F0710);
		rebase(AddRootNode2, 0x100F0E10);
		rebase(AddChildNode, 0x100F0670);
		rebase(AddChildNode2, 0x100F0D10);
		
		rebase(radialMenuCount, 0x10BD0230);
		rebase(radialMenus, 0x115B2060);
		rebase(selectedEntry, 0x10BD01E0);
		rebase(standardNodeIndices, 0x11E76CE4);
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
