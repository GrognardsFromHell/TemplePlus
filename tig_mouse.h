
#pragma once
#include "addresses.h"

enum class MouseButton : uint32_t {
	LEFT = 0,
	RIGHT = 1,
	MIDDLE = 2
};

struct MouseFuncs : AddressTable {
	
	int (__cdecl *SetCursor)(int shaderId);
	void (__cdecl *ResetCursor)();
	void (__cdecl *SetButtonState)(MouseButton button, bool pressed);
	void (__cdecl *SetPos)(int x, int y, int wheelDelta);
	void RefreshCursor();

	void rebase(Rebaser rebase) override {
		rebase(SetCursor, 0x101DDDD0);
		rebase(ResetCursor, 0x101DD780);
		rebase(SetButtonState, 0x101DD1B0);
		rebase(SetPos, 0x101DD070);
	}
};

extern MouseFuncs mouseFuncs;

void hook_mouse();
