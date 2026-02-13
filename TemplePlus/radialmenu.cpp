
#include "stdafx.h"
#include "radialmenu.h"
#include "condition.h"
#include <infrastructure/elfhash.h>
#include "util/fixes.h"
#include "obj.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include <infrastructure/keyboard.h>
#define DIRECTINPUT_VERSION 0x800
#include "tig/tig_keyboard.h"
#include "ui/ui.h"
#include "gamesystems/d20/d20_help.h"
#include "hotkeys.h"
#include "party.h"
#include "turn_based.h"
#include "action_sequence.h"
#include "critter.h"
#include "combat.h"
#include "weapon.h"
#include <tig/tig_mouse.h>
#include "config/config.h"
#include "gamesystems/d20/d20stats.h"
#include "gamesystems/legacymapsystems.h"

RadialMenus radialMenus;
int RadialMenus::standardNodeIndices[200]; // was 120 in Co8

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
	BOOL(__cdecl * CopyEntryToSelected)(objHndl obj, RadialMenuEntry* entry);
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
	static int RmbReleasedHandler(TigMsg* msg);
	void ReplaceStandardRadialNodes();

	// adds reload radials for weapons that won't have conditions for it
	static void AddReload(objHndl critter);

	static RadialMenuNode* HookedGetActiveMenuNode(int nodeId);

	void apply() override {


		// GetStandardNode
		replaceFunction<int(int)>(0x100F12B0, [](int stdNode){
			return radialMenus.GetStandardNode(static_cast<RadialMenuStandardNode>(stdNode));
		});

		ReplaceStandardRadialNodes(); // replaces reference to standardNodeIndices

		// Replace the general radial menu builder
		replaceFunction<void(__cdecl)(objHndl)>(0x100F2650, [](objHndl handle)
		{
			radialMenus.BuildStandardRadialMenu(handle);
		});

		//// SetSandardNode
		//replaceFunction<void(objHndl, int, int)>(0x100F12F0, [](objHndl handle, int stdNode, int specialParent) {
		//	radialMenus.SetStandardNode(handle, stdNode, specialParent);
		//	
		//});


		// RadialMenuUpdate
		replaceFunction<void(__cdecl)(objHndl)>(0x1004D1F0, [](objHndl objHnd){
			critterSys.BuildRadialMenu(objHnd);
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
			
			// keypress
			if (evtType != TigMsgType::MOUSE){
				if (evtType == TigMsgType::KEYSTATECHANGE){
					return radialMenus.RadialMenuKeypressHandler(msg);
				}
				return 0;
			}

			auto mouseFlags = static_cast<MouseStateFlags>(msg->arg4);
			if (! (mouseFlags&MSF_LMB_RELEASED )){
				if (mouseFlags & MSF_RMB_RELEASED){
					//return RmbReleasedHandler(msg);
					RmbReleasedHandler(msg);
				}
			}

			auto result = orgMsgHandler(msg);
			return result;
		});
		

		replaceFunction<int(int)>(0x100F0850, [](int activeNodeIdx)->int
		{
			return radialMenus.GetActiveMenuChildrenCount(activeNodeIdx);
		});

		// AddParentChildNode
		replaceFunction<int(__cdecl)(objHndl, RadialMenuEntry*, int)>(0x100F0D10, [](objHndl handle, RadialMenuEntry *entry, int parIdx)
		{
			return radialMenus.AddParentChildNode(handle, entry, parIdx);
		});

		// GetChild
		replaceFunction<int(int,int)>(0x100F0890, [](int activeNodeIdx, int i)->int
		{
			return radialMenus.GetChild(activeNodeIdx,i);
		});

		//// GetActiveRadMenuNodeRegardMorph
		redirectCall(0x1013C2ED, HookedGetActiveMenuNode);

		static int (__cdecl*orgRadialMenuGlobal)(DispatcherCallbackArgs) =
			replaceFunction<int (__cdecl)(DispatcherCallbackArgs)>(0x100EEFC0,
				[](DispatcherCallbackArgs args) {
					auto result = orgRadialMenuGlobal(args);
					AddReload(args.objHndCaller);
					return result;
				});
	}
};

int RadialMenuReplacements::RmbReleasedHandler(TigMsg* msg)
{
	float radX, radY;
	radialMenus.GetRadialMenuXY(radX, radY);

	float screenX, screenY;
	auto worldToLocalScreen = temple::GetRef<void(__cdecl)(PointNode, float&, float&)>(0x10029040);
	worldToLocalScreen(PointNode(radX, 0, radY), screenX, screenY);

	auto& intgameRadialmenuIgnoreClose = temple::GetRef<BOOL>(0x10BE6D80);
	auto& intgameRadialmenuIgnoreCloseTillMove = temple::GetRef<BOOL>(0x10BE6D70);
	
	// todo: finish rest of this

	return 1;
}

