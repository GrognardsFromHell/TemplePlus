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

#include "ui_render.h"
#include <EASTL/hash_map.h>
#include <EASTL/fixed_string.h>
#include <gamesystems/d20/d20stats.h>
#include <gamesystems/gamesystems.h>
#include <gamesystems/objects/objsystem.h>
#include <d20_level.h>
#include <tig/tig_msg.h>
#include "graphics/imgfile.h"
#include "graphics/render_hooks.h"
#include <python/python_integration_class_spec.h>

#include <pybind11/embed.h>
#include <pybind11/cast.h>
#include <pybind11/stl.h>

#include <python/python_object.h>
#include <radialmenu.h>
#include <action_sequence.h>
#include <condition.h>
#include <gamesystems/d20/d20_help.h>
#include <infrastructure/elfhash.h>
#include <tig/tig_mouse.h>
#include <combat.h>
#include "ui_assets.h"
#include <infrastructure/keyboard.h>
#include "gamesystems/deity/legacydeitysystem.h"

namespace py = pybind11;

Chargen chargen;
temple::GlobalStruct<LegacyCharEditorSystem, 0x102FA6C8> lgcySystems;
class UiCharEditor {
	friend class UiCharEditorHooks;
public:

	objHndl GetEditedChar();
	CharEditorSelectionPacket & GetCharEditorSelPacket();

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
	bool IsSelectingFeats();
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
	void MainWndRender(int widId);


	void ClassBtnRender(int widId);
	BOOL ClassBtnMsg(int widId, TigMsg* msg);
	BOOL ClassNextBtnMsg(int widId, TigMsg* msg);
	BOOL ClassPrevBtnMsg(int widId, TigMsg* msg);
	BOOL FinishBtnMsg(int widId, TigMsg* msg); // goes after the original FinishBtnMsg
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
	int &GetStatesComplete();

	// logic
	void ClassSetPermissibles();
	bool ClassSanitize(); // re-check class requirements. Returns true if re-evaluation is required.
	bool IsSelectingNormalFeat(); // the normal feat you get every 3rd level in 3.5ed
	bool IsSelectingBonusFeat(); // selecting a class bonus feat

	// utilities
	int GetNewLvl(Stat classEnum = stat_level);
	
	
	std::string GetFeatName(feat_enums feat); // includes strings for Mutli-selection feat categories e.g. FEAT_WEAPON_FOCUS
	TigTextStyle & GetFeatStyle(feat_enums feat, bool allowMultiple = true);
	bool FeatAlreadyPicked(feat_enums feat);
	bool FeatCanPick(feat_enums feat);
	bool IsSelectingRangerSpec();
	bool IsClassBonusFeat(feat_enums feat);
	bool CanDropToNormalSlot(feat_enums feat);

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
	LgcyWindow featsMainWnd, featsMultiSelectWnd;
	LgcyScrollBar featsScrollbar, featsExistingScrollbar, featsMultiSelectScrollbar;
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
	bool selectingSpells = true;


	int spellsWndId = 0;
	LgcyWindow spellsWnd;
	int spellsScrollbarId = 0, spellsScrollbar2Id = 0;
	LgcyScrollBar spellsScrollbar, spellsScrollbar2;
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
	TigTextStyle featsGreyedStyle, featsNormalTextStyle, featsExistingTitleStyle, featsGoldenStyle, featsClassStyle, featsCenteredStyle;
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
	std::vector<FeatInfo> mExistingFeats, mSelectableFeats, mMultiSelectFeats, mMultiSelectMasterFeats; 
	//std::unique_ptr<CharEditorStatsSystem> mStats;
	//std::unique_ptr<CharEditorFeaturesSystem> mFeatures;
	//std::unique_ptr<CharEditorSkillsSystem> mSkills;
	//std::unique_ptr<CharEditorFeatsSystem> mFeats;
	//std::unique_ptr<CharEditorSpellsSystem> mSpells;
} uiCharEditor;


// Helper function for the python character editor
int hasFeat(int feat) {
	auto handle = chargen.GetEditedChar();
	auto &charPkt = chargen.GetCharEditorSelPacket();

	auto levelRaised = charPkt.classCode;
	if (chargen.IsNewChar())
		levelRaised = (Stat)0;

	uint32_t domain1 = Domain_None;
	uint32_t domain2 = Domain_None;
	uint32_t alignmentChoice = 0;

	if (!chargen.IsNewChar()) {
		if (levelRaised == stat_level_cleric) {
			domain1 = charPkt.domain1;
			domain2 = charPkt.domain2;
			alignmentChoice = charPkt.alignmentChoice;
		}
	}

	auto result = feats.HasFeatCountByClass(handle, static_cast<feat_enums>(feat), levelRaised, 0, domain1, domain2, alignmentChoice);

	if (feat == charPkt.feat0) {
		result++;
	}
	if (feat == charPkt.feat1) {
		result++;
	}
	if (feat == charPkt.feat2) {
		result++;
	}
	if (feat == charPkt.feat3) {
		result++;
	}

	return result;
}


template <> class py::detail::type_caster<objHndl> {
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



PYBIND11_EMBEDDED_MODULE(char_editor, mm) {
	
	mm.doc() = "Temple+ Char Editor, used for extending the ToEE character editor.";
	
	#pragma region KnownSpellInfo
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

			auto firstSpellLvl = spellSys.GetSpellLevelBySpellClass(first, spellSys.GetSpellClass(self.spellClass));
			auto secondSpellLvl = spellSys.GetSpellLevelBySpellClass(second, spellSys.GetSpellClass(other.spellClass));

			// Check for Advanced Learning Extended List Spells
			if (firstSpellLvl == -1) {
				auto castingClass = spellSys.GetCastingClass(self.spellClass);
				if (pythonClassIntegration.HasAdvancedLearning(castingClass)) {
					firstSpellLvl = pythonClassIntegration.GetAdvancedLearningSpellLevel(castingClass, first);
				}
			}

			if (secondSpellLvl == -1) {
				auto castingClass = spellSys.GetCastingClass(other.spellClass);
				if (pythonClassIntegration.HasAdvancedLearning(castingClass)) {
					secondSpellLvl = pythonClassIntegration.GetAdvancedLearningSpellLevel(castingClass, second);
				}
			}

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
			ksi.spellClass = spellSys.GetSpellClass(classEnum, false);
		}, py::arg("class_enum"))
		;

	#pragma endregion


	py::class_<FeatInfo>(mm, "FeatInfo")
		.def(py::init<int>(), py::arg("feat_enum"))
		.def(py::init<std::string &>(), py::arg("feat_name"))
		.def_readwrite("feat_enum", &FeatInfo::featEnum)
		.def_readwrite("feat_status_flags", &FeatInfo::flag, "0 - normal, 1 - automatic class feat, 2 - bonus selectable feat, 4 - selectable feat (disregard reqs)")
		;
		// methods
