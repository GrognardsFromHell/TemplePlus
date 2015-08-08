
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

	static CefRefPtr<CefV8Context> GetMainJsContext() {
		return mMainContext;
	}

private:
	IMPLEMENT_REFCOUNTING(UiBrowserApp);

	static CefRefPtr<CefV8Context> mMainContext;

	CefRefPtr<UiResourceBundleHandler> mResourceBundleHandler;
};
