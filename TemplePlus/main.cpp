#include "stdafx.h"

#include <infrastructure/breakpad.h>

#include "temple_functions.h"
#include "util/fixes.h"
#include "config/config.h"
#include "startup/installationdir.h"
#include "startup/installationdirs.h"
#include "startup/installationdirpicker.h"
#include "util/folderutils.h"

#include "util/datadump.h"

void InitLogging(const std::wstring &logFile);

// Defined in temple_main.cpp for now
int TempleMain(HINSTANCE hInstance, const string& commandLine);

InstallationDir GetInstallationDir(gsl::not_null<bool*> userCancelled);
void ShowIncompatibilityWarning(const InstallationDir& dir);

// This is required to get "new style" common dialogs like message boxes
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

void SetIniPath() {

	auto userDataFolder = GetUserDataFolder();

	auto iniPath = userDataFolder + L"TemplePlus.ini";

	config.SetPath(ucs2_to_local(iniPath));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd) {

	/*VirtualAlloc(reinterpret_cast<void*>(0x10000000),
		1,
		MEM_RESERVE,
		PAGE_NOACCESS);*/

	// We reserve space for temple.dll as early as possible to avoid rebasing of temple.dll
	auto& dll = temple::Dll::GetInstance();
	dll.ReserveMemoryRange();

	Breakpad breakpad;

	ComInitializer comInitializer;

	SetIniPath();

	config.Load();
	config.Save();

	auto logFile = GetUserDataFolder() + L"TemplePlus.log";
	InitLogging(logFile);

	logger->info("Starting Temple Plus");
	logger->info("Version: {}", GetTemplePlusVersion());
	logger->info("Commit: {}", GetTemplePlusCommitId());

	bool userCancelled;
	auto toeeDir = GetInstallationDir(&userCancelled);

	if (userCancelled) {
		return 0; // Not an error, the user cancelled
	}

	dll.Load(toeeDir.GetDirectory());

	if (dll.HasBeenRebased()) {
		auto moduleName = dll.FindConflictingModule();
		auto msg = format(L"Module '{}' caused temple.dll to be loaded at a different address than usual.\n"
			                L"This will most likely lead to crashes.", moduleName);
		MessageBox(nullptr, msg.c_str(), L"Module Conflict", MB_OK | MB_ICONWARNING);
	}

	dll.SetDebugOutputCallback([](const std::string &msg) {
		logger->info("{}", msg);
	});

	TempleFixes::apply();

	MH_EnableHook(MH_ALL_HOOKS);

	auto result = TempleMain(hInstance, lpCmdLine);

	config.Save();

	return result;
}

// Shows a warning if the given ToEE installation may be incompatible
static void ShowIncompatibilityWarning(const InstallationDir& dir) {

	if (!dir.IsSupportedDllVersion()) {
		auto msg = L"An unknown game version was encountered. Please use the GOG.com version or "
			L"apply the Circle of Eight Modpack (www.co8.org).";
		auto title = L"Temple of Elemental Evil - Installation Directory";
		MessageBox(nullptr, msg, title, MB_OK | MB_ICONWARNING);
	}

}

InstallationDir GetInstallationDir(gsl::not_null<bool*> userCancelled) {
	*userCancelled = false;

	if (!config.toeeDir.empty()) {
		InstallationDir configuredDir(config.toeeDir);
		if (configuredDir.IsUsable()) {
			return configuredDir;
		}
	}

	auto toeeDirs = InstallationDirs::FindInstallations();

	InstallationDir toeeDir;
	if (toeeDirs.empty()) {
		// None could be found automatically, let the user pick
		toeeDir = InstallationDirPicker::Pick(userCancelled);
	} else {
		toeeDir = toeeDirs[0];
	}

	// Save the new directory only if the user didn't cancel selection
	if (!*userCancelled) {
		ShowIncompatibilityWarning(toeeDir);
		config.toeeDir = toeeDir.GetDirectory();
		config.usingCo8 = toeeDir.IsCo8();
		config.Save();
	}

	return toeeDir;

}
