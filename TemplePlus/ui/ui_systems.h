
#pragma once

#include <memory>

struct UiResizeArgs;

class UiSystem;
class UiMainMenu;
class UiMM;
class UiLoadGame;
class UiSaveGame;
class UiInGame;
class UiInGameSelect;
class UiRadialMenu;
class UiTurnBased;
class UiAnim;
class UiTB;
class UiWMapRnd;
class UiCombat;
class UiSlide;
class UiDlg;
class UiPcCreationSys;
class UiChar;
class UiToolTip;
class UiLogbook;
class UiScrollpane;
class UiTownmap;
class UiPopup;
class UiTextDialog;
class UiFocusManager;
class UiWorldmap;
class UiRandomEncounter;
class UiHelp;
class UiItemCreation;
class UiSkillMastery;
class UiUtilityBar;
class UiDM;
class UiTrack;
class UiPartyPool;
class UiPccPortrait;
class UiParty;
class UiFormation;
class UiCamping;
class UiHelpInventory;
class UiPartyQuickview;
class UiOptions;
class UiKeyManager;
class UiHelpManager;
class UiSlider;
class UiWritten;
class UiCharmap;
class UiPython;

class UiSystems {
public:
	UiSystems(int width, int height);
	~UiSystems();

	UiMainMenu& GetMainMenu() const {
		Expects(!!mMainMenu);
		return *mMainMenu;
	}
	UiMM& GetMM() const {
		Expects(!!mMM);
		return *mMM;
	}
	UiLoadGame& GetLoadGame() const {
		Expects(!!mLoadGame);
		return *mLoadGame;
	}
	UiSaveGame& GetSaveGame() const {
		Expects(!!mSaveGame);
		return *mSaveGame;
	}
	UiInGame& GetInGame() const {
		Expects(!!mInGame);
		return *mInGame;
	}
	UiInGameSelect& GetInGameSelect() const {
		Expects(!!mInGameSelect);
		return *mInGameSelect;
	}
	UiRadialMenu& GetRadialMenu() const {
		Expects(!!mRadialMenu);
		return *mRadialMenu;
	}
	UiTurnBased& GetTurnBased() const {
		Expects(!!mTurnBased);
		return *mTurnBased;
	}
	UiAnim& GetAnim() const {
		Expects(!!mAnim);
		return *mAnim;
	}
	UiTB& GetTB() const {
		Expects(!!mTB);
		return *mTB;
	}
	UiWMapRnd& GetWMapRnd() const {
		Expects(!!mWMapRnd);
		return *mWMapRnd;
	}
	UiCombat& GetCombat() const {
		Expects(!!mCombat);
		return *mCombat;
	}
	UiSlide& GetSlide() const {
		Expects(!!mSlide);
		return *mSlide;
	}
	UiDlg& GetDlg() const {
		Expects(!!mDlg);
		return *mDlg;
	}
	UiPcCreationSys& GetPcCreation() const {
		Expects(!!mPcCreation);
		return *mPcCreation;
	}
	UiChar& GetChar() const {
		Expects(!!mChar);
		return *mChar;
	}
	UiToolTip& GetToolTip() const {
		Expects(!!mToolTip);
		return *mToolTip;
	}
	UiLogbook& GetLogbook() const {
		Expects(!!mLogbook);
		return *mLogbook;
	}
	UiScrollpane& GetScrollpane() const {
		Expects(!!mScrollpane);
		return *mScrollpane;
	}
	UiTownmap& GetTownmap() const {
		Expects(!!mTownmap);
		return *mTownmap;
	}
	UiPopup& GetPopup() const {
		Expects(!!mPopup);
		return *mPopup;
	}
	UiTextDialog& GetTextDialog() const {
		Expects(!!mTextDialog);
		return *mTextDialog;
	}
	UiFocusManager& GetFocusManager() const {
		Expects(!!mFocusManager);
		return *mFocusManager;
	}
	UiWorldmap& GetWorldmap() const {
		Expects(!!mWorldmap);
		return *mWorldmap;
	}
	UiRandomEncounter& GetRandomEncounter() const {
		Expects(!!mRandomEncounter);
		return *mRandomEncounter;
	}
	UiHelp& GetHelp() const {
		Expects(!!mHelp);
		return *mHelp;
	}
	UiItemCreation& GetItemCreation() const {
		Expects(!!mItemCreation);
		return *mItemCreation;
	}
	UiSkillMastery& GetSkillMastery() const {
		Expects(!!mSkillMastery);
		return *mSkillMastery;
	}
	UiUtilityBar& GetUtilityBar() const {
		Expects(!!mUtilityBar);
		return *mUtilityBar;
	}
	UiDM & GetDM() const {
		Expects(!!mDungeonMaster);
		return *mDungeonMaster;
	}
	UiTrack& GetTrack() const {
		Expects(!!mTrack);
		return *mTrack;
	}
	UiPartyPool& GetPartyPool() const {
		Expects(!!mPartyPool);
		return *mPartyPool;
	}
	UiPccPortrait& GetPccPortrait() const {
		Expects(!!mPccPortrait);
		return *mPccPortrait;
	}
	UiParty& GetParty() const {
		Expects(!!mParty);
		return *mParty;
	}
	UiFormation& GetFormation() const {
		Expects(!!mFormation);
		return *mFormation;
	}
	UiCamping& GetCamping() const {
		Expects(!!mCamping);
		return *mCamping;
	}
	UiHelpInventory& GetHelpInventory() const {
		Expects(!!mHelpInventory);
		return *mHelpInventory;
	}
	UiPartyQuickview& GetPartyQuickview() const {
		Expects(!!mPartyQuickview);
		return *mPartyQuickview;
	}
	UiOptions& GetOptions() const {
		Expects(!!mOptions);
		return *mOptions;
	}
	UiKeyManager& GetManager() const {
		Expects(!!mManager);
		return *mManager;
	}
	UiHelpManager& GetHelpManager() const {
		Expects(!!mHelpManager);
		return *mHelpManager;
	}
	UiSlider& GetSlider() const {
		Expects(!!mSlider);
		return *mSlider;
	}
	UiWritten& GetWritten() const {
		Expects(!!mWritten);
		return *mWritten;
	}
	UiCharmap& GetCharmap() const {
		Expects(!!mCharmap);
		return *mCharmap;
	}

