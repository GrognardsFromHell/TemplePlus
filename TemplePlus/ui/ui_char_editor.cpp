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
#include <tig/tig_msg.h>
#include <python/python_integration_class_spec.h>

#undef HAVE_ROUND
#define PYBIND11_EXPORT
#include <pybind11/pybind11.h>
#include <pybind11/common.h>
#include <pybind11/cast.h>
#include <pybind11/stl.h>
#include <python/python_object.h>
#include <radialmenu.h>
#include <action_sequence.h>
#include <condition.h>

namespace py = pybind11;
using namespace pybind11;
using namespace pybind11::detail;

struct KnownSpellInfo {
	int spEnum = 0;
	uint8_t spFlag = 0;
	int spellClass = 0;
	int spellLevel = 0;
	KnownSpellInfo() { spEnum = 0; spFlag = 0; spellClass = 0; };
	KnownSpellInfo(int SpellEnum, int SpellFlag) :spEnum(SpellEnum), spFlag(SpellFlag) { spellClass = 0; };
	KnownSpellInfo(int SpellEnum, int SpellFlag, int SpellClass) :spEnum(SpellEnum), spFlag(SpellFlag), spellClass(SpellClass) {};

};


class UiCharEditor {
	friend class UiCharEditorHooks;
public:
	objHndl GetEditedChar();
	CharEditorSelectionPacket & GetCharEditorSelPacket();
	std::vector<KnownSpellInfo>& GetKnownSpellInfo();
	std::vector<SpellEntry> &GetAvailableSpells();


	void PrepareNextStages();
	void BtnStatesUpdate(int systemId);


	// 
	BOOL ClassSystemInit(GameSystemConf & conf);
	BOOL ClassWidgetsInit();
	void ClassWidgetsFree();
	BOOL ClassShow();
	BOOL ClassHide();
	BOOL ClassWidgetsResize(UiResizeArgs & args);
	BOOL ClassCheckComplete();



	void SpellsActivate();
	BOOL SpellsCheckComplete();
	void SpellsFinalize();

	int &GetState();


	void StateTitleRender(int widId);
	void ClassBtnRender(int widId);
	BOOL ClassBtnMsg(int widId, TigMsg* msg);
	BOOL ClassNextBtnMsg(int widId, TigMsg* msg);
	BOOL ClassPrevBtnMsg(int widId, TigMsg* msg);
	BOOL FinishBtnMsg(int widId, TigMsg* msg);
	void ClassNextBtnRender(int widId);
	void ClassPrevBtnRender(int widId);

	// state
	int classWndPage = 0;
	eastl::vector<int> classBtnMapping; // used as an index of choosable character classes
	int GetClassWndPage();
	Stat GetClassCodeFromWidgetAndPage(int idx, int page);
	int GetStatesComplete();

	// logic
	void ClassSetPermissibles();

	// utilities
	void SpellsPopulateAvailableEntries(Stat classEnum, int maxSpellLvl, bool skipCantrips = false);

	// widget IDs
	int classWndId = 0;
	int classNextBtn = 0, classPrevBtn = 0;
	eastl::vector<int> classBtnIds; \

		// geometry
		TigRect classNextBtnRect, classNextBtnFrameRect, classNextBtnTextRect,
		classPrevBtnRect, classPrevBtnFrameRect, classPrevBtnTextRect;

	// caches
	eastl::hash_map<int, eastl::string> classNamesUppercase;
	eastl::vector<TigRect> classBtnFrameRects;
	eastl::vector<TigRect> classBtnRects;
	eastl::vector<TigRect> classTextRects;

	// art assets
	int buttonBox = 0;
	ColorRect classBtnShadowColor = ColorRect(0xFF000000);
	ColorRect classBtnColorRect = ColorRect(0xFFFFffff);
	TigTextStyle classBtnTextStyle;


	CharEditorClassSystem& GetClass() const {
		Expects(!!mClass);
		return *mClass;
	}

private:
	int mPageCount = 0;

	std::unique_ptr<CharEditorClassSystem> mClass;
	std::vector<KnownSpellInfo> mSpellInfo;
	std::vector<SpellEntry> mAvailableSpells; // spells available for learning
	//std::unique_ptr<CharEditorStatsSystem> mStats;
	//std::unique_ptr<CharEditorFeaturesSystem> mFeatures;
	//std::unique_ptr<CharEditorSkillsSystem> mSkills;
	//std::unique_ptr<CharEditorFeatsSystem> mFeats;
	//std::unique_ptr<CharEditorSpellsSystem> mSpells;
} uiCharEditor;


template <> class type_caster<objHndl> {
public:
	bool load(handle src, bool) {
		value = PyObjHndl_AsObjHndl(src.ptr());
		success = true;
		return true;
	}

	static handle cast(const objHndl &src, return_value_policy /* policy */, handle /* parent */) {
		return PyObjHndl_Create(src);
	}

	PYBIND11_TYPE_CASTER(objHndl, _("objHndl"));
protected:
	bool success = false;
};



