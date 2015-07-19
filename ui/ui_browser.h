
#pragma once

#include <include/internal/cef_ptr.h>

class UiBrowserApp;
class UiBrowserClient;

class UiBrowser {
public:
	UiBrowser();
	~UiBrowser();
	
	void Update();
	void Render();

private:

	void InitializeCef();
	void CreateBrowser();
	void LoadUi();

	CefRefPtr<UiBrowserApp> mApp;
	CefRefPtr<UiBrowserClient> mClient;
};
