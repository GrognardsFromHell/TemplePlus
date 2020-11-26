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
#include "feat.h"
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
#include <ui\ui_popup.h>
#include <combat.h>

class UiCharImpl;
UiCharImpl* uiCharImpl = nullptr;

#define NUM_SPELLBOOK_SLOTS 18 // 18 in vanilla
#define MM_FEAT_COUNT_VANILLA 9

feat_enums metaMagicStandardFeats[MM_FEAT_COUNT_VANILLA] = {
	FEAT_EMPOWER_SPELL, FEAT_ENLARGE_SPELL, FEAT_EXTEND_SPELL, FEAT_HEIGHTEN_SPELL,
	FEAT_MAXIMIZE_SPELL, FEAT_QUICKEN_SPELL, FEAT_SILENT_SPELL, FEAT_STILL_SPELL,
	FEAT_WIDEN_SPELL,
};

constexpr int WEAP_COMBO_MAIN = 1;
constexpr int WEAP_COMBO_SECONDARY = 2;
constexpr int WEAP_COMBO_AMMO = 3;
constexpr int WEAP_COMBO_SHIELD = 4;

struct TigTextStyle;

struct SpellList
{
	SpellStoreData spells[SPELL_ENUM_MAX_VANILLA];
	uint32_t count;
public:
	void Remove(int idx);
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

struct UiMetaMagicData {
	SpellStoreData spellData;
	int availableCount;
	feat_enums availableMmFeats[MM_FEAT_COUNT_VANILLA];
	int appliedCount;
	feat_enums appliedMmFeats[MM_FEAT_COUNT_VANILLA];

public:
	void Clear();
	void Reset(objHndl handle, SpellStoreData &spell);
	void AddAppliedCount(feat_enums feat, int count);
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
	UiMetaMagicData* uiMetaMagicData;
	UiMetaMagicData* uiMetaMagicDataOrg;
	int* uiCharSpellsDraggedWidId;
	char** uiCharSpellFontName;
	int *uiCharSpellFontSize;

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
		rebase(uiMetaMagicData, 0x10C818B8);
		rebase(uiMetaMagicDataOrg, 0x10C816E0);
		
		rebase(uiCharSpellFontName, 0x10C81B78);
		rebase(uiCharSpellFontSize, 0x10C81B7C);
		
		rebase(uiCharSpellsDraggedWidId, 0x10C816CC);
	}
} uiCharAddresses;

class UiCharImpl {
	friend class CharUiSystem;
	friend class UiChar;
public:
	UiCharImpl();
protected:
	MesHandle& uiCharSpellsUiText = temple::GetRef<MesHandle>(0x10C81590);
};

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
	static UiCharSpellPacket& GetUiCharSpellPacket(int idx = -1);
	static UiCharSpellsNavPacket& GetUiCharSpellsNavPacket(int idx = -1);
	

	static BOOL MemorizeSpellMsg(int widId, TigMsg* tigMsg);
	static BOOL(*orgMemorizeSpellMsg)(int widId, TigMsg* tigMsg);

	static bool CharSpellsNavClassTabMsg(int widId, TigMsg* tigMsg);
	static bool(*orgCharSpellsNavClassTabMsg)(int widId, TigMsg* tigMsg);

	static void SpellbookSpellsRender(int widId, TigMsg& tigMsg);
	static BOOL SpellbookSpellsMsg(int widId, TigMsg* tigMsg){
		return orgSpellbookSpellsMsg(widId, tigMsg);
	};
	static BOOL(*orgSpellbookSpellsMsg)(int widId, TigMsg* tigMsg);

	static BOOL SpellMetamagicBtnMsg(int widId, TigMsg& tigMsg);
	static BOOL SpellPopupAppliedWndMsg(int widId, TigMsg& tigMsg);
	static void SpellMetamagicDotsRender(int x, int y, int mmData);

	static int specSlotIndices[NUM_SPELL_LEVELS]; // indices of the Specialization School spell slots in the GUI 
	static void SpellsShow(objHndl obj);
	static void(*orgSpellsShow)(objHndl obj);
	static BOOL IsSpecializationSchoolSlot(int idx);
	
	
	static int HookedCharSpellGetSpellbookScrollbarY();
	static int SpellMetamagicGetBtnIdx(int widId);

	static int SpellMetamagicAppliedGetIdx(int widId);
	static feat_enums SpellMetamagicAppliedGetFeat(int widId);
	static int SpellMetamagicAvailableGetIdx(int widId);
	static feat_enums SpellMetamagicAvailableGetFeat(int widId);

	static int SpellMetaMagicFeatGetSpellLevelModifier(feat_enums feat);
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

