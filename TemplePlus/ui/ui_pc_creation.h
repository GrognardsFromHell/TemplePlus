
#pragma once

#include "ui/ui.h"
#include "tig/tig_font.h"
#include "ui_system.h"
#include "widgets/widgets.h"
#include "ui_chargen.h"

struct UiSystemConf;
class CombinedImgFile;

enum ChargenStages : int {
	CG_Stage_Stats = 0,
	CG_Stage_Race,
	CG_Stage_Gender,
	CG_Stage_Height,
	CG_Stage_Hair,
	CG_Stage_Class,
	CG_Stage_Alignment,
	CG_Stage_Deity,
	CG_Stage_Abilities,
	CG_Stage_Feats,
	CG_Stage_Skills,
	CG_Stage_Spells,
	CG_Stage_Portrait,
	CG_Stage_Voice,

	CG_STAGE_COUNT
};

class ChargenSystem {
	friend class PcCreationHooks;
public:
	virtual std::string GetName() = 0;
	virtual void Reset(CharEditorSelectionPacket & charSpec) {};
	virtual void Activate() {};
	virtual BOOL SystemInit(const UiSystemConf *);
	virtual void Free(){};
	virtual BOOL Resize(UiResizeArgs &resizeArgs) { return TRUE; };
	virtual void Hide() { mWnd->Hide(); };
	virtual void Show(){ mWnd->Show();	mWnd->BringToFront();	};
	virtual BOOL CheckComplete() { return TRUE; }; // checks if the char editing stage is complete (thus allowing you to move on to the next stage). This is checked at every render call.
	virtual void Finalize(CharEditorSelectionPacket & charSpec, objHndl & handle){};
	virtual void ButtonExited(){};

	static void UpdateDescriptionBox(); // updates the description of the character being created

	ChargenSystem(){};

protected:
	

	virtual bool WidgetsInit(int w, int h) { return true; };
	std::unique_ptr<WidgetContainer> mWnd;
	eastl::vector<LgcyWidgetId> mBigButtons;

	ChargenStages mStage;
	void MakeStateTitle();
};


class RaceChargen : public ChargenSystem , PagianatedChargenSystem
{
	friend class PcCreationHooks;
public:
	static constexpr auto Name = "chargen_race";
	RaceChargen(const UiSystemConf & conf);

	virtual std::string GetName() { return "Chargen: Race"; }
	
	
	virtual BOOL SystemInit(const UiSystemConf *) override;
	virtual bool WidgetsInit(int w, int h) override;
	virtual void Reset(CharEditorSelectionPacket & charSpec) override;
	virtual BOOL CheckComplete() override; // checks if the char editing stage is complete (thus allowing you to move on to the next stage). This is checked at every render call.
protected:
	eastl::vector<LgcyWidgetId> mSubraceBtns;

	static void SetScrollboxText(Race race);
	static void UpdateScrollbox();
	void UpdateActiveRace();
	void UpdateSubraceButtons(RaceBase raceBase);
};

class ClassChargen : ChargenSystem, PagianatedChargenSystem
{
public:
	virtual std::string GetName() { return "Chargen: Class"; };
	static constexpr auto Name = "chargen_class";

	ClassChargen(const UiSystemConf & conf);
	virtual void Reset(CharEditorSelectionPacket & charSpec) override {

	};
protected:
	
};

class UiPcCreationSys : public UiSystem {
	friend class PcCreationHooks;

public:

	static constexpr auto Name = "pc_creation";
	UiPcCreationSys(const UiSystemConf &config);
	~UiPcCreationSys();
	void ResizeViewport(const UiResizeArgs &resizeArgs) override;
	const std::string &GetName() const override;

	/**
	* Will immediately start a new game if the default party was set.
	*/
	void Start();


	ClassChargen& GetClass() const {
		Expects(!!mClass);
		return *mClass;
	}
	RaceChargen& GetRace() const {
		Expects(!!mRace);
		return *mRace;
	}

	bool IsVisible();

protected:
	std::unique_ptr<ClassChargen> mClass;
	std::unique_ptr<RaceChargen> mRace;

	BOOL &mIsHidden = temple::GetRef<BOOL>(0x102F7BF0);

	template<typename T, typename... TArgs>
	std::unique_ptr<T> InitializeSystem(TArgs&&... args);
};

class UiPcCreation {
	friend class PcCreationHooks;
public:
	objHndl GetEditedChar();
	CharEditorSelectionPacket & GetCharEditorSelPacket();
	LgcyWindow &GetPcCreationWnd();
	LgcyWindow &GetStatsWnd();
	int &GetState();
	int &GetStatesComplete();
	Alignment GetPartyAlignment();

	void PrepareNextStages();
	void BtnStatesUpdate(int systemId);
	void ResetNextStages(int systemId); // activates the reset callback for all the subsequent stages
	void ToggleClassRelatedStages(); // Spell Selection and Class Features

#pragma region Systems

	// Height
	void HeightShow();

	// Class
	BOOL ClassSystemInit(UiSystemConf & conf);
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
	BOOL FeatsSystemInit(UiSystemConf & conf);
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
	BOOL SpellsSystemInit(UiSystemConf & conf);
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
	BOOL FinishBtnMsg(int widId, TigMsg* msg); // goes after the original FinishBtnMsg
	void MainWndRender(int id);
	BOOL MainWndMsg(int id, TigMsg* msg);

	
	void RenderCharInfos(int widId);
	void RenderCharStatTexts(int stat, int widId); // Labels
	void RenderCharStats(int stat, int widId); // Stat levels (modified by race) + modifier
	void RenderCharDimensions(int widId); // Height, Weight
	void RenderCharExpLvl(int widId); // XP, Level
	void RenderCharSavingThrows(int widId); // Saving Throws
	void RenderCharHpAc(int widId); // HP AC
	void RenderCharMovementInit(int widId); // Movement speed, Initiative
	void RenderCharToHitBonus(int widId); // Melee, Ranged To Hit Bonuses
	void DrawTextInWidgetCentered(int widgetId, const string & text, const TigRect & rect, const TigTextStyle & style);

