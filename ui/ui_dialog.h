
#pragma once
#include <obj.h>

#pragma pack(push, 1)
struct UiCurrentDlgInfo {
	int slotIdx;
	int padding;
	objHndl npc;
	int padding2[10];
	objHndl pc;
	int padding3[10];
	int unk;
	int scriptNo;
};
#pragma pack(pop)

class UiDialog {
public:

	void Hide();

	bool IsActive();

	// this seems to be mostly internal for the python based picker
	UiCurrentDlgInfo *GetCurrentDialog();
	void ReShowDialog(UiCurrentDlgInfo *info, int line);
	void Unk();

};

extern UiDialog uiDialog;