UiCharSpellsNavPacket& CharUiSystem::GetUiCharSpellsNavPacket(int idx)
{
	if (idx == -1) {
		idx = *uiCharAddresses.uiCharSpellsNavClassTabIdx;
	}
	return uiCharAddresses.uiCharSpellsNavPackets[idx];
}
UiCharSpellPacket& CharUiSystem::GetUiCharSpellPacket(int idx)
{
	if (idx == -1) {
		idx = *uiCharAddresses.uiCharSpellsNavClassTabIdx;
	}
	return uiCharAddresses.uiCharSpellPackets[idx];
}

BOOL CharUiSystem::MemorizeSpellMsg(int widId, TigMsg* tigMsg){

	auto& curSpellPacket = GetUiCharSpellPacket();

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

		auto navClassPackets = uiCharAddresses.uiCharSpellsNavPackets;
		auto spellClassCode = navClassPackets[uiCharSpellsNavClassTabIdxAfter].spellClassCode;
		auto classCode = spellSys.GetCastingClass(spellClassCode);
		auto& curCharSpellPkt = uiCharAddresses.uiCharSpellPackets[uiCharSpellsNavClassTabIdxAfter];

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

	auto navClassPackets = uiCharAddresses.uiCharSpellsNavPackets;
	auto charSpellPackets = uiCharAddresses.uiCharSpellPackets;

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
	uiManager->SetHidden((*uiCharAddresses.uiCharSpellsNavClassTabWnd)->widgetId, false);
	uiManager->SetHidden((*uiCharAddresses.uiCharSpellsSpellsPerDayWnd)->widgetId, false);
	uiManager->BringToFront(spellsMainWndId);
	uiManager->BringToFront((*uiCharAddresses.uiCharSpellsSpellsPerDayWnd)->widgetId);

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
		uiManager->BringToFront((*uiCharAddresses.uiCharSpellsNavClassTabWnd)->widgetId);
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

	uiManager->BringToFront((*uiCharAddresses.uiCharSpellsNavClassTabWnd)->widgetId);


	auto charSpellPkts = &uiCharAddresses.uiCharSpellPackets[0];
	
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
	auto scrollbar = GetUiCharSpellPacket().spellbookScrollbar;
	if (scrollbar) {
		return (scrollbar->GetY());
	}
	logger->warn("Null scrollbar! Returning y=0");
	return 0;
}

/* 0x101B6FD0 */
void CharUiSystem::SpellbookSpellsRender(int widId, TigMsg& tigMsg)
{
	auto handle = GetCurrentCritter();
	auto obj = objSystem->GetObject(handle); if (!obj) return;
	auto& uiCharSpellPkt = GetUiCharSpellPacket();
	auto& uiCharSpellNav = GetUiCharSpellsNavPacket();

	auto wizSpec = spellSys.getWizSchool(handle);
	if (spellSys.GetCastingClass(uiCharSpellNav.spellClassCode) != stat_level_wizard) {
		wizSpec = 0u;
	}

	if (uiSystems->GetChar().GetInventoryObjectState() == 1 && *uiCharAddresses.uiCharSpellsDraggedWidId  == widId) {
		return;
	}

	auto wnd = uiManager->GetWindow(widId); if (!wnd) return;
	
	UiRenderer::PushFont(*uiCharAddresses.uiCharSpellFontName, *uiCharAddresses.uiCharSpellFontSize);
	// look up widget's idx in list
	auto widgetIdx = -1;
	for (auto i = 0; i < NUM_SPELLBOOK_SLOTS; ++i) {
		if (uiCharSpellPkt.spellbookSpellWnds[i]->widgetId == widId) {
			widgetIdx = i;
			break;
		}
	}

	auto scrollbarY = 0;
	if (uiManager->ScrollbarGetY(uiCharSpellPkt.spellbookScrollbar->widgetId, &scrollbarY)) {
		scrollbarY = -1; // fail case
	}
	auto &spData = uiCharSpellPkt.spellsKnown.spells[widgetIdx + scrollbarY];


	auto alignOpposed = spellSys.SpellOpposesCritterAlignment(spData, handle);
	
	TigTextStyle style;
	auto txtR = temple::GetRef<int>(0x10C81B88);
	auto txtG = temple::GetRef<int>(0x10C81B8C);
	auto txtB = temple::GetRef<int>(0x10C81B90);
	auto txtA = temple::GetRef<int>(0x10C81B94);

	ColorRect textColor = !alignOpposed ? 
		ColorRect( XMCOLOR((float)txtR, (float)txtG, (float)txtB, (float)txtA))
		: ColorRect(XMCOLOR(0xFF5D5D5D));
	ColorRect shadowColor(XMCOLOR(0, 0, 0, 255));
	ColorRect spellLabelColor(XMCOLOR(0xFF4D7197));
	ColorRect wizSpecColor(XMCOLOR(0xFFFFFF80));
	style.textColor = &textColor;
	style.shadowColor = &shadowColor;
	style.flags = 0x4008; // drop shadow + truncate too long text with ellipsis
	style.kerning = 1;
	style.tracking = 2;

	if (!spData.spellEnum) { // spell label

		if ((int)spData.spellLevel > -1) {
			style.textColor = &spellLabelColor;
			MesLine mesLine;
			mesFuncs.ReadLineDirect(uiCharImpl->uiCharSpellsUiText, 3, &mesLine);
			auto text = fmt::format("{} {}", mesLine.value, spData.spellLevel);
			auto rect = TigRect(-8, 0, wnd->width, wnd->height);
			UiRenderer::DrawTextInWidget(widId, text, rect, style);
			UiRenderer::PopFont();
			return;
		}
	}
	else if (spData.spellEnum > 0) {
		std::string text;
		if (spellSys.isDomainSpell(spData.classCode)) {
			text = fmt::format("{} ({})", spellSys.GetSpellMesline(spData.spellEnum), spellSys.GetDomainName( spData.classCode) );
		}
		else {
			if (wizSpec && spellSys.GetSpellSchool(spData.spellEnum) == wizSpec) {
				style.textColor = &wizSpecColor;
			}
			text = fmt::format("{}", spellSys.GetSpellMesline(spData.spellEnum));
		}
		
		auto rect = TigRect(0, 0, wnd->width, wnd->height);
		UiRenderer::DrawTextInWidget(widId, text, rect, style);

		auto measRect = UiRenderer::MeasureTextSize(text, style);
		SpellMetamagicDotsRender(wnd->x + rect.x + measRect.width + 2, wnd->y + 2, spData.metaMagicData);

		UiRenderer::PopFont();
		return;

	}
	else if (spData.spellEnum == -1 && spData.spellLevel > -1) {
		// does this ever happen??
	}
	
	
	UiRenderer::PopFont();

	
}

