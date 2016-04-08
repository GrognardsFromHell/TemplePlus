
#include "stdafx.h"
#include "radialmenu.h"
#include "condition.h"
#include <infrastructure/elfhash.h>
#include "util/fixes.h"
#include "obj.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include <infrastructure/keyboard.h>
#include <dinput.h>
#include "ui\ui.h"
#include "gamesystems/map/d20_help.h"
#include "hotkeys.h"
#include "party.h"
#include "turn_based.h"
#include "action_sequence.h"
#include "critter.h"
#include "combat.h"
//#include "temple_functions.h"

RadialMenus radialMenus;

static_assert(temple::validate_size<RadialMenuEntry, 0x48>::value, "Structure has an incorrect size.");
static_assert(temple::validate_size<RadialMenuNode, 0x11C>::value, "Structure has an incorrect size.");
static_assert(temple::validate_size<RadialMenu, 0x45570>::value, "Structure has an incorrect size.");

static struct RadialMenuAddresses : temple::AddressTable {

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
	int(__cdecl * RadialMenuCheckboxSthgSub_100F0200)(objHndl objHnd, RadialMenuEntry* radialMenuEntry);
	void(__cdecl * CopyEntryToSelected)(objHndl obj, RadialMenuEntry* entry);
	RadialMenu ** activeRadialMenu;
	int * activeRadialMenuNode;

	RadialMenuAddresses() {
		rebase(RadialMenuCheckboxSthgSub_100F0200, 0x100F0200);
		rebase(CopyEntryToSelected, 0x100F0290);

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

		rebase(activeRadialMenu, 0x115B2048);
		rebase(activeRadialMenuNode, 0x115B204C);
		rebase(radialMenus, 0x115B2060);
		rebase(selectedEntry, 0x10BD01E0);
		rebase(standardNodeIndices, 0x11E76CE4);
		rebase(d20RadialDefs, 0x102E8738);
	}


} addresses;


class RadialMenuReplacements : public TempleFix
{
	const char* name() override {
		return "Radial Menu Replacements";
	}

	void apply() override {
		// RadialMenuUpdate
		replaceFunction<void(__cdecl)(objHndl)>(0x1004D1F0, [](objHndl objHnd){
			auto obj = gameSystems->GetObj().GetObject(objHnd);
			Dispatcher * dispatcher = obj->GetDispatcher();
			
			if (dispatch.dispatcherValid(dispatcher)){
				if (objects.IsPlayerControlled(objHnd))	{
					objects.dispatch.DispatcherProcessor(dispatcher, dispTypeRadialMenuEntry, 0, nullptr);
					auto setActiveRadial = temple::GetRef<void(__cdecl)(objHndl)>(0x100F0A70);
					setActiveRadial(objHnd);
				}
			}
		});

		// RadialMenuMsg
		static int (__cdecl*orgMsgHandler)(TigMsg* msg) = replaceFunction<int(__cdecl)(TigMsg*)>(0x1013DC90, [](TigMsg* msg)
		{
			auto evtType = msg->type;
			if (evtType == TigMsgType::KEYSTATECHANGE || evtType == TigMsgType::KEYDOWN || evtType == TigMsgType::CHAR) {
				int dumy = 1;
				DIK_HOME;
			}
			
			auto shiftPressed = false;
			infrastructure::gKeyboard.Update();
			if (infrastructure::gKeyboard.IsKeyPressed(VK_LSHIFT) || infrastructure::gKeyboard.IsKeyPressed(VK_RSHIFT))	{
				shiftPressed = true;
			}
			auto& uiRadmenuShiftPressed = temple::GetRef<int>(0x10BD0234) ;
			uiRadmenuShiftPressed = shiftPressed;

			if (radialMenus.GetActiveRadialMenuNode() == -1)
				return 0;

			

			
			if (evtType != TigMsgType::MOUSE){
				if (evtType == TigMsgType::KEYSTATECHANGE){
					return radialMenus.RadialMenuKeypressHandler(msg);
				}
				return 0;
			}

			auto result = orgMsgHandler(msg);
			return result;
		});
		

	}
};

RadialMenuReplacements radMenuReplace;


int return0()
{
	return 0;
}

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

int RadialMenus::Sub_100F0200(objHndl objHnd, RadialMenuEntry* radEntry)
{
	return addresses.RadialMenuCheckboxSthgSub_100F0200(objHnd, radEntry);
}

int RadialMenus::AddChildNode(objHndl objHnd, RadialMenuEntry* radialMenuEntry, int parentIdx)
{
	auto radMenu = GetForObj(objHnd);
	auto nodeCount = radMenu->nodeCount;
	RadialMenuNode * node = (RadialMenuNode*)&radMenu->nodes[nodeCount];
	((RadialMenu*)radMenu)->nodeCount++;
	memcpy(&node->entry, radialMenuEntry, sizeof(RadialMenuEntry));
	node->parent = parentIdx;
	node->childCount = 0;
	node->morphsTo = -1;
	RadialMenuNode * parentNode = (RadialMenuNode*)&radMenu->nodes[parentIdx];
	node->entry.textHash = conds.hashmethods.StringHash(radialMenuEntry->text);
	parentNode->children[parentNode->childCount++] = nodeCount;
	return nodeCount;

//	return addresses.AddChildNode(objHnd, (const RadialMenuEntry &)radialMenuEntry, parentIdx);
}