PYBIND11_PLUGIN(tp_char_editor){
	py::module mm("char_editor", "Temple+ Char Editor, used for extending the ToEE character editor.");

	py::class_<KnownSpellInfo>(mm, "KnownSpellInfo")
		.def(py::init())
		.def(py::init<int, int>(), py::arg("spell_enum"), py::arg("spell_status"))
		.def(py::init<int, int, int>(), py::arg("spell_enum"), py::arg("spell_status"), py::arg("spell_class"))
		.def("__repr__", [](const KnownSpellInfo& spInfo)->std::string {
			return fmt::format("KnownSpellInfo: Enum {}, status {}", spInfo.spEnum, spInfo.spFlag);
		})
		.def("__cmp__", [](const KnownSpellInfo& self, const KnownSpellInfo &other)->int // sort comparer
		{
			auto first = self.spEnum, second = other.spEnum;

			auto firstIsLabel = spellSys.IsLabel(first);
			auto secondIsLabel = spellSys.IsLabel(second);

			auto firstIsNewSlot = spellSys.IsNewSlotDesignator(first);
			auto secondIsNewSlot = spellSys.IsNewSlotDesignator(second);

			auto firstSpellLvl = spellSys.GetSpellLevelBySpellClass(first, spellSys.GetSpellClass(self.spellClass) );
			auto secondSpellLvl = spellSys.GetSpellLevelBySpellClass(second, spellSys.GetSpellClass(other.spellClass));

			if (firstSpellLvl != secondSpellLvl)
				return firstSpellLvl - secondSpellLvl;

			// if they are the same level

			if (firstIsLabel) {
				if (secondIsLabel)
					return 0; // shouldn't ever happen...
				else
					return -1; // label appears first
			}
			if (secondIsLabel) // so first is not a label, thus it appears later (i.e. is bigger)
				return 1;

			if (firstIsNewSlot) { // if first is a new slot, it is last
				if (secondIsNewSlot) {
					return 0;
				}
				return 1;
			}


			auto name1 = spellSys.GetSpellName(first);
			auto name2 = spellSys.GetSpellName(second);
			auto nameCmp = _strcmpi(name1, name2);
			return nameCmp ;
		})
		.def_readwrite("spell_enum", &KnownSpellInfo::spEnum)
		.def_readwrite("spell_status", &KnownSpellInfo::spFlag, "1 - denotes as replaceable spell (e.g. sorcerers), 2 - ??, 3 - new spell slot; use 0 for the spell level labels")
		;

	// methods
	mm
	.def("get_spell_enums", []()->std::vector<KnownSpellInfo>& {
		auto &spFlags = temple::GetRef<uint8_t[802]>(0x10C72F20);
		auto &selPkt = temple::GetRef<CharEditorSelectionPacket>(0x11E72F00);
		auto result = std::vector<KnownSpellInfo>();
		for (auto i = 0; i < selPkt.spellEnumsAddedCount; i++) {
			result.push_back({ selPkt.spellEnums[i], spFlags[i] });
		}
		return result;
		//return uiCharEditor.GetKnownSpellInfo();
	})
	.def("set_spell_enums", [](std::vector<KnownSpellInfo> &ksi){
		auto &spFlags = temple::GetRef<uint8_t[802]>(0x10C72F20);
		auto &selPkt = temple::GetRef<CharEditorSelectionPacket>(0x11E72F00);
		for (auto i = 0; i < ksi.size(); i++) {
			spFlags[i] = ksi[i].spFlag;
			selPkt.spellEnums[i] = ksi[i].spEnum;
		}
		selPkt.spellEnumsAddedCount = ksi.size();
		//auto &spInfo = uiCharEditor.GetKnownSpellInfo();
		//spInfo = ksi;
	})
	.def("append_spell_enums", [](std::vector<KnownSpellInfo> &ksi) {
		auto &spFlags = temple::GetRef<uint8_t[802]>(0x10C72F20);
		auto &selPkt = temple::GetRef<CharEditorSelectionPacket>(0x11E72F00);
		for (auto i = 0; i <  ksi.size(); i++) {
			spFlags[selPkt.spellEnumsAddedCount+i] = ksi[i].spFlag;
			selPkt.spellEnums[selPkt.spellEnumsAddedCount+i] = ksi[i].spEnum;
		}
		selPkt.spellEnumsAddedCount += ksi.size();
		/*auto &spInfo = uiCharEditor.GetKnownSpellInfo();
		for (auto i = 0u; i < ksi.size(); i++){
			spInfo.push_back(ksi[i]);
		}*/
	})
	.def("get_class_code", []()->int
	{
		auto &selPkt = temple::GetRef<CharEditorSelectionPacket>(0x11E72F00);
		return selPkt.classCode;
	})
	.def("is_selecting_spells", [](objHndl handle, int classEnum)->int {
		return d20ClassSys.IsSelectingSpellsOnLevelup(handle, (Stat)classEnum);
	})
	.def("init_spell_selection", [](objHndl handle, int classEnum)
	{
		d20ClassSys.LevelupInitSpellSelection(handle, (Stat)classEnum);
	})
	.def("spells_finalize", [](objHndl handle, int classEnum)
	{
		d20ClassSys.LevelupSpellsFinalize(handle, (Stat)classEnum);
	})
	.def("get_max_spell_level", [](const objHndl & handle, int classEnum, int characterLvl)
	{
		return spellSys.GetMaxSpellLevel(handle, (Stat)classEnum, characterLvl);
	})
	.def("get_known_class_spells", [](objHndl handle, int classEnum)->std::vector<KnownSpellInfo>
	{ // get all spells belonging to the classEnum
		auto knownSpells = std::vector<KnownSpellInfo>();
		knownSpells.reserve(SPELL_ENUM_MAX_EXPANDED);
		int _knownSpells[SPELL_ENUM_MAX_EXPANDED] = { 0, };
		int numKnown = critterSys.GetSpellEnumsKnownByClass(handle, spellSys.GetSpellClass(classEnum), &_knownSpells[0], SPELL_ENUM_MAX_EXPANDED);
		for (int i = 0; i < numKnown; i++) {
			knownSpells.push_back(KnownSpellInfo(_knownSpells[i], 0, classEnum));
		}
		return knownSpells;
	})
	.def("get_spell_level", [](int spEnum, int classEnum)->int {
		auto spellClass = spellSys.GetSpellClass(classEnum);
		return spellSys.GetSpellLevelBySpellClass(spEnum, spellClass);
	})
	.def("populate_available_spells", [](int classEnum, int maxSpellLvl, bool skipCantrips)
	{
		uiCharEditor.SpellsPopulateAvailableEntries((Stat)classEnum, maxSpellLvl, skipCantrips);
	}, py::arg("class_enum"), py::arg("max_spell_level"), py::arg("skip_cantrips") = false);


	py::class_<CharEditorSelectionPacket>(mm, "CharEdSpecs", "Holds the character editing specs.")
		.def_readwrite("class_code", &CharEditorSelectionPacket::classCode, "Chosen class")
		.def_readwrite("skill_pts", &CharEditorSelectionPacket::availableSkillPoints, "Available Skill points")
		.def_readwrite("stat_raised", &CharEditorSelectionPacket::statBeingRaised, "Stat being raised")
		.def_readwrite("feat_0", &CharEditorSelectionPacket::feat0, "First feat slot; 649 for none")
		.def_readwrite("feat_1", &CharEditorSelectionPacket::feat1, "Second feat slot; 649 for none")
		.def_readwrite("feat_2", &CharEditorSelectionPacket::feat2, "Third feat slot; 649 for none")
		.def_readwrite("feat_3", &CharEditorSelectionPacket::feat3, "Fourth feat slot; 649 for none")
		.def_readwrite("feat_4", &CharEditorSelectionPacket::feat4, "Fifth feat slot; 649 for none")
		//.def_readwrite("spells", (int CharEditorSelectionPacket::*) &CharEditorSelectionPacket::spellEnums, "Spell enums available for learning")
		//.def_readwrite("spells_count", &CharEditorSelectionPacket::spellEnumsAddedCount, "Number of Spell enums available for learning")
		;
	return mm.ptr();
}


