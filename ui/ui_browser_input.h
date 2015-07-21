
#pragma once

#include <include/cef_browser.h>

class UiBrowserInput {
public:
	UiBrowserInput(CefRefPtr<CefBrowser> browser);
	~UiBrowserInput();

private:
	CefRefPtr<CefBrowser> browser_;

	bool WndMessageFilter(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);

	void HandleMouseEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	void HandleFocus(bool focus);
	void HandleCaptureLost();
	void HandleKeyEvent(UINT message, WPARAM wParam, LPARAM lParam);
	
	// Utils
	int GetCefMouseModifiers(WPARAM wparam);
	int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam);
	bool IsKeyDown(WPARAM wparam);

	// Mouse state tracking.
	bool mouse_tracking_;
	int last_click_x_;
	int last_click_y_;
	CefBrowserHost::MouseButtonType last_click_button_;
	int last_click_count_;
	double last_click_time_;
};
