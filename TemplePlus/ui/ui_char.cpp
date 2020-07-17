#include "stdafx.h"
#include "ui_char.h"
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
#include "tig/tig_font.h"
#include "tig/tig_startup.h"
#include "fonts/fonts.h"
#include "ui_tooltip.h"
#include "ui_assets.h"
#include <gamesystems/d20/d20stats.h>

#include "ui/ui_legacysystems.h"
#include "ui/ui_systems.h"
#include "d20_race.h"
#include "gamesystems/d20/d20_help.h"
#include "objlist.h"
#include "pathfinding.h"

#define NUM_SPELLBOOK_SLOTS 18 // 18 in vanilla

constexpr int WEAP_COMBO_MAIN = 1;
constexpr int WEAP_COMBO_SECONDARY = 2;
constexpr int WEAP_COMBO_AMMO = 3;
constexpr int WEAP_COMBO_SHIELD = 4;

struct TigTextStyle;

struct SpellList
{
	SpellStoreData spells[SPELL_ENUM_MAX_VANILLA];
	uint32_t count;
};

struct UiCharSpellPacket
{
	LgcyWindow * classSpellbookWnd;
	LgcyWindow * classMemorizeWnd;
	LgcyWindow * spellbookSpellWnds[NUM_SPELLBOOK_SLOTS];
	LgcyWindow * memorizeSpellWnds[NUM_SPELLBOOK_SLOTS];
	LgcyButton * metamagicButtons[NUM_SPELLBOOK_SLOTS];
	LgcyScrollBar * spellbookScrollbar;
	LgcyScrollBar * memorizeScrollbar;
	SpellList spellsKnown;
	SpellList spellsMemorized;
};
static_assert(sizeof(UiCharSpellPacket) == 0xC970, "UiCharSpellPacket should have size 51568 "); //  51568

struct UiCharSpellsNavPacket
{
	uint32_t spellClassCode;
	LgcyButton* button;
	int numSpellsForLvl[NUM_SPELL_LEVELS]; // starting from level 0 (cantrips), and including bonus from ability modifier and school specialization
};

static_assert(sizeof(UiCharSpellsNavPacket) == 0x30, "UiCharSpellPacket should have size 48"); //  48

struct UiCharAddresses : temple::AddressTable
{
	UiCharSpellsNavPacket *uiCharSpellsNavPackets; // size 12 array
	UiCharSpellPacket * uiCharSpellPackets; // size 12 array
	int * uiCharSpellsNavClassTabIdx;
	LgcyWindow** uiCharSpellsMainWnd;
	LgcyWindow** uiCharSpellsNavClassTabWnd;
	LgcyWindow** uiCharSpellsSpellsPerDayWnd;
	LgcyWindow** uiCharSpellsPerDayLevelBtns; // array of 6 buttons for levels 0-5

	int * wndDisplayed; // 0 thru 4 - inventories (1-4 were meant for the various "bags"); 5 - Skills; 6 - Feats ; 7 - Spells

	UiCharAddresses()
	{
		rebase(uiCharSpellsMainWnd, 0x10C81BC0);
		rebase(uiCharSpellsNavClassTabWnd, 0x10C81BC4);
		rebase(uiCharSpellsSpellsPerDayWnd, 0x10C81BC8);
		rebase(uiCharSpellsPerDayLevelBtns, 0x10C81BCC);
		rebase(uiCharSpellsNavPackets, 0x10C81BE4);
		rebase(uiCharSpellPackets, 0x10C81E24);
		rebase(uiCharSpellsNavClassTabIdx, 0x10D18F68);
		rebase(wndDisplayed, 0x10BE9948);
	}
} addresses;



class CharUiSystem : TempleFix
{
public: 
	#pragma region Main Wnd
	static void ClassLevelBtnRender(int widId);
	static void AlignGenderRaceBtnRender(int widId);
	#pragma endregion 

#pragma region Stats Wnd
	static int StatsLvlBtnRenderHook(objHndl handle);
#pragma endregion

	#pragma region Spellbook functions
	static BOOL MemorizeSpellMsg(int widId, TigMsg* tigMsg);
	static BOOL(*orgMemorizeSpellMsg)(int widId, TigMsg* tigMsg);

	static bool CharSpellsNavClassTabMsg(int widId, TigMsg* tigMsg);
	static bool(*orgCharSpellsNavClassTabMsg)(int widId, TigMsg* tigMsg);

	static BOOL SpellbookSpellsMsg(int widId, TigMsg* tigMsg){
		return orgSpellbookSpellsMsg(widId, tigMsg);
	};
	static BOOL(*orgSpellbookSpellsMsg)(int widId, TigMsg* tigMsg);

	static int specSlotIndices[NUM_SPELL_LEVELS]; // indices of the Specialization School spell slots in the GUI 
	static void SpellsShow(objHndl obj);
	static void(*orgSpellsShow)(objHndl obj);
	static BOOL IsSpecializationSchoolSlot(int idx);
	static int HookedCharSpellGetSpellbookScrollbarY();
	#pragma endregion 


	#pragma region Inventory (Looting / Bartering / Applying Spell (e.g. Read Magic))
	static objHndl GetCurrentCritter(); // gets a handle on the critter with inventory open
	static objHndl GetCritterLooted(); // this may be a substitute inventory object when buying from NPCs with shops
	static objHndl GetVendor();

	static int InventorySlotMsg(int widId, TigMsg* msg);
	static BOOL (*orgInventorySlotMsg)(int widId, TigMsg* msg);
	static char* HookedItemDescriptionBarter(objHndl obj, objHndl item);
	static void ItemGetDescrAddon(objHndl obj, objHndl item, std::string&); // makes an addon string for the item description boxes
	void LongDescriptionPopupCreate(objHndl item);

	static void TotalWeightOutputBtnTooltip(int x, int y, int *widId);

	static objHndl GetBag();
	static void WeaponComboActivate(objHndl handle, int weapomCombo);
#pragma endregion 
	

#pragma region Feats
	static void FeatsShow();
#pragma endregion 

	static const char *TabNameReplacement(int stat);

protected:
	void apply() override;

} charUiSys;

