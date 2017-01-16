
#pragma once

#include "ui_system.h"
#include "widgets/widgets.h"

#include <temple/dll.h>

struct UiSystemConf;

/*
	These are the structs used by the old system internally...

#pragma pack(push, 1)
struct MainMenuPageButton {
	LgcyButton widget;
	LgcyWidgetId widgetId;
	LgcyWidgetId parentWidgetId;
	int buttonIdx;
	char *label;
	TigRect rect;
	TigRect textureRect;
	int _1;
	int textureIdNormal;
	int _2;
	int textureIdHover;
	int _3;
	int textureIdDown;
};

struct MainMenuPage {
	LgcyWindow widget;
	LgcyWidgetId widgetId;
	MainMenuPageButton buttons[10];
};
using MainMenuPages = MainMenuPage[10];
#pragma pack(pop)
*/

enum class MainMenuPage {
	MainMenu,
	Difficulty,
	InGameNormal,
	InGameIronman,
	Options,
	SetPieces
};

class ViewCinematicsDialog;

class UiMM : public UiSystem {
public:
	static constexpr auto Name = "MM-UI";
	UiMM(const UiSystemConf &config);
	~UiMM();
	void ResizeViewport(const UiResizeArgs &resizeArgs) override;
	const std::string &GetName() const override;

	bool IsVisible() const;

	void Show(MainMenuPage page);
	void Hide();

private:
	// MainMenuPages &mPages = temple::GetRef<MainMenuPages>(0x10BD4F40);

	MainMenuPage mCurrentPage = MainMenuPage::MainMenu;

	std::unique_ptr<ViewCinematicsDialog> mViewCinematicsDialog;

	std::unique_ptr<WidgetContainer> mMainWidget;
	std::map<MainMenuPage, WidgetContainer*> mPageWidgets;
	// The widget that contains all pages
	WidgetContainer *mPagesWidget;

	void RepositionWidgets(int width, int height);

	void LaunchTutorial();
	void SetupTutorialMap();
	void TransitionToMap(int mapId);
};
