
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
