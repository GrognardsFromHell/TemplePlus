#pragma once

#include <string>

/*
	Allows the user to pick the ToEE installation directory
	and returns it.
*/
class InstallationDirPicker {
public:

	// Returns true if the user did not cancel
	bool PickDirectory();

	// Returns the picked directory after PickDirectory was called and returned true
	const std::wstring& GetDirectory() const {
		return mDirectory;
	}

private:
	std::wstring mDirectory;

};
