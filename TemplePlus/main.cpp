#include "stdafx.h"

#include <infrastructure/stringutil.h>

#include "temple_functions.h"
#include "util/fixes.h"
#include "util/config.h"
#include <infrastructure/breakpad.h>
#include "startup/installationdir.h"

void InitLogging();

// Defined in temple_main.cpp for now
int TempleMain(HINSTANCE hInstance, const string& commandLine);

static wstring GetInstallationDir();

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd) {

	Breakpad breakpad;

	config.Load();
	config.Save();

	InitLogging();

	logger->info("Starting Temple Plus");
	logger->info("Version: {}", GetTemplePlusVersion());
	logger->info("Commit: {}", GetTemplePlusCommitId());

	try {
		auto toeeDir = GetInstallationDir();

		auto& dll = temple::Dll::GetInstance();
		dll.Load(toeeDir);

		if (dll.HasBeenRebased()) {
			auto moduleName = dll.FindConflictingModule();
			auto msg = format(L"Module '{}' caused temple.dll to be loaded at a different address than usual.\n"
			                  L"This will most likely lead to crashes.", moduleName);
			MessageBox(nullptr, msg.c_str(), L"Module Conflict", MB_OK | MB_ICONWARNING);
		}

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

wstring GetInstallationDir() {
	wstring toeeDir;
	InstallationDir dir;
	if (!config.toeeDir.empty()) {
		toeeDir = utf8_to_ucs2(config.toeeDir);
		if (dir.Validate(toeeDir)) {
			return dir.Normalize(toeeDir);
		}
	}

	auto toeeDirs = dir.FindInstallations();
	if (toeeDirs.empty()) {
		throw TempleException("A Temple of Elemental Evil installation could not be found.");
	}

	config.toeeDir = ucs2_to_utf8(toeeDirs[0]);
	config.Save();

	toeeDir = toeeDirs[0];
	logger->info("Using ToEE installation @ {}", ucs2_to_utf8(toeeDir));
	return toeeDir;
}
