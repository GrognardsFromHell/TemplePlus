
#pragma once

#include <include/internal/cef_ptr.h>

class UiBrowserApp;
class UiBrowserClient;
class UiBrowserInput;

class UiBrowser {
public:
	UiBrowser();
	~UiBrowser();
	
	void Update();
	void Render();

	void DoSomething();

private:

	void InitializeCef();
	void CreateBrowser();
	void LoadUi();

	CefRefPtr<UiBrowserApp> mApp;
	CefRefPtr<UiBrowserClient> mClient;
	std::unique_ptr<UiBrowserInput> mInput;
};