	UiPython& GetPython() const {
		Expects(!!mPython);
		return *mPython;
	}

	void Reset();

	void ResizeViewport(int width, int height);

	void LoadModule();

	void UnloadModule();

private:

	template<typename T, typename... TArgs>
	std::unique_ptr<T> InitializeSystem(TArgs&&... args);

	std::vector<class SaveGameAwareUiSystem*> mSaveGameSystems;

	std::vector<UiSystem*> mLoadedSystems;

	std::unique_ptr<UiMainMenu> mMainMenu;
	std::unique_ptr<UiMM> mMM;
	std::unique_ptr<UiLoadGame> mLoadGame;
	std::unique_ptr<UiSaveGame> mSaveGame;
	std::unique_ptr<UiInGame> mInGame;
	std::unique_ptr<UiInGameSelect> mInGameSelect;
	std::unique_ptr<UiRadialMenu> mRadialMenu;
	std::unique_ptr<UiTurnBased> mTurnBased;
	std::unique_ptr<UiAnim> mAnim;
	std::unique_ptr<UiTB> mTB;
	std::unique_ptr<UiWMapRnd> mWMapRnd;
	std::unique_ptr<UiCombat> mCombat;
	std::unique_ptr<UiSlide> mSlide;
	std::unique_ptr<UiDlg> mDlg;
	std::unique_ptr<UiPcCreationSys> mPcCreation;
	std::unique_ptr<UiChar> mChar;
	std::unique_ptr<UiToolTip> mToolTip;
	std::unique_ptr<UiLogbook> mLogbook;
	std::unique_ptr<UiScrollpane> mScrollpane;
	std::unique_ptr<UiTownmap> mTownmap;
	std::unique_ptr<UiPopup> mPopup;
	std::unique_ptr<UiTextDialog> mTextDialog;
	std::unique_ptr<UiFocusManager> mFocusManager;
	std::unique_ptr<UiWorldmap> mWorldmap;
	std::unique_ptr<UiRandomEncounter> mRandomEncounter;
	std::unique_ptr<UiHelp> mHelp;
	std::unique_ptr<UiItemCreation> mItemCreation;
	std::unique_ptr<UiSkillMastery> mSkillMastery;
	std::unique_ptr<UiUtilityBar> mUtilityBar;
	std::unique_ptr<UiDM> mDungeonMaster;
	std::unique_ptr<UiTrack> mTrack;
	std::unique_ptr<UiPartyPool> mPartyPool;
	std::unique_ptr<UiPccPortrait> mPccPortrait;
	std::unique_ptr<UiParty> mParty;
	std::unique_ptr<UiFormation> mFormation;
	std::unique_ptr<UiCamping> mCamping;
	std::unique_ptr<UiHelpInventory> mHelpInventory;
	std::unique_ptr<UiPartyQuickview> mPartyQuickview;
	std::unique_ptr<UiOptions> mOptions;
	std::unique_ptr<UiKeyManager> mManager;
	std::unique_ptr<UiHelpManager> mHelpManager;
	std::unique_ptr<UiSlider> mSlider;
	std::unique_ptr<UiWritten> mWritten;
	std::unique_ptr<UiCharmap> mCharmap;
	std::unique_ptr<UiPython> mPython;
};

/*
  Utility class to load and unload the module in the UI system using RAII.
*/
class UiModuleLoader {
public:
	explicit UiModuleLoader(UiSystems &ui) : mUi(ui) {
		mUi.LoadModule();
	}
	~UiModuleLoader() {
		mUi.UnloadModule();
	}
private:
	UiSystems &mUi;
};

extern UiSystems* uiSystems;
