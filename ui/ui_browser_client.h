
#pragma once

#include <include/cef_client.h>

class UiRenderHandler;

class UiBrowserClient : public CefClient,
						public CefDisplayHandler,
						public CefLifeSpanHandler,
						public CefLoadHandler {
public:
	UiBrowserClient();
	~UiBrowserClient();

	void Render();
	
	CefRefPtr<CefRenderHandler> GetRenderHandler() override;

	CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;

	CefRefPtr<CefLoadHandler> GetLoadHandler() override;

	/* CefLoadHandler Callbacks */

	void OnLoadError(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefLoadHandler::ErrorCode errorCode,
		const CefString& errorText,
		const CefString& failedUrl) override;

private:
	IMPLEMENT_REFCOUNTING(UiBrowserClient);

	CefRefPtr<UiRenderHandler> mRenderHandler;
};
