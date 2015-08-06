
#include "stdafx.h"
#include "ui_browser_input.h"
#include "../mainwindow.h"

#include <include/cef_browser.h>

#include <windowsx.h>

UiBrowserInput::UiBrowserInput(CefRefPtr<CefBrowser> browser, MainWindow &mainWindow) : browser_(browser), mMainWindow(mainWindow) {

	mouse_tracking_ = true;
	last_click_x_ = 0;
	last_click_y_ = 0;
	last_click_button_ = MBT_LEFT;
	last_click_count_ = 0;
	last_click_time_ = 0;

	// Install our msg filter
	mMainWindow.AddMessageHandler([this](HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam, LRESULT &result) {
		return this->WndMessageFilter(hWnd, msg, wparam, lparam, result);
	});
}

UiBrowserInput::~UiBrowserInput() {
}

bool UiBrowserInput::WndMessageFilter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT &result) {
	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_MOUSELEAVE:
	case WM_MOUSEWHEEL:		
		HandleMouseEvent(hWnd, message, wParam, lParam, result);
		return true;

	case WM_SIZE:
		browser_->GetHost()->WasResized();
		break;

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		HandleFocus(message == WM_SETFOCUS);
		return true;
		
	case WM_CAPTURECHANGED:
	case WM_CANCELMODE:
		HandleCaptureLost();
		return true;

	case WM_SYSCHAR:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
		HandleKeyEvent(message, wParam, lParam);
		return true;
	}
	return false;
}

void UiBrowserInput::HandleMouseEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT &result) {
	LONG currentTime = 0;
	bool cancelPreviousClick = false;

	CefRefPtr<CefBrowserHost> browser_host = browser_->GetHost();

	if (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
		message == WM_MBUTTONDOWN || message == WM_MOUSEMOVE ||
		message == WM_MOUSELEAVE) {
		currentTime = GetMessageTime();
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		cancelPreviousClick =
			(abs(last_click_x_ - x) > (GetSystemMetrics(SM_CXDOUBLECLK) / 2))
			|| (abs(last_click_y_ - y) > (GetSystemMetrics(SM_CYDOUBLECLK) / 2))
			|| ((currentTime - last_click_time_) > GetDoubleClickTime());
		if (cancelPreviousClick &&
			(message == WM_MOUSEMOVE || message == WM_MOUSELEAVE)) {
			last_click_count_ = 0;
			last_click_x_ = 0;
			last_click_y_ = 0;
			last_click_time_ = 0;
		}
	}

	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN: {
		::SetCapture(hwnd);
		::SetFocus(hwnd);
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		CefBrowserHost::MouseButtonType btnType =
			(message == WM_LBUTTONDOWN ? MBT_LEFT : (
			message == WM_RBUTTONDOWN ? MBT_RIGHT : MBT_MIDDLE));
		if (!cancelPreviousClick && (btnType == last_click_button_)) {
			++last_click_count_;
		}
		else {
			last_click_count_ = 1;
			last_click_x_ = x;
			last_click_y_ = y;
		}
		last_click_time_ = currentTime;
		last_click_button_ = btnType;

		CefRefPtr<CefBrowserHost> browser_host = browser_->GetHost();
		if (browser_host) {
			CefMouseEvent mouse_event;
			mouse_event.x = x;
			mouse_event.y = y;
			mouse_event.modifiers = GetCefMouseModifiers(wParam);
			browser_host->SendMouseClickEvent(mouse_event, btnType, false,
				last_click_count_);
		}
	} break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	{
		if (GetCapture() == hwnd)
			ReleaseCapture();

		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		{
			CefBrowserHost::MouseButtonType btnType =
				(message == WM_LBUTTONUP ? MBT_LEFT : (
				message == WM_RBUTTONUP ? MBT_RIGHT : MBT_MIDDLE));
			if (browser_host) {
				CefMouseEvent mouse_event;
				mouse_event.x = x;
				mouse_event.y = y;
				mouse_event.modifiers = GetCefMouseModifiers(wParam);
				browser_host->SendMouseClickEvent(mouse_event, btnType, true,
					last_click_count_);
			}
		}
		break;
	}

	case WM_MOUSEMOVE: {
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		
		if (!mouse_tracking_) {
			// Start tracking mouse leave. Required for the WM_MOUSELEAVE event to
			// be generated.
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hwnd;
			TrackMouseEvent(&tme);
			mouse_tracking_ = true;
		}

		if (browser_host) {
			CefMouseEvent mouse_event;
			mouse_event.x = x;
			mouse_event.y = y;
			mouse_event.modifiers = GetCefMouseModifiers(wParam);
			browser_host->SendMouseMoveEvent(mouse_event, false);
		}
		break;
	}

	case WM_MOUSELEAVE: {
		if (mouse_tracking_) {
			// Stop tracking mouse leave.
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE & TME_CANCEL;
			tme.hwndTrack = hwnd;
			TrackMouseEvent(&tme);
			mouse_tracking_ = false;
		}

		if (browser_host) {
			// Determine the cursor position in screen coordinates.
			POINT p;
			::GetCursorPos(&p);
			::ScreenToClient(hwnd, &p);

			CefMouseEvent mouse_event;
			mouse_event.x = p.x;
			mouse_event.y = p.y;
			mouse_event.modifiers = GetCefMouseModifiers(wParam);
			browser_host->SendMouseMoveEvent(mouse_event, true);
		}
	} break;

	case WM_MOUSEWHEEL:
		if (browser_host) {
			POINT screen_point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			HWND scrolled_wnd = ::WindowFromPoint(screen_point);
			if (scrolled_wnd != hwnd)
				break;

			ScreenToClient(hwnd, &screen_point);
			int delta = GET_WHEEL_DELTA_WPARAM(wParam);

			CefMouseEvent mouse_event;
			mouse_event.x = screen_point.x;
			mouse_event.y = screen_point.y;
			mouse_event.modifiers = GetCefMouseModifiers(wParam);

			browser_host->SendMouseWheelEvent(mouse_event,
				IsKeyDown(VK_SHIFT) ? delta : 0,
				!IsKeyDown(VK_SHIFT) ? delta : 0);
		}
		break;
	}
}

