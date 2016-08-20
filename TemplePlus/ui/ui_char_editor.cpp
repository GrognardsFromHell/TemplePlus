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
#include "graphics/imgfile.h"
#include "graphics/render_hooks.h"
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
#include <gamesystems/map/d20_help.h>
#include <infrastructure/elfhash.h>
#include <tig/tig_mouse.h>
#include <combat.h>

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
	KnownSpellInfo(int SpellEnum, int SpellFlag, int SpellClass) :spEnum(SpellEnum), spFlag(SpellFlag), spellClass( spellSys.GetSpellClass(SpellClass) ){
		spellLevel = spellSys.GetSpellLevelBySpellClass(SpellEnum, SpellClass);
	};
	KnownSpellInfo(int SpellEnum, int SpellFlag, int SpellClass, int isDomain) :spEnum(SpellEnum), spFlag(SpellFlag), spellClass(spellSys.GetSpellClass(SpellClass,isDomain != 0)){
		spellLevel = spellSys.GetSpellLevelBySpellClass(SpellEnum, SpellClass);
	};

};
struct FeatInfo {
	int featEnum;
	int flag =0;
	int minLevel = 1;
	FeatInfo(int FeatEnum) : featEnum(FeatEnum) {};
};


class UiCharEditor {
	friend class UiCharEditorHooks;
public:
	objHndl GetEditedChar();
	CharEditorSelectionPacket & GetCharEditorSelPacket();
	std::vector<KnownSpellInfo>& GetKnownSpellInfo();
	std::vector<KnownSpellInfo> &GetAvailableSpells();


	void PrepareNextStages();
	void BtnStatesUpdate(int systemId);

#pragma region Systems
	// 
	BOOL ClassSystemInit(GameSystemConf & conf);
	BOOL ClassWidgetsInit();
	void ClassWidgetsFree();
	BOOL ClassShow();
	BOOL ClassHide();
	BOOL ClassWidgetsResize(UiResizeArgs & args);
	BOOL ClassCheckComplete();



	BOOL FeatsSystemInit(GameSystemConf & conf);
	BOOL FeatsWidgetsInit(int w, int h);
	void FeatsFree();
	void FeatWidgetsFree();
	BOOL FeatsWidgetsResize(UiResizeArgs &args);
	BOOL FeatsShow();
	BOOL FeatsHide();
	void FeatsActivate();
	BOOL FeatsCheckComplete();
	void FeatsFinalize();
	void FeatsReset(CharEditorSelectionPacket& selPkt);

	BOOL SpellsSystemInit(GameSystemConf & conf);
	void SpellsFree();
	BOOL SpellsWidgetsInit();
	void SpellsWidgetsFree();
	BOOL SpellsShow();
	BOOL SpellsHide();
	BOOL SpellsWidgetsResize(UiResizeArgs &args);
	void SpellsActivate();
	BOOL SpellsCheckComplete();
	void SpellsFinalize();
	void SpellsReset(CharEditorSelectionPacket& selPkt);

#pragma endregion 

	int &GetState();

#pragma region Widget callbacks
	void StateTitleRender(int widId);
	void ClassBtnRender(int widId);
	BOOL ClassBtnMsg(int widId, TigMsg* msg);
	BOOL ClassNextBtnMsg(int widId, TigMsg* msg);
	BOOL ClassPrevBtnMsg(int widId, TigMsg* msg);
	BOOL FinishBtnMsg(int widId, TigMsg* msg); // gets after the original FinishBtnMsg
	void ClassNextBtnRender(int widId);
	void ClassPrevBtnRender(int widId);

	BOOL FeatsWndMsg(int widId, TigMsg* msg);
	void FeatsWndRender(int widId);

	BOOL FeatsEntryBtnMsg(int widId, TigMsg* msg);
	void FeatsEntryBtnRender(int widId);
	BOOL FeatsExistingBtnMsg(int widId, TigMsg* msg);
	void FeatsExistingBtnRender(int widId);

	void FeatsMultiSelectWndRender(int widId);
	BOOL FeatsMultiSelectWndMsg(int widId, TigMsg* msg);
	void FeatsMultiOkBtnRender(int widId);
	BOOL FeatsMultiOkBtnMsg(int widId, TigMsg* msg);
	void FeatsMultiCancelBtnRender(int widId);
	BOOL FeatsMultiCancelBtnMsg(int widId, TigMsg* msg);
	void FeatsMultiBtnRender(int widId);
	BOOL FeatsMultiBtnMsg(int widId, TigMsg* msg);


	void SpellsWndRender(int widId);
	BOOL SpellsWndMsg(int widId, TigMsg* msg);
	void SpellsPerDayUpdate();
	BOOL SpellsEntryBtnMsg(int widId, TigMsg* msg);
	void SpellsEntryBtnRender(int widId);

	BOOL SpellsAvailableEntryBtnMsg(int widId, TigMsg* msg);
	void SpellsAvailableEntryBtnRender(int widId);
#pragma endregion
	// state
	int classWndPage = 0;
	eastl::vector<int> classBtnMapping; // used as an index of choosable character classes
	int GetClassWndPage();
	Stat GetClassCodeFromWidgetAndPage(int idx, int page);
	int GetStatesComplete();

	// logic
	void ClassSetPermissibles();
	bool IsSelectingNormalFeat(); // the normal feat you get every 3rd level in 3.5ed
	bool IsSelectingBonusFeat(); // selecting a class bonus feat

	// utilities
	int GetNewLvl(Stat classEnum = stat_level);
	void SpellsPopulateAvailableEntries(Stat classEnum, int maxSpellLvl, bool skipCantrips = false);
	bool SpellIsForbidden(int spEnum);
	bool SpellIsAlreadyKnown(int spEnum, int spellClass);
	std::string GetFeatName(feat_enums feat); // includes strings for Mutli-selection feat categories e.g. FEAT_WEAPON_FOCUS
	TigTextStyle & GetFeatStyle(feat_enums feat, bool allowMultiple = true);
	bool FeatAlreadyPicked(feat_enums feat);
	bool FeatCanPick(feat_enums feat);
	bool IsSelectingRangerSpec();
	bool IsClassBonusFeat(feat_enums feat);
	void SetBonusFeats(std::vector<FeatInfo> & fti);
	void FeatsSanitize();
	void FeatsMultiSelectActivate(feat_enums feat);
	feat_enums FeatsMultiGetFirst(feat_enums feat); // first alphabetical

	// widget IDs
	int classWndId = 0;
	int classNextBtn = 0, classPrevBtn = 0;
	eastl::vector<int> classBtnIds;

		// geometry
		TigRect classNextBtnRect, classNextBtnFrameRect, classNextBtnTextRect,
		classPrevBtnRect, classPrevBtnFrameRect, classPrevBtnTextRect;
		TigRect spellsChosenTitleRect, spellsAvailTitleRect;
		TigRect spellsPerDayTitleRect;
		int featsMultiCenterX, featsMultiCenterY;
		TigRect featMultiOkRect, featMultiOkTextRect, featMultiCancelRect, featMultiCancelTextRect, featMultiTitleRect;
		TigRect featsAvailTitleRect, featsTitleRect, featsExistingTitleRect, featsClassBonusRect;
		TigRect featsSelectedBorderRect, featsClassBonusBorderRect, feat0TextRect, feat1TextRect, feat2TextRect;

	int featsMainWndId = 0, featsMultiSelectWndId =0;
	int featsScrollbarId =0, featsExistingScrollbarId =0, featsMultiSelectScrollbarId =0;
	int featsScrollbarY =0, featsExistingScrollbarY =0, featsMultiSelectScrollbarY =0;
	WidgetType1 featsMainWnd, featsMultiSelectWnd;
	WidgetType3 featsScrollbar, featsExistingScrollbar, featsMultiSelectScrollbar;
	eastl::vector<int> featsAvailBtnIds, featsExistingBtnIds, featsMultiSelectBtnIds;
	int featsMultiOkBtnId = 0, featsMultiCancelBtnId = 0;
	const int FEATS_AVAIL_BTN_COUNT = 17; // vanilla 18
	const int FEATS_AVAIL_BTN_HEIGHT = 12; // vanilla 11
	const int FEATS_EXISTING_BTN_COUNT = 10; // vanilla 11
	const int FEATS_EXISTING_BTN_HEIGHT = 13; // vanilla 12
	const int FEATS_MULTI_BTN_COUNT = 15;
	const int FEATS_MULTI_BTN_HEIGHT = 12;
	std::string featsAvailTitleString, featsExistingTitleString;
	std::string featsTitleString;
	std::string featsClassBonusTitleString;


	int spellsWndId = 0;
	WidgetType1 spellsWnd;
	WidgetType3 spellsScrollbar, spellsScrollbar2;
	int spellsScrollbarId = 0, spellsScrollbar2Id = 0;
	int spellsScrollbarY = 0, spellsScrollbar2Y = 0;
	eastl::vector<int> spellsAvailBtnIds, spellsChosenBtnIds;
	const int SPELLS_BTN_COUNT = 17; // vanilla had 20, decreasing this to increase the font
	const int SPELLS_BTN_HEIGHT = 13; // vanilla was 11 (so 13*17 = 221 ~= 220 vanilla)
	std::string spellsAvailLabel;
	std::string spellsChosenLabel;
	std::string spellsPerDayLabel;
	const int SPELLS_PER_DAY_BOXES_COUNT = 6;


	// caches
	eastl::hash_map<int, eastl::string> classNamesUppercase;
	eastl::vector<TigRect> classBtnFrameRects;
	eastl::vector<TigRect> classBtnRects;
	eastl::vector<TigRect> classTextRects;
	eastl::vector<TigRect> featsMultiBtnRects;
	eastl::vector<TigRect> featsBtnRects, featsExistingBtnRects;
	eastl::hash_map<int, std::string> featsMasterFeatStrings;
	eastl::vector<string> levelLabels;
	eastl::vector<string> spellLevelLabels;
	eastl::vector<string> spellsPerDayTexts;
	eastl::vector<TigRect> spellsPerDayTextRects;
	eastl::vector<TigRect> spellsNewSpellsBoxRects;
	

	// art assets
	int buttonBox = 0;
	ColorRect genericShadowColor = ColorRect(0xFF000000);
	ColorRect whiteColorRect = ColorRect(0xFFFFffff);
	ColorRect blueColorRect = ColorRect(0xFF0000ff);
	ColorRect classBtnShadowColor = ColorRect(0xFF000000);
	ColorRect classBtnColorRect = ColorRect(0xFFFFffff);
	TigTextStyle whiteTextGenericStyle;
	TigTextStyle blueTextStyle;
	TigTextStyle classBtnTextStyle;
	TigTextStyle featsGreyedStyle, featsBonusTextStyle, featsExistingTitleStyle, featsGoldenStyle, featsClassStyle, featsCenteredStyle;
	TigTextStyle spellsTextStyle;
	TigTextStyle spellsTitleStyle;
	TigTextStyle spellLevelLabelStyle;
	TigTextStyle spellsAvailBtnStyle;
	TigTextStyle spellsPerDayStyle;
	TigTextStyle spellsPerDayTitleStyle;
	CombinedImgFile* levelupSpellbar, *featsbackdrop;


	CharEditorClassSystem& GetClass() const {
		Expects(!!mClass);
		return *mClass;
	}

	UiCharEditor(){
		TigTextStyle baseStyle;
		baseStyle.flags = 0x4000;
		baseStyle.field2c = -1;
		baseStyle.shadowColor = &genericShadowColor;
		baseStyle.field0 = 0;
		baseStyle.kerning = 1;
		baseStyle.leading = 0;
		baseStyle.tracking = 3;
		baseStyle.textColor = baseStyle.colors2 = baseStyle.colors4 = &whiteColorRect;
		whiteTextGenericStyle = baseStyle;

		blueTextStyle = baseStyle;
		blueTextStyle.colors4 = blueTextStyle.colors2 = blueTextStyle.textColor = &blueColorRect;
	}

private:
	int mPageCount = 0;

	bool mFeatsActivated = false;
	bool mIsSelectingBonusFeat = false;
	bool mBonusFeatOk = false;
	feat_enums featsMultiSelected = FEAT_NONE, mFeatsMultiMasterFeat = FEAT_NONE;

	std::unique_ptr<CharEditorClassSystem> mClass;
	std::vector<KnownSpellInfo> mSpellInfo;
	std::vector<KnownSpellInfo> mAvailableSpells; // spells available for learning
	std::vector<FeatInfo> mExistingFeats, mSelectableFeats, mMultiSelectFeats, mMultiSelectMasterFeats, mBonusFeats;
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
		.def(py::init<int, int, int, int>(), py::arg("spell_enum"), py::arg("spell_status"), py::arg("spell_class"), py::arg("is_domain_spell") = 0)
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

			auto firstClass = self.spellClass;
			auto secondClass = other.spellClass;

			if (firstClass != secondClass){
				return firstClass - secondClass;
			}