#pragma region methods
	mm
	.def("stat_level_get", [](int statEnum, int statArg)->int{
		auto stat = (Stat)statEnum;
		auto handle = chargen.GetEditedChar();
		auto &charPkt = chargen.GetCharEditorSelPacket();

		auto statLvl = 0;
		if (statArg != -1)
			statLvl = objects.StatLevelGet(handle, stat, statArg);
		else
			statLvl = objects.StatLevelGet(handle, stat);

		// Increase character level if appropriate
		if (!chargen.IsNewChar()){
			if (charPkt.classCode == stat) {
				statLvl++;
			}
			// Report 1 higher if the stat is being raised
			if (stat == charPkt.statBeingRaised) {
				statLvl++;
			}
		}
		
		return statLvl;
	}, py::arg("stat"), py::arg("stat_arg") = -1)
	.def("skill_level_get", [](int skillEnum)->int {
		auto skill = (SkillEnum)skillEnum;
		auto handle = chargen.GetEditedChar();
		auto &charPkt = chargen.GetCharEditorSelPacket();

		auto skillLevel = critterSys.SkillLevel(handle, skill);
		
		// Add additional skill points from the character editor if necessary

		if (!chargen.IsNewChar()) {
			auto levelRaised = charPkt.classCode;
			auto pointsSpent = 0;
			if (skill >= 0 && skill < skill_count) {
				pointsSpent += charPkt.skillPointsAdded[skill];
			}
			// Check if class skill - if not, halve their contribution (TODO sucks if skillLevel itself was rounded down :( )
			auto numAdded = pointsSpent/2;
			if (d20ClassSys.IsClassSkill(skill, levelRaised) ||
				(levelRaised == stat_level_cleric && deitySys.IsDomainSkill(handle, skill))
				|| d20Sys.D20QueryPython(handle, "Is Class Skill", skill)) {
				numAdded = pointsSpent;
			}

			skillLevel += numAdded;
			
		}
		return skillLevel;
	})
	.def("skill_ranks_get", [](int skillEnum)->int {
		auto skill = (SkillEnum)skillEnum;
		auto handle = chargen.GetEditedChar();
		auto &charPkt = chargen.GetCharEditorSelPacket();

		// Add additional skill points from the character editor if necessary
		auto skillRanks = critterSys.SkillBaseGet(handle, skill);

		if (!chargen.IsNewChar()){
			if (skill < skill_count) {
				skillRanks += charPkt.skillPointsAdded[skill];
			}
		}
		return skillRanks;
	})
	.def("has_feat", [](std::string featString)->int {
		auto feat = ElfHash::Hash(featString);
		return hasFeat(feat);
	})
	.def("get_feats", []()->std::vector<int> {
		std::vector<int> returnValues;
		auto handle = chargen.GetEditedChar();
		auto feats = objects.feats.GetFeats(handle);
		auto &charPkt = chargen.GetCharEditorSelPacket();

		if ((charPkt.feat0 != FEAT_INVALID) && (charPkt.feat0 != FEAT_NONE)) {
			feats.push_back(charPkt.feat0);
		}

		if ((charPkt.feat1 != FEAT_INVALID) && (charPkt.feat1 != FEAT_NONE)) {
			feats.push_back(charPkt.feat1);
		}

		if((charPkt.feat2 != FEAT_INVALID) && (charPkt.feat2 != FEAT_NONE)) {
			feats.push_back(charPkt.feat2);
		}

		if ((charPkt.feat3 != FEAT_INVALID) && (charPkt.feat3 != FEAT_NONE)) {
			feats.push_back(charPkt.feat3);
		}

		if ((charPkt.feat4 != FEAT_INVALID) && (charPkt.feat4 != FEAT_NONE)) {
			feats.push_back(charPkt.feat4);
		}

		for (auto &&feat : feats) {
			returnValues.push_back(static_cast<int>(feat));
		}
		
		return returnValues;
	})
	.def("has_feat", [](int featEnum)->int{
		auto feat = (feat_enums)featEnum;
		return hasFeat(feat);
	})

	.def("has_metamagic_feat", []()->int {
		int featCount = 0;
		auto handle = chargen.GetEditedChar();
		auto &charPkt = chargen.GetCharEditorSelPacket();

		for (auto mmfeat : feats.metamagicFeats) {
			featCount += feats.HasFeatCountByClass(handle, mmfeat);
		}

		if (feats.IsMetamagicFeat(charPkt.feat0)) {
			featCount++;
		}

		if (feats.IsMetamagicFeat(charPkt.feat1)) {
			featCount++;
		}

		if (feats.IsMetamagicFeat(charPkt.feat2)) {
			featCount++;
		}

		if (feats.IsMetamagicFeat(charPkt.feat3)) {
			featCount++;
		}

		return featCount;
	})
	
	.def("set_bonus_feats", [](std::vector<FeatInfo> & fti){
		chargen.SetBonusFeats(fti);
	})
	.def("get_spell_enums", []()->std::vector<KnownSpellInfo>& { // the right hand side ("Chosen Spells")
		return chargen.GetKnownSpellInfo();
	})
	.def("set_spell_enums", [](std::vector<KnownSpellInfo> &ksi){
		auto &spInfo = chargen.GetKnownSpellInfo();
		spInfo = ksi;
	})
	.def("append_spell_enums", [](std::vector<KnownSpellInfo> &ksi) {
		auto &spInfo = chargen.GetKnownSpellInfo();
		for (auto i = 0u; i < ksi.size(); i++){
			spInfo.push_back(ksi[i]);
		}
	})
	
	.def("get_available_spells", []()->std::vector<KnownSpellInfo>& { // the left hand side ("Available Spells")
		return chargen.GetAvailableSpells();
	})
	.def("append_available_spells", [](std::vector<KnownSpellInfo>& ksi) {
		auto& avSpells = chargen.GetAvailableSpells();
		for (auto i = 0u; i < ksi.size(); i++) {
			avSpells.push_back(ksi[i]);
		}
	})
	.def("has_armored_arcane_caster_feature", []()->bool
	{
		auto handle = chargen.GetEditedChar();
		auto &charPkt = chargen.GetCharEditorSelPacket();
		
		auto &classes = d20ClassSys.GetArmoredArcaneCasterFeatureClasses();

		for (auto c : classes) {
			if (charPkt.classCode == c) {
				return true;
			}
			if (objects.StatLevelGet(handle, c)) {
				return true;
			}
		}

		return false;
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
		auto &pkt = chargen.GetCharEditorSelPacket();
		auto orgStatLvl = 0;
		if (pkt.statBeingRaised >= stat_strength && pkt.statBeingRaised <= stat_charisma) {
			orgStatLvl = objects.StatLevelGetBase(handle, pkt.statBeingRaised);
			objects.StatLevelSetBase(handle,pkt.statBeingRaised, orgStatLvl + 1);
		}
		auto result = spellSys.GetMaxSpellLevel(handle, (Stat)classEnum, characterLvl);
		if (orgStatLvl > 0) {
			objects.StatLevelSetBase(handle, pkt.statBeingRaised, orgStatLvl);
		}
		return result;
	})
	.def("get_known_class_spells", [](objHndl handle, int classEnum)->std::vector<KnownSpellInfo>
	{ // get all spells the character knows that belong to the classEnum
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
	
	
	.def("spell_known_add", [](std::vector<KnownSpellInfo> &ksi){
		auto handle = chargen.GetEditedChar();
		for (auto it: ksi){
			auto spEnum = it.spEnum;
			if (spellSys.IsLabel(spEnum)
				|| spEnum == SPELL_ENUM_VACANT
				|| spellSys.IsNewSlotDesignator(spEnum))
				continue;

			SpellStoreData spData(spEnum, it.spellLevel, it.spellClass, 0, SpellStoreType::spellStoreKnown );
			if (spData.spellLevel == -1) {
				spData.spellLevel = spellSys.GetSpellLevelBySpellClass(spEnum, spData.classCode);
				if (spData.spellLevel == -1) {
					int castingClass = spellSys.GetCastingClass(it.spellClass);
					if (pythonClassIntegration.HasAdvancedLearning(castingClass)) {
						spData.spellLevel = pythonClassIntegration.GetAdvancedLearningSpellLevel(castingClass, spData.spellEnum);
					}
				}
			}
			
			if (spellSys.IsSpellKnown(handle,spEnum, spData.classCode))
				continue;

			spellSys.SpellKnownAdd(handle, spEnum, spData.classCode, spData.spellLevel, SpellStoreType::spellStoreKnown, 0 );
		}
	})
	;
#pragma endregion

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

}


objHndl UiCharEditor::GetEditedChar(){
	return chargen.GetEditedChar();
}

CharEditorSelectionPacket& UiCharEditor::GetCharEditorSelPacket(){
	return chargen.GetCharEditorSelPacket();
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

	uiManager->SetButtonState(stateBtnIds[2], LgcyButtonState::Disabled); // features

	// gain stat every 4 levels
	if (lvlNew % 4)
		uiManager->SetButtonState(stateBtnIds[1], LgcyButtonState::Disabled); // stats
	else
		uiManager->SetButtonState(stateBtnIds[1], LgcyButtonState::Normal); 

	if (lvlNew % 3)
		uiManager->SetButtonState(stateBtnIds[4], LgcyButtonState::Disabled); // feats
	else
		uiManager->SetButtonState(stateBtnIds[4], LgcyButtonState::Normal);


	mIsSelectingBonusFeat = false;
	// feats and features
	if (classCode >= stat_level_barbarian){

		auto classLvlNew = GetNewLvl(classCode);

		if (d20ClassSys.IsSelectingFeatsOnLevelup(handle, classCode) ) {
			uiManager->SetButtonState(stateBtnIds[4], LgcyButtonState::Normal); // feats
			mIsSelectingBonusFeat = true;
		}

		if (d20ClassSys.IsSelectingFeaturesOnLevelup(handle, classCode)) { // vanilla: was hardcoded for Wiz/Clr level == 1, Rgr lvl in [1,2, every 5th level]
			uiManager->SetButtonState(stateBtnIds[2], LgcyButtonState::Normal); // features
		}
	}
	
	// Spells
	if (selectingSpells){
		uiManager->SetButtonState(stateBtnIds[5], LgcyButtonState::Normal);
	} 
	else
	{
		uiManager->SetButtonState(stateBtnIds[5], LgcyButtonState::Disabled);
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

/* 0x101B0B70 */
BOOL UiCharEditor::ClassWidgetsInit(){
	static LgcyWindow classWnd(259,117, 405, 271);
	snprintf(classWnd.name , sizeof(classWnd.name), "char_editor_class_ui.c 193" );
	classWnd.flags = 1;
	classWnd.render = [](int widId) { uiCharEditor.StateTitleRender(widId); };
	classWndId = uiManager->AddWindow(classWnd);

	int coloff = 0, rowoff = 0;

	for (auto it: d20ClassSys.vanillaClassEnums){
		// class buttons
		LgcyButton classBtn("Class btn", classWndId, 71 + coloff, 47 + rowoff, 130, 20);
		coloff = 139 - coloff;
		if (!coloff)
			rowoff += 29;
		if (rowoff == 5 * 29) // the bottom button
			coloff = 69;

		classBtnRects.push_back(TigRect(classBtn.x, classBtn.y, classBtn.width, classBtn.height));
		classBtn.x += classWnd.x; classBtn.y += classWnd.y;
		classBtn.render = [](int id) {uiCharEditor.ClassBtnRender(id); };
		classBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.ClassBtnMsg(id, msg); };
		classBtn.SetDefaultSounds();
		classBtnIds.push_back(uiManager->AddButton(classBtn, classWndId));

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

	LgcyButton nextBtn("Class Next Button", classWndId, classWnd.x + 293, classWnd.y + 230, 55, 20);
	nextBtn.handleMessage = [](int widId, TigMsg*msg)->BOOL {
		if (uiCharEditor.classWndPage < uiCharEditor.mPageCount)
			uiCharEditor.classWndPage++;
		uiCharEditor.ClassSetPermissibles();
		return 1; };
	nextBtn.render = [](int id) { uiCharEditor.ClassNextBtnRender(id); };
	nextBtn.handleMessage = [](int widId, TigMsg*msg)->BOOL {	return uiCharEditor.ClassNextBtnMsg(widId, msg); };
	nextBtn.SetDefaultSounds();
	classNextBtn = uiManager->AddButton(nextBtn, classWndId);

	LgcyButton prevBtn("Class Prev. Button", classWndId, classWnd.x + 58, classWnd.y + 230, 55, 20);
	prevBtn.render = [](int id) { uiCharEditor.ClassPrevBtnRender(id); };
	prevBtn.handleMessage = [](int widId, TigMsg*msg)->BOOL {	return uiCharEditor.ClassPrevBtnMsg(widId, msg); };
	prevBtn.SetDefaultSounds();
	classPrevBtn = uiManager->AddButton(prevBtn, classWndId);
	
	return TRUE;
}

void UiCharEditor::ClassWidgetsFree(){
	for (auto it: classBtnIds){
		uiManager->RemoveChildWidget(it);
	}
	classBtnIds.clear();
	uiManager->RemoveChildWidget(classNextBtn);
	uiManager->RemoveChildWidget(classPrevBtn);
	uiManager->RemoveWidget(classWndId);
}

BOOL UiCharEditor::ClassShow(){
	chargen.SetIsNewChar(false);
	uiManager->SetHidden(classWndId, false);
	uiManager->BringToFront(classWndId);
	return 1;
}

BOOL UiCharEditor::ClassHide(){
	auto handle = GetEditedChar();
	auto classCode = GetCharEditorSelPacket().classCode;

	//Call IsSelectingSpellsOnLevelup now and keep the value so it does not need to be called continually
	selectingSpells = d20ClassSys.IsSelectingSpellsOnLevelup(handle, classCode);
	
	uiManager->SetHidden(classWndId, true);
	return 0;
}

BOOL UiCharEditor::ClassWidgetsResize(UiResizeArgs & args){
	for (auto it: classBtnIds){
		uiManager->RemoveChildWidget(it);
	}
	classBtnIds.clear();
	uiManager->RemoveChildWidget(classNextBtn);
	uiManager->RemoveChildWidget(classPrevBtn);
	uiManager->RemoveWidget(classWndId);
	classBtnFrameRects.clear();
	classBtnRects.clear();
	classTextRects.clear();
	return ClassWidgetsInit();
}

BOOL UiCharEditor::ClassCheckComplete(){
	auto &selPkt = GetCharEditorSelPacket();
	return (BOOL)(selPkt.classCode != 0);
}

/* 0x101B0DE0 */
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
		auto classEnum = (Stat)it;
		auto className = _strdup(d20Stats.GetStatName(classEnum));
		classNamesUppercase[it] = className;
		for (auto &letter: classNamesUppercase[it]){
			letter = toupper(letter);
		}

		if (config.nonCoreMaterials || d20ClassSys.IsCoreClass(classEnum)){
			if (config.newClasses || d20ClassSys.IsBaseClass(classEnum))
				classBtnMapping.push_back(it);
		}
			
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

	featsCenteredStyle = featsGreyedStyle = featsNormalTextStyle = featsExistingTitleStyle =  featsGoldenStyle =  featsClassStyle = baseStyle;

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
	//mIsSelectingBonusFeat = false; // should not do this here, since then if a user goes back to skills and decreases/increases them, it can cause problems

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
	featsMainWnd = LgcyWindow(259, 117, 405, 271);
	featsMainWnd.flags = 1;
	featsMainWnd.render = [](int widId) {uiCharEditor.FeatsWndRender(widId); };
	featsMainWnd.handleMessage = [](int widId, TigMsg*msg) { return uiCharEditor.FeatsWndMsg(widId, msg); };
	featsMainWndId = uiManager->AddWindow(featsMainWnd);

	// multi select wnd
	featsMultiCenterX = (w - 289) / 2;
	featsMultiCenterY = (h - 355) / 2;
	featsMultiSelectWnd = LgcyWindow(0, 0, w, h);
	auto featsMultiRefX = featsMultiCenterX + featsMultiSelectWnd.x;
	auto featsMultiRefY = featsMultiCenterY + featsMultiSelectWnd.y;
	featsMultiSelectWnd.flags = 1;
	featsMultiSelectWnd.render = [](int widId) {uiCharEditor.FeatsMultiSelectWndRender(widId); };
	featsMultiSelectWnd.handleMessage = [](int widId, TigMsg*msg) { return uiCharEditor.FeatsMultiSelectWndMsg(widId, msg); };
	featsMultiSelectWndId = uiManager->AddWindow(featsMultiSelectWnd);
	//scrollbar
	featsMultiSelectScrollbar.Init(256, 71, 219);
	featsMultiSelectScrollbar.parentId = featsMultiSelectWndId;
	featsMultiSelectScrollbar.x += featsMultiRefX;
	featsMultiSelectScrollbar.y += featsMultiRefY;
	featsMultiSelectScrollbarId = uiManager->AddScrollBar(featsMultiSelectScrollbar, featsMultiSelectWndId);
	
	//ok btn
	{
		LgcyButton multiOkBtn("Feats Multiselect Ok Btn", featsMultiSelectWndId, 29, 307, 110, 22);
		multiOkBtn.x += featsMultiRefX; multiOkBtn.y += featsMultiRefY;
		featMultiOkRect = TigRect(multiOkBtn.x, multiOkBtn.y, multiOkBtn.width, multiOkBtn.height);
		featMultiOkTextRect = TigRect(multiOkBtn.x, multiOkBtn.y + 4, multiOkBtn.width, multiOkBtn.height - 8);
		multiOkBtn.render = [](int id) {uiCharEditor.FeatsMultiOkBtnRender(id); };
		multiOkBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.FeatsMultiOkBtnMsg(id, msg); };
		multiOkBtn.renderTooltip = nullptr;
		multiOkBtn.SetDefaultSounds();
		featsMultiOkBtnId = uiManager->AddButton(multiOkBtn, featsMultiSelectWndId);
	}

	//cancel btn
	{
		LgcyButton multiCancelBtn("Feats Multiselect Cancel Btn", featsMultiSelectWndId, 153, 307, 110, 22);
		multiCancelBtn.x += featsMultiRefX; multiCancelBtn.y += featsMultiRefY;
		featMultiCancelRect = TigRect(multiCancelBtn.x, multiCancelBtn.y, multiCancelBtn.width, multiCancelBtn.height);
		featMultiCancelTextRect = TigRect(multiCancelBtn.x, multiCancelBtn.y + 4, multiCancelBtn.width, multiCancelBtn.height - 8);
		multiCancelBtn.render = [](int id) {uiCharEditor.FeatsMultiCancelBtnRender(id); };
		multiCancelBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.FeatsMultiCancelBtnMsg(id, msg); };
		multiCancelBtn.renderTooltip = nullptr;
		multiCancelBtn.SetDefaultSounds();
		featsMultiCancelBtnId = uiManager->AddButton(multiCancelBtn, featsMultiSelectWndId);
	}
	
	featMultiTitleRect = TigRect(featsMultiCenterX, featsMultiCenterY + 20, 289, 12);
	featsMultiSelectBtnIds.clear();
	featsMultiBtnRects.clear();
	auto rowOff = 75;
	for (auto i=0; i < FEATS_MULTI_BTN_COUNT; i++){
		LgcyButton featMultiBtn("Feats Multiselect btn", featsMultiSelectWndId, 23, 75 + i*(FEATS_MULTI_BTN_HEIGHT+2), 233, FEATS_MULTI_BTN_HEIGHT);

		featMultiBtn.x += featsMultiRefX; featMultiBtn.y += featsMultiRefY;
		featMultiBtn.render = [](int id) {uiCharEditor.FeatsMultiBtnRender(id); };
		featMultiBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.FeatsMultiBtnMsg(id, msg); };
		featMultiBtn.renderTooltip = nullptr;
		featMultiBtn.SetDefaultSounds();
		featsMultiSelectBtnIds.push_back(uiManager->AddButton(featMultiBtn, featsMultiSelectWndId));
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
		LgcyButton featsAvailBtn("Feats Available btn", featsMainWndId, 7, 38 + i*(FEATS_AVAIL_BTN_HEIGHT + 1), 169, FEATS_AVAIL_BTN_HEIGHT);

		featsAvailBtn.x += featsMainWnd.x; featsAvailBtn.y += featsMainWnd.y;
		featsAvailBtn.render = [](int id) {uiCharEditor.FeatsEntryBtnRender(id); };
		featsAvailBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.FeatsEntryBtnMsg(id, msg); };
		featsAvailBtn.renderTooltip = nullptr;
		featsAvailBtn.SetDefaultSounds();
		featsAvailBtnIds.push_back(uiManager->AddButton(featsAvailBtn, featsMainWndId));
		featsBtnRects.push_back(TigRect(featsAvailBtn.x - featsMainWnd.x, featsAvailBtn.y - featsMainWnd.y, featsAvailBtn.width, featsAvailBtn.height));
	}
	//scrollbar
	featsScrollbar.Init(186, 35, 230);
	featsScrollbar.parentId = featsMainWndId;
	featsScrollbar.x += featsMainWnd.x;
	featsScrollbar.y += featsMainWnd.y;
	featsScrollbarId = uiManager->AddScrollBar(featsScrollbar, featsMainWndId);


	// Existing feats
	featsExistingBtnIds.clear();
	featsExistingBtnRects.clear();
	for (auto i = 0; i < FEATS_EXISTING_BTN_COUNT; i++) {
		LgcyButton featsExistingBtn("Feats Existing btn", featsMainWndId, 212, 121 + i*(FEATS_EXISTING_BTN_HEIGHT + 1), 175, FEATS_EXISTING_BTN_HEIGHT);

		featsExistingBtn.x += featsMainWnd.x; featsExistingBtn.y += featsMainWnd.y;
		featsExistingBtn.render = [](int id) {uiCharEditor.FeatsExistingBtnRender(id); };
		featsExistingBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.FeatsExistingBtnMsg(id, msg); };
		featsExistingBtn.renderTooltip = nullptr;
		featsExistingBtn.SetDefaultSounds();
		featsExistingBtnIds.push_back(uiManager->AddButton(featsExistingBtn, featsMainWndId));
		featsExistingBtnRects.push_back(TigRect(featsExistingBtn.x - featsMainWnd.x, featsExistingBtn.y - featsMainWnd.y, featsExistingBtn.width, featsExistingBtn.height));
	}
	//scrollbar
	featsExistingScrollbar.Init(381, 117, 148);
	featsExistingScrollbar.parentId = featsMainWndId;
	featsExistingScrollbar.x += featsMainWnd.x;
	featsExistingScrollbar.y += featsMainWnd.y;
	featsExistingScrollbarId = uiManager->AddScrollBar(featsExistingScrollbar, featsMainWndId);

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
		uiManager->RemoveChildWidget(featsMultiSelectBtnIds[i]);
	}
	featsMultiSelectBtnIds.clear();

	for (auto i = 0; i < FEATS_AVAIL_BTN_COUNT; i++) {
		uiManager->RemoveChildWidget(featsAvailBtnIds[i]);
	}
	featsAvailBtnIds.clear();

	for (auto i = 0; i < FEATS_EXISTING_BTN_COUNT; i++) {
		uiManager->RemoveChildWidget(featsExistingBtnIds[i]);
	}
	featsExistingBtnIds.clear();

	uiManager->RemoveChildWidget(featsMultiOkBtnId);
	uiManager->RemoveChildWidget(featsMultiCancelBtnId);
	uiManager->RemoveChildWidget(featsMultiSelectScrollbarId);
	uiManager->RemoveChildWidget(featsScrollbarId);
	uiManager->RemoveChildWidget(featsExistingScrollbarId);

	auto wid = uiManager->GetWindow(featsMultiSelectWndId);
	auto wid2 = uiManager->GetWindow(featsMainWndId);

	uiManager->RemoveWidget(featsMultiSelectWndId);
	uiManager->RemoveWidget(featsMainWndId);
	
	
}