void RadialMenuReplacements::ReplaceStandardRadialNodes()
{
	auto writeval = reinterpret_cast<int>(RadialMenus::standardNodeIndices);

	// SetStandardNode
	write(0x100F145F + 3, &writeval, sizeof(int*));
	write(0x100F1441 + 3, &writeval, sizeof(int*));


	write(0x100F15B9 + 3, &writeval, sizeof(int*));
	write(0x100F16E7 + 3, &writeval, sizeof(int*));
	write(0x100F1818 + 3, &writeval, sizeof(int*));
	write(0x100F1945 + 3, &writeval, sizeof(int*));
	write(0x100F1A77 + 3, &writeval, sizeof(int*));
	write(0x100F1BA8 + 3, &writeval, sizeof(int*));
	write(0x100F1D30 + 3, &writeval, sizeof(int*));
	write(0x100F1D70 + 3, &writeval, sizeof(int*));
	write(0x100F1E97 + 3, &writeval, sizeof(int*));
	write(0x100F1FD0 + 3, &writeval, sizeof(int*));
	write(0x100F2100 + 3, &writeval, sizeof(int*));
	write(0x100F2384 + 3, &writeval, sizeof(int*));

	// BuildRadialMenuNormal
	write(0x100F2674 + 2, &writeval, sizeof(int*));


	writeval = reinterpret_cast<int>(&RadialMenus::standardNodeIndices[4]) ;
	write(0x100F26EB + 1, &writeval, sizeof(int*));

	writeval = reinterpret_cast<int>(&RadialMenus::standardNodeIndices[15]);
	write(0x100F2723 + 1, &writeval, sizeof(int*));
	
	writeval = reinterpret_cast<int>(&RadialMenus::standardNodeIndices[0]);
	write(0x100F27EC + 3, &writeval, sizeof(int*));
	
	writeval = reinterpret_cast<int>(&RadialMenus::standardNodeIndices[2]);
	write(0x100F2876 + 2, &writeval, sizeof(int*));
	write(0x100F28F4 + 1, &writeval, sizeof(int*));
	write(0x100F2969 + 2, &writeval, sizeof(int*));
	write(0x100F29E3 + 2, &writeval, sizeof(int*));
	write(0x100F2A62 + 1, &writeval, sizeof(int*));
	write(0x100F2AD0 + 2, &writeval, sizeof(int*));

	writeval = reinterpret_cast<int>(&RadialMenus::standardNodeIndices[10]) ;
	write(0x100F2B71 + 2, &writeval, sizeof(int*));
}

void RadialMenuReplacements::AddReload(objHndl critter)
{
	if (!config.stricterRulesEnforcement) return;

	auto weap = inventory.ItemWornAt(critter, WeaponPrimary);
	if (!weap || !weapons.IsUnloaded(weap)) return;

	if (objects.GetWeaponType(weap) != wt_sling) return;

	RadialMenuEntryAction radEntry(5009, D20A_RELOAD, 0, "TAG_WEAPONS_SLING");
	radEntry.AddChildToStandard(critter, Offense);
}

RadialMenuNode * RadialMenuReplacements::HookedGetActiveMenuNode(int nodeId){
	return radialMenus.GetActiveMenuNodeRegardMorph(nodeId);
}

RadialMenuReplacements radMenuReplace;


int return0()
{
	return 0;
}

