
#include "stdafx.h"

#include "ui_browser.h"
#include "ui_browser_app.h"
#include "ui_browser_client.h"
#include "ui_browser_input.h"
#include "../util/exception.h"
#include "../util/stopwatch.h"
#include "../graphics.h"
#include "../mainwindow.h"
#include "ui_threads.h"
#include <include/cef_app.h>
#include <include/cef_browser.h>

UiBrowser::UiBrowser(MainWindow &mainWindow) {
	mApp = new UiBrowserApp;
	mClient = new UiBrowserClient(mainWindow);

	InitializeCef(mainWindow.GetHInstance());
	CreateBrowser(mainWindow);
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

void UiBrowser::InitializeCef(HINSTANCE instance) {
	StopwatchReporter watch("Initialized browser in {}");

	CefSettings settings;
	settings.no_sandbox = true;
	settings.background_color = CefColorSetARGB(0, 0, 0, 0);
	settings.command_line_args_disabled = true;
	settings.multi_threaded_message_loop = false;
	settings.windowless_rendering_enabled = true;
	settings.single_process = true;
	settings.log_severity = LOGSEVERITY_VERBOSE;

	CefMainArgs mainArgs(instance);

	CefExecuteProcess(mainArgs, mApp.get(), nullptr);
	
	bool result = CefInitialize(mainArgs, settings, mApp.get(), NULL); ;
	/*UiThreads::PostUiTask<bool>([&] {
		return CefInitialize(mainArgs, settings, mApp.get(), NULL);
	}).get();*/
	if (!result) {
		throw new TempleException("Unable to initialize CEF.");
	}
}

void UiBrowser::CreateBrowser(MainWindow &mainWindow) {

	UiThreads::PostUiTask<bool>([&] {
		CefWindowInfo windowInfo;
		windowInfo.windowless_rendering_enabled = true;
		windowInfo.SetAsWindowless(mainWindow.GetHwnd(), true);

		CefBrowserSettings settings;
		settings.plugins = STATE_DISABLED;
		settings.webgl = STATE_DISABLED;
		settings.java = STATE_DISABLED;
		settings.background_color = CefColorSetARGB(0, 0, 0, 0);

		if (!CefBrowserHost::CreateBrowserSync(windowInfo, mClient, "http://localhost:8000/", settings, nullptr)) {
			throw new TempleException("Unable to create browser");
		}

		if (!mClient->browser()) {
			throw new TempleException("Creation of browser did not succeed.");
		}

		mClient->browser()->GetHost()->WasResized();

		mClient->browser()->GetHost()->SetWindowVisibility(true);

		// This initializes the message filter that routes input events to the browser
		mInput.reset(new UiBrowserInput(mClient->browser(), mainWindow));

		return true;
	}).wait();
	
}

void UiBrowser::LoadUi() {
	// Wait until the browser says the page has loaded completely, since that means
	// we can show it.
}