objHndl UiCharEditor::GetEditedChar(){
	return temple::GetRef<objHndl>(0x11E741A0);
}

CharEditorSelectionPacket& UiCharEditor::GetCharEditorSelPacket(){
	return temple::GetRef<CharEditorSelectionPacket>(0x11E72F00);
}

std::vector<KnownSpellInfo>& UiCharEditor::GetKnownSpellInfo(){
	return mSpellInfo;
}

void UiCharEditor::PrepareNextStages(){
	//auto handle = GetEditedChar();
	//auto &selPkt = GetCharEditorSelPacket();
	//auto classCode = selPkt.classCode;
	//auto classLvlNew = objects.StatLevelGet(handle, classCode) + 1;
	//auto &stateBtnIds = temple::GetRef<int[6]>(0x11E72E40);

	//if (classCode == stat_level_cleric){
	//	if (classLvlNew == 1)
	//		ui.ButtonSetButtonState(stateBtnIds[2], UBS_NORMAL); // features
	//}
	//if (classCode == stat_level_ranger) {
	//	if (classLvlNew == 2)
	//		ui.ButtonSetButtonState(stateBtnIds[2], UBS_NORMAL); // features
	//}
	//if (classCode == stat_level_fighter) {
	//	if (! (classLvlNew % 2) )
	//		ui.ButtonSetButtonState(stateBtnIds[4], UBS_NORMAL); // fighter feats
	//}
	//if (classCode == stat_level_monk) {
	//	if (classLvlNew == 2 || classLvlNew == 6)
	//		ui.ButtonSetButtonState(stateBtnIds[4], UBS_NORMAL); // monk feats
	//}
	//if (classCode == stat_level_rogue) {
	//	if (classLvlNew >= 10 && (! ((classLvlNew-10) % 3)) )
	//		ui.ButtonSetButtonState(stateBtnIds[4], UBS_NORMAL); // rogue special feat
	//}
	//if (classCode == stat_level_wizard)	{
	//	if (classLvlNew == 1)
	//		ui.ButtonSetButtonState(stateBtnIds[2], UBS_NORMAL); // wizard special school
	//	if ( !(classLvlNew % 5) )
	//		ui.ButtonSetButtonState(stateBtnIds[4], UBS_NORMAL); // wizard feats
	//}
	BtnStatesUpdate(0);
}

void UiCharEditor::BtnStatesUpdate(int systemId){
	auto handle = GetEditedChar();
	auto &selPkt = GetCharEditorSelPacket();
	auto classCode = selPkt.classCode;
	auto lvlNew = objects.StatLevelGet(handle, stat_level) + 1;
	auto &stateBtnIds = temple::GetRef<int[6]>(0x11E72E40);

	ui.ButtonSetButtonState(stateBtnIds[2], UBS_DISABLED); // features

	// gain stat every 4 levels
	if (lvlNew % 4)
		ui.ButtonSetButtonState(stateBtnIds[1], UBS_DISABLED); // stats
	else
		ui.ButtonSetButtonState(stateBtnIds[1], UBS_NORMAL); 

	if (lvlNew % 3)
		ui.ButtonSetButtonState(stateBtnIds[4], UBS_DISABLED); // feats
	else
		ui.ButtonSetButtonState(stateBtnIds[4], UBS_NORMAL);


	if (classCode >= stat_level_barbarian){
		auto classLvlNew = objects.StatLevelGet(handle, classCode) + 1;
		if (classCode == stat_level_cleric) {
			if (classLvlNew == 1)
				ui.ButtonSetButtonState(stateBtnIds[2], UBS_NORMAL); // features
		}
		if (classCode == stat_level_ranger) {
			if (classLvlNew == 1 || classLvlNew == 2 || !(classLvlNew % 5))
				ui.ButtonSetButtonState(stateBtnIds[2], UBS_NORMAL); // features
		}
		if (classCode == stat_level_fighter) {
			if (classLvlNew == 1 || !(classLvlNew % 2) )
				ui.ButtonSetButtonState(stateBtnIds[4], UBS_NORMAL); // fighter feats
		}
		if (classCode == stat_level_monk) {
			if (classLvlNew == 2 || classLvlNew == 6)
				ui.ButtonSetButtonState(stateBtnIds[4], UBS_NORMAL); // monk feats
		}
		if (classCode == stat_level_rogue) {
			if (classLvlNew >= 10 && (!((classLvlNew - 10) % 3)))
				ui.ButtonSetButtonState(stateBtnIds[4], UBS_NORMAL); // rogue special feat
		}
		if (classCode == stat_level_wizard) {
			if (classLvlNew == 1)
				ui.ButtonSetButtonState(stateBtnIds[2], UBS_NORMAL); // wizard special school
			if (!(classLvlNew % 5))
				ui.ButtonSetButtonState(stateBtnIds[4], UBS_NORMAL); // wizard feats
		}
	}
	
	// Spells
	if (d20ClassSys.IsSelectingSpellsOnLevelup(handle, classCode)){
		ui.ButtonSetButtonState(stateBtnIds[5], UBS_NORMAL);
	} 
	else
	{
		ui.ButtonSetButtonState(stateBtnIds[5], UBS_DISABLED);
	};
	/*switch (classCode){
	case stat_level_bard:
	case stat_level_sorcerer:
	case stat_level_wizard:
		ui.ButtonSetButtonState(stateBtnIds[5], UBS_NORMAL);
		break;
	default:
		ui.ButtonSetButtonState(stateBtnIds[5], UBS_DISABLED);
		break;
	}*/

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	auto &stateTitles = temple::GetRef<const char*[6]>(0x10BE8D1C);
	auto text = stateTitles[systemId];
	auto &rect = temple::GetRef<TigRect>(0x10BE8E64);
	auto &style = temple::GetRef<TigTextStyle>(0x10BE9640);
	auto textMeas = UiRenderer::MeasureTextSize(text, style);
	rect.x = (423 - textMeas.width) / 2 + 4;
	rect.y = (13 - textMeas.height) + 4;
	rect.height = textMeas.height;
	rect.width = textMeas.width;
	UiRenderer::PopFont();

}

