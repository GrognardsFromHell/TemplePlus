
#pragma once
#include <temple/dll.h>

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

extern temple::GlobalStruct<TigMouseState, 0x10D25184> mouseState;

struct MouseFuncs : temple::AddressTable {
	
	static int (__cdecl SetCursor)(int shaderId);
	static void(__cdecl ResetCursor)();
	void (__cdecl *SetButtonState)(MouseButton button, bool pressed);
	void (__cdecl *SetPos)(int x, int y, int wheelDelta);
	void RefreshCursor();
	int (__cdecl*GetState)(TigMouseState * mouseState);

	void SetMmbReference()
	{
		if (mmbReference.x == -1 && mmbReference.y == -1)
		{
			mmbReference = GetPos();
		}
	};
	POINT GetMmbReference()
	{
		return mmbReference;
	}
	void ResetMmbReference()
	{
		mmbReference.x = -1;
		mmbReference.y = -1;
	}


	void (__cdecl *SetBounds)(int maxX, int maxY);
	POINT GetPos() {
		return { mouseState->x, mouseState->y };
	}

	void MouseOutsideWndSet(bool state)
	{
		mMouseOutsideWnd = state;
	}
	bool MouseOutsideWndGet()
	{
		return mMouseOutsideWnd;
	}

	void ShowCursor() {
		mouseState->flags &= ~MF_HIDE_CURSOR;
	}
	void HideCursor() {
		mouseState->flags |= MF_HIDE_CURSOR;
	}

	MouseFuncs() {
		rebase(SetButtonState, 0x101DD1B0);
		rebase(SetPos,    0x101DD070);
		rebase(SetBounds, 0x101DD010);
		rebase(GetState,  0x101DDEA0);
		mMouseOutsideWnd = false;
		mmbReference.x = -1;
		mmbReference.y = -1;
	}

	using CursorDrawCallback = std::function<void(int, int)>;

	void SetCursorDrawCallback(CursorDrawCallback callback, uint32_t id);
	uint32_t GetCursorDrawCallbackId() const {
		return mCursorDrawCallbackId;
	}

	void InvokeCursorDrawCallback() const;
	
	void DrawItemUnderCursor() const;

	bool SetDraggedIcon(uint32_t textureId, int centerX, int centerY);

private:
	bool mMouseOutsideWnd;
	POINT mmbReference;

	// Callback called right before the cursor is drawn
	std::function<void(int x, int y)> mCursorDrawCallback;
	// This is sometimes queried by ToEE to check which callback is active
	// It contains the callback function's address
	uint32_t mCursorDrawCallbackId = 0;
};

extern MouseFuncs mouseFuncs;