int RadialMenus::AddParentChildNode(objHndl objHnd, RadialMenuEntry* radialMenuEntry, int parentIdx)
{
	auto radMenu = GetForObj(objHnd);
	auto nodeCount = radMenu->nodeCount;
	RadialMenuNode * node = (RadialMenuNode*)&radMenu->nodes[nodeCount];
	((RadialMenu*)radMenu)->nodeCount++;
	
	node->entry.SetDefaults();
	memcpy(&node->entry, radialMenuEntry, sizeof(RadialMenuEntry));
	node->entry.d20ActionType = D20A_NONE;
	node->parent = parentIdx;
	node->childCount = 0;
	node->entry.callback = (void ( __cdecl*)(objHndl, RadialMenuEntry*) )return0;
	node->entry.type = RadialMenuEntryType::Parent;
	node->morphsTo = -1;
	node->entry.textHash = conds.hashmethods.StringHash(radialMenuEntry->text);
	if (parentIdx != -1){
		RadialMenuNode * parentNode = (RadialMenuNode*)&radMenu->nodes[parentIdx];
		parentNode->children[parentNode->childCount++] = nodeCount;
	}

	return nodeCount;
}

int RadialMenus::AddParentChildNodeClickable(objHndl objHnd, RadialMenuEntry* radialMenuEntry, int parentIdx){
	auto radMenu = GetForObj(objHnd);
	auto nodeCount = radMenu->nodeCount;
	RadialMenuNode * node = (RadialMenuNode*)&radMenu->nodes[nodeCount];
	((RadialMenu*)radMenu)->nodeCount++;

	node->entry.SetDefaults();
	memcpy(&node->entry, radialMenuEntry, sizeof(RadialMenuEntry));
	//node->entry.d20ActionType = D20A_NONE;
	node->parent = parentIdx;
	node->childCount = 0;
	//node->entry.callback = (void(__cdecl*)(objHndl, RadialMenuEntry*))return0;
	node->entry.type = RadialMenuEntryType::Parent;
	node->morphsTo = -1;
	node->entry.textHash = conds.hashmethods.StringHash(radialMenuEntry->text);
	if (parentIdx != -1){
		RadialMenuNode * parentNode = (RadialMenuNode*)&radMenu->nodes[parentIdx];
		parentNode->children[parentNode->childCount++] = nodeCount;
	}

	return nodeCount;
}

int RadialMenus::AddRootNode(objHndl obj, const RadialMenuEntry* entry){
	auto radMenu = GetForObj(obj);
	auto nodeCount = radMenu->nodeCount;
	RadialMenuNode * node = (RadialMenuNode*)&radMenu->nodes[nodeCount];
	((RadialMenu*)radMenu)->nodeCount++;

	memcpy(&node->entry, entry, sizeof(RadialMenuEntry));
	node->parent = -1;
	node->childCount = 0;
	node->morphsTo = -1;
	node->entry.textHash = ElfHash::Hash(entry->text);
	return nodeCount;
}

int RadialMenus::AddRootParentNode(objHndl obj, RadialMenuEntry* entry){
	auto radMenu = GetForObj(obj);
	auto nodeCount = radMenu->nodeCount;
	RadialMenuNode * node = (RadialMenuNode*)&radMenu->nodes[nodeCount];
	node->entry.SetDefaults();
	((RadialMenu*)radMenu)->nodeCount++;
	memcpy(&node->entry, entry, sizeof(RadialMenuEntry));

	node->entry.d20ActionType = D20A_NONE;
	node->childCount = 0;
	node->entry.callback = (void(__cdecl*)(objHndl, RadialMenuEntry*))return0;
	node->entry.type = RadialMenuEntryType::Parent;
	node->morphsTo = -1;
	node->entry.textHash = ElfHash::Hash(entry->text);
	return nodeCount;

}

void RadialMenus::SetMorphsTo(objHndl obj, int nodeIdx, int spontSpellNode){
	RadialMenu* radMenu = (RadialMenu*)GetForObj(obj);
	auto nodeCount = radMenu->nodeCount;
	if (nodeCount >= nodeIdx){
		if (nodeCount>= spontSpellNode)	{
			radMenu->nodes[nodeIdx].morphsTo = spontSpellNode;
		}
	}
}



void RadialMenus::SetCallbackCopyEntryToSelected(RadialMenuEntry* radEntry)
{
	radEntry->callback = addresses.CopyEntryToSelected;
}

int RadialMenus::GetActiveRadialMenuNode(){
	if (*addresses.activeRadialMenu){
		return *addresses.activeRadialMenuNode;
	}
	return -1;	
}

BOOL RadialMenus::ActiveRadialMenuHasActiveNode()
{
	return GetActiveRadialMenuNode() != -1;
}

