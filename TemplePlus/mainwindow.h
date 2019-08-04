
#pragma once

#include <platform/windows.h>
#include <functional>

using MouseMoveHandler = std::function<void(int x, int y, int wheelDelta)>;
using WindowMsgFilter = std::function<bool(UINT msg, WPARAM wparam, LPARAM lparam)>;

class MainWindow {
public:
	
	void CreateHwnd();
	MainWindow(HINSTANCE hInstance);
	~MainWindow();

	HWND GetHwnd() const {
		return mHwnd;
	}

	HINSTANCE GetHinstance() const {
		return mHinstance;
	}

	DWORD GetWidth() const {
		return mWidth;
	}
	
	DWORD GetHeight() const {
		return mHeight;
	}

	// Locks the mouse cursor to this window 
	// if we're in the foreground
	void LockCursor(int x, int y, int w, int h) const;

	void SetMouseMoveHandler(MouseMoveHandler handler) {
		mMouseMoveHandler = handler;
	}

	// Sets a filter that receives a chance at intercepting all window messages
	void SetWindowMsgFilter(WindowMsgFilter filter) {
		mWindowMsgFilter = filter;
	}

private:
	HINSTANCE mHinstance;
	HWND mHwnd;
	DWORD mWidth;
	DWORD mHeight;

	void RegisterWndClass();
	void UnregisterWndClass();
	void CreateWindowRectAndStyles(RECT &rect, DWORD &style, DWORD &styleEx);

	static LRESULT CALLBACK WndProcTrampoline(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);
	void UpdateMousePos(int xAbs, int yAbs, int wheelDelta);
	int ToDirectInputKey(int vk);

	MouseMoveHandler mMouseMoveHandler;
	WindowMsgFilter mWindowMsgFilter;

};
