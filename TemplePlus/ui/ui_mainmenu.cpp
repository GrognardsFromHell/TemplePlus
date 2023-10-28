
#include "stdafx.h"

#include "ui_mainmenu.h"
#include "ui_systems.h"
#include "ui_legacysystems.h"

#include "animgoals/anim.h"
#include "critter.h"
#include "party.h"
#include "fade.h"

#include "gamesystems/gamesystems.h"
#include "gamesystems/mapsystem.h"
#include "gamesystems/legacysystems.h"
#include "gamesystems/objects/objsystem.h"

#include "tig/tig_msg.h"
#include "movies.h"
#include "messages/messagequeue.h"

#include "widgets/widget_content.h"
#include "widgets/widget_doc.h"
#include "widgets/widget_styles.h"
#include "tig/tig_keyboard.h"
#include "ui_pc_creation.h"
#include "ui/ui_char.h"
#include "python/python_integration_obj.h"

class ViewCinematicsDialog {
public:
	ViewCinematicsDialog();

	void Show();
	void Select(int i); // changes scrollbox selection
	bool IsMovieSeen(int movieId);
private:
	int mSelection = 0;
	std::unique_ptr<WidgetContainer> mWidget;
	MesFile::Content mMovieNames;
	WidgetScrollView *mListBox;
	std::vector<LgcyWidgetId> btnIds;
	std::vector<int> seenIndices; // indices into movieIds / mMovieNames

	std::vector<int> movieIds = { 
	1000,	1009,	1007, 
	1012 ,	1002,	1015,
	1005,      1010,      1004,
	1013,      1006,      1016,
	1001,      1011,      1008,
	1014,      1003,      1017,
	304,       300,       303,
	301,       302,      1009
	};
};

class SetPiecesDialog
{
public:
	SetPiecesDialog();

	void Select(int i);
	void Show();
	void LaunchScenario();
	void TransitionToMap(int destMap);
	void SetupScenario();
private:
	int mSelection = 0;
	std::unique_ptr<WidgetContainer> mWidget;
	WidgetScrollView *mListBox;
	std::vector<LgcyWidgetId> btnIds;
};

//*****************************************************************************
//* MM-UI
//*****************************************************************************
#pragma region MM-UI
UiMM::UiMM(const UiSystemConf &config) {
	auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10117370);

	if (!USE_NEW_WIDGETS){
		if (!startup(&config)) {
			throw TempleException("Unable to initialize game system MM-UI");
		}

		return;
	}
	
	auto mmRulesMesfile = MesFile::ParseFile("rules\\mainmenu.mes");
	auto mmTitleImg = uiAssets->LoadImg(mmRulesMesfile[10000].c_str());
	temple::GetRef<ImgFile*>(0x10BD4DB0) = mmTitleImg;
	auto mmImageX = atol(mmRulesMesfile[10001].c_str());
	auto mmImageY = atol(mmRulesMesfile[10002].c_str());
	if (!mmImageX){
		mmImageX = (config.width - mmTitleImg->width) / 2;
	}
	temple::GetRef<int>(0x10BD48F0) = mmImageX;
	temple::GetRef<int>(0x10BD4998) = mmImageY;
	
	WidgetDoc widgetDoc(WidgetDoc::Load("templeplus/ui/main_menu.json"));
	mMainWidget = widgetDoc.TakeRootContainer();

	mViewCinematicsDialog = std::make_unique<ViewCinematicsDialog>();
	mSetPiecesDialog = std::make_unique<SetPiecesDialog>();

	// This eats all mouse messages that reach the full-screen main menu
	mMainWidget->SetMouseMsgHandler([](auto msg) {
		return true;
	});
	mMainWidget->SetWidgetMsgHandler([](auto msg) {
		return true;
	});
	
	mMainWidget->SetKeyStateChangeHandler([this](const TigKeyStateChangeMsg &msg) {
		// Close the menu if it's the ingame menu
		if (msg.key == DIK_ESCAPE && !msg.down) {
			if (mCurrentPage == MainMenuPage::InGameNormal || mCurrentPage == MainMenuPage::InGameIronman) {
				Hide();
			}
		}
		if (mCurrentPage == MainMenuPage::MainMenu) {
			return false; // so it doesn't block F10 diag window
		}
		return true;
	});

	mPagesWidget = widgetDoc.GetWindow("pages");

	mPageWidgets[MainMenuPage::MainMenu] = widgetDoc.GetWindow("page-main-menu");
	mPageWidgets[MainMenuPage::Difficulty] = widgetDoc.GetWindow("page-difficulty");
	mPageWidgets[MainMenuPage::InGameNormal] = widgetDoc.GetWindow("page-ingame-normal");
	mPageWidgets[MainMenuPage::InGameIronman] = widgetDoc.GetWindow("page-ingame-ironman");
	mPageWidgets[MainMenuPage::Options] = widgetDoc.GetWindow("page-options");
	//mPageWidgets[MainMenuPage::SetPieces] = widgetDoc.GetWindow("page-set-pieces");

	// Wire up buttons on the main menu
	widgetDoc.GetButton("new-game")->SetClickHandler([this]() {
		Show(MainMenuPage::Difficulty);
	});
	widgetDoc.GetButton("load-game")->SetClickHandler([this]() {
		Hide();
		uiSystems->GetLoadGame().Show(true);
	});
	/*widgetDoc.GetButton("set-pieces")->SetClickHandler([this]() {
		Hide();
		mSetPiecesDialog->Show();
	});*/
	widgetDoc.GetButton("tutorial")->SetClickHandler([this]() {
		LaunchTutorial();
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
		// TODO ui_mm_msg_ui4();
		mViewCinematicsDialog->Show();
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
	auto shutdown = temple::GetPointer<void()>(0x101164c0);
	if (!USE_NEW_WIDGETS){
		shutdown();
	}
}
void UiMM::ResizeViewport(const UiResizeArgs& resizeArg) {
	auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x101172c0);
	if (!USE_NEW_WIDGETS){
		resize(&resizeArg);
		return;
	}
	
	RepositionWidgets(resizeArg.rect1.width, resizeArg.rect1.height);
}
const std::string &UiMM::GetName() const {
	static std::string name("MM-UI");
	return name;
}

