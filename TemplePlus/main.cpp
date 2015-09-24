#include "stdafx.h"

#include <infrastructure/stringutil.h>
#include <infrastructure/breakpad.h>

#include "temple_functions.h"
#include "util/fixes.h"
#include "util/config.h"
#include "startup/installationdir.h"
#include "startup/installationdirpicker.h"

void InitLogging();

// Defined in temple_main.cpp for now
int TempleMain(HINSTANCE hInstance, const string& commandLine);

InstallationDir GetInstallationDir(Guide::not_null<bool*> userCancelled);
void ShowIncompatibilityWarning(const InstallationDir &dir);

// This is required to get "new style" common dialogs like message boxes
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd) {

	// We reserve space for temple.dll as early as possible to avoid rebasing of temple.dll
	auto& dll = temple::Dll::GetInstance();
	dll.ReserveMemoryRange();

	Breakpad breakpad;

	config.Load();
	config.Save();

	InitLogging();

	logger->info("Starting Temple Plus");
	logger->info("Version: {}", GetTemplePlusVersion());
	logger->info("Commit: {}", GetTemplePlusCommitId());

	try {
		bool userCancelled;
		auto toeeDir = GetInstallationDir(&userCancelled);

		if (userCancelled) {
			return 0; // Not an error, the user cancelled
		}

		ShowIncompatibilityWarning(toeeDir);

		dll.Load(toeeDir.GetDirectory());

		if (dll.HasBeenRebased()) {
			auto moduleName = dll.FindConflictingModule();
			auto msg = format(L"Module '{}' caused temple.dll to be loaded at a different address than usual.\n"
			                  L"This will most likely lead to crashes.", moduleName);
			MessageBox(nullptr, msg.c_str(), L"Module Conflict", MB_OK | MB_ICONWARNING);
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

// Shows a warning if the given ToEE installation may be incompatible
static void ShowIncompatibilityWarning(const InstallationDir &dir) {

	if (!dir.IsSupportedDllVersion()) {
		auto msg = L"An unknown game version was encountered. Please use the GOG.com version or "
			L"apply the Circle of Eight Modpack (www.co8.org).";
		auto title = L"Temple of Elemental Evil - Installation Directory";
		MessageBox(nullptr, msg, title, MB_OK | MB_ICONWARNING);
	}

}

InstallationDir GetInstallationDir(Guide::not_null<bool*> userCancelled) {
	*userCancelled = false;

	if (!config.toeeDir.empty()) {
		InstallationDir configuredDir(utf8_to_ucs2(config.toeeDir));
		if (configuredDir.IsUsable()) {
			return configuredDir;
		}
	}
		
	auto toeeDirs = InstallationDirs::FindInstallations();
	if (toeeDirs.empty()) {
		auto title = L"Temple of Elemental Evil - Installation Directory";
		MessageBox(nullptr, 
			L"Sorry! TemplePlus could not find your Temple of Elemental Evil installation automatically.\n\n"
			L"Please select your Temple of Elemental Evil installation directory manually.", 
			title,
			MB_OK|MB_ICONERROR);

		InstallationDirPicker dirPicker;
		do {
			if (!dirPicker.PickDirectory()) {
				*userCancelled = true;
				return InstallationDir();
			}

			InstallationDir pickedDir(dirPicker.GetDirectory());
			if (pickedDir.IsUsable()) {
				return pickedDir;
			}

			auto msg = fmt::format(L"Oops! The selected directory '{}' cannot be used by TemplePlus for the following reasons:\n{}", 
				pickedDir.GetDirectory(),
				pickedDir.GetNotUsableReason());
			MessageBox(nullptr, msg.c_str(), title, MB_OK | MB_ICONERROR);
		} while (true);
	}

	config.toeeDir = ucs2_to_utf8(toeeDirs[0].GetDirectory());
	config.Save();

	return InstallationDir(toeeDirs[0].GetDirectory());
}
