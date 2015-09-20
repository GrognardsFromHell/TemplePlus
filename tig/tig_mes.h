
#pragma once

#include <temple/dll.h>

typedef uint32_t MesHandle;

struct MesLine {
	uint32_t key;
	const char *value;
};

struct MesFuncs : temple::AddressTable {
	bool (__cdecl *Open)(const char *name, MesHandle *handleOut);
	bool (__cdecl *Close)(MesHandle handle);
	MesLine *(__cdecl *GetLine)(MesHandle handle, MesLine *line);
	MesLine *(__cdecl *GetLine_Safe)(MesHandle handle, MesLine *line); // returns "Error! Missing line %d in %s" if it fails to find it
	const char *GetLineById(MesHandle handle, uint32_t key) {
		MesLine line = { key, nullptr };
		if (GetLine(handle, &line)) {
			return line.value;
		} else {
			return nullptr;
		}
	}

	MesFuncs() {
		rebase(Open, 0x101E6D00);
		rebase(Close, 0x101E6360);
		rebase(GetLine, 0x101E6760);
		rebase(GetLine_Safe, 0x101E65E0);
	}

};
extern MesFuncs mesFuncs;

// Helper for wrapping the MES file functions above
class MesFile {
public:
	MesFile(const char *name) : mValid(false) {
		if (mesFuncs.Open(name, &mHandle)) {
			mValid = true;
		}
	}
	~MesFile() {
		if (mValid) {
			mesFuncs.Close(mHandle);
		}
	}

	bool valid() const {
		return mValid;
	}

	const char *operator[](uint32_t key)
	{
		if (!mValid) {
			return nullptr;
		}
		return mesFuncs.GetLineById(mHandle, key);
	}

	bool GetLine(uint32_t key, const char *&line) {
		line = nullptr;
		if (!mValid) {
			return false;
		}
		MesLine lineArg;
		lineArg.key = key;
		if (mesFuncs.GetLine(mHandle, &lineArg)) {
			line = lineArg.value;
			return true;
		}
		return false;
	}
private:
	MesHandle mHandle;
	bool mValid;
};
