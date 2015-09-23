#include "stdafx.h"

#include <atlbase.h>
#include <infrastructure/logging.h>

#include "installationdir.h"

bool InstallationDir::Validate(const wstring& path) {

	auto normalizedPath(path);
	
	if (normalizedPath.empty()) {
		return false;
	}

	// Ensure backslash
	if (normalizedPath.back() != L'/' && normalizedPath.back() != L'\\') {
		normalizedPath.append(L"\\");
	}

	// Check for temple.dll
	auto dllPath = normalizedPath + L"temple.dll";
	if (!PathFileExists(dllPath.c_str())) {
		logger->debug("Path {} is invalid because temple.dll is missing", ucs2_to_utf8(normalizedPath));
		return false;
	}

	return true;

}

std::wstring InstallationDir::Normalize(const std::wstring& path) {
	auto normalized(path);
	
	// Ensure backslash
	if (normalized.back() != L'/' && normalized.back() != L'\\') {
		normalized.append(L"\\");
	}
	
	return normalized;
}

std::vector<std::wstring> InstallationDir::FindInstallations() {

	std::vector<std::wstring> result;

	logger->info("ToEE Installation Directories:");

	// Try for current directory
	TCHAR currentDir[MAX_PATH];
	if (GetCurrentDirectory(MAX_PATH, currentDir) != 0) {
		wstring currentDirStr(currentDir);
		logger->info("Current Directory: {}", ucs2_to_utf8(currentDirStr));
		if (Validate(currentDirStr)) {
			result.push_back(Normalize(currentDirStr));
		}
	}

	auto gogPath = ReadRegistryString(L"SOFTWARE\\GOG.com\\Games\\1207658889", L"PATH");
	logger->info("GoG: {}", ucs2_to_utf8(gogPath));

	if (Validate(gogPath)) {
		result.push_back(Normalize(gogPath));
	}

	return result;

}

std::wstring InstallationDir::ReadRegistryString(const std::wstring& keyName, const std::wstring& valueName) {

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
	} else {
		return std::wstring();
	}
}
