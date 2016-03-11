#include "stdafx.h"
#include "common.h"
#include "config/config.h"
#include "d20.h"
#include "util/fixes.h"
#include "ui/ui.h"
#include "tig/tig_msg.h"
#include <infrastructure/keyboard.h>
#include <party.h>
#include <gamesystems/objects/objsystem.h>
#include <tig/tig_font.h>
#include "ui_render.h"
#include <critter.h>
#include <d20_level.h>
#include <gamesystems/gamesystems.h>

#define NUM_SPELLBOOK_SLOTS 18 // 18 in vanilla


struct TigTextStyle;

struct SpellList
{
	SpellStoreData spells[SPELL_ENUM_MAX];
	uint32_t count;
};

struct UiCharSpellPacket
{
	WidgetType1 * classSpellbookWnd;
	WidgetType1 * classMemorizeWnd;
	WidgetType1 * spellbookSpellWnds[NUM_SPELLBOOK_SLOTS];
	WidgetType1 * memorizeSpellWnds[NUM_SPELLBOOK_SLOTS];
	WidgetType2 * metamagicButtons[NUM_SPELLBOOK_SLOTS];
	WidgetType3 * spellbookScrollbar;
	WidgetType3 * memorizeScrollbar;
	SpellList spellsKnown;
	SpellList spellsMemorized;
};
static_assert(sizeof(UiCharSpellPacket) == 0xC970, "UiCharSpellPacket should have size 51568 "); //  51568

struct UiCharSpellsNavPacket
{
	uint32_t spellClassCode;
	WidgetType2* button;
	int numSpellsForLvl[NUM_SPELL_LEVELS]; // starting from level 0 (cantrips), and including bonus from ability modifier and school specialization
};

static_assert(sizeof(UiCharSpellsNavPacket) == 0x30, "UiCharSpellPacket should have size 48"); //  48

struct UiCharAddresses : temple::AddressTable
{
	UiCharSpellsNavPacket *uiCharSpellsNavPackets;
	UiCharSpellPacket * uiCharSpellPackets; // size
	int * uiCharSpellsNavClassTabIdx;
	WidgetType1** uiCharSpellsMainWnd;
	WidgetType1** uiCharSpellsNavClassTabWnd;
	WidgetType1** uiCharSpellsSpellsPerDayWnd;
	WidgetType2** uiCharSpellsPerDayLevelBtns; // array of 6 buttons for levels 0-5
	UiCharAddresses()
	{
		rebase(uiCharSpellsMainWnd, 0x10C81BC0);
		rebase(uiCharSpellsNavClassTabWnd, 0x10C81BC4);
		rebase(uiCharSpellsSpellsPerDayWnd, 0x10C81BC8);
		rebase(uiCharSpellsPerDayLevelBtns, 0x10C81BCC);
		rebase(uiCharSpellsNavPackets, 0x10C81BE4);
		rebase(uiCharSpellPackets, 0x10C81E24);
		rebase(uiCharSpellsNavClassTabIdx, 0x10D18F68);
	}
} addresses;

class CharUiSystem : TempleFix
{
public: 
	const char* name() override { return "CharUi - disabling stat text draw calls";} 

	static BOOL MemorizeSpellMsg(int widId, TigMsg* tigMsg)
	{
		if (tigMsg->type == TigMsgType::WIDGET)
		{
			int dumy = 1;

			if (tigMsg->arg2 == 0)
			{
				dumy = 1;
			}

			if (tigMsg->arg2 == 1)
			{
				dumy = 1;
			}

			if (tigMsg->arg2 == 2)
			{
				dumy = 1;
			}
			if (tigMsg->arg2 == 3)
			{
				dumy = 1;
			}
			if (tigMsg->arg2 == 4)
			{
				dumy = 1;
			}
		}
		return orgMemorizeSpellMsg(widId, tigMsg);
	};
	static BOOL (*orgMemorizeSpellMsg)(int widId, TigMsg* tigMsg);

