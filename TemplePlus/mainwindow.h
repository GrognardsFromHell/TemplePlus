
#pragma once

#include <platform/windows.h>
#include <functional>

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
	void LockCursor() const;
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

};