			// if they are of the same class

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
			.def_readwrite("spell_class", &KnownSpellInfo::spellClass, "Note: this is not the same as the casting class enum (use set_casting_class instead)")
			.def_readwrite("spell_level", &KnownSpellInfo::spellLevel, "Spell level; is calculated automatically on init according to the spell class")
		.def("get_casting_class", [](KnownSpellInfo& ksi)->int{
			return spellSys.GetCastingClass(ksi.spellClass);
		})
		.def("set_casting_class", [](KnownSpellInfo& ksi, int classEnum) {
			ksi.spellClass = spellSys.GetCastingClass(classEnum);
		}, py::arg("class_enum"))
		;

	py::class_<FeatInfo>(mm, "FeatInfo")
		.def(py::init<int>(), py::arg("feat_enum"))
		.def_readwrite("feat_enum", &FeatInfo::featEnum)
		.def_readwrite("feat_status", &FeatInfo::flag, "0 - normal, 1 - automatic class feat, 2 - bonus selectable feat")
		;
		// methods
	mm
	.def("set_bonus_feats", [](std::vector<FeatInfo> & fti){
		uiCharEditor.SetBonusFeats(fti);
	})
	.def("get_spell_enums", []()->std::vector<KnownSpellInfo>& {
		return uiCharEditor.GetKnownSpellInfo();
	})
	.def("set_spell_enums", [](std::vector<KnownSpellInfo> &ksi){
		auto &spInfo = uiCharEditor.GetKnownSpellInfo();
		spInfo = ksi;
	})
	.def("append_spell_enums", [](std::vector<KnownSpellInfo> &ksi) {
		auto &spInfo = uiCharEditor.GetKnownSpellInfo();
		for (auto i = 0u; i < ksi.size(); i++){
			spInfo.push_back(ksi[i]);
		}
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
	.def("spells_check_complete", [](objHndl handle, int classEnum)->int{
		if (d20ClassSys.IsSelectingSpellsOnLevelup(handle, (Stat)classEnum))
			return d20ClassSys.LevelupSpellsCheckComplete(handle, (Stat)classEnum);
		else
			return 1;
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
			knownSpells.push_back(KnownSpellInfo(_knownSpells[i], 0, spellSys.GetSpellClass(classEnum) ));
		}
		return knownSpells;
	})
	.def("get_learnable_spells", [](objHndl handle, int classEnum, int maxSpellLvl, int is_domain_spell_class)->std::vector<KnownSpellInfo>
	{
		auto result = std::vector<KnownSpellInfo>();
		auto spellClass = spellSys.GetSpellClass(classEnum, is_domain_spell_class != 0);
		std::vector<SpellEntry> spEntries;
		auto numSpells = spellSys.CopyLearnableSpells(handle, spellClass, spEntries);
		for (auto i = 0u; i < spEntries.size(); i++) {
			auto shouldCull = false;

			auto &spEnt = spEntries[i];

			auto spLvl = spellSys.GetSpellLevelBySpellClass(spEnt.spellEnum, spellClass);
			if (spLvl < 0 || spLvl > maxSpellLvl)
				shouldCull = true;

			if (!shouldCull)
				result.push_back(KnownSpellInfo(spEnt.spellEnum, 0, spellClass, is_domain_spell_class ));
		}
		return result;

	}, py::arg("obj_handle"), py::arg("class_enum"), py::arg("max_spell_level"),py::arg("is_domain_spell_class") = 0)
	.def("get_spell_level", [](int spEnum, int classEnum)->int {
		auto spellClass = spellSys.GetSpellClass(classEnum);
		return spellSys.GetSpellLevelBySpellClass(spEnum, spellClass);
	})
	.def("populate_available_spells", [](int classEnum, int maxSpellLvl, int skipCantrips){
		uiCharEditor.SpellsPopulateAvailableEntries((Stat)classEnum, maxSpellLvl, skipCantrips != 0);
	}, py::arg("class_enum"), py::arg("max_spell_level"), py::arg("skip_cantrips") = 0)
	.def("append_available_spells", [](std::vector<KnownSpellInfo> & ksi){
		auto &avSpells = uiCharEditor.GetAvailableSpells();
		for (auto i = 0u; i < ksi.size(); i++) {
			avSpells.push_back(ksi[i]);
		}
	})
	.def("spell_known_add", [](std::vector<KnownSpellInfo> &ksi){
		auto handle = uiCharEditor.GetEditedChar();
		for (auto it: ksi){
			auto spEnum = it.spEnum;
			if (spellSys.IsLabel(spEnum)
				|| spEnum == SPELL_ENUM_VACANT
				|| spellSys.IsNewSlotDesignator(spEnum))
				continue;

			SpellStoreData spData(spEnum, it.spellLevel, it.spellClass, 0, SpellStoreType::spellStoreKnown );
			if (spData.spellLevel == -1)
				spData.spellLevel = spellSys.GetSpellLevelBySpellClass(spEnum, spData.classCode);
			
			if (spellSys.IsSpellKnown(handle,spEnum, spData.classCode))
				continue;

			spellSys.SpellKnownAdd(handle, spEnum, spData.classCode, spData.spellLevel, SpellStoreType::spellStoreKnown, 0 );
		}
	})
	;


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

std::vector<KnownSpellInfo>& UiCharEditor::GetAvailableSpells(){
	return mAvailableSpells;
}

void UiCharEditor::PrepareNextStages(){
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


	mIsSelectingBonusFeat = false;
	// feats and features
	if (classCode >= stat_level_barbarian){

		auto classLvlNew = GetNewLvl(classCode);

		if (d20ClassSys.IsSelectingFeatsOnLevelup(handle, classCode) ) {
			ui.ButtonSetButtonState(stateBtnIds[4], UBS_NORMAL); // feats
			mIsSelectingBonusFeat = true;
		}

		
		if (classCode == stat_level_cleric) {
			if (classLvlNew == 1)
				ui.ButtonSetButtonState(stateBtnIds[2], UBS_NORMAL); // features
		}
		if (classCode == stat_level_ranger) {
			if (classLvlNew == 1 || classLvlNew == 2 || !(classLvlNew % 5))
				ui.ButtonSetButtonState(stateBtnIds[2], UBS_NORMAL); // features
		}
		if (classCode == stat_level_wizard) {
			if (classLvlNew == 1)
				ui.ButtonSetButtonState(stateBtnIds[2], UBS_NORMAL); // wizard special school
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

BOOL UiCharEditor::FeatsSystemInit(GameSystemConf & conf){

	auto pcCreationMes = temple::GetRef<MesHandle>(0x11E72EF0);
	MesLine mesline;

	TigTextStyle baseStyle;
	baseStyle.flags = 0x4000;
	baseStyle.field2c = -1;
	baseStyle.shadowColor = &genericShadowColor;
	baseStyle.field0 = 0;
	baseStyle.kerning = 1;
	baseStyle.leading = 0;
	baseStyle.tracking = 3;
	baseStyle.textColor = baseStyle.colors2 = baseStyle.colors4 = &whiteColorRect;

	featsCenteredStyle = featsGreyedStyle = featsBonusTextStyle = featsExistingTitleStyle =  featsGoldenStyle =  featsClassStyle = baseStyle;

	featsCenteredStyle.flags = 0x10;

	static ColorRect goldenColor(0xFFFFD919);
	featsGoldenStyle.colors2 = featsGoldenStyle.colors4 = featsGoldenStyle.textColor = &goldenColor;

	static ColorRect greyColor(0xFF5D5D5D);
	featsGreyedStyle.colors2 = featsGreyedStyle.colors4 = featsGreyedStyle.textColor = &greyColor;

#pragma region Titles and strings
	// Feats Available title
	mesline.key = 19000;
	mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
	featsAvailTitleString.append(mesline.value);

	// Existing Feats title
	mesline.key = 19005;
	mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
	featsExistingTitleString.append(mesline.value);

	// Feats title
	mesline.key = 19001;
	mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
	featsTitleString.append(mesline.value);

	// Class Bonus title
	mesline.key = 19003;
	mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
	featsClassBonusTitleString.append(mesline.value);

	static auto prefabFeatsMasterMesPairs = eastl::hash_map<int, int>({
		{ FEAT_EXOTIC_WEAPON_PROFICIENCY, 19101 },
		{ FEAT_IMPROVED_CRITICAL , 19102 },
		{ FEAT_MARTIAL_WEAPON_PROFICIENCY , 19103 },
		{ FEAT_SKILL_FOCUS , 19104 },
		{ FEAT_WEAPON_FINESSE , 19105 },
		{ FEAT_WEAPON_FOCUS, 19106 },
		{ FEAT_WEAPON_SPECIALIZATION , 19107 },
		{ FEAT_GREATER_WEAPON_FOCUS , 19108 }
	});
	
	for (auto it: prefabFeatsMasterMesPairs){
		mesline.key = it.second;
		mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
		featsMasterFeatStrings[it.first].append(mesline.value);
	}
	featsMasterFeatStrings[FEAT_GREATER_WEAPON_SPECIALIZATION].append(feats.GetFeatName(FEAT_GREATER_WEAPON_SPECIALIZATION));


#pragma endregion

	featsbackdrop = new CombinedImgFile("art\\interface\\pc_creation\\meta_backdrop.img");
	if (!featsbackdrop)
		return 0;
	return FeatsWidgetsInit(conf.width, conf.height);
}

BOOL UiCharEditor::FeatsCheckComplete(){

	auto handle = GetEditedChar();
	auto &selPkt = GetCharEditorSelPacket();

	// is a 3rd level and no feat chosen
	if (IsSelectingNormalFeat()  && selPkt.feat0 == FEAT_NONE)
		return 0;

	if (IsSelectingBonusFeat() && selPkt.feat2 == FEAT_NONE) // the logic will be handled in the msg callbacks & Python API now
		return 0;

	return 1;
}

void UiCharEditor::FeatsFinalize()
{
}

void UiCharEditor::FeatsReset(CharEditorSelectionPacket & selPkt){
	mFeatsActivated = false;
	mIsSelectingBonusFeat = false;

	selPkt.feat0 = FEAT_NONE;
	selPkt.feat1 = FEAT_NONE;
	if (selPkt.classCode != stat_level_ranger || objects.StatLevelGet(GetEditedChar(), stat_level_ranger) != 1)
		selPkt.feat2 = FEAT_NONE;

	mExistingFeats.clear();
	mSelectableFeats.clear();
	mMultiSelectFeats.clear();
}

BOOL UiCharEditor::SpellsSystemInit(GameSystemConf & conf){

	auto pcCreationMes = temple::GetRef<MesHandle>(0x11E72EF0);
	MesLine mesline;

	TigTextStyle baseStyle;
	baseStyle.flags = 0;
	baseStyle.field2c = -1;
	baseStyle.shadowColor = &genericShadowColor;
	baseStyle.field0 = 0;
	baseStyle.kerning = 1;
	baseStyle.leading = 0;
	baseStyle.tracking = 3;
	baseStyle.textColor = baseStyle.colors2 = baseStyle.colors4 = &whiteColorRect;
	

	spellsTitleStyle = baseStyle;

	// generic spells text style
	spellsTextStyle = baseStyle;

	// Spell Level Label Style
	spellLevelLabelStyle = baseStyle;
	static ColorRect spellLevelLabelColor(0x0FF43586E);
	spellLevelLabelStyle.textColor = spellLevelLabelStyle.colors2 = spellLevelLabelStyle.colors4 = &spellLevelLabelColor;

	// Spells Available Btn Style
	spellsAvailBtnStyle = baseStyle;
	static ColorRect spellsAvailColor1(0x0FF5D5D5D);
	spellsAvailBtnStyle.textColor = &spellsAvailColor1;
	spellsAvailBtnStyle.colors2 = &spellsAvailColor1;
	spellsAvailBtnStyle.colors4 = &spellsAvailColor1;

	
	// Spells Per Day style
	spellsPerDayStyle = baseStyle;

	spellsPerDayTitleStyle = baseStyle;

	// Spells Available title
	mesline.key = 21000;
	mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
	spellsAvailLabel.append(mesline.value);

	// Spells Chosen title
	mesline.key = 21001;
	mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
	spellsChosenLabel.append(mesline.value);

	// Spells Per Day title
	mesline.key = 21002;
	mesFuncs.GetLine_Safe(pcCreationMes, &mesline);
	spellsPerDayLabel.append(mesline.value); 

	// Spell Level label texts
	MesLine line(21100), line2(21200);
	mesFuncs.GetLine_Safe(pcCreationMes, &line);
	mesFuncs.GetLine_Safe(pcCreationMes, &line2);

	for (auto i = 0; i < NUM_SPELL_LEVELS; i++){
		std::string text;
		text.append(line2.value);
		text[text.size()-1] = '0'+i;
		levelLabels.push_back(text);

		text.clear();
		text.append(line.value);
		text[text.size() - 1] = '0' + i;

		spellLevelLabels.push_back(text);
	}

	for (auto i = 0; i < SPELLS_PER_DAY_BOXES_COUNT;i++){
		spellsPerDayTextRects.push_back(TigRect());
	}

	levelupSpellbar = new CombinedImgFile("art\\interface\\pc_creation\\levelup_spellbar.img");
	if (!levelupSpellbar)
		return 0;

	// Widgets
	return uiCharEditor.SpellsWidgetsInit();
}

void UiCharEditor::SpellsFree(){
	SpellsWidgetsFree();
}


BOOL UiCharEditor::FeatsWidgetsInit(int w, int h) {
	featsMainWnd = WidgetType1(259, 117, 405, 271);
	featsMainWnd.widgetFlags = 1;
	featsMainWnd.render = [](int widId) {uiCharEditor.FeatsWndRender(widId); };
	featsMainWnd.handleMessage = [](int widId, TigMsg*msg) { return uiCharEditor.FeatsWndMsg(widId, msg); };
	featsMainWnd.Add(&featsMainWndId);

	// multi select wnd
	featsMultiCenterX = (w - 289) / 2;
	featsMultiCenterY = (h - 355) / 2;
	featsMultiSelectWnd = WidgetType1(0, 0, w, h);
	auto featsMultiRefX = featsMultiCenterX + featsMultiSelectWnd.x;
	auto featsMultiRefY = featsMultiCenterY + featsMultiSelectWnd.y;
	featsMultiSelectWnd.widgetFlags = 1;
	featsMultiSelectWnd.render = [](int widId) {uiCharEditor.FeatsMultiSelectWndRender(widId); };
	featsMultiSelectWnd.handleMessage = [](int widId, TigMsg*msg) { return uiCharEditor.FeatsMultiSelectWndMsg(widId, msg); };
	featsMultiSelectWnd.Add(&featsMultiSelectWndId);
	//scrollbar
	featsMultiSelectScrollbar.Init(256, 71, 219);
	featsMultiSelectScrollbar.parentId = featsMultiSelectWndId;
	featsMultiSelectScrollbar.x += featsMultiRefX;
	featsMultiSelectScrollbar.y += featsMultiRefY;
	featsMultiSelectScrollbar.Add(&featsMultiSelectScrollbarId);
	ui.BindToParent(featsMultiSelectWndId, featsMultiSelectScrollbarId);
	
	//ok btn
	{
		int newId = 0;
		WidgetType2 multiOkBtn("Feats Multiselect Ok Btn", featsMultiSelectWndId, 29, 307, 110, 22);
		multiOkBtn.x += featsMultiRefX; multiOkBtn.y += featsMultiRefY;
		featMultiOkRect = TigRect(multiOkBtn.x, multiOkBtn.y, multiOkBtn.width, multiOkBtn.height);
		featMultiOkTextRect = TigRect(multiOkBtn.x, multiOkBtn.y + 4, multiOkBtn.width, multiOkBtn.height - 8);
		multiOkBtn.render = [](int id) {uiCharEditor.FeatsMultiOkBtnRender(id); };
		multiOkBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.FeatsMultiOkBtnMsg(id, msg); };
		multiOkBtn.renderTooltip = nullptr;
		multiOkBtn.Add(&newId);
		featsMultiOkBtnId = newId;
		ui.SetDefaultSounds(newId);
		ui.BindToParent(featsMultiSelectWndId, newId);
	}

	//cancel btn
	{
		int newId = 0;
		WidgetType2 multiCancelBtn("Feats Multiselect Cancel Btn", featsMultiSelectWndId, 153, 307, 110, 22);
		multiCancelBtn.x += featsMultiRefX; multiCancelBtn.y += featsMultiRefY;
		featMultiCancelRect = TigRect(multiCancelBtn.x, multiCancelBtn.y, multiCancelBtn.width, multiCancelBtn.height);
		featMultiCancelTextRect = TigRect(multiCancelBtn.x, multiCancelBtn.y + 4, multiCancelBtn.width, multiCancelBtn.height - 8);
		multiCancelBtn.render = [](int id) {uiCharEditor.FeatsMultiCancelBtnRender(id); };
		multiCancelBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.FeatsMultiCancelBtnMsg(id, msg); };
		multiCancelBtn.renderTooltip = nullptr;
		multiCancelBtn.Add(&newId);
		featsMultiCancelBtnId = newId;
		ui.SetDefaultSounds(newId);
		ui.BindToParent(featsMultiSelectWndId, newId);
	}
	
	featMultiTitleRect = TigRect(featsMultiCenterX, featsMultiCenterY + 20, 289, 12);
	featsMultiSelectBtnIds.clear();
	featsMultiBtnRects.clear();
	auto rowOff = 75;
	for (auto i=0; i < FEATS_MULTI_BTN_COUNT; i++){
		int newId = 0;
		WidgetType2 featMultiBtn("Feats Multiselect btn", featsMultiSelectWndId, 23, 75 + i*(FEATS_MULTI_BTN_HEIGHT+2), 233, FEATS_MULTI_BTN_HEIGHT);

		featMultiBtn.x += featsMultiRefX; featMultiBtn.y += featsMultiRefY;
		featMultiBtn.render = [](int id) {uiCharEditor.FeatsMultiBtnRender(id); };
		featMultiBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.FeatsMultiBtnMsg(id, msg); };
		featMultiBtn.renderTooltip = nullptr;
		featMultiBtn.Add(&newId);
		featsMultiSelectBtnIds.push_back(newId);
		ui.SetDefaultSounds(newId);
		ui.BindToParent(featsMultiSelectWndId, newId);
		featsMultiBtnRects.push_back(TigRect(featMultiBtn.x, featMultiBtn.y, featMultiBtn.width, featMultiBtn.height));
	}

	featsAvailTitleRect = TigRect(7,22,185, 10 );
	featsTitleRect = TigRect(206, 27, 185, 10);
	featsExistingTitleRect =TigRect(206, 103, 185, 10);
	featsClassBonusRect = TigRect(206, 65, 185, 10);

	// Selectable feats
	featsAvailBtnIds.clear();
	featsBtnRects.clear();
	for (auto i = 0; i < FEATS_AVAIL_BTN_COUNT; i++) {
		int newId = 0;
		WidgetType2 featsAvailBtn("Feats Available btn", featsMainWndId, 7, 38 + i*(FEATS_AVAIL_BTN_HEIGHT + 1), 169, FEATS_AVAIL_BTN_HEIGHT);

		featsAvailBtn.x += featsMainWnd.x; featsAvailBtn.y += featsMainWnd.y;
		featsAvailBtn.render = [](int id) {uiCharEditor.FeatsEntryBtnRender(id); };
		featsAvailBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.FeatsEntryBtnMsg(id, msg); };
		featsAvailBtn.renderTooltip = nullptr;
		featsAvailBtn.Add(&newId);
		featsAvailBtnIds.push_back(newId);
		ui.SetDefaultSounds(newId);
		ui.BindToParent(featsMainWndId, newId);
		featsBtnRects.push_back(TigRect(featsAvailBtn.x - featsMainWnd.x, featsAvailBtn.y - featsMainWnd.y, featsAvailBtn.width, featsAvailBtn.height));
	}
	//scrollbar
	featsScrollbar.Init(186, 35, 230);
	featsScrollbar.parentId = featsMainWndId;
	featsScrollbar.x += featsMainWnd.x;
	featsScrollbar.y += featsMainWnd.y;
	featsScrollbar.Add(&featsScrollbarId);
	ui.BindToParent(featsMainWndId, featsScrollbarId);


	// Existing feats
	featsExistingBtnIds.clear();
	featsExistingBtnRects.clear();
	for (auto i = 0; i < FEATS_EXISTING_BTN_COUNT; i++) {
		int newId = 0;
		WidgetType2 featsExistingBtn("Feats Existing btn", featsMainWndId, 212, 121 + i*(FEATS_EXISTING_BTN_HEIGHT + 1), 175, FEATS_EXISTING_BTN_HEIGHT);

		featsExistingBtn.x += featsMainWnd.x; featsExistingBtn.y += featsMainWnd.y;
		featsExistingBtn.render = [](int id) {uiCharEditor.FeatsExistingBtnRender(id); };
		featsExistingBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.FeatsExistingBtnMsg(id, msg); };
		featsExistingBtn.renderTooltip = nullptr;
		featsExistingBtn.Add(&newId);
		featsExistingBtnIds.push_back(newId);
		ui.SetDefaultSounds(newId);
		ui.BindToParent(featsMainWndId, newId);
		featsExistingBtnRects.push_back(TigRect(featsExistingBtn.x - featsMainWnd.x, featsExistingBtn.y - featsMainWnd.y, featsExistingBtn.width, featsExistingBtn.height));
	}
	//scrollbar
	featsExistingScrollbar.Init(381, 117, 148);
	featsExistingScrollbar.parentId = featsMainWndId;
	featsExistingScrollbar.x += featsMainWnd.x;
	featsExistingScrollbar.y += featsMainWnd.y;
	featsExistingScrollbar.Add(&featsExistingScrollbarId);
	ui.BindToParent(featsMainWndId, featsExistingScrollbarId);