void CharUiSystem::ClassLevelBtnRender(int widId){
	auto btn = uiManager->GetButton(widId);
	const int maxWidth = 340;
	UiRenderer::PushFont(temple::GetRef<char*>(0x10BE93A4), temple::GetRef<int>(0x10BE93A0) );

	TigTextStyle style;
	auto txtR = temple::GetRef<int>(0x10BE9360);
	auto txtG = temple::GetRef<int>(0x10BE9364);
	auto txtB = temple::GetRef<int>(0x10BE9368);
	auto txtA = temple::GetRef<int>(0x10BE936C);

	ColorRect textColor( XMCOLOR((float)txtR, (float)txtG, (float)txtB, (float)txtA)  );
	ColorRect shadowColor(XMCOLOR(0, 0, 0, 255));
	style.textColor = &textColor;
	style.shadowColor = &shadowColor;
	style.flags = 8;
	style.kerning = 0;
	style.tracking = 2;

	auto dude = charUiSys.GetCurrentCritter();
	if (!dude)
		return;
	
	auto obj = gameSystems->GetObj().GetObject(dude);
	auto objType = obj->type;

	std::string text;
	static auto charUiTextMes = temple::GetRef<MesHandle>(0x10BE963C);
	if (objType == obj_t_pc || config.showNpcStats){
		// cycle through classes
		bool isFirst = true;
		for (auto classCode:d20ClassSys.classEnums){
			auto classLvl = objects.StatLevelGet(dude, (Stat)classCode);
			if (!classLvl)
				continue;
			
			if (!isFirst){ // add a "/" separator
				MesLine line; 
				mesFuncs.ReadLineDirect(charUiTextMes, 10, &line);
				text.append(fmt::format(" {} ", line.value));
			}
			else
				isFirst = false;

			MesLine line;
			mesFuncs.ReadLineDirect(charUiTextMes, 9, &line);
			text.append(fmt::format("{} {} {}", d20Stats.GetStatName((Stat)classCode), line.value, classLvl));
			
		}
	}
	else {
		MesLine line;
		mesFuncs.ReadLineDirect(charUiTextMes, 11, &line); // NPC
		text.append(fmt::format("({})", line.value));
	}

	auto textMeas = UiRenderer::MeasureTextSize(text, style);
	if (textMeas.width > maxWidth){ // get class shortnames
		text.clear();
		bool isFirst = true;
		for (auto classCode : d20ClassSys.classEnums) {
			auto classLvl = objects.StatLevelGet(dude, (Stat)classCode);
			if (!classLvl)
				continue;

			if (!isFirst) { // add a "/" separator
				MesLine line;
				mesFuncs.ReadLineDirect(charUiTextMes, 10, &line);
				text.append(fmt::format(" {} ", line.value));
			}
			else
				isFirst = false;

			text.append(fmt::format("{} {}", d20Stats.GetStatShortName((Stat)classCode), classLvl));

		}
	}
	textMeas = UiRenderer::MeasureTextSize(text, style);
	if (textMeas.width > maxWidth){ // if still too long, truncate
		style.flags |= 0x4000; // truncate
		textMeas = UiRenderer::MeasureTextSize(text, style, maxWidth);
	}
	auto x = btn->x + abs((int)btn->width - textMeas.width) / 2;
	if (textMeas.width > 290){
		x -= 20;
	}

	TigRect rect(x, btn->y, textMeas.width, textMeas.height);
	UiRenderer::RenderText(text, rect, style);
	UiRenderer::PopFont();
}

void CharUiSystem::AlignGenderRaceBtnRender(int widId){
	auto btn = uiManager->GetButton(widId);
	const int maxWidth = 200;
	UiRenderer::PushFont(temple::GetRef<char*>(0x10BE9394), temple::GetRef<int>(0x10BE9390));

	TigTextStyle style;
	auto txtR = temple::GetRef<int>(0x10BE9360);
	auto txtG = temple::GetRef<int>(0x10BE9364);
	auto txtB = temple::GetRef<int>(0x10BE9368);
	auto txtA = temple::GetRef<int>(0x10BE936C);

	ColorRect textColor(XMCOLOR((float)txtR, (float)txtG, (float)txtB, (float)txtA));
	ColorRect shadowColor(XMCOLOR(0, 0, 0, 255));
	style.textColor = &textColor;
	style.shadowColor = &shadowColor;
	style.flags = 8;
	style.kerning = 1;
	style.tracking = 2;

	auto dude = charUiSys.GetCurrentCritter();
	if (!dude)
		return;

	auto obj = gameSystems->GetObj().GetObject(dude);
	auto objType = obj->type;

	std::string text;
	static auto charUiTextMes = temple::GetRef<MesHandle>(0x10BE963C);

	// Alignment
	if (objType == obj_t_pc || config.showNpcStats){
		auto alignment = (Alignment)objects.StatLevelGet(dude, stat_alignment);
		auto alignmentName = d20Stats.GetAlignmentName(alignment);
		text.append(fmt::format("{} ", alignmentName));
	}
	// Subtype
	
	if (objType == obj_t_npc){
		auto isHuman = critterSys.IsCategorySubtype(dude, mc_subtype_human) && critterSys.IsCategoryType(dude, MonsterCategory::mc_type_humanoid);
		
		for (auto i = 0; (1 << i) <= MonsterSubcategoryFlag::mc_subtype_water; i += 1) {
			auto monSubcat = (MonsterSubcategoryFlag)(1 << i);
			if (monSubcat == mc_subtype_human && isHuman) continue; // skip silly string of "Human Humanoid"
			if (critterSys.IsCategorySubtype(dude, monSubcat)) {
				text.append(fmt::format("{} ", d20Stats.GetMonsterSubcategoryName(i)));
			}
		}
		
	}

	auto gender = objects.StatLevelGet(dude, stat_gender);
	auto genderName = d20Stats.GetGenderName(gender);

	if (objType == obj_t_pc) {
		auto race = critterSys.GetRace(dude, false); 
		auto raceName = d20Stats.GetRaceName(race);
		text.append(fmt::format("{} {}", genderName, raceName));
	}
	else {
		auto moncat = critterSys.GetCategory(dude);
		auto monsterCatName = d20Stats.GetMonsterCategoryName(moncat);

		text.append(fmt::format("{} {}", genderName, monsterCatName));

		//MesLine line;
		//mesFuncs.ReadLineDirect(charUiTextMes, 11, &line); // NPC
		//text.append(fmt::format("({})", line.value));
	}

	auto textMeas = UiRenderer::MeasureTextSize(text, style);
	if (textMeas.width > maxWidth) {
		style.flags |= 0x4000; // truncation
		textMeas = UiRenderer::MeasureTextSize(text, style, maxWidth);
	}

	auto x = btn->x + abs((int)btn->width - textMeas.width);
	/*if (textMeas.width > 290) {
		x -= 20;
	}
*/
	TigRect rect(x, btn->y, textMeas.width, textMeas.height);
	UiRenderer::RenderText(text, rect, style);
	UiRenderer::PopFont();
}

int CharUiSystem::StatsLvlBtnRenderHook(objHndl handle){
	return critterSys.GetEffectiveLevel(handle);
}

BOOL CharUiSystem::MemorizeSpellMsg(int widId, TigMsg* tigMsg){

	auto charSpellPackets = addresses.uiCharSpellPackets;
	auto& uiCharSpellsNavClassTabIdx = temple::GetRef<int>(0x10D18F68);

	auto &curSpellPacket = charSpellPackets[uiCharSpellsNavClassTabIdx];

	auto widIdx = WidgetIdIndexOf(widId, &curSpellPacket.memorizeSpellWnds[0], NUM_SPELLBOOK_SLOTS);
	auto scrollbarId = curSpellPacket.memorizeScrollbar->widgetId;
	auto scrollbar = uiManager->GetScrollBar(scrollbarId);
	auto scrollbarY = scrollbar?scrollbar->GetY(): 0;
	auto &spData = curSpellPacket.spellsMemorized.spells[widIdx + scrollbarY];

	if (spData.classCode == Domain::Domain_Special){
		if (tigMsg->type == TigMsgType::MOUSE){
			auto tigMouseMsg = (TigMsgMouse*)(tigMsg);
			if (tigMouseMsg->buttonStateFlags & MouseStateFlags::MSF_LMB_RELEASED){
				if (!helpSys.IsClickForHelpActive())
					return TRUE;
			}
			if (tigMouseMsg->buttonStateFlags & MouseStateFlags::MSF_LMB_CLICK) {
				if (!helpSys.IsClickForHelpActive())
					return TRUE;
			}
		}
		else if (tigMsg->type == TigMsgType::WIDGET){
			auto tigWidgetMsg = (TigMsgWidget*)(tigMsg);
			if (tigWidgetMsg->widgetEventType == TigMsgWidgetEvent::MouseReleased){
				return TRUE;
			}
			if (tigWidgetMsg->widgetEventType == TigMsgWidgetEvent::Clicked) {
				return TRUE;
			}
		}
	}

	return orgMemorizeSpellMsg(widId, tigMsg);
}

