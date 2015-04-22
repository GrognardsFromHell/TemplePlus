
#pragma once
#include "obj.h"

struct DialogState {
	int dialogHandle;
	int unk;
	objHndl pc;
	char padding1[40];
	objHndl npc;
	char field_40[40];
	int reqNpcLineId;
	int dialogScriptId; // current known use is for speech
	char npcLineText[1000];
	/*
		If the speaker is a PC, this is just the speech file id.
		If the speaker is a NPC, the upper 16-bit are his dlg script id, the
		lower 16-bit are the voice sample id.
	*/
	int speechId;
	int pcLines;
	char pcLineText1[1000];
	char pcLineText2[1000];
	char pcLineText3[1000];
	char pcLineText4[1000];
	char pcLineText5[1000];
	int field_17E8;
	int field_17EC;
	int field_17F0;
	int field_17F4;
	int field_17F8;
	int field_17FC;
	int lineNumber;
	int field_1804;
	int field_1808;
	int field_180C;
	int field_1810;
	int field_1814;
	int field_1818;
	int field_181C;
	int field_1820;
	int field_1824;
	int field_1828;
	int field_182C;
	int field_1830;
	int field_1834;
	int field_1838;
	int field_183C;
	int field_1840;
	int field_1844;
	int field_1848;
	int field_184C;
	int field_1850;
	int answerLineId;
	int field_1858;
	int field_185C;
};

// Subsystem for handling dialog parsing and logic
class DialogScripts {
public:

	/*
		Retrieves the filename of the dialog scrpt for the given script id.
		Returns an empty string if it fails.
	*/
	string GetFilename(int scriptId);

	/*
		Loads the dialog file identified by the given filename and returns true 
		on success. Stores the handle for the loaded dialog in dlgHandle
	*/
	bool Load(const string &filename, int &dlgHandle);

	/*
		Loads the dialog line specified by reqNpcLineId. 
		If force is false, it will be checked if the NPC can initiate dialog
		with the PC and if not, the line will be set to an empty string and
		the speechId will be set to -1.
	*/
	void LoadNpcLine(DialogState &state, bool force = false);

	/*
		Frees a previously loaded dialog script.
	*/
	void Free(int dlgHandle);

};

extern DialogScripts dialogScripts;