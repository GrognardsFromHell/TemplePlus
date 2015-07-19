
#pragma once

#include <include/cef_app.h>

class UiResourceBundleHandler;

class UiBrowserApp : public CefApp {
public:
	UiBrowserApp();
	~UiBrowserApp();

	void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;

	void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) override;

	CefRefPtr<CefResourceBundleHandler> GetResourceBundleHandler() override;
private:
	IMPLEMENT_REFCOUNTING(UiBrowserApp);

	CefRefPtr<UiResourceBundleHandler> mResourceBundleHandler;
};
