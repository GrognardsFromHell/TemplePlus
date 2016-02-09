#include "stdafx.h"
#include "ui_picker.h"
#include <maps.h>
#include <temple_functions.h>
#include <tig/tig_loadingscreen.h>
#include "ui_intgame_select.h"

UiPicker uiPicker;

static struct PickerAddresses : temple::AddressTable {
	bool(__cdecl *ShowPicker)(const PickerArgs &args, void *callbackArgs);
	bool(__cdecl *FreeCurrentPicker)();
	uint32_t(__cdecl * sub_100BA030)(objHndl, PickerArgs*);
	int * activePickerIdx; 
	BOOL(__cdecl* PickerActiveCheck)();

	PickerAddresses() {
		rebase(ShowPicker, 0x101357E0);
		rebase(PickerActiveCheck, 0x10135970);
		rebase(FreeCurrentPicker, 0x10137680);
		
		rebase(activePickerIdx, 0x102F920C);
		macRebase(sub_100BA030, 100BA030)
		macRebase(sub_100BA480, 100BA480)
		macRebase(sub_100BA540, 100BA540) // for area range type
		macRebase(sub_100BA6A0, 100BA6A0) // something with conical selection
	}

	uint32_t (__cdecl *sub_100BA540)(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs);
	void (__cdecl *sub_100BA6A0)(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs);
	uint32_t (__cdecl * sub_100BA480)(objHndl objHnd, PickerArgs* pickerArgs);
} addresses;

BOOL UiPicker::PickerActiveCheck()
{
	return (*addresses.activePickerIdx) >= 0;
}

int UiPicker::ShowPicker(const PickerArgs& args, void* callbackArgs) {
	//if (maps.GetCurrentMapId() == 5118 ) // tutorial map
	//{
	//	if (args.spellEnum == 288)
	//	{
	//		
	//	} else if (args.spellEnum == 171)
	//	{
	//		
	//	}
	//}
	//if (*addresses.activePickerIdx >= 32)
	//	return 0;
	//ui.WidgetSetHidden(uiIntgameSelect.GetId(), 0);
	//(*addresses.activePickerIdx)++;
	//if (*addresses.activePickerIdx >= 32 || *addresses.activePickerIdx < 0)
	//	return 0;
	//
	
	return addresses.ShowPicker(args, callbackArgs);
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