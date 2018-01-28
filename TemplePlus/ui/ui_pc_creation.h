
#pragma once

#include "ui/ui.h"
#include "tig/tig_font.h"

class CombinedImgFile;

class UiPcCreation {
	friend class PcCreationHooks;

public:
	objHndl GetEditedChar();
	CharEditorSelectionPacket & GetCharEditorSelPacket();
	LgcyWindow &GetPcCreationWnd();
	LgcyWindow &GetStatsWnd();
	int &GetState();
	Alignment GetPartyAlignment();

	void PrepareNextStages();
	void BtnStatesUpdate(int systemId);
	void ResetNextStages(int systemId); // activates the reset callback for all the subsequent stages
	void ToggleClassRelatedStages(); // Spell Selection and Class Features

#pragma region Systems
									 // Class
	BOOL ClassSystemInit(GameSystemConf & conf);
	BOOL ClassWidgetsInit();
	void ClassWidgetsFree();
	BOOL ClassShow();
	BOOL ClassHide();
	BOOL ClassWidgetsResize(UiResizeArgs & args);
	BOOL ClassCheckComplete();
	void ClassBtnEntered();
	void ClassActivate();
	void ClassFinalize(CharEditorSelectionPacket & selPkt, objHndl & handle);

	// Feats
	BOOL FeatsSystemInit(GameSystemConf & conf);
	BOOL FeatsWidgetsInit(int w, int h);
	void FeatsFree();
	void FeatWidgetsFree();
	BOOL FeatsWidgetsResize(UiResizeArgs &args);
	BOOL FeatsShow();
	BOOL FeatsHide();
	void FeatsActivate();
	BOOL FeatsCheckComplete();
	void FeatsFinalize(CharEditorSelectionPacket& selPkt, objHndl & handle);
	void FeatsReset(CharEditorSelectionPacket& selPkt);

	// Spells
	BOOL SpellsSystemInit(GameSystemConf & conf);
	void SpellsFree();
	BOOL SpellsWidgetsInit();
	void SpellsReset();
	void SpellsWidgetsFree();
	BOOL SpellsShow();
	BOOL SpellsHide();
	BOOL SpellsWidgetsResize(UiResizeArgs &args);
	void SpellsActivate();
	BOOL SpellsCheckComplete();
	void SpellsFinalize();
	void SpellsReset(CharEditorSelectionPacket& selPkt);
#pragma endregion

#pragma region Widget callbacks
	void StateTitleRender(int widId);

	// stats
	int GetRolledStatIdx(int x, int y, int *xyOut = nullptr); // gets the index of the Rolled Stats button according to the mouse position. Returns -1 if none.
	BOOL StatsWndMsg(int widId, TigMsg *msg);

	// class
	void ClassBtnRender(int widId);
	BOOL ClassBtnMsg(int widId, TigMsg* msg);
	BOOL ClassNextBtnMsg(int widId, TigMsg* msg);
	BOOL ClassPrevBtnMsg(int widId, TigMsg* msg);
	BOOL FinishBtnMsg(int widId, TigMsg* msg); // goes after the original FinishBtnMsg
	void ClassNextBtnRender(int widId);
	void ClassPrevBtnRender(int widId);

	// Feats
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

	// spells
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

#pragma region logic 
	// class
	void ClassSetPermissibles();

	// feats
	bool IsSelectingNormalFeat(); // the normal feat you get every 3rd level in 3.5ed
	bool IsSelectingSecondFeat(); // currently racial bonus for humans
	bool IsSelectingBonusFeat(); // selecting a class bonus feat

								 // deity
	void DeitySetPermissibles();
#pragma endregion 

	// utilities
	bool IsCastingStatSufficient(Stat classEnum);
	bool IsAlignmentOk(Stat classEnums); // checks if class is compatible with the selected party alignment
	void ClassScrollboxTextSet(Stat classEnum); // sets the chargen textbox to the class's short description from stat.mes
	void ButtonEnteredHandler(int helpId);
	int GetNewLvl(Stat classEnum = stat_level);


