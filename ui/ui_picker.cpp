#include "stdafx.h"
#include "ui_picker.h"

UiPicker uiPicker;

static struct PickerAddresses : AddressTable {
	bool(__cdecl *ShowPicker)(const PickerArgs &args, void *callbackArgs);
	bool(__cdecl *FreeCurrentPicker)();
	uint32_t(__cdecl * sub_100BA030)(objHndl, PickerArgs*);

	PickerAddresses() {
		rebase(ShowPicker, 0x101357E0);
		rebase(FreeCurrentPicker, 0x10137680);
		macRebase(sub_100BA030, 100BA030)
		macRebase(sub_100BA480, 100BA480)
		macRebase(sub_100BA540, 100BA540) // for area range type
		macRebase(sub_100BA6A0, 100BA6A0) // something with conical selection
	}

	uint32_t (__cdecl *sub_100BA540)(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs);
	void (__cdecl *sub_100BA6A0)(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs);
	uint32_t (__cdecl * sub_100BA480)(objHndl objHnd, PickerArgs* pickerArgs);
} addresses;

void UiPicker::ShowPicker(const PickerArgs& args, void* callbackArgs) {
	addresses.ShowPicker(args, callbackArgs);
}

uint32_t UiPicker::sub_100BA030(objHndl objHnd, PickerArgs* pickerArgs)
{
	return addresses.sub_100BA030(objHnd, pickerArgs);
}

void UiPicker::FreeCurrentPicker() {
	addresses.FreeCurrentPicker();
}

uint32_t UiPicker::sub_100BA480(objHndl objHnd, PickerArgs* pickerArgs)
{
	return addresses.sub_100BA480(objHnd, pickerArgs);
}

void UiPicker::sub_100BA6A0(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs)
{
	addresses.sub_100BA6A0(locAndOffsets, pickerArgs);
}

uint32_t UiPicker::sub_100BA540(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs)
{
	return addresses.sub_100BA540(locAndOffsets, pickerArgs);
}