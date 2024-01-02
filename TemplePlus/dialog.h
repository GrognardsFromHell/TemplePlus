
#pragma once
#include "obj.h"
#include "gametime.h"

#define DIALOG_REPLIES_MAX (5)
struct DialogLineNew;

struct DialogMini
{
	int flags; // 1 - is PC line
	const char* lineText;
	DialogMini *prev;
	DialogMini *next;
};

struct DialogState {
	int dialogHandle;
	int unk;
	objHndl pc;
	char padding1[40]; // TimeEventObjInfo for PC
	objHndl npc;
	char field_40[40]; // TimeEventObjInfo for NPC
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
	int pcLineSkillUse[5]; // 0 - none, 1 - bluff, 2 -diplo, 3 - intimidate, 4 - sense motive, 5 - gather info
	/*
	   determined by the reply Op Code
		0 - normal
		1 - exit 
		2 - barter
		3 - Exit with bubble
		7 - something deprecated I think
		4,5,6,8 - ??
	 */
	int actionType;
	int lineNumber;
	/*
	     for regular lines: depends on NPC response ID to the line.
				answer ID = 0 then 1 (exit);  
				answer ID > 0 then 0 (normal go to line); 
				answer ID < 0 then 2 (normal go to line, but skip effect)
		 for barter lines : 3 (26 if NPC has to sell equipment first)
		 for rumor lines  : 8
	 */
	int pcReplyOpcode[5]; 
	int npcReplyIds[5]; // the ID of the NPC response to each PC line
	int field_182C[5];     // I'm guessing this was the test field for each line, but no reason to replicate it here since these are already compiled from actually possible responses
	char* effectFields[5]; // python commands to run for each line
	int answerLineId;
	int rngSeed;
	int field_185C;
};
struct DialogLine
{
	int key;
	char *txt;
	char *genderField; // set to -1 for PC lines
	int minIq;         // 0 for NPC lines
	char *testField;  // condition script that determines whether to display the line
	int answerLineId; // NPC line to display next
	char *effectField;// script line to run

	bool IsPcLine();
	bool IsNpcLine();

	DialogLine & operator =(DialogLineNew&);
};

struct DialogLineNew
{
	int key;
	std::string txt;
	std::string genderField; // set to -1 for PC lines
	int minIq;         // 0 for NPC lines
	std::string testField;  // condition script that determines whether to display the line
	int answerLineId; // NPC line to display next
	std::string effectField;// script line to run
	
	bool IsPcLine();
	bool IsNpcLine();

};

struct DialogFile
{
	char filename[260];
	int refCount;
	GameTime lastTimeLoaded;
	int lineCount;
	int lineCapacity;
	DialogLine* lines;
	int unk11C;
	void GetLinesFromFile();
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

	/*
	 retrieves a dialog file entry by handle
	 */
	DialogFile *GetDialogFileEntry(int dlgHandle);

	void SaveToFile(int dlgHandle);

protected:
	bool GetFileEntryByFilename(const std::string &filename, int &dlgHandleOut);
};

extern DialogScripts dialogScripts;