	featsSelectedBorderRect = TigRect(featsMainWnd.x + 207, featsMainWnd.y + 42, 185, 19 );
	featsClassBonusBorderRect = TigRect(featsMainWnd.x + 207, featsMainWnd.y + 81, 185, 19);
	feat0TextRect = TigRect(209, 46, 185, 12);
	feat1TextRect = TigRect(209, 46+21, 185, 12);
	feat2TextRect = TigRect(209, 85, 185, 12);

	return 1;
}

void UiCharEditor::FeatsFree(){
	FeatWidgetsFree();
}

void UiCharEditor::FeatWidgetsFree(){
	for (auto i = 0; i < FEATS_MULTI_BTN_COUNT; i++) {
		ui.WidgetRemoveRegardParent(featsMultiSelectBtnIds[i]);
	}
	featsMultiSelectBtnIds.clear();

	for (auto i = 0; i < FEATS_AVAIL_BTN_COUNT; i++) {
		ui.WidgetRemoveRegardParent(featsAvailBtnIds[i]);
	}
	featsAvailBtnIds.clear();

	for (auto i = 0; i < FEATS_EXISTING_BTN_COUNT; i++) {
		ui.WidgetRemoveRegardParent(featsExistingBtnIds[i]);
	}
	featsExistingBtnIds.clear();

	ui.WidgetRemoveRegardParent(featsMultiOkBtnId);
	ui.WidgetRemoveRegardParent(featsMultiCancelBtnId);
	ui.WidgetRemoveRegardParent(featsMultiSelectScrollbarId);
	ui.WidgetRemoveRegardParent(featsScrollbarId);
	ui.WidgetRemoveRegardParent(featsExistingScrollbarId);

	auto wid = ui.WidgetGetType1(featsMultiSelectWndId);
	auto wid2 = ui.WidgetGetType1(featsMainWndId);

	ui.WidgetRemove(featsMultiSelectWndId);
	ui.WidgetRemove(featsMainWndId);
	
	
}

BOOL UiCharEditor::FeatsWidgetsResize(UiResizeArgs & args){
	FeatWidgetsFree();
	return FeatsWidgetsInit(args.rect1.width, args.rect1.height);
}

BOOL UiCharEditor::FeatsShow(){
	featsMultiSelected = FEAT_NONE;
	ui.WidgetSetHidden(featsMainWndId, 0);
	ui.WidgetBringToFront(featsMainWndId);
	return 1;
}

BOOL UiCharEditor::FeatsHide()
{
	ui.WidgetSetHidden(featsMainWndId, 1);
	return 0;
}

