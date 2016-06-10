#include "stdafx.h"
#include "common.h"
#include "feat.h"
#include "ui_char_editor.h"
#include "obj.h"
#include "ui/ui.h"
#include "util/fixes.h"
#include <tig/tig_texture.h>
#include <tig/tig_font.h>
#include <critter.h>
#include <EASTL/hash_map.h>
#include "ui_render.h"
#include <EASTL/fixed_string.h>
#include <gamesystems/d20/d20stats.h>
#include <gamesystems/gamesystems.h>
#include <gamesystems/objects/objsystem.h>
#include <d20_level.h>

class UiCharEditor{
friend class UiCharEditorHooks;
	objHndl GetEditedChar();
	CharEditorSelectionPacket & GetCharEditorSelPacket();
	
	BOOL WidgetsInit();
	BOOL SystemInit(GameSystemConf & conf);


	void SpellsActivate();

	int &GetState();


	void StateTitleRender(int widId);

	// widget IDs
	int classWndId=0;

	// caches
	eastl::hash_map<int, eastl::string> classNamesUppercase;
	eastl::hash_map<int, WidgetType2> classBtns;
	eastl::vector<int> classBtnIds;
	eastl::vector<TigRect> classBtnFrameRects;

	// art assets
	int buttonBox =0;
	ColorRect classBtnShadowColor = ColorRect(0xFF000000);
	ColorRect classBtnColorRect = ColorRect(0xFFFFffff);
	TigTextStyle classBtnTextStyle;


	CharEditorClassSystem& GetClass() const {
		Expects(!!mClass);
		return *mClass;
	}

private:
	std::unique_ptr<CharEditorClassSystem> mClass;
	//std::unique_ptr<CharEditorStatsSystem> mStats;
	//std::unique_ptr<CharEditorFeaturesSystem> mFeatures;
	//std::unique_ptr<CharEditorSkillsSystem> mSkills;
	//std::unique_ptr<CharEditorFeatsSystem> mFeats;
	//std::unique_ptr<CharEditorSpellsSystem> mSpells;
} uiCharEditor;

objHndl UiCharEditor::GetEditedChar()
{
	return temple::GetRef<objHndl>(0x11E741A0);
}

CharEditorSelectionPacket& UiCharEditor::GetCharEditorSelPacket(){
	return temple::GetRef<CharEditorSelectionPacket>(0x11E72F00);
}

BOOL UiCharEditor::WidgetsInit(){
	static WidgetType1 classWnd(259,117, 405, 271);
	classWnd.widgetFlags = 1;
	classWnd.render = [](int widId) { uiCharEditor.StateTitleRender(widId); };
	if (classWnd.Add(&classWndId))
		return 0;
	int coloff = 0, rowoff = 0;
	for (auto it: d20ClassSys.vanillaClassEnums){
		
		int newId = 0;
		WidgetType2 classBtn("Class btn", classWndId, 81 + coloff, 47 + rowoff, 110, 20);
		coloff = 119 - coloff;
		if (!coloff)
			rowoff += 29;
		classBtn.Add(&newId);
		classBtnFrameRects.push_back(TigRect(classBtn.x-5, classBtn.y-5, classBtn.width+10, classBtn.height+10));
		UiRenderer::PushFont(PredefinedFont::PRIORY_12);
		auto classMeasure = UiRenderer::MeasureTextSize(classNamesUppercase[it].c_str(), classBtnTextStyle);

		UiRenderer::PopFont();
	}

	return TRUE;
}

BOOL UiCharEditor::SystemInit(GameSystemConf& conf){
	if (textureFuncs.RegisterTexture("art\\interface\\pc_creation\\buttonbox.tga", &buttonBox))
		return 0;

	classBtnTextStyle.flags = 8;
	classBtnTextStyle.field2c = -1;
	classBtnTextStyle.textColor = &classBtnColorRect;
	classBtnTextStyle.shadowColor = &classBtnShadowColor;
	classBtnTextStyle.colors4 = &classBtnColorRect;
	classBtnTextStyle.colors2 = &classBtnColorRect;
	classBtnTextStyle.field0 = 0;
	classBtnTextStyle.kerning = 1;
	classBtnTextStyle.leading = 0;
	classBtnTextStyle.tracking = 3;

	for (auto it: d20ClassSys.vanillaClassEnums){
		auto className = _strdup(d20Stats.GetStatName(it));
		classNamesUppercase[it] = className;
		for (auto &letter: classNamesUppercase[it]){
			letter = toupper(letter);
		}
	}

	return WidgetsInit();
}

