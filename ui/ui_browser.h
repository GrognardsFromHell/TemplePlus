
#pragma once

#include <include/internal/cef_ptr.h>
#include <include/internal/cef_win.h>

class UiBrowserApp;
class UiBrowserClient;
class UiBrowserInput;
class MainWindow;
class CefBrowser;

class UiBrowser {
public:
	UiBrowser(MainWindow &mainWindow);
	~UiBrowser();
	
	void Update();
	void Render();

	CefRefPtr<CefBrowser> GetBrowser();
	
private:
	UiBrowser(UiBrowser&) = delete;
	UiBrowser &operator=(UiBrowser&) = delete;
			
	void InitializeCef(HINSTANCE instance);
	void CreateBrowser(MainWindow &mainWindow);
	void LoadUi();

	CefRefPtr<UiBrowserApp> mApp;
	CefRefPtr<UiBrowserClient> mClient;
	unique_ptr<UiBrowserInput> mInput;
};