void RadialMenus::BuildStandardRadialMenu(objHndl handle){

	AssignMenu(handle); 
	standardNodeIndices[0] = 0; // root
	// set the main 6 nodes: Spells, Skills, Feats, Class, Combat, Items
	for (int i = RadialMenuStandardNode::Spells; i <= RadialMenuStandardNode::Items; i++){
		SetStandardNode(handle, i, RadialMenuStandardNode::Root);
	}

	// Set the combat nodes
	for (int i= RadialMenuStandardNode::Movement; i <= RadialMenuStandardNode::Options; i++)
	{
		SetStandardNode(handle, i, RadialMenuStandardNode::Combat);
	}

	// Set the magic item nodes: potions, wands, scrolls
	for (int i = RadialMenuStandardNode::Potions; i <= RadialMenuStandardNode::Scrolls; i++)
	{
		SetStandardNode(handle, i, RadialMenuStandardNode::Items);
	}


	// Set spell nodes (classes & spell numbers)
	SetStandardNode(handle, RadialMenuStandardNode::CopyScroll, RadialMenuStandardNode::Class);

	for (int i=RadialMenuStandardNode::SpellsWizard; i <= RadialMenuStandardNode::SpellsDomain; i++)
	{
		SetStandardNode(handle, i, RadialMenuStandardNode::Spells);
	}

	for (int i = RadialMenuStandardNode::SpellsWizard; i <= RadialMenuStandardNode::SpellsDomain; i++)
	{
		for (auto j = 24 + (i - RadialMenuStandardNode::SpellsWizard) * 10; j < 34 + (i- RadialMenuStandardNode::SpellsWizard)*10; j++)
		{
			SetStandardNode(handle, j, i);
		}
	}

	// add the tactical options
	for (auto i=0; i < 18; i++){
		auto & def = addresses.d20RadialDefs[i];

		RadialMenuEntryAction radEntry(def.combatMesLineIdx, def.d20ActionType, def.d20ActionData1, def.helpSystemEntryName);
		radEntry.callback = def.callback;
		radEntry.AddAsChild(handle, GetStandardNode((RadialMenuStandardNode)def.parent));
	}

	// Decipher Script
	auto umdSkill = critterSys.SkillBaseGet(handle, skill_use_magic_device);
	if (umdSkill)
	{
		RadialMenuEntryAction radEntry(5073, D20ActionType::D20A_USE_MAGIC_DEVICE_DECIPHER_WRITTEN_SPELL, 0, "TAG_UMD");
		radEntry.AddAsChild(handle, GetStandardNode(RadialMenuStandardNode::Skills));
	}

	// Disable Device
	if (critterSys.SkillBaseGet(handle, skill_disable_device))
	{
		RadialMenuEntryAction radEntry(5080, D20ActionType::D20A_DISABLE_DEVICE, 0, "TAG_DISABLE_DEVICE");
		radEntry.AddAsChild(handle, GetStandardNode(RadialMenuStandardNode::Skills));
	}

	// Open Lock
	if (critterSys.SkillBaseGet(handle, skill_open_lock))
	{
		RadialMenuEntryAction radEntry(5086, D20ActionType::D20A_OPEN_LOCK, 0, "TAG_OPEN_LOCK");
		radEntry.AddAsChild(handle, GetStandardNode(RadialMenuStandardNode::Skills));
	}

	// Open Lock
	if (critterSys.SkillBaseGet(handle, skill_pick_pocket))
	{
		RadialMenuEntryAction radEntry(5087, D20ActionType::D20A_SLEIGHT_OF_HAND, 0, "TAG_SLEIGHT_OF_HAND");
		radEntry.AddAsChild(handle, GetStandardNode(RadialMenuStandardNode::Skills));
	}

	// Sneak
	RadialMenuEntryAction sneakRadEntry(5082 + (critterSys.IsMovingSilently(handle) != 0), D20ActionType::D20A_SNEAK, 0, "TAG_RADIAL_MENU_SNEAK");
	sneakRadEntry.AddAsChild(handle, GetStandardNode(RadialMenuStandardNode::Skills));

	// Search
	if (!combatSys.isCombatActive()){
		RadialMenuEntryAction radEntry(5081, D20ActionType::D20A_SEARCH, 0, "TAG_SEARCH");
		radEntry.AddAsChild(handle, GetStandardNode(RadialMenuStandardNode::Skills));
	}


	// Spells
	auto obj = objSystem->GetObject(handle);
	auto numKnown = obj->GetSpellArray(obj_f_critter_spells_known_idx).GetSize();
	auto numMem = obj->GetSpellArray(obj_f_critter_spells_memorized_idx).GetSize();
	if (numKnown > 0 || numMem > 0){
		RadialMenuEntryAction radEntry(5091, D20ActionType::D20A_READY_COUNTERSPELL, 0, "TAG_RADIAL_MENU_READY_COUNTERSPELL");
		radEntry.AddAsChild(handle, GetStandardNode(RadialMenuStandardNode::Tactical));
	}

	
	int specNode;
	RadialMenuEntry spellEntryAction;
	for (auto i=0u; i<numMem; i++){
		auto spData = obj->GetSpell(obj_f_critter_spells_memorized_idx, i);
		if (spellSys.isDomainSpell(spData.classCode) 
			|| d20ClassSys.IsVancianCastingClass(spellSys.GetCastingClass(spData.classCode))){
			AddSpell(handle, spData, specNode, spellEntryAction);
			
		}
	}
	// Spells Known
	for (auto i = 0u; i<numKnown; i++) {
		auto spData = obj->GetSpell(obj_f_critter_spells_known_idx, i);
		if (!spellSys.isDomainSpell(spData.classCode)
			&& d20ClassSys.IsNaturalCastingClass(spellSys.GetCastingClass(spData.classCode))) {
			AddSpell(handle, spData, specNode, spellEntryAction);
		}
	}

	// dismiss spells
	if (d20Sys.d20Query(handle, DK_QUE_Critter_Can_Dismiss_Spells)){
		RadialMenuEntryParent dism(5101);
		SetStandardNode(handle, RadialMenuStandardNode::SpellsDismiss ,RadialMenuStandardNode::Spells);
	}
}

bool RadialMenus::IsShiftPressed(){
	auto shiftPressed = temple::GetRef<int>(0x10BD0234);
	return shiftPressed != 0;
}

/* 0x100F0EE0 */
void RadialMenus::AssignMenu(objHndl handle)
{
	auto radMenu = (RadialMenu*)GetForObj(handle);
	if (radMenu) {
		for (auto i = 0u; i < radMenu->nodeCount; ++i) {
			auto &node = radMenu->nodes[i];
			if (node.entry.flags & (int)RadialMenuEntryFlags::HasText) {
				logger->debug("AssignMenu: \t Freeing {}", i);
				free(node.entry.text);
			}
		}
	}
	else {
		auto &radMenuCount = *addresses.radialMenuCount;
		const int RAD_MENU_CAPACITY = 10;
		Expects(radMenuCount < RAD_MENU_CAPACITY); // bug in vanilla - no checks here
		radMenu = &(addresses.radialMenus[radMenuCount]);
		radMenuCount++;
		radMenu->obj = handle;
	}
	radMenu->nodeCount = 0;
	AddParentChildNode(handle, /*root node*/ temple::GetPointer<RadialMenuEntry>(0x115B2000), -1);
}

