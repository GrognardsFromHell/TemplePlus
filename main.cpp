#include "stdafx.h"
#include "temple_functions.h"
#include "libraryholder.h"
#include "fixes.h"
#include "config.h"

#include <psapi.h>

void InitLogging();

// Defined in temple_main.cpp for now
int TempleMain(HINSTANCE hInstance, const wstring& commandLine);

static wstring GetInstallationDir();

string FindConflictingModule();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int showCmd) {
	config.Load();
	config.Save();

	InitLogging();

	HMODULE templeDllHandle = LoadLibraryW(L"temple.dll");
	if (templeDllHandle != reinterpret_cast<HMODULE>(0x10000000)) {
		auto moduleName = FindConflictingModule();

		auto msg = format("Temple.dll has been loaded to a different base address than 0x10000000: 0x{:08x}\nConflicts with: {}", 
			reinterpret_cast<uint32_t>(templeDllHandle), moduleName);

		MessageBoxA(nullptr, msg.c_str(), "Rebase Warning", MB_OK | MB_ICONWARNING);
	}

	try {
		wstring toeeDir = GetInstallationDir();

		// logger->info("Starting up with toee path: " << toeeDir;

		if (!SetCurrentDirectory(toeeDir.c_str())) {
			logger->error("Unable to change current working directory!");
		}
		if (!SetDllDirectory(toeeDir.c_str())) {
			logger->error("Unable to change DLL search directory.");
		}

		// Initialize minhook
		MH_Initialize();

		init_functions();
		init_hooks();

		TempleFixes::apply();

		auto ourModule = GetModuleHandleW(nullptr);
		auto result = TempleMain(ourModule, lpCmdLine);

		MH_Uninitialize();

		config.Save();

		return result;
	} catch (exception& e) {
		string msg = format("Uncaught exception: {}", e.what());
		MessageBoxA(NULL, msg.c_str(), "Fatal Error", MB_OK | MB_ICONERROR);
		return 1;
	}
}

shared_ptr<spdlog::logger> logger;

struct OutputDebugStringSink : public spdlog::sinks::base_sink<std::mutex> {
protected:
	void _sink_it(const spdlog::details::log_msg& msg) override {
		OutputDebugStringA(msg.formatted.c_str());
	}
};

void InitLogging() {

	spdlog::set_level(spdlog::level::debug);

	try {
		// Always log to a file
		DeleteFile(L"TemplePlus.log");
		auto fileSink = make_shared<spdlog::sinks::simple_file_sink_mt>("TemplePlus.log", true);
		auto debugSink = make_shared<OutputDebugStringSink>();
		logger = spdlog::create("core", {fileSink, debugSink});
	} catch (const spdlog::spdlog_ex& e) {
		auto msg = format("Unable to initialize the logging subsystem:\n{}", e.what());
		MessageBoxA(NULL, msg.c_str(), "Logging Error", MB_OK | MB_ICONERROR);
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

string FindConflictingModule() {
	HMODULE hMods[1024];
	DWORD cbNeeded;
	char moduleName[MAX_PATH];

	auto hProcess = GetCurrentProcess();

	string conflicting = "";

	const uint32_t templeImageSize = 0x01EB717E;
	const uint32_t templeDesiredStart = 0x10000000;
	const uint32_t templeDesiredEnd = templeDesiredStart + templeImageSize;

	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
		for (uint32_t i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
			GetModuleFileNameA(hMods[i], moduleName, MAX_PATH);
			MODULEINFO moduleInfo;
			GetModuleInformation(hProcess, hMods[i], &moduleInfo, cbNeeded);
			auto fromAddress = reinterpret_cast<uint32_t>(moduleInfo.lpBaseOfDll);
			auto toAddress = fromAddress + moduleInfo.SizeOfImage;
			logger->debug(" Module {}: 0x{:08x}-0x{:08x}", moduleName, fromAddress, toAddress);

			if (fromAddress <= templeDesiredEnd && toAddress > templeDesiredStart) {
				conflicting = format("{} (0x{:08x}-0x{:08x})", moduleName, fromAddress, toAddress);
			}
		}
	}

	CloseHandle(hProcess);

	return conflicting;
}
