
#include "stdafx.h"

#include "ui_mainmenu.h"
#include "ui_systems.h"
#include "ui_legacysystems.h"

#include "anim.h"

#include "gamesystems/gamesystems.h"

#include "tig/tig_msg.h"
#include "movies.h"
#include "messages/messagequeue.h"

#include "widgets/widget_content.h"
#include "widgets/widget_doc.h"

//*****************************************************************************
//* MM-UI
//*****************************************************************************

UiMM::UiMM(const UiSystemConf &config) {
	/*auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10117370);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system MM-UI");
	}*/
	
	WidgetDoc widgetDoc(WidgetDoc::Load("templeplus/ui/main_menu.json"));
	mMainWidget = widgetDoc.TakeRootContainer();
	widgetDoc.GetButton("new-game")->SetClickHandler([] {
		logger->info("Hello World!");
	});
	mPagesWidget = widgetDoc.GetWindow("pages");

	mPageWidgets[MainMenuPage::MainMenu] = widgetDoc.GetWindow("page-main-menu");
	mPageWidgets[MainMenuPage::Difficulty] = widgetDoc.GetWindow("page-difficulty");
	mPageWidgets[MainMenuPage::InGameNormal] = widgetDoc.GetWindow("page-ingame-normal");
	mPageWidgets[MainMenuPage::InGameIronman] = widgetDoc.GetWindow("page-ingame-ironman");
	mPageWidgets[MainMenuPage::Options] = widgetDoc.GetWindow("page-options");

	// Wire up buttons on the main menu
	widgetDoc.GetButton("new-game")->SetClickHandler([this]() {
		Show(MainMenuPage::Difficulty);
	});
	widgetDoc.GetButton("load-game")->SetClickHandler([this]() {
		Hide();
		uiSystems->GetLoadGame().Show(true);
	});
	widgetDoc.GetButton("tutorial")->SetClickHandler([this]() {
		/*
		ui_mm_launch_tutorial();
		sub_10111AD0();
		sub_10111240();
		ui_mm_hide();
		ui_party_refresh();
		*/
	});
	widgetDoc.GetButton("options")->SetClickHandler([this]() {
		Show(MainMenuPage::Options);
	});
	widgetDoc.GetButton("quit-game")->SetClickHandler([this]() {
		Message msg;
		msg.type = TigMsgType::EXIT;
		msg.arg1 = 0;
		messageQueue->Enqueue(msg);
	});

	// Wire up buttons on the difficulty selection page
	widgetDoc.GetButton("difficulty-normal")->SetClickHandler([this]() {
		gameSystems->SetIronman(false);
		Hide();
		uiSystems->GetPcCreation().Start();
	});
	widgetDoc.GetButton("difficulty-ironman")->SetClickHandler([this]() {
		gameSystems->SetIronman(true);
		Hide();
		uiSystems->GetPcCreation().Start();
	});
	widgetDoc.GetButton("difficulty-exit")->SetClickHandler([this]() {
		Show(MainMenuPage::MainMenu);
	});

	// Wire up buttons on the ingame menu (normal difficulty)
	widgetDoc.GetButton("ingame-normal-load")->SetClickHandler([this]() {
		Hide();
		uiSystems->GetLoadGame().Show(false);
	});
	widgetDoc.GetButton("ingame-normal-save")->SetClickHandler([this]() {
		Hide();
		uiSystems->GetSaveGame().Show(true);
	});
	widgetDoc.GetButton("ingame-normal-close")->SetClickHandler([this]() {
		Hide();
	});
	widgetDoc.GetButton("ingame-normal-quit")->SetClickHandler([this]() {
		Hide();
		gameSystems->ResetGame();
		uiSystems->Reset();
		Show(MainMenuPage::MainMenu);
	});

	// Wire up buttons on the ingame menu (ironman difficulty)
	widgetDoc.GetButton("ingame-ironman-close")->SetClickHandler([this]() {
		Hide();
	});
	widgetDoc.GetButton("ingame-ironman-save-quit")->SetClickHandler([this]() {
		if (gameSystems->SaveGameIronman()) {
			gameSystems->ResetGame();
			uiSystems->Reset();
			Show(MainMenuPage::MainMenu);
		}
	});

	// Wire up buttons on the ingame menu (ironman difficulty)
	widgetDoc.GetButton("options-show")->SetClickHandler([this]() {
		Hide();
		uiSystems->GetOptions().Show(true);
	});
	widgetDoc.GetButton("options-view-cinematics")->SetClickHandler([this]() {
		Hide();
		uiSystems->GetUtilityBar().Hide();
		//ui_mm_msg_ui4();
	});	
	widgetDoc.GetButton("options-credits")->SetClickHandler([this]() {
		Hide();

		static std::vector<int> creditsMovies{ 100, 110, 111, 112, 113 };
		for (auto movieId : creditsMovies) {
			movieFuncs.MovieQueueAdd(movieId);
		}		
		movieFuncs.MovieQueuePlay();

		Show(MainMenuPage::Options);
	});
	widgetDoc.GetButton("options-back")->SetClickHandler([this]() {
		Show(MainMenuPage::MainMenu);
	});

	RepositionWidgets(config.width, config.height);
}
UiMM::~UiMM() {
	//auto shutdown = temple::GetPointer<void()>(0x101164c0);
	//shutdown();
}
void UiMM::ResizeViewport(const UiResizeArgs& resizeArg) {
	/*auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x101172c0);
	resize(&resizeArg);*/

	RepositionWidgets(resizeArg.rect1.width, resizeArg.rect1.height);
}
const std::string &UiMM::GetName() const {
	static std::string name("MM-UI");
	return name;
}

bool UiMM::IsVisible() const
{
	// The main menu is defined as visible, if any of the pages is visible
	for (auto &entry : mPageWidgets) {
		if (entry.second->IsVisible()) {
			return true;
		}
	}
	return false;
}

void UiMM::Show(MainMenuPage page)
{
	// Was previously @ 0x10116500

	// In case the main menu is shown in-game, we have to take care of some business
	if (!IsVisible()) {
		if (page == MainMenuPage::InGameNormal || page == MainMenuPage::InGameIronman)
		{
			gameSystems->TakeSaveScreenshots();
			gameSystems->GetAnim().PushDisableFidget();
		}
	}

	// TODO: This seems wrong actually... This should come after hide()
	mCurrentPage = page;
	Hide();

	uiSystems->GetSaveGame().Hide();
	uiSystems->GetLoadGame().Hide();
	uiSystems->GetUtilityBar().HideOpenedWindows(false);
	uiSystems->GetChar().Hide();

	mMainWidget->Show();
	mMainWidget->BringToFront();

	for (auto &entry : mPageWidgets) {
		entry.second->SetVisible(entry.first == page);
	}

	if (page != MainMenuPage::InGameNormal) {
		uiSystems->GetUtilityBar().Hide();
	}    
	uiSystems->GetInGame().ResetInput();
}

void UiMM::Hide()
{
	if (IsVisible()) {
		if (mCurrentPage == MainMenuPage::InGameNormal || mCurrentPage == MainMenuPage::InGameIronman) {
			gameSystems->GetAnim().PopDisableFidget();
		}
	}

	for (auto &entry : mPageWidgets) {
		entry.second->SetVisible(false);
	}
	mMainWidget->Hide();

	if (mCurrentPage == MainMenuPage::InGameNormal) {
		uiSystems->GetUtilityBar().Show();
	}
}

void UiMM::RepositionWidgets(int width, int height)
{
	// Attach the pages to the bottom of the screen
	mPagesWidget->SetY(height - mPagesWidget->GetHeight());
}