const char *CharUiSystem::TabNameReplacement(int stat)
// Replace the tab text (in method UiCharSpellsNavClassTabRender) with long or short text for the names as appropriate
{
	auto uiCharSpellTabsCount = temple::GetRef<int>(0x10D18F6C);
	const char *res = nullptr;

	if (uiCharSpellTabsCount <= 3) {
		res = d20Stats.GetStatName(static_cast<Stat>(stat));
	} else {
		res = d20Stats.GetStatShortName(static_cast<Stat>(stat));
	}

	return res;
}

bool CharUiSystem::CharSpellsNavClassTabMsg(int widId, TigMsg* tigMsg)
{
	auto uiCharSpellsNavClassTabIdxBefore = temple::GetRef<int>(0x10D18F68);

	auto ret = orgCharSpellsNavClassTabMsg(widId, tigMsg);

	auto uiCharSpellsNavClassTabIdxAfter = temple::GetRef<int>(0x10D18F68);

	//For a change of tab, hide memorized spells for all new natural casting classes
	if (uiCharSpellsNavClassTabIdxBefore != uiCharSpellsNavClassTabIdxAfter) {

		auto navClassPackets = addresses.uiCharSpellsNavPackets;
		auto spellClassCode = navClassPackets[uiCharSpellsNavClassTabIdxAfter].spellClassCode;
		auto classCode = spellSys.GetCastingClass(spellClassCode);
		auto& curCharSpellPkt = addresses.uiCharSpellPackets[uiCharSpellsNavClassTabIdxAfter];

		if (d20ClassSys.IsNaturalCastingClass(classCode) && !spellSys.isDomainSpell(spellClassCode) 
			&& (classCode != stat_level_sorcerer) && (classCode != stat_level_bard)) {
			for (auto i = 0u; i < NUM_SPELLBOOK_SLOTS; i++) {
		        uiManager->SetHidden(curCharSpellPkt.memorizeSpellWnds[i]->widgetId, true);
			}
		}

	}

	return ret;
}