bool UiMM::IsVisible() const
{
	static auto ui_mm_is_visible = temple::GetPointer<int()>(0x101157f0);
	if (!USE_NEW_WIDGETS)
		return ui_mm_is_visible() != FALSE;

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
	static auto ui_mm_show_page = temple::GetPointer<void(int page)>(0x10116500);

	if (!USE_NEW_WIDGETS){
		ui_mm_show_page((int)page);
		if (!uiSystems->GetUtilityBar().IsVisible())
			uiSystems->GetDM().Hide();
		return;
	}
	
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

	if (!uiSystems->GetUtilityBar().IsVisible())
		uiSystems->GetDM().Hide();
}

void UiMM::Hide()
{
	static auto ui_mm_hide = temple::GetPointer<void()>(0x10116220);
	if (!USE_NEW_WIDGETS){
		ui_mm_hide();
		if (uiSystems->GetUtilityBar().IsVisible())
			uiSystems->GetDM().Show();
		return;
	}
	

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

	//if (uiSystems->GetUtilityBar().IsVisible())
	//	uiSystems->GetDM().Show();
}

void UiMM::RepositionWidgets(int width, int height)
{
	// Attach the pages to the bottom of the screen
	mPagesWidget->SetY(height - mPagesWidget->GetHeight());
}

void UiMM::LaunchTutorial()
{
	auto velkorProto = objSystem->GetProtoHandle(13105);
	
	auto velkor = objSystem->CreateObject(velkorProto, locXY{480, 40});
	auto velkorObj = objSystem->GetObject(velkor);
	velkorObj->SetInt32(obj_f_pc_voice_idx, 11);
	critterSys.GenerateHp(velkor);
	party.AddToPCGroup(velkor);

	static auto spawn_velkor_equipment = temple::GetPointer<void(objHndl)>(0x1006d300);
	spawn_velkor_equipment(velkor);
	
	auto anim = objects.GetAnimHandle(velkor);
	objects.UpdateRenderHeight(velkor, *anim);
	objects.UpdateRadius(velkor, *anim);
	
	SetupTutorialMap();
	uiSystems->GetParty().UpdateAndShowMaybe();
	Hide();
	uiSystems->GetParty().Update();
}

// Was @ 10111AD0
void UiMM::SetupTutorialMap()
{	
	static auto ui_tutorial_isactive = temple::GetPointer<int()>(0x10124a10);
	static auto ui_tutorial_toggle = temple::GetPointer<int()>(0x101249e0);

	if (!ui_tutorial_isactive()) {
		ui_tutorial_toggle();
	}

	auto tutorialMap = gameSystems->GetMap().GetMapIdByType(MapType::TutorialMap);
	TransitionToMap(tutorialMap);
}