void UiCharEditor::FeatsActivate(){
	mFeatsActivated = true;

	auto handle = GetEditedChar();
	auto &selPkt = GetCharEditorSelPacket();

	mIsSelectingBonusFeat = d20ClassSys.IsSelectingFeatsOnLevelup(handle, selPkt.classCode);
	mBonusFeats.clear();
	if (mIsSelectingBonusFeat)
		d20ClassSys.LevelupGetBonusFeats(handle, selPkt.classCode);

	feat_enums existingFeats[122];
	auto isRangerStyleChoosing = selPkt.classCode == stat_level_ranger && (objects.StatLevelGet(handle, stat_level_ranger) == 1);
	auto existingCount = feats.FeatListGet(handle, existingFeats, selPkt.classCode, isRangerStyleChoosing == true?selPkt.feat2:FEAT_ACROBATIC);

	mExistingFeats.clear();
	for (auto i=0u; i<existingCount; i++){
		auto ftEnum = existingFeats[i];
		if (selPkt.feat0 != ftEnum && selPkt.feat1 != ftEnum && selPkt.feat2 != ftEnum)
			mExistingFeats.push_back(FeatInfo(ftEnum));
	}
	static auto featSorter = [](FeatInfo &first, FeatInfo &second) {

		auto firstEnum = (feat_enums)first.featEnum;
		auto secEnum = (feat_enums)second.featEnum;

		auto firstName = uiCharEditor.GetFeatName(firstEnum);
		auto secondName = uiCharEditor.GetFeatName(secEnum);

		return _stricmp(firstName.c_str(), secondName.c_str()) < 0;
	};

	std::sort(mExistingFeats.begin(), mExistingFeats.end(), featSorter);

	ui.WidgetCopy(featsExistingScrollbarId, &featsExistingScrollbar);
	featsExistingScrollbar.scrollbarY = 0;
	featsExistingScrollbarY = 0;
	featsExistingScrollbar.yMax = max((int)mExistingFeats.size() - FEATS_EXISTING_BTN_COUNT, 0);
	ui.WidgetSet(featsExistingScrollbarId, &featsExistingScrollbar);

	// Available feats
	mSelectableFeats.clear();
	for (auto i = 0u; i < NUM_FEATS; i++){
		auto feat = (feat_enums)i;
		if (!feats.IsFeatEnabled(feat) && !feats.IsFeatMultiSelectMaster(feat))
			continue;
		if (feats.IsFeatRacialOrClassAutomatic(feat))
			continue;
		if (feats.IsFeatPartOfMultiselect(feat))
			continue;
		if (feat == FEAT_NONE)
			continue;
		mSelectableFeats.push_back(FeatInfo(feat));
	}
	std::sort(mSelectableFeats.begin(), mSelectableFeats.end(), featSorter);

	ui.WidgetCopy(featsScrollbarId, &featsScrollbar);
	featsScrollbar.scrollbarY = 0;
	featsScrollbarY= 0;
	featsScrollbar.yMax = max((int)mSelectableFeats.size() - FEATS_AVAIL_BTN_COUNT, 0);
	ui.WidgetSet(featsScrollbarId, &featsScrollbar);
}

BOOL UiCharEditor::SpellsWidgetsInit(){

	const int spellsWndX = 259, spellsWndY = 117, spellsWndW = 405, spellsWndH = 271;
	spellsWnd = WidgetType1(spellsWndX, spellsWndY, spellsWndW, spellsWndH);
	spellsWnd.widgetFlags = 1;
	spellsWnd.render = [](int widId) {uiCharEditor.SpellsWndRender(widId); };
	spellsWnd.handleMessage = [](int widId, TigMsg*msg) { return uiCharEditor.SpellsWndMsg(widId, msg); };
	spellsWnd.Add(&spellsWndId);

	// Available Spells Scrollbar
	spellsScrollbar.Init(183, 37, 230);
	spellsScrollbar.parentId = spellsWndId;
	spellsScrollbar.x += spellsWnd.x;
	spellsScrollbar.y += spellsWnd.y;
	spellsScrollbar.Add(&spellsScrollbarId);
	ui.BindToParent(spellsWndId, spellsScrollbarId);

	// Spell selection scrollbar
	spellsScrollbar2.Init(385, 37, 230);
	spellsScrollbar2.parentId = spellsWndId;
	spellsScrollbar2.x += spellsWnd.x;
	spellsScrollbar2.y += spellsWnd.y;
	spellsScrollbar2.Add(&spellsScrollbar2Id);
	ui.BindToParent(spellsWndId, spellsScrollbar2Id);

	int rowOff = 39;
	for (auto i = 0; i < SPELLS_BTN_COUNT; i++, rowOff += SPELLS_BTN_HEIGHT){
		
		int newId = 0;
		WidgetType2 spellAvailBtn("Spell Available btn", spellsWndId, 4, rowOff, 180, SPELLS_BTN_HEIGHT);

		spellAvailBtn.x += spellsWnd.x; spellAvailBtn.y += spellsWnd.y;
		spellAvailBtn.render = [](int id) {uiCharEditor.SpellsAvailableEntryBtnRender(id); };
		spellAvailBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.SpellsAvailableEntryBtnMsg(id, msg); };
		spellAvailBtn.renderTooltip = nullptr;
		spellAvailBtn.Add(&newId);
		spellsAvailBtnIds.push_back(newId);
		ui.SetDefaultSounds(newId);
		ui.BindToParent(spellsWndId, newId);

		WidgetType2 spellChosenBtn("Spell Chosen btn", spellsWndId, 206, rowOff, 170, SPELLS_BTN_HEIGHT);

		spellChosenBtn.x += spellsWnd.x; spellChosenBtn.y += spellsWnd.y;
		spellChosenBtn.render = [](int id) {uiCharEditor.SpellsEntryBtnRender(id); };
		spellChosenBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.SpellsEntryBtnMsg(id, msg); };
		spellChosenBtn.renderTooltip = nullptr;
		spellChosenBtn.Add(&newId);
		spellsChosenBtnIds.push_back(newId);
		ui.SetDefaultSounds(newId);
		ui.BindToParent(spellsWndId, newId);

	}

	// titles
	spellsAvailTitleRect.x = 5;
	spellsAvailTitleRect.y = 23;
	spellsChosenTitleRect.x = 219;
	spellsChosenTitleRect.y = 23;
	UiRenderer::PushFont("priory-12", 12);

	// Spells Per Day title
	auto spellsPerDayMeasure = UiRenderer::MeasureTextSize(spellsPerDayLabel, spellsTextStyle);
	spellsPerDayTitleRect = TigRect(5, 279, 99, 12);
	spellsPerDayTitleRect.x += (spellsPerDayTitleRect.width - spellsPerDayMeasure.width) / 2;
	spellsPerDayTitleRect.width = spellsPerDayMeasure.width;
	spellsPerDayTitleRect.height = spellsPerDayMeasure.height;

	// Spell Level labels
	TigTextStyle &spellLevelLabelStyle = temple::GetRef<TigTextStyle>(0x10C74B08);
	spellsNewSpellsBoxRects.clear();
	for (auto lvl = 0u; lvl < NUM_SPELL_LEVELS; lvl++){
		auto textMeas = UiRenderer::MeasureTextSize(levelLabels[lvl].c_str(), spellLevelLabelStyle);
		spellsNewSpellsBoxRects.push_back(TigRect(116 + lvl * 45, 287, 29, 12));

	}
	UiRenderer::PopFont();
	return 1;
}

void UiCharEditor::SpellsWidgetsFree(){
	for (auto i = 0; i < SPELLS_BTN_COUNT; i++){
		ui.WidgetRemoveRegardParent(spellsChosenBtnIds[i]);
		ui.WidgetRemoveRegardParent(spellsAvailBtnIds[i]);
	}
	spellsChosenBtnIds.clear();
	spellsAvailBtnIds.clear();
	ui.WidgetRemove(spellsWndId);
}

BOOL UiCharEditor::SpellsShow(){
	ui.WidgetSetHidden(spellsWndId, 0);
	ui.WidgetBringToFront(spellsWndId);
	return 1;
}

BOOL UiCharEditor::SpellsHide(){
	ui.WidgetSetHidden(spellsWndId, 1);
	return 0;
}

BOOL UiCharEditor::SpellsWidgetsResize(UiResizeArgs & args){
	SpellsWidgetsFree();
	SpellsWidgetsInit();
	return 0;
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
		auto sbId = uiCharEditor.spellsScrollbarId;
		ui.ScrollbarSetY(sbId, 0);
		int numEntries = (int)uiCharEditor.mAvailableSpells.size();
		ui.ScrollbarSetYmax(sbId, max(0, numEntries - uiCharEditor.SPELLS_BTN_COUNT));
		ui.WidgetCopy(sbId, &uiCharEditor.spellsScrollbar);
		uiCharEditor.spellsScrollbar.y = 0;
		uiCharEditor.spellsScrollbarY = 0;

		auto &charEdSelPkt = uiCharEditor.GetCharEditorSelPacket();
		auto sbAddedId = uiCharEditor.spellsScrollbar2Id;
		int numAdded = (int)uiCharEditor.mSpellInfo.size();
		ui.ScrollbarSetY(sbAddedId, 0); 
		ui.ScrollbarSetYmax(sbAddedId, max(0, numAdded - uiCharEditor.SPELLS_BTN_COUNT));
		ui.WidgetCopy(sbAddedId, &uiCharEditor.spellsScrollbar2);
		uiCharEditor.spellsScrollbar2.y = 0;
		uiCharEditor.spellsScrollbar2Y = 0;
	};

	if (!needPopulateEntries) {
		setScrollbars();
		return;
	}


	mSpellInfo.clear();
	mAvailableSpells.clear();

	d20ClassSys.LevelupInitSpellSelection(handle, selPkt.classCode);

	for (auto i = 0u; i < mSpellInfo.size(); i++){
		auto spEnum = mSpellInfo[i].spEnum;
		if (spellSys.IsNewSlotDesignator(spEnum)){
			mSpellInfo[i].spEnum = 802;
			mSpellInfo[i].spFlag = 3;
		}
	}

	SpellsPerDayUpdate();

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

	return d20ClassSys.LevelupSpellsCheckComplete(GetEditedChar(), selPkt.classCode);

}

void UiCharEditor::SpellsFinalize(){
	auto charEdited = GetEditedChar();
	auto &selPkt = GetCharEditorSelPacket();

	d20ClassSys.LevelupSpellsFinalize(charEdited, selPkt.classCode);
}

