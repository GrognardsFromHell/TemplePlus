#pragma once

#include <string>
#include <vector>

class InstallationDir {
public:
	bool Validate(const std::wstring& path);
	std::wstring Normalize(const std::wstring& path);

	std::vector<std::wstring> FindInstallations();

private:
	std::wstring mHint;

	std::wstring ReadRegistryString(const std::wstring& keyName, const std::wstring& valueName);
};