BOOL UiCharEditor::FeatsWidgetsResize(UiResizeArgs & args){
	FeatWidgetsFree();
	return FeatsWidgetsInit(args.rect1.width, args.rect1.height);
}

bool UiCharEditor::IsSelectingFeats()
{
	auto result = false;

	if (uiManager) {
		result = !uiManager->IsHidden(featsMainWndId);
	}

	return result;
}

BOOL UiCharEditor::FeatsShow(){
	featsMultiSelected = FEAT_NONE;
	uiManager->SetHidden(featsMainWndId, false);
	uiManager->BringToFront(featsMainWndId);
	return 1;
}

BOOL UiCharEditor::FeatsHide()
{
	uiManager->SetHidden(featsMainWndId, true);
	return 0;
}

void UiCharEditor::FeatsActivate(){
	mFeatsActivated = true;

	auto handle = GetEditedChar();
	auto &selPkt = GetCharEditorSelPacket();

	mIsSelectingBonusFeat = d20ClassSys.IsSelectingFeatsOnLevelup(handle, selPkt.classCode);
	chargen.BonusFeatsClear();
	if (mIsSelectingBonusFeat)
		d20ClassSys.LevelupGetBonusFeats(handle, selPkt.classCode); // can call set_bonus_feats

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

	featsExistingScrollbar = *uiManager->GetScrollBar(featsExistingScrollbarId);
	featsExistingScrollbar.scrollbarY = 0;
	featsExistingScrollbarY = 0;
	featsExistingScrollbar.yMax = max((int)mExistingFeats.size() - FEATS_EXISTING_BTN_COUNT, 0);
	*uiManager->GetScrollBar(featsExistingScrollbarId) = featsExistingScrollbar;

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
	for (auto feat: feats.newFeats){
		if (!feats.IsFeatEnabled(feat) && !feats.IsFeatMultiSelectMaster(feat))
			continue;
		if (!config.nonCoreMaterials && feats.IsNonCore(feat))
			continue;
		if (IsClassBonusFeat(feat)){
			mSelectableFeats.push_back(FeatInfo(feat));
			continue;
		}
		if (feats.IsFeatRacialOrClassAutomatic(feat))
			continue;
		if (feats.IsFeatPartOfMultiselect(feat))
			continue;
		if (feat == FEAT_NONE)
			continue;

		mSelectableFeats.push_back(FeatInfo(feat));
	}
	std::sort(mSelectableFeats.begin(), mSelectableFeats.end(), featSorter);

	featsScrollbar = *uiManager->GetScrollBar(featsScrollbarId);
	featsScrollbar.scrollbarY = 0;
	featsScrollbarY= 0;
	featsScrollbar.yMax = max((int)mSelectableFeats.size() - FEATS_AVAIL_BTN_COUNT, 0);
	*uiManager->GetScrollBar(featsScrollbarId) = featsScrollbar ;
}