/* Originally 0x101BA580 */
BOOL CharUiSystem::SpellMetamagicBtnMsg(int widId, TigMsg& tigMsg)
{
	auto msgType = tigMsg.type;
	if (msgType != TigMsgType::WIDGET) {
		return TRUE;
	}
	auto& msgWidget = (TigMsgWidget&)tigMsg;
	if (msgWidget.widgetEventType != TigMsgWidgetEvent::MouseReleased) {
		return TRUE;
	}
	auto handle = uiSystems->GetChar().GetCritter();
	if (!feats.HasMetamagicFeat(handle)) {
		return TRUE;
	}

	auto widIdx = SpellMetamagicGetBtnIdx(widId);
	auto scrollbarY = HookedCharSpellGetSpellbookScrollbarY(); // fixed bug in vanilla - would always get the first tab
	auto spellBtnIdx = scrollbarY + widIdx;

	auto& uiCharSpellPkt = GetUiCharSpellPacket();
	auto spellEnum = uiCharSpellPkt.spellsKnown.spells[spellBtnIdx].spellEnum;
	if (spellEnum <= 0)
		return TRUE;

	logger->debug("metamagic button#{} for spell {} pressed!", spellBtnIdx, spellSys.GetSpellMesline(spellEnum));
	
	// Init uiMetaMagicData
	auto& mmData = *uiCharAddresses.uiMetaMagicData;
	auto& spell = uiCharSpellPkt.spellsKnown.spells[spellBtnIdx];
	mmData.Reset(handle, spell);
	if (mmData.appliedCount)
		memcpy(uiCharAddresses.uiMetaMagicDataOrg, uiCharAddresses.uiMetaMagicData, /*sizeof(UiMetaMagicData)*/ 212);
	
	UiPromptPacket uiPrompt;
	uiPrompt.idx = 1;

	uiPrompt.image = temple::GetRef<int*>(0x10C81594)[0];
	uiPrompt.btnNormalTexture   = temple::GetRef<int>(0x10C815C4);
	uiPrompt.btn2NormalTexture  = temple::GetRef<int>(0x10C81598);
	uiPrompt.btnHoverTexture    = temple::GetRef<int>(0x10C816C8);
	uiPrompt.btn2HoverTexture   = temple::GetRef<int>(0x10C816D0);
	uiPrompt.btnPressedTexture  = temple::GetRef<int>(0x10C81BBC);
	uiPrompt.btn2PressedTexture = temple::GetRef<int>(0x10C816D4);
	uiPrompt.wndRect            = temple::GetRef<TigRect>(0x10C81AA0);
	uiPrompt.okRect             = temple::GetRef<TigRect>(0x10C81AB0);
	uiPrompt.cancelRect         = temple::GetRef<TigRect>(0x10C81AC0);
	uiPrompt.onPopupShow        = temple::GetRef<int(__cdecl)()>(0x101B51F0);
	uiPrompt.onPopupHide        = temple::GetRef<void(__cdecl)()>(0x101B52A0);
	uiPrompt.callback           = temple::GetRef<void(__cdecl)(int)>(0x101B5310);
	uiPrompt.okBtnText          = combatSys.GetCombatMesLine(6009);
	uiPrompt.cancelBtnText      = combatSys.GetCombatMesLine(6010);
	uiPrompt.Show(1, 0);

	return TRUE;
}