	static BOOL SpellbookSpellsMsg(int widId, TigMsg* tigMsg)
	{
		if (tigMsg->type == TigMsgType::WIDGET)
		{
			int dumy = 1;
			if (tigMsg->arg2 == 0)
			{
				dumy = 1;
			}

			if (tigMsg->arg2 == 1)
			{
				dumy = 1;
			}

			if (tigMsg->arg2 == 2)
			{
				dumy = 1;
			}
			if (tigMsg->arg2 == 3)
			{
				dumy = 1;
			}
			if (tigMsg->arg2 == 4)
			{
				dumy = 1;
			}
		}
		else if (tigMsg->type == TigMsgType::MOUSE)
		{
			int dummy = 1;
			if (tigMsg->arg4 & MSF_RMB_RELEASED)
			{
				auto shit = &addresses.uiCharSpellPackets[0];
				dummy = 1;
			}
			else if (tigMsg->arg4 & MSF_UNK_4000)
			{
				dummy = 1;
			}
		}
		return orgSpellbookSpellsMsg(widId, tigMsg);
	};
	static BOOL(*orgSpellbookSpellsMsg)(int widId, TigMsg* tigMsg);


	static int specSlotIndices[NUM_SPELL_LEVELS]; // indices of the Specialization School spell slots in the GUI 
	static void SpellsShow(objHndl obj);
	static void(*orgSpellsShow)(objHndl obj);
	static BOOL IsSpecializationSchoolSlot(int idx);
	static int HookedCharSpellGetSpellbookScrollbarY();


	static objHndl GetCritterLooted(); // this may be a substitute inventory object when buying from NPCs with shops
	static objHndl GetVendor();

	static int GetInventoryState(); // 1 - looting, 2 - bartering, 4 - using magic on inventory item
	static int InventorySlotMsg(int widId, TigMsg* msg);
	static BOOL (*orgInventorySlotMsg)(int widId, TigMsg* msg);

	void apply() override 
	{
		if (config.usingCo8) // disabling stat text draw calls
		{
			writeHex(0x1011DD4D, "90 90 90 90 90");
		}
		int charSheetAttackCodeForAttackBonusDisplay = 1 + ATTACK_CODE_OFFHAND;
		write(0x101C45F3 + 7, &charSheetAttackCodeForAttackBonusDisplay, sizeof(int));
		write(0x101C8C7B + 4, &charSheetAttackCodeForAttackBonusDisplay, sizeof(int));

		charSheetAttackCodeForAttackBonusDisplay = 1 + ATTACK_CODE_OFFHAND + 1; //the offhand
		write(0x101C491B + 7, &charSheetAttackCodeForAttackBonusDisplay, sizeof(int));
		write(0x101C8D74 + 4, &charSheetAttackCodeForAttackBonusDisplay, sizeof(int));
		
		//orgSpellbookSpellsMsg = replaceFunction(0x101B8F10, SpellbookSpellsMsg);
		//orgMemorizeSpellMsg= replaceFunction(   0x101B9360, MemorizeSpellMsg);
		replaceFunction(0x101B2EE0, IsSpecializationSchoolSlot);
		orgSpellsShow = replaceFunction(0x101B5D80, SpellsShow);
		
		// UiCharSpellGetScrollbarY  has bug when called from Spellbook, it receives the first tab's scrollbar always
		writeCall(0x101BA5D9, HookedCharSpellGetSpellbookScrollbarY);
		

		static bool (__cdecl* orgUiCharLootingLootWndMsg)(int , TigMsg* ) = replaceFunction<bool(__cdecl)(int , TigMsg* ) >(0x101406D0, [](int widId, TigMsg* msg)
		{
			if (msg->type == TigMsgType::MOUSE)
			{
				int dummy = 1;
			}

			if (msg->type == TigMsgType::WIDGET)
			{
				int dummy = 1;
			}
			return orgUiCharLootingLootWndMsg(widId, msg);
		});

		orgInventorySlotMsg = replaceFunction<int(__cdecl)(int, TigMsg*) >(0x10157DC0, InventorySlotMsg);
		


	}
} charUiSys;

