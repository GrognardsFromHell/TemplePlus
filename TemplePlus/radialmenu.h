
#pragma once

#include "common.h"
#include <map>

/*
	Data structures
*/

/*
	Describes an entry in the radial menu (on it's own,
	without relation to where it is in the menu)
*/
#pragma pack(push, 1)
#include "spell_structs.h"
#include "tig/tig_msg.h"

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
	SpellsDomain = 23, // above this are the spell numbers i.e. 0-9 for each class in the above order

	SpellsDismiss = 199
};

enum class RadialMenuEntryType : uint32_t {
	Action = 0,
	Slider = 1,
	Toggle = 2, // Toggle button
	Choice = 3, // One of N (broken in vanilla, i.e. Guidance)
	Parent = 4
};

enum class RadialMenuEntryFlags : int
{
	HasText = 0x1, // might refer to dynamic text specifically
	HasMinArg = 0x2,
	HasMaxArg = 0x4
};

struct RadialMenuEntry {
	char *text; // Text to display
	int field4; // string for popup dialog title, so far
	uint32_t textHash; // ELF hash of "text"
	int fieldc;
	RadialMenuEntryType type; // May define how the children are ordered (seen 4 been used here)
	int minArg;
	int maxArg;
	int actualArg;
	D20ActionType d20ActionType;
	int d20ActionData1;
	D20CAF d20Caf;
	D20SpellData d20SpellData;
	int dispKey; // example: DestructionDomainRadialMenu (the only one I've encountered so far), using this for python actions too now
	BOOL (__cdecl *callback)(objHndl a1, RadialMenuEntry *entry);
	int flags; // see RadialMenuEntryFlags
	int helpId; // String hash for the help topic associated with this entry
	int spellId; // used for stuff like Break Free / Dismiss Spell, and it also puts the id in the d20ActionData1 field

	RadialMenuEntry();
	void SetDefaults();
	int AddChildToStandard(objHndl handle, RadialMenuStandardNode stdNode); // adds node as child to standard node and returns its ID   // wtf gave me some odd errors
	int AddAsChild(objHndl handle, int parentId); // adds node as child to specified parent
};

const int testSizeOfRadMenEn = sizeof(RadialMenuEntry);


struct RadialMenuEntryParent : RadialMenuEntry {
	RadialMenuEntryParent(int combatMesLine);
	RadialMenuEntryParent(std::string & text);
	int AddChildToStandard(objHndl handle, RadialMenuStandardNode stdNode);
	int AddAsChild(objHndl handle, int parentId);
	
};

struct RadialMenuEntrySlider : RadialMenuEntry{
	RadialMenuEntrySlider( int combatMesLine, int _minArg, int _maxArg, void* actualArg, int combatMesHeaderTextLine, uint32_t helpId);
	RadialMenuEntrySlider(const std::string& radialText, const std::string& titleText, int _minArg, int _maxArg, const std::string & helpId);
};

struct RadialMenuEntryAction : RadialMenuEntry
{
	RadialMenuEntryAction(int combatMesLine, D20ActionType d20aType, int data1,  uint32_t helpId);
	RadialMenuEntryAction(int combatMesLine, D20ActionType d20aType, int data1, const char *helpId);
	RadialMenuEntryAction(int combatMesLine, int d20aType, int data1, const char *helpId);
	RadialMenuEntryAction(std::string & text, int d20aType, int data1, std::string & helpId);
	RadialMenuEntryAction(SpellStoreData &spData);
};

struct RadialMenuEntryPythonAction : RadialMenuEntryAction
{
	RadialMenuEntryPythonAction(int combatMesLine, int d20aType, int d20aKey, int data1, const char *helpId);
	RadialMenuEntryPythonAction(int combatMesLine, int d20aType, const char *d20aKey, int data1, const char *helpId);
	RadialMenuEntryPythonAction(std::string & text, int d20aType, int d20aKey, int data1, const char *helpId);
	RadialMenuEntryPythonAction(SpellStoreData& spData, int d20aType, int d20aKey, int data1, const char *helpId = "");
};

