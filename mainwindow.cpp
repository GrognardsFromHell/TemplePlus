#include "stdafx.h"

#include <windowsx.h>

#include "util/fixes.h"
#include "graphics.h"
#include "mainwindow.h"
#include "movies.h"
#include "tig/tig_startup.h"
#include "tig/tig_msg.h"
#include "tig/tig_mouse.h"

LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);

/*
	Indicates that incoming events are due to window creation.
*/
static bool creatingWindow = false;

struct WindowFuncs : AddressTable {
	void (__cdecl *WindowActivationChanged)(bool active);

	WindowFuncs() {
		rebase(WindowActivationChanged, 0x101DF4E0);
	}
} windowFuncs;

GlobalPrimitive<uint32_t, 0x10D25C38> globalWwndproc;

MessageFilterFn messageFilter = nullptr;

void SetMessageFilter(MessageFilterFn filter) {
	messageFilter = filter;
}

bool CreateMainWindow(TigConfig* settings) {
	bool windowed = config.windowed;
	bool unknownFlag = (settings->flags & 0x100) != 0;

	video->hinstance = settings->hinstance;

	// Is not actually used anymore, but messages.c checks for it being != 0
	globalWwndproc = 1;

	WNDCLASSA wndClass;
	ZeroMemory(&wndClass, sizeof(WNDCLASSA));
	wndClass.style = CS_DBLCLKS;
	wndClass.lpfnWndProc = MainWindowProc;
	wndClass.hInstance = video->hinstance;
	wndClass.hIcon = LoadIconA(video->hinstance, "icon");
	wndClass.hCursor = LoadCursorA(0, MAKEINTRESOURCEA(IDC_ARROW));
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszClassName = "TIGClass";

	if (!RegisterClassA(&wndClass)) {
		return false;
	}

	auto screenWidth = GetSystemMetrics(SM_CXSCREEN);
	auto screenHeight = GetSystemMetrics(SM_CYSCREEN);

	RECT windowRect;
	HMENU menu;
	DWORD dwStyle;
	DWORD dwExStyle;

	if (!windowed) {
		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = screenWidth;
		windowRect.bottom = screenHeight;
		menu = 0;
		dwStyle = WS_POPUP;
		// This is bad for debugging
		if (!IsDebuggerPresent()) {
			dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
		} else {
			dwExStyle = 0;
		}
		memcpy(&video->screenSizeRect, &windowRect, sizeof(RECT));
	} else {
		// Apparently this flag controls whether x,y are preset from the outside
		if (unknownFlag) {
			windowRect.left = settings->x;
			windowRect.top = settings->y;
			windowRect.right = settings->x + config.windowWidth;
			windowRect.bottom = settings->y + config.windowHeight;
		} else {
			windowRect.left = (screenWidth - settings->width) / 2;
			windowRect.top = (screenHeight - settings->height) / 2;
			windowRect.right = windowRect.left + config.windowWidth;
			windowRect.bottom = windowRect.top + config.windowHeight;
		}

		menu = NULL;
		dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		dwExStyle = 0;

  		AdjustWindowRect(&windowRect, dwStyle, FALSE);
		int extraWidth = (windowRect.right - windowRect.left) - config.windowWidth;
		int extraHeight = (windowRect.bottom - windowRect.top) - config.windowHeight;
		windowRect.left = (screenWidth - config.windowWidth) / 2 - (extraWidth / 2);
		windowRect.top = (screenHeight - config.windowHeight) / 2 - (extraHeight / 2);
		windowRect.right = windowRect.left + config.windowWidth + extraWidth;
		windowRect.bottom = windowRect.top + config.windowHeight + extraHeight;
	}

	dwStyle |= WS_VISIBLE;
	video->width = settings->width;
	video->height = settings->height;

	temple_set<0x10D24E0C>(0);
	temple_set<0x10D24E10>(0);
	temple_set<0x10D24E14>(settings->width);

	string windowTitle = "Temple of Elemental Evil - Co8";

	DWORD windowWidth = windowRect.right - windowRect.left;
	DWORD windowHeight = windowRect.bottom - windowRect.top;
	logger->info("Creating window with dimensions {}x{}", windowWidth, windowHeight);
	video->hwnd = CreateWindowExA(
		dwExStyle,
		"TIGClass",
		windowTitle.c_str(),
		dwStyle,
		windowRect.left,
		windowRect.top,
		windowWidth,
		windowHeight,
		0,
		menu,
		settings->hinstance,
		0);

	if (video->hwnd) {
		RECT clientRect;
		GetClientRect(video->hwnd, &clientRect);
		
		auto w = clientRect.right - clientRect.left;
		auto h = clientRect.bottom - clientRect.top;
		graphics.UpdateWindowSize(w, h);

		// Scratchbuffer size sometimes doesn't seem to be set by ToEE itself
		video->current_width = config.renderWidth;
		video->current_height = config.renderHeight;
		temple_set<0x10307284>(video->current_width);
		temple_set<0x10307288>(video->current_height);

		return true;
	}
	
	return false;
}