void UiBrowserInput::HandleFocus(bool focus) {
	if (browser_) {
		logger->info("Setting focus: {}", focus);
		browser_->GetHost()->SendFocusEvent(focus);
	}
}

void UiBrowserInput::HandleCaptureLost() {
	if (browser_) {
		browser_->GetHost()->SendCaptureLostEvent();
	}
}

void UiBrowserInput::HandleKeyEvent(UINT message, WPARAM wParam, LPARAM lParam) {

}

int UiBrowserInput::GetCefMouseModifiers(WPARAM wparam) {
	int modifiers = 0;
	if (wparam & MK_CONTROL)
		modifiers |= EVENTFLAG_CONTROL_DOWN;
	if (wparam & MK_SHIFT)
		modifiers |= EVENTFLAG_SHIFT_DOWN;
	if (IsKeyDown(VK_MENU))
		modifiers |= EVENTFLAG_ALT_DOWN;
	if (wparam & MK_LBUTTON)
		modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
	if (wparam & MK_MBUTTON)
		modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
	if (wparam & MK_RBUTTON)
		modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;

	// Low bit set from GetKeyState indicates "toggled".
	if (::GetKeyState(VK_NUMLOCK) & 1)
		modifiers |= EVENTFLAG_NUM_LOCK_ON;
	if (::GetKeyState(VK_CAPITAL) & 1)
		modifiers |= EVENTFLAG_CAPS_LOCK_ON;
	return modifiers;
}

int UiBrowserInput::GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam) {
	int modifiers = 0;
	if (IsKeyDown(VK_SHIFT))
		modifiers |= EVENTFLAG_SHIFT_DOWN;
	if (IsKeyDown(VK_CONTROL))
		modifiers |= EVENTFLAG_CONTROL_DOWN;
	if (IsKeyDown(VK_MENU))
		modifiers |= EVENTFLAG_ALT_DOWN;

	// Low bit set from GetKeyState indicates "toggled".
	if (::GetKeyState(VK_NUMLOCK) & 1)
		modifiers |= EVENTFLAG_NUM_LOCK_ON;
	if (::GetKeyState(VK_CAPITAL) & 1)
		modifiers |= EVENTFLAG_CAPS_LOCK_ON;

	switch (wparam) {
	case VK_RETURN:
		if ((lparam >> 16) & KF_EXTENDED)
			modifiers |= EVENTFLAG_IS_KEY_PAD;
		break;
	case VK_INSERT:
	case VK_DELETE:
	case VK_HOME:
	case VK_END:
	case VK_PRIOR:
	case VK_NEXT:
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
		if (!((lparam >> 16) & KF_EXTENDED))
			modifiers |= EVENTFLAG_IS_KEY_PAD;
		break;
	case VK_NUMLOCK:
	case VK_NUMPAD0:
	case VK_NUMPAD1:
	case VK_NUMPAD2:
	case VK_NUMPAD3:
	case VK_NUMPAD4:
	case VK_NUMPAD5:
	case VK_NUMPAD6:
	case VK_NUMPAD7:
	case VK_NUMPAD8:
	case VK_NUMPAD9:
	case VK_DIVIDE:
	case VK_MULTIPLY:
	case VK_SUBTRACT:
	case VK_ADD:
	case VK_DECIMAL:
	case VK_CLEAR:
		modifiers |= EVENTFLAG_IS_KEY_PAD;
		break;
	case VK_SHIFT:
		if (IsKeyDown(VK_LSHIFT))
			modifiers |= EVENTFLAG_IS_LEFT;
		else if (IsKeyDown(VK_RSHIFT))
			modifiers |= EVENTFLAG_IS_RIGHT;
		break;
	case VK_CONTROL:
		if (IsKeyDown(VK_LCONTROL))
			modifiers |= EVENTFLAG_IS_LEFT;
		else if (IsKeyDown(VK_RCONTROL))
			modifiers |= EVENTFLAG_IS_RIGHT;
		break;
	case VK_MENU:
		if (IsKeyDown(VK_LMENU))
			modifiers |= EVENTFLAG_IS_LEFT;
		else if (IsKeyDown(VK_RMENU))
			modifiers |= EVENTFLAG_IS_RIGHT;
		break;
	case VK_LWIN:
		modifiers |= EVENTFLAG_IS_LEFT;
		break;
	case VK_RWIN:
		modifiers |= EVENTFLAG_IS_RIGHT;
		break;
	}
	return modifiers;
}

bool UiBrowserInput::IsKeyDown(WPARAM wparam) {
	return (GetKeyState(wparam) & 0x8000) != 0;
}
