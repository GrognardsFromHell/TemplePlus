
#include "stdafx.h"
#include "templeplus.h"

#include "ui/ui_browser.h"

#include "util/config.h"
#include "util/exception.h"
#include "util/version.h"
#include "util/stringutil.h"
#include "util/fixes.h"

#include "temple_functions.h"
#include "temple_library.h"

#include "mainwindow.h"

// Defined in temple_main.cpp for now
int TempleMain(HINSTANCE hInstance, const string& commandLine);

TemplePlus *tp = nullptr;

TemplePlus::TemplePlus()
{
	if (tp) {
		throw new TempleException("There should only be a single instance of the TemplePlus class");
	}
	tp = this;
}

TemplePlus::~TemplePlus()
{
	tp = nullptr;
}

int TemplePlus::Run(HINSTANCE hInstance, LPSTR cmdLine)
{
	// Loading the config and initializing the logfile is always required
	config.Load();
	config.Save();

	InitLogging();

	// Load temple.dll first because we have to avoid address space conflicts with CEF
	try {
		mLibrary.reset(new TempleLibrary);
	} catch (exception& e) {
		string msg = format("Unable to initialize temple.dll: {}", e.what());
		MessageBoxA(NULL, msg.c_str(), "Fatal Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	// --- From here on out, it's our main process that's running
	return RunGame(hInstance, cmdLine);
}

int TemplePlus::RunGame(HINSTANCE hInstance, LPSTR cmdLine)
{
	logger->info("Starting Temple Plus");
	logger->info("Version: {}", GetTemplePlusVersion());
	logger->info("Commit: {}", GetTemplePlusCommitId());

	// Create main window
	mMainWindow.reset(new MainWindow(hInstance));

	// Create browser infrastructure
	mBrowser.reset(new UiBrowser(mainWindow()));

	// Initialize minhook
	MH_Initialize();

	int result = 1;
	try {
		init_functions();
		init_hooks();

		TempleFixes::apply();

		result = TempleMain(hInstance, cmdLine);
	} catch (exception& e) {
		string msg = format("Uncaught exception: {}", e.what());
		MessageBoxA(NULL, msg.c_str(), "Fatal Error", MB_OK | MB_ICONERROR);
	}

	MH_Uninitialize();

	config.Save();

	return result;
}

shared_ptr<spdlog::logger> logger;

struct OutputDebugStringSink : public spdlog::sinks::base_sink<std::mutex> {
protected:
	void _sink_it(const spdlog::details::log_msg& msg) override {
		OutputDebugStringA(msg.formatted.c_str());
	}
};

void TemplePlus::InitLogging()
{

	spdlog::set_level(spdlog::level::debug);

	try {
		// Make this a config option
		if (true) {
			DeleteFile(L"TemplePlus.log");
			auto fileSink = make_shared<spdlog::sinks::simple_file_sink_mt>("TemplePlus.log", true);
			auto debugSink = make_shared<OutputDebugStringSink>();
			logger = spdlog::create("core", { fileSink, debugSink });
		} else {
			auto debugSink = make_shared<OutputDebugStringSink>();
			logger = spdlog::create("core", { debugSink });
		}
	}
	catch (const spdlog::spdlog_ex& e) {
		auto msg = format("Unable to initialize the logging subsystem:\n{}", e.what());
		MessageBoxA(NULL, msg.c_str(), "Logging Error", MB_OK | MB_ICONERROR);
	}

}
