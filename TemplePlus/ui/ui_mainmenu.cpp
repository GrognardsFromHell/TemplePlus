
#include "stdafx.h"

#include "ui_mainmenu.h"
#include "ui_legacysystems.h"

#include "widgets/widget_content.h"
#include "widgets/widget_doc.h"

//*****************************************************************************
//* MM-UI
//*****************************************************************************

UiMM::UiMM(const UiSystemConf &config) {
	auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10117370);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system MM-UI");
	}
	
	WidgetDoc widgetDoc(WidgetDoc::Load("templeplus/ui/main_menu.json"));
	mMainWidget = widgetDoc.TakeRootContainer();
	widgetDoc.GetButton("new-game")->SetClickHandler([] {
		logger->info("Hello World!");
	});
	mPagesWidget = widgetDoc.GetWindow("pages");

	for (int i = 1; i <= 5; i++) {
		auto page = widgetDoc.GetWindow(fmt::format("page{}", i));
		page->SetVisible(false);
		mPageWidgets.push_back(page);
	}

	// Wire up buttons on page 1
	widgetDoc.GetButton("new-game")->SetClickHandler([this]() {
		ShowPage(1);
	});
	widgetDoc.GetButton("load-game")->SetClickHandler([]() {
	});
	widgetDoc.GetButton("tutorial")->SetClickHandler([]() {
	});
	widgetDoc.GetButton("options")->SetClickHandler([]() {
	});
	widgetDoc.GetButton("quit-game")->SetClickHandler([]() {
	});

	RepositionWidgets(config.width, config.height);
}
UiMM::~UiMM() {
	auto shutdown = temple::GetPointer<void()>(0x101164c0);
	shutdown();
}
void UiMM::ResizeViewport(const UiResizeArgs& resizeArg) {
	auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x101172c0);
	resize(&resizeArg);

	RepositionWidgets(resizeArg.rect1.width, resizeArg.rect1.height);
}
const std::string &UiMM::GetName() const {
	static std::string name("MM-UI");
	return name;
}

bool UiMM::IsVisible() const
{
	static auto ui_mm_is_visible = temple::GetPointer<int()>(0x101157f0);
	return ui_mm_is_visible() == TRUE;
}

void UiMM::ShowPage(int page)
{
	static auto ui_mm_show_page = temple::GetPointer<void(int page)>(0x10116500);
	ui_mm_show_page(page);

	mMainWidget->BringToFront();

	for (auto &page : mPageWidgets) {
		page->SetVisible(false);
	}
	mPageWidgets[page]->SetVisible(true);
}

void UiMM::RepositionWidgets(int width, int height)
{
	// Center main widget horizontally
	mPagesWidget->SetY(height - mPagesWidget->GetHeight());
}