void RadialMenus::SetStandardNode(objHndl handle, int stdNode, int specialParent){

	auto meskey = 1000 + stdNode;
	auto isSpellNode = false;
	auto isVanillaNode = false;

	if (stdNode == RadialMenuStandardNode::SpellsDismiss){
		meskey = 5101;
	}
	else if (stdNode > RadialMenuStandardNode::SpellsDomain && stdNode <= 104) {
		isSpellNode = true;

		if (specialParent == RadialMenuStandardNode::SpellsSorcerer && stdNode < 34
			|| specialParent == RadialMenuStandardNode::SpellsBard && stdNode < 44)
			isVanillaNode = true;

		if (isVanillaNode)
			meskey = (stdNode - 24) % NUM_SPELL_LEVELS_VANILLA + 1024;
		else
			meskey = (stdNode - 24) % NUM_SPELL_LEVELS + 1024;
	}

	RadialMenuEntryParent radMenuEntry(meskey);

	// change name
	if (stdNode >= RadialMenuStandardNode::SpellsWizard && stdNode < RadialMenuStandardNode::SpellsDomain){
		auto spellClass = GetSpellClassFromSpecialNode(handle, stdNode);
		
		if (!spellSys.isDomainSpell(spellClass)){
			auto classEnum = spellSys.GetCastingClass(spellClass);
			radMenuEntry.text = (char*)d20Stats.GetStatShortName(classEnum);
		} else
		{
			radMenuEntry.text = (char*)combatSys.GetCombatMesLine(1000 + RadialMenuStandardNode::SpellsDomain);
		}
	
	}
	// Set min/max for Natural Casting
	else if (isSpellNode) {
		auto spellClass = GetSpellClassFromSpecialNode(handle, specialParent);
		if (!spellSys.isDomainSpell(spellClass))	{
			auto classCode = spellSys.GetCastingClass(spellClass);
			auto isNaturalCasting = d20ClassSys.IsNaturalCastingClass(classCode);

			if (isNaturalCasting) {
				radMenuEntry.flags |= 6; // draw min/max arg
				auto spLvl = (stdNode - 24) % NUM_SPELL_LEVELS;

				//if (isVanillaNode) {
				//	spLvl = (stdNode - 24) % 6; // vanilla num spell levels
				//}

				auto spellClass = spellSys.GetSpellClass(classCode);
				auto numSpellsPerDay = spellSys.GetNumSpellsPerDay(handle, classCode, spLvl);
				if (numSpellsPerDay < 0)
					numSpellsPerDay = 0;

				radMenuEntry.maxArg = numSpellsPerDay;

				auto numSpellsCast = spellSys.NumSpellsInLevel(handle, obj_f_critter_spells_cast_idx, spellClass, spLvl);
				if (numSpellsCast < numSpellsPerDay)
					radMenuEntry.minArg = numSpellsPerDay - numSpellsCast;
				else
					radMenuEntry.minArg = 0;

			}
		}


	}


	RadialMenus::standardNodeIndices[stdNode] = radialMenus.AddParentChildNode(handle, &radMenuEntry, RadialMenus::standardNodeIndices[specialParent]);
}

int RadialMenus::GetSpellClassFromSpecialNode(objHndl handle, int specialParent){
	auto classArr = objSystem->GetObject(handle)->GetInt32Array(obj_f_critter_level_idx);
	auto classArrSize = classArr.GetSize();
	std::vector<int> spellClasses;
	for (auto classEnum: d20ClassSys.classEnumsWithSpellLists){

		if (objects.StatLevelGet(handle, classEnum) <= 0)
			continue;

		spellClasses.push_back( spellSys.GetSpellClass(classEnum) );
	}

	auto idx = specialParent - RadialMenuStandardNode::SpellsWizard;
	if (idx < spellClasses.size()){
		return spellClasses[specialParent - RadialMenuStandardNode::SpellsWizard];
	}
		
	return 0; // will register as domain spell

	// the legacy stuff
	static auto getClassFromNode = [](int specPar) {
		if (specPar== RadialMenuStandardNode::SpellsWizard)
			return stat_level_wizard;
		if (specPar== RadialMenuStandardNode::SpellsBard)
			return stat_level_bard;
		if (specPar== RadialMenuStandardNode::SpellsCleric)
			return stat_level_cleric;
		if (specPar== RadialMenuStandardNode::SpellsDomain)
			return stat_level_cleric;
		if (specPar== RadialMenuStandardNode::SpellsDruid)
			return stat_level_druid;
		if (specPar== RadialMenuStandardNode::SpellsPaladin)
			return stat_level_paladin;
		if (specPar== RadialMenuStandardNode::SpellsRanger)
			return stat_level_ranger;
		if (specPar== RadialMenuStandardNode::SpellsSorcerer)
			return stat_level_sorcerer;
		return (Stat) 0;
	};
	return spellSys.GetSpellClass(getClassFromNode(specialParent));
}

