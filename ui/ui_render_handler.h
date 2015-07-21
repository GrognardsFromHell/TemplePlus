
#pragma once

#include <include/cef_render_handler.h>

#include <atlbase.h>
#include <d3d9.h>

class UiRenderHandler : public CefRenderHandler {
public:
	UiRenderHandler();
	~UiRenderHandler();

	bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect);

	bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
		int viewX,
		int viewY,
		int& screenX,
		int& screenY);

	/*
		Called by CEF to figure out how much space it has to lay out the web pages.
	*/
	bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
	
	/*
		Called by the CEF renderer to update our UI texture.
	*/
	void OnPaint(CefRefPtr<CefBrowser> browser,
		PaintElementType type,
		const RectList& dirtyRects,
		const void* buffer,
		int width, int height) override;

	/*
		Draws the current UI texture full screen.
	*/
	void Render();

private:
	IMPLEMENT_REFCOUNTING(UiRenderHandler);

	void EnsureTextureSize(int w, int h);

	CComPtr<IDirect3DTexture9> mTexture;
	int mTextureMaxWidth = 0;
	int mTextureMaxHeight = 0;
	int mTextureHeight = 0;
	int mTextureWidth = 0;
};