void UiMM::TransitionToMap(int mapId)
{
	FadeArgs fadeArgs;
	fadeArgs.flags = 0;
	fadeArgs.color = 0;
	fadeArgs.countSthgUsually48 = 1;
	fadeArgs.transitionTime = 0;
	fadeArgs.field10 = 0;
	fade.PerformFade(fadeArgs);
	gameSystems->GetAnim().StartFidgetTimer();

	FadeAndTeleportArgs fadeTp;
	fadeTp.destLoc = gameSystems->GetMap().GetStartPos(mapId);
	fadeTp.destMap = mapId;
	fadeTp.flags = 4;
	fadeTp.somehandle = party.GetLeader();

	auto enterMovie = gameSystems->GetMap().GetEnterMovie(mapId, true);
	if (enterMovie) {
		fadeTp.flags |= 1;
		fadeTp.field20 = 0;
		fadeTp.movieId = enterMovie;
	}
	fadeTp.field48 = 1;
	fadeTp.field4c = 0xFF000000;
	fadeTp.field50 = 64;
	fadeTp.somefloat2 = 3.0;
	fadeTp.field58 = 0;
	fade.FadeAndTeleport(fadeTp);

	gameSystems->GetSoundGame().StopAll(false);
	uiSystems->GetWMapRnd().StartRandomEncounterTimer();
	gameSystems->GetAnim().PopDisableFidget();
}

#pragma endregion

#pragma region ViewCinematicsDialog

ViewCinematicsDialog::ViewCinematicsDialog()
{
	WidgetDoc doc = WidgetDoc::Load("templeplus/ui/main_menu_cinematics.json");

	doc.GetButton("view")->SetClickHandler([this]() {
		if (mSelection < 0 || mSelection >= seenIndices.size())
			return;
		auto movieIdx = seenIndices[mSelection];
		if (movieIdx < 0 || movieIdx >= movieIds.size())
			return;
		auto movieId = movieIds[movieIdx];
		movieFuncs.PlayMovieId(movieId, 0, 0);
	});
	doc.GetButton("cancel")->SetClickHandler([this]() {
		mWidget->Hide();
		uiSystems->GetMM().Show(MainMenuPage::Options);
	});

	mListBox = doc.GetScrollView("cinematicsList");

	mWidget = std::move(doc.TakeRootContainer());
	mWidget->Hide();

	auto mmMes = MesFile::ParseFile("mes\\mainmenu.mes");
	for (auto i = 0; i < 24; i++){
		mMovieNames[i] = mmMes[2000 + i];
	}
	
}

void ViewCinematicsDialog::Show()
{
	mListBox->Clear();
	btnIds.clear();
	seenIndices.clear();


	for (auto i = 0; i < movieIds.size(); i++) {
		if (IsMovieSeen(movieIds[i])){
			seenIndices.push_back(i);
		}
	}

	int y = 0;
	for (int i = 0; i < seenIndices.size(); i++) {
		auto movieInd = seenIndices[i];

		auto button = std::make_unique<WidgetButton>();
		button->SetText(mMovieNames[movieInd]);
		button->SetId(mMovieNames[movieInd]);
		auto innerWidth = mListBox->GetInnerWidth();
		button->SetWidth(innerWidth);
		button->SetAutoSizeWidth(false);
		button->SetStyle("mm-cinematics-list-button");
		button->SetY(y);
		//auto pBtn = button.get();
		btnIds.push_back(button->GetWidgetId());
		button->SetClickHandler([i, this](){
			Select(i);
		});
		y += button->GetHeight();
		mListBox->Add(std::move(button));
	}
	
	mWidget->Show();
}

void ViewCinematicsDialog::Select(int idx){
	
	for (auto it: btnIds){
		auto pBtn = (WidgetButton*)uiManager->GetAdvancedWidget(it);
		pBtn->SetStyle("mm-cinematics-list-button");
	}

	mSelection = idx;
	if (mSelection >= 0 && mSelection < btnIds.size()) {
	auto pBtn = (WidgetButton*)uiManager->GetAdvancedWidget(btnIds[mSelection]);
	pBtn->SetStyle("mm-cinematics-list-button-selected");
	}
	
}

