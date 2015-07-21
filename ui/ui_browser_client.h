
#pragma once

#include <include/cef_client.h>

class UiRenderHandler;
class UiRequestHandler;

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

	CefRefPtr<CefRequestHandler> GetRequestHandler() override;

	/* CefLoadHandler callbacks */

	void OnLoadError(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefLoadHandler::ErrorCode errorCode,
		const CefString& errorText,
		const CefString& failedUrl) override;


	/* CefLifeSpanHandler callbacks */
	void OnAfterCreated(CefRefPtr<CefBrowser> browser);

	CefRefPtr<CefBrowser> browser() {
		return mBrowser;
	}

private:
	IMPLEMENT_REFCOUNTING(UiBrowserClient);

	CefRefPtr<CefBrowser> mBrowser;

	CefRefPtr<UiRenderHandler> mRenderHandler;

	CefRefPtr<UiRequestHandler> mRequestHandler;
};