void UiCharEditor::SpellsReset(CharEditorSelectionPacket & selPkt){
	temple::GetRef<int>(0x10C4D4C4) = 1; // needsPopulateEntries
	mSpellInfo.clear();
	mAvailableSpells.clear();
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
		auto textboxText = fmt::format("Prestige Classes\n\n Currently limited to Eldritch Knight and Mystic Theurge. The rest are on the way!");
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

	if (!config.newClasses)
		return;

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

	if (!config.newClasses)
		return;

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


#pragma region Feats

void UiCharEditor::FeatsWndRender(int widId){
	
	auto &selPkt = GetCharEditorSelPacket();

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	
	// Feats title
	RenderHooks::RenderRectInt(featsMainWnd.x + 3, featsMainWnd.y + 36, 185, 227, 0xFF5D5D5D);
	UiRenderer::DrawTextInWidget(widId, featsAvailTitleString, featsAvailTitleRect, whiteTextGenericStyle);

	// Feat Slot
	if (IsSelectingNormalFeat()){
		RenderHooks::RenderRectInt(featsSelectedBorderRect.x , featsSelectedBorderRect.y, featsSelectedBorderRect.width, featsSelectedBorderRect.height, 0xFFFFffff);
		UiRenderer::DrawTextInWidget(widId, featsTitleString, featsTitleRect, featsBonusTextStyle);
		if (selPkt.feat0 != FEAT_NONE){
			UiRenderer::DrawTextInWidget(widId, GetFeatName(selPkt.feat0), feat0TextRect ,GetFeatStyle(selPkt.feat0));
		}
	}
	
	// Class Bonus Feat slot
	if (IsSelectingBonusFeat()){
		// title Class Bonus Feat
		RenderHooks::RenderRectInt(featsClassBonusBorderRect.x, featsClassBonusBorderRect.y, featsClassBonusBorderRect.width, featsClassBonusBorderRect.height, 0xFFFFD919);
		UiRenderer::DrawTextInWidget(widId, featsClassBonusTitleString, featsClassBonusRect, featsGoldenStyle);
		
		if (selPkt.feat2 != FEAT_NONE){
			UiRenderer::DrawTextInWidget(widId, GetFeatName(selPkt.feat2), feat2TextRect, GetFeatStyle(selPkt.feat2));
		}
	}

	// Existing Feats title
	RenderHooks::RenderRectInt(featsMainWnd.x + 207, featsMainWnd.y + 118, 185, 145, 0xFF5D5D5D);
	UiRenderer::DrawTextInWidget(widId, featsExistingTitleString, featsExistingTitleRect, featsExistingTitleStyle);

	StateTitleRender(widId);

	UiRenderer::PopFont();
}

BOOL UiCharEditor::FeatsWndMsg(int widId, TigMsg * msg)
{
	if (msg->type == TigMsgType::WIDGET) {
		auto msgW = (TigMsgWidget*)msg;
		if (msgW->widgetEventType == TigMsgWidgetEvent::Scrolled) {
			ui.ScrollbarGetY(featsScrollbarId, &featsScrollbarY);
			ui.ScrollbarGetY(featsExistingScrollbarId, &featsExistingScrollbarY);
		}
		return FALSE;
	}

	if (msg->type != TigMsgType::MOUSE)
		return FALSE;

	
	auto msgM = (TigMsgMouse*)msg;
	auto &selPkt = GetCharEditorSelPacket();

	if (msgM->buttonStateFlags & MouseStateFlags::MSF_RMB_RELEASED && ui.IsWidgetHidden(featsMultiSelectWndId)) {
		
		bool dumy = 1;
		auto putFeat = false;
		feat_enums feat;

		// cycle thru widgets to find the one where the RMB happened
		for (auto i=0; i < FEATS_AVAIL_BTN_COUNT; i++){
			if (!featsBtnRects[i].ContainsPoint(msgM->x - featsMainWnd.x, msgM->y - featsMainWnd.y))
				continue;

			auto featIdx = i + featsScrollbarY;
			if (featIdx >= (int)mSelectableFeats.size())
				break;

			feat = (feat_enums)mSelectableFeats[featIdx].featEnum;


			if (IsSelectingNormalFeat() && selPkt.feat0 == FEAT_NONE){
				selPkt.feat0 = feat;
				putFeat = true;
				break;
			} 
			else if (IsSelectingBonusFeat() && IsClassBonusFeat(feat) && selPkt.feat2 == FEAT_NONE)
			{
				selPkt.feat2 = feat;
				putFeat = true;
				break;
			}
		}

		if (putFeat){
			
			if (feats.IsFeatMultiSelectMaster(feat)){
				FeatsMultiSelectActivate(feat);
			}
			FeatsSanitize();
			if (feat == FEAT_SKILL_MASTERY && selPkt.feat2 == feat){
				auto skillMasteryActivate = temple::GetRef<void(__cdecl)(objHndl, int(__cdecl*)(int))>(0x1016C2B0);
				skillMasteryActivate(GetEditedChar(), temple::GetRef<int(__cdecl)(int)>(0x101A86D0));
			}
		}

		else if (featsSelectedBorderRect.ContainsPoint(msgM->x, msgM->y)){
			selPkt.feat0 = FEAT_NONE;
		} 
		else if (featsClassBonusBorderRect.ContainsPoint(msgM->x, msgM->y) && IsSelectingBonusFeat()){
			selPkt.feat2 = FEAT_NONE;
		}
		
	}

	if (!(msgM->buttonStateFlags & MouseStateFlags::MSF_SCROLLWHEEL_CHANGE))
		return TRUE;

	TigMsgMouse msgCopy = *msgM;
	msgCopy.buttonStateFlags = MouseStateFlags::MSF_SCROLLWHEEL_CHANGE;

	if ((int)msgM->x >= featsMainWnd.x + 3 && (int)msgM->x <= featsMainWnd.x + 188
		&& (int)msgM->y >= featsMainWnd.y +36 && (int)msgM->y <= featsMainWnd.y + 263) {
		ui.WidgetCopy(featsScrollbarId, &featsScrollbar);
		if (featsScrollbar.handleMessage)
			return featsScrollbar.handleMessage(featsScrollbarId, (TigMsg*)&msgCopy);
	}

	if ((int)msgM->x >= featsMainWnd.x + 207 && (int)msgM->x <= featsMainWnd.x + 392
		&& (int)msgM->y >= featsMainWnd.y + 118 && (int)msgM->y <= featsMainWnd.y + 263) {
		ui.WidgetCopy(featsExistingScrollbarId, &featsExistingScrollbar);
		if (featsExistingScrollbar.handleMessage)
			return featsExistingScrollbar.handleMessage(featsExistingScrollbarId, (TigMsg*)&msgCopy);
	}

	return FALSE;
}

BOOL UiCharEditor::FeatsEntryBtnMsg(int widId, TigMsg * msg){

	if (msg->type != TigMsgType::WIDGET)
		return 0;
	auto msgW = (TigMsgWidget*)msg;

	auto widIdx = ui.WidgetlistIndexof(widId, &featsAvailBtnIds[0], FEATS_AVAIL_BTN_COUNT);
	auto featIdx = widIdx + featsScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mSelectableFeats.size())
		return FALSE;

	auto featInfo = mSelectableFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;

	auto &selPkt = GetCharEditorSelPacket();
	auto btn = ui.GetButton(widId);

	switch (msgW->widgetEventType){
	case TigMsgWidgetEvent::Clicked:
		if (!FeatAlreadyPicked(feat) && FeatCanPick(feat)){
			auto origX = msgW->x - btn->x, origY = msgW->y - btn->y;
			auto style = uiCharEditor.GetFeatStyle(feat);
			auto featCallback = [origX, origY, feat, style](int x, int y) {
				std::string text(uiCharEditor.GetFeatName(feat));
				UiRenderer::PushFont(PredefinedFont::PRIORY_12);
				TigRect rect(x - origX, y - origY, 180, uiCharEditor.FEATS_AVAIL_BTN_HEIGHT);
				tigFont.Draw(text.c_str(), rect, style);
				UiRenderer::PopFont();
			};
			mouseFuncs.SetCursorDrawCallback(featCallback, (uint32_t)&featCallback);

		}
		return TRUE;
	case TigMsgWidgetEvent::MouseReleased:
		if (helpSys.IsClickForHelpActive()){
			mouseFuncs.SetCursorDrawCallback(nullptr, 0);
			helpSys.PresentWikiHelp(109 + feat);
			return TRUE;
		}
		case TigMsgWidgetEvent::MouseReleasedAtDifferentButton:
			if (FeatAlreadyPicked(feat) || !FeatCanPick(feat))
				return TRUE;
			mouseFuncs.SetCursorDrawCallback(nullptr, 0);
			
			// check if inserted into the normal slot
			if (featsSelectedBorderRect.ContainsPoint(msgW->x, msgW->y) && IsSelectingNormalFeat()){
				selPkt.feat0 = feat;
				if (feats.IsFeatMultiSelectMaster(feat))
					FeatsMultiSelectActivate(feat);
			}
			// check if inserted into the bonus slot
			else if (IsSelectingBonusFeat()  			
				&& featsClassBonusBorderRect.ContainsPoint(msgW->x, msgW->y) && IsClassBonusFeat(feat)){
				selPkt.feat2 = feat;
				if (feats.IsFeatMultiSelectMaster(feat))
					FeatsMultiSelectActivate(feat);
				else if (feat == FEAT_SKILL_MASTERY){
					auto skillMasteryActivate = temple::GetRef<void(__cdecl)(objHndl, int(__cdecl*)(int))>(0x1016C2B0);
					skillMasteryActivate(GetEditedChar(), temple::GetRef<int(__cdecl)(int)>(0x101A86D0));
				}
			}
			FeatsSanitize();
			return TRUE;
		case TigMsgWidgetEvent::Entered:
			temple::GetRef<void(int, char*, size_t)>(0x10162A10)(FeatsMultiGetFirst(feat), temple::GetRef<char[1024]>(0x10C76B48), 1024u); // UiTooltipSetForFeat
			temple::GetRef<void(char*)>(0x10162C00)(temple::GetRef<char[1024]>(0x10C76B48)); // UiCharTextboxSet
			return TRUE;
		case TigMsgWidgetEvent::Exited:
			temple::GetRef<void(__cdecl)(char *)>(0x10162C00)(""); // UiCharTextboxSet
			return TRUE;
		default:
			return FALSE;
				
		}
	return TRUE;
}


void UiCharEditor::FeatsEntryBtnRender(int widId){

	auto widIdx = ui.WidgetlistIndexof(widId, &featsAvailBtnIds[0], FEATS_AVAIL_BTN_COUNT);
	auto featIdx = widIdx + featsScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mSelectableFeats.size())
		return;

	auto featInfo = mSelectableFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;
	
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);

	UiRenderer::DrawTextInWidget(featsMainWndId, GetFeatName(feat), featsBtnRects[widIdx], GetFeatStyle(feat, false));

	UiRenderer::PopFont();
}

BOOL UiCharEditor::FeatsExistingBtnMsg(int widId, TigMsg* msg)
{
	if (msg->type != TigMsgType::WIDGET)
		return 0;
	auto msgW = (TigMsgWidget*)msg;

	auto widIdx = ui.WidgetlistIndexof(widId, &featsExistingBtnIds[0], FEATS_EXISTING_BTN_COUNT);
	auto featIdx = widIdx + featsExistingScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mExistingFeats.size())
		return FALSE;

	auto featInfo = mExistingFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;

	auto &selPkt = GetCharEditorSelPacket();
	auto btn = ui.GetButton(widId);

	switch (msgW->widgetEventType) {
	case TigMsgWidgetEvent::Entered:
		temple::GetRef<void(int, char*, size_t)>(0x10162A10)(FeatsMultiGetFirst(feat), temple::GetRef<char[1024]>(0x10C76B48), 1024u); // UiTooltipSetForFeat
		temple::GetRef<void(char*)>(0x10162C00)(temple::GetRef<char[1024]>(0x10C76B48)); // UiCharTextboxSet
		return TRUE;
	case TigMsgWidgetEvent::Exited:
		temple::GetRef<void(__cdecl)(char *)>(0x10162C00)(""); // UiCharTextboxSet
		return TRUE;
	default:
		return FALSE;

	}
	return TRUE;
}

void UiCharEditor::FeatsExistingBtnRender(int widId){
	auto widIdx = ui.WidgetlistIndexof(widId, &featsExistingBtnIds[0], FEATS_EXISTING_BTN_COUNT);
	auto featIdx = widIdx + featsExistingScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mExistingFeats.size())
		return;

	auto featInfo = mExistingFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);

	UiRenderer::DrawTextInWidget(featsMainWndId, GetFeatName(feat), featsExistingBtnRects[widIdx], featsClassStyle);

	UiRenderer::PopFont();
}


void UiCharEditor::FeatsMultiSelectActivate(feat_enums feat) {
	
	if (!FeatCanPick(feat))
		return;

	auto &selPkt = GetCharEditorSelPacket();
	if (feat == FEAT_WEAPON_FINESSE) {
		if (selPkt.feat0 == FEAT_WEAPON_FINESSE)
			selPkt.feat0 = FEAT_WEAPON_FINESSE_DAGGER;
		if (selPkt.feat1 == FEAT_WEAPON_FINESSE)
			selPkt.feat1 = FEAT_WEAPON_FINESSE_DAGGER;
		if (selPkt.feat2 == FEAT_WEAPON_FINESSE)
			selPkt.feat2 = FEAT_WEAPON_FINESSE_DAGGER;
		return;
	}

	mFeatsMultiMasterFeat = feat;
	featsMultiSelected = FEAT_NONE;

	// populate list
	mMultiSelectFeats.clear();

	auto featIt = FEAT_ACROBATIC;
	auto featProp = 0x100;
	switch(feat){
		case FEAT_EXOTIC_WEAPON_PROFICIENCY:
			featProp = FPF_EXOTIC_WEAP_ITEM;
			break;
		case FEAT_IMPROVED_CRITICAL:
			featProp = FPF_IMPR_CRIT_ITEM;
			break;
		case FEAT_MARTIAL_WEAPON_PROFICIENCY:
			featProp = FPF_MARTIAL_WEAP_ITEM;
			break;
		case FEAT_SKILL_FOCUS:
			featProp = FPF_SKILL_FOCUS_ITEM;
			break;
		case FEAT_WEAPON_FINESSE:
			featProp = FPF_WEAP_FINESSE_ITEM;
			break;
		case FEAT_WEAPON_FOCUS:
			featProp = FPF_WEAP_FOCUS_ITEM;
			break;
		case FEAT_WEAPON_SPECIALIZATION:
			featProp = FPF_WEAP_SPEC_ITEM;
			break;
		case FEAT_GREATER_WEAPON_FOCUS:
			featProp = FPF_GREATER_WEAP_FOCUS_ITEM;
			break;
		case FEAT_GREATER_WEAPON_SPECIALIZATION:
			featProp = FPF_GREAT_WEAP_SPEC_ITEM;
			break;
		default:
			break;
	}

	for (auto ft = 0; ft < NUM_FEATS; ft++) {
		featIt = (feat_enums)ft;
		if (feats.IsFeatPropertySet(featIt, featProp) && feats.IsFeatEnabled(featIt)) {
			mMultiSelectFeats.push_back(FeatInfo(ft));
		}
	}

	ui.WidgetCopy(featsMultiSelectScrollbarId, &featsMultiSelectScrollbar);
	featsMultiSelectScrollbar.scrollbarY = 0;
	featsMultiSelectScrollbarY = 0;
	featsMultiSelectScrollbar.yMax = max(0, (int)mMultiSelectFeats.size() - FEATS_MULTI_BTN_COUNT);
	ui.WidgetSet(featsMultiSelectScrollbarId, &featsMultiSelectScrollbar);
	ui.ButtonSetButtonState(featsMultiOkBtnId, UBS_DISABLED);

	ui.WidgetSetHidden(featsMultiSelectWndId, 0);
	ui.WidgetBringToFront(featsMultiSelectWndId);
}

void UiCharEditor::FeatsMultiSelectWndRender(int widId){
	featsbackdrop->SetX(featsMultiSelectWnd.x + featsMultiCenterX);
	featsbackdrop->SetY(featsMultiSelectWnd.y + featsMultiCenterY);
	featsbackdrop->Render();

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);

	UiRenderer::DrawTextInWidget(widId, GetFeatName(mFeatsMultiMasterFeat), featMultiTitleRect, featsCenteredStyle);

	UiRenderer::PopFont();

}

BOOL UiCharEditor::FeatsMultiSelectWndMsg(int widId, TigMsg * msg){
	if (msg->type != TigMsgType::WIDGET && msg->type != TigMsgType::KEYSTATECHANGE)
		return FALSE;
	
	ui.ScrollbarGetY(featsMultiSelectScrollbarId, &featsMultiSelectScrollbarY);
	
	return TRUE;
}

