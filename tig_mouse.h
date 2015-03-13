
#pragma once
#include "addresses.h"

struct MouseFuncs : AddressTable {
	
	int (__cdecl *SetCursor)(int shaderId);
	void (__cdecl *ResetCursor)();

	void rebase(Rebaser rebase) override {
		rebase(SetCursor, 0x101DDDD0);
		rebase(ResetCursor, 0x101DD780);
	}
};

extern MouseFuncs mouseFuncs;

void hook_mouse();