BOOL UiCharEditor::SpellsWidgetsInit(){

	const int spellsWndX = 259, spellsWndY = 117, spellsWndW = 405, spellsWndH = 271;
	spellsWnd = LgcyWindow(spellsWndX, spellsWndY, spellsWndW, spellsWndH);
	spellsWnd.flags = 1;
	spellsWnd.render = [](int widId) {uiCharEditor.SpellsWndRender(widId); };
	spellsWnd.handleMessage = [](int widId, TigMsg*msg) { return uiCharEditor.SpellsWndMsg(widId, msg); };
	spellsWndId = uiManager->AddWindow(spellsWnd);

	// Available Spells Scrollbar
	spellsScrollbar.Init(183, 37, 230);
	spellsScrollbar.parentId = spellsWndId;
	spellsScrollbar.x += spellsWnd.x;
	spellsScrollbar.y += spellsWnd.y;
	spellsScrollbarId = uiManager->AddScrollBar(spellsScrollbar, spellsWndId);

	// Spell selection scrollbar
	spellsScrollbar2.Init(385, 37, 230);
	spellsScrollbar2.parentId = spellsWndId;
	spellsScrollbar2.x += spellsWnd.x;
	spellsScrollbar2.y += spellsWnd.y;
	spellsScrollbar2Id = uiManager->AddScrollBar(spellsScrollbar2, spellsWndId);

	int rowOff = 39;
	for (auto i = 0; i < SPELLS_BTN_COUNT; i++, rowOff += SPELLS_BTN_HEIGHT){
		
		LgcyButton spellAvailBtn("Spell Available btn", spellsWndId, 4, rowOff, 180, SPELLS_BTN_HEIGHT);

		spellAvailBtn.x += spellsWnd.x; spellAvailBtn.y += spellsWnd.y;
		spellAvailBtn.render = [](int id) {uiCharEditor.SpellsAvailableEntryBtnRender(id); };
		spellAvailBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.SpellsAvailableEntryBtnMsg(id, msg); };
		spellAvailBtn.renderTooltip = nullptr;
		spellAvailBtn.SetDefaultSounds();
		spellsAvailBtnIds.push_back(uiManager->AddButton(spellAvailBtn, spellsWndId));

		LgcyButton spellChosenBtn("Spell Chosen btn", spellsWndId, 206, rowOff, 170, SPELLS_BTN_HEIGHT);

		spellChosenBtn.x += spellsWnd.x; spellChosenBtn.y += spellsWnd.y;
		spellChosenBtn.render = [](int id) {uiCharEditor.SpellsEntryBtnRender(id); };
		spellChosenBtn.handleMessage = [](int id, TigMsg* msg) { return uiCharEditor.SpellsEntryBtnMsg(id, msg); };
		spellChosenBtn.renderTooltip = nullptr;
		spellChosenBtn.SetDefaultSounds();
		spellsChosenBtnIds.push_back(uiManager->AddButton(spellChosenBtn, spellsWndId));

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
		auto textMeas = UiRenderer::MeasureTextSize(chargen.levelLabels[lvl].c_str(), spellLevelLabelStyle);
		spellsNewSpellsBoxRects.push_back(TigRect(116 + lvl * 45, 287, 29, 12));

	}
	UiRenderer::PopFont();
	return 1;
}

void UiCharEditor::SpellsWidgetsFree(){
	for (auto i = 0; i < SPELLS_BTN_COUNT; i++){
		uiManager->RemoveChildWidget(spellsChosenBtnIds[i]);
		uiManager->RemoveChildWidget(spellsAvailBtnIds[i]);
	}
	spellsChosenBtnIds.clear();
	spellsAvailBtnIds.clear();
	uiManager->RemoveWidget(spellsWndId);
}

