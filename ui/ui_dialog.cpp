
#include "stdafx.h"
#include "ui_dialog.h"
#include "util/addresses.h"

UiDialog uiDialog;

static struct UiDialogAddresses : AddressTable {
	
	void(__cdecl *Hide)();
	bool(__cdecl *IsActive)();
	DialogState* (__cdecl *GetCurrentDlg)();
	void(__cdecl *ReShowDialog)(DialogState *info, int line);
	void (__cdecl *Unk)(); // This is speech related
	int (__cdecl *ShowTextBubble)(objHndl pc, objHndl speakingTo, const char *text, int speechId);
	
	UiDialogAddresses() {
		rebase(Hide, 0x1014CA20);
		rebase(IsActive, 0x1014BFE0);
		rebase(GetCurrentDlg, 0x1014BA30);
		rebase(ReShowDialog, 0x100388D0);
		rebase(Unk, 0x1014C8F0);
		rebase(ShowTextBubble, 0x1014CDE0);
	}
} addresses;


void UiDialog::Hide() {
	addresses.Hide();
}

bool UiDialog::IsActive() {
	return addresses.IsActive();
}

DialogState* UiDialog::GetCurrentDialog() {
	return addresses.GetCurrentDlg();
}

void UiDialog::ReShowDialog(DialogState* info, int line) {
	addresses.ReShowDialog(info, line);
}

void UiDialog::Unk() {
	addresses.Unk();
}

void UiDialog::ShowTextBubble(objHndl speaker, objHndl speakingTo, const string &text, int speechId) {
	addresses.ShowTextBubble(speaker, speakingTo, text.c_str(), speechId);
}
