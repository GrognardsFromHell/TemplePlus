
#pragma once

#include "ui_system.h"
#include "widgets/widgets.h"

#include <temple/dll.h>

struct UiSystemConf;

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

class UiMM : public UiSystem {
public:
	static constexpr auto Name = "MM-UI";
	UiMM(const UiSystemConf &config);
	~UiMM();
	void ResizeViewport(const UiResizeArgs &resizeArgs) override;
	const std::string &GetName() const override;

	bool IsVisible() const;

	void ShowPage(int page);

private:
	MainMenuPages &mPages = temple::GetRef<MainMenuPages>(0x10BD4F40);

	std::unique_ptr<WidgetContainer> mMainWidget;
	std::vector<WidgetContainer*> mPageWidgets;
	// The widget that contains all pages
	WidgetContainer *mPagesWidget;

	void RepositionWidgets(int width, int height);
};
