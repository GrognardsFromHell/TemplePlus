
#include "stdafx.h"

#include "ui_browser.h"
#include "ui_browser_app.h"
#include "ui_browser_client.h"
#include "../util/exception.h"
#include "../util/stopwatch.h"
#include "../graphics.h"

#include <include/cef_app.h>
#include <include/cef_browser.h>

UiBrowser::UiBrowser() {
	mApp = new UiBrowserApp;
	mClient = new UiBrowserClient;

	InitializeCef();
	CreateBrowser();
	LoadUi();
}

UiBrowser::~UiBrowser() {
	CefShutdown();
}

void UiBrowser::Update() {
	CefDoMessageLoopWork();
}

void UiBrowser::Render() {
	mClient->Render();
}

void UiBrowser::InitializeCef() {
	StopwatchReporter watch("Initialized browser in {}");

	CefMainArgs mainArgs;
	int exitCode;
	if ((exitCode = CefExecuteProcess(mainArgs, mApp, nullptr)) >= 0) {
		exit(exitCode);
	}

	CefSettings settings;
	settings.no_sandbox = true;
	settings.background_color = CefColorSetARGB(0, 0, 0, 0);
	settings.single_process = true;
	settings.command_line_args_disabled = true;
	// settings.multi_threaded_message_loop = true;
	settings.windowless_rendering_enabled = true;
	settings.log_severity = LOGSEVERITY_VERBOSE;

	if (!CefInitialize(mainArgs, settings, mApp.get(), NULL)) {
		throw new TempleException("Unable to initialize CEF.");
	}
}

void UiBrowser::CreateBrowser() {
	CefWindowInfo windowInfo;
	windowInfo.windowless_rendering_enabled = true;
	windowInfo.SetAsWindowless(video->hwnd, true);

	CefBrowserSettings settings;
	settings.plugins = STATE_DISABLED;
	settings.webgl = STATE_DISABLED;
	settings.java = STATE_DISABLED;
	settings.background_color = CefColorSetARGB(0, 0, 0, 0);
	
	if (!CefBrowserHost::CreateBrowserSync(windowInfo, mClient, "http://localhost:8000/", settings, nullptr)) {
		throw new TempleException("Unable to create browser");
	}

}

void UiBrowser::LoadUi() {
}
