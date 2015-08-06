#include "stdafx.h"

#include <windowsx.h>

#include "util/fixes.h"
#include "graphics.h"
#include "mainwindow.h"
#include "movies.h"
#include "tig/tig_startup.h"
#include "tig/tig_msg.h"
#include "tig/tig_mouse.h"
#include "templeplus.h"

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

bool CreateMainWindow(TigConfig* settings) {
	video->hinstance = settings->hinstance;

	// Is not actually used anymore, but messages.c checks for it being != 0
	globalWwndproc = 1;

	auto hwnd = tp->mainWindow().GetHwnd();
	video->hwnd = hwnd;
	RECT rect;
	GetClientRect(hwnd, &rect);

	memcpy(&video->screenSizeRect, &rect, sizeof(RECT));

	video->width = settings->width;
	video->height = settings->height;

	temple_set<0x10D24E0C>(0);
	temple_set<0x10D24E10>(0);
	temple_set<0x10D24E14>(settings->width);

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

MainWindow::MainWindow(HINSTANCE instance) : mHInstance(instance)
{
	RegisterClass(instance);
	CreateMainWindow(instance);
}

MainWindow::~MainWindow()
{
	DestroyMainWindow();
	UnregisterClass();
}

void MainWindow::RegisterClass(HINSTANCE instance)
{
	WNDCLASS wndClass;
	ZeroMemory(&wndClass, sizeof(WNDCLASS));
	wndClass.style = CS_DBLCLKS;
	wndClass.lpfnWndProc = MainWindow::HandleMessageDispatcher;
	wndClass.hInstance = instance;
	wndClass.hIcon = LoadIcon(instance, TEXT("icon"));
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszClassName = L"TemplePlusWnd";
	wndClass.cbWndExtra = sizeof(MainWindow*); // We store our this ptr in the HWND

	mClass = ::RegisterClass(&wndClass);

	if (!mClass) {
		throw new TempleException("Unable to register Window Class");
	}
}

void MainWindow::UnregisterClass()
{
	if (mClass) {
		::UnregisterClass(MAKEINTATOM(mClass), nullptr);
	}
}

void MainWindow::CreateMainWindow(HINSTANCE instance)
{
	DWORD style, styleEx;

	RECT windowRect;
	CalculateWindowRectAndStyle(windowRect, style, styleEx);

	style |= WS_VISIBLE;

	LPTSTR windowTitle = TEXT("Temple of Elemental Evil - Co8");

	auto windowWidth = windowRect.right - windowRect.left;
	auto windowHeight = windowRect.bottom - windowRect.top;
	logger->info("Creating window with dimensions {}x{}", windowWidth, windowHeight);
	
	mHwnd = CreateWindowEx(
		styleEx,
		MAKEINTATOM(mClass),
		windowTitle,
		style,
		windowRect.left,
		windowRect.top,
		windowWidth,
		windowHeight,
		nullptr, // No parent
		nullptr, // No menu
		instance,
		this // Pointer to this
	);

	if (!mHwnd) {
		throw new TempleException("Unable to create the TemplePlus main window");
	}

	// Process all outstanding messages
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

}

void MainWindow::DestroyMainWindow()
{
	if (mHwnd) {
		DestroyWindow(mHwnd);
		mHwnd = nullptr;
	}
}

void MainWindow::CalculateWindowRectAndStyle(RECT &windowRect, DWORD &style, DWORD &styleEx)
{
	// Required for both fullscreen size and centering a window
	auto screenWidth = GetSystemMetrics(SM_CXSCREEN);
	auto screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (!config.windowed) {
		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = screenWidth;
		windowRect.bottom = screenHeight;
		style = WS_POPUP;
		
		/*
			App Window and Topmost leads to the window not being able to be tabbed away
			from while it's being debugged. To make debugging easier, we disable this behaviour.
		*/
		if (!IsDebuggerPresent()) {
			styleEx = WS_EX_APPWINDOW | WS_EX_TOPMOST;
		} else {
			styleEx = 0;
		}
	} else {
		windowRect.left = (screenWidth - config.windowWidth) / 2;
		windowRect.top = (screenHeight - config.windowHeight) / 2;
		windowRect.right = windowRect.left + config.windowWidth;
		windowRect.bottom = windowRect.top + config.windowHeight;

		style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		styleEx = 0;

		AdjustWindowRect(&windowRect, style, FALSE);
		int extraWidth = (windowRect.right - windowRect.left) - config.windowWidth;
		int extraHeight = (windowRect.bottom - windowRect.top) - config.windowHeight;
		windowRect.left = (screenWidth - config.windowWidth) / 2 - (extraWidth / 2);
		windowRect.top = (screenHeight - config.windowHeight) / 2 - (extraHeight / 2);
		windowRect.right = windowRect.left + config.windowWidth + extraWidth;
		windowRect.bottom = windowRect.top + config.windowHeight + extraHeight;
	}
}

LRESULT MainWindow::HandleMessage(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{

	switch (msg) {
	case WM_SIZE:
		mClientWidth = LOWORD(lparam);
		mClientHeight = HIWORD(lparam);
		break;
	}
		
	for (MsgHandler &handler : mMsgHandlers) {
		LRESULT result;
		if (handler(hWnd, msg, wparam, lparam, result)) {
			return result;
		}
	}

	return DefWindowProc(hWnd, msg, wparam, lparam);
}

LRESULT MainWindow::HandleMessageDispatcher(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	MainWindow* mainWindow;

	// Special handling for window creation
	if (msg == WM_CREATE) {
		auto data = (CREATESTRUCT*) lparam;
		mainWindow = (MainWindow*) data->lpCreateParams;		
		SetWindowLongPtr(hWnd, 0, (LONG) mainWindow);
	} else {
		mainWindow = (MainWindow*) GetWindowLongPtr(hWnd, 0);
	}

	if (mainWindow) {
		return mainWindow->HandleMessage(hWnd, msg, wparam, lparam);
	} else {		
		return DefWindowProc(hWnd, msg, wparam, lparam);
	}
}
