
#pragma once

#include <temple/dll.h>
#include <map>

typedef int MesHandle;

struct MesLine {
	uint32_t key;
	const char *value;
	MesLine();
	MesLine(uint32_t key);
	MesLine(uint32_t key, const char* line);
};

struct MesFuncs : temple::AddressTable {
	bool (__cdecl *Open)(const char *name, MesHandle *handleOut);
	bool (__cdecl *Close)(MesHandle handle);
	void(__cdecl* CopyContents)(MesHandle tgt, MesHandle src); // copies contents of src to tgt. duplicate handling: only checks for duplicates

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

	void AddToMap(MesHandle openedMesHandle, std::map<int, std::string> & mesMap, int *highestKey = nullptr);
	void AddToMap(std::string &mesFilename, std::map<int, std::string> & mesMap, int *highestKey = nullptr);

	MesFuncs() {
		rebase(Open, 0x101E6D00);
		rebase(Close, 0x101E6360);
		rebase(CopyContents, 0x101E6890);
		rebase(GetLine, 0x101E6760);
		rebase(GetLine_Safe, 0x101E65E0);
	}

	int GetNumLines(MesHandle mesHandle);
	/*
	// reads a line based on its ordinal rather than its entry index
	e.g. if the second line is index {1000}, you would use lineCount = 2 to retrieve it this way
	*/
	BOOL ReadLineDirect(MesHandle mesHandle, int lineCount, MesLine* mesLine);
	bool GetFirstLine(MesHandle mesHandle, MesLine* lineOut);
};
extern MesFuncs mesFuncs;
