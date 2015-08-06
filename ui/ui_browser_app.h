
#pragma once

#include <include/cef_app.h>

class UiResourceBundleHandler;

class UiBrowserApp : public CefApp, public CefRenderProcessHandler {
public:
	UiBrowserApp();
	~UiBrowserApp();

	void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;

	void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) override;

	CefRefPtr<CefResourceBundleHandler> GetResourceBundleHandler() override;

	CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
		return this;
	}

	void OnContextCreated(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefV8Context> context) override;

private:
	IMPLEMENT_REFCOUNTING(UiBrowserApp);

	CefRefPtr<UiResourceBundleHandler> mResourceBundleHandler;
};