BOOL UiCharEditor::SpellsShow(){
	uiManager->SetHidden(spellsWndId, false);
	uiManager->BringToFront(spellsWndId);
	return 1;
}

BOOL UiCharEditor::SpellsHide(){
	uiManager->SetHidden(spellsWndId, true);
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
	auto &avSpInfo = chargen.GetAvailableSpells();
	auto &knSpInfo = chargen.GetKnownSpellInfo();

	
	auto &needPopulateEntries = temple::GetRef<int>(0x10C4D4C4);

	static auto setScrollbars = []() {
		auto sbId = uiCharEditor.spellsScrollbarId;
		uiManager->ScrollbarSetY(sbId, 0);
		int numEntries = (int)chargen.GetAvailableSpells().size();
		uiManager->ScrollbarSetYmax(sbId, max(0, numEntries - uiCharEditor.SPELLS_BTN_COUNT));
		uiCharEditor.spellsScrollbar = *uiManager->GetScrollBar(sbId);
		uiCharEditor.spellsScrollbar.y = 0;
		uiCharEditor.spellsScrollbarY = 0;

		auto &charEdSelPkt = uiCharEditor.GetCharEditorSelPacket();
		auto sbAddedId = uiCharEditor.spellsScrollbar2Id;
		int numAdded = (int)chargen.GetKnownSpellInfo().size();
		uiManager->ScrollbarSetY(sbAddedId, 0); 
		uiManager->ScrollbarSetYmax(sbAddedId, max(0, numAdded - uiCharEditor.SPELLS_BTN_COUNT));
		uiCharEditor.spellsScrollbar2 = *uiManager->GetScrollBar(sbAddedId);
		uiCharEditor.spellsScrollbar2.y = 0;
		uiCharEditor.spellsScrollbar2Y = 0;
	};

	if (!needPopulateEntries) {
		setScrollbars();
		return;
	}


	knSpInfo.clear();
	avSpInfo.clear();
	selPkt.spellEnumToRemove = 0;


	/*auto newLvl = GetNewLvl(selPkt.classCode);
	if (newLvl == 1)
		newLvl = 0;*/
	d20ClassSys.LevelupInitSpellSelection(handle, selPkt.classCode);

	
	for (auto i = 0u; i < knSpInfo.size(); i++){
		auto spEnum = knSpInfo[i].spEnum;
		if (spellSys.IsNewSlotDesignator(spEnum)){
			knSpInfo[i].spEnum = SPELL_ENUM_VACANT;
			knSpInfo[i].spFlag = 3;
		}
	}

	SpellsPerDayUpdate();

	setScrollbars();
	needPopulateEntries = 0; // needPopulateEntries

}

BOOL UiCharEditor::SpellsCheckComplete(){
	auto &selPkt = GetCharEditorSelPacket();
	auto handle = GetEditedChar();

	if (!selectingSpells) {
		return true;
	}

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
	chargen.GetKnownSpellInfo().clear();
	chargen.GetAvailableSpells().clear();
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

void UiCharEditor::MainWndRender(int widId){
	auto &dword_10BE9970 = temple::GetRef<int>(0x10BE9970);
	if (!dword_10BE9970){
		auto statesComplete = 0;

		auto stage = 0;
		auto &mStagesComplete = GetStatesComplete();
		auto &selPkt = GetCharEditorSelPacket();
		auto &mActiveStage = GetState();


		for (stage = 0; stage < std::min(mStagesComplete + 1, (int)CharEditorStages::CE_STAGE_COUNT); stage++) {
			auto &sys = lgcySystems[stage];
			if (!sys.checkComplete)
				break;
			if (!sys.checkComplete())
				break;
		}

		if (stage != mStagesComplete) {
			mStagesComplete = stage;
			if (mActiveStage > stage)
				mActiveStage = stage;

			// reset the next stages
			for (auto nextStage = stage + 1; nextStage < CharEditorStages::CE_STAGE_COUNT; nextStage++) {
				if (lgcySystems[nextStage].reset) {
					lgcySystems[nextStage].reset(selPkt);
				}
			}
		}
	}

	auto wnd = uiManager->GetWindow(widId);
	RenderHooks::RenderImgFile(temple::GetRef<ImgFile*>(0x10BE9974), wnd->x, wnd->y);
	
	UiRenderer::DrawTextureInWidget( widId, temple::GetRef<int>(0x10BE993C),
		{406, 15, 120, 227} , { 1,1,120,227 });
	

}

void UiCharEditor::ClassBtnRender(int widId){
	auto idx = WidgetIdIndexOf(widId, &classBtnIds[0], classBtnIds.size());
	if (idx == -1)
		return;

	auto page = GetClassWndPage();
	auto classCode = GetClassCodeFromWidgetAndPage(idx, page);
	if (classCode == (Stat)-1)
		return;

	static TigRect srcRect(1, 1, 120, 30);
	UiRenderer::DrawTexture(buttonBox, classBtnFrameRects[idx], srcRect);

	auto btnState = uiManager->GetButtonState(widId);
	if (btnState != LgcyButtonState::Disabled && btnState != LgcyButtonState::Down)
	{
		auto &selPkt = GetCharEditorSelPacket();
		if (selPkt.classCode == classCode)
			btnState = LgcyButtonState::Released;
		else if (btnState != LgcyButtonState::Hovered)
			btnState = LgcyButtonState::Normal;
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
	
	auto idx = WidgetIdIndexOf(widId, &classBtnIds[0], classBtnIds.size());
	if (idx == -1)
		return 0;
	
	auto _msg = (TigMsgWidget*)msg;
	auto classCode = GetClassCodeFromWidgetAndPage(idx, GetClassWndPage());
	if (classCode == (Stat)-1)
		return 0;

	if (_msg->widgetEventType == TigMsgWidgetEvent::Clicked){
		if (helpSys.IsClickForHelpActive()){
			helpSys.PresentWikiHelp(HELP_IDX_CLASSES + classCode - stat_level_barbarian, D20HelpType::Classes);
			return TRUE;
		}

		GetCharEditorSelPacket().classCode = classCode;
		PrepareNextStages();
		temple::GetRef<void(__cdecl)(int)>(0x10143FF0)(0); // resets all the next systems in case of change
		if (ClassSanitize()) { // resets the chosen class in case the user cheats (e.g. by selecting skills up ahead)
			PrepareNextStages();
			temple::GetRef<void(__cdecl)(int)>(0x10143FF0)(0); // resets all the next systems in case of change
		}
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
		return FALSE;

	auto _msg = (TigMsgWidget*)msg;

	if (_msg->widgetEventType == TigMsgWidgetEvent::Clicked) {
		if (classWndPage < mPageCount-1)
			classWndPage++;
		uiCharEditor.ClassSetPermissibles();
		return TRUE;
	}

	if (_msg->widgetEventType == TigMsgWidgetEvent::Exited) {
		temple::GetRef<void(__cdecl)(const char*)>(0x10162C00)("");
		return 1;
	}

	if (_msg->widgetEventType == TigMsgWidgetEvent::Entered) {
		auto textboxText = fmt::format("Prestige Classes");
		if (textboxText.size() >= 1024)
			textboxText[1023] = 0;
		strcpy(temple::GetRef<char[1024]>(0x10C80CC0), &textboxText[0]);
		temple::GetRef<void(__cdecl)(const char*)>(0x10162C00)(temple::GetRef<char[1024]>(0x10C80CC0));
		return 1;
	}

	return FALSE;
}

BOOL UiCharEditor::ClassPrevBtnMsg(int widId, TigMsg * msg){
	if (msg->type != TigMsgType::WIDGET)
		return 0;

	auto _msg = (TigMsgWidget*)msg;

	if (_msg->widgetEventType == TigMsgWidgetEvent::Clicked) {
		if (classWndPage > 0)
			classWndPage--;
		uiCharEditor.ClassSetPermissibles();
		return TRUE;
	}


	return FALSE;
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
	
	auto spellcastCond = (std::string)d20ClassSys.GetSpellCastingCondition(selPkt.classCode);
	if ( spellcastCond.size() ){
		conds.AddTo(charEdited, spellcastCond, {0,0,0,0, 0,0,0,0});
	}

	// Final refresh once alignment_choice has been set
	d20StatusSys.D20StatusRefresh(charEdited);

	return 1;
}

void UiCharEditor::ClassNextBtnRender(int widId){

	if (mPageCount <= 1)
		return;

	static TigRect srcRect(1, 1, 120, 30);
	UiRenderer::DrawTexture(buttonBox, classNextBtnFrameRect, srcRect);

	auto btnState = uiManager->GetButtonState(widId);
	if (btnState != LgcyButtonState::Disabled && btnState != LgcyButtonState::Down){
		btnState = btnState == LgcyButtonState::Hovered ? LgcyButtonState::Hovered : LgcyButtonState::Normal;
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

	if (mPageCount <= 1)
		return;

	static TigRect srcRect(1, 1, 120, 30);
	UiRenderer::DrawTexture(buttonBox, classPrevBtnFrameRect, srcRect);

	auto btnState = uiManager->GetButtonState(widId);
	if (btnState != LgcyButtonState::Disabled && btnState != LgcyButtonState::Down) {
		btnState = btnState == LgcyButtonState::Hovered ? LgcyButtonState::Hovered : LgcyButtonState::Normal;
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
		UiRenderer::DrawTextInWidget(widId, featsTitleString, featsTitleRect, featsNormalTextStyle);
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
			uiManager->ScrollbarGetY(featsScrollbarId, &featsScrollbarY);
			uiManager->ScrollbarGetY(featsExistingScrollbarId, &featsExistingScrollbarY);
		}
		return FALSE;
	}

	if (msg->type != TigMsgType::MOUSE)
		return FALSE;

	
	auto msgM = (TigMsgMouse*)msg;
	auto &selPkt = GetCharEditorSelPacket();

	if (msgM->buttonStateFlags & MouseStateFlags::MSF_RMB_RELEASED && uiManager->IsHidden(featsMultiSelectWndId)) {
		
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

			if (FeatAlreadyPicked(feat)) // fixes being able to right click and gain the feat more than once when you shouldn't
				break;

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
		featsScrollbar = *uiManager->GetScrollBar(featsScrollbarId);
		if (featsScrollbar.handleMessage)
			return featsScrollbar.handleMessage(featsScrollbarId, (TigMsg*)&msgCopy);
	}

	if ((int)msgM->x >= featsMainWnd.x + 207 && (int)msgM->x <= featsMainWnd.x + 392
		&& (int)msgM->y >= featsMainWnd.y + 118 && (int)msgM->y <= featsMainWnd.y + 263) {
		featsExistingScrollbar = *uiManager->GetScrollBar(featsExistingScrollbarId);
		if (featsExistingScrollbar.handleMessage)
			return featsExistingScrollbar.handleMessage(featsExistingScrollbarId, (TigMsg*)&msgCopy);
	}

	return FALSE;
}

BOOL UiCharEditor::FeatsEntryBtnMsg(int widId, TigMsg * msg){

	if (msg->type != TigMsgType::WIDGET)
		return 0;
	auto msgW = (TigMsgWidget*)msg;

	auto widIdx = WidgetIdIndexOf(widId, &featsAvailBtnIds[0], FEATS_AVAIL_BTN_COUNT);
	auto featIdx = widIdx + featsScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mSelectableFeats.size())
		return FALSE;

	auto featInfo = mSelectableFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;

	auto &selPkt = GetCharEditorSelPacket();
	auto btn = uiManager->GetButton(widId);

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
			if (featsSelectedBorderRect.ContainsPoint(msgW->x, msgW->y) && IsSelectingNormalFeat() && CanDropToNormalSlot(feat)){
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

	auto widIdx = WidgetIdIndexOf(widId, &featsAvailBtnIds[0], FEATS_AVAIL_BTN_COUNT);
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

	auto widIdx = WidgetIdIndexOf(widId, &featsExistingBtnIds[0], FEATS_EXISTING_BTN_COUNT);
	auto featIdx = widIdx + featsExistingScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mExistingFeats.size())
		return FALSE;

	auto featInfo = mExistingFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;

	auto &selPkt = GetCharEditorSelPacket();
	auto btn = uiManager->GetButton(widId);

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
	auto widIdx = WidgetIdIndexOf(widId, &featsExistingBtnIds[0], FEATS_EXISTING_BTN_COUNT);
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

	if (feat >NUM_FEATS){
		std::vector<feat_enums> tmp;
		feats.MultiselectGetChildren(feat, tmp);
		for (auto it: tmp){
			mMultiSelectFeats.push_back(FeatInfo(it));
		}
	}
	else{
		auto featIt = FEAT_ACROBATIC;
		auto featProp = 0x100;
		switch (feat) {
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
	}

	

	featsMultiSelectScrollbar = *uiManager->GetScrollBar(featsMultiSelectScrollbarId);
	featsMultiSelectScrollbar.scrollbarY = 0;
	featsMultiSelectScrollbarY = 0;
	featsMultiSelectScrollbar.yMax = max(0, (int)mMultiSelectFeats.size() - FEATS_MULTI_BTN_COUNT);
	featsMultiSelectScrollbar = *uiManager->GetScrollBar(featsMultiSelectScrollbarId);
	uiManager->SetButtonState(featsMultiOkBtnId, LgcyButtonState::Disabled);

	uiManager->SetHidden(featsMultiSelectWndId, false);
	uiManager->BringToFront(featsMultiSelectWndId);
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
	
	uiManager->ScrollbarGetY(featsMultiSelectScrollbarId, &featsMultiSelectScrollbarY);
	
	return TRUE;
}

void UiCharEditor::FeatsMultiOkBtnRender(int widId){
	auto buttonState = uiManager->GetButtonState(widId);

	int texId;
	switch(buttonState){
	case LgcyButtonState::Normal:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptNormal, texId);
		break;
	case LgcyButtonState::Hovered:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptHover, texId);
		break;
	case LgcyButtonState::Down:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptPressed, texId);
		break;
	case LgcyButtonState::Disabled:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::DisabledNormal, texId);
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
	uiManager->SetHidden(featsMultiSelectWndId, true);

	return TRUE;
}

