
#include "stdafx.h"

#include "ui_render_handler.h"
#include "ui_render.h"

#include "../graphics.h"
#include "../mainwindow.h"

UiRenderHandler::UiRenderHandler(MainWindow &mainWindow) : mMainWindow(mainWindow) {
}

UiRenderHandler::~UiRenderHandler() {
}

bool UiRenderHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
	RECT window_rect = { 0 };
	HWND root_window = GetAncestor(mMainWindow.GetHwnd(), GA_ROOT);
	if (::GetWindowRect(root_window, &window_rect)) {
		rect = CefRect(window_rect.left,
			window_rect.top,
			window_rect.right - window_rect.left,
			window_rect.bottom - window_rect.top);
		return true;
	}
	return false;
}

bool UiRenderHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser,
	int viewX,
	int viewY,
	int& screenX,
	int& screenY) {

	if (!::IsWindow(mMainWindow.GetHwnd()))
		return false;

	// Convert the point from view coordinates to actual screen coordinates.
	POINT screen_pt = { viewX, viewY };
	ClientToScreen(mMainWindow.GetHwnd(), &screen_pt);
	screenX = screen_pt.x;
	screenY = screen_pt.y;
	return true;
}

bool UiRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
	rect.x = 0;
	rect.y = 0;
	rect.width = mMainWindow.GetClientWidth();
	rect.height = mMainWindow.GetClientHeight();
	return true;
}

void UiRenderHandler::OnPaint(CefRefPtr<CefBrowser> browser,
	PaintElementType type,
	const RectList& dirtyRects,
	const void* buffer,
	int width, int height) {

	if (!graphics.device()) {
		return; // D3d not initialized yet
	}

	EnsureTextureSize(width, height);

	D3DLOCKED_RECT locked;
	RECT rect;
	for (const CefRect &dirtyRect : dirtyRects) {
		// Fill in the rectangle we need to lock
		rect.left = dirtyRect.x;
		rect.right = rect.left + dirtyRect.width;
		rect.top = dirtyRect.y;		
		rect.bottom = rect.top + dirtyRect.height;

		if (D3DLOG(mTexture->LockRect(0, &locked, &rect, 0)) != D3D_OK) {
			logger->error("Unable to lock UI texture for update.");
			continue; // Cannot update this dirty rect
		}

		// Assuming 32-bit pixels here
		int* srcData = ((int*)buffer) + dirtyRect.y * width + dirtyRect.x;
		char *destData = (char*) locked.pBits;
		for (int y = 0; y < dirtyRect.height; ++y) {
			memcpy(destData, srcData, dirtyRect.width * 4); // Copy one line
			srcData += width; // Skip one entire line
			destData += locked.Pitch;
		}

		if (D3DLOG(mTexture->UnlockRect(0))) {
			logger->error("Unable to unlock UI texture after update.");
		}
	}

}

void UiRenderHandler::Render() {

	if (!mTexture) {
		return; // Nothing to render yet
	}

	// Src and Dest Rect are identical
	TigRect rect;
	rect.x = 0;
	rect.y = 0;
	rect.width = mTextureWidth;
	rect.height = mTextureHeight;

	UiRenderer::DrawTexture(mTexture, mTextureMaxWidth, mTextureMaxHeight,
		rect, rect);

}

void UiRenderHandler::EnsureTextureSize(int w, int h) {
	// Check desired dimensions
	mTextureWidth = w;
	mTextureHeight = h;

	// Destroy previous texture if it is too small
	if (mTextureWidth > mTextureMaxWidth || mTextureHeight > mTextureMaxHeight) {
		mTexture.Release();
	}

	if (!mTexture) {
		D3DPOOL texPool = config.useDirect3d9Ex ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
		DWORD usage = config.useDirect3d9Ex ? D3DUSAGE_DYNAMIC : 0;

		logger->info("Resizing UI texture to {}, {}", w, h);
		if (D3DLOG(graphics.device()->CreateTexture(mTextureWidth,
			mTextureHeight,
			1, 
			usage,
			D3DFMT_A8R8G8B8, 
			texPool, 
			&mTexture, 
			nullptr)) != D3D_OK) {
			throw new TempleException("Cannot create the texture for drawing the UI");
		}

		mTextureMaxWidth = mTextureWidth;
		mTextureMaxHeight = mTextureHeight;
	}

}