void UiCharEditor::FeatsMultiOkBtnRender(int widId){
	UiButtonState buttonState;
	if (ui.GetButtonState(widId, buttonState))
		return;

	int texId;
	switch(buttonState){
	case UBS_NORMAL:
		ui.GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptNormal, texId);
		break;
	case UBS_HOVERED:
		ui.GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptHover, texId);
		break;
	case UBS_DOWN:
		ui.GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptPressed, texId);
		break;
	case UBS_DISABLED:
		ui.GetAsset(UiAssetType::Generic, UiGenericAsset::DisabledNormal, texId);
		break;
	default:	
		break;
	}
	
	static TigRect srcRect(1,1,110,22);
	UiRenderer::DrawTextureInWidget(featsMultiSelectWndId, texId, featMultiOkRect, srcRect);

	
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	UiRenderer::DrawTextInWidget(featsMultiSelectWndId, combatSys.GetCombatMesLine(6009), featMultiOkTextRect, featsCenteredStyle);
	UiRenderer::PopFont();

}

BOOL UiCharEditor::FeatsMultiOkBtnMsg(int widId, TigMsg * msg){

	if (msg->type != TigMsgType::WIDGET)
		return FALSE;
	auto msgW = (TigMsgWidget*)msg;
	if (msgW->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return FALSE;

	auto &selPkt = GetCharEditorSelPacket();

	if (featsMultiSelected == FEAT_NONE)	{
		if (selPkt.feat0 == mFeatsMultiMasterFeat) {
			selPkt.feat0 = FEAT_NONE;
		}
		if (selPkt.feat1 == mFeatsMultiMasterFeat) {
			selPkt.feat1 = FEAT_NONE;
		}
		if (selPkt.feat2 == mFeatsMultiMasterFeat) {
			selPkt.feat2 = FEAT_NONE;
		}
	}
	else
	{
		if (selPkt.feat2 == mFeatsMultiMasterFeat) {
			selPkt.feat2 = featsMultiSelected;
		}
		else if (selPkt.feat0 == mFeatsMultiMasterFeat) {
			selPkt.feat0 = featsMultiSelected;
		}
		else if (selPkt.feat1 == mFeatsMultiMasterFeat) {
			selPkt.feat1 = featsMultiSelected;
		}
	}
	
	mFeatsMultiMasterFeat = FEAT_NONE;
	featsMultiSelected = FEAT_NONE;
	ui.WidgetSetHidden(featsMultiSelectWndId, 1);

	return TRUE;
}

void UiCharEditor::FeatsMultiCancelBtnRender(int widId){
	UiButtonState buttonState;
	if (ui.GetButtonState(widId, buttonState))
		return;

	int texId;
	switch (buttonState) {
	case UBS_NORMAL:
		ui.GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineNormal, texId);
		break;
	case UBS_HOVERED:
		ui.GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineHover, texId);
		break;
	case UBS_DOWN:
		ui.GetAsset(UiAssetType::Generic, UiGenericAsset::DeclinePressed, texId);
		break;
	case UBS_DISABLED:
		ui.GetAsset(UiAssetType::Generic, UiGenericAsset::DisabledNormal, texId);
		break;
	default:
		break;
	}

	static TigRect srcRect(1, 1, 110, 22);
	UiRenderer::DrawTextureInWidget(featsMultiSelectWndId, texId, featMultiCancelRect, srcRect);


	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	UiRenderer::DrawTextInWidget(featsMultiSelectWndId, combatSys.GetCombatMesLine(6010), featMultiCancelTextRect, featsCenteredStyle);
	UiRenderer::PopFont();
}

BOOL UiCharEditor::FeatsMultiCancelBtnMsg(int widId, TigMsg * msg){
	if (msg->type != TigMsgType::WIDGET)
		return FALSE;
	auto msgW = (TigMsgWidget*)msg;
	if (msgW->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return FALSE;

	auto &selPkt = GetCharEditorSelPacket();
	
	if (selPkt.feat0 == mFeatsMultiMasterFeat){
		selPkt.feat0 = FEAT_NONE;
	}
	if (selPkt.feat1 == mFeatsMultiMasterFeat) {
		selPkt.feat1 = FEAT_NONE;
	}
	if (selPkt.feat2 == mFeatsMultiMasterFeat) {
		selPkt.feat2 = FEAT_NONE;
	}

	mFeatsMultiMasterFeat = FEAT_NONE;
	featsMultiSelected = FEAT_NONE;
	ui.WidgetSetHidden(featsMultiSelectWndId, 1);

	return TRUE;
}

void UiCharEditor::FeatsMultiBtnRender(int widId){
	auto widIdx = ui.WidgetlistIndexof(widId, &featsMultiSelectBtnIds[0], FEATS_MULTI_BTN_COUNT);
	auto featIdx = widIdx + featsMultiSelectScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mMultiSelectFeats.size())
		return;

	auto featInfo = mMultiSelectFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;



	auto getFeatShortName = [](feat_enums ft){
		if (feats.IsFeatMultiSelectMaster(ft))
			return uiCharEditor.GetFeatName(ft);
		

		auto mesKey = 50000 + ft;
		
		if (feats.IsFeatPropertySet(ft, FPF_GREAT_WEAP_SPEC_ITEM)){
			mesKey = 50000 + (ft - FEAT_GREATER_WEAPON_SPECIALIZATION_GAUNTLET + FEAT_WEAPON_SPECIALIZATION_GAUNTLET);
		}

		MesLine line(mesKey);
		auto pcCreationMes = temple::GetRef<MesHandle>(0x11E72EF0);
		auto text = mesFuncs.GetLineById(pcCreationMes,mesKey);
		if (!text)
			text = uiCharEditor.GetFeatName(ft).c_str();
		return std::string(text);
	};

	auto ftName = getFeatShortName(feat);

	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	UiRenderer::DrawTextInWidget(featsMultiSelectWndId, ftName, featsMultiBtnRects[widIdx], GetFeatStyle(feat, false));

	UiRenderer::PopFont();
}

BOOL UiCharEditor::FeatsMultiBtnMsg(int widId, TigMsg* msg){

	if (msg->type == TigMsgType::MOUSE)
		return TRUE;

	if (msg->type != TigMsgType::WIDGET)
		return FALSE;


	auto msgW = (TigMsgWidget*)msg;

	auto widIdx = ui.WidgetlistIndexof(widId, &featsMultiSelectBtnIds[0], FEATS_MULTI_BTN_COUNT);
	auto featIdx = widIdx + featsMultiSelectScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mMultiSelectFeats.size())
		return FALSE;

	auto featInfo = mMultiSelectFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;

	auto &selPkt = GetCharEditorSelPacket();
	auto btn = ui.GetButton(widId);

	switch (msgW->widgetEventType) {
	case TigMsgWidgetEvent::MouseReleased:
		if (helpSys.IsClickForHelpActive()) {
			helpSys.PresentWikiHelp(109 + feat);
			return TRUE;
		}
		if (FeatCanPick(feat)){
			featsMultiSelected = feat;
			ui.ButtonSetButtonState(featsMultiOkBtnId, UBS_NORMAL);
		} else
		{
			featsMultiSelected = FEAT_NONE;
			ui.ButtonSetButtonState(featsMultiOkBtnId, UBS_DISABLED);
		}
		return TRUE;
	default:
		return FALSE;

	}
	
	return FALSE;
}



bool UiCharEditor::IsSelectingNormalFeat() {
	auto handle = GetEditedChar();
	auto newLvl = GetNewLvl();
	return (newLvl % 3) == 0;
}

bool UiCharEditor::IsSelectingBonusFeat() {
	return mIsSelectingBonusFeat;
}


std::string UiCharEditor::GetFeatName(feat_enums feat) {

	if (feat >= FEAT_EXOTIC_WEAPON_PROFICIENCY && feat <= FEAT_GREATER_WEAPON_FOCUS)
		return featsMasterFeatStrings[feat];

	return std::string(feats.GetFeatName(feat));

}

TigTextStyle & UiCharEditor::GetFeatStyle(feat_enums feat, bool allowMultiple) {
	auto &selPkt = GetCharEditorSelPacket();
	auto newLvl = uiCharEditor.GetNewLvl(selPkt.classCode);

	if ((allowMultiple || !uiCharEditor.FeatAlreadyPicked(feat))
		&& uiCharEditor.FeatCanPick(feat))
	{
		if (uiCharEditor.featsMultiSelected == feat) {
			return uiCharEditor.blueTextStyle;
		}
		if (feats.IsClassFeat(feat)) { // class Specific feat
			return uiCharEditor.featsClassStyle;
		}
		else if (uiCharEditor.IsClassBonusFeat(feat)) // is choosing class bonus right now
		{
			return uiCharEditor.featsGoldenStyle;
		}
		else
			return uiCharEditor.featsBonusTextStyle;
	}

	return uiCharEditor.featsGreyedStyle;

}

bool UiCharEditor::FeatAlreadyPicked(feat_enums feat) {
	if (feats.IsFeatPropertySet(feat, 0x1)  // can be gained multiple times
		|| feats.IsFeatMultiSelectMaster(feat))
		return false;
	auto &selPkt = GetCharEditorSelPacket();
	if (selPkt.feat0 == feat || selPkt.feat1 == feat || selPkt.feat2 == feat)
		return true;

	auto handle = GetEditedChar();

	auto isRangerSpecial = IsSelectingRangerSpec();
	return feats.HasFeatCountByClass(handle, feat, selPkt.classCode, isRangerSpecial ? selPkt.feat2 : FEAT_ACROBATIC) != 0;
}

bool UiCharEditor::FeatCanPick(feat_enums feat) {
	std::vector<feat_enums> featsPicked;
	auto &selPkt = GetCharEditorSelPacket();
	auto handle = GetEditedChar();

	if (selPkt.feat0 != FEAT_NONE) {
		featsPicked.push_back(selPkt.feat0);
	}
	if (selPkt.feat1 != FEAT_NONE) {
		featsPicked.push_back(selPkt.feat1);
	}
	if (selPkt.feat2 != FEAT_NONE) {
		featsPicked.push_back(selPkt.feat2);
	}

	// TODO extend the specials
	if (feat == FEAT_IMPROVED_TRIP || feat == FEAT_IMPROVED_DISARM) {
		if (selPkt.classCode == stat_level_monk && GetNewLvl(stat_level_monk) == 6)
			return true;
	}

	if (IsSelectingBonusFeat() && IsClassBonusFeat(feat) && feats.IsFeatPropertySet(feat, FPF_ROGUE_BONUS)){
		return true;
	}


	if (!feats.IsFeatMultiSelectMaster(feat)) {
		return feats.FeatPrereqsCheck(handle, feat, featsPicked.size() > 0 ? &featsPicked[0] : nullptr, featsPicked.size(), selPkt.classCode, selPkt.statBeingRaised);
	}


	// Multiselect Master feats

	auto ftrLvl = objects.StatLevelGet(handle, stat_level_fighter);
	if (selPkt.classCode == stat_level_fighter)
		ftrLvl++;
	bool hasFocus = false;
	switch (feat) {
	case FEAT_EXOTIC_WEAPON_PROFICIENCY:
		return critterSys.GetBaseAttackBonus(handle, selPkt.classCode) >= 1;
	case FEAT_IMPROVED_CRITICAL:
		return critterSys.GetBaseAttackBonus(handle, selPkt.classCode) >= 8;

	case FEAT_MARTIAL_WEAPON_PROFICIENCY:
	case FEAT_SKILL_FOCUS:
		return true;

	case FEAT_WEAPON_FINESSE:
		if (critterSys.GetBaseAttackBonus(handle, selPkt.classCode) < 1)
			return false;
		for (auto i = (int)FEAT_WEAPON_FINESSE_GAUNTLET; i <= FEAT_WEAPON_FINESSE_NET; i++) {
			if (feats.HasFeatCountByClass(handle, (feat_enums)i, (Stat)0, 0))
				return false;
		}
		for (auto it : featsPicked) {
			if (feats.IsFeatPropertySet(it, FPF_WEAP_FINESSE_ITEM))
				return false;
		}
		return true;

	case FEAT_WEAPON_FOCUS:
		return critterSys.GetBaseAttackBonus(handle, selPkt.classCode) >= 1;

	case FEAT_WEAPON_SPECIALIZATION:

		if (ftrLvl < 4)
			return false;
		// check if has weapon focus

		for (auto i = (int)FEAT_WEAPON_FOCUS_GAUNTLET; i <= FEAT_WEAPON_FOCUS_RAY; i++) {
			if (feats.HasFeatCountByClass(handle, (feat_enums)i, (Stat)0, 0)) {
				return true;
			}
			// if not, check if it's one of the picked ones
			for (auto it : featsPicked) {
				if (it == (feat_enums)i)
					return true;
			}
		}
		return false;

	case FEAT_GREATER_WEAPON_FOCUS:
		return ftrLvl >= 8;

	case FEAT_GREATER_WEAPON_SPECIALIZATION:
		if (ftrLvl < 12)
			return false;

		for (auto i = (int)FEAT_GREATER_WEAPON_FOCUS_GAUNTLET; i <= FEAT_GREATER_WEAPON_FOCUS_RAY; i++) {
			hasFocus = false;
			if (feats.HasFeatCountByClass(handle, (feat_enums)i, (Stat)0, 0)) {
				hasFocus = true;
			}
			// if not, check if it's one of the picked ones
			for (auto it : featsPicked) {
				if (it == (feat_enums)i)
					hasFocus =  true;
				break;
			}
			// if has Greater Weapon Focus, check for Weapon Specialization
			if (hasFocus) {
				
				for (auto j = (int)FEAT_WEAPON_SPECIALIZATION_GAUNTLET; j <= FEAT_WEAPON_SPECIALIZATION_GRAPPLE; j++) {
					if (feats.HasFeatCountByClass(handle, (feat_enums)j, (Stat)0, 0))
						return true;
				}
			}
		}

	default:
		return true;
	}


}