bool ViewCinematicsDialog::IsMovieSeen(int movieId){
	auto moviesSeen = config.GetVanillaString("movies_seen");
	auto movieStr = fmt::format("({},-1)", movieId);
	return strstr(moviesSeen.c_str(), movieStr.c_str()) != nullptr;
}

#pragma endregion

SetPiecesDialog::SetPiecesDialog(){
	WidgetDoc doc = WidgetDoc::Load("templeplus/ui/main_menu_setpieces.json");

	doc.GetButton("go")->SetClickHandler([this]() {
		mWidget->Hide();
		LaunchScenario();
	});
	doc.GetButton("cancel")->SetClickHandler([this]() {
		mWidget->Hide();
		uiSystems->GetMM().Show(MainMenuPage::MainMenu);
	});

	mListBox = doc.GetScrollView("scenariosList");

	mWidget = std::move(doc.TakeRootContainer());
	mWidget->Hide();

}

void SetPiecesDialog::Select(int i)
{
	mSelection = i;
}

void SetPiecesDialog::Show(){
	mListBox->Clear();

	int y = 0;
#define NUM_SCENARIOS 0
	for (int i = 0; i < NUM_SCENARIOS; i++) {
		
		auto button = std::make_unique<WidgetButton>();
		button->SetText("Arena");
		button->SetId("Arena");
		auto innerWidth = mListBox->GetInnerWidth();
		button->SetWidth(innerWidth);
		button->SetAutoSizeWidth(false);
		button->SetStyle("mm-setpieces-list-button");
		button->SetY(y);
		//auto pBtn = button.get();
		btnIds.push_back(button->GetWidgetId());
		button->SetClickHandler([i, this]() {
			Select(i);
		});
		y += button->GetHeight();
		mListBox->Add(std::move(button));
	}

	mWidget->Show();
}

void SetPiecesDialog::LaunchScenario(){
	
	auto velkorProto = objSystem->GetProtoHandle(13105);

	auto velkor = objSystem->CreateObject(velkorProto, locXY{ 480, 40 });
	auto velkorObj = objSystem->GetObject(velkor);
	velkorObj->SetInt32(obj_f_pc_voice_idx, 11);
	critterSys.GenerateHp(velkor);
	party.AddToPCGroup(velkor);

	static auto spawn_velkor_equipment = temple::GetPointer<void(objHndl)>(0x1006d300);
	spawn_velkor_equipment(velkor);

	auto anim = objects.GetAnimHandle(velkor);
	objects.UpdateRenderHeight(velkor, *anim);
	objects.UpdateRadius(velkor, *anim);

	SetupScenario();
	uiSystems->GetParty().UpdateAndShowMaybe();
	uiSystems->GetMM().Hide();
	uiSystems->GetParty().Update();

}

void SetPiecesDialog::TransitionToMap(int mapId){
	FadeArgs fadeArgs;
	fadeArgs.flags = 0;
	fadeArgs.color = 0;
	fadeArgs.countSthgUsually48 = 1;
	fadeArgs.transitionTime = 0;
	fadeArgs.field10 = 0;
	fade.PerformFade(fadeArgs);
	gameSystems->GetAnim().StartFidgetTimer();

	FadeAndTeleportArgs fadeTp;
	fadeTp.destLoc = gameSystems->GetMap().GetStartPos(mapId);
	fadeTp.destMap = mapId;
	fadeTp.flags = 4;
	fadeTp.somehandle = party.GetLeader();

	auto enterMovie = gameSystems->GetMap().GetEnterMovie(mapId, true);
	if (enterMovie) {
		fadeTp.flags |= 1;
		fadeTp.field20 = 0;
		fadeTp.movieId = enterMovie;
	}
	fadeTp.field48 = 1;
	fadeTp.field4c = 0xFF000000;
	fadeTp.field50 = 64;
	fadeTp.somefloat2 = 3.0;
	fadeTp.field58 = 0;
	fade.FadeAndTeleport(fadeTp);

	gameSystems->GetSoundGame().StopAll(false);
	uiSystems->GetWMapRnd().StartRandomEncounterTimer();
	gameSystems->GetAnim().PopDisableFidget();
}

void SetPiecesDialog::SetupScenario(){
	
	auto destMap = gameSystems->GetMap().GetMapIdByType(MapType::ArenaMap);
	TransitionToMap(destMap);

	auto args = PyTuple_New(0);

	auto result = pythonObjIntegration.ExecuteScript("arena_script", "OnStartup", args);
	Py_DECREF(result);
	Py_DECREF(args);
}
