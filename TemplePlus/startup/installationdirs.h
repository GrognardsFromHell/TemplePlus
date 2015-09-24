#pragma once

#include <vector>
#include <string>

#include "installationdir.h"

class InstallationDirs {
public:

	static std::vector<InstallationDir> FindInstallations();

private:

	static std::wstring ReadRegistryString(const std::wstring& keyName,
	                                       const std::wstring& valueName);
};