bool UiCharEditor::IsSelectingRangerSpec()
{
	auto &selPkt = GetCharEditorSelPacket();
	auto handle = GetEditedChar();
	auto isRangerSpecial = selPkt.classCode == stat_level_ranger && (objects.StatLevelGet(handle, stat_level_ranger) + 1) == 2;
	return isRangerSpecial;
}

bool UiCharEditor::IsClassBonusFeat(feat_enums feat) {
	// mBonusFeats is delivered via the python class API
	for (auto it : mBonusFeats) {
		if (it.featEnum == feat)
			return true;
	}

	// the old stuff
	auto &selPkt = GetCharEditorSelPacket();
	auto newLvl = GetNewLvl(selPkt.classCode);


	switch (selPkt.classCode) {
	case stat_level_fighter:
		return feats.IsFighterFeat(feat);
	case stat_level_monk:
		if (feats.IsFeatPropertySet(feat, 0x20) && newLvl == 1)
			return true;
		if (feats.IsFeatPropertySet(feat, 0x40) && newLvl == 2)
			return true;
		if (feats.IsFeatPropertySet(feat, 0x80) && newLvl == 6)
			return true;
		return false;
	case stat_level_ranger:
		return (newLvl == 2 && (feat == FEAT_RANGER_TWO_WEAPON_STYLE || feat == FEAT_RANGER_ARCHERY_STYLE));
	case stat_level_rogue:
		return (feat < FEAT_NONE &&  newLvl >= 10 && !( (newLvl-10) % 3));
	case stat_level_wizard:
		return feats.IsMagicFeat(feat);
	default:
		return false;
	}
}

void UiCharEditor::SetBonusFeats(std::vector<FeatInfo>& fti) {
	mBonusFeats.clear();
	for (auto it : fti) {
		uiCharEditor.mBonusFeats.push_back(it);
	}
}

void UiCharEditor::FeatsSanitize() {
	auto &selPkt = GetCharEditorSelPacket();

	for (auto i = 0; i < 3; i++) { // check if any of the feat now lack the prereq (due to user removal). loop three times to ensure up-to-date state.
		if (selPkt.feat0 != FEAT_NONE && !FeatCanPick(selPkt.feat0))
			selPkt.feat0 = FEAT_NONE;
		if (selPkt.feat1 != FEAT_NONE && !FeatCanPick(selPkt.feat1)) {
			selPkt.feat1 = FEAT_NONE;
		}
		if (selPkt.feat2 != FEAT_NONE && !FeatCanPick(selPkt.feat2) && !IsSelectingRangerSpec())
			selPkt.feat2 = FEAT_NONE;
	}


}

feat_enums UiCharEditor::FeatsMultiGetFirst(feat_enums feat) {
	switch (feat)
	{
	case FEAT_EXOTIC_WEAPON_PROFICIENCY:
		return FEAT_EXOTIC_WEAPON_PROFICIENCY_BASTARD_SWORD;

	case FEAT_IMPROVED_CRITICAL:
		return FEAT_IMPROVED_CRITICAL_BASTARD_SWORD;

	case FEAT_MARTIAL_WEAPON_PROFICIENCY:
		return FEAT_MARTIAL_WEAPON_PROFICIENCY_BATTLEAXE;

	case FEAT_SKILL_FOCUS:
		return FEAT_SKILL_FOCUS_APPRAISE;

	case FEAT_WEAPON_FINESSE:
		return FEAT_WEAPON_FINESSE_BASTARD_SWORD;

	case FEAT_WEAPON_FOCUS:
		return FEAT_WEAPON_FOCUS_BASTARD_SWORD;

	case FEAT_GREATER_WEAPON_FOCUS:
		return FEAT_GREATER_WEAPON_FOCUS_BASTARD_SWORD;

	case FEAT_WEAPON_SPECIALIZATION:
		return FEAT_WEAPON_SPECIALIZATION_BASTARD_SWORD;

	case FEAT_GREATER_WEAPON_SPECIALIZATION:
		return FEAT_GREATER_WEAPON_SPECIALIZATION_BASTARD_SWORD;
	default:
		return feat;
	}
}


#pragma endregion

#pragma region Spells

void UiCharEditor::SpellsWndRender(int widId){
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	UiRenderer::DrawTextInWidget(widId, spellsAvailLabel, spellsAvailTitleRect, spellsTitleStyle);
	UiRenderer::DrawTextInWidget(widId, spellsChosenLabel, spellsChosenTitleRect, spellsTitleStyle);
	

	levelupSpellbar->SetX(spellsWnd.x);
	levelupSpellbar->SetY(spellsWnd.y + 275);
	levelupSpellbar->Render();

	
	// RenderSpellsPerDay
	UiRenderer::DrawTextInWidget(widId, spellsPerDayLabel, spellsPerDayTitleRect, spellsTextStyle);
	for (auto i = 0; i < SPELLS_PER_DAY_BOXES_COUNT; i++){
		UiRenderer::DrawTextInWidget(widId, spellsPerDayTexts[i], spellsPerDayTextRects[i], spellsPerDayStyle);
	}

	UiRenderer::PopFont();

	// Rects
	RenderHooks::RenderRectInt(spellsWnd.x , spellsWnd.y + 38, 195, 227, 0xFF5D5D5D);
	RenderHooks::RenderRectInt(spellsWnd.x + 201, spellsWnd.y + 38, 195, 227, 0xFF5D5D5D);

	StateTitleRender(widId);
}

BOOL UiCharEditor::SpellsWndMsg(int widId, TigMsg * msg){
	
	if (msg->type == TigMsgType::WIDGET){
		auto msgW = (TigMsgWidget*)msg;
		if (msgW->widgetEventType == TigMsgWidgetEvent::Scrolled){
			ui.ScrollbarGetY(spellsScrollbarId, &spellsScrollbarY);
			ui.ScrollbarGetY(spellsScrollbar2Id, &spellsScrollbar2Y);
			SpellsPerDayUpdate();
			return 1;
		}
		return 0;
	}

	if (msg->type == TigMsgType::MOUSE){
		auto msgM = (TigMsgMouse*)msg;
		if ((msgM->buttonStateFlags & MouseStateFlags::MSF_LMB_RELEASED) && helpSys.IsClickForHelpActive()){
			// LMB handler - present help for spell
			for (auto i = 0; i < SPELLS_BTN_COUNT; i++){
				// check if mouse within button
				if (!ui.WidgetContainsPoint(spellsChosenBtnIds[i], msgM->x, msgM->y))
					continue;
				
				auto spellIdx = i + spellsScrollbar2Y;
				if ((uint32_t)spellIdx >= mSpellInfo.size())
					break;
				
				auto spEnum = mSpellInfo[spellIdx].spEnum;
				// ensure is not label
				if (spellSys.IsLabel(spEnum))
					break;
				
				helpSys.PresentWikiHelp(860 + spEnum);
				return 1;
			}
		}
		if (msgM->buttonStateFlags & MouseStateFlags::MSF_RMB_RELEASED) {
			// RMB handler - add to known spells
			for (auto i = 0; i < SPELLS_BTN_COUNT; i++) {
				// get spell btn
				if (!ui.WidgetContainsPoint(spellsAvailBtnIds[i], msgM->x, msgM->y))
					continue;
				auto spellAvailIdx = i + spellsScrollbarY;
				if ((uint32_t)spellAvailIdx >= mAvailableSpells.size())
					break;

				// got the avail btn, now search for suitable vacant slot
				auto spEnum = mAvailableSpells[spellAvailIdx].spEnum;
				auto spClass = mAvailableSpells[spellAvailIdx].spellClass;
				auto spLevel = mAvailableSpells[spellAvailIdx].spellLevel;

				if (spellSys.IsLabel(spEnum))
					break;

				if (SpellIsAlreadyKnown(spEnum, spClass) || SpellIsForbidden(spEnum))
					break;

				auto curSpellLvl = -1;
				auto foundSlot = false;
				for (auto j = 0u; j < mSpellInfo.size(); j++){
					auto spInfo = mSpellInfo[j];
					if (spInfo.spellClass != spClass)
						continue;
					if (spellSys.IsLabel(spInfo.spEnum)){
						curSpellLvl = spInfo.spellLevel;
						if (curSpellLvl > spLevel)
							break;
						continue;
					}

					if (spInfo.spEnum != SPELL_ENUM_VACANT)
						continue;

					// ensure spell slot is of correct level
					if (spInfo.spellLevel == -1 // for "wildcard" empty slots (e.g. Wizard)
						|| curSpellLvl == spLevel   ){
							mSpellInfo[j].spEnum = spEnum; // spell level might still be -1 so be careful when adding to spellbook later on!
							break;
					}
				}
			}
		}
		if (!(msgM->buttonStateFlags & MouseStateFlags::MSF_SCROLLWHEEL_CHANGE))
			return 1;

		TigMsgMouse msgCopy = *msgM;
		msgCopy.buttonStateFlags = MouseStateFlags::MSF_SCROLLWHEEL_CHANGE;

		if ((int)msgM->x >= spellsWnd.x + 4 && (int)msgM->x <= spellsWnd.x + 184
			&& (int)msgM->y >= spellsWnd.y && (int)msgM->y <= spellsWnd.y + 259){
			ui.WidgetCopy(spellsScrollbarId, &spellsScrollbar);
			if (spellsScrollbar.handleMessage)
				return spellsScrollbar.handleMessage(spellsScrollbarId, (TigMsg*)&msgCopy);
		}

		if ((int)msgM->x >= spellsWnd.x +206 && (int)msgM->x <= spellsWnd.x + 376
			&& (int)msgM->y >= spellsWnd.y && (int)msgM->y <= spellsWnd.y + 259) {
			ui.WidgetCopy(spellsScrollbar2Id, &spellsScrollbar2);
			if (spellsScrollbar2.handleMessage)
				return spellsScrollbar2.handleMessage(spellsScrollbar2Id, (TigMsg*)&msgCopy);
		}
		return 1;

	}

	return 0;
}

void UiCharEditor::SpellsPerDayUpdate(){
	UiRenderer::PushFont(PredefinedFont::ARIAL_BOLD_24);
	auto &selPkt = GetCharEditorSelPacket();

	spellsPerDayTexts.clear();
	for (auto i = 0; i < SPELLS_PER_DAY_BOXES_COUNT; i++){
		auto &handle = GetEditedChar();
		auto casterLvl = objects.StatLevelGet(handle, selPkt.classCode);
		auto numSpells = d20ClassSys.GetNumSpellsFromClass(handle, selPkt.classCode, i, casterLvl);
		if (numSpells < 0)
			numSpells = 0;
		std::string text(fmt::format("{}", numSpells));
		spellsPerDayTexts.push_back(text);

		auto textMeas = UiRenderer::MeasureTextSize(text, spellsPerDayStyle);
		spellsPerDayTextRects[i].x = spellsNewSpellsBoxRects[i].x +
			(spellsNewSpellsBoxRects[i].width - textMeas.width)/2;
		spellsPerDayTextRects[i].y = spellsNewSpellsBoxRects[i].y +
			(spellsNewSpellsBoxRects[i].height - textMeas.height) / 2;
		spellsPerDayTextRects[i].width = textMeas.width;
		spellsPerDayTextRects[i].height = textMeas.height;
	}
	UiRenderer::PopFont();
}

BOOL UiCharEditor::SpellsEntryBtnMsg(int widId, TigMsg * msg)
{
	if (msg->type != TigMsgType::WIDGET)
		return 0;
	return 0;
}

