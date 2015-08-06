
#pragma once

#include "temple_library.h"
#include "ui/ui_browser.h"

class MainWindow;
class UiBrowser;

class TemplePlus {
public:
	TemplePlus();
	~TemplePlus();

	int Run(HINSTANCE hInstance, LPSTR cmdLine);

	TempleLibrary &library() {
		return *mLibrary.get();
	}

	UiBrowser &browser() {
		return *mBrowser.get();
	}

	MainWindow &mainWindow() {
		return *mMainWindow.get();
	}

private:
	int RunGame(HINSTANCE hInstance, LPSTR cmdLine);

	void InitLogging();

	unique_ptr<TempleLibrary> mLibrary;
	unique_ptr<UiBrowser> mBrowser;
	unique_ptr<MainWindow> mMainWindow;
};

extern TemplePlus *tp;