void UiCharEditor::SpellsActivate(){
	auto handle = GetEditedChar();
	auto obj = gameSystems->GetObj().GetObject(handle);
	auto &selPkt = GetCharEditorSelPacket();

	

	// get the new caster level for the levelled class (1 indicates a newly taken class)
	auto casterLvlNew = 1;
	auto classLeveled = selPkt.classCode;
	auto lvls = obj->GetInt32Array(obj_f_critter_level_idx);
	for (auto i = 0u; i < lvls.GetSize(); i++){
		auto classCode = static_cast<Stat>(lvls[i]);
		if (classLeveled == classCode) // is the same the one being levelled
			casterLvlNew++;
	}

	auto &needPopulateEntries = temple::GetRef<int>(0x10C4D4C4);

	static auto setScrollbars = []() {
		auto sbId = temple::GetRef<int>(0x10C737C0);
		ui.ScrollbarSetY(sbId, 0);
		auto numEntries = temple::GetRef<int>(0x10C75A60);
		ui.ScrollbarSetYmax(sbId, max(0, numEntries - 20));
		temple::GetRef<int>(0x10C73C34) = 0; // scrollbarY
		ui.WidgetCopy(sbId, &temple::GetRef<WidgetType3>(0x10C736C0));

		auto &charEdSelPkt = uiCharEditor.GetCharEditorSelPacket();
		auto sbAddedId = temple::GetRef<int>(0x10C4D548);
		ui.ScrollbarSetY(sbAddedId, 0); ui.ScrollbarSetYmax(sbAddedId, max(0, charEdSelPkt.spellEnumsAddedCount - 20));
		temple::GetRef<int>(0x10C75BC0) = 0; // scrollbarY
		ui.WidgetCopy(sbAddedId, &temple::GetRef<WidgetType3>(0x10C75AC0));
	};

	if (!needPopulateEntries) {
		setScrollbars();
		return;
	}


	auto & spellFlags = temple::GetRef<char[802]>(0x10C72F20);
	memset(spellFlags, 0, sizeof(spellFlags));
	selPkt.spellEnumToRemove = 0;


	auto isNewClass = casterLvlNew == 1; // todo: generalize for PrC's

	// newly taken class
	if (isNewClass){
		
		// let natural casters choose 4 cantrips
		auto count = 0;
		// todo: generalize to handle new base classes
		if (classLeveled == stat_level_bard || classLeveled == stat_level_sorcerer ){
			selPkt.spellEnums[count++] = 803; // Spell Level 0 label
			for (int i = 1; i < 5; i++)	{
				selPkt.spellEnums[count] = 802;
				spellFlags[count++] = 3;
			}
				
			selPkt.spellEnumsAddedCount = count;
		}
		
		// add 2 level 1 spells for wiz/sorc
		// todo: generalize to handle new base classes
		if (classLeveled == stat_level_wizard || classLeveled == stat_level_sorcerer){
			selPkt.spellEnums[count++] = 804; // Spell Level 1 label
			for (int i = 0; i < 2; i++) {
				selPkt.spellEnums[count] = 802;
				spellFlags[count++] = 3;
			}
			selPkt.spellEnumsAddedCount = count;
		}

		// bonus spells for wizards
		if (classLeveled == stat_level_wizard){
			auto intScore = objects.StatLevelGet(handle, stat_intelligence);
			if (selPkt.statBeingRaised == stat_intelligence)
				intScore++;
			auto intScoreMod = objects.GetModFromStatLevel(intScore);
			for (auto i = 0; i < intScoreMod; i++){
				selPkt.spellEnums[count] = 802;
				spellFlags[count++] = 3;
			}
			selPkt.spellEnumsAddedCount = count;
		}
	} 
	
	// progressing with a class    todo: generalize for PrC's
	else{
		// 2 new spells for Vancians
		if (d20ClassSys.IsVancianCastingClass(classLeveled, handle) ){
			for (int i = 0; i < 2; i++) {
				selPkt.spellEnums[selPkt.spellEnumsAddedCount] = SPELL_ENUM_VACANT;
				spellFlags[selPkt.spellEnumsAddedCount++] = 3;
			}
		}

		// innate casters - show all known spells and add slots as necessary
		if (d20ClassSys.IsNaturalCastingClass(classLeveled, handle)){

			struct KnownSpellInfo{
				int spEnum = 0;
				char spFlag = 0;
				KnownSpellInfo(int a, int b) :spEnum(a), spFlag(b){};
			};
			std::vector<KnownSpellInfo> knownSpells;
			knownSpells.reserve(SPELL_ENUM_MAX_EXPANDED);
			// get all known spells for innate casters
			int _knownSpells[3999] = {0,};
			int numKnown = critterSys.GetSpellEnumsKnownByClass(handle, spellSys.GetSpellClass(classLeveled), &_knownSpells[0], SPELL_ENUM_MAX_EXPANDED);
			for (int i = 0; i < numKnown; i++){
				knownSpells.push_back(KnownSpellInfo( _knownSpells[i],0 ));
			}
			selPkt.spellEnumsAddedCount = numKnown;

			
			// get max spell level   todo: extend!
			auto maxSpellLvl = casterLvlNew / 2;
			if (classLeveled == stat_level_bard) {
				maxSpellLvl = (casterLvlNew - 1) / 3 + 1;
			}

			// add labels
			for (int spellLvl = 0u; spellLvl <= maxSpellLvl; spellLvl++) {
				//selPkt.spellEnums[selPkt.spellEnumsAddedCount++] = SPELL_ENUM_LABEL_START + spellLvl;
				knownSpells.push_back(KnownSpellInfo(SPELL_ENUM_LABEL_START + spellLvl, 0));
			}

			for (auto spellLvl = 0; spellLvl <= maxSpellLvl; spellLvl++)	{
				int numNewSpellsForLevel = 
					d20LevelSys.GetSpellsPerLevel(handle, classLeveled, spellLvl, casterLvlNew)
					- d20LevelSys.GetSpellsPerLevel(handle, classLeveled, spellLvl, casterLvlNew-1);
				for (int i = 0; i < numNewSpellsForLevel; i++){
					//selPkt.spellEnums[selPkt.spellEnumsAddedCount++] = SPELL_ENUM_NEW_SLOT_START + spellLvl ;
					knownSpells.push_back(KnownSpellInfo(SPELL_ENUM_NEW_SLOT_START + spellLvl, 0));
				}
			}
			auto spellClass = spellSys.GetSpellClass(classLeveled);
			std::sort(knownSpells.begin(), knownSpells.end(), [spellClass,handle](KnownSpellInfo &ksiFirst, KnownSpellInfo &ksiSecond){
				auto first = ksiFirst.spEnum, second = ksiSecond.spEnum;

				auto firstIsLabel = (first >= SPELL_ENUM_LABEL_START) && (first < SPELL_ENUM_LABEL_START + 10);
				auto secondIsLabel = (second >= SPELL_ENUM_LABEL_START) && (second < SPELL_ENUM_LABEL_START + 10);

				auto firstIsNewSlot = (first >= SPELL_ENUM_NEW_SLOT_START) && (first < SPELL_ENUM_NEW_SLOT_START + 10);
				auto secondIsNewSlot = (second >= SPELL_ENUM_NEW_SLOT_START) && (second < SPELL_ENUM_NEW_SLOT_START + 10);

				auto firstSpellLvl = 0;
				if (firstIsLabel)
					firstSpellLvl = first - SPELL_ENUM_LABEL_START;
				else if (firstIsNewSlot)
				{
					firstSpellLvl = first - SPELL_ENUM_NEW_SLOT_START;
				}
				else
					firstSpellLvl = spellSys.GetSpellLevelBySpellClass(first, spellClass ,handle);

				auto secondSpellLvl = 0;
				if (secondIsLabel)
					secondSpellLvl = second - SPELL_ENUM_LABEL_START;
				else if (secondIsNewSlot)
				{
					secondSpellLvl = second - SPELL_ENUM_NEW_SLOT_START;
				}
				else
					secondSpellLvl = spellSys.GetSpellLevelBySpellClass(second, spellClass ,handle);

				if (firstSpellLvl != secondSpellLvl)
					return firstSpellLvl < secondSpellLvl;

				// if they are the same level

				if (firstIsLabel){
					return !secondIsLabel;
				}
				if (secondIsLabel)
					return false;

				if (firstIsNewSlot){
					return false;
				}

				if (secondIsNewSlot){
					return true;
				}
					

				auto name1 = spellSys.GetSpellName(first);
				auto name2 = spellSys.GetSpellName(second);
				auto nameCmp = _strcmpi(name1, name2);
				return nameCmp < 0;

				//return _stricmp(spellSys.GetSpellMesline(second), spellSys.GetSpellMesline(first) );

			});

			// convert the "New Spell Slot" enums to vacant enums (they were just used for sorting)
			for (auto i = 0u; i < knownSpells.size(); i++) {
				if (knownSpells[i].spEnum >= SPELL_ENUM_NEW_SLOT_START && knownSpells[i].spEnum  < SPELL_ENUM_NEW_SLOT_START + 10){
					knownSpells[i].spEnum = 802;
					knownSpells[i].spFlag = 3;
				}
			}

			// mark old replaceable spells for sorc lvls 4,6,8,... and bards 5,8,11,... 
			
			bool isReplacingSpells = false;
			if (classLeveled == stat_level_bard){
				if (casterLvlNew >= 5 
					&& !((casterLvlNew -5) % 3) )
					isReplacingSpells = true;
			}
			if (classLeveled == stat_level_sorcerer){
				if (casterLvlNew >= 4 && !(casterLvlNew % 2)){
					isReplacingSpells = true;
				}
			}
			if (isReplacingSpells){
				auto spLvlReplaceable = maxSpellLvl - 2;
				for (auto i = 0u; i < knownSpells.size(); i++){
					auto spEnum = knownSpells[i].spEnum;
					auto spLvl = 0;
					if (spEnum >= SPELL_ENUM_LABEL_START && spEnum < SPELL_ENUM_LABEL_START + 10)
						spLvl = spEnum - SPELL_ENUM_LABEL_START;
					else if (spEnum != SPELL_ENUM_MAX)
					{
						spLvl = spellSys.GetSpellLevelBySpellClass(spEnum, spellClass, handle);
						if (spLvl > spLvlReplaceable)
							break;
						knownSpells[i].spFlag = 1; // denotes as replaceable
					}
				}
			}

			selPkt.spellEnumsAddedCount = min(802u, knownSpells.size());
			for (auto i = 0u; i < knownSpells.size(); i++) {
				selPkt.spellEnums[i] = knownSpells[i].spEnum;
				spellFlags[i] = knownSpells[i].spFlag;
			}

		}


	}

	// populate entries
	temple::GetRef<void(__cdecl)()>(0x101A7390)(); // CharEditorLearnableSpellEntriesListPopulate
	temple::GetRef<void(__cdecl)()>(0x101A5F30)(); // CharEditorSpellCountBoxesUpdate

	setScrollbars();
	needPopulateEntries = 0; // needPopulateEntries
}

int &UiCharEditor::GetState(){
	return temple::GetRef<int>(0x10BE8D34);
}

void UiCharEditor::StateTitleRender(int widId){
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	int state = GetState();
	auto &stateTitles = temple::GetRef<const char*[6]>(0x10BE8D1C);
	auto &rect = temple::GetRef<TigRect>(0x10BE8E64);
	auto &style = temple::GetRef<TigTextStyle>(0x10BE9640);
	UiRenderer::DrawTextInWidget(widId, stateTitles[state], rect, style);
	UiRenderer::PopFont();
}

class UiCharEditorHooks : public TempleFix {
	

	void apply() override {
		replaceFunction<void()>(0x101A75F0, [](){
			uiCharEditor.SpellsActivate();
		});
	}
} uiCharEditorHooks;


CharEditorClassSystem::CharEditorClassSystem(const GameSystemConf& config)
{
}

CharEditorClassSystem::~CharEditorClassSystem()
{
}

const std::string& CharEditorClassSystem::GetName() const
{
	static std::string name("char_editor_class");
	return name;
}