BOOL UiCharEditor::ClassWidgetsInit(){
	static WidgetType1 classWnd(259,117, 405, 271);
	classWnd.widgetFlags = 1;
	classWnd.render = [](int widId) { uiCharEditor.StateTitleRender(widId); };
	if (classWnd.Add(&classWndId))
		return 0;

	int coloff = 0, rowoff = 0;

	for (auto it: d20ClassSys.vanillaClassEnums){
		// class buttons
		int newId = 0;
		WidgetType2 classBtn("Class btn", classWndId, 71 + coloff, 47 + rowoff, 130, 20);
		coloff = 139 - coloff;
		if (!coloff)
			rowoff += 29;
		if (rowoff == 5 * 29) // the bottom button
			coloff = 69;

		classBtnRects.push_back(TigRect(classBtn.x, classBtn.y, classBtn.width, classBtn.height));
		classBtn.x += classWnd.x; classBtn.y += classWnd.y;
		classBtn.render = [](int id) {uiCharEditor.ClassBtnRender(id); };
		classBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.ClassBtnMsg(id, msg); };
		classBtn.Add(&newId);
		classBtnIds.push_back(newId);
		ui.SetDefaultSounds(newId);
		ui.BindToParent(classWndId, newId);

		//rects
		classBtnFrameRects.push_back(TigRect(classBtn.x-5, classBtn.y-5, classBtn.width+10, classBtn.height+10));
		

		UiRenderer::PushFont(PredefinedFont::PRIORY_12);
		auto classMeasure = UiRenderer::MeasureTextSize(classNamesUppercase[it].c_str(), classBtnTextStyle);
		TigRect rect(classBtn.x + (110 - classMeasure.width) / 2 - classWnd.x,
			classBtn.y + (20 - classMeasure.height) / 2 - classWnd.y,
			classMeasure.width, classMeasure.height);
		classTextRects.push_back(rect);
		UiRenderer::PopFont();
	}

	classNextBtnTextRect = classNextBtnRect = TigRect(classWnd.x + 293, classWnd.y + 234, 55, 20);
	classPrevBtnTextRect = classPrevBtnRect = TigRect(classWnd.x + 58, classWnd.y + 234, 55, 20);
	classNextBtnFrameRect = TigRect(classWnd.x + 293-3, classWnd.y + 234 -5, 55+6, 20+10);
	classPrevBtnFrameRect = TigRect(classWnd.x + 58 -3, classWnd.y + 234 -5, 55+6, 20+10);
	classNextBtnTextRect.x -= classWnd.x; classNextBtnTextRect.y -= classWnd.y;
	classPrevBtnTextRect.x -= classWnd.x; classPrevBtnTextRect.y -= classWnd.y;

	WidgetType2 nextBtn("Class Next Button", classWndId, classWnd.x + 293, classWnd.y + 230, 55, 20),
		prevBtn("Class Prev. Button", classWndId, classWnd.x + 58, classWnd.y + 230, 55, 20);

	nextBtn.handleMessage = [](int widId, TigMsg*msg)->BOOL {
		if (uiCharEditor.classWndPage < uiCharEditor.mPageCount)
			uiCharEditor.classWndPage++;
		uiCharEditor.ClassSetPermissibles();
		return 1; };
	nextBtn.render = [](int id) { uiCharEditor.ClassNextBtnRender(id); };
	nextBtn.handleMessage = [](int widId, TigMsg*msg)->BOOL {	return uiCharEditor.ClassNextBtnMsg(widId, msg); };
	prevBtn.render = [](int id) { uiCharEditor.ClassPrevBtnRender(id); };
	prevBtn.handleMessage = [](int widId, TigMsg*msg)->BOOL {	return uiCharEditor.ClassPrevBtnMsg(widId, msg); };
	nextBtn.Add(&classNextBtn);	prevBtn.Add(&classPrevBtn);
	
	ui.SetDefaultSounds(classNextBtn);	ui.BindToParent(classWndId, classNextBtn);
	ui.SetDefaultSounds(classPrevBtn);	ui.BindToParent(classWndId, classPrevBtn);

	return TRUE;
}

void UiCharEditor::ClassWidgetsFree(){
	for (auto it: classBtnIds){
		ui.WidgetRemoveRegardParent(it);
	}
	classBtnIds.clear();
	ui.WidgetRemoveRegardParent(classNextBtn);
	ui.WidgetRemoveRegardParent(classPrevBtn);
	ui.WidgetRemove(classWndId);
}

BOOL UiCharEditor::ClassShow(){
	ui.WidgetSetHidden(classWndId, 0);
	ui.WidgetBringToFront(classWndId);
	return 1;
}

BOOL UiCharEditor::ClassHide(){
	ui.WidgetSetHidden(classWndId, 1);
	return 0;
}

BOOL UiCharEditor::ClassWidgetsResize(UiResizeArgs & args){
	for (auto it: classBtnIds){
		ui.WidgetRemoveRegardParent(it);
	}
	classBtnIds.clear();
	ui.WidgetRemoveRegardParent(classNextBtn);
	ui.WidgetRemoveRegardParent(classPrevBtn);
	ui.WidgetAndWindowRemove(classWndId);
	classBtnFrameRects.clear();
	classBtnRects.clear();
	classTextRects.clear();
	return ClassWidgetsInit();
}

BOOL UiCharEditor::ClassCheckComplete(){
	auto &selPkt = GetCharEditorSelPacket();
	return (BOOL)(selPkt.classCode != 0);
}

BOOL UiCharEditor::ClassSystemInit(GameSystemConf &conf){
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

	for (auto it: d20ClassSys.classEnums){
		auto className = _strdup(d20Stats.GetStatName((Stat)it));
		classNamesUppercase[it] = className;
		for (auto &letter: classNamesUppercase[it]){
			letter = toupper(letter);
		}
		classBtnMapping.push_back(it);
	}
	mPageCount = classBtnMapping.size() / 11;
	if (mPageCount * 11u < classBtnMapping.size())
		mPageCount++;

	return ClassWidgetsInit();
}

