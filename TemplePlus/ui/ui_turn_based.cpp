#include "stdafx.h";
#include "ui_turn_based.h";
#include <temple/dll.h>

UiTurnBased uiTurnBased;

struct UiTurnBasedAddresses : temple::AddressTable
{
	int(__cdecl**uiFuncs)(void*, void*);
	UiTurnBasedAddresses()
	{
		rebase(uiFuncs, 0x10B3D5D0);
	}
	
} addresses;

int UiTurnBased::ShowPicker(void * a, void* b)
{
	if (addresses.uiFuncs[55])
		return addresses.uiFuncs[55](a,b);
	return 1;
}