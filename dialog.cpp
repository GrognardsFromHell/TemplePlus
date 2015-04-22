
#include "stdafx.h"
#include "dialog.h"

DialogScripts dialogScripts;

static struct DialogScriptsAddresses : AddressTable {

	bool (__cdecl *GetFilename)(int scriptId, char *filename);

	bool (__cdecl *LoadScript)(const char *filename, int &handleOut);

	void (__cdecl *LoadNpcLine)(DialogState &state, bool force);

	void (__cdecl *FreeScript)(int handle);

	DialogScriptsAddresses() {
		rebase(GetFilename, 0x1007E270);
		rebase(LoadScript, 0x10036600);
		rebase(LoadNpcLine, 0x10038590);
		rebase(FreeScript, 0x100346F0);
	}
} addresses;

string DialogScripts::GetFilename(int scriptId) {
	char filename[MAX_PATH];
	if (addresses.GetFilename(scriptId, filename)) {
		return filename;
	} else {
		return string();		
	}
}

bool DialogScripts::Load(const string& filename, int& dlgHandle) {
	return addresses.LoadScript(filename.c_str(), dlgHandle);
}

void DialogScripts::LoadNpcLine(DialogState& state, bool force) {
	addresses.LoadNpcLine(state, force);
}

void DialogScripts::Free(int dlgHandle) {
	addresses.FreeScript(dlgHandle);
}
