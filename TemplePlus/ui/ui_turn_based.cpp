
#include "stdafx.h"

#include "ui_turn_based.h"
#include <temple/dll.h>
#include <util/fixes.h>
#include "common.h"

UiTurnBased uiTurnBased;

struct UiTurnBasedAddresses : temple::AddressTable
{
	int(__cdecl**uiFuncs)(void*, void*);
	int * maxInitiativeSmallPortraits; // when this is exceeded, it switches to mini portraits
	int * maxInitiativeMiniPortraits; // at least in terms of available space in two rows; actual limit seems to be 32
	UiTurnBasedAddresses()
	{
		rebase(uiFuncs, 0x10B3D5D0);
		rebase(maxInitiativeSmallPortraits, 0x102F9AF8);
		rebase(maxInitiativeMiniPortraits, 0x102F9AFC);
	}
	
} addresses;

class UiTurnBasedReplacements: TempleFix
{
public: 


	static int(_cdecl*orgSub_101430B0)();
	static int sub_101430B0();

	void apply() override 
	{
		//orgSub_101430B0 = replaceFunction(0x101430B0, sub_101430B0);
	}
} uiTbReplacements;

int(_cdecl*UiTurnBasedReplacements::orgSub_101430B0)();

int UiTurnBasedReplacements::sub_101430B0()
{
	int result = orgSub_101430B0();
	return result;
}

int UiTurnBased::ShowPicker(void * a, void* b)
{
	if (addresses.uiFuncs[55])
		return addresses.uiFuncs[55](a,b);
	return 1;
}