static void UpdateMousePos(int xAbs, int yAbs, int wheelDelta) {
	auto rect = graphics.sceneRect();
	int sw = rect.right - rect.left;
	int sh = rect.bottom - rect.top;
	
	// Make the mouse pos relative to the scene rectangle
	xAbs -= rect.left;
	yAbs -= rect.top;

	// MOve it into the scene rectangle coordinate space
	xAbs = (int) round(xAbs / graphics.sceneScale());
	yAbs = (int) round(yAbs / graphics.sceneScale());

	// Account for a resized screen
	if (xAbs < 0 || yAbs < 0 || xAbs >= sw || yAbs >= sh)
		return;

	mouseFuncs.SetPos(xAbs, yAbs, wheelDelta);
}

static LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	static int mousePosX = 0; // Replaces memory @ 10D25CEC
	static int mousePosY = 0; // Replaces memory @ 10D25CF0
	RECT rect;
	TigMsg tigMsg;

	if (messageFilter && messageFilter(hWnd, msg, wparam, lparam)) {
		return DefWindowProcA(hWnd, msg, wparam, lparam);
	}

	switch (msg) {
	case WM_SETCURSOR:
		SetCursor(nullptr); // Disables default cursor
		if (!movieFuncs.MovieIsPlaying) {
			video->d3dDevice->delegate->ShowCursor(TRUE);
		}
		return TRUE; // This prevents windows from setting the default cursor for us
	case WM_LBUTTONDOWN:
		mouseFuncs.SetButtonState(MouseButton::LEFT, true);
		break;
	case WM_LBUTTONUP:
		mouseFuncs.SetButtonState(MouseButton::LEFT, false);
		break;
	case WM_RBUTTONDOWN:
		mouseFuncs.SetButtonState(MouseButton::RIGHT, true);
		break;
	case WM_RBUTTONUP:
		mouseFuncs.SetButtonState(MouseButton::RIGHT, false);
		break;
	case WM_MBUTTONDOWN:
		mouseFuncs.SetButtonState(MouseButton::MIDDLE, true);
		break;
	case WM_MBUTTONUP:
		mouseFuncs.SetButtonState(MouseButton::MIDDLE, false);
		break;
	case WM_KEYDOWN:
		if (wparam >= VK_HOME && wparam <= VK_DOWN || wparam == VK_DELETE) {
			tigMsg.createdMs = timeGetTime();
			tigMsg.type = TigMsgType::KEYDOWN;
			tigMsg.arg1 = wparam;
			msgFuncs.Enqueue(&tigMsg);
		}
		break;
	case WM_CHAR:
		tigMsg.createdMs = timeGetTime();
		tigMsg.type = TigMsgType::CHAR;
		tigMsg.arg1 = wparam;
		msgFuncs.Enqueue(&tigMsg);
		break;
	case WM_SYSCOMMAND:
		if (wparam == SC_KEYMENU || wparam == SC_SCREENSAVE || wparam == SC_MONITORPOWER) {
			return 0;
		}
		break;
	case WM_ACTIVATEAPP:
		windowFuncs.WindowActivationChanged(wparam == 1);
		break;
	case WM_CLOSE:
		tigMsg.createdMs = timeGetTime();
		tigMsg.type = TigMsgType::EXIT;
		tigMsg.arg1 = 1;
		msgFuncs.Enqueue(&tigMsg);
		return 0;
		// Does not seem to be used:
		/*case WM_CREATE:
			GetClientRect(hWnd, &rect);
			ClientToScreen(hWnd, (LPPOINT)&rect);
			ClientToScreen(hWnd, (LPPOINT)&rect.right);
			refresh_screen_rect(&rect);
			break;			
		case WM_MOVE:
			GetClientRect(hWnd, &rect);
			ClientToScreen(hWnd, (LPPOINT)&rect);
			ClientToScreen(hWnd, (LPPOINT)&rect.right);
			refresh_screen_rect(&rect);
			get_window_rect(&rect);
			nullsub_1();
			goto LABEL_40;*/
	case WM_ERASEBKGND:
		return 0;
	case WM_MOUSEWHEEL:
		GetWindowRect(hWnd, &rect);
		UpdateMousePos(
			GET_X_LPARAM(lparam) - rect.left,
			GET_Y_LPARAM(lparam) - rect.top,
			GET_WHEEL_DELTA_WPARAM(wparam)
			);
		break;
	case WM_MOUSEMOVE:
		mousePosX = GET_X_LPARAM(lparam);
		mousePosY = GET_Y_LPARAM(lparam);
		UpdateMousePos(mousePosX, mousePosY, 0);
		break;
	default:
		break;
	}

	if (msg != WM_KEYDOWN) {
		UpdateMousePos(mousePosX, mousePosY, 0);
	}

	// Previously, ToEE called a global window proc here but it did nothing useful.
	return DefWindowProcA(hWnd, msg, wparam, lparam);
}