	// stats
	int GetRolledStatIdx(int x, int y, int *xyOut = nullptr); // gets the index of the Rolled Stats button according to the mouse position. Returns -1 if none.
	int GetAssignedStatIdx(int x, int y, int *xyOut = nullptr); // gets the index of the Assigned Stats button according to the mouse position. Returns -1 if none.
	BOOL StatsWndMsg(int widId, TigMsg *msg);

	// Race
	void RaceWndRender(int widId);

	// Gender
	void GenderFinalize(CharEditorSelectionPacket& selPkt, objHndl& handle);

	// Hair
	void HairUpdate();
	void HairUpdateStyleBtnTextures();

	// class
	void ClassBtnRender(int widId);
	BOOL ClassBtnMsg(int widId, TigMsg* msg);
	BOOL ClassNextBtnMsg(int widId, TigMsg* msg);
	BOOL ClassPrevBtnMsg(int widId, TigMsg* msg);
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
	int raceWndPage = 0;
	eastl::vector<int> raceBtnMapping; // used as an index of choosable character classes
	int GetRaceWndPage();

	eastl::vector<int> classBtnMapping; // used as an index of choosable character classes
	int GetClassWndPage();
	Stat GetClassCodeFromWidgetAndPage(int idx, int page);

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
	void ButtonEnteredHandler(const std::string&);
	int GetNewLvl(Stat classEnum = stat_level);
	int GetProtoIdByRaceGender(Race race, int genderId);// get protos.tab id by race and gender


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
	int raceWndId = 0;
	eastl::vector<int> raceBtnIds;
	int raceNextBtn = 0, racePrevBtn = 0;

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
	int raceBox = 0; // the 7 race buttons background
	ColorRect blackColorRect = ColorRect(0xFF000000);
	ColorRect whiteColorRect = ColorRect(0xFFFFffff);
	ColorRect blueColorRect = ColorRect(0xFF0000ff);
	ColorRect darkGreenColorRect = ColorRect(0xFF006003);
	ColorRect classBtnShadowColor = ColorRect(0xFF000000);
	ColorRect classBtnColorRect = ColorRect(0xFFFFffff);
	TigTextStyle whiteTextGenericStyle;
	TigTextStyle blackTextGenericStyle;
	TigTextStyle blueTextStyle;
	TigTextStyle bigBtnTextStyle; // text style for stuff like Race / Class buttons
	TigTextStyle featsGreyedStyle, featsBonusTextStyle, featsNormalTextStyle, featsExistingTitleStyle, featsGoldenStyle, featsClassStyle, featsCenteredStyle;
	TigTextStyle spellsTextStyle;
	TigTextStyle spellsTitleStyle;
	TigTextStyle spellLevelLabelStyle;
	TigTextStyle spellsAvailBtnStyle;
	TigTextStyle spellsPerDayStyle;
	TigTextStyle spellsPerDayTitleStyle;
	CombinedImgFile* levelupSpellbar, *featsbackdrop;


	UiPcCreation() {
		TigTextStyle baseStyle;
		baseStyle.flags = 0x4000;
		baseStyle.field2c = -1;
		baseStyle.shadowColor = &blackColorRect;
		baseStyle.field0 = 0;
		baseStyle.kerning = 1;
		baseStyle.leading = 0;
		baseStyle.tracking = 3;
		baseStyle.textColor = baseStyle.colors2 = baseStyle.colors4 = &whiteColorRect;
		whiteTextGenericStyle = baseStyle;

		blueTextStyle = baseStyle;
		blueTextStyle.colors4 = blueTextStyle.colors2 = blueTextStyle.textColor = &blueColorRect;

		
		blackTextGenericStyle = baseStyle;
		blackTextGenericStyle.flags = 0;
		blackTextGenericStyle.colors4 = blackTextGenericStyle.colors2 = blackTextGenericStyle.textColor = &blackColorRect;
		blackTextGenericStyle.shadowColor = &whiteColorRect;

		bigBtnTextStyle.flags = 8;
		bigBtnTextStyle.field2c = -1;
		bigBtnTextStyle.textColor = &classBtnColorRect;
		bigBtnTextStyle.shadowColor = &classBtnShadowColor;
		bigBtnTextStyle.colors4 = &classBtnColorRect;
		bigBtnTextStyle.colors2 = &classBtnColorRect;
		bigBtnTextStyle.field0 = 0;
		bigBtnTextStyle.kerning = 1;
		bigBtnTextStyle.leading = 0;
		bigBtnTextStyle.tracking = 3;
	}

private:
	int GetStatIdx(const int x0, const int xMax, int x, int y, int *xyOut);

	int mPageCount = 0;

	bool mFeatsActivated = false;
	bool mIsSelectingBonusFeat = false;
	bool mBonusFeatOk = false;
	feat_enums featsMultiSelected = FEAT_NONE, mFeatsMultiMasterFeat = FEAT_NONE;

	std::vector<FeatInfo> mExistingFeats, mSelectableFeats, mMultiSelectFeats, mMultiSelectMasterFeats;

	int &GetDeityBtnId(int deityId);

};

int __cdecl PcCreationFeatUiPrereqCheck(feat_enums feat);

extern UiPcCreation uiPcCreation;