void CharUiSystem::SpellsShow(objHndl obj)
{

	//orgSpellsShow(obj);
	//return;
	
	auto dude = temple::GetRef<objHndl>(0x10BE9940); // critter with inventory open

	auto navClassPackets = addresses.uiCharSpellsNavPackets;
	auto charSpellPackets = addresses.uiCharSpellPackets;

	auto spellsMainWnd = temple::GetRef<WidgetType1*>(0x10C81BC0);
	auto spellsMainWndId = spellsMainWnd->widgetId;

	auto& uiCharSpellFeat = temple::GetRef<int>(0x10300258);
	uiCharSpellFeat = 649;
	auto& numSpecializationSlots = temple::GetRef<int>(0x10D18F64);
	numSpecializationSlots = 0;
	// auto& specSlotIndices = temple::GetRef<int[10]>(0x10C8159C); // now contained in Temple+

	auto& uiCharSpellTabsCount = temple::GetRef<int>(0x10D18F6C);
	uiCharSpellTabsCount = 0;
	auto& uiCharSpellsNavClassTabIdx = temple::GetRef<int>(0x10D18F68);
	uiCharSpellsNavClassTabIdx = 0;

	auto insertToSpellList = [](int idx, SpellStoreData& source, SpellList& spArray) {
		auto spCount = spArray.count;

		if (idx<0 || idx > spCount) {
			if (idx == spCount + 1)
			{
				spArray.spells[idx] = source;
				spArray.count++;
			}
			return 0;
		}

		for (int it = spCount; it > idx; it--) {
			spArray.spells[it] = spArray.spells[it - 1];
		}
		spArray.spells[idx] = source;
		++spArray.count;
		return 0;

	};

	ui.WidgetSetHidden(spellsMainWndId, 0);
	ui.WidgetSetHidden((*addresses.uiCharSpellsNavClassTabWnd)->widgetId, 0);
	ui.WidgetSetHidden((*addresses.uiCharSpellsSpellsPerDayWnd)->widgetId, 0);
	ui.WidgetBringToFront(spellsMainWndId);
	ui.WidgetBringToFront((*addresses.uiCharSpellsSpellsPerDayWnd)->widgetId);

	UiRenderer::PushFont(PredefinedFont::ARIAL_12);

	TigTextStyle style;
	TigFontMetrics metric;
	style.tracking = 2;

	MesHandle spellsUiText = temple::GetRef<MesHandle>(0x10C81590);
	MesLine mesline;

	int navTabX = 0;
	std::string line;

	temple::GetRef<0x10D18F70, int>() = 1;
	temple::GetRef<0x10C816CC, int>() = -1;


	auto uiCharSpellTabPadding = temple::GetRef<int>(0x10C81B40);

	auto getStatName = temple::GetRef<const char*(__cdecl)(Stat)>(0x10074950);

	// find which caster class tabs should appear
	for (int i = 0; i < NUM_CLASSES ; i++){
		auto classCode = d20ClassSys.classEnums[i];
		auto spellClassCode = (classCode & 0x7F) | 0x80;

		if (!d20ClassSys.IsCastingClass(classCode))
			continue;

		auto classLvl = objects.StatLevelGet(dude, classCode) ;
		if (classLvl <= 0)
			continue;

		line = getStatName(classCode);
		auto textSize = UiRenderer::MeasureTextSize(line, style);

		auto navTabBtn = navClassPackets[uiCharSpellTabsCount].button;
		navTabBtn->width = textSize.width + 2 * uiCharSpellTabPadding;
		navTabBtn->x += navTabX;
		navTabX += navTabBtn->width ;
		ui.WidgetSetHidden(navTabBtn->widgetId, 0);
		
		navClassPackets[uiCharSpellTabsCount].spellClassCode = spellClassCode;

		int n1 = 0;
		for (int spellLvl = 0; spellLvl < 10; spellLvl++){
			int numSpells = d20ClassSys.GetNumSpellsFromClass(dude, classCode, spellLvl, classLvl);
			if (numSpells > 0)
				n1 += numSpells;
		}
		int nMemo = critterSys.SpellNumByFieldAndClass(dude, obj_f_critter_spells_memorized_idx, spellClassCode);
		if (n1 > nMemo)
			nMemo = n1;

		int nKnown = max(0, critterSys.SpellNumByFieldAndClass(dude, obj_f_critter_spells_known_idx, spellClassCode)-2);
		
		ui.ScrollbarSetYmax(charSpellPackets[uiCharSpellTabsCount].spellbookScrollbar->widgetId, nKnown);
		ui.ScrollbarSetYmax(charSpellPackets[uiCharSpellTabsCount].memorizeScrollbar->widgetId, nMemo-2 <=0 ? 0 : nMemo-2);

		uiCharSpellTabsCount++;

	}

	// domain
	auto clericLvl = objects.StatLevelGet(dude, stat_level_cleric);
	if ( clericLvl > 0) {
		mesline.key = 2;
		mesFuncs.GetLine_Safe(spellsUiText, &mesline);
		line = mesline.value;
		auto textSize = UiRenderer::MeasureTextSize(line, style);

		auto domainTabBtn = navClassPackets[uiCharSpellTabsCount].button;
		domainTabBtn->width = textSize.width + 2 * uiCharSpellTabPadding;
		domainTabBtn->x += navTabX;
		navTabX += textSize.width;
		ui.WidgetSetHidden(domainTabBtn->widgetId, 0);
		navClassPackets[uiCharSpellTabsCount].spellClassCode = Domain_Count;


		int n1 = d20ClassSys.NumDomainSpellsKnownFromClass(dude, stat_level_cleric);
		int nMemo = critterSys.DomainSpellNumByField(dude, obj_f_critter_spells_memorized_idx);
		if (n1 > nMemo)
			nMemo = n1;

		int nKnown = max(0, critterSys.DomainSpellNumByField(dude, obj_f_critter_spells_known_idx) - 2);


		ui.ScrollbarSetYmax(charSpellPackets[uiCharSpellTabsCount].spellbookScrollbar->widgetId, nKnown);
		ui.ScrollbarSetYmax(charSpellPackets[uiCharSpellTabsCount].memorizeScrollbar->widgetId, nMemo - 2 <= 0 ? 0 : nMemo - 2);

		uiCharSpellTabsCount++;
	}

	// show the first class's spellbook / spells memorized
	if (uiCharSpellTabsCount > 0 && !spellSys.isDomainSpell(navClassPackets[0].spellClassCode))	{
		auto classCode = spellSys.GetCastingClass(navClassPackets[0].spellClassCode );
		if (d20ClassSys.isVancianCastingClass(classCode)){
			ui.WidgetSetHidden(charSpellPackets[0].classMemorizeWnd->widgetId, 0);
		}
		ui.WidgetSetHidden(charSpellPackets[0].classSpellbookWnd->widgetId, 0);
		ui.WidgetBringToFront(charSpellPackets[0].classSpellbookWnd->widgetId);
		ui.WidgetBringToFront(charSpellPackets[0].classMemorizeWnd->widgetId);

	}

	// hide the other spellbook windows
	for (int i = 1; i < uiCharSpellTabsCount; i++){
		ui.WidgetSetHidden(charSpellPackets[i].classSpellbookWnd->widgetId, 1);
		ui.WidgetSetHidden(charSpellPackets[i].classMemorizeWnd->widgetId, 1);
	}

	// hide inactive nav tabs
	for (int i = uiCharSpellTabsCount; i < NUM_CLASSES + 1; i++){
		ui.WidgetSetHidden(navClassPackets[i].button->widgetId, 1);
	}

	// if no tabs, return
	if (!uiCharSpellTabsCount)
	{
		UiRenderer::PopFont();
		ui.WidgetBringToFront((*addresses.uiCharSpellsNavClassTabWnd)->widgetId);
		return;
	}

	for (int i = 0; i < uiCharSpellTabsCount; i++)	{
		auto spellClassCode = navClassPackets[i].spellClassCode;
		

		// fill out the number of spells per level for each NavPacket

		if (!spellSys.isDomainSpell(spellClassCode)){ //normal casting class
			LevelPacket lvlPkt;
			auto classCode = spellSys.GetCastingClass(spellClassCode);
			auto classLvl = objects.StatLevelGet(dude, classCode);
			lvlPkt.GetLevelPacket(classCode, dude, 0, classLvl);
			auto numSpellsForLvl = navClassPackets[i].numSpellsForLvl;
			for (int j = 0; j < NUM_SPELL_LEVELS; j++){
				if (lvlPkt.spellCountFromClass[j] > 0){
					numSpellsForLvl[j] = lvlPkt.spellCountBonusFromStatMod[j] + lvlPkt.spellCountFromClass[j];
					if (numSpellsForLvl[j] > 0 && classCode == stat_level_wizard) {
						if (spellSys.getWizSchool(dude))
							++numSpellsForLvl[j];
					}
				}
				else
					numSpellsForLvl[j] = 0;
			}
		} 
		else if(clericLvl > 0) // domain spell
		{
			auto maxClericSpellLvl = d20ClassSys.ClericMaxSpellLvl(clericLvl);
			if (maxClericSpellLvl >= 1)	{
				for (int j = 1; j <= maxClericSpellLvl; j++) {
					navClassPackets[i].numSpellsForLvl[j] = 1;
				}
			}	
		}


		// fill out the spells per each level
		auto spellsKnownArray = gameSystems->GetObj().GetObject(dude)->GetSpellArray(obj_f_critter_spells_known_idx);
		auto spellsKnownArraySize = spellsKnownArray.GetSize();
		std::vector<SpellStoreData> spellsKnownVector;
		for (int j = 0; j < spellsKnownArraySize; j++){
			SpellStoreData spStore = spellsKnownArray[j];
			if (spellSys.isDomainSpell(spStore.classCode)) {
				// domain spell
				if (navClassPackets[i].spellClassCode != Domain_Count)
					continue;
			}
			else if (!spellSys.isDomainSpell(navClassPackets[i].spellClassCode)) {
				if (navClassPackets[i].spellClassCode != spellsKnownArray[j].classCode)
					continue;
			}
			else
				continue;
			spellsKnownVector.push_back(spellsKnownArray[j]);
		}
		std::sort(spellsKnownVector.begin(),spellsKnownVector.end(), [](const SpellStoreData& sp1, const SpellStoreData& sp2)->bool
		{
			int levelDelta = sp1.spellLevel - sp2.spellLevel;
			if (levelDelta < 0)
				return true;
			else if (levelDelta > 0)
				return false;

			// if levels are equal
			auto name1 = spellSys.GetSpellName(sp1.spellEnum);
			auto name2 = spellSys.GetSpellName(sp2.spellEnum);
			auto nameCmp = _strcmpi(name1, name2);
			return nameCmp < 0;
		});
		
		for (auto it: spellsKnownVector){
			charSpellPackets[i].spellsKnown.spells[charSpellPackets[i].spellsKnown.count++] = it;
		}

		// insert "Spell Level #" labels
		for (int spellLvl = 0; spellLvl < NUM_SPELL_LEVELS; spellLvl++){
			auto spellKnNum = charSpellPackets[i].spellsKnown.count;
			
			SpellStoreData spStore;
			memset(&spStore, 0, sizeof SpellStoreData);
			spStore.spellLevel = spellLvl;

			int spFound = -1;
			for (int j = 0; j < spellKnNum; j++) {
				if (charSpellPackets[i].spellsKnown.spells[j].spellLevel == spellLvl) {
					spFound = j;
					break;
				}
			}
			// none found, find the next higher level
			if (spFound == -1){
				int j = spellKnNum;
				spFound = spellKnNum;
				while (j >= 0){
					if (charSpellPackets[i].spellsKnown.spells[j].spellLevel <= spellLvl){
						spFound = j;
					}
					j--;
				}
				if (spFound == spellKnNum || (spFound == 0 && spellLvl != 0))
					spFound = -1;
			}

			insertToSpellList(spFound, spStore, charSpellPackets[i].spellsKnown);
		}


		// fill out the spells memorized
		auto spellsMemoArray = gameSystems->GetObj().GetObject(dude)->GetSpellArray(obj_f_critter_spells_memorized_idx);
		auto numSpMemo = spellsMemoArray.GetSize();

		for (int classLvl = 0; classLvl < d20ClassSys.ClassLevelMax; classLvl++) {
			for (int j = 0; j < numSpMemo; j++)	{
				SpellStoreData spStore = spellsMemoArray[j];
				// normal spells
				if (!spellSys.isDomainSpell(spStore.classCode) && !spellSys.isDomainSpell(navClassPackets[i].spellClassCode)){
					auto casterLvl = spellsMemoArray[j].spellLevel * 2;
					if (spStore.pad0 & 0x80)
						casterLvl += 1;
					if (casterLvl != classLvl)
						continue;
				} 
				else if (spellSys.isDomainSpell(spStore.classCode) && spellSys.isDomainSpell(navClassPackets[i].spellClassCode)) {//domain
					auto casterLvl = spStore.spellLevel * 2;
					if (spStore.pad0 & 0x80)
						casterLvl += 1;
					if (casterLvl != classLvl)
						continue;
				}
				else 
					continue;
				auto spellMemNum = charSpellPackets[i].spellsMemorized.count;
				charSpellPackets[i].spellsMemorized.spells[spellMemNum] = spStore;
				++charSpellPackets[i].spellsMemorized.count;
			}
		}

		// create the blank slots for the rest
		for (int spellLvl = 0; spellLvl < NUM_SPELL_LEVELS; spellLvl++) {
			SpellStoreData spStore;
			auto numSpellsForLvl = navClassPackets[i].numSpellsForLvl[spellLvl];
			auto spMemNum = charSpellPackets[i].spellsMemorized.count;
			
			

			int spFound = 0;
			for (int j = 0; j < spMemNum; j++) {
				if (charSpellPackets[i].spellsMemorized.spells[j].spellLevel >= spellLvl) {
					break;
				}
				spFound++;
			}
			auto spFound2 = spFound;
			auto numEmptySlotsForLvl = 0;
			while (spFound < spMemNum )	{
				auto spFoundLvl = charSpellPackets[i].spellsMemorized.spells[spFound].spellLevel;
				if (charSpellPackets[i].spellsMemorized.spells[spFound].pad0 & 0x80)
					spFoundLvl++;
				if (spFoundLvl > spellLvl)
					break;
				spFound++;
				numEmptySlotsForLvl++;
			}

			for (int j = 0; j < numSpellsForLvl - numEmptySlotsForLvl; j++) {
				spStore.spellLevel = spellLvl;
				spStore.spellEnum = -1;
				spStore.metaMagicData = 0;
				spStore.spellStoreState.usedUp = 0;
				spStore.spellStoreState.spellStoreType = SpellStoreType::spellStoreNone;
				spStore.classCode = navClassPackets[i].spellClassCode;
				insertToSpellList(spFound, spStore, charSpellPackets[i].spellsMemorized);
			}
		}

		// insert "Spell Level #" labels
		for (int spellLvl = 0; spellLvl < NUM_SPELL_LEVELS; spellLvl++)	{
			SpellStoreData spStore;
			memset(&spStore, 0, sizeof SpellStoreData);
			spStore.spellLevel = spellLvl;
			spStore.spellEnum = 0;
			int spFound = -1;
			for (int j = 0; j < charSpellPackets[i].spellsMemorized.count; j++)	{
				if (charSpellPackets[i].spellsMemorized.spells[j].spellLevel == spellLvl)
				{
					insertToSpellList(j, spStore, charSpellPackets[i].spellsMemorized);
					break;
				}
			}
		}

		// mark the school specialization slots
		if (!spellSys.isDomainSpell(spellClassCode) && spellSys.GetCastingClass(spellClassCode) == stat_level_wizard && spellSys.getWizSchool(dude)){
			int slotIdx = 0;
			for (int spellLvl = 0; spellLvl < NUM_SPELL_LEVELS; spellLvl++)
			{
				slotIdx += navClassPackets[i].numSpellsForLvl[spellLvl];
				specSlotIndices[numSpecializationSlots++] = slotIdx;
				slotIdx += 1; // is without the +1 for the first one
			}
		}

	}

	// hide the Spellbook slots without spells
	auto& curCharSpellPkt = charSpellPackets[uiCharSpellsNavClassTabIdx];
	for (int i = 0; i < NUM_SPELLBOOK_SLOTS; i++){
		if (i < curCharSpellPkt.spellsKnown.count){
			ui.WidgetSetHidden(curCharSpellPkt.spellbookSpellWnds[i]->widgetId, 0);
			ui.WidgetBringToFront(curCharSpellPkt.spellbookSpellWnds[i]->widgetId);

		} else
		{
			ui.WidgetSetHidden(curCharSpellPkt.spellbookSpellWnds[i]->widgetId, 1);
		}
	}

	// hide memorized slots without spells
	bool showMemSpells = true;
	auto spellClassCode = navClassPackets[uiCharSpellsNavClassTabIdx].spellClassCode;
	if (!spellSys.isDomainSpell(spellClassCode)){
		auto casterClassCode = spellSys.GetCastingClass(spellClassCode);
		if (d20ClassSys.isNaturalCastingClass(casterClassCode))
			showMemSpells = false;
	}
	
	for (int i = 0; i < NUM_SPELLBOOK_SLOTS; i++) {
		if (showMemSpells && i < curCharSpellPkt.spellsMemorized.count) {
			ui.WidgetSetHidden(curCharSpellPkt.memorizeSpellWnds[i]->widgetId, 0);
			ui.WidgetBringToFront(curCharSpellPkt.memorizeSpellWnds[i]->widgetId);
		}
		else
		{
			ui.WidgetSetHidden(curCharSpellPkt.memorizeSpellWnds[i]->widgetId, 1);
		}
	}
	

		//UiRenderer::RenderText(line, rect, mImpl->style);


	



	

	UiRenderer::PopFont();

	ui.WidgetBringToFront((*addresses.uiCharSpellsNavClassTabWnd)->widgetId);


	auto charSpellPkts = &addresses.uiCharSpellPackets[0];
	
}

