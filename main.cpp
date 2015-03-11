
#include "stdafx.h"
#include "temple_functions.h"
#include "libraryholder.h"

#include <boost/log/utility/setup.hpp>

void InitLogging();

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

	string localCmdLineStart = "\"" + toeeDir.string() + "toee.exe\" -window";
	auto ourModule = GetModuleHandleW(nullptr);
	int result = temple_main(ourModule, nullptr, localCmdLineStart.data(), SW_SHOWDEFAULT);

	MH_Uninitialize();

	return result;
}

void InitLogging()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);

	boost::log::add_file_log(boost::log::keywords::file_name = "TemplePlus.log", 
		boost::log::keywords::auto_flush = true);
	boost::log::add_console_log(cout);
}
