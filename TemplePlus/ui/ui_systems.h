
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
		assert(!!mMainMenu);
		return *mMainMenu;
	}
	UiMM& GetMM() const {
		assert(!!mMM);
		return *mMM;
	}
	UiLoadGame& GetLoadGame() const {
		assert(!!mLoadGame);
		return *mLoadGame;
	}
	UiSaveGame& GetSaveGame() const {
		assert(!!mSaveGame);
		return *mSaveGame;
	}
	UiInGame& GetInGame() const {
		assert(!!mInGame);
		return *mInGame;
	}
	UiInGameSelect& GetInGameSelect() const {
		assert(!!mInGameSelect);
		return *mInGameSelect;
	}
	UiRadialMenu& GetRadialMenu() const {
		assert(!!mRadialMenu);
		return *mRadialMenu;
	}
	UiTurnBased& GetTurnBased() const {
		assert(!!mTurnBased);
		return *mTurnBased;
	}
	UiAnim& GetAnim() const {
		assert(!!mAnim);
		return *mAnim;
	}
	UiTB& GetTB() const {
		assert(!!mTB);
		return *mTB;
	}
	UiWMapRnd& GetWMapRnd() const {
		assert(!!mWMapRnd);
		return *mWMapRnd;
	}
	UiCombat& GetCombat() const {
		assert(!!mCombat);
		return *mCombat;
	}
	UiSlide& GetSlide() const {
		assert(!!mSlide);
		return *mSlide;
	}
	UiDlg& GetDlg() const {
		assert(!!mDlg);
		return *mDlg;
	}
	UiPcCreationSys& GetPcCreation() const {
		assert(!!mPcCreation);
		return *mPcCreation;
	}
	UiChar& GetChar() const {
		assert(!!mChar);
		return *mChar;
	}
	UiToolTip& GetToolTip() const {
		assert(!!mToolTip);
		return *mToolTip;
	}
	UiLogbook& GetLogbook() const {
		assert(!!mLogbook);
		return *mLogbook;
	}
	UiScrollpane& GetScrollpane() const {
		assert(!!mScrollpane);
		return *mScrollpane;
	}
	UiTownmap& GetTownmap() const {
		assert(!!mTownmap);
		return *mTownmap;
	}
	UiPopup& GetPopup() const {
		assert(!!mPopup);
		return *mPopup;
	}
	UiTextDialog& GetTextDialog() const {
		assert(!!mTextDialog);
		return *mTextDialog;
	}
	UiFocusManager& GetFocusManager() const {
		assert(!!mFocusManager);
		return *mFocusManager;
	}
	UiWorldmap& GetWorldmap() const {
		assert(!!mWorldmap);
		return *mWorldmap;
	}
	UiRandomEncounter& GetRandomEncounter() const {
		assert(!!mRandomEncounter);
		return *mRandomEncounter;
	}
	UiHelp& GetHelp() const {
		assert(!!mHelp);
		return *mHelp;
	}
	UiItemCreation& GetItemCreation() const {
		assert(!!mItemCreation);
		return *mItemCreation;
	}
	UiSkillMastery& GetSkillMastery() const {
		assert(!!mSkillMastery);
		return *mSkillMastery;
	}
	UiUtilityBar& GetUtilityBar() const {
		assert(!!mUtilityBar);
		return *mUtilityBar;
	}
	UiDM & GetDM() const {
		assert(!!mDungeonMaster);
		return *mDungeonMaster;
	}
	UiTrack& GetTrack() const {
		assert(!!mTrack);
		return *mTrack;
	}
	UiPartyPool& GetPartyPool() const {
		assert(!!mPartyPool);
		return *mPartyPool;
	}
	UiPccPortrait& GetPccPortrait() const {
		assert(!!mPccPortrait);
		return *mPccPortrait;
	}
	UiParty& GetParty() const {
		assert(!!mParty);
		return *mParty;
	}
	UiFormation& GetFormation() const {
		assert(!!mFormation);
		return *mFormation;
	}
	UiCamping& GetCamping() const {
		assert(!!mCamping);
		return *mCamping;
	}
	UiHelpInventory& GetHelpInventory() const {
		assert(!!mHelpInventory);
		return *mHelpInventory;
	}
	UiPartyQuickview& GetPartyQuickview() const {
		assert(!!mPartyQuickview);
		return *mPartyQuickview;
	}
	UiOptions& GetOptions() const {
		assert(!!mOptions);
		return *mOptions;
	}
	UiKeyManager& GetManager() const {
		assert(!!mManager);
		return *mManager;
	}
	UiHelpManager& GetHelpManager() const {
		assert(!!mHelpManager);
		return *mHelpManager;
	}
	UiSlider& GetSlider() const {
		assert(!!mSlider);
		return *mSlider;
	}
	UiWritten& GetWritten() const {
		assert(!!mWritten);
		return *mWritten;
	}
	UiCharmap& GetCharmap() const {
		assert(!!mCharmap);
		return *mCharmap;
	}

	UiPython& GetPython() const {
		assert(!!mPython);
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
