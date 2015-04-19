
#include "stdafx.h"
#include "ui_dialog.h"
#include "util/addresses.h"

UiDialog uiDialog;

static struct UiDialogAddresses : AddressTable {
	
	void(__cdecl *Hide)();
	bool(__cdecl *IsActive)();
	UiCurrentDlgInfo* (__cdecl *GetCurrentDlg)();
	void (__cdecl *ReShowDialog)(UiCurrentDlgInfo *info, int line);
	void (__cdecl *Unk)();
	
	UiDialogAddresses() {
		rebase(Hide, 0x1014CA20);
		rebase(IsActive, 0x1014BFE0);
		rebase(GetCurrentDlg, 0x1014BA30);
		rebase(ReShowDialog, 0x100388D0);
		rebase(Unk, 0x1014C8F0);
	}
} addresses;


void UiDialog::Hide() {
	addresses.Hide();
}

bool UiDialog::IsActive() {
	return addresses.IsActive();
}

UiCurrentDlgInfo* UiDialog::GetCurrentDialog() {
	return addresses.GetCurrentDlg();
}

void UiDialog::ReShowDialog(UiCurrentDlgInfo* info, int line) {
	addresses.ReShowDialog(info, line);
}

void UiDialog::Unk() {
	addresses.Unk();
}
