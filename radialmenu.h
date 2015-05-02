
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

enum class RadialMenuEntryType : uint32_t {
	Action = 0,
	Slider = 1,
	Toggle = 2, // Toggle button
	Choice = 3, // One of N (broken in vanilla, i.e. Guidance)
	Parent = 4
};

struct RadialMenuEntry;
struct RadialMenuEntry {
	char *text; // Text to display
	int field4;
	uint32_t textHash; // ELF hash of "text"
	int fieldc;
	RadialMenuEntryType type; // May define how the children are ordered (seen 4 been used here)
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
	int helpId; // String hash for the help topic associated with this entry
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
	Defines the standard nodes that are always present in the radial menu.
*/
enum RadialMenuStandardNode : uint32_t {
	// Groupings
	Root = 0,
	Spells = 1,
	Skills = 2,
	Feats = 3,
	Class = 4,
	Combat = 5,
	Items = 6,

	// Root->Skills->Alchemy
	Alchemy = 7,

	Movement = 8,
	Offense = 9,
	Tactical = 10,
	Options,
	Potions,
	Wands,
	Scrolls,
	CopyScroll,	// wizard class ability
	SpellsWizard,
	SpellsSorcerer,
	SpellsBard,
	SpellsCleric,
	SpellsPaladin,
	SpellsDruid,
	SpellsRanger,
	SpellsDomain,
};

/*
	Data driven functions for the radial menus.
*/
class RadialMenus {
public:

	/*
		Returns the radial menu for the given object or null
		if no radial menu exists.
	*/
	const RadialMenu *GetForObj(objHndl handle);

	/*
		Gets all currently used radial menus.
	*/
	vector<const RadialMenu*> GetAll();

	/*
		Returns the last selected radial menu entry.
	*/
	const RadialMenuEntry &GetLastSelected();

	/*
		Gets the index in the current radial menu for a given standard node.
	*/
	int GetStandardNode(RadialMenuStandardNode node);

};

extern RadialMenus radialMenus;
