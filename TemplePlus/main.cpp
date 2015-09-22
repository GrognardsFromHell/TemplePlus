#include "stdafx.h"
#include "temple_functions.h"
#include "util/fixes.h"
#include "util/config.h"
#include "breakpad.h"

void InitLogging();

// Defined in temple_main.cpp for now
int TempleMain(HINSTANCE hInstance, const string& commandLine);

static wstring GetInstallationDir();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd) {

	Breakpad breakpad;

	config.Load();
	config.Save();

	InitLogging();

	logger->info("Starting Temple Plus");
	logger->info("Version: {}", GetTemplePlusVersion());
	logger->info("Commit: {}", GetTemplePlusCommitId());
	
	try {
		auto& dll = temple::Dll::GetInstance();
		dll.Load(".");

		if (dll.HasBeenRebased()) {
			auto moduleName = dll.FindConflictingModule();
			auto msg = format("Module '{}' caused temple.dll to be loaded at a different address than usual.\n"
				"This will most likely lead to crashes.", moduleName);
			MessageBoxA(nullptr, msg.c_str(), "Module Conflict", MB_OK | MB_ICONWARNING);
		}

		auto toeeDir = GetInstallationDir();

		// logger->info("Starting up with toee path: " << toeeDir;

		if (!SetCurrentDirectory(toeeDir.c_str())) {
			logger->error("Unable to change current working directory!");
		}
		if (!SetDllDirectory(toeeDir.c_str())) {
			logger->error("Unable to change DLL search directory.");
		}

		init_hooks();

		TempleFixes::apply();

		auto ourModule = GetModuleHandleA(nullptr);
		auto result = TempleMain(ourModule, lpCmdLine);

		config.Save();

		return result;
	} catch (const std::exception& e) {
		auto msg = format("Uncaught exception: {}", e.what());
		MessageBoxA(nullptr, msg.c_str(), "Fatal Error", MB_OK | MB_ICONERROR);
		return 1;
	}
}


// TODO this should go elsewhere
#include <codecvt>
typedef codecvt_utf8<wchar_t> convert_type;
static wstring_convert<convert_type, wchar_t> converter;

static wstring GetInstallationDir() {
	WCHAR path[MAX_PATH];
	GetModuleFileName(GetModuleHandle(L"temple"), path, MAX_PATH);

	wstring pathStr(path);

	if (!PathRemoveFileSpec(path)) {
		logger->error("Unable to remove trailing filename in temple.dll path: {}", converter.to_bytes(pathStr));
	}

	if (!PathAddBackslash(path)) {
		logger->error("Unable to append the backslash to the end of the installation directory: {}", converter.to_bytes(pathStr));
	}

	return path;
}
