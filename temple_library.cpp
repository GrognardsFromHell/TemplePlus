
#include "stdafx.h"

#include <psapi.h>
#include <atlbase.h>

#include "util/config.h"
#include "temple_library.h"
#include "util/stringutil.h"
#include "util/exception.h"

#define LOAD_TEMPLEDLL_DYNAMIC

#ifndef LOAD_TEMPLEDLL_DYNAMIC

#pragma comment(lib, "TEMPLE")

extern "C"
{
	int __declspec(dllimport) __cdecl temple_main(HINSTANCE hInstance, HINSTANCE hPrevInstance, const char* lpCommandLine, int nCmdShow);
}

static void *templeMainRefHack;
#endif

TempleLibrary::TempleLibrary() {
#ifndef LOAD_TEMPLEDLL_DYNAMIC
	// Even though we are not using this function, we have to force the linker to not optimize our dependency away...
	templeMainRefHack = &temple_main;
#endif

	auto installPath = GetInstallationDir();

	logger->info("Starting up with ToEE path: {}", ToUTF8(installPath));

	if (!SetCurrentDirectory(installPath.c_str())) {
		throw new TempleException("Unable to change current working directory!");
	}
	if (!SetDllDirectory(installPath.c_str())) {
		throw new TempleException("Unable to change DLL search directory.");
	}

	// Load the library
	mTempleDllHandle = LoadLibraryW(L"temple.dll");
	if (mTempleDllHandle != reinterpret_cast<HMODULE>(0x10000000)) {
		auto moduleName = FindConflictingModule();

		auto msg = format("Temple.dll has been loaded to a different base address than 0x10000000: 0x{:08x}\nConflicts with: {}",
			reinterpret_cast<uint32_t>(mTempleDllHandle), moduleName);

		MessageBoxA(nullptr, msg.c_str(), "Rebase Warning", MB_OK | MB_ICONWARNING);
	}
}

TempleLibrary::~TempleLibrary() {
	if (mTempleDllHandle) {
		FreeLibrary(mTempleDllHandle);
	}
}

string TempleLibrary::FindConflictingModule() {
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

wstring TempleLibrary::GetInstallationDir() {
	WCHAR path[MAX_PATH];
#ifndef LOAD_TEMPLEDLL_DYNAMIC
	GetModuleFileName(GetModuleHandle(L"temple"), path, MAX_PATH);

	wstring pathStr(path);

	if (!PathRemoveFileSpec(path)) {
		logger->error("Unable to remove trailing filename in temple.dll path: {}", ToUTF8(pathStr));
	}
#else
	wstring registryPath;

	// Path from config takes priority
	if (!config.toeeDir.empty()) {
		logger->info("Using ToEE directory from config file: {}", config.toeeDir);
		wcscpy_s(path, MAX_PATH, FromUTF8(config.toeeDir).data());
	}
	
	// Try finding the GoG installation
	else if (FindInRegistry(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\GOG.com\\GOGTEMPLEOFELEMENTAEVIL", L"PATH", registryPath)) {
		logger->info("Using ToEE directory found in registry (GoG): {}", ToUTF8(registryPath));
		wcscpy_s(path, MAX_PATH, registryPath.data());
	}
	
#endif

	if (!PathAddBackslash(path)) {
		logger->error("Unable to append the backslash to the end of the installation directory: {}", ToUTF8(wstring(path)));
	}

	return path;
}

bool TempleLibrary::FindInRegistry(HKEY root, const wstring &key, const wstring &value, wstring &path) {
	
	CRegKey regKey;
	if (regKey.Open(root, key.c_str(), KEY_READ) != ERROR_SUCCESS) {
		logger->debug("Cannot open registry key Root: {}, Key: {}", reinterpret_cast<void*>(root), ToUTF8(key));
		return false;
	}
	
	// Get length of the string value
	ULONG valueLength = 0;	
	if (regKey.QueryStringValue(value.c_str(), nullptr, &valueLength) != ERROR_SUCCESS) {
		logger->debug("Cannot determine length of registry value Root: {}, Key: {}, Value: {}", 
			reinterpret_cast<void*>(root), ToUTF8(key), ToUTF8(key));
		return false;
	}

	// Buffer for the return value
	unique_ptr<TCHAR> buffer(new TCHAR[valueLength]);
	if (regKey.QueryStringValue(value.c_str(), buffer.get(), &valueLength) != ERROR_SUCCESS) {
		logger->debug("Cannot get registry value Root: {}, Key: {}, Value: {}",
			reinterpret_cast<void*>(root), ToUTF8(key), ToUTF8(key));
		return false;
	}

	path = buffer.get();
	return true;		
}
