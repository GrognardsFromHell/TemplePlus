
#include "stdafx.h"

#include "ui_systems.h"
#include "ui_legacysystems.h"
#include "ui_turn_based.h"
#include "ui_mainmenu.h"
#include "ui_worldmap.h"
#include "ui_pc_creation.h"
#include "ui_char.h"
#include "ui_townmap.h"
#include "ui_dialog.h"
#include "ui_python.h"

UiSystems* uiSystems = nullptr;

UiSystems::UiSystems(int width, int height)
{
	UiSystemConf config;
	config.width = width;
	config.height = height;

	mMainMenu = InitializeSystem<UiMainMenu>(config);
	mMM = InitializeSystem<UiMM>(config);
	mLoadGame = InitializeSystem<UiLoadGame>(config);
	mSaveGame = InitializeSystem<UiSaveGame>(config);
	mInGame = InitializeSystem<UiInGame>(config);
	mInGameSelect = InitializeSystem<UiInGameSelect>(config);
	mRadialMenu = InitializeSystem<UiRadialMenu>(config);
	mTurnBased = InitializeSystem<UiTurnBased>(config.width, config.height);
	mAnim = InitializeSystem<UiAnim>(config);
	mTB = InitializeSystem<UiTB>(config);
	mWMapRnd = InitializeSystem<UiWMapRnd>();
	mCombat = InitializeSystem<UiCombat>(config);
	mSlide = InitializeSystem<UiSlide>();
	mDlg = InitializeSystem<UiDlg>(config);
	mPcCreation = InitializeSystem<UiPcCreationSys>(config);
	mChar = InitializeSystem<UiChar>(config);
	mToolTip = InitializeSystem<UiToolTip>(config);
	mLogbook = InitializeSystem<UiLogbook>(config);
	mScrollpane = InitializeSystem<UiScrollpane>(config);
	mTownmap = InitializeSystem<UiTownmap>(config);
	mPopup = InitializeSystem<UiPopup>(config);
	mTextDialog = InitializeSystem<UiTextDialog>(config);
	mFocusManager = InitializeSystem<UiFocusManager>(config);
	mWorldmap = InitializeSystem<UiWorldmap>(config.width, config.height);
	mRandomEncounter = InitializeSystem<UiRandomEncounter>(config);
	mHelp = InitializeSystem<UiHelp>(config);
	mItemCreation = InitializeSystem<UiItemCreation>(config);
	mSkillMastery = InitializeSystem<UiSkillMastery>(config);
	mUtilityBar = InitializeSystem<UiUtilityBar>(config);
	mDungeonMaster = InitializeSystem<UiDM>(config);
	mTrack = InitializeSystem<UiTrack>(config);
	mPartyPool = InitializeSystem<UiPartyPool>(config);
	mPccPortrait = InitializeSystem<UiPccPortrait>(config);
	mParty = InitializeSystem<UiParty>(config);
	mFormation = InitializeSystem<UiFormation>(config);
	mCamping = InitializeSystem<UiCamping>(config);
	mHelpInventory = InitializeSystem<UiHelpInventory>(config);
	mPartyQuickview = InitializeSystem<UiPartyQuickview>();
	mOptions = InitializeSystem<UiOptions>(config);
	mManager = InitializeSystem<UiKeyManager>(config);
	mHelpManager = InitializeSystem<UiHelpManager>(config);
	mSlider = InitializeSystem<UiSlider>(config);
	mWritten = InitializeSystem<UiWritten>(config);
	mCharmap = InitializeSystem<UiCharmap>(config);
	mPython = InitializeSystem<UiPython>(config);

	if (!uiSystems) {
		uiSystems = this;
	}
}

UiSystems::~UiSystems()
{
	if (uiSystems == this) {
		uiSystems = nullptr;
	}

	mCharmap.reset();
	mWritten.reset();
	mSlider.reset();
	mHelpManager.reset();
	mManager.reset();
	mOptions.reset();
	mPartyQuickview.reset();
	mHelpInventory.reset();
	mCamping.reset();
	mFormation.reset();
	mParty.reset();
	mPccPortrait.reset();
	mPartyPool.reset();
	mTrack.reset();
	mUtilityBar.reset();
	mSkillMastery.reset();
	mItemCreation.reset();
	mHelp.reset();
	mRandomEncounter.reset();
	mWorldmap.reset();
	mFocusManager.reset();
	mTextDialog.reset();
	mPopup.reset();
	mTownmap.reset();
	mScrollpane.reset();
	mLogbook.reset();
	mToolTip.reset();
	mChar.reset();
	mPcCreation.reset();
	mDlg.reset();
	mSlide.reset();
	mCombat.reset();
	mWMapRnd.reset();
	mTB.reset();
	mAnim.reset();
	mTurnBased.reset();
	mRadialMenu.reset();
	mInGameSelect.reset();
	mInGame.reset();
	mSaveGame.reset();
	mLoadGame.reset();
	mMM.reset();
	mMainMenu.reset();
	mPython.reset();
}

void UiSystems::Reset()
{
	for (auto system : mLoadedSystems) {
		system->Reset();
	}
}

void UiSystems::ResizeViewport(int w, int h)
{
	UiResizeArgs arg;
	memset(&arg, 0, sizeof(arg));
	arg.rect1.width = w;
	arg.rect1.height = h;
	arg.rect2.width = w;
	arg.rect2.height = h;

	for (auto system : mLoadedSystems) {
		system->ResizeViewport(arg);
	}
}

void UiSystems::LoadModule()
{
	for (auto system : mLoadedSystems) {
		system->LoadModule();
	}
}

void UiSystems::UnloadModule()
{
	for (auto it = mLoadedSystems.rbegin(); it != mLoadedSystems.rend(); it++) {
		(*it)->UnloadModule();
	}
}

template <typename Type, typename... Args>
std::unique_ptr<Type> UiSystems::InitializeSystem(Args&&... args) {
	logger->info("Loading UI system {}", Type::Name);
	
	auto result(std::make_unique<Type>(std::forward<Args>(args)...));

	mLoadedSystems.push_back(result.get());

	if (std::is_convertible<Type*, SaveGameAwareUiSystem*>()) {
		mSaveGameSystems.push_back((SaveGameAwareUiSystem*)(result.get()));
	}

	return std::move(result);
}
