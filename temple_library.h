
#pragma once

#include <string>

/*
	Loads the original temple.dll from whatever directory the
	game is installed in. This might not be the same directory
	that TemplePlus has been installed to.
*/
class TempleLibrary {
public:
	TempleLibrary();
	~TempleLibrary();

	void* BaseAddress() const {
		return (void*)mTempleDllHandle;
	}

	int BaseAddressOffset() {
		return (int) mTempleDllHandle - DefaultBaseAddress;
	}

	const int DefaultBaseAddress = 0x10000000;

private:
	HMODULE mTempleDllHandle = nullptr;

	TempleLibrary(TempleLibrary&) = delete;
	TempleLibrary& operator=(TempleLibrary&) = delete;
	
	string FindConflictingModule();
	wstring GetInstallationDir();

	bool FindInRegistry(HKEY root, const wstring &key, const wstring &value, wstring &path);
};