struct RadialMenuEntryToggle : RadialMenuEntry
{
	RadialMenuEntryToggle(int combatMesLine, void* actualArg, const char *helpId);
	RadialMenuEntryToggle(std::string &, const char *helpId);
};

const auto TestSizeOfRadialMenuEntry = sizeof(RadialMenuEntry); // should be 72 (0x48)
struct RadialMenuNode {
	RadialMenuEntry entry;
	int children[50]; // Indices of children in the radial menu
	int childCount;
	int parent; // Index of parent node or -1
	int morphsTo; // is set for spontaneous casting (shift held down); see function inside AddSpell
};

struct RadialMenu {
	objHndl obj; // For which object is this the radial menu?
	RadialMenuNode nodes[1000];
	int nodeCount;
	int field8;
};
#pragma pack(pop)



struct D20RadialMenuDef
{
	int parent;
	D20ActionType d20ActionType;
	int d20ActionData1;
	int combatMesLineIdx;
	const char * helpSystemEntryName;
	BOOL(__cdecl * callback)(objHndl, RadialMenuEntry*);
};

/*
	Data driven functions for the radial menus.
*/
class RadialMenus {
	friend class  RadialMenuReplacements;
public:

	void BuildStandardRadialMenu(objHndl handle); // called from the RadialMenuGlobal dispatcher callback
		

	static int standardNodeIndices[200];

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

	/*
		a common callback, used for Checbox types mostly but some others as well
	*/
	int Sub_100F0200(objHndl objHnd, RadialMenuEntry *radEntry);
	int AddChildNode(objHndl objHndCaller, RadialMenuEntry* radialMenuEntry, int parentIdx);
	int AddParentChildNode(objHndl objHndCaller, RadialMenuEntry* radialMenuEntry, int parentIdx); // adds a child who's a parent itself
	int AddParentChildNodeClickable(objHndl objHndCaller, RadialMenuEntry* radialMenuEntry, int parentIdx); // adds a child who's a parent itself, and one that can eb clicked
	int AddRootNode(objHndl obj, const RadialMenuEntry * entry); // might be more accurate to say "parentless node"; this is used for adding spontaneous spells, which are tucked away until needed
	int RadialMenus::AddRootParentNode(objHndl obj, RadialMenuEntry* entry);
	void SetMorphsTo(objHndl obj, int nodeIdx, int spontSpellNode);
	void SetCallbackCopyEntryToSelected(RadialMenuEntry* radEntry);

	int GetActiveRadialMenuNode();
	BOOL ActiveRadialMenuHasActiveNode();
	int MsgHandler(const TigMsg* msg);
	int SpawnMenu(int x, int y);

	void ClearActiveRadialMenu();
	int RadialMenuKeypressHandler(TigMsg* msg);
	/*
		gets the radial menu coordinates in absolute worldspace coords
	*/
	void GetRadialMenuXY(float& radX, float& radY) const;
	RadialMenu* GetActiveRadialMenu() const;
	int GetActiveMenuChildrenCount(int activeNodeIdx) const;
	BOOL PythonActionCallback(const objHndl& handle, RadialMenuEntry* entry);

	std::map<int, std::string> radMenuStrings;

	int lastPythonActionNode = -1; // node id of the last radial menu node accessed via GetActiveMenuNodeRegardMorph; used for communicating with the Python Action layer

protected:
	bool IsShiftPressed();


	void AssignMenu(objHndl handle);
	void SetStandardNode(objHndl handle, int stdNode, int specialParent);

	int GetSpellClassFromSpecialNode(objHndl, int specialParent);
	void AddSpell(objHndl handle, SpellStoreData &spData, int &specNode, RadialMenuEntry &spellEntryAction);
	int GetSpellLevelNodeFromSpellClass(objHndl handle, int spellClass);

	RadialMenuNode* GetActiveMenuNodeRegardMorph(int nodeId);
	int GetChild(int parentId, int i);
};

extern RadialMenus radialMenus;