BOOL CharUiSystem::IsSpecializationSchoolSlot(int idx)
{
	auto numSpecSlots = temple::GetRef<int>(0x10D18F64);
	for (int i = 0; i < numSpecSlots;i++)
	{
		if (specSlotIndices[i] == idx)
			return 1;
	}
	return 0;
}

int CharUiSystem::HookedCharSpellGetSpellbookScrollbarY()
{
	auto tabIdx = *addresses.uiCharSpellsNavClassTabIdx;
	auto scrollbar = addresses.uiCharSpellPackets[tabIdx].spellbookScrollbar;
	if (scrollbar) {
		return (scrollbar->GetY());
	}
	logger->warn("Null scrollbar! Returning y=0");
	return 0;
}

objHndl CharUiSystem::GetCritterLooted()
{
	return temple::GetRef<objHndl>(0x10BE6EC0);
}

objHndl CharUiSystem::GetVendor()
{
	return temple::GetRef<objHndl>(0x10BE6EC8);
}

int CharUiSystem::GetInventoryState()
{
	return temple::GetRef<int>(0x10BE994C);
}

int CharUiSystem::InventorySlotMsg(int widId, TigMsg* msg)
{
	if (ui.CharLootingIsActive())
	{
		if (msg->type == TigMsgType::MOUSE)
		{
			if (msg->arg4 & MSF_LMB_CLICK)
			{
				if ( infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) )
				{
					objHndl critterLooted = GetCritterLooted(); // may be substitute inventory object (i.e. a container)

					auto uiCharInvItemGetFromWidId = temple::GetRef<objHndl(__cdecl)(int)>(0x10157060);
					auto item = uiCharInvItemGetFromWidId(widId);
					objHndl vendor = GetVendor();

					logger->debug("Quickselling item {} to {} ({})", description.getDisplayName(item), description.getDisplayName(vendor),description.getDisplayName(critterLooted));
					
					objHndl appraiser = party.PartyMemberWithHighestSkill(SkillEnum::skill_appraise);
					
					
					int appraisedWorth = inventory.GetAppraisedWorth(item, appraiser, vendor, SkillEnum::skill_appraise);
					int plat =0, gold=0, silver=0, copper=0;
					if (appraisedWorth && GetInventoryState() == 2){
						
						int qty = max(1, inventory.GetQuantity(item));
						appraisedWorth *= qty;
						inventory.MoneyToCoins(appraisedWorth, &plat, &gold, &silver, &copper);
					}
					
					ItemErrorCode itemTransferError = inventory.TransferWithFlags(item, critterLooted, -1, 1+2+4+8+16, 0i64);
					if (itemTransferError == IEC_OK && GetInventoryState() == 2){
						objSystem->GetObject(item)->SetItemFlag(OIF_IDENTIFIED, 1);
						party.MoneyAdj(plat, gold, silver, copper);
					} 
					else if (itemTransferError == IEC_No_Room_For_Item){
						int itemIdx = 100;
						for (itemIdx = 100; itemIdx< 255; itemIdx++){
							if (!inventory.GetItemAtInvIdx(critterLooted, itemIdx))
								break;
						}
						if (itemIdx < 200)
							itemTransferError = inventory.TransferWithFlags(item, critterLooted, itemIdx, 1 + 2 + 4 + 8 , 0i64);
						if (itemTransferError == IEC_OK && GetInventoryState() == 2) {
							objSystem->GetObject(item)->SetItemFlag(OIF_IDENTIFIED, 1);
							party.MoneyAdj(plat, gold, silver, copper);
						}
					}
					return 1;
				}
				
			}
		}
	}


	if (msg->type == TigMsgType::WIDGET)
	{
		int dummy = 1;
	}
	return orgInventorySlotMsg(widId, msg);
}

BOOL(*CharUiSystem::orgSpellbookSpellsMsg)(int widId, TigMsg* tigMsg);
BOOL(*CharUiSystem::orgMemorizeSpellMsg)(int widId, TigMsg* tigMsg);
void(*CharUiSystem::orgSpellsShow)(objHndl obj);
BOOL(*CharUiSystem::orgInventorySlotMsg)(int widId, TigMsg* msg);
int CharUiSystem::specSlotIndices[10];