int RadialMenus::MsgHandler(TigMsg* msg)
{
	static auto msgHandler = temple::GetRef<int(__cdecl)(TigMsg*)>(0x1013DC90);
	return msgHandler(msg);
}

int RadialMenus::SpawnMenu(int x, int y)
{
	static auto spawnMenu = temple::GetRef<int(__cdecl)(int, int)>(0x1013B250);
	return spawnMenu(x, y);
}

void RadialMenus::ClearActiveRadialMenu()
{
	*addresses.activeRadialMenu = nullptr;
	*addresses.activeRadialMenuNode = -1;
}

int RadialMenus::RadialMenuKeypressHandler(TigMsg* msg)
{
	logger->debug("RadialMenuKeypressHandler: msg arg1 {}   arg2 {}", msg->arg1, msg->arg2);
	if (msg->arg2)
		return 0;

	auto hotkey = msg->arg1;
	auto& uiRadialAssigningHotkey = temple::GetRef<int>(0x10BE6D9C);
	auto hotkeyAssignMouseTextCreate = temple::GetPointer<void(__cdecl)(int, int, void*)>(0x1013C130);
	if (hotkey == DIK_ESCAPE){
		if (uiRadialAssigningHotkey) {
			if (ui.GetCursorTextDrawCallback() == hotkeyAssignMouseTextCreate) {
				ui.SetCursorTextDrawCallback(nullptr, nullptr);
			}
			uiRadialAssigningHotkey = 0;
			return 1;
		}

		radialMenus.ClearActiveRadialMenu();
		return 1;
	}

	if (hotkey == DIK_H){
		helpSys.ClickForHelpToggle();
		return 1;
	}

	if (hotkeys.IsReservedHotkey(msg->arg1)){
		//infrastructure::gKeyboard.Update();
		if (hotkey == DIK_LCONTROL || hotkey == DIK_RCONTROL)
			return 1;
		
		if (infrastructure::gKeyboard.IsKeyPressed(VK_LCONTROL) || infrastructure::gKeyboard.IsKeyPressed(VK_RCONTROL)){
			logger->debug("HotkeyReservedPopup for key {}", msg->arg1);
			hotkeys.HotkeyReservedPopup(msg->arg1);
			return 1;
		}
	} else if (hotkeys.IsNormalNonreservedHotkey(msg->arg1))
	{
		//infrastructure::gKeyboard.Update();
		if (infrastructure::gKeyboard.IsKeyPressed(VK_LCONTROL) || infrastructure::gKeyboard.IsKeyPressed(VK_RCONTROL)){
			uiRadialAssigningHotkey = 1;
			auto& dikToBeHotkeyed = temple::GetRef<uint32_t>(0x10BE6DA0);
			dikToBeHotkeyed = msg->arg1;
			ui.SetCursorTextDrawCallback(hotkeyAssignMouseTextCreate, nullptr);
			return 1;
		}

		if (uiRadialAssigningHotkey){
			uiRadialAssigningHotkey = 0;
			if (ui.GetCursorTextDrawCallback() == hotkeyAssignMouseTextCreate) {
				ui.SetCursorTextDrawCallback(nullptr, nullptr);
			}
		}

		auto leader = party.GetConsciousPartyLeader();
		actSeqSys.TurnBasedStatusInit(leader);
		actSeqSys.ActSeqSpellReset();
		logger->debug("Radial Menu: Reseting sequence.");
		actSeqSys.curSeqReset(leader);
		d20Sys.GlobD20ActnInit();
		auto radmenuHotkeySthg = temple::GetRef<RadialMenu*(__cdecl)(objHndl, int)>(0x100F3D60);
		if (!radmenuHotkeySthg(leader, msg->arg1))
			return 0;
		
		actSeqSys.ActionAddToSeq();
		actSeqSys.sequencePerform();
		auto comrade = party.GetFellowPc(leader);
		char text[1000];
		int soundId;
		critterSys.GetCritterVoiceLine(leader, comrade, text, &soundId);
		critterSys.PlayCritterVoiceLine(leader, comrade, text, soundId);

		radialMenus.ClearActiveRadialMenu();
		return 1;
	} 
	
	return 0;
	

}

RadialMenuEntry::RadialMenuEntry(){
	SetDefaults();
}

void RadialMenuEntry::SetDefaults() {
	addresses.SetDefaults(this);
}

RadialMenuEntrySlider::RadialMenuEntrySlider(int combatMesLine, int _minArg, int _maxArg, void* _actualArg, int combatMesHeaderText, uint32_t _helpId): RadialMenuEntry()
{
	type = RadialMenuEntryType::Slider;
	text = combatSys.GetCombatMesLine(combatMesLine);
	maxArg = max(0, _maxArg);
	minArg = 0;
	actualArg = reinterpret_cast<int>(_actualArg);
	field4 = reinterpret_cast<int>(combatSys.GetCombatMesLine(combatMesHeaderText)); // "Select number of rounds to activate."
	helpId = _helpId;
	callback = (void(__cdecl*)(objHndl, RadialMenuEntry*))temple::GetPointer(0x100F0200);
}
