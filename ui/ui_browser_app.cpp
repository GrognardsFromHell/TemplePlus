
#include "stdafx.h"
#include "ui_browser_app.h"
#include "ui_resourcebundlehandler.h"

UiBrowserApp::UiBrowserApp() {
	mResourceBundleHandler = new UiResourceBundleHandler;
}

UiBrowserApp::~UiBrowserApp() {
}

void UiBrowserApp::OnBeforeCommandLineProcessing(const CefString& process_type, 
	CefRefPtr<CefCommandLine> command_line) {

	//command_line->AppendSwitch("disable-gpu");
	//command_line->AppendSwitch("disable-gpu-compositing");

}

void UiBrowserApp::OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) {
	// registrar->AddCustomScheme("tp", true, true, false);
}

CefRefPtr<CefResourceBundleHandler> UiBrowserApp::GetResourceBundleHandler() {
	return mResourceBundleHandler;
}
