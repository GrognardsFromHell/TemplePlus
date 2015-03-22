
#include "stdafx.h"
#include "temple_functions.h"
#include "libraryholder.h"
#include "fixes.h"
#include "config.h"

#include <boost/log/utility/setup.hpp>

void InitLogging();

// Defined in temple_main.cpp for now
int TempleMain(HINSTANCE hInstance, const wstring &commandLine);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int showCmd)
{
	InitLogging();
	
	WCHAR toeeDllFilename[MAX_PATH];
	GetModuleFileName(GetModuleHandle(L"temple"), toeeDllFilename, MAX_PATH);
	wpath toeeDir(wpath(toeeDllFilename).parent_path());

	LOG(info) << "Starting up with toee path: " << toeeDir;

	system::error_code errorCode;
	current_path(toeeDir, errorCode);

	// Set DLL search path and current directory
	if (errorCode)
	{
		LOG(error) << "Unable to set current directory.";
	}
	if (!SetDllDirectoryW(toeeDir.wstring().data()))
	{
		LOG(error) << "Unable to change DLL search directory.";
	}

	// Initialize minhook
	MH_Initialize();

	init_functions();
	init_hooks();

	TempleFixes::apply();
	
	// this forces us to be REALLY linked against temple.dll If we do not reference anything from it, the ref will be optimized out
	bool dontChangeMe = false;
	if (dontChangeMe) {
		temple_main(nullptr, nullptr, nullptr, 0);
	}

	auto ourModule = GetModuleHandleW(nullptr);
	auto result = TempleMain(ourModule, lpCmdLine);

	MH_Uninitialize();

	return result;
}

void InitLogging()
{
	if (config.showConsoleWindow) {
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		boost::log::add_console_log(cout);
	}

	boost::log::add_file_log(boost::log::keywords::file_name = "TemplePlus.log", 
		boost::log::keywords::auto_flush = true);	
}