BOOL CharUiSystem::SpellPopupAppliedWndMsg(int widId, TigMsg& tigMsg)
{
	auto& uiMmData = *uiCharAddresses.uiMetaMagicData;
	auto& uiMmDataMod = *uiCharAddresses.uiMetaMagicDataOrg;

	if (tigMsg.type == TigMsgType::MOUSE) { // remove MM effect
		auto msgMouse = (TigMsgMouse&)tigMsg;
		if (msgMouse.buttonStateFlags & MouseStateFlags::MSF_LMB_CLICK)
			return TRUE;
		if (!(msgMouse.buttonStateFlags & (MSF_LMB_RELEASED | MSF_RMB_RELEASED)))
			return TRUE;
		if (uiSystems->GetChar().GetInventoryObjectState() == 1)
			return TRUE;
		auto feat = SpellMetamagicAppliedGetFeat(widId);
		if (feat >= FEAT_NONE)
			return TRUE;
		logger->debug("{} pressed", feats.GetFeatName(feat));
		auto idx = SpellMetamagicAppliedGetIdx(widId) 
			      + temple::GetRef<int>(0x10C81988); // probably some deprecated scrollbar, seems to be always 0
		uiMmData.spellData.spellLevel -= SpellMetaMagicFeatGetSpellLevelModifier(feat);
		switch (feat) {
			case FEAT_EMPOWER_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicEmpowerSpellCount -= 1;
				break;
			case FEAT_ENLARGE_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicEnlargeSpellCount -= 1;
				break;
			case FEAT_EXTEND_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicExtendSpellCount -= 1;
				break;
			case FEAT_HEIGHTEN_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicHeightenSpellCount -= 1;
				break;
			case FEAT_MAXIMIZE_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicFlags &= ~(MetaMagicFlags::MetaMagic_Maximize);
				break;
			case FEAT_QUICKEN_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicFlags &= ~(MetaMagicFlags::MetaMagic_Quicken);
				break;
			case FEAT_SILENT_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicFlags &= ~(MetaMagicFlags::MetaMagic_Silent);
				break;
			case FEAT_STILL_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicFlags &= ~(MetaMagicFlags::MetaMagic_Still);
				break;
			case FEAT_WIDEN_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicWidenSpellCount -= 1;
				break;
			default:
				break;
		}
		if (uiMmData.appliedCount <= 1) {
			uiMmData.appliedMmFeats[idx] = FEAT_NONE;
		}
		else {
			if (idx == uiMmData.appliedCount - 1) {
				uiMmData.appliedMmFeats[idx] = FEAT_NONE;
			}
			else if (idx < uiMmData.appliedCount -1){
				uiMmData.appliedMmFeats[idx] = uiMmData.appliedMmFeats[uiMmData.appliedCount - 1];
			}
		}
		uiMmData.appliedCount--;
		if (!uiMmData.appliedCount) {
			auto &uiCharSpellPkt = GetUiCharSpellPacket();
			auto knownCount = uiCharSpellPkt.spellsKnown.count;
			for (auto i = 0u; i < knownCount; ++i) {
				auto& knSpell = uiCharSpellPkt.spellsKnown.spells[i];
				if (knSpell.spellLevel != uiMmDataMod.spellData.spellLevel
					|| knSpell.spellEnum != uiMmDataMod.spellData.spellEnum) {
					continue;
				}
				// found spell with same enum and spell level
				if (uiMmDataMod.spellData.spellEnum == 0)
					break;
				// is this buggy? removes spell with same level & enum, with no regard to MM modifiers
				spellSys.SpellKnownRemove( GetCurrentCritter(), uiMmDataMod.spellData);
				uiCharSpellPkt.spellsKnown.Remove(i);
				break;
			}
		}
		return TRUE;
	}

	else if (tigMsg.type == TigMsgType::WIDGET) {
		auto msgWid = (TigMsgWidget&)tigMsg;

		auto& spellFeat = temple::GetRef<int>(0x10300258);
		auto& mmAvailWnds = temple::GetRef<LgcyWindow* [9]>(0x10C81934);
		
		auto widEvt = msgWid.widgetEventType;
		switch (widEvt) {
		case TigMsgWidgetEvent::Clicked:
		case TigMsgWidgetEvent::MouseReleased :
		case TigMsgWidgetEvent::MouseReleasedAtDifferentButton :
			break;
		case TigMsgWidgetEvent::Entered :
			spellFeat = FEAT_NONE;
			return FALSE;
		case TigMsgWidgetEvent::Exited :
		case TigMsgWidgetEvent::Scrolled:
		default:
			return FALSE;
		}
		auto widIdOrg = msgWid.widgetId;
		if (widIdOrg == widId)
			return FALSE;
		logger->debug("CharUiSystem::SpellPopupAppliedWndMsg: Mouse up =( {} )", widId);
		auto availIdx = SpellMetamagicAvailableGetIdx(widIdOrg);
		auto appliedIdx = SpellMetamagicAppliedGetIdx(widId);
		if (appliedIdx < 0 || availIdx < 0) {
			return TRUE;
		}
		appliedIdx += temple::GetRef<int>(0x10C81988); // probably some deprecated scrollbar, seems to be always 0
		if (appliedIdx < 0) return FALSE;
		if (appliedIdx > uiMmData.appliedCount) appliedIdx = uiMmData.appliedCount;
		auto appliedFeat = SpellMetamagicAppliedGetFeat(widId);
		auto availFeat   = SpellMetamagicAvailableGetFeat(widIdOrg);

		// Check if effect can be applied only once (on/off)
		for (auto i = 0; i < uiMmData.appliedCount; ++i) {
			if (uiMmData.appliedMmFeats[i] == availFeat) {
				// Vanilla only allowed Heighten to pass through
				if (availFeat == FEAT_HEIGHTEN_SPELL) {
					continue;
				}
				// In Moebius/Co8 DLL
				// It looks like Moebius tried to expand this in his stacking MM
				// mod. Widen was left out however (he tested feat enum with & 0x100)
				// Since this is not RAW, this has been converted to a House Rules option.
				if (config.metamagicStacking && 
					(availFeat == FEAT_EXTEND_SPELL ||
					availFeat == FEAT_ENLARGE_SPELL ||
					availFeat == FEAT_EMPOWER_SPELL ||
					availFeat == FEAT_WIDEN_SPELL) ){
						continue;
				}
				return TRUE;
			}
		}

		if (appliedFeat != FEAT_NONE) { // replacing an existing effect
			uiMmData.spellData.spellLevel -= SpellMetaMagicFeatGetSpellLevelModifier(appliedFeat);
			switch (appliedFeat) {
			case FEAT_EMPOWER_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicEmpowerSpellCount -= 1;
				break;
			case FEAT_ENLARGE_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicEnlargeSpellCount -= 1;
				break;
			case FEAT_EXTEND_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicExtendSpellCount -= 1;
				break;
			case FEAT_HEIGHTEN_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicHeightenSpellCount -= 1;
				break;
			case FEAT_MAXIMIZE_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicFlags &= ~(MetaMagicFlags::MetaMagic_Maximize);
				break;
			case FEAT_QUICKEN_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicFlags &= ~(MetaMagicFlags::MetaMagic_Quicken);
				break;
			case FEAT_SILENT_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicFlags &= ~(MetaMagicFlags::MetaMagic_Silent);
				break;
			case FEAT_STILL_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicFlags &= ~(MetaMagicFlags::MetaMagic_Still);
				break;
			case FEAT_WIDEN_SPELL:
				uiMmData.spellData.metaMagicData.metaMagicWidenSpellCount -= 1;
				break;
			default:
				break;
			}
		}

		auto spellLevelNew = uiMmData.spellData.spellLevel + SpellMetaMagicFeatGetSpellLevelModifier(availFeat);
		if (spellLevelNew > NUM_SPELL_LEVELS-1) {
			return TRUE;
		}
		uiMmData.spellData.spellLevel = spellLevelNew;
		switch (availFeat) {
		case FEAT_EMPOWER_SPELL:
			uiMmData.spellData.metaMagicData.metaMagicEmpowerSpellCount += 1;
			break;
		case FEAT_ENLARGE_SPELL:
			uiMmData.spellData.metaMagicData.metaMagicEnlargeSpellCount += 1;
			break;
		case FEAT_EXTEND_SPELL:
			uiMmData.spellData.metaMagicData.metaMagicExtendSpellCount  += 1;
			break;
		case FEAT_HEIGHTEN_SPELL:
			uiMmData.spellData.metaMagicData.metaMagicHeightenSpellCount += 1;
			break;
		case FEAT_MAXIMIZE_SPELL:
			uiMmData.spellData.metaMagicData.metaMagicFlags |= (MetaMagicFlags::MetaMagic_Maximize);
			break;
		case FEAT_QUICKEN_SPELL:
			uiMmData.spellData.metaMagicData.metaMagicFlags |= (MetaMagicFlags::MetaMagic_Quicken);
			break;
		case FEAT_SILENT_SPELL:
			uiMmData.spellData.metaMagicData.metaMagicFlags |= (MetaMagicFlags::MetaMagic_Silent);
			break;
		case FEAT_STILL_SPELL:
			uiMmData.spellData.metaMagicData.metaMagicFlags |= (MetaMagicFlags::MetaMagic_Still);
			break;
		case FEAT_WIDEN_SPELL:
			uiMmData.spellData.metaMagicData.metaMagicWidenSpellCount += 1;
			break;
		default:
			break;
		}
		uiMmData.appliedCount++;
		uiMmData.appliedMmFeats[appliedIdx] = availFeat;
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/* 0x101B5740 signature changed*/
void CharUiSystem::SpellMetamagicDotsRender(int x, int y, int mmDataRaw)
{
	auto& spellsTextures = temple::GetRef<int*>(0x10C816D8);
	MetaMagicData mmData(mmDataRaw);

	constexpr int DOT_SIZE = 10;
	const TigRect SRC_RECT(0, 0, DOT_SIZE, DOT_SIZE);
	TigRect destRect(x, y, DOT_SIZE, DOT_SIZE);
	
	if (mmData.metaMagicEmpowerSpellCount > 0) {
		UiRenderer::DrawTexture(spellsTextures[0], destRect, SRC_RECT);
		destRect.x += DOT_SIZE;
	}
	if (mmData.metaMagicEnlargeSpellCount > 0) {
		UiRenderer::DrawTexture(spellsTextures[1], destRect, SRC_RECT);
		destRect.x += DOT_SIZE;
	}
	if (mmData.metaMagicExtendSpellCount > 0) {
		UiRenderer::DrawTexture(spellsTextures[2], destRect, SRC_RECT);
		destRect.x += DOT_SIZE;
	}if (mmData.metaMagicHeightenSpellCount> 0) {
		UiRenderer::DrawTexture(spellsTextures[4], destRect, SRC_RECT);
		destRect.x += DOT_SIZE;
	}
	if (mmData.metaMagicFlags & MetaMagicFlags::MetaMagic_Maximize) {
		UiRenderer::DrawTexture(spellsTextures[4], destRect, SRC_RECT);
		destRect.x += DOT_SIZE;
	}
	if (mmData.metaMagicFlags & MetaMagicFlags::MetaMagic_Quicken) {
		UiRenderer::DrawTexture(spellsTextures[5], destRect, SRC_RECT);
		destRect.x += DOT_SIZE;
	}
	if (mmData.metaMagicFlags & MetaMagicFlags::MetaMagic_Silent) {
		UiRenderer::DrawTexture(spellsTextures[6], destRect, SRC_RECT);
		destRect.x += DOT_SIZE;
	}
	if (mmData.metaMagicFlags & MetaMagicFlags::MetaMagic_Still) {
		UiRenderer::DrawTexture(spellsTextures[7], destRect, SRC_RECT);
		destRect.x += DOT_SIZE;
	}
	if (mmData.metaMagicWidenSpellCount> 0) {
		UiRenderer::DrawTexture(spellsTextures[8], destRect, SRC_RECT);
		destRect.x += DOT_SIZE;
	}
}

int CharUiSystem::SpellMetamagicGetBtnIdx(int widId)
{
	auto& uiCharSpellPacket = GetUiCharSpellPacket();
	for (auto i = 0; i < NUM_SPELLBOOK_SLOTS; ++i) {
		if (uiCharSpellPacket.metamagicButtons[i]->widgetId == widId)
			return i;
	}
	return -1;
}

int CharUiSystem::SpellMetamagicAppliedGetIdx(int widId)
{
	auto& popupAppliedWnds = temple::GetRef<LgcyWindow* [MM_FEAT_COUNT_VANILLA]>(0x10C81958);
	for (auto i = 0; i < MM_FEAT_COUNT_VANILLA; ++i) {
		if (popupAppliedWnds[i]->widgetId == widId)
			return i;
	}

	return -1;
}

int CharUiSystem::SpellMetamagicAvailableGetIdx(int widId)
{
	auto& popupAvailWnds = temple::GetRef<LgcyWindow* [MM_FEAT_COUNT_VANILLA]>(0x10C81934);
	for (auto i = 0; i < MM_FEAT_COUNT_VANILLA; ++i) {
		if (popupAvailWnds[i]->widgetId == widId)
			return i;
	}

	return -1;
}

feat_enums CharUiSystem::SpellMetamagicAvailableGetFeat(int widId)
{
	auto& uiMmData = *uiCharAddresses.uiMetaMagicData;
	auto& popupAvailWnds = temple::GetRef<LgcyWindow* [MM_FEAT_COUNT_VANILLA]>(0x10C81934);
	for (auto i = 0; i < MM_FEAT_COUNT_VANILLA && i < uiMmData.availableCount; ++i) {
		if (popupAvailWnds[i]->widgetId == widId) {
			return uiMmData.availableMmFeats[i];
		}
			
	}

	return FEAT_NONE;
}

/* 0x101B5590 */
feat_enums CharUiSystem::SpellMetamagicAppliedGetFeat(int widId)
{
	auto& uiMmData = *uiCharAddresses.uiMetaMagicData;
	auto& popupAppliedWnds = temple::GetRef<LgcyWindow* [MM_FEAT_COUNT_VANILLA]>(0x10C81958);
	for (auto i = 0; i < MM_FEAT_COUNT_VANILLA && i < uiMmData.appliedCount; ++i) {
		if (popupAppliedWnds[i]->widgetId == widId)
			return uiMmData.appliedMmFeats[i];
	}

	return FEAT_NONE;
}

int CharUiSystem::SpellMetaMagicFeatGetSpellLevelModifier(feat_enums feat)
{
	switch (feat) {
	case FEAT_EMPOWER_SPELL:
		return 2;
	case FEAT_ENLARGE_SPELL:
	case FEAT_EXTEND_SPELL:
	case FEAT_HEIGHTEN_SPELL:
		return 1;
	case FEAT_MAXIMIZE_SPELL:
		return 3;
	case FEAT_QUICKEN_SPELL:
		return 4;
	case FEAT_SILENT_SPELL:
		return 1;
	case FEAT_STILL_SPELL:
		return 1;
	case FEAT_WIDEN_SPELL:
		return 3;
	default:
		return 0;
	}

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
	mImpl = std::make_unique<UiCharImpl>();
	uiCharImpl = mImpl.get();
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

	// Spells

	//orgSpellbookSpellsMsg = replaceFunction(0x101B8F10, SpellbookSpellsMsg);
	orgMemorizeSpellMsg = replaceFunction(0x101B9360, MemorizeSpellMsg);
	orgCharSpellsNavClassTabMsg = replaceFunction(0x101B8B60, CharSpellsNavClassTabMsg);
	redirectCall(0x101B6ED8, TabNameReplacement);
	replaceFunction(0x101B2EE0, IsSpecializationSchoolSlot);
	orgSpellsShow = replaceFunction(0x101B5D80, SpellsShow);
	replaceFunction(0x101B6FD0, SpellbookSpellsRender);

	static BOOL(__cdecl* orgMetamagicBtnMsg)(int, TigMsg&) = replaceFunction<BOOL(__cdecl)(int, TigMsg&)>(0x101BA580, [](int widId, TigMsg& msg) {
		return SpellMetamagicBtnMsg(widId, msg);
		// return orgMetamagicBtnMsg(widId, msg);

	});

	static BOOL(__cdecl * orgMetamagicAppliedWngMsg)(int, TigMsg&) = replaceFunction<BOOL(__cdecl)(int, TigMsg&)>(0x101B9880, [](int widId, TigMsg& msg) {
		return SpellPopupAppliedWndMsg(widId, msg);
		// return orgMetamagicAppliedWngMsg(widId, msg);
		});

	// UiCharSpellGetScrollbarY  has bug when called from Spellbook, it receives the first tab's scrollbar always
	writeCall(0x101BA5D9, HookedCharSpellGetSpellbookScrollbarY);

	writeNoops(0x101B957E); // so it doesn't decrement the spells memorized num (this causes weirdness in right clicking from the spellbook afterwards)

	// UiCharSpellPopupInterfaceCallback hook
	// Fixes issue when adding MM effect that is at the highest known spell level or higher.
	// Previously it would add it at the top of the spell list. It's just a visual glitch since re-entering the spell book
	// fixed this, but it was still confusing.
	// There's still an issue when the new spell level is 9, in that the game doesn't add a Spell Level 9 label, but at least
	// it's at the bottom.
	static void(__cdecl * orgMetamagicCallback)(int) = replaceFunction<void(__cdecl)(int)>(0x101B5310, [](int popupBtnIdx) {

		

		auto& navClassTabIdx = temple::GetRef<int>(0x10D18F68);
		auto charSpellPackets = uiCharAddresses.uiCharSpellPackets;
		auto &curSpellList = charSpellPackets[navClassTabIdx].spellsKnown;
		auto orgFirstSpellLevel = curSpellList.spells[0].spellLevel;
		auto& uiMmData    = *uiCharAddresses.uiMetaMagicData;
		auto &uiMmDataMod = *uiCharAddresses.uiMetaMagicDataOrg;

		orgMetamagicCallback(popupBtnIdx);
		/*
		auto idxToInsert = 0;
		if (popupBtnIdx) {
			logger->debug("CharUiSystem::MetamagicCallback: no metamagic spell created");
		}
		else {
			auto& spell = uiMmData.spellData;
			logger->debug("CharUiSystem::MetamagicCallback: spell {}, new_level {}", spellSys.GetSpellMesline(spell.spellEnum), spell.spellLevel );

			if (spell.metaMagicData == 0u) {
				logger->debug("CharUiSystem::MetamagicCallback: no metamagic spell created");
			}
			else {
				if (uiMmDataMod.appliedCount == uiMmData.appliedCount) {
				 blah blah refactor
				}
			}
		}

		uiMmDataMod.Clear();
		*/

		
		auto &spellCount = curSpellList.count;
		auto &firstSpell = curSpellList.spells[0]; // if the MM callback fucks up, the new MM spell ends up here
		if (firstSpell.spellLevel > orgFirstSpellLevel) {

			auto& lastSpell = curSpellList.spells[spellCount - 1];
			
			if (firstSpell.spellLevel > lastSpell.spellLevel) {
				// add spell level label
				SpellStoreData spellLabel(0, firstSpell.spellLevel, firstSpell.classCode, 0,0);
				curSpellList.spells[spellCount++] = spellLabel;
				
			}
			auto newSpell = firstSpell;
			memcpy(&curSpellList.spells[0], &curSpellList.spells[1], sizeof(SpellStoreData) * spellCount);
			curSpellList.spells[spellCount - 1] = newSpell;
		}
		});
	
	
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

void UiMetaMagicData::Clear()
{
	memset(this, 0, sizeof(UiMetaMagicData));
	for (auto i = 0; i < MM_FEAT_COUNT_VANILLA; ++i) {
		appliedMmFeats[i] = availableMmFeats[i] = FEAT_NONE;
	}
}

void UiMetaMagicData::Reset(objHndl handle, SpellStoreData& spell)
{
	availableCount = appliedCount = 0;
	for (auto i = 0; i < MM_FEAT_COUNT_VANILLA; ++i) {
		appliedMmFeats[i] = FEAT_NONE;
		availableMmFeats[i] = FEAT_NONE;
	}
	spellData = spell;

	//for (auto feat : feats.metamagicFeats) {
	for (auto i = 0; i < MM_FEAT_COUNT_VANILLA; ++i) {
		auto feat = metaMagicStandardFeats[i];
		if (feat == FEAT_SILENT_SPELL && spellData.classCode == spellSys.GetSpellClass(stat_level_bard))
			continue;
		if (feats.HasFeatCountByClass(handle, feat)) { // fixed issue in vanilla - checked exactly == 1, but sometimes you got 2 here (likely due to the feat right click selection bug :P)
			availableMmFeats[availableCount++] = feat;
		}
	}

	auto mmData = spellData.metaMagicData;
	if (mmData.metaMagicFlags & MetaMagicFlags::MetaMagic_Maximize) {
		appliedMmFeats[appliedCount++] = FEAT_MAXIMIZE_SPELL;
	}
	if (mmData.metaMagicFlags & MetaMagicFlags::MetaMagic_Quicken) {
		appliedMmFeats[appliedCount++] = FEAT_QUICKEN_SPELL;
	}
	if (mmData.metaMagicFlags & MetaMagicFlags::MetaMagic_Silent) {
		appliedMmFeats[appliedCount++] = FEAT_QUICKEN_SPELL; // hmmm? maybe Moebius changed this
	}
	if (mmData.metaMagicFlags & MetaMagicFlags::MetaMagic_Still) {
		appliedMmFeats[appliedCount++] = FEAT_QUICKEN_SPELL; // hmmm? maybe Moebius changed this
	}
	
	AddAppliedCount(FEAT_EMPOWER_SPELL, mmData.metaMagicEmpowerSpellCount);
	AddAppliedCount(FEAT_ENLARGE_SPELL, mmData.metaMagicEnlargeSpellCount);
	AddAppliedCount(FEAT_EXTEND_SPELL, mmData.metaMagicExtendSpellCount);
	AddAppliedCount(FEAT_HEIGHTEN_SPELL, mmData.metaMagicHeightenSpellCount);
	AddAppliedCount(FEAT_WIDEN_SPELL, mmData.metaMagicWidenSpellCount & 3);
	
}

void UiMetaMagicData::AddAppliedCount(feat_enums feat, int count)
{
	for (auto i = 0; i < count && appliedCount < MM_FEAT_COUNT_VANILLA; ++i) {
		appliedMmFeats[appliedCount++] = feat;
	}
}

void SpellList::Remove(int idx)
{
	if (idx >= count || idx < 0)
		return;

	for (auto i=idx; i < count-1; ++i ){
		spells[i] = spells[i + 1];
	}

	auto& spell = spells[count - 1];
	memset(&spell, 0, sizeof(spell));
	spell.spellLevel = -1;
	count--;
	return;
	
}

UiCharImpl::UiCharImpl()
{
	uiCharImpl = this;
}