void UiCharEditor::SpellsActivate() {
	auto handle = GetEditedChar();
	auto obj = gameSystems->GetObj().GetObject(handle);
	auto &selPkt = GetCharEditorSelPacket();



	// get the new caster level for the levelled class (1 indicates a newly taken class)
	auto casterLvlNew = 1;
	auto classLeveled = selPkt.classCode;
	auto lvls = obj->GetInt32Array(obj_f_critter_level_idx);
	for (auto i = 0u; i < lvls.GetSize(); i++) {
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


	auto & spellFlags = temple::GetRef<uint8_t[802]>(0x10C72F20);
	memset(spellFlags, 0, sizeof(spellFlags));
	selPkt.spellEnumToRemove = 0;

	d20ClassSys.LevelupInitSpellSelection(handle, selPkt.classCode);

	// convert the "New Spell Slot" enums to vacant enums (they were just used for sorting)
	for (auto i = 0u; i < selPkt.spellEnumsAddedCount; i++) {
		if (selPkt.spellEnums[i] >= SPELL_ENUM_NEW_SLOT_START && selPkt.spellEnums[i] < SPELL_ENUM_NEW_SLOT_START + 10) {
			selPkt.spellEnums[i] = 802;
			spellFlags[i] = 3;
		}
	}

	// legacy code
	//auto isNewClass = casterLvlNew == 1; // todo: generalize for PrC's
	//
	// newly taken class
	//if (isNewClass){
		//
		// let natural casters choose 4 cantrips (Now done via Python API)
		//auto count = 0;
		//if (classLeveled == stat_level_bard || classLeveled == stat_level_sorcerer ){
		//	selPkt.spellEnums[count++] = 803; // Spell Level 0 label
		//	for (int i = 1; i < 5; i++)	{
		//		selPkt.spellEnums[count] = 802;
		//		spellFlags[count++] = 3;
		//	}
		//		
		//	selPkt.spellEnumsAddedCount = count;
		//}
		//
		//// add 2 level 1 spells for wiz/sorc (Now done via Python API)
		//if (classLeveled == stat_level_wizard || classLeveled == stat_level_sorcerer){
		//	selPkt.spellEnums[count++] = 804; // Spell Level 1 label
		//	for (int i = 0; i < 2; i++) {
		//		selPkt.spellEnums[count] = 802;
		//		spellFlags[count++] = 3;
		//	}
		//	selPkt.spellEnumsAddedCount = count;
		//}
		//
		//// bonus spells for wizards (now done via Python API)
		//if (classLeveled == stat_level_wizard){
		//	auto intScore = objects.StatLevelGet(handle, stat_intelligence);
		//	if (selPkt.statBeingRaised == stat_intelligence)
		//		intScore++;
		//	auto intScoreMod = objects.GetModFromStatLevel(intScore);
		//	for (auto i = 0; i < intScoreMod; i++){
		//		selPkt.spellEnums[count] = 802;
		//		spellFlags[count++] = 3;
		//	}
		//	selPkt.spellEnumsAddedCount = count;
		//}
	//} 
	//
	// progressing with a class    todo: generalize for PrC's
	//else{
		//// 2 new spells for Vancians (Now done via Python API)
		//if (d20ClassSys.IsVancianCastingClass(classLeveled, handle) ){
		//	for (int i = 0; i < 2; i++) {
		//		selPkt.spellEnums[selPkt.spellEnumsAddedCount] = SPELL_ENUM_VACANT;
		//		spellFlags[selPkt.spellEnumsAddedCount++] = 3;
		//	}
		//}
		//
		// innate casters - show all known spells and add slots as necessary
		//if (d20ClassSys.IsNaturalCastingClass(classLeveled, handle)){
			//
			//	
			//std::vector<KnownSpellInfo> knownSpells;
			//knownSpells.reserve(SPELL_ENUM_MAX_EXPANDED);
			//// get all known spells for innate casters
			//int _knownSpells[3999] = {0,};
			//int numKnown = critterSys.GetSpellEnumsKnownByClass(handle, spellSys.GetSpellClass(classLeveled), &_knownSpells[0], SPELL_ENUM_MAX_EXPANDED);
			//for (int i = 0; i < numKnown; i++){
			//	knownSpells.push_back(KnownSpellInfo( _knownSpells[i],0 ));
			//}
			//selPkt.spellEnumsAddedCount = numKnown;
			//
			//
			//
			//auto maxSpellLvl = spellSys.GetMaxSpellLevel(handle, classLeveled , casterLvlNew);
			// get max spell level
			//auto maxSpellLvl = casterLvlNew / 2;
			//if (classLeveled == stat_level_bard) {
			//	maxSpellLvl = (casterLvlNew - 1) / 3 + 1;
			//}
			//
			// add labels
			//for (int spellLvl = 0u; spellLvl <= maxSpellLvl; spellLvl++) {
			//	//selPkt.spellEnums[selPkt.spellEnumsAddedCount++] = SPELL_ENUM_LABEL_START + spellLvl;
			//	knownSpells.push_back(KnownSpellInfo(SPELL_ENUM_LABEL_START + spellLvl, 0));
			//}
			//
			//for (auto spellLvl = 0; spellLvl <= maxSpellLvl; spellLvl++)	{
			//	
			//	int numNewSpellsForLevel = 
			//		d20LevelSys.GetSpellsPerLevel(handle, classLeveled, spellLvl, casterLvlNew)
			//		- d20LevelSys.GetSpellsPerLevel(handle, classLeveled, spellLvl, casterLvlNew-1);
			//	for (int i = 0; i < numNewSpellsForLevel; i++){
			//		//selPkt.spellEnums[selPkt.spellEnumsAddedCount++] = SPELL_ENUM_NEW_SLOT_START + spellLvl ;
			//		knownSpells.push_back(KnownSpellInfo(SPELL_ENUM_NEW_SLOT_START + spellLvl, 0));
			//	}
			//}
			//auto spellClass = spellSys.GetSpellClass(classLeveled);
			//for (auto i = 0; i < selPkt.spellEnumsAddedCount; i++){
			//	knownSpells.push_back(KnownSpellInfo(selPkt.spellEnums[i], spellFlags[i]));
			//}
			//
			//std::sort(knownSpells.begin(), knownSpells.end(), [spellClass](KnownSpellInfo &firstEntry, KnownSpellInfo &secondEntry){
			//	auto first = firstEntry.spEnum, second = secondEntry.spEnum;
			//
			//auto firstIsLabel = spellSys.IsLabel(first);
			//auto secondIsLabel = spellSys.IsLabel(second);
			//
			//auto firstIsNewSlot = spellSys.IsNewSlotDesignator(first);
			//auto secondIsNewSlot = spellSys.IsNewSlotDesignator(second);
			//
			//auto firstSpellLvl = spellSys.GetSpellLevelBySpellClass(first, spellClass);
			//auto secondSpellLvl = spellSys.GetSpellLevelBySpellClass(second, spellClass);
			//
			//
			//	if (firstSpellLvl != secondSpellLvl)
			//		return firstSpellLvl < secondSpellLvl;
			//
			//	// if they are the same level
			//
			//	if (firstIsLabel){
			//		return !secondIsLabel;
			//	}
			//	if (secondIsLabel)
			//		return false;
			//
			//	if (firstIsNewSlot){
			//		return false;
			//	}
			//
			//	if (secondIsNewSlot){
			//		return true;
			//	}
			//		
			//
			//	auto name1 = spellSys.GetSpellName(first);
			//	auto name2 = spellSys.GetSpellName(second);
			//	auto nameCmp = _strcmpi(name1, name2);
			//	return nameCmp < 0;
			//
			//	//return _stricmp(spellSys.GetSpellMesline(second), spellSys.GetSpellMesline(first) );
			//
			//});
			//
			// convert the "New Spell Slot" enums to vacant enums (they were just used for sorting)
			//for (auto i = 0u; i < knownSpells.size(); i++) {
			//	if (knownSpells[i].spEnum >= SPELL_ENUM_NEW_SLOT_START && knownSpells[i].spEnum  < SPELL_ENUM_NEW_SLOT_START + 10){
			//		knownSpells[i].spEnum = 802;
			//		knownSpells[i].spFlag = 3;
			//	}
			//}
			//
			// mark old replaceable spells for sorc lvls 4,6,8,... and bards 5,8,11,... 
			//
			//bool isReplacingSpells = false;
			//if (classLeveled == stat_level_bard){
			//	if (casterLvlNew >= 5 
			//		&& !((casterLvlNew -5) % 3) )
			//		isReplacingSpells = true;
			//}
			//if (classLeveled == stat_level_sorcerer){
			//	if (casterLvlNew >= 4 && !(casterLvlNew % 2)){
			//		isReplacingSpells = true;
			//	}
			//}
			//if (isReplacingSpells){
			//	auto spLvlReplaceable = maxSpellLvl - 2;
			//	for (auto i = 0u; i < knownSpells.size(); i++){
			//		auto spEnum = knownSpells[i].spEnum;
			//		auto spLvl = 0;
			//		if (spEnum >= SPELL_ENUM_LABEL_START && spEnum < SPELL_ENUM_LABEL_START + 10)
			//			spLvl = spEnum - SPELL_ENUM_LABEL_START;
			//		else if (spEnum != SPELL_ENUM_MAX)
			//		{
			//			spLvl = spellSys.GetSpellLevelBySpellClass(spEnum, spellClass, handle);
			//			if (spLvl > spLvlReplaceable)
			//				break;
			//			knownSpells[i].spFlag = 1; // denotes as replaceable
			//		}
			//	}
			//}
			//
			//selPkt.spellEnumsAddedCount = min(802u, knownSpells.size());
			//for (auto i = 0u; i < knownSpells.size(); i++) {
			//	selPkt.spellEnums[i] = knownSpells[i].spEnum;
			//	spellFlags[i] = knownSpells[i].spFlag;
			//}
		//
		//}
	//
	//
	//}

	// populate entries
	//temple::GetRef<void(__cdecl)()>(0x101A7390)(); // CharEditorLearnableSpellEntriesListPopulate (now done via Python API)
	temple::GetRef<void(__cdecl)()>(0x101A5F30)(); // CharEditorSpellCountBoxesUpdate

	setScrollbars();
	needPopulateEntries = 0; // needPopulateEntries
}

BOOL UiCharEditor::SpellsCheckComplete(){
	auto selPkt = GetCharEditorSelPacket();
	auto handle = GetEditedChar();
	if (!d20ClassSys.IsSelectingSpellsOnLevelup(handle, selPkt.classCode))
		return true;

	auto &needPopulateEntries = temple::GetRef<int>(0x10C4D4C4);

	if (needPopulateEntries == 1)
		return false;

	return d20ClassSys.LevelupSpellsCheckComplete(GetEditedChar(), selPkt.classCode, selPkt.spellEnums, selPkt.spellEnumsAddedCount);

}

void UiCharEditor::SpellsFinalize(){
	auto charEdited = GetEditedChar();
	auto &selPkt = GetCharEditorSelPacket();

	d20ClassSys.LevelupSpellsFinalize(charEdited, selPkt.classCode);
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

void UiCharEditor::ClassBtnRender(int widId){
	auto idx = ui.WidgetlistIndexof(widId, &classBtnIds[0], classBtnIds.size());
	if (idx == -1)
		return;

	auto page = GetClassWndPage();
	auto classCode = GetClassCodeFromWidgetAndPage(idx, page);
	if (classCode == (Stat)-1)
		return;

	static TigRect srcRect(1, 1, 120, 30);
	UiRenderer::DrawTexture(buttonBox, classBtnFrameRects[idx], srcRect);

	UiButtonState btnState; 
	ui.GetButtonState(widId, btnState);
	if (btnState != UiButtonState::UBS_DISABLED && btnState != UiButtonState::UBS_DOWN)
	{
		auto &selPkt = GetCharEditorSelPacket();
		if (selPkt.classCode == classCode)
			btnState = UiButtonState::UBS_RELEASED;
		else
			btnState = btnState == UiButtonState::UBS_HOVERED ? UiButtonState::UBS_HOVERED : UiButtonState::UBS_NORMAL;
	}
		
	auto texId = temple::GetRef<int[15]>(0x11E74140)[(int)btnState];
	static TigRect srcRect2(1, 1, 110, 20);
	auto &rect = classBtnRects[idx];
	UiRenderer::DrawTextureInWidget(classWndId ,texId, rect, srcRect2);

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	auto textt = classNamesUppercase[classCode].c_str();
	
	auto textMeas = UiRenderer::MeasureTextSize(textt, classBtnTextStyle );
	TigRect classTextRect(rect.x + (rect.width - textMeas.width)/2,
		rect.y + (rect.height - textMeas.height)/2,
		textMeas.width, textMeas.height);

	UiRenderer::DrawTextInWidget(classWndId, textt, classTextRect, classBtnTextStyle);
	UiRenderer::PopFont();
}

BOOL UiCharEditor::ClassBtnMsg(int widId, TigMsg * msg){
	if (msg->type != TigMsgType::WIDGET)
		return 0;
	
	auto idx = ui.WidgetlistIndexof(widId, &classBtnIds[0], classBtnIds.size());
	if (idx == -1)
		return 0;
	
	auto _msg = (TigMsgWidget*)msg;
	auto classCode = GetClassCodeFromWidgetAndPage(idx, GetClassWndPage());
	if (classCode == (Stat)-1)
		return 0;

	if (_msg->widgetEventType == TigMsgWidgetEvent::Clicked){
		GetCharEditorSelPacket().classCode = classCode;
		PrepareNextStages();
		temple::GetRef<void(__cdecl)(int)>(0x10143FF0)(0); // resets all the next systems in case of change
		return 1;
	}

	if (_msg->widgetEventType == TigMsgWidgetEvent::Exited) {
		temple::GetRef<void(__cdecl)(const char*)>(0x10162C00)("");
		return 1;
	}

	if (_msg->widgetEventType == TigMsgWidgetEvent::Entered) {
		auto textboxText = fmt::format("{}", d20ClassSys.GetClassShortHelp(classCode));
		if (textboxText.size() >= 1024)
			textboxText[1023] = 0;
		strcpy( temple::GetRef<char[1024]>(0x10C80CC0), &textboxText[0]);
		temple::GetRef<void(__cdecl)(const char*)>(0x10162C00)(temple::GetRef<char[1024]>(0x10C80CC0));
		return 1;
	}


	return 0;
}

BOOL UiCharEditor::ClassNextBtnMsg(int widId, TigMsg * msg){
	if (msg->type != TigMsgType::WIDGET)
		return 0;

	auto _msg = (TigMsgWidget*)msg;

	if (_msg->widgetEventType == TigMsgWidgetEvent::Clicked) {
		if (classWndPage < mPageCount-1)
			classWndPage++;
		uiCharEditor.ClassSetPermissibles();
		//temple::GetRef<void(__cdecl)(int)>(0x10143FF0)(0); // resets all the next systems in case of change
		return 1;
	}

	if (_msg->widgetEventType == TigMsgWidgetEvent::Exited) {
		temple::GetRef<void(__cdecl)(const char*)>(0x10162C00)("");
		return 1;
	}

	if (_msg->widgetEventType == TigMsgWidgetEvent::Entered) {
		auto textboxText = fmt::format("Coming soon!");
		if (textboxText.size() >= 1024)
			textboxText[1023] = 0;
		strcpy(temple::GetRef<char[1024]>(0x10C80CC0), &textboxText[0]);
		temple::GetRef<void(__cdecl)(const char*)>(0x10162C00)(temple::GetRef<char[1024]>(0x10C80CC0));
		return 1;
	}

	return 0;
}

BOOL UiCharEditor::ClassPrevBtnMsg(int widId, TigMsg * msg){
	if (msg->type != TigMsgType::WIDGET)
		return 0;

	auto _msg = (TigMsgWidget*)msg;

	if (_msg->widgetEventType == TigMsgWidgetEvent::Clicked) {
		if (classWndPage > 0)
			classWndPage--;
		uiCharEditor.ClassSetPermissibles();
		//temple::GetRef<void(__cdecl)(int)>(0x10143FF0)(0); // resets all the next systems in case of change
		return 1;
	}


	return 0;
}

BOOL UiCharEditor::FinishBtnMsg(int widId, TigMsg * msg){
	if (msg->type == TigMsgType::MOUSE)
		return 1;

	if (msg->type != TigMsgType::WIDGET)
		return 0;

	auto _msg = (TigMsgWidget*)msg;

	if (_msg->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return 1;

	auto stComplete = GetStatesComplete();
	if (stComplete != 6)
		return 1;

	auto &selPkt = GetCharEditorSelPacket();
	auto charEdited = GetEditedChar();

	// add spell casting condition
	if (d20ClassSys.IsCastingClass(selPkt.classCode)){
		auto spellcastCond = (std::string)d20ClassSys.GetSpellCastingCondition(selPkt.classCode);
		if ( spellcastCond.size() ){
			conds.AddTo(charEdited, spellcastCond, {0,0,0,0, 0,0,0,0});
		}
	}
	return 1;
}

void UiCharEditor::ClassNextBtnRender(int widId){

	static TigRect srcRect(1, 1, 120, 30);
	UiRenderer::DrawTexture(buttonBox, classNextBtnFrameRect, srcRect);

	UiButtonState btnState;
	ui.GetButtonState(widId, btnState);
	if (btnState != UiButtonState::UBS_DISABLED && btnState != UiButtonState::UBS_DOWN){
		btnState = btnState == UiButtonState::UBS_HOVERED ? UiButtonState::UBS_HOVERED : UiButtonState::UBS_NORMAL;
	}

	auto texId = temple::GetRef<int[15]>(0x11E74140)[(int)btnState];
	static TigRect srcRect2(1, 1, 110, 20);
	UiRenderer::DrawTexture(texId, classNextBtnRect, srcRect2);

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	auto textt = fmt::format("NEXT");
	auto textMeas = UiRenderer::MeasureTextSize(textt, classBtnTextStyle);
	TigRect textRect(classNextBtnTextRect.x + (classNextBtnTextRect.width -textMeas.width)/2,
		classNextBtnTextRect.y + (classNextBtnTextRect.height - textMeas.height) / 2,
		textMeas.width, textMeas.height);
	UiRenderer::DrawTextInWidget(classWndId, textt, textRect, classBtnTextStyle);
	UiRenderer::PopFont();
}

void UiCharEditor::ClassPrevBtnRender(int widId){
	static TigRect srcRect(1, 1, 120, 30);
	UiRenderer::DrawTexture(buttonBox, classPrevBtnFrameRect, srcRect);

	UiButtonState btnState;
	ui.GetButtonState(widId, btnState);
	if (btnState != UiButtonState::UBS_DISABLED && btnState != UiButtonState::UBS_DOWN) {
		btnState = btnState == UiButtonState::UBS_HOVERED ? UiButtonState::UBS_HOVERED : UiButtonState::UBS_NORMAL;
	}

	auto texId = temple::GetRef<int[15]>(0x11E74140)[(int)btnState];
	static TigRect srcRect2(1, 1, 110, 20);
	UiRenderer::DrawTexture(texId, classPrevBtnRect, srcRect2);

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	auto textt = fmt::format("PREV");
	auto textMeas = UiRenderer::MeasureTextSize(textt, classBtnTextStyle);
	TigRect textRect(classPrevBtnTextRect.x + (classPrevBtnTextRect.width - textMeas.width) / 2,
		classPrevBtnTextRect.y + (classPrevBtnTextRect.height - textMeas.height) / 2,
		textMeas.width, textMeas.height);
	UiRenderer::DrawTextInWidget(classWndId, textt, textRect, classBtnTextStyle);
	UiRenderer::PopFont();
}

int UiCharEditor::GetClassWndPage(){
	return classWndPage;
}

Stat UiCharEditor::GetClassCodeFromWidgetAndPage(int idx, int page){
	if (page == 0)
		return (Stat)(stat_level_barbarian + idx);

	auto idx2 = idx + page * 11u;
	if (idx2 >= classBtnMapping.size())
		return (Stat)-1;
	return (Stat)classBtnMapping[idx2];
}

int UiCharEditor::GetStatesComplete(){
	return temple::GetRef<int>(0x10BE8D38);
}

void UiCharEditor::ClassSetPermissibles(){
	auto page = GetClassWndPage();
	auto idx = 0;
	auto handle = GetEditedChar();
	for (auto it:classBtnIds){
		auto classCode = GetClassCodeFromWidgetAndPage(idx++, page);
		if (classCode == (Stat)-1)
			ui.ButtonSetButtonState(it, UBS_DISABLED);
		else if (d20ClassSys.ReqsMet(handle, classCode)){
			ui.ButtonSetButtonState(it, UBS_NORMAL);
		}
		else{
			ui.ButtonSetButtonState(it, UBS_DISABLED);
		}
		
	}
	if (page > 0)
		ui.ButtonSetButtonState(classPrevBtn, UBS_NORMAL);
	else 
		ui.ButtonSetButtonState(classPrevBtn, UBS_DISABLED);

	if (page < mPageCount-1)
		ui.ButtonSetButtonState(classNextBtn, UBS_NORMAL);
	else
		ui.ButtonSetButtonState(classNextBtn, UBS_DISABLED);
}

void UiCharEditor::SpellsPopulateAvailableEntries(Stat classEnum, int maxSpellLvl, bool skipCantrips){
	auto charEdited = GetEditedChar();
	auto spellClass = spellSys.GetSpellClass(classEnum);

	// get available spell entries
	std::vector<SpellEntry> spEntries;
	spEntries.reserve(SPELL_ENUM_MAX_EXPANDED);
	SpellEntry _spEntries[SPELL_ENUM_MAX]; memset(_spEntries, 0, sizeof _spEntries);
	//auto spellRegistryCopyAllLearnablesByClass = temple::GetRef<int(__cdecl)(objHndl, Stat, SpellEntry*, int)>(0x1007B210);
	//auto numSpells = spellRegistryCopyAllLearnablesByClass(charEdited, (Stat)classEnum, _spEntries, SPELL_ENUM_MAX_EXPANDED);
	/*for (auto i = 0u; i < numSpells; i++) {
		spEntries.push_back(_spEntries[i]);
	}*/
	auto numSpells = spellSys.CopyLearnableSpells(charEdited, spellClass, spEntries );
	
	
	
	// add labels
	for (auto i = 0; i <= maxSpellLvl; i++ ){
		SpellEntry spEntryTemp;
		spEntryTemp.spellEnum = SPELL_ENUM_LABEL_START + i;
		spEntries.push_back(spEntryTemp);
	}

	// cull too high level spells, domain spells and (optionally) cantrips
	for (auto i = 0; i < spEntries.size(); /* do not increment here! */ ){
		auto shouldCull = false;

		auto &spEnt = spEntries[i];
		
		auto spLvl = spellSys.GetSpellLevelBySpellClass(spEnt.spellEnum, spellClass);
		if (spLvl < 0 || spLvl > maxSpellLvl 
			|| (spLvl == 0 && skipCantrips))
			shouldCull = true;
		
		if (shouldCull)
			spEntries.erase(spEntries.begin()+i);
		else
			i++;
	}

	std::sort(spEntries.begin(), spEntries.end(), [spellClass](SpellEntry &firstEntry, SpellEntry &secondEntry){
		auto first = firstEntry.spellEnum, second = secondEntry.spellEnum;
	
	auto firstIsLabel = spellSys.IsLabel(first);	auto secondIsLabel = spellSys.IsLabel(second);
	
	auto firstIsNewSlot = spellSys.IsNewSlotDesignator(first);	auto secondIsNewSlot = spellSys.IsNewSlotDesignator(second);
	
	auto firstSpellLvl = spellSys.GetSpellLevelBySpellClass(first, spellClass);
	auto secondSpellLvl = spellSys.GetSpellLevelBySpellClass(second, spellClass);
	
	
		if (firstSpellLvl != secondSpellLvl)
			return firstSpellLvl < secondSpellLvl;
	
		// if they are the same level
	
		if (firstIsLabel){
			return !secondIsLabel;
		}
		if (secondIsLabel)
			return false;
	
		if (firstIsNewSlot)
			return false;
		
		if (secondIsNewSlot)
			return true;
					
		auto name1 = spellSys.GetSpellName(first);	auto name2 = spellSys.GetSpellName(second);
		return _strcmpi(name1, name2) < 0;
	});

	numSpells = min(SPELL_ENUM_MAX, spEntries.size());
	// copy to CharEditorSpellsAvailableEntries
	memcpy(temple::GetRef<SpellEntry[]>(0x10C4D550), &spEntries[0], sizeof(SpellEntry)*numSpells );

	// set charEditorSpellsNumEntries
	temple::GetRef<int>(0x10C75A60) = numSpells;
}

class UiCharEditorHooks : public TempleFix {
	

	void apply() override {

		// general
		replaceFunction<void()>(0x101B0760, []() {
			uiCharEditor.PrepareNextStages();
		});
		replaceFunction<void(int)>(0x10148B70, [](int systemId) {
			uiCharEditor.BtnStatesUpdate(systemId);
		});
		// Finish Btn Msg
		static BOOL(__cdecl *orgFinishBtnMsg)(int, TigMsg*) = replaceFunction<BOOL(int, TigMsg*)>(0x10148FE0, [](int widId, TigMsg* msg)
		{
			auto result = orgFinishBtnMsg(widId, msg);
			uiCharEditor.FinishBtnMsg(widId, msg);
			return result;
		});

		//classes
		replaceFunction<BOOL(GameSystemConf&)>(0x101B0DE0, [](GameSystemConf &conf) {
			return uiCharEditor.ClassSystemInit(conf);
		});

		replaceFunction<BOOL(UiResizeArgs&)>(0x101B0EC0, [](UiResizeArgs&args) {
			return uiCharEditor.ClassWidgetsResize(args);
		});

		replaceFunction<void()>(0x101B0890, []() {
			uiCharEditor.ClassWidgetsFree();
		});

		replaceFunction<void()>(0x101B0990, []() {
			uiCharEditor.ClassSetPermissibles();
		});

		replaceFunction<BOOL()>(0x101B05E0, []() {
			return uiCharEditor.ClassHide();
		});

		replaceFunction<BOOL()>(0x101B0600, []() {
			return uiCharEditor.ClassShow();
		});

		replaceFunction<BOOL()>(0x101B0620, []() {
			return uiCharEditor.ClassCheckComplete();
		});
		

		// spells
		replaceFunction<void()>(0x101A75F0, [](){
			uiCharEditor.SpellsActivate();
		});

		replaceFunction<BOOL()>(0x101A5C00, []()
		{
			return uiCharEditor.SpellsCheckComplete();
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
