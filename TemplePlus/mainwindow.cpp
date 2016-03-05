#include "stdafx.h"

#include <graphics/device.h>
#include <platform/windows.h>
#include <windowsx.h>

#include "movies.h"
#include "tig/tig_msg.h"
#include "tig/tig_mouse.h"
#include "config/config.h"
#include "mainwindow.h"
#include "tig/tig_startup.h"

struct WindowFuncs : temple::AddressTable {
	void (*TigSoundSetActive)(BOOL active);

	WindowFuncs() {
		rebase(TigSoundSetActive, 0x101E3EE0);
	}
} windowFuncs;

temple::GlobalPrimitive<uint32_t, 0x10D25C38> globalWwndproc;

/*
	The window class name used for RegisterClass 
	and CreateWindow.
*/
constexpr auto WindowClassName = L"TemplePlusMainWnd";
constexpr auto WindowTitle = L"Temple of Elemental Evil (Temple+)";

MainWindow::MainWindow(HINSTANCE hInstance) : mHinstance(hInstance), mHwnd(nullptr) {
	RegisterWndClass();
	CreateHwnd();
}

MainWindow::~MainWindow() {

	if (mHwnd) {
		DestroyWindow(mHwnd);
	}

	UnregisterWndClass();

}

void MainWindow::LockCursor() const {
	bool isForeground = GetForegroundWindow() == mHwnd;

	auto &device(tig->GetRenderingDevice());

	auto sceneRect = device.GetSceneRect();
	RECT rect {
		(int)sceneRect.x,
		(int)sceneRect.y,
		(int)(sceneRect.x + sceneRect.z),
		(int)(sceneRect.y + sceneRect.w)
	};
	if (isForeground) {
		ClipCursor(&rect);
	}
}

void MainWindow::RegisterWndClass() {

	WNDCLASS wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.style = 0;
	wndClass.lpfnWndProc = WndProcTrampoline;
	wndClass.hInstance = mHinstance;
	wndClass.hIcon = LoadIcon(mHinstance, L"icon");
	wndClass.hCursor = LoadCursor(0, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszClassName = WindowClassName;
	wndClass.cbWndExtra = sizeof(MainWindow*);

	if (!RegisterClass(&wndClass)) {
		throw TempleException("Unable to register window class: {}",
		                      GetLastWin32Error());
	}
}

void MainWindow::UnregisterWndClass() {

	if (!UnregisterClass(WindowClassName, mHinstance)) {
		logger->error("Unable to unregister window class: {}",
		              GetLastWin32Error());
	}

}

void MainWindow::CreateHwnd() {

	// Is not actually used anymore, but messages.c checks for it being != 0
	globalWwndproc = 1; // TODO: Validate necessity

	RECT windowRect;
	DWORD style = 0;
	DWORD styleEx = 0;

	CreateWindowRectAndStyles(windowRect, style, styleEx);

	style |= WS_VISIBLE;

	auto width = windowRect.right - windowRect.left;
	auto height = windowRect.bottom - windowRect.top;
	logger->info("Creating window with dimensions {}x{}", width, height);
	mHwnd = CreateWindowEx(
		styleEx,
		WindowClassName,
		WindowTitle,
		style,
		windowRect.left,
		windowRect.top,
		width,
		height,
		0,
		nullptr,
		mHinstance,
		nullptr);

	if (!mHwnd) {
		throw TempleException("Unable to create main window: {}",
		                      GetLastWin32Error());
	}

	// Store our this pointer in the window
	SetWindowLongPtr(mHwnd, 0, reinterpret_cast<LONG>(this));

}

void MainWindow::CreateWindowRectAndStyles(RECT& windowRect, DWORD& style, DWORD& styleEx) {

	auto screenWidth = GetSystemMetrics(SM_CXSCREEN);
	auto screenHeight = GetSystemMetrics(SM_CYSCREEN);

	styleEx = 0;

	if (!config.windowed) {
		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = screenWidth;
		windowRect.bottom = screenHeight;
		style = WS_POPUP;
		mWidth = screenWidth;
		mHeight = screenHeight;
		if (config.renderWidth == -1 && config.renderHeight == -1){
			config.renderWidth = mWidth;
			config.renderHeight = mHeight;
		}
	} else {
		// Apparently this flag controls whether x,y are preset from the outside
		windowRect.left = (screenWidth - config.windowWidth) / 2;
		windowRect.top = (screenHeight - config.windowHeight) / 2;
		windowRect.right = windowRect.left + config.windowWidth;
		windowRect.bottom = windowRect.top + config.windowHeight;

		style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

		AdjustWindowRect(&windowRect, style, FALSE);
		int extraWidth = (windowRect.right - windowRect.left) - config.windowWidth;
		int extraHeight = (windowRect.bottom - windowRect.top) - config.windowHeight;
		windowRect.left = (screenWidth - config.windowWidth) / 2 - (extraWidth / 2);
		windowRect.top = (screenHeight - config.windowHeight) / 2 - (extraHeight / 2);
		windowRect.right = windowRect.left + config.windowWidth + extraWidth;
		windowRect.bottom = windowRect.top + config.windowHeight + extraHeight;

		mWidth = config.windowWidth;
		mHeight = config.windowHeight;
	}

}

LRESULT MainWindow::WndProcTrampoline(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	// Retrieve our this pointer from the wnd
	auto mainWindow = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, 0));

	if (mainWindow) {
		return mainWindow->WndProc(hWnd, msg, wparam, lparam);
	}

	return DefWindowProc(hWnd, msg, wparam, lparam);

}