void RadialMenus::AddSpell(objHndl handle, SpellStoreData & spData, int & specNode, RadialMenuEntry & entry) {
	//auto addSpell = temple::GetRef<void(__cdecl)(objHndl, SpellStoreData&, int&, RadialMenuEntry&)>(0x100F1470);
	//addSpell(handle, spData, specNode, spellEntryAction);

	if (spData.spellStoreState.usedUp & 1) {
		return;
	}

	if (spellSys.spellCanCast(handle, spData.spellEnum, spData.classCode, spData.spellLevel) != TRUE) {
		return;
	}

	if (spData.classCode == spellSys.GetSpellClass(stat_level_paladin) && d20Sys.d20Query(handle, DK_QUE_IsFallenPaladin)){
		return;
	}

	entry.SetDefaults();
	entry.text = (char*)spellSys.GetSpellMesline(spData.spellEnum);
	
	if (spData.spellLevel >= 10)
		return;

	specNode = GetSpellLevelNodeFromSpellClass(handle, spData.classCode) + spData.spellLevel;

	if (!spellSys.SpellHasMultiSelection(spData.spellEnum)){
		
		entry.d20ActionType = D20A_CAST_SPELL;
		entry.d20SpellData.Set(spData.spellEnum, spData.classCode, spData.spellLevel, -1, spData.metaMagicData);
		entry.d20ActionData1 = 0;
		entry.helpId = ElfHash::Hash(spellSys.GetSpellEnumTAG(spData.spellEnum));
		auto nodeIdx = entry.AddAsChild(handle, GetStandardNode((RadialMenuStandardNode)specNode));
		spellSys.SetSpontaneousCastingAltNode(handle, nodeIdx ,&spData);
		return;
	}

	// Multiselection spells section
	
	// the parent node
	auto parentNodeIdx = radialMenus.AddParentChildNode(handle, &entry, GetStandardNode((RadialMenuStandardNode)specNode));
	spellSys.SetSpontaneousCastingAltNode(handle, parentNodeIdx, &spData);

	// the options
	std::vector<SpellMultiOption> multiOptions;
	if (!spellSys.GetMultiSelectOptions(spData.spellEnum, multiOptions))
	{
		logger->error("Spell multiselect options not found!");
		return;
	}

	// populate options
	auto numOptions = multiOptions.size();
	for (auto i=0u; i<numOptions; i++){
		auto &op = multiOptions[i];
		entry.SetDefaults();
		
		entry.d20SpellData.Set(spData.spellEnum, spData.classCode, spData.spellLevel, -1, spData.metaMagicData);
		entry.d20ActionType = D20A_CAST_SPELL;
		entry.d20ActionData1 = 0;
		entry.helpId = ElfHash::Hash(spellSys.GetSpellEnumTAG(spData.spellEnum));
		SetCallbackCopyEntryToSelected(&entry);


		
		if (op.isProto){
			auto protoId = multiOptions[i].value;
			entry.minArg = protoId;

			auto protoHandle = objSystem->GetProtoHandle(protoId);
			auto protoObj = objSystem->GetObject(protoHandle);
			if (!protoObj) {
				logger->error("Multioption radial: missing proto, ID {}", protoId);
				continue;
			}
			entry.text = (char*)description.GetDescriptionString(protoObj->GetInt32(obj_f_description));

		} else	{
			MesLine line(multiOptions[i].value);
			mesFuncs.GetLine_Safe(*spellSys.spellsRadialMenuOptionsMes, &line);
			entry.text = (char*)line.value;	
			entry.minArg = i + 1;
		}

		entry.AddAsChild(handle, parentNodeIdx);
	}
}

int RadialMenus::GetSpellLevelNodeFromSpellClass(objHndl handle, int spellClass){

	auto classArr = objSystem->GetObject(handle)->GetInt32Array(obj_f_critter_level_idx);
	auto classArrSize = classArr.GetSize();
	std::vector<int> spellClasses;
	for (auto classEnum : d20ClassSys.classEnumsWithSpellLists) {

		if (objects.StatLevelGet(handle, classEnum) <= 0)
			continue;

		spellClasses.push_back(spellSys.GetSpellClass(classEnum));
	}

	// domain spells go last
	if (spellSys.isDomainSpell(spellClass))
		return RadialMenuStandardNode::SpellsDomain + 1 +  spellClasses.size() * NUM_SPELL_LEVELS;

	for (auto i = 0u; i < spellClasses.size(); i++) {
		if (spellClasses[i] == spellClass)
			return RadialMenuStandardNode::SpellsDomain + 1 + i * NUM_SPELL_LEVELS;
	}

	logger->warn("GetSpellMasterNodeFromSpellClass: No matching class found for memorized/known spell???");
	return RadialMenuStandardNode::SpellsDomain + 1;
}

RadialMenuNode * RadialMenus::GetActiveMenuNodeRegardMorph(int nodeId){
	auto activeMenu = GetActiveRadialMenu();
	if (IsShiftPressed() && activeMenu->nodes[nodeId].morphsTo != -1){
		nodeId = activeMenu->nodes[nodeId].morphsTo;
	}

	if (activeMenu->nodes[nodeId].entry.d20ActionType == D20A_PYTHON_ACTION){
		lastPythonActionNode = nodeId;
	}
		

	return &activeMenu->nodes[nodeId];
}

