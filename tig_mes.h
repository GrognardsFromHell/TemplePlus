
#pragma once

#include "addresses.h"

typedef uint32_t MesHandle;

struct MesLine {
	uint32_t key;
	const char *value;
};

struct MesFuncs : public AddressTable {
	bool (__cdecl *Open)(const char *name, MesHandle *handleOut);
	bool (__cdecl *Close)(MesHandle handle);
	MesLine *(__cdecl *GetLine)(MesHandle handle, MesLine *line);
	const char *GetLineById(MesHandle handle, uint32_t key) {
		MesLine line = { key, nullptr };
		if (GetLine(handle, &line)) {
			return line.value;
		} else {
			return nullptr;
		}
	}

	void rebase(Rebaser rebase) override {
		rebase(Open, 0x101E6D00);
		rebase(Close, 0x101E6360);
		rebase(GetLine, 0x101E6760);
	}

};
extern MesFuncs mesFuncs;