void UiCharEditor::FeatsMultiCancelBtnRender(int widId){
	auto buttonState = uiManager->GetButtonState(widId);

	int texId;
	switch (buttonState) {
	case LgcyButtonState::Normal:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineNormal, texId);
		break;
	case LgcyButtonState::Hovered:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineHover, texId);
		break;
	case LgcyButtonState::Down:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::DeclinePressed, texId);
		break;
	case LgcyButtonState::Disabled:
		uiAssets->GetAsset(UiAssetType::Generic, UiGenericAsset::DisabledNormal, texId);
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
	uiManager->SetHidden(featsMultiSelectWndId, true);

	return TRUE;
}

void UiCharEditor::FeatsMultiBtnRender(int widId){
	auto widIdx = WidgetIdIndexOf(widId, &featsMultiSelectBtnIds[0], FEATS_MULTI_BTN_COUNT);
	auto featIdx = widIdx + featsMultiSelectScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mMultiSelectFeats.size())
		return;

	auto featInfo = mMultiSelectFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;



	auto getFeatShortName = [](feat_enums ft){

		if (ft > NUM_FEATS)
		{
			auto dummy = 1;
		}

		if (feats.IsFeatMultiSelectMaster(ft))
			return uiCharEditor.GetFeatName(ft);
		

		auto mesKey = 50000 + ft;
		
		if (feats.IsFeatPropertySet(ft, FPF_GREAT_WEAP_SPEC_ITEM)){
			mesKey = 50000 + (ft - FEAT_GREATER_WEAPON_SPECIALIZATION_GAUNTLET + FEAT_WEAPON_SPECIALIZATION_GAUNTLET);
		}

		MesLine line(mesKey);
		auto pcCreationMes = temple::GetRef<MesHandle>(0x11E72EF0);
		auto text = mesFuncs.GetLineById(pcCreationMes,mesKey);
		if (text){
			return std::string(text);
		}
		else
			return uiCharEditor.GetFeatName(ft);	
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

	auto widIdx = WidgetIdIndexOf(widId, &featsMultiSelectBtnIds[0], FEATS_MULTI_BTN_COUNT);
	auto featIdx = widIdx + featsMultiSelectScrollbarY;
	if (widIdx == -1 || featIdx >= (int)mMultiSelectFeats.size())
		return FALSE;

	auto featInfo = mMultiSelectFeats[featIdx];
	auto feat = (feat_enums)featInfo.featEnum;

	auto &selPkt = GetCharEditorSelPacket();
	auto btn = uiManager->GetButton(widId);

	switch (msgW->widgetEventType) {
	case TigMsgWidgetEvent::MouseReleased:
		if (helpSys.IsClickForHelpActive()) {
			helpSys.PresentWikiHelp(109 + feat);
			return TRUE;
		}
		if (FeatCanPick(feat) && !FeatAlreadyPicked(feat)){
			featsMultiSelected = feat;
			uiManager->SetButtonState(featsMultiOkBtnId, LgcyButtonState::Normal);
		} else
		{
			featsMultiSelected = FEAT_NONE;
			uiManager->SetButtonState(featsMultiOkBtnId, LgcyButtonState::Disabled);
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

		if (uiCharEditor.IsClassBonusFeat(feat)) {  // is choosing class bonus right now 
			return uiCharEditor.featsGoldenStyle;
		}
		else if (feats.IsClassFeat(feat))// class Specific feat
		{
			return uiCharEditor.featsClassStyle;
		}
		else
			return uiCharEditor.featsNormalTextStyle;
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

	if (IsSelectingBonusFeat() && IsClassBonusFeat(feat)){
		if ( feats.IsFeatPropertySet(feat, FPF_ROGUE_BONUS))
			return true;
		if (chargen.IsBonusFeatDisregardingPrereqs(feat))
			return true;
	}


	if (!feats.IsFeatMultiSelectMaster(feat)) {
		return feats.FeatPrereqsCheck(handle, feat, featsPicked.size() > 0 ? &featsPicked[0] : nullptr, featsPicked.size(), selPkt.classCode, selPkt.statBeingRaised) != FALSE;
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

		return (ftrLvl >= 4);
		

	case FEAT_GREATER_WEAPON_FOCUS:
		if (ftrLvl < 8)
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
		return feats.FeatPrereqsCheck(handle, feat, featsPicked.size() > 0 ? &featsPicked[0] : nullptr, featsPicked.size(), selPkt.classCode, selPkt.statBeingRaised) != FALSE;
	}
}

bool UiCharEditor::IsSelectingRangerSpec()
{
	auto &selPkt = GetCharEditorSelPacket();
	auto handle = GetEditedChar();
	auto isRangerSpecial = selPkt.classCode == stat_level_ranger && (objects.StatLevelGet(handle, stat_level_ranger) + 1) == 2;
	return isRangerSpecial;
}

bool UiCharEditor::IsClassBonusFeat(feat_enums feat){
	return chargen.IsClassBonusFeat(feat);
}

bool UiCharEditor::CanDropToNormalSlot(feat_enums feat) {
	return !chargen.IsBonusOnlyFeat(feat);
}

bool Chargen::IsClassBonusFeat(feat_enums feat) {
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
		return feats.IsFighterFeat(feat) != FALSE;
	case stat_level_monk:
		if (feats.IsFeatPropertySet(feat, FPF_MONK_BONUS_1st) && newLvl == 1)
			return true;
		if (feats.IsFeatPropertySet(feat, FPF_MONK_BONUS_2nd) && newLvl == 2)
			return true;
		if (feats.IsFeatPropertySet(feat, FPF_MONK_BONUS_6th) && newLvl == 6)
			return true;
		return false;
	case stat_level_ranger:
		return (newLvl == 2 && (feat == FEAT_RANGER_TWO_WEAPON_STYLE || feat == FEAT_RANGER_ARCHERY_STYLE));
	case stat_level_rogue:
		return (feat < FEAT_NONE &&  newLvl >= 10 && !( (newLvl-10) % 3));
	case stat_level_wizard:
		return feats.IsMagicFeat(feat) != FALSE;
	default:
		return false;
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

	return feats.MultiselectGetFirst(feat);
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
			uiManager->ScrollbarGetY(spellsScrollbarId, &spellsScrollbarY);
			uiManager->ScrollbarGetY(spellsScrollbar2Id, &spellsScrollbar2Y);
			SpellsPerDayUpdate();
			return 1;
		}
		return 0;
	}

	if (msg->type == TigMsgType::MOUSE){
		auto msgM = (TigMsgMouse*)msg;
		if ((msgM->buttonStateFlags & MouseStateFlags::MSF_LMB_RELEASED) && helpSys.IsClickForHelpActive()){
			// LMB handler - present help for spell

			auto &knSpInfo = chargen.GetKnownSpellInfo();
			for (auto i = 0; i < SPELLS_BTN_COUNT; i++){
				// check if mouse within button
				if (!uiManager->DoesWidgetContain(spellsChosenBtnIds[i], msgM->x, msgM->y))
					continue;
				
				auto spellIdx = i + spellsScrollbar2Y;
				if ((uint32_t)spellIdx >= knSpInfo.size())
					break;
				
				auto spEnum = knSpInfo[spellIdx].spEnum;
				// ensure is not label
				if (spellSys.IsLabel(spEnum))
					break;
				
				helpSys.PresentWikiHelp(HELP_IDX_SPELLS + spEnum);
				return 1;
			}
		}
		if (msgM->buttonStateFlags & MouseStateFlags::MSF_RMB_RELEASED) {
			// RMB handler - add to known spells

			auto &knSpInfo = chargen.GetKnownSpellInfo();
			auto &avSpInfo = chargen.GetAvailableSpells();

			for (auto i = 0; i < SPELLS_BTN_COUNT; i++) {
				// get spell btn
				if (!uiManager->DoesWidgetContain(spellsAvailBtnIds[i], msgM->x, msgM->y))
					continue;
				auto spellAvailIdx = i + spellsScrollbarY;
				if ((uint32_t)spellAvailIdx >= avSpInfo.size())
					break;

				// got the avail btn, now search for suitable vacant slot
				auto spEnum = avSpInfo[spellAvailIdx].spEnum;
				auto spClass = avSpInfo[spellAvailIdx].spellClass;
				auto spLevel = avSpInfo[spellAvailIdx].spellLevel;

				if (spellSys.IsLabel(spEnum))
					break;

				if (chargen.SpellIsAlreadyKnown(spEnum, spClass) || chargen.SpellIsForbidden(spEnum, spClass))
					break;

				auto curSpellLvl = -1;
				auto foundSlot = false;
				for (auto j = 0u; j < knSpInfo.size(); j++){
					auto spInfo = knSpInfo[j];
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
							knSpInfo[j].spEnum = spEnum; // spell level might still be -1 so be careful when adding to spellbook later on!
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
			spellsScrollbar = *uiManager->GetScrollBar(spellsScrollbarId);
			if (spellsScrollbar.handleMessage)
				return spellsScrollbar.handleMessage(spellsScrollbarId, (TigMsg*)&msgCopy);
		}

		if ((int)msgM->x >= spellsWnd.x +206 && (int)msgM->x <= spellsWnd.x + 376
			&& (int)msgM->y >= spellsWnd.y && (int)msgM->y <= spellsWnd.y + 259) {
			spellsScrollbar2 = *uiManager->GetScrollBar(spellsScrollbar2Id);
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
	auto widIdx = WidgetIdIndexOf(widId, &spellsChosenBtnIds[0], SPELLS_BTN_COUNT);
	if (widIdx == -1)
		return;

	auto &knSpInfo = chargen.GetKnownSpellInfo();

	auto spellIdx = widIdx + spellsScrollbar2Y;
	if (spellIdx >= (int)knSpInfo.size())
		return;

	auto spInfo = knSpInfo[spellIdx];
	auto spFlag = spInfo.spFlag;
	auto spEnum = spInfo.spEnum;
	auto spLvl = spInfo.spellLevel;

	auto btn = uiManager->GetButton(widId);
	
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
			text.append(fmt::format("{}", chargen.spellLevelLabels[spLvl]));
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

	auto widIdx = WidgetIdIndexOf(widId, &spellsAvailBtnIds[0], SPELLS_BTN_COUNT);
	if (widIdx == -1)
		return 0;

	auto &avSpInfo = chargen.GetAvailableSpells();
	auto spellIdx = widIdx + spellsScrollbarY;
	if (spellIdx >= (int)avSpInfo.size())
		return 0;

	auto spInfo = avSpInfo[spellIdx];
	auto spFlag = spInfo.spFlag;
	auto spEnum = spInfo.spEnum;
	auto spLvl = spInfo.spellLevel;
	auto spClass = spInfo.spellClass;

	if (!spellSys.IsLabel(spEnum)){
		
		auto btn = uiManager->GetButton(widId);
		auto curSpellLvl = -1;
		auto &selPkt = GetCharEditorSelPacket();
		auto &knSpInfo = chargen.GetKnownSpellInfo();

		switch (msgW->widgetEventType){
			case TigMsgWidgetEvent::Clicked: // button down - initiate drag
				if (!chargen.SpellIsAlreadyKnown(spEnum, spClass ) && !chargen.SpellIsForbidden(spEnum, spClass)){
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
					helpSys.PresentWikiHelp(spEnum + HELP_IDX_SPELLS);
					return 1;
				}
			case TigMsgWidgetEvent::MouseReleasedAtDifferentButton: 
				mouseFuncs.SetCursorDrawCallback(nullptr, 0);
				if (chargen.SpellIsAlreadyKnown(spEnum, spClass)
					|| chargen.SpellIsForbidden(spEnum, spClass))
					return 1;


				for (auto i = 0u; i < knSpInfo.size(); i++){
					auto rhsSpInfo = knSpInfo[i];

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
					if (!uiManager->DoesWidgetContain(spellsChosenBtnIds[chosenWidIdx], msgW->x, msgW->y))
						continue;

					if (rhsSpInfo.spellLevel == -1 // wildcard slot (a la Wizards)
						|| rhsSpInfo.spellLevel == spLvl 
						   && rhsSpInfo.spFlag != 0
						   && (rhsSpInfo.spFlag != 1 || !selPkt.spellEnumToRemove)
						)
					{
						
						if (rhsSpInfo.spFlag == 1){ // replaceable spell - replace it, and remember which one we removed
							knSpInfo[i].spFlag = 2;
							selPkt.spellEnumToRemove = rhsSpInfo.spEnum;
						} 
						else if (rhsSpInfo.spFlag == 2 && selPkt.spellEnumToRemove == spEnum){ // was already replaced, and now restoring
							knSpInfo[i].spFlag = 1;
							selPkt.spellEnumToRemove = 0;
						}
						knSpInfo[i].spEnum = spEnum;
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

	auto widIdx = WidgetIdIndexOf(widId, &spellsAvailBtnIds[0], SPELLS_BTN_COUNT);
	if (widIdx == -1)
		return;

	auto &avSpInfo = chargen.GetAvailableSpells();

	auto spellIdx = widIdx + spellsScrollbarY;
	if (spellIdx >= (int)avSpInfo.size())
		return;

	auto btn = uiManager->GetButton(widId);
	auto spEnum = avSpInfo[spellIdx].spEnum;

	std::string text;
	TigRect rect(btn->x - spellsWnd.x, btn->y - spellsWnd.y, btn->width, btn->height);
	UiRenderer::PushFont(PredefinedFont::PRIORY_12);
	if (spellSys.IsLabel(spEnum)){
		rect.x += 2;
		auto spLvl = avSpInfo[spellIdx].spellLevel;
		if (spLvl >= 0 && spLvl < NUM_SPELL_LEVELS)
		{
			text.append(fmt::format("{}", chargen.spellLevelLabels[spLvl]));
			UiRenderer::DrawTextInWidget(spellsWndId, text, rect, spellLevelLabelStyle);
		}
			
	} 
	else
	{
		text.append(fmt::format("{}", spellSys.GetSpellMesline(spEnum)));
		rect.x += 12;
		//rect.width -= 11;
		if (chargen.SpellIsAlreadyKnown(spEnum, avSpInfo[spellIdx].spellClass)
			|| chargen.SpellIsForbidden(spEnum, avSpInfo[spellIdx].spellClass))
			UiRenderer::DrawTextInWidget(spellsWndId, text, rect, spellsAvailBtnStyle);
		else
			UiRenderer::DrawTextInWidget(spellsWndId, text, rect, spellsTextStyle);
	}
	UiRenderer::PopFont();
	
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

int& UiCharEditor::GetStatesComplete(){
	return temple::GetRef<int>(0x10BE8D38);
}

void UiCharEditor::ClassSetPermissibles(){
	auto page = GetClassWndPage();
	auto idx = 0;
	auto handle = GetEditedChar();
	for (auto it:classBtnIds){
		auto classCode = GetClassCodeFromWidgetAndPage(idx++, page);
		if (classCode == (Stat)-1)
			uiManager->SetButtonState(it, LgcyButtonState::Disabled);
		else if (d20ClassSys.ReqsMet(handle, classCode) && pythonClassIntegration.IsAlignmentCompatible(handle, classCode)){
			uiManager->SetButtonState(it, LgcyButtonState::Normal);
		}
		else{
			uiManager->SetButtonState(it, LgcyButtonState::Disabled);
		}
		
	}

	if (mPageCount <= 1){
		uiManager->SetButtonState(classNextBtn, LgcyButtonState::Disabled);
		uiManager->SetButtonState(classPrevBtn, LgcyButtonState::Disabled);
		return;
	}

	if (page > 0)
		uiManager->SetButtonState(classPrevBtn, LgcyButtonState::Normal);
	else 
		uiManager->SetButtonState(classPrevBtn, LgcyButtonState::Disabled);

	if (page < mPageCount-1)
		uiManager->SetButtonState(classNextBtn, LgcyButtonState::Normal);
	else
		uiManager->SetButtonState(classNextBtn, LgcyButtonState::Disabled);
}

bool UiCharEditor::ClassSanitize(){
	auto handle = GetEditedChar();
	auto &selPkt = GetCharEditorSelPacket();
	auto classCode = selPkt.classCode;
	if (!d20ClassSys.ReqsMet(handle, classCode) || !pythonClassIntegration.IsAlignmentCompatible(handle, classCode)){
		selPkt.classCode = (Stat)0;
		return true;
	}

	return false;
}


int UiCharEditor::GetNewLvl(Stat classEnum){ // default is classEnum  = stat_level i.e. get the overall new level
	auto handle = GetEditedChar();
	return objects.StatLevelGet(handle, classEnum) + 1;
}



class UiCharEditorHooks : public TempleFix {
	
	/*static int HookedHasFeatCountByClassSimple(objHndl handle, feat_enums feat) {
		return feats.HasFeatCountByClass(handle, feat) > 0 ? 1 : 0;
	}*/

	void apply() override {

		// Fixes error - apparently a player somehow managed to get a MM feat (extend spell) more than once
		//redirectCall(0x101BA672, HookedHasFeatCountByClassSimple); // function 0x101BA580 now completely replaced

		// general
		replaceFunction<void(int)>(0x10148880, [](int widId){
			uiCharEditor.MainWndRender(widId);
		});

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
		
		// Stat

		// Hook for stat increase buttons to reset subsequent levelup state
		static BOOL (__cdecl*orgStatBtnEvent)(int, TigMsg*) =
			replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x101B0110,
					[](int widId, TigMsg *msg) {
						auto &charEdSelPkt = uiCharEditor.GetCharEditorSelPacket();
						auto pre = charEdSelPkt.statBeingRaised;
						auto result = orgStatBtnEvent(widId, msg);
						auto post = charEdSelPkt.statBeingRaised;

						// if we changed to/from intelligence, reset subsequent steps
						if (pre != post &&
								(pre == stat_intelligence || post == stat_intelligence)) {
							temple::GetRef<void(__cdecl)(int)>(0x10143FF0)(1);
						}
						return result;
					});

		// Skill

		// Hook for SkillIncreaseBtnMsg to raise 4 times when ctrl/alt is pressed
		static BOOL(__cdecl*orgSkillIncBtnMsg)(int, TigMsg*) = replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x101ABFA0, [](int widId, TigMsg* msg) {
			if (msg->type != TigMsgType::WIDGET)
				return FALSE;
			auto widMsg = (TigMsgWidget*)msg;
			if (widMsg->widgetEventType != TigMsgWidgetEvent::MouseReleased)
				return FALSE;

			if (infrastructure::gKeyboard.IsKeyPressed(VK_CONTROL) || infrastructure::gKeyboard.IsKeyPressed(VK_LCONTROL)
				|| infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) || infrastructure::gKeyboard.IsKeyPressed(VK_RMENU)) {
				orgSkillIncBtnMsg(widId, msg);
				int safetyCounter = 3;
				while (uiManager->GetButtonState(widId) != LgcyButtonState::Disabled && safetyCounter >= 0) {
					orgSkillIncBtnMsg(widId, msg);
					safetyCounter--;
				}
				return TRUE;
			};
			return orgSkillIncBtnMsg(widId, msg);
		});


		// Feats
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

KnownSpellInfo::KnownSpellInfo(int SpellEnum, int SpellFlag) :spEnum(SpellEnum), spFlag(SpellFlag){
	spellClass = 0;
};

KnownSpellInfo::KnownSpellInfo(int SpellEnum, int SpellFlag, int SpellClass) :spEnum(SpellEnum), spFlag(SpellFlag), spellClass(spellSys.GetSpellClass(SpellClass)) {
	spellLevel = spellSys.GetSpellLevelBySpellClass(SpellEnum, SpellClass);
}
KnownSpellInfo::KnownSpellInfo(int SpellEnum, int SpellFlag, int SpellClass, int isDomain) : spEnum(SpellEnum), spFlag(SpellFlag), spellClass(spellSys.GetSpellClass(SpellClass, isDomain != 0)) {
	spellLevel = spellSys.GetSpellLevelBySpellClass(SpellEnum, SpellClass);
};

FeatInfo::FeatInfo(std::string & featName) : featEnum(ElfHash::Hash(featName)) {};

objHndl Chargen::GetEditedChar(){
	return temple::GetRef<objHndl>(0x11E741A0);
}

CharEditorSelectionPacket & Chargen::GetCharEditorSelPacket(){
	return temple::GetRef<CharEditorSelectionPacket>(0x11E72F00);
}

std::vector<KnownSpellInfo>& Chargen::GetKnownSpellInfo(){
	return mSpellInfo;
}

std::vector<KnownSpellInfo>& Chargen::GetAvailableSpells(){
	return mAvailableSpells;
}

int * Chargen::GetRolledStats(){
	return temple::GetRef<int[]>(0x10C44C50);
}

int Chargen::GetRolledStatIdx(int x, int y, int * xyOut)
{
	return 0;
}

bool Chargen::IsSelectingFeats()
{
	return uiCharEditor.IsSelectingFeats();
}

void Chargen::SetIsNewChar(bool state)
{
	mIsNewChar = state;
}

bool Chargen::IsNewChar()
{
	return mIsNewChar;
}

bool Chargen::SpellsNeedReset()
{
	return mSpellsNeedReset;
}

void Chargen::SpellsNeedResetSet(bool value){
	mSpellsNeedReset = value;
}

bool Chargen::SpellIsAlreadyKnown(int spEnum, int spellClass){

	auto &knSpInfo = GetKnownSpellInfo();

	for (auto i = 0u; i < knSpInfo.size(); i++) {
		if (knSpInfo[i].spEnum == spEnum
			&& knSpInfo[i].spellClass == spellClass)
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

bool Chargen::SpellIsForbidden(int spEnum, int spellClass){
	if (spellSys.GetCastingClass(spellClass) == stat_level_wizard) {  // Only for wizards
		auto& selPkt = GetCharEditorSelPacket();
		auto handle = GetEditedChar();
		SpellEntry spEntry(spEnum);
		auto spSchool = spEntry.spellSchoolEnum;

		if (spSchool == selPkt.forbiddenSchool1
			|| spSchool == selPkt.forbiddenSchool2)
			return true;
		if (spellSys.IsForbiddenSchool(handle, spSchool))
			return true;
	}
	return false;
}

void Chargen::BonusFeatsClear()
{
	mBonusFeats.clear();
}

void Chargen::SetBonusFeats(std::vector<FeatInfo>& fti)
{
	mBonusFeats.clear();
	for (auto it : fti) {
		mBonusFeats.push_back(it);
	}
}

int Chargen::GetNewLvl(Stat classEnum)
{
	auto handle = GetEditedChar();
	return objects.StatLevelGet(handle, classEnum) + 1;
}

bool Chargen::IsBonusFeatDisregardingPrereqs(feat_enums feat){
	for (auto it : mBonusFeats) {
		if (it.featEnum == feat)
			return (it.flag & FeatInfoFlag::DisregardPrereqs) != 0;
	}

	return false;
}

bool Chargen::IsBonusOnlyFeat(feat_enums feat)
{
	for (auto it : mBonusFeats) {
		if (it.featEnum == feat)
			return (it.flag & FeatInfoFlag::BonusOnly) != 0;
	}

	return false;
}