void CharUiSystem::SpellsShow(objHndl obj)
{

	//orgSpellsShow(obj);
	//return;
	
	auto dude = temple::GetRef<objHndl>(0x10BE9940); // critter with inventory open

	auto navClassPackets = addresses.uiCharSpellsNavPackets;
	auto charSpellPackets = addresses.uiCharSpellPackets;

	auto spellsMainWnd = temple::GetRef<LgcyWindow*>(0x10C81BC0);
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

		if (idx < 0 || (uint32_t) idx > spCount) {
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

	uiManager->SetHidden(spellsMainWndId, false);
	uiManager->SetHidden((*addresses.uiCharSpellsNavClassTabWnd)->widgetId, false);
	uiManager->SetHidden((*addresses.uiCharSpellsSpellsPerDayWnd)->widgetId, false);
	uiManager->BringToFront(spellsMainWndId);
	uiManager->BringToFront((*addresses.uiCharSpellsSpellsPerDayWnd)->widgetId);

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

	// Find the tab count before making the tabs to determine if long or short names should be used
	std::vector<Stat> tabClasses;
	for (auto it : d20ClassSys.classEnums) {
		auto classCode = (Stat)it;
		auto spellClassCode = spellSys.GetSpellClass(classCode);

		if (!d20ClassSys.HasSpellList(classCode))
			continue;

		auto classLvl = objects.StatLevelGet(dude, classCode);
		if (classLvl <= 0)
			continue;

		uiCharSpellTabsCount++;
		tabClasses.push_back(classCode);
	}
	
	//One more tab for clerics
	auto clericLvl = critterSys.GetCasterLevelForClass(dude, stat_level_cleric);
	if (clericLvl > 0) {
		uiCharSpellTabsCount++;
	}

	// find which caster class tabs should appear
	int nTabIdx = 0;  //Current Tab index
	for (auto classCode : tabClasses) {
		auto spellClassCode = spellSys.GetSpellClass(classCode);

		// Shorten the names of tabs if there are more than 3 so they fit reasonably
		if (uiCharSpellTabsCount <= 3) {
			line = d20Stats.GetStatName(classCode);
		}
		else {
			line = d20Stats.GetStatShortName(classCode);
		}
		auto textSize = UiRenderer::MeasureTextSize(line, style);

		auto navTabBtn = navClassPackets[nTabIdx].button;
		navTabBtn->width = textSize.width + 2 * uiCharSpellTabPadding;
		navTabBtn->x += navTabX;
		navTabX += navTabBtn->width ;
		uiManager->SetHidden(navTabBtn->widgetId, false);
		
		navClassPackets[nTabIdx].spellClassCode = spellClassCode;
		
		int n1 = 0;
		for (int spellLvl = 0; spellLvl < 10; spellLvl++){
			int numSpells = spellSys.GetNumSpellsPerDay(dude, classCode, spellLvl); //d20ClassSys.GetNumSpellsFromClass(dude, classCode, spellLvl, classLvl);
			if (numSpells > 0)
				n1 += numSpells;
		}
		int nMemo = critterSys.SpellNumByFieldAndClass(dude, obj_f_critter_spells_memorized_idx, spellClassCode);
		if (n1 > nMemo)
			nMemo = n1;

		int nKnown = max(0, critterSys.SpellNumByFieldAndClass(dude, obj_f_critter_spells_known_idx, spellClassCode)-2);
		
		uiManager->ScrollbarSetYmax(charSpellPackets[nTabIdx].spellbookScrollbar->widgetId, nKnown);
		uiManager->ScrollbarSetYmax(charSpellPackets[nTabIdx].memorizeScrollbar->widgetId, nMemo-2 <=0 ? 0 : nMemo-2);

		nTabIdx++;
	}

	// domain
	if ( clericLvl > 0) {
		mesline.key = 2;
		mesFuncs.GetLine_Safe(spellsUiText, &mesline);
		line = mesline.value;
		auto textSize = UiRenderer::MeasureTextSize(line, style);

		auto domainTabBtn = navClassPackets[nTabIdx].button;
		domainTabBtn->width = textSize.width + 2 * uiCharSpellTabPadding;
		domainTabBtn->x += navTabX;
		navTabX += textSize.width;
		uiManager->SetHidden(domainTabBtn->widgetId, false);
		navClassPackets[nTabIdx].spellClassCode = Domain_Count;


		int n1 = d20ClassSys.NumDomainSpellsKnownFromClass(dude, stat_level_cleric);
		int nMemo = critterSys.DomainSpellNumByField(dude, obj_f_critter_spells_memorized_idx);
		if (n1 > nMemo)
			nMemo = n1;

		int nKnown = max(0, critterSys.DomainSpellNumByField(dude, obj_f_critter_spells_known_idx) - 2);


		uiManager->ScrollbarSetYmax(charSpellPackets[nTabIdx].spellbookScrollbar->widgetId, nKnown);
		uiManager->ScrollbarSetYmax(charSpellPackets[nTabIdx].memorizeScrollbar->widgetId, nMemo - 2 <= 0 ? 0 : nMemo - 2);
	}

	// show the first class's spellbook / spells memorized
	if (uiCharSpellTabsCount > 0 && !spellSys.isDomainSpell(navClassPackets[0].spellClassCode))	{
		auto classCode = spellSys.GetCastingClass(navClassPackets[0].spellClassCode );
		// if is vancian, show the spells memorized too
		if (d20ClassSys.IsVancianCastingClass(classCode)){
			uiManager->SetHidden(charSpellPackets[0].classMemorizeWnd->widgetId, false);
		} else
		{
			uiManager->SetHidden(charSpellPackets[0].classMemorizeWnd->widgetId, true);
		}
		uiManager->SetHidden(charSpellPackets[0].classSpellbookWnd->widgetId, false);
		uiManager->BringToFront(charSpellPackets[0].classSpellbookWnd->widgetId);
		uiManager->BringToFront(charSpellPackets[0].classMemorizeWnd->widgetId);

	}

	// hide the other spellbook windows
	for (int i = 1; i < uiCharSpellTabsCount; i++){
		uiManager->SetHidden(charSpellPackets[i].classSpellbookWnd->widgetId, true);
		uiManager->SetHidden(charSpellPackets[i].classMemorizeWnd->widgetId, true);
	}

	// hide inactive nav tabs 
	// TODO extend to support more than 11 tabs... for that one crazy ass player who picks a shitload of casting classes :P
	for (int i = uiCharSpellTabsCount; i < VANILLA_NUM_CLASSES + 1; i++){
		uiManager->SetHidden(navClassPackets[i].button->widgetId, true);
	}

	// if no tabs, return
	if (!uiCharSpellTabsCount)
	{
		UiRenderer::PopFont();
		uiManager->BringToFront((*addresses.uiCharSpellsNavClassTabWnd)->widgetId);
		return;
	}

	for (int i = 0; i < uiCharSpellTabsCount; i++)	{
		auto spellClassCode = navClassPackets[i].spellClassCode;
		

		// fill out the number of spells per day (per level) for each NavPacket

		if (!spellSys.isDomainSpell(spellClassCode)){ //normal casting class
			//LevelPacket lvlPkt;
			auto classCode = spellSys.GetCastingClass(spellClassCode);
			//auto classLvl = objects.StatLevelGet(dude, classCode);
			//classLvl += critterSys.GetSpellListLevelExtension(dude, classCode);
			//lvlPkt.GetLevelPacket(classCode, dude, 0, classLvl);
			auto numSpellsForLvl = navClassPackets[i].numSpellsForLvl;
			for (int j = 0; j < NUM_SPELL_LEVELS; j++){
				auto numSp = spellSys.GetNumSpellsPerDay(dude, classCode, j);  //d20ClassSys.GetNumSpellsFromClass(dude, classCode, j, classLvl);
				if  (numSp >= 0){ //(lvlPkt.spellCountFromClass[j] >= 0){
					numSpellsForLvl[j] = numSp; //lvlPkt.spellCountBonusFromStatMod[j] + lvlPkt.spellCountFromClass[j];
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
			auto &domainSpellPacket = navClassPackets[i];
			auto maxClericSpellLvl = d20ClassSys.ClericMaxSpellLvl(clericLvl);
			if (maxClericSpellLvl >= 1)	{
				for (int j = 1; j <= maxClericSpellLvl; j++) {
					domainSpellPacket.numSpellsForLvl[j] = 1;
				}
			}
			auto race = critterSys.GetRace(dude, false);
			auto racialSpells = d20RaceSys.GetSpellLikeAbilities(race);
			for (auto it:racialSpells){
				auto &spell = it.first;
				auto specifiedCount = it.second;
				domainSpellPacket.numSpellsForLvl[spell.spellLevel] += specifiedCount;				
			}
		}


		// fill out the spells per each level
		auto spellsKnownArray = gameSystems->GetObj().GetObject(dude)->GetSpellArray(obj_f_critter_spells_known_idx);
		auto spellsKnownArraySize = spellsKnownArray.GetSize();
		std::vector<SpellStoreData> spellsKnownVector;
		for (auto j = 0u; j < spellsKnownArraySize; j++){
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
		for (auto spellLvl = 0u; spellLvl < NUM_SPELL_LEVELS; spellLvl++){
			auto spellKnNum = charSpellPackets[i].spellsKnown.count;
			
			SpellStoreData spStore;
			memset(&spStore, 0, sizeof SpellStoreData);
			spStore.spellLevel = spellLvl;

			int spFound = -1;
			for (auto j = 0u; j < spellKnNum; j++) {
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
			for (auto j = 0u; j < numSpMemo; j++)	{
				SpellStoreData spStore = spellsMemoArray[j];
				// normal spells
				if (!spellSys.isDomainSpell(spStore.classCode) && !spellSys.isDomainSpell(navClassPackets[i].spellClassCode)){
					if (spStore.classCode != navClassPackets[i].spellClassCode)
						continue;
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
		for (auto spellLvl = 0u; spellLvl < NUM_SPELL_LEVELS; spellLvl++) {
			SpellStoreData spStore;
			auto numSpellsForLvl = navClassPackets[i].numSpellsForLvl[spellLvl];
			auto spMemNum = charSpellPackets[i].spellsMemorized.count;
			
			auto spFound = 0u;
			for (auto j = 0u; j < spMemNum; j++) {
				if (charSpellPackets[i].spellsMemorized.spells[j].spellLevel >= spellLvl) {
					break;
				}
				spFound++;
			}
			auto spFound2 = spFound;
			auto numEmptySlotsForLvl = 0;
			auto hasSpecializationSlot = 0;
			while (spFound < spMemNum )	{
				auto spFoundLvl = charSpellPackets[i].spellsMemorized.spells[spFound].spellLevel;
				if (charSpellPackets[i].spellsMemorized.spells[spFound].pad0 & 0x80){
					hasSpecializationSlot = 1; 
					spFoundLvl++;
				}
					
				if (spFoundLvl > spellLvl)
					break;
				spFound++;
				numEmptySlotsForLvl++;
			}
			
			for (int j = 0; j < numSpellsForLvl - numEmptySlotsForLvl - hasSpecializationSlot; j++) {
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
			for (auto j = 0u; j < charSpellPackets[i].spellsMemorized.count; j++)	{
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
	for (auto i = 0u; i < NUM_SPELLBOOK_SLOTS; i++){
		if (i < curCharSpellPkt.spellsKnown.count){
			uiManager->SetHidden(curCharSpellPkt.spellbookSpellWnds[i]->widgetId, false);
			uiManager->BringToFront(curCharSpellPkt.spellbookSpellWnds[i]->widgetId);

		} else
		{
			uiManager->SetHidden(curCharSpellPkt.spellbookSpellWnds[i]->widgetId, true);
		}
	}

	// hide memorized slots without spells and for natural casting classes
	bool showMemSpells = true;
	auto spellClassCode = navClassPackets[uiCharSpellsNavClassTabIdx].spellClassCode;
	if (!spellSys.isDomainSpell(spellClassCode)){
		auto casterClassCode = spellSys.GetCastingClass(spellClassCode);

		if (d20ClassSys.IsNaturalCastingClass(casterClassCode))
			showMemSpells = false;
	}
	
	for (auto i = 0u; i < NUM_SPELLBOOK_SLOTS; i++) {
		if (showMemSpells && i < curCharSpellPkt.spellsMemorized.count) {
			uiManager->SetHidden(curCharSpellPkt.memorizeSpellWnds[i]->widgetId, false);
			uiManager->BringToFront(curCharSpellPkt.memorizeSpellWnds[i]->widgetId);
		}
		else
		{
			uiManager->SetHidden(curCharSpellPkt.memorizeSpellWnds[i]->widgetId, true);
		}
	}
	

		//UiRenderer::RenderText(line, rect, mImpl->style);


	



	

	UiRenderer::PopFont();

	uiManager->BringToFront((*addresses.uiCharSpellsNavClassTabWnd)->widgetId);


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

objHndl CharUiSystem::GetCurrentCritter()
{
	return temple::GetRef<objHndl>(0x10BE9940);
}

objHndl CharUiSystem::GetCritterLooted()
{
	return temple::GetRef<objHndl>(0x10BE6EC0);
}

objHndl CharUiSystem::GetVendor()
{
	return temple::GetRef<objHndl>(0x10BE6EC8);
}

void UiChar::LootingWidgetsInit(){
	auto wnd = temple::GetRef<LgcyWindow*>(0x10BE6EA0);
	auto wndId = wnd->widgetId;

	mLootingWnd = std::make_unique<WidgetContainer>(32,32);
	mLootingWnd->SetPos(wnd->x + 90, wnd->y + 50);
	auto mNextBtn = std::make_unique<WidgetButton>();
	mNextBtn->SetPos(0,0);
	mNextBtn->SetStyle("action-pointer-next");
	mNextBtn->SetClickHandler([&](){
		auto &lootedCritter = uiSystems->GetChar().GetLootedObject();
		mCrittersLootedIdx++;
		if (mCrittersLootedIdx >= static_cast<int>(mCrittersLootedList.size()))
			mCrittersLootedIdx = 0;
		auto nextCritter = mCrittersLootedList[mCrittersLootedIdx];
		uiSystems->GetChar().SetLootedObject(nextCritter);
		auto resetLootIcons = temple::GetRef<void(__cdecl)()>(0x1013DD20);
		resetLootIcons();
	});
	mLootingWnd->Add(std::move(mNextBtn));
	mLootingWnd->Hide();
}

std::vector<objHndl> UiChar::GetNearbyLootableCritters(const objHndl& handle){

	auto result = std::vector<objHndl>();
	result.push_back(handle);
	if (!handle)
		return result;
	auto obj = objSystem->GetObject(handle);
	ObjList objList;
	objList.ListRadius(obj->GetLocationFull(), INCH_PER_TILE * 10, ObjectListFilter::OLC_CRITTERS);

	for (auto i = 0; i < objList.size(); i++) {
		auto corpse = objList[i];
		if (corpse != handle && critterSys.IsLootableCorpse(corpse) && !party.IsInParty(corpse) && pathfindingSys.CanPathTo(handle, corpse))
			result.push_back(corpse);
	}

	return result;
}

void UiChar::ShowLooting(){
	if (uiSystems->GetChar().GetDisplayType() != UiCharDisplayType::Looting){
		mLootingWnd->Hide();
		return;
	}
	auto lootedCritter = uiSystems->GetChar().GetLootedObject();
	if (!lootedCritter || !objects.IsCritter(lootedCritter)){
		mLootingWnd->Hide();
		return;
	}

	mCrittersLootedList = GetNearbyLootableCritters(lootedCritter);
	mCrittersLootedIdx = 0;

	if (mCrittersLootedList.size() <= 1){
		mLootingWnd->Hide();
		return;
	}

	mLootingWnd->Show();
	mLootingWnd->BringToFront();
}

void UiChar::HideLooting(){
	mLootingWnd->Hide();
}


int CharUiSystem::InventorySlotMsg(int widId, TigMsg* msg)
{
	// Alt-click to Quicksell
	if (uiSystems->GetChar().IsLootingActive())
	{
		if (msg->type == TigMsgType::MOUSE)
		{
			if (msg->arg4 & MSF_LMB_CLICK)
			{
				
				if ( infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) || infrastructure::gKeyboard.IsKeyPressed(VK_RMENU)){

					objHndl critterLooted = GetCritterLooted(); // may be substitute inventory object (i.e. a container)

					auto uiCharInvItemGetFromWidId = temple::GetRef<objHndl(__cdecl)(int)>(0x10157060);
					auto item = uiCharInvItemGetFromWidId(widId);

					objHndl vendor = GetVendor();

					logger->debug("Quickselling item {} to {} ({})", description.getDisplayName(item), description.getDisplayName(vendor),description.getDisplayName(critterLooted));
					
					objHndl appraiser = party.PartyMemberWithHighestSkill(SkillEnum::skill_appraise);
					
					
					int appraisedWorth = inventory.GetAppraisedWorth(item, appraiser, vendor, SkillEnum::skill_appraise);
					int plat =0, gold=0, silver=0, copper=0;
					if (appraisedWorth && uiSystems->GetChar().IsBartering()) {
						
						int qty = max(1, inventory.GetQuantity(item));
						appraisedWorth *= qty;
						inventory.MoneyToCoins(appraisedWorth, &plat, &gold, &silver, &copper);
					}
					// IIF_Allow_Swap | IIF_Use_Wield_Slots | IIF_4 | IIF_Use_Max_Idx_200 | IIF_10 
					ItemErrorCode itemTransferError = inventory.TransferWithFlags(item, critterLooted, -1, 1+2+4+8+16, objHndl::null);
					if (itemTransferError == IEC_OK && uiSystems->GetChar().IsBartering()) {
						objSystem->GetObject(item)->SetItemFlag(OIF_IDENTIFIED, 1);
						party.MoneyAdj(plat, gold, silver, copper);
					} 
					else if (itemTransferError == IEC_No_Room_For_Item){
						int itemIdx = 100;
						for (itemIdx = 100; itemIdx< 255; itemIdx++){
							if (!inventory.GetItemAtInvIdx(critterLooted, itemIdx))
								break;
						}
						if (itemIdx < INVENTORY_WORN_IDX_START)
							itemTransferError = inventory.TransferWithFlags(item, critterLooted, itemIdx, 1 + 2 + 4 + 8 , objHndl::null);
						if (itemTransferError == IEC_OK && uiSystems->GetChar().IsBartering()) {
							objSystem->GetObject(item)->SetItemFlag(OIF_IDENTIFIED, 1);
							party.MoneyAdj(plat, gold, silver, copper);
						}
					}
					return TRUE;
				}
				
				
			}
		}
	}


	if (msg->type == TigMsgType::WIDGET){

		auto msg_ = (TigMsgWidget*)msg;

		
	}
	return orgInventorySlotMsg(widId, msg);
}

char* CharUiSystem::HookedItemDescriptionBarter(objHndl obj, objHndl item)
{
	// append item non-profiency etc. warnings to barter tooltip
	auto orgItemDescriptionBarter = temple::GetRef<char*(__cdecl)(objHndl, objHndl)>(0x10123220);
	auto strOut = orgItemDescriptionBarter(obj, item);
	auto itemType = gameSystems->GetObj().GetObject(item)->type;
	if (itemType == obj_t_weapon)
	{
		auto wieldType = inventory.GetWieldType(obj, item);
		if (wieldType == 3)
		{
			auto itemError = inventory.GetItemErrorString(IEC_Item_Too_Large);
			sprintf(strOut, "%s\n\n%s", strOut, itemError);
		} else
		{
			auto wpnType = objects.GetWeaponType(item);
			if (!feats.WeaponFeatCheck(obj, nullptr, 0, (Stat)0, wpnType))
			{
				auto itemError = uiAssets->GetTooltipString(136);
				sprintf(strOut, "%s\n\n%s", strOut, itemError);
			}
		}
	} 
	else if (itemType == obj_t_armor)
	{
		auto itemWearFlags = objects.GetItemWearFlags(item);
		if ((itemWearFlags & OIF_WEAR::OIF_WEAR_ARMOR))
		{
			auto armorType = inventory.GetArmorType(objects.getInt32(item, obj_f_armor_flags));
			if (armorType != ARMOR_TYPE_NONE
				&& !inventory.IsProficientWithArmor(obj, item))
				sprintf(strOut, "%s\n\nNot Proficient With Armor!", strOut);
		}
			
	}
	
	if (description.LongDescriptionHas(item) && inventory.IsIdentified(item) ){
		sprintf(strOut, "%s\n\n%s", strOut, temple::GetRef<const char*(__cdecl)(int)>(0x10122DA0)(6048));
	}

	return strOut;
}

void CharUiSystem::ItemGetDescrAddon(objHndl obj, objHndl item, std::string& addStr){
	auto itemObj = gameSystems->GetObj().GetObject(item);
	if (itemObj->type == obj_t_food || itemObj->type == obj_t_scroll){
		
		if (!(itemObj->GetItemFlags() & OIF_IDENTIFIED))
			return;

		auto spellData = itemObj->GetSpell(obj_f_item_spell_idx, 0);
		int spellLevel = spellData.spellLevel;
		int casterLevel = max(1, spellLevel * 2 - 1);
		SpellEntry spellEntry(spellData.spellEnum);
		if (spellEntry.spellSchoolEnum > 8 || spellEntry.spellSchoolEnum < 0)
			spellEntry.spellSchoolEnum = 0;

		if (spellEntry.spellSchoolEnum)

			if (itemObj->type == obj_t_scroll)	{

				if (spellSys.IsArcaneSpellClass(spellData.classCode)){	
					addStr = fmt::format("{}: {}  [Arcane, {}]", d20Stats.GetStatName(stat_caster_level), casterLevel, spellSys.GetSpellMesline(15000 + spellEntry.spellSchoolEnum));
				}
				else if (spellData.classCode == Domain_Special)
				{
					addStr = fmt::format("{}: {}  [{}]", d20Stats.GetStatName(stat_caster_level), casterLevel, spellSys.GetSpellMesline(15000 + spellEntry.spellSchoolEnum));
				}  
				else 
				{ // divine spell
					addStr = fmt::format("{}: {}  [Divine, {}]", d20Stats.GetStatName(stat_caster_level), casterLevel, spellSys.GetSpellMesline(15000 + spellEntry.spellSchoolEnum));
				}
			} 
			
			else{
				
				addStr = fmt::format("{}: {}  [{}]", d20Stats.GetStatName(stat_caster_level), casterLevel, spellSys.GetSpellMesline(15000 + spellEntry.spellSchoolEnum));
			}
		else
			addStr = fmt::format("{}: {}", d20Stats.GetStatName(stat_caster_level), casterLevel);
	}

	if (itemObj->type == obj_t_generic || itemObj->type == obj_t_weapon){
		auto remCharges = gameSystems->GetObj().GetObject(item)->GetInt32(obj_f_item_spell_charges_idx);
		if (remCharges > 0){
			addStr = fmt::format("Remaining Charges: {}", remCharges);
		}
	}
	{
		objHndl vendor = GetVendor();
		objHndl appraiser = party.PartyMemberWithHighestSkill(SkillEnum::skill_appraise);
		int worth = gameSystems->GetObj().GetObject(item)->GetInt32(obj_f_item_worth);
		int appraisedWorth = inventory.GetAppraisedWorth(item, obj, vendor, SkillEnum::skill_appraise);
		int plat = 0, gold = 0, silver = 0, copper = 0;
		inventory.MoneyToCoins(appraisedWorth, &plat, &gold, &silver, &copper);
		gold += plat * 10;
		plat = 0;
		addStr = fmt::format((addStr.length() > 0) ? "{0}\nPrice: {1}" : "Price: {1}{2}{3}{4}"
			, addStr
			, (gold || (!silver && !copper)) ? fmt::format("{0} gp ", gold) : ""
			, (silver) ? fmt::format("{0} sp ", silver) : ""
			, (copper) ? fmt::format("{0} cp ", copper) : ""
			, (worth != appraisedWorth) ? fmt::format("({0:g} gp) ", worth / 100.0) : ""
		);
	}
}

void CharUiSystem::LongDescriptionPopupCreate(objHndl item){
	temple::GetRef<void(__cdecl)(objHndl)>(0x10144400)(item);
}

void CharUiSystem::TotalWeightOutputBtnTooltip(int x, int y, int* widId)
{

	LgcyButton * btn = uiManager->GetButton(*widId);
	if (btn->buttonState == 2)
		return;

	TigTextStyle style;
	const ColorRect cRect1(XMCOLOR(0xCC111111));
	style.bgColor = const_cast<ColorRect*>(&cRect1);
	const ColorRect cRectShadow(XMCOLOR(0xFF000000));
	style.shadowColor = const_cast<ColorRect*>(&cRectShadow);
	const ColorRect cRectText[3] = { ColorRect( 0xFFFFFFFF ), ColorRect(0xFFFFFF66), ColorRect(0xFFF33333) };
	style.textColor = const_cast<ColorRect*>(cRectText);

	style.flags = 0xC08;
	style.kerning = 2;
	style.tracking = 2;

	style.field2c = -1;

	auto obj = charUiSys.GetCurrentCritter();

	auto charInvTtStyleIdx = temple::GetRef<int>(0x10BEECB0);
	auto tt = tooltips.GetStyle(charInvTtStyleIdx);

	UiRenderer::PushFont(tt.fontName, tt.fontSize);

	std::string encumText;

	auto isEncLight = d20Sys.d20Query(obj, DK_QUE_Critter_Is_Encumbered_Light);
	auto youAreEncumbered = tooltips.GetTooltipString(125);

	auto strScore = objects.StatLevelGet(obj, stat_strength);

	auto encumNextWeight = temple::GetRef<int(__cdecl)(int, int)>(0x100EBB20); // gets the max weight for current encumbrance

	if (isEncLight) {
		auto youAreNotEnc = tooltips.GetTooltipString(124);
		auto encLoad = tooltips.GetTooltipString(126);
		auto encMin = tooltips.GetTooltipString(134);
		auto encMax = tooltips.GetTooltipString(129);

		encumText = fmt::format("@0({})@0\n\n{} ({}/{}): ({}/{})",
			youAreNotEnc, encLoad, encMin, encMax, 0, isEncLight);
	}
	else{
		auto isEncMed = d20Sys.d20Query(obj, DK_QUE_Critter_Is_Encumbered_Medium);
		if (isEncMed){
			auto youAreEnc = tooltips.GetTooltipString(125);
			auto encLoad = tooltips.GetTooltipString(127);
			auto encMin = tooltips.GetTooltipString(134);
			auto encMax = tooltips.GetTooltipString(129);

			encumText = fmt::format("{} @1({})@0\n\n{} ({}/{}): ({}/{})",
				youAreEnc, encLoad, encLoad, encMin, encMax, encumNextWeight(strScore, 1), isEncMed);
		}
		else {
			auto isEncHeavy = d20Sys.d20Query(obj, DK_QUE_Critter_Is_Encumbered_Heavy);
			if (isEncHeavy) {
				auto youAreEnc = tooltips.GetTooltipString(125);
				auto encLoad = tooltips.GetTooltipString(128);
				auto encMin = tooltips.GetTooltipString(134);
				auto encMax = tooltips.GetTooltipString(129);

				encumText = fmt::format("{} @2({})@0\n\n{} ({}/{}): ({}/{})",
					youAreEnc, encLoad, encLoad, encMin, encMax, encumNextWeight(strScore, 2), isEncHeavy);

			}
			else {
				auto isEncOver = d20Sys.d20Query(obj, DK_QUE_Critter_Is_Encumbered_Overburdened);
				if (isEncOver) {
					auto youAreEnc = tooltips.GetTooltipString(125);
					auto encLoad = tooltips.GetTooltipString(135);
					auto encMin = tooltips.GetTooltipString(134);
					auto encMax = tooltips.GetTooltipString(129);

					encumText = fmt::format("{} @2({})@0\n\n{} ({}/{}): ({}/{})",
						youAreEnc, encLoad, encLoad, encMin, encMax, encumNextWeight(strScore, 2), isEncHeavy);

				}
				else {
					return;
				}
			}
		}
	}


	auto measuredSize = UiRenderer::MeasureTextSize(encumText, style);
	TigRect extents(x, y - measuredSize.height, measuredSize.width, measuredSize.height);
	if (extents.y  < 0){
		extents.y = y;
	}
	auto wftWidth = temple::GetRef<int>(0x103012C8);
	if ( extents.x + measuredSize.width > wftWidth)	{
		extents.x = wftWidth - measuredSize.width;
	}
	UiRenderer::RenderText(encumText, extents, style);

	UiRenderer::PopFont();
}

/* 0x10144350 */
objHndl CharUiSystem::GetBag(){
	return objHndl::null;
}

/* 0x101A2FB0 */
void CharUiSystem::WeaponComboActivate(objHndl handle, int weaponCombo)
{
	auto bag = GetBag();

	{
		int weaponInvLoc = inventory.PcWeaponComboGetValue(handle, 4 * weaponCombo + WEAP_COMBO_MAIN);
		if (weaponInvLoc != INVENTORY_IDX_UNDEFINED) {
			auto weapon = inventory.GetItemAtInvIdx(handle, weaponInvLoc);
			if (weapon) {
				inventory.TransferWithFlags(weapon, handle, INVENTORY_WORN_IDX_START + EquipSlot::WeaponPrimary, ItemInsertFlags::IIF_4, bag);
				inventory.PcWeaponComboSetValue(handle, 4 * weaponCombo + WEAP_COMBO_MAIN, INVENTORY_WORN_IDX_START + EquipSlot::WeaponPrimary);
			}
		}
	}
	
	{
		int weaponInvLoc = inventory.PcWeaponComboGetValue(handle, 4 * weaponCombo + WEAP_COMBO_SECONDARY);
		if (weaponInvLoc != INVENTORY_IDX_UNDEFINED) {
			auto weapon = inventory.GetItemAtInvIdx(handle, weaponInvLoc);
			if (weapon) {
				inventory.TransferWithFlags(weapon, handle, INVENTORY_WORN_IDX_START + EquipSlot::WeaponSecondary, ItemInsertFlags::IIF_4, bag);
				inventory.PcWeaponComboSetValue(handle, 4 * weaponCombo + WEAP_COMBO_SECONDARY, INVENTORY_WORN_IDX_START + EquipSlot::WeaponSecondary);
			}
		}
	}
	
	{
		int ammoInvLoc = inventory.PcWeaponComboGetValue(handle, 4 * weaponCombo + WEAP_COMBO_AMMO);
		if (ammoInvLoc != INVENTORY_IDX_UNDEFINED) {
			auto ammo = inventory.GetItemAtInvIdx(handle, ammoInvLoc);
			if (ammo) {
				inventory.TransferWithFlags(ammo, handle, INVENTORY_WORN_IDX_START + EquipSlot::Ammo, ItemInsertFlags::IIF_4, bag);
				inventory.PcWeaponComboSetValue(handle, 4 * weaponCombo + WEAP_COMBO_AMMO, INVENTORY_WORN_IDX_START + EquipSlot::Ammo);
			}
		}
	}

	{
		int ammoInvLoc = inventory.PcWeaponComboGetValue(handle, 4 * weaponCombo + WEAP_COMBO_SHIELD);
		if (ammoInvLoc != INVENTORY_IDX_UNDEFINED) {
			auto ammo = inventory.GetItemAtInvIdx(handle, ammoInvLoc);
			if (ammo) {
				inventory.TransferWithFlags(ammo, handle, INVENTORY_WORN_IDX_START + EquipSlot::Shield, ItemInsertFlags::IIF_4, bag);
				inventory.PcWeaponComboSetValue(handle, 4 * weaponCombo + WEAP_COMBO_SHIELD, INVENTORY_WORN_IDX_START + EquipSlot::Shield);
			}
		}
	}
}

void CharUiSystem::FeatsShow(){
	auto dude = temple::GetRef<objHndl>(0x10BE9940); // critter with inventory open

	temple::GetRef<int>(0x10D19E8C) = 1; // featsActive

	auto &featsNum = temple::GetRef<int>(0x10D19388);
	auto &featList = temple::GetRef<feat_enums[]>(0x10D1938C);
	featsNum = 0;

	for (auto i=0; i< NUM_FEATS; i++){
		auto feat = (feat_enums)i;
		if (feats.HasFeatCountByClass(dude, feat))
		{
			featList[featsNum++] = feat;
		};
	}
	for (auto feat:feats.newFeats){
		if (feats.HasFeatCountByClass(dude, feat))
		{
			featList[featsNum++] = feat;
		};
	}


	for (auto i=featsNum; i < 128; i++)
	{
		featList[i] = FEAT_NONE;
	}

	auto featsScrollbar = temple::GetRef<LgcyScrollBar*>(0x10D19DB4);
	uiManager->ScrollbarSetYmax(featsScrollbar->widgetId, featsNum < 20 ? 0 : featsNum - 20);

	auto featsWnd = temple::GetRef<LgcyWindow*>(0x10D19DB0);
	uiManager->SetHidden(featsWnd->widgetId, false); // FeatsMainWnd
	uiManager->BringToFront(featsWnd->widgetId);
}



//*****************************************************************************
//* Char-UI
//*****************************************************************************

UiChar::UiChar(const UiSystemConf &config) {
	auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x1014b900);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Char-UI");
	}
	LootingWidgetsInit();
}
UiChar::~UiChar() {
	auto shutdown = temple::GetPointer<void()>(0x10149820);
	shutdown();
}
void UiChar::ResizeViewport(const UiResizeArgs& resizeArg) {
	auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x1014ba20);
	resize(&resizeArg);
}
void UiChar::Reset() {
	auto reset = temple::GetPointer<void()>(0x10143f80);
	reset();
}
const std::string &UiChar::GetName() const {
	static std::string name("Char-UI");
	return name;
}

void UiChar::Show(UiCharDisplayType type)
{
	static auto ui_show_charui = temple::GetPointer<void(UiCharDisplayType)>(0x10148e20);
	ui_show_charui(type);
}

void UiChar::ShowForCritter(UiCharDisplayType type, objHndl handle) {
	mCurrentCritter = handle;
	Show(type);
	SetCritter(handle);
}

objHndl UiChar::GetCritter(){ // originally 0x10144050
	return mCurrentCritter;
}

void UiChar::SetCritter(objHndl handle) {
	static auto ui_show_charui = temple::GetPointer<void(objHndl)>(0x101499E0);
	ui_show_charui(handle);
}

void CharUiSystem::apply(){
	
	replaceFunction(0x101A2FB0, WeaponComboActivate);

	replaceFunction(0x10144B40, ClassLevelBtnRender);
	replaceFunction(0x10145020, AlignGenderRaceBtnRender);

	if (temple::Dll::GetInstance().HasCo8Hooks()) {
		writeHex(0x1011DD4D, "90 90 90 90 90"); // disabling stat text draw calls
	}

	if (config.showNpcStats) {
		writeNoops(0x101C2247); // str output btn
		writeNoops(0x101C2529); // dex output btn
		writeNoops(0x101C2809); // con output btn
		writeNoops(0x101C2AE9); // int output btn
		writeNoops(0x101C2DC9); // wis output btn
		writeNoops(0x101C30A9); // cha output btn

		writeNoops(0x101C4D5A); // str mod btn
		writeNoops(0x101C4F4A); // dex mod btn
		writeNoops(0x101C513A); // con mod btn
		writeNoops(0x101C532A); // int mod btn
		writeNoops(0x101C551A); // wis mod btn
		writeNoops(0x101C570A); // cha mod btn

	}

	writeCall(0x101C1F98, StatsLvlBtnRenderHook);

	int charSheetAttackCodeForAttackBonusDisplay = 1 + ATTACK_CODE_OFFHAND;
	write(0x101C45F3 + 7, &charSheetAttackCodeForAttackBonusDisplay, sizeof(int));
	write(0x101C8C7B + 4, &charSheetAttackCodeForAttackBonusDisplay, sizeof(int));

	charSheetAttackCodeForAttackBonusDisplay = 1 + ATTACK_CODE_OFFHAND + 1; //the offhand
	write(0x101C491B + 7, &charSheetAttackCodeForAttackBonusDisplay, sizeof(int));
	write(0x101C8D74 + 4, &charSheetAttackCodeForAttackBonusDisplay, sizeof(int));


	// Feats
	replaceFunction(0x101BBDB0, FeatsShow);



	//orgSpellbookSpellsMsg = replaceFunction(0x101B8F10, SpellbookSpellsMsg);
	orgMemorizeSpellMsg = replaceFunction(0x101B9360, MemorizeSpellMsg);
	orgCharSpellsNavClassTabMsg = replaceFunction(0x101B8B60, CharSpellsNavClassTabMsg);
	redirectCall(0x101B6ED8, TabNameReplacement);
	replaceFunction(0x101B2EE0, IsSpecializationSchoolSlot);
	orgSpellsShow = replaceFunction(0x101B5D80, SpellsShow);

	static BOOL(__cdecl* orgMetamagicBtnMsg)(int, TigMsg&) = replaceFunction<BOOL(__cdecl)(int, TigMsg&)>(0x101BA580, [](int widId, TigMsg& msg) {
		auto msgType = msg.type;
		if (msgType != TigMsgType::WIDGET){
			return TRUE;
		}
		auto &msgWidget = (TigMsgWidget&)msg;
		if (msgWidget.widgetEventType != TigMsgWidgetEvent::MouseReleased ){
			return TRUE;
		}
		auto handle = uiSystems->GetChar().GetCritter();
		if (!feats.HasMetamagicFeat(handle)){
			return TRUE;
		}

		return orgMetamagicBtnMsg(widId, msg);

	});

	// UiCharSpellGetScrollbarY  has bug when called from Spellbook, it receives the first tab's scrollbar always
	writeCall(0x101BA5D9, HookedCharSpellGetSpellbookScrollbarY);

	writeNoops(0x101B957E); // so it doesn't decrement the spells memorized num (this causes weirdness in right clicking from the spellbook afterwards)

	
	static void(__cdecl* orgCharLootingShow)(objHndl) = replaceFunction<void(__cdecl)(objHndl)>(0x1013F6C0, [](objHndl handle) {
		orgCharLootingShow(handle);
		uiSystems->GetChar().ShowLooting();
	});

	static void(__cdecl* orgCharLootingHide)() = replaceFunction<void(__cdecl)()>(0x1013F880, []() {
		orgCharLootingHide();
		uiSystems->GetChar().HideLooting();
	});
	

	static BOOL(__cdecl* orgUiCharLootingLootWndMsg)(int, TigMsg*) = replaceFunction<BOOL(__cdecl)(int, TigMsg*) >(0x101406D0, [](int widId, TigMsg* msg) {


		if (msg->type == TigMsgType::WIDGET) {

			auto msg_ = (TigMsgWidget*)msg;
			if (msg_->widgetEventType == TigMsgWidgetEvent::Clicked
				&& (infrastructure::gKeyboard.IsKeyPressed(VK_LSHIFT) || infrastructure::gKeyboard.IsKeyPressed(VK_RSHIFT))) {

				objHndl critterLooted = GetCritterLooted(); // may be substitute inventory object (i.e. a container)
				auto invenIdx = temple::GetRef<int(__cdecl)(int)>(0x1013F9C0)(widId);

				auto item = inventory.GetItemAtInvIdx(critterLooted, invenIdx);
				if (item && inventory.IsIdentified(item) && description.LongDescriptionHas(item)) {
					charUiSys.LongDescriptionPopupCreate(item);
					return TRUE;
				}

			}

		}

		return orgUiCharLootingLootWndMsg(widId, msg);
	});

	writeCall(0x10140134, HookedItemDescriptionBarter);
	writeCall(0x1014016B, HookedItemDescriptionBarter);

	orgInventorySlotMsg = replaceFunction<int(__cdecl)(int, TigMsg*) >(0x10157DC0, InventorySlotMsg);

	// Addendums to the item description box
	static char* (*orgGetItemDescr)(objHndl, objHndl) = replaceFunction<char*(__cdecl)(objHndl, objHndl)>(0x10122DD0, [](objHndl observer, objHndl item) {
		// need to put this here
		temple::GetRef<objHndl>(0x10BF07B8) = item;
		auto itemObj = objSystem->GetObject(item);
		auto descBuffer = temple::GetRef<char[]>(0x10BDDEC8);

		if (itemObj->type == obj_t_armor){
			
			auto desc = fmt::format("{}\n\n{}: {}", objects.GetDisplayName(item, observer),
				uiAssets->GetTooltipString(100) /* Weight*/, itemObj->GetInt32(obj_f_item_weight));

			auto acBonus  = dispatch.DispatchItemQuery(item, DK_QUE_Armor_Get_AC_Bonus);
			auto dexBonus = dispatch.DispatchItemQuery(item, DK_QUE_Armor_Get_Max_DEX_Bonus);
			auto maxSpeed = dispatch.DispatchItemQuery(item, DK_QUE_Armor_Get_Max_Speed);
			auto dexBonusString = dexBonus == 100 ? fmt::format(" - ") : fmt::format("{:+d}", dexBonus);
			auto maxSpeedString = maxSpeed == 100 ? fmt::format(" - ") : fmt::format("{}", maxSpeed);
			auto armorCheckPenalty = temple::GetRef<int(__cdecl)(objHndl)>(0x1004F0D0)(item);
			auto spellFailureChance = itemObj->GetInt32(obj_f_armor_arcane_spell_failure);
			spellFailureChance += dispatch.DispatchItemQuery(item, DK_QUE_Get_Arcane_Spell_Failure);
			if (spellFailureChance < 0) spellFailureChance = 0;
			sprintf(descBuffer, "%s\n%s: %+d\n%s: %s   %s: %s\n%s: %d   %s: %d%%",
				desc.c_str(), 
				uiAssets->GetTooltipString(101) /* AC */, acBonus,
				uiAssets->GetTooltipString(120) /* Max DEX Bonus */, dexBonusString.c_str(),
				uiAssets->GetTooltipString(122) /* Max Speed */, maxSpeedString.c_str(),
				uiAssets->GetTooltipString(121) /* Armor Check Penalty */, armorCheckPenalty,
				uiAssets->GetTooltipString(102) /* Spell Failure */, spellFailureChance
			);
		}
		else{
			descBuffer = orgGetItemDescr(observer, item);
		}
		

		
		std::string addonStr;
		ItemGetDescrAddon(observer, item, addonStr);
		sprintf(descBuffer, "%s\n%s", descBuffer, addonStr.c_str());

		return descBuffer;
	});

	replaceFunction(0x10155D20, TotalWeightOutputBtnTooltip);

}

BOOL(*CharUiSystem::orgSpellbookSpellsMsg)(int widId, TigMsg* tigMsg);
BOOL(*CharUiSystem::orgMemorizeSpellMsg)(int widId, TigMsg* tigMsg);
bool(*CharUiSystem::orgCharSpellsNavClassTabMsg)(int widId, TigMsg* tigMsg);
void(*CharUiSystem::orgSpellsShow)(objHndl obj);
BOOL(*CharUiSystem::orgInventorySlotMsg)(int widId, TigMsg* msg);
int CharUiSystem::specSlotIndices[10];