LRESULT MainWindow::WndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	if (hWnd != mHwnd || !tig) {
		return DefWindowProcA(hWnd, msg, wparam, lparam);
	}

	static int mousePosX = 0; // Replaces memory @ 10D25CEC
	static int mousePosY = 0; // Replaces memory @ 10D25CF0
	RECT rect;
	TigMsg tigMsg;

	switch (msg) {
	case WM_SETFOCUS:
		// Make our window topmost unless a debugger is attached
		if ((HWND)wparam == mHwnd && !IsDebuggerPresent()) {
			SetWindowPos(mHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
		break;
	case WM_KILLFOCUS:
		// Make our window topmost unless a debugger is attached
		if ((HWND)wparam == mHwnd && !IsDebuggerPresent()) {
			SetWindowPos(mHwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
		break;
	case WM_SETCURSOR:
		SetCursor(nullptr); // Disables default cursor
		if (!movieFuncs.MovieIsPlaying) {
			// TODO: Rip this out, circular dependency
			tig->GetRenderingDevice().GetDevice()->ShowCursor(TRUE);
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
		mouseFuncs.SetMmbReference();
		mouseFuncs.SetButtonState(MouseButton::MIDDLE, true);
		break;
	case WM_MBUTTONUP:
		mouseFuncs.ResetMmbReference();
		mouseFuncs.SetButtonState(MouseButton::MIDDLE, false);
		break;
	case WM_KEYDOWN:
		if (wparam >= VK_HOME && wparam <= VK_DOWN || wparam == VK_DELETE) {
			tigMsg.createdMs = timeGetTime();
			tigMsg.type = TigMsgType::KEYDOWN;
			tigMsg.arg1 = wparam;
			msgFuncs.Enqueue(&tigMsg);
		}

		tigMsg.createdMs = timeGetTime();
		tigMsg.type = TigMsgType::KEYSTATECHANGE;
		tigMsg.arg1 = ToDirectInputKey(wparam);
		tigMsg.arg2 = 1; // Means it has changed to pressed
		if (tigMsg.arg1 != 0) {
			msgFuncs.Enqueue(&tigMsg);
		}
		break;
	case WM_SYSKEYDOWN:
		tigMsg.createdMs = timeGetTime();
		tigMsg.type = TigMsgType::KEYSTATECHANGE;
		tigMsg.arg1 = ToDirectInputKey(wparam);
		tigMsg.arg2 = 1; // Means it has changed to pressed
		if (tigMsg.arg1 != 0) {
			msgFuncs.Enqueue(&tigMsg);
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		tigMsg.createdMs = timeGetTime();
		tigMsg.type = TigMsgType::KEYSTATECHANGE;
		tigMsg.arg1 = ToDirectInputKey(wparam);
		tigMsg.arg2 = 0; // Means it has changed to unpressed
		if (tigMsg.arg1 != 0) {
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
		windowFuncs.TigSoundSetActive(wparam == 1);
		break;
	case WM_CLOSE:
		tigMsg.createdMs = timeGetTime();
		tigMsg.type = TigMsgType::EXIT;
		tigMsg.arg1 = 1;
		msgFuncs.Enqueue(&tigMsg);
		return 0;
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

void MainWindow::UpdateMousePos(int xAbs, int yAbs, int wheelDelta) {
	auto &device(tig->GetRenderingDevice());

	// Move it into the scene rectangle coordinate space
	auto& sceneRect(device.GetSceneRect());
	auto x = xAbs - sceneRect.x;
	auto y = yAbs - sceneRect.y;

	auto orgX = xAbs;
	auto orgY = yAbs;
	
	// Scale it to the coordinate system that was used to render the scene
	xAbs = (int)(x / device.GetSceneScale());
	yAbs = (int)(y / device.GetSceneScale());

	// Account for a resized screen
	if (xAbs < 0 || yAbs < 0 || xAbs >= device.GetRenderWidth() || yAbs >= device.GetRenderHeight())
	{
		if (config.windowed) 
		{
			if ( (xAbs > -7 && xAbs < device.GetRenderWidth() + 7 && yAbs > -7 && yAbs < device.GetRenderHeight() + 7))
			{
				if (xAbs < 0) 
					xAbs = 0;
				else if (xAbs > device.GetRenderWidth()) 
					xAbs = device.GetRenderWidth();
				if (yAbs < 0) 
					yAbs = 0;
				else if (yAbs > device.GetRenderHeight()) 
					yAbs = device.GetRenderHeight();
				mouseFuncs.MouseOutsideWndSet(false);
				mouseFuncs.SetPos(xAbs, yAbs, wheelDelta);
				return;
			} else
			{
				mouseFuncs.MouseOutsideWndSet(true);
			}
		}	
		return;
	}
	mouseFuncs.MouseOutsideWndSet(false);

	mouseFuncs.SetPos(xAbs, yAbs, wheelDelta);
}

int MainWindow::ToDirectInputKey(int vk) {

	// This seems to map using scan codes, which
	// actually look like original US keyboard ones
	auto mapped = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
	if (mapped != 0) {
		return mapped;
	}

	switch (vk) {
	case VK_ESCAPE:
		return 0x01; // DIK_ESCAPE
	case '1':
		return 0x02; // DIK_1
	case '2':
		return 0x03; // DIK_2
	case '3':
		return 0x04; // DIK_3
	case '4':
		return 0x05; // DIK_4
	case '5':
		return 0x06; // DIK_5
	case '6':
		return 0x07; // DIK_6
	case '7':
		return 0x08; // DIK_7
	case '8':
		return 0x09; // DIK_8
	case '9':
		return 0x0A; // DIK_9
	case '0':
		return 0x0B; // DIK_0
	case VK_OEM_MINUS:
		return 0x0C; // DIK_MINUS/* - on main keyboard */
	case VK_OEM_PLUS:
		return 0x0D; // DIK_EQUALS
	case VK_BACK:
		return 0x0E; // DIK_BACK/* backspace */
	case VK_TAB:
		return 0x0F; // DIK_TAB
	case 'Q':
		return 0x10; // DIK_Q
	case 'W':
		return 0x11; // DIK_W
	case 'E':
		return 0x12; // DIK_E
	case 'R':
		return 0x13; // DIK_R
	case 'T':
		return 0x14; // DIK_T
	case 'Y':
		return 0x15; // DIK_Y
	case 'U':
		return 0x16; // DIK_U
	case 'I':
		return 0x17; // DIK_I
	case 'O':
		return 0x18; // DIK_O
	case 'P':
		return 0x19; // DIK_P
	case VK_OEM_4:
		return 0x1A; // DIK_LBRACKET
	case VK_OEM_6:
		return 0x1B; // DIK_RBRACKET
	case VK_RETURN:
		return 0x1C; // DIK_RETURN/* Enter on main keyboard */
	case VK_LCONTROL:
		return 0x1D; // DIK_LCONTROL
	case 'A':
		return 0x1E; // DIK_A
	case 'S':
		return 0x1F; // DIK_S
	case 'D':
		return 0x20; // DIK_D
	case 'F':
		return 0x21; // DIK_F
	case 'G':
		return 0x22; // DIK_G
	case 'H':
		return 0x23; // DIK_H
	case 'J':
		return 0x24; // DIK_J
	case 'K':
		return 0x25; // DIK_K
	case 'L':
		return 0x26; // DIK_L
	case VK_OEM_1:
		return 0x27; // DIK_SEMICOLON
	case VK_OEM_7:
		return 0x28; // DIK_APOSTROPHE
	case VK_OEM_3:
		return 0x29; // DIK_GRAVE/* accent grave */
	case VK_LSHIFT:
		return 0x2A; // DIK_LSHIFT
	case VK_OEM_5:
		return 0x2B; // DIK_BACKSLASH
	case 'Z':
		return 0x2C; // DIK_Z
	case 'X':
		return 0x2D; // DIK_X
	case 'C':
		return 0x2E; // DIK_C
	case 'V':
		return 0x2F; // DIK_V
	case 'B':
		return 0x30; // DIK_B
	case 'N':
		return 0x31; // DIK_N
	case 'M':
		return 0x32; // DIK_M
	case VK_OEM_COMMA:
		return 0x33; // DIK_COMMA
	case VK_OEM_PERIOD:
		return 0x34; // DIK_PERIOD/* . on main keyboard */
	case VK_OEM_2:
		return 0x35; // DIK_SLASH/* / on main keyboard */
	case VK_RSHIFT:
		return 0x36; // DIK_RSHIFT
	case VK_MULTIPLY:
		return 0x37; // DIK_MULTIPLY/* * on numeric keypad */
	case VK_LMENU:
		return 0x38; // DIK_LMENU/* left Alt */
	case VK_SPACE:
		return 0x39; // DIK_SPACE
	case VK_CAPITAL:
		return 0x3A; // DIK_CAPITAL
	case VK_F1:
		return 0x3B; // DIK_F1
	case VK_F2:
		return 0x3C; // DIK_F2
	case VK_F3:
		return 0x3D; // DIK_F3
	case VK_F4:
		return 0x3E; // DIK_F4
	case VK_F5:
		return 0x3F; // DIK_F5
	case VK_F6:
		return 0x40; // DIK_F6
	case VK_F7:
		return 0x41; // DIK_F7
	case VK_F8:
		return 0x42; // DIK_F8
	case VK_F9:
		return 0x43; // DIK_F9
	case VK_F10:
		return 0x44; // DIK_F10
	case VK_NUMLOCK:
		return 0x45; // DIK_NUMLOCK
	case VK_SCROLL:
		return 0x46; // DIK_SCROLL/* Scroll Lock */
	case VK_NUMPAD7:
		return 0x47; // DIK_NUMPAD7
	case VK_NUMPAD8:
		return 0x48; // DIK_NUMPAD8
	case VK_NUMPAD9:
		return 0x49; // DIK_NUMPAD9
	case VK_SUBTRACT:
		return 0x4A; // DIK_SUBTRACT/* - on numeric keypad */
	case VK_NUMPAD4:
		return 0x4B; // DIK_NUMPAD4
	case VK_NUMPAD5:
		return 0x4C; // DIK_NUMPAD5
	case VK_NUMPAD6:
		return 0x4D; // DIK_NUMPAD6
	case VK_ADD:
		return 0x4E; // DIK_ADD/* + on numeric keypad */
	case VK_NUMPAD1:
		return 0x4F; // DIK_NUMPAD1
	case VK_NUMPAD2:
		return 0x50; // DIK_NUMPAD2
	case VK_NUMPAD3:
		return 0x51; // DIK_NUMPAD3
	case VK_NUMPAD0:
		return 0x52; // DIK_NUMPAD0
	case VK_DECIMAL:
		return 0x53; // DIK_DECIMAL/* . on numeric keypad */
	case VK_F11:
		return 0x57; // DIK_F11
	case VK_F12:
		return 0x58; // DIK_F12
	case VK_F13:
		return 0x64; // DIK_F13/*                     (NEC PC98) */
	case VK_F14:
		return 0x65; // DIK_F14/*                     (NEC PC98) */
	case VK_F15:
		return 0x66; // DIK_F15/*                     (NEC PC98) */
	case VK_RCONTROL:
		return 0x9D; // DIK_RCONTROL
	case VK_DIVIDE:
		return 0xB5; // DIK_DIVIDE/* / on numeric keypad */
	case VK_RMENU:
		return 0xB8; // DIK_RMENU/* right Alt */
	case VK_HOME:
		return 0xC7; // DIK_HOME/* Home on arrow keypad */
	case VK_UP:
		return 0xC8; // DIK_UP/* UpArrow on arrow keypad */
	case VK_PRIOR:
		return 0xC9; // DIK_PRIOR/* PgUp on arrow keypad */
	case VK_LEFT:
		return 0xCB; // DIK_LEFT/* LeftArrow on arrow keypad */
	case VK_RIGHT:
		return 0xCD; // DIK_RIGHT/* RightArrow on arrow keypad */
	case VK_END:
		return 0xCF; // DIK_END/* End on arrow keypad */
	case VK_DOWN:
		return 0xD0; // DIK_DOWN/* DownArrow on arrow keypad */
	case VK_NEXT:
		return 0xD1; // DIK_NEXT/* PgDn on arrow keypad */
	case VK_INSERT:
		return 0xD2; // DIK_INSERT/* Insert on arrow keypad */
	case VK_DELETE:
		return 0xD3; // DIK_DELETE/* Delete on arrow keypad */
	case VK_LWIN:
		return 0xDB; // DIK_LWIN/* Left Windows key */
	case VK_RWIN:
		return 0xDC; // DIK_RWIN/* Right Windows key */
	case VK_APPS:
		return 0xDD; // DIK_APPS/* AppMenu key */
	case VK_PAUSE:
		return 0xC5; // DIK_PAUSE
	case VK_SNAPSHOT:
		return 0xB7; // DIK_SYSRQ (print screen)
	default:
		return 0;
	}

}
