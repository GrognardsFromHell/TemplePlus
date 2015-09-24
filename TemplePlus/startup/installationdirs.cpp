#include "stdafx.h"

#include <atlbase.h>
#include <infrastructure/logging.h>
#include <experimental/filesystem>

#include "installationdirs.h"

std::vector<InstallationDir> InstallationDirs::FindInstallations() {

	std::vector<InstallationDir> result;

	logger->info("ToEE Installation Directories:");

	// Try for current directory
	TCHAR currentDir[MAX_PATH];
	if (GetCurrentDirectory(MAX_PATH, currentDir) != 0) {
		wstring currentDirStr(currentDir);
		logger->info("Current Directory: {}", ucs2_to_utf8(currentDirStr));
		InstallationDir installDir(currentDirStr);
		if (installDir.IsUsable()) {
			result.push_back(installDir);
		}
	}

	auto gogPath = ReadRegistryString(L"SOFTWARE\\GOG.com\\Games\\1207658889", L"PATH");
	logger->info("GoG: {}", ucs2_to_utf8(gogPath));

	InstallationDir gogDir(gogPath);
	if (gogDir.IsUsable()) {
		result.push_back(gogDir);
	}

	return result;

}

std::wstring InstallationDirs::ReadRegistryString(const std::wstring& keyName, const std::wstring& valueName) {

	CRegKey key;
	auto result = key.Open(HKEY_LOCAL_MACHINE, keyName.c_str(), KEY_WOW64_32KEY | KEY_READ);

	if (result != ERROR_SUCCESS) {
		return std::wstring();
	}

	// Query the length
	ULONG length = 0;
	result = key.QueryStringValue(valueName.c_str(), nullptr, &length);
	if (result != ERROR_SUCCESS) {
		return std::wstring();
	}

	std::unique_ptr<wchar_t[]> buffer;
	do {
		buffer.reset(new wchar_t[length]);
		result = key.QueryStringValue(valueName.c_str(), buffer.get(), &length);
	} while (result == ERROR_MORE_DATA);

	if (result == ERROR_SUCCESS) {
		return buffer.get();
	}

	return std::wstring();
}