	// Feat Utilities
	std::string GetFeatName(feat_enums feat); // includes strings for Mutli-selection feat categories e.g. FEAT_WEAPON_FOCUS
	TigTextStyle & GetFeatStyle(feat_enums feat, bool allowMultiple = true);
	bool FeatAlreadyPicked(feat_enums feat);
	bool FeatCanPick(feat_enums feat);
	bool IsSelectingRangerSpec();
	bool IsClassBonusFeat(feat_enums feat);
	bool IsBonusFeatDisregardingPrereqs(feat_enums feat);

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
	TigRect featsSelectedBorderRect, featsSelected2BorderRect, featsClassBonusBorderRect, feat0TextRect, feat1TextRect, feat2TextRect;

	int featsMainWndId = 0, featsMultiSelectWndId = 0;
	int featsScrollbarId = 0, featsExistingScrollbarId = 0, featsMultiSelectScrollbarId = 0;
	int featsScrollbarY = 0, featsExistingScrollbarY = 0, featsMultiSelectScrollbarY = 0;
	LgcyWindow featsMainWnd, featsMultiSelectWnd;
	LgcyScrollBar featsScrollbar, featsExistingScrollbar, featsMultiSelectScrollbar;
	eastl::vector<int> featsAvailBtnIds, featsExistingBtnIds, featsMultiSelectBtnIds;
	int featsMultiOkBtnId = 0, featsMultiCancelBtnId = 0;
	const int FEATS_AVAIL_BTN_COUNT = 15; // vanilla 15
	const int FEATS_AVAIL_BTN_HEIGHT = 12; // vanilla 11
	const int FEATS_EXISTING_BTN_COUNT = 7; // vanilla 8
	const int FEATS_EXISTING_BTN_HEIGHT = 13; // vanilla 12
	const int FEATS_MULTI_BTN_COUNT = 15;
	const int FEATS_MULTI_BTN_HEIGHT = 12;
	std::string featsAvailTitleString, featsExistingTitleString;
	std::string featsTitleString;
	std::string featsClassBonusTitleString;

	const int DEITY_BTN_COUNT = 20;

	int spellsWndId = 0;
	LgcyWindow spellsWnd;
	LgcyScrollBar spellsScrollbar, spellsScrollbar2;
	int spellsScrollbarId = 0, spellsScrollbar2Id = 0;
	int spellsScrollbarY = 0, spellsScrollbar2Y = 0;
	eastl::vector<int> spellsAvailBtnIds, spellsChosenBtnIds;
	const int SPELLS_BTN_COUNT = 11; // vanilla had 12, decreasing this to increase the font
	const int SPELLS_BTN_HEIGHT = 13; // vanilla was 12 (so 13*11 = 143 ~= 144 vanilla)
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
	eastl::vector<TigRect> spellsPerDayBorderRects;
	eastl::vector<TigRect> spellsLevelLabelRects;


	// art assets
	int buttonBox = 0;
	ColorRect genericShadowColor = ColorRect(0xFF000000);
	ColorRect whiteColorRect = ColorRect(0xFFFFffff);
	ColorRect blueColorRect = ColorRect(0xFF0000ff);
	ColorRect darkGreenColorRect = ColorRect(0xFF006003);
	ColorRect classBtnShadowColor = ColorRect(0xFF000000);
	ColorRect classBtnColorRect = ColorRect(0xFFFFffff);
	TigTextStyle whiteTextGenericStyle;
	TigTextStyle blueTextStyle;
	TigTextStyle classBtnTextStyle;
	TigTextStyle featsGreyedStyle, featsBonusTextStyle, featsNormalTextStyle, featsExistingTitleStyle, featsGoldenStyle, featsClassStyle, featsCenteredStyle;
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

	UiPcCreation() {
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

	int &GetDeityBtnId(int deityId);

};

int __cdecl PcCreationFeatUiPrereqCheck(feat_enums feat);

extern UiPcCreation uiPcCreation;
