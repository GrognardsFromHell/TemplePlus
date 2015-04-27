
#pragma once

#include "common.h"

/*
	Data structures
*/

/*
	Describes an entry in the radial menu (on it's own,
	without relation to where it is in the menu)
*/
#pragma pack(push, 1)
struct RadialMenuEntry;
struct RadialMenuEntry {
	char *text; // Text to display
	int field4;
	uint32_t textHash; // ELF hash of "text"
	int fieldc;
	int ordermaybe; // May define how the children are ordered (seen 4 been used here)
	int minArg;
	int maxArg;
	int actualArg;
	D20ActionType d20ActionType;
	int d20ActionData1;
	D20CAF d20Caf;
	int spellEnumOrg;
	uint32_t spellMetaMagic;
	int field34;
	void (__cdecl *callback)(objHndl a1, RadialMenuEntry *entry);
	int field3c;
	int strhash2; // Another string hash
	int field44;

	void SetDefaults();
};

struct RadialMenuNode {
	RadialMenuEntry entry;
	int children[50]; // Indices of children in the radial menu
	int childCount;
	int parent; // Index of parent node or -1
	int field118;
};

struct RadialMenu {
	objHndl obj; // For which object is this the radial menu?
	RadialMenuNode nodes[1000];
	int nodeCount;
	int field8;
};
#pragma pack(pop)

/*
	Data driven functions for the radial menus.
*/
class RadialMenus {
public:

	/*
		Returns the radial menu for the given object or null
		if no radial menu exists.
	*/
	const RadialMenu *GetRadialMenu(objHndl handle);

	/*
		Gets all currently used radial menus.
	*/
	vector<const RadialMenu*> GetRadialMenus();

	/*
		Returns the last selected radial menu entry.
	*/
	const RadialMenuEntry &GetLastSelected();

};

extern RadialMenus radialMenus;
