
#pragma once

#include "tig/tig_startup.h"

#include <functional>

bool CreateMainWindow(TigConfig* settings);

/*
	Creates and controls the Windows API window that will be used to display the game.
	The window is created based on the game configuration.
*/
class MainWindow {
public:
	typedef function<bool(HWND, UINT, WPARAM, LPARAM, LRESULT&)> MsgHandler;

	MainWindow(HINSTANCE hInstance);
	~MainWindow();

	// Gets the Windows API handle for the main window. (nullptr initially)
	HWND GetHwnd() const {
		return mHwnd;
	}

	HINSTANCE GetHInstance() const {
		return mHInstance;
	}

	int GetClientWidth() {
		return mClientWidth;
	}

	int GetClientHeight() {
		return mClientHeight;
	}

	void AddMessageHandler(const MsgHandler &newMsgHandler) {
		mMsgHandlers.push_back(newMsgHandler);
	}

private:
	HWND mHwnd = nullptr;
	HINSTANCE mHInstance;
	ATOM mClass = 0;
	int mClientWidth;
	int mClientHeight;
	ULONG mGdiPlusToken;
	vector<MsgHandler> mMsgHandlers;
	
    // Creates the window class used for the main window
	void RegisterClass(HINSTANCE hInstance);

	// Unregisters the window class used for the main window
	void UnregisterClass();

	void CreateMainWindow(HINSTANCE instance);

	void DestroyMainWindow();

	void CalculateWindowRectAndStyle(RECT &rect, DWORD &style, DWORD &styleEx);

	LRESULT CALLBACK HandleMessage(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);

	// Dispatches the wnd messages to the MainWindow instance associated with the HWND
	static LRESULT CALLBACK HandleMessageDispatcher(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);

};