void UiCharEditor::SpellsEntryBtnRender(int widId)
{
	auto widIdx = ui.WidgetlistIndexof(widId, &spellsChosenBtnIds[0], SPELLS_BTN_COUNT);
	if (widIdx == -1)
		return;
	auto spellIdx = widIdx + spellsScrollbar2Y;
	if (spellIdx >= (int)mSpellInfo.size())
		return;

	auto spInfo = mSpellInfo[spellIdx];
	auto spFlag = spInfo.spFlag;
	auto spEnum = spInfo.spEnum;
	auto spLvl = spInfo.spellLevel;

	auto btn = ui.GetButton(widId);
	
	auto &selPkt = GetCharEditorSelPacket();
	if (spFlag && (!selPkt.spellEnumToRemove || spFlag != 1)){
		RenderHooks::RenderRectInt(btn->x + 11, btn->y, btn->width - 11, btn->height, 0xFF222C37);
	}

	std::string text;
	TigRect rect(btn->x - spellsWnd.x, btn->y - spellsWnd.y, btn->width, btn->height);
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	if (spEnum == SPELL_ENUM_VACANT){
		// don't draw text (will only draw the frame)
	}
	else if (spellSys.IsLabel(spEnum)) {
		if (spLvl >= 0 && spLvl < NUM_SPELL_LEVELS){
			text.append(fmt::format("{}", spellLevelLabels[spLvl]));
			UiRenderer::DrawTextInWidget(spellsWndId, text, rect, spellLevelLabelStyle);
		}
	}
	else
	{
		text.append(fmt::format("{}", spellSys.GetSpellMesline(spEnum)));
		rect.x += 11;
		//rect.width -= 11;
		UiRenderer::DrawTextInWidget(spellsWndId, text, rect, spellsTextStyle);
	}
	UiRenderer::PopFont();
}

BOOL UiCharEditor::SpellsAvailableEntryBtnMsg(int widId, TigMsg * msg)
{
	if (msg->type != TigMsgType::WIDGET)
		return 0;
	auto msgW = (TigMsgWidget*)msg;

	auto widIdx = ui.WidgetlistIndexof(widId, &spellsAvailBtnIds[0], SPELLS_BTN_COUNT);
	if (widIdx == -1)
		return 0;

	auto spellIdx = widIdx + spellsScrollbarY;
	if (spellIdx >= (int)mAvailableSpells.size())
		return 0;

	auto spInfo = mAvailableSpells[spellIdx];
	auto spFlag = spInfo.spFlag;
	auto spEnum = spInfo.spEnum;
	auto spLvl = spInfo.spellLevel;
	auto spClass = spInfo.spellClass;

	if (!spellSys.IsLabel(spEnum)){
		
		auto btn = ui.GetButton(widId);
		auto curSpellLvl = -1;
		auto &selPkt = GetCharEditorSelPacket();

		switch (msgW->widgetEventType){
			case TigMsgWidgetEvent::Clicked: // button down - initiate drag
				if (!SpellIsAlreadyKnown(spEnum, spClass ) && !SpellIsForbidden(spEnum)){
					auto origX = msgW->x - btn->x, origY = msgW->y - btn->y;
					auto spellCallback = [origX, origY, spEnum](int x, int y){
						std::string text(spellSys.GetSpellMesline(spEnum));
						UiRenderer::PushFont(PredefinedFont::PRIORY_12);
						TigRect rect(x - origX ,y - origY,180,uiCharEditor.SPELLS_BTN_HEIGHT);
						tigFont.Draw(text.c_str(), rect, uiCharEditor.spellsTextStyle);
						UiRenderer::PopFont();
					};
					mouseFuncs.SetCursorDrawCallback(spellCallback, (uint32_t)&spellCallback);
				}
				return 1;
			case TigMsgWidgetEvent::MouseReleased: 
				if (helpSys.IsClickForHelpActive()){
					mouseFuncs.SetCursorDrawCallback(nullptr, 0);
					helpSys.PresentWikiHelp(spEnum + 860);
					return 1;
				}
			case TigMsgWidgetEvent::MouseReleasedAtDifferentButton: 
				mouseFuncs.SetCursorDrawCallback(nullptr, 0);
				if (SpellIsAlreadyKnown(spEnum, spClass)
					|| SpellIsForbidden(spEnum))
					return 1;


				for (auto i = 0u; i < mSpellInfo.size(); i++){
					auto rhsSpInfo = mSpellInfo[i];

					// make sure the spell class is ok
					if (rhsSpInfo.spellClass != spClass)
						continue;

					// if encountered label - go on
					if (spellSys.IsLabel(rhsSpInfo.spEnum)){
						curSpellLvl = rhsSpInfo.spellLevel;
						continue;
					}

					// else - make sure is visible slot
					if ((int)i < spellsScrollbar2Y)
						continue;
					if ((int)i >= spellsScrollbar2Y + SPELLS_BTN_COUNT)
						break;

					auto chosenWidIdx = (int)i - spellsScrollbar2Y;
					if (!ui.WidgetContainsPoint(spellsChosenBtnIds[chosenWidIdx], msgW->x, msgW->y))
						continue;

					if (rhsSpInfo.spellLevel == -1 // wildcard slot
						|| rhsSpInfo.spellLevel == spLvl 
						   && rhsSpInfo.spFlag != 0
						   && (rhsSpInfo.spFlag != 1 || !selPkt.spellEnumToRemove)
						){
						
						if (rhsSpInfo.spFlag == 1){ // replaceable spell
							mSpellInfo[i].spFlag = 2;
							selPkt.spellEnumToRemove = rhsSpInfo.spEnum;
						} 
						else if (rhsSpInfo.spFlag == 2 && selPkt.spellEnumToRemove == spEnum){ // was already replaced, and now restoring
							mSpellInfo[i].spFlag = 1;
							selPkt.spellEnumToRemove = 0;
						}
						mSpellInfo[i].spEnum = spEnum;
						return 1;
					}

				}

				return 1;
			case TigMsgWidgetEvent::Entered: 
				temple::GetRef<void(int, char*, size_t)>(0x10162AB0)(spEnum, temple::GetRef<char[1024]>(0x10C732B0), 1024u); // UiTooltipSetForSpell
				temple::GetRef<void(char*)>(0x10162C00)(temple::GetRef<char[1024]>(0x10C732B0)); // UiCharTextboxSet
				return 1;
			case TigMsgWidgetEvent::Exited: 
				temple::GetRef<void(__cdecl)(char *)>(0x10162C00)(""); // UiCharTextboxSet
				return 1;
			default: 
				return 0;
		}
	}

	/*if (msgW->widgetEventType == TigMsgWidgetEvent::Entered){
		std::string text;
		text.append(fmt::format(""));
		auto helpTopicId = ElfHash::Hash(text);
		temple::GetRef<void(__cdecl)(uint32_t)>(0x)(helpTopicId);
		return 1;
	}*/

	if (msgW->widgetEventType == TigMsgWidgetEvent::Exited) {
		temple::GetRef<void(__cdecl)(char *)>(0x10162C00)(""); // UiCharTextboxSet
		return 1;
	}

	return 0;
}

void UiCharEditor::SpellsAvailableEntryBtnRender(int widId){

	auto widIdx = ui.WidgetlistIndexof(widId, &spellsAvailBtnIds[0], SPELLS_BTN_COUNT);
	if (widIdx == -1)
		return;
	auto spellIdx = widIdx + spellsScrollbarY;
	if (spellIdx >= (int)mAvailableSpells.size())
		return;

	auto btn = ui.GetButton(widId);
	auto spEnum = mAvailableSpells[spellIdx].spEnum;

	std::string text;
	TigRect rect(btn->x - spellsWnd.x, btn->y - spellsWnd.y, btn->width, btn->height);
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	if (spellSys.IsLabel(spEnum)){
		rect.x += 2;
		auto spLvl = mAvailableSpells[spellIdx].spellLevel;
		if (spLvl >= 0 && spLvl < NUM_SPELL_LEVELS)
		{
			text.append(fmt::format("{}", spellLevelLabels[spLvl]));
			UiRenderer::DrawTextInWidget(spellsWndId, text, rect, spellLevelLabelStyle);
		}
			
	} 
	else
	{
		text.append(fmt::format("{}", spellSys.GetSpellMesline(spEnum)));
		rect.x += 12;
		//rect.width -= 11;
		if (SpellIsAlreadyKnown(spEnum, mAvailableSpells[spellIdx].spellClass)
			|| SpellIsForbidden(spEnum))
			UiRenderer::DrawTextInWidget(spellsWndId, text, rect, spellsAvailBtnStyle);
		else
			UiRenderer::DrawTextInWidget(spellsWndId, text, rect, spellsTextStyle);
	}
	UiRenderer::PopFont();
	
}

bool UiCharEditor::SpellIsForbidden(int spEnum)
{
	auto &selPkt = GetCharEditorSelPacket();
	auto handle = GetEditedChar();
	SpellEntry spEntry(spEnum);
	auto spSchool = spEntry.spellSchoolEnum;

	if (spSchool == selPkt.forbiddenSchool1
		|| spSchool == selPkt.forbiddenSchool2)
		return true;
	if (spellSys.IsForbiddenSchool(handle, spSchool))
		return true;
	return false;
}

bool UiCharEditor::SpellIsAlreadyKnown(int spEnum, int spellClass) {
	for (auto i = 0u; i < mSpellInfo.size(); i++) {
		if (mSpellInfo[i].spEnum == spEnum
			&& mSpellInfo[i].spellClass == spellClass)
			return true;
	}

	auto &selPkt = GetCharEditorSelPacket();
	if (spellSys.IsSpellKnown(GetEditedChar(), spEnum, spellClass)) {
		if (selPkt.spellEnumToRemove == spEnum)
			return false; // Oh god... TODO! (need to record class too..)
		return true;
	}

	return false;
}


#pragma endregion

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

	if (!config.newClasses){
		ui.ButtonSetButtonState(classNextBtn, UBS_DISABLED);
		ui.ButtonSetButtonState(classPrevBtn, UBS_DISABLED);
		return;
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


int UiCharEditor::GetNewLvl(Stat classEnum){ // default is classEnum  = stat_level i.e. get the overall new level
	auto handle = GetEditedChar();
	return objects.StatLevelGet(handle, classEnum) + 1;
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
	for (auto i = 0u; i < spEntries.size(); /* do not increment here! */ ){
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
		static BOOL(__cdecl *orgFinishBtnMsg)(int, TigMsg*) = replaceFunction<BOOL(int, TigMsg*)>(0x10148FE0, [](int widId, TigMsg* msg){
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
		
		// feats
		replaceFunction<BOOL(GameSystemConf&)>(0x101AAB80, [](GameSystemConf &conf) {
			return uiCharEditor.FeatsSystemInit(conf);
		});

		replaceFunction<void()>(0x101A8CA0, []() {
			uiCharEditor.FeatsFree();
		});

		replaceFunction<BOOL(UiResizeArgs& args)>(0x101AAE50, [](UiResizeArgs& args) {
			return uiCharEditor.FeatsWidgetsResize(args);
		});

		replaceFunction<BOOL()>(0x101A7EB0, []() {
			return uiCharEditor.FeatsShow();
		});

		replaceFunction<BOOL()>(0x101A7E90, []() {
			return uiCharEditor.FeatsHide();
		});


		replaceFunction<void()>(0x101A8960, []() {
			uiCharEditor.FeatsActivate();
		});

		replaceFunction<BOOL()>(0x101AA140, []() {
			return uiCharEditor.FeatsCheckComplete();
		});

		/*replaceFunction<void(CharEditorSelectionPacket&, objHndl&)>(, [](CharEditorSelectionPacket& selPkt, objHndl& handle) {
			uiCharEditor.FeatsFinalize(); // there is no finalizer for feats
		});*/

		replaceFunction<void(CharEditorSelectionPacket &)>(0x101A7E30, [](CharEditorSelectionPacket &selPkt) {
			uiCharEditor.FeatsReset(selPkt);
		});


		// spells

		replaceFunction<BOOL(GameSystemConf&)>(0x101A7B10, [](GameSystemConf &conf) {
			return uiCharEditor.SpellsSystemInit(conf);
		});

		replaceFunction<void()>(0x101A6350, []() {
			uiCharEditor.SpellsFree();
		});

		replaceFunction<BOOL(UiResizeArgs& args)>(0x101A7E10, [](UiResizeArgs& args){
			uiCharEditor.SpellsWidgetsFree();
			return uiCharEditor.SpellsWidgetsInit();
		});

		replaceFunction<BOOL()>(0x101A5BE0, []() {
			return uiCharEditor.SpellsShow();
		});
		
		replaceFunction<BOOL()>(0x101A5BC0, []() {
			return uiCharEditor.SpellsHide();
		});
		

		replaceFunction<void()>(0x101A75F0, [](){
			uiCharEditor.SpellsActivate();
		});

		replaceFunction<BOOL()>(0x101A5C00, [](){
			return uiCharEditor.SpellsCheckComplete();
		});

		replaceFunction<void(CharEditorSelectionPacket&, objHndl&)>(0x101A5C50, [](CharEditorSelectionPacket& selPkt, objHndl& handle){
			uiCharEditor.SpellsFinalize();
		});

		replaceFunction<void(CharEditorSelectionPacket &)>(0x101A5B90, [](CharEditorSelectionPacket &selPkt){
			uiCharEditor.SpellsReset(selPkt);
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
