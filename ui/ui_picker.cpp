
#include "stdafx.h"
#include "ui_picker.h"

UiPicker uiPicker;

static struct PickerAddresses : AddressTable {
	bool(__cdecl *ShowPicker)(const PickerArgs &args, void *callbackArgs);
	bool(__cdecl *FreeCurrentPicker)();

	PickerAddresses() {
		rebase(ShowPicker, 0x101357E0);
		rebase(FreeCurrentPicker, 0x10137680);
	}
} addresses;

void UiPicker::ShowPicker(const PickerArgs& args, void* callbackArgs) {
	addresses.ShowPicker(args, callbackArgs);
}

void UiPicker::FreeCurrentPicker() {
	addresses.FreeCurrentPicker();
}
