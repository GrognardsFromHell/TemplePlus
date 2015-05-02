
#pragma once
#include "util/addresses.h"

enum class MouseButton : uint32_t {
	LEFT = 0,
	RIGHT = 1,
	MIDDLE = 2
};

enum TigMouseFlags {
	MF_HIDE_CURSOR = 1
};

struct TigMouseState {
	int flags;
	int mouseCenterX;
	int mouseCenterY;
	int cursorTexWidth;
	int cursorTexHeight;
	int cursorHotspotX;
	int cursorHotspotY;
	int x;
	int y;
	int field24;
};

extern GlobalStruct<TigMouseState, 0x10D25184> mouseState;

struct MouseFuncs : AddressTable {
	
	static int (__cdecl SetCursor)(int shaderId);
	static void(__cdecl ResetCursor)();
	void (__cdecl *SetButtonState)(MouseButton button, bool pressed);
	void (__cdecl *SetPos)(int x, int y, int wheelDelta);
	void RefreshCursor();
	void (__cdecl *SetBounds)(int maxX, int maxY);
	POINT GetPos() {
		return { mouseState->x, mouseState->y };
	}

	void ShowCursor() {
		mouseState->flags &= ~MF_HIDE_CURSOR;
	}
	void HideCursor() {
		mouseState->flags |= MF_HIDE_CURSOR;
	}

	void DrawCursor();

	MouseFuncs() {
		rebase(SetButtonState, 0x101DD1B0);
		rebase(SetPos, 0x101DD070);
		rebase(SetBounds, 0x101DD010);
	}
};

extern MouseFuncs mouseFuncs;

void hook_mouse();