int RadialMenus::GetChild(int nodeId, int i){
	auto activeMenu = GetActiveRadialMenu();
	auto altNode = activeMenu->nodes[nodeId].morphsTo;

	if (IsShiftPressed() && altNode != -1){
		nodeId = activeMenu->nodes[nodeId].morphsTo;
	}

	auto result = activeMenu->nodes[nodeId].children[i];
	if (IsShiftPressed() && activeMenu->nodes[result].morphsTo != -1){
		result = activeMenu->nodes[result].morphsTo;
	}

	return result;
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
	return standardNodeIndices[(int)node];
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
	node->entry = *radialMenuEntry;
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
	node->entry = *radialMenuEntry;
	node->entry.d20ActionType = D20A_NONE;
	node->parent = parentIdx;
	node->childCount = 0;
	node->entry.callback = (int(__cdecl*)(objHndl, RadialMenuEntry*))temple::GetRef<void(__cdecl)()>(0x10262530); //return0; ToEE actually checks against this... fucking hell
	node->entry.type = RadialMenuEntryType::Parent;
	node->morphsTo = -1;
	node->entry.textHash = ElfHash::Hash(radialMenuEntry->text);
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
	node->entry.callback = (int(__cdecl*)(objHndl, RadialMenuEntry*))temple::GetRef<void(__cdecl)()>(0x10262530); //return0; ToEE actually checks against this... fucking hell
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
	//node->entry.callback = (int(__cdecl*)(objHndl, RadialMenuEntry*))return0;
	node->entry.callback = (int(__cdecl*)(objHndl, RadialMenuEntry*))temple::GetRef<void(__cdecl)()>(0x10262530); //return0; ToEE actually checks against this... fucking hell
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

int RadialMenus::MsgHandler(const TigMsg* msg)
{
	static auto msgHandler = temple::GetRef<int(__cdecl)(const TigMsg*)>(0x1013DC90);
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
	static auto hotkeyAssignMouseTextCreate = temple::GetPointer<void(__cdecl)(int, int, void*)>(0x1013C130);
	if (hotkey == DIK_ESCAPE){
		if (uiRadialAssigningHotkey) {
			if (mouseFuncs.GetCursorDrawCallbackId() == (uint32_t)hotkeyAssignMouseTextCreate) {
				mouseFuncs.SetCursorDrawCallback(nullptr, 0);
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
			mouseFuncs.SetCursorDrawCallback([](int x, int y) {
				hotkeyAssignMouseTextCreate(x, y, nullptr);
			}, (uint32_t)hotkeyAssignMouseTextCreate);
			return 1;
		}

		if (uiRadialAssigningHotkey){
			uiRadialAssigningHotkey = 0;
			if (mouseFuncs.GetCursorDrawCallbackId() == (uint32_t) hotkeyAssignMouseTextCreate) {
				mouseFuncs.SetCursorDrawCallback(nullptr, 0);
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
		critterSys.GetOkayVoiceLine(leader, comrade, text, &soundId);
		critterSys.PlayCritterVoiceLine(leader, comrade, text, soundId);

		radialMenus.ClearActiveRadialMenu();
		return 1;
	} 
	
	return 0;
	

}

void RadialMenus::GetRadialMenuXY(float& radX, float& radY) const
{
	radX = temple::GetRef<float>(0x10BD022C);
	radY = temple::GetRef<float>(0x10BD0228);
}

RadialMenu* RadialMenus::GetActiveRadialMenu() const
{
	return (*addresses.activeRadialMenu);
}

int RadialMenus::GetActiveMenuChildrenCount(int nodeidx) const
{
	auto activeMenu = GetActiveRadialMenu();
	if (!activeMenu)
		return 0;
	if (temple::GetRef<BOOL>(0x10BD0234) && activeMenu && activeMenu->nodes[nodeidx].morphsTo != -1)
	{
		nodeidx = activeMenu->nodes[nodeidx].morphsTo;
	}
	return activeMenu->nodes[nodeidx].childCount;
}

BOOL RadialMenus::PythonActionCallback(const objHndl & handle, RadialMenuEntry * entry){
	actSeqSys.TurnBasedStatusInit(handle);
	actSeqSys.SequenceSwitch(handle);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(entry->d20ActionType, entry->d20ActionData1);
	// d20Sys.globD20ActionKey = static_cast<D20DispatcherKey>(entry->dispKey);

	d20Sys.globD20Action->SetPythonActionEnum( (D20DispatcherKey)entry->dispKey);

	auto entryType = entry->type;
	switch (entryType){
		case RadialMenuEntryType::Slider: 
		case RadialMenuEntryType::Toggle: 
		case RadialMenuEntryType::Choice: 
			d20Sys.globD20Action->radialMenuActualArg = *reinterpret_cast<uint32_t*>(entry->actualArg); // actualArg is actually a pointer to a condition's arg in practice
			break;
		default: 
			break;
	}
	
	d20Sys.globD20Action->d20Caf |= entry->d20Caf;
	d20Sys.GlobD20ActnSetSpellData(&entry->d20SpellData);
	ClearActiveRadialMenu();

	return TRUE;
}

RadialMenuEntry::RadialMenuEntry(){
	SetDefaults();
}

void RadialMenuEntry::SetDefaults() {
	addresses.SetDefaults(this);
}

int RadialMenuEntry::AddChildToStandard(objHndl handle, RadialMenuStandardNode stdNode){
	int parentNode = radialMenus.GetStandardNode(static_cast<RadialMenuStandardNode>(stdNode));
	return radialMenus.AddChildNode(handle, this, parentNode);
}

int RadialMenuEntry::AddAsChild(objHndl handle, int parentId) {
	return radialMenus.AddChildNode(handle, this, parentId);
}


RadialMenuEntrySlider::RadialMenuEntrySlider(int combatMesLine, int _minArg, int _maxArg, void* _actualArg, int combatMesHeaderText, uint32_t _helpId): RadialMenuEntry()
{
	type = RadialMenuEntryType::Slider;
	text = combatSys.GetCombatMesLine(combatMesLine);
	maxArg = max(0, _maxArg);
	minArg = 0;
	actualArg = reinterpret_cast<int>(_actualArg);
	if (combatMesHeaderText != -1)
		field4 = reinterpret_cast<int>(combatSys.GetCombatMesLine(combatMesHeaderText)); // the popup title
	helpId = _helpId;
	callback = (BOOL(__cdecl*)(objHndl, RadialMenuEntry*))temple::GetPointer(0x100F0200);
}

RadialMenuEntrySlider::RadialMenuEntrySlider(const std::string& radialText, const std::string& titleText, int _minArg, int _maxArg, const std::string& helpId)
{
	type = RadialMenuEntryType::Slider;
	{
		auto textId = ElfHash::Hash(radialText);
		auto textCache = radialMenus.radMenuStrings.find(textId);
		if (textCache == radialMenus.radMenuStrings.end()) {
			radialMenus.radMenuStrings[textId] = radialText;
		}
		this->text = (char*)radialMenus.radMenuStrings[textId].c_str();
	}
	if (titleText.size() > 0){
		auto textId = ElfHash::Hash(titleText);
		auto textCache = radialMenus.radMenuStrings.find(textId);
		if (textCache == radialMenus.radMenuStrings.end()) {
			radialMenus.radMenuStrings[textId] = titleText;
		}
		this->field4 = reinterpret_cast<int>(radialMenus.radMenuStrings[textId].c_str() );
	}

	this->helpId = ElfHash::Hash(helpId);
	maxArg = max(0, _maxArg);
	minArg = _minArg;
	
	callback = (BOOL(__cdecl*)(objHndl, RadialMenuEntry*))temple::GetPointer(0x100F0200);
}

RadialMenuEntryAction::RadialMenuEntryAction(int combatMesLine, D20ActionType d20aType, int data1, uint32_t HelpId) : RadialMenuEntry() {
	type = RadialMenuEntryType::Action;
	if (combatMesLine > 0)
		text = combatSys.GetCombatMesLine(combatMesLine);
	else
		text = "NULL";
	helpId = HelpId;
	d20ActionType = d20aType;
	d20ActionData1 = data1;
}

RadialMenuEntryAction::RadialMenuEntryAction(int combatMesLine, D20ActionType d20aType, int data1, const char *helpId): RadialMenuEntryAction(combatMesLine, d20aType, data1, ElfHash::Hash(helpId))
{

}

RadialMenuEntryAction::RadialMenuEntryAction(int combatMesLine, int d20aType, int data1, const char *helpId):RadialMenuEntryAction(combatMesLine, static_cast<D20ActionType>(d20aType), data1, ElfHash::Hash(helpId))
{
}

RadialMenuEntryAction::RadialMenuEntryAction(std::string & textArg, int d20aType, int data1, std::string & helpId) : RadialMenuEntry()
{
	type = RadialMenuEntryType::Action;
	auto textId = ElfHash::Hash(textArg);
	auto textCache = radialMenus.radMenuStrings.find(textId);
	if (textCache == radialMenus.radMenuStrings.end()){
		radialMenus.radMenuStrings[textId] = textArg;
	}

	this->text = (char*)radialMenus.radMenuStrings[textId].c_str();
	this->helpId = ElfHash::Hash(helpId);
	d20ActionType = static_cast<D20ActionType>(d20aType);
	d20ActionData1 = data1;
}

RadialMenuEntryAction::RadialMenuEntryAction(SpellStoreData & spData){
	type = RadialMenuEntryType::Action;
	if (spData.spellEnum > 0)
		this->text = (char*)spellSys.GetSpellName(spData.spellEnum);
	else
		text = "NULL SPELL";
	this->d20ActionType = D20A_CAST_SPELL;
	this->d20ActionData1 = 0;

	this->helpId = ElfHash::Hash(spellSys.GetSpellEnumTAG(spData.spellEnum));
	this->d20SpellData.Set(spData.spellEnum, spData.classCode, spData.spellLevel, -1, spData.metaMagicData);
}

RadialMenuEntryToggle::RadialMenuEntryToggle(int combatMesLine, void* ActualArg, const char *HelpId): RadialMenuEntry()
{
	type = RadialMenuEntryType::Toggle;
	text = combatSys.GetCombatMesLine(combatMesLine);
	this->helpId = ElfHash::Hash(HelpId);
	callback = temple::GetRef<BOOL(__cdecl)(objHndl, RadialMenuEntry*)>(0x100F0200);
	minArg = 0;
	maxArg = 1;
	actualArg = reinterpret_cast<int>(ActualArg);
}
RadialMenuEntryToggle::RadialMenuEntryToggle(std::string & textArg, const char *HelpId): RadialMenuEntry()
{
	type = RadialMenuEntryType::Toggle;
	auto textId = ElfHash::Hash(textArg);
	auto textCache = radialMenus.radMenuStrings.find(textId);
	if (textCache == radialMenus.radMenuStrings.end()) {
		radialMenus.radMenuStrings[textId] = textArg;
	}
	this->text = (char*)radialMenus.radMenuStrings[textId].c_str();
	this->helpId = ElfHash::Hash(HelpId);
	callback = temple::GetRef<BOOL(__cdecl)(objHndl, RadialMenuEntry*)>(0x100F0200);
	minArg = 0;
	maxArg = 1;
}
;

//
//int RadialMenuEntry::AddChildToStandard(objHndl handle, RadialMenuStandardNode stdNode)
//{
//	int parentNode = radialMenus.GetStandardNode(stdNode);
//	return radialMenus.AddChildNode(handle, this, parentNode);
//}

RadialMenuEntryParent::RadialMenuEntryParent(int combatMesLine):RadialMenuEntry() {
	type = RadialMenuEntryType::Parent;
	text = combatSys.GetCombatMesLine(combatMesLine);
}

RadialMenuEntryParent::RadialMenuEntryParent(std::string & textArg) : RadialMenuEntryParent(1)
{
	auto textId = ElfHash::Hash(textArg);
	auto textCache = radialMenus.radMenuStrings.find(textId);
	if (textCache == radialMenus.radMenuStrings.end()) {
		radialMenus.radMenuStrings[textId] = textArg;
	}

	this->text = (char*)radialMenus.radMenuStrings[textId].c_str();
}


int RadialMenuEntryParent::AddChildToStandard(objHndl handle, RadialMenuStandardNode stdNode) {
	int parentNode = radialMenus.GetStandardNode(static_cast<RadialMenuStandardNode>(stdNode));
	return radialMenus.AddParentChildNode(handle, this, parentNode);
}

int RadialMenuEntryParent::AddAsChild(objHndl handle, int parentId) {
	return radialMenus.AddParentChildNode(handle, this, parentId);
}

RadialMenuEntryPythonAction::RadialMenuEntryPythonAction(int combatMesLine, int d20aType, int d20aKey, int data1, const char *helpId): RadialMenuEntryAction(1, d20aType, data1, helpId) {
	if (combatMesLine == -1){
		this->text = (char*)(d20Sys.GetPythonActionName((D20DispatcherKey)d20aKey).c_str());
	} else
	{
		this->text = combatSys.GetCombatMesLine(combatMesLine);
	}
	this->dispKey = d20aKey;
	this->callback = [](objHndl handle, RadialMenuEntry* entry) { return radialMenus.PythonActionCallback(handle, entry); };
}

RadialMenuEntryPythonAction::RadialMenuEntryPythonAction(int combatMesLine, int d20aType, const char *d20aKey, int data1, const char *helpId):RadialMenuEntryAction(combatMesLine, d20aType, data1, helpId) {
	this->dispKey = ElfHash::Hash(d20aKey);
	this->callback = [](objHndl handle, RadialMenuEntry* entry) { return radialMenus.PythonActionCallback(handle, entry); };
}

RadialMenuEntryPythonAction::RadialMenuEntryPythonAction(std::string & textArg, int d20aType, int d20aKey, int data1, const char *helpId) : RadialMenuEntryAction(1, d20aType, data1, helpId)
{
	auto textId = ElfHash::Hash(textArg);
	auto textCache = radialMenus.radMenuStrings.find(textId);
	if (textCache == radialMenus.radMenuStrings.end()) {
		radialMenus.radMenuStrings[textId] = textArg;
	}

	this->text = (char*)radialMenus.radMenuStrings[textId].c_str();
	this->helpId = ElfHash::Hash(helpId);
	d20ActionType = static_cast<D20ActionType>(d20aType);
	d20ActionData1 = data1;
	this->callback = [](objHndl handle, RadialMenuEntry* entry) { return radialMenus.PythonActionCallback(handle, entry); };
	this->dispKey = d20aKey;
}

RadialMenuEntryPythonAction::RadialMenuEntryPythonAction(SpellStoreData & spData, int d20aType, int d20aKey, int data1, const char *helpId):RadialMenuEntryAction(spData){
	this->d20ActionType = (D20ActionType)d20aType;
	if (helpId && *helpId){
		this->helpId = ElfHash::Hash(helpId);
	}
	this->d20ActionData1 = data1;
	this->dispKey = d20aKey;
	this->callback = [](objHndl handle, RadialMenuEntry* entry) { return radialMenus.PythonActionCallback(handle, entry); };
}
