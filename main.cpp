
#include "system.h"
#include "temple_functions.h"
#include "libraryholder.h"

#include <boost/program_options.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>

namespace po = boost::program_options;

void InitLogging();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int showCmd)
{	
	WCHAR toeeDllFilename[MAX_PATH];
	GetModuleFileName(GetModuleHandle(L"temple"), toeeDllFilename, MAX_PATH);
	wpath toeeDir(wpath(toeeDllFilename).parent_path());
	
	InitLogging();

	LOG(info) << "Starting up with toee path: " << toeeDir;
	
	boost::system::error_code errorCode;
	current_path(toeeDir, errorCode);
    
    // Set DLL search path and current directory
    if (errorCode) {
        LOG(error) << "Unable to set current directory.";
    }
    if (!SetDllDirectoryW(toeeDir.wstring().data())) {
        LOG(error) << "Unable to change DLL search directory.";
    }

    // Initialize minhook
    MH_Initialize();
	
    init_functions();
    init_hooks();
	
    string localCmdLineStart = "\"" + toeeDir.string() + "toee.exe\"";
    auto ourModule = GetModuleHandleW(NULL);
    int result = temple_main(ourModule, NULL, localCmdLineStart.data(), SW_SHOWDEFAULT);

    MH_Uninitialize();

	return result;
}

void InitLogging() {
	AllocConsole();
	freopen("CONOUT$", "w", stdout);

	//boost::log::add_file_log("sample.log");
	// boost::log::add_console_log();
}
