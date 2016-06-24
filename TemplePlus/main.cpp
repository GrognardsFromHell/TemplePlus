#include "stdafx.h"

#include <infrastructure/breakpad.h>

#include "temple_functions.h"
#include "util/fixes.h"
#include "config/config.h"
#include "util/folderutils.h"

#include "util/datadump.h"

#include <QtGui/QGuiApplication>

void InitLogging(const std::wstring &logFile);

// Defined in temple_main.cpp for now
int TempleMain(HINSTANCE hInstance, const string& commandLine);

// This is required to get "new style" common dialogs like message boxes
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

static void SetIniPath();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd) {

	// The following code will cause temple.dll to be rebased
	/*VirtualAlloc(reinterpret_cast<void*>(0x10000000),
		1,
		MEM_RESERVE,
		PAGE_NOACCESS);*/

	// We reserve space for temple.dll as early as possible to avoid rebasing of temple.dll
	auto& dll = temple::Dll::GetInstance();
	dll.ReserveMemoryRange();

	int qtArgc = 1;
	char qtArg1[MAX_PATH];
	char *qtArgv[1] = { &qtArg1[0] };
	GetModuleFileNameA(nullptr, qtArg1, MAX_PATH);
	QGuiApplication qtApp(qtArgc, qtArgv);
	// Enables use of ANGLE instead of Desktop GL
	qtApp.setAttribute(Qt::AA_UseOpenGLES);

	// Cannot get known folders without initializing COM sadly
	ComInitializer comInitializer;

	Breakpad breakpad(GetUserDataFolder());

	SetIniPath();

	config.Load();
	config.Save();

	auto logFile = GetUserDataFolder() + L"TemplePlus.log";
	InitLogging(logFile);

	logger->info("Starting Temple Plus");
	logger->info("Version: {}", GetTemplePlusVersion());
	logger->info("Commit: {}", GetTemplePlusCommitId());

	dll.Load(config.toeeDir);

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

static void SetIniPath() {

	auto userDataFolder = GetUserDataFolder();

	auto iniPath = userDataFolder + L"TemplePlus.ini";

	if (!PathFileExists(iniPath.c_str())) {
		// No configuration file exists yet.
		// Do we have a companion configuration utility that we can launch?
		wchar_t configUtilPath[MAX_PATH];
		GetModuleFileNameW(nullptr, configUtilPath, MAX_PATH);
		PathRemoveFileSpec(configUtilPath);
		PathAppend(configUtilPath, L"TemplePlusConfig.exe");

		// Launch the configuration utility if it's missing
		if (PathFileExists(configUtilPath)) {
			STARTUPINFO sInfo;
			ZeroMemory(&sInfo, sizeof(sInfo));
			PROCESS_INFORMATION pInfo;
			ZeroMemory(&pInfo, sizeof(pInfo));
			if (CreateProcess(configUtilPath, nullptr, nullptr, nullptr, FALSE,
				0, nullptr, nullptr, &sInfo, &pInfo)) {
				MsgWaitForMultipleObjects(1, &pInfo.hProcess, FALSE, INFINITE, 0);
				CloseHandle(pInfo.hThread);
				CloseHandle(pInfo.hProcess);
			}
			else {
				MessageBox(
					nullptr,
					L"The configuration file TemplePlus.ini is missing and the configuration utility TemplePlusConfig.exe could not be launched.",
					L"Configuration Error",
					MB_OK | MB_ICONERROR
					);
				exit(-1);
			}

			// If no configuration file exist now, the user probably cancelled
			if (!PathFileExists(iniPath.c_str())) {
				exit(0);
			}
		}
		else {
			MessageBox(
				nullptr,
				L"The configuration file TemplePlus.ini is missing and the configuration utility TemplePlusConfig.exe could not be found.",
				L"Configuration Error",
				MB_OK | MB_ICONERROR
				);
			exit(-1);
		}
	}

	config.SetPath(ucs2_to_local(iniPath));
}
