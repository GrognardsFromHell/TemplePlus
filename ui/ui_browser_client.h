
#pragma once

#include <include/cef_client.h>

class UiRenderHandler;
class UiRequestHandler;
class MainWindow;

class UiBrowserClient : public CefClient,
						public CefDisplayHandler,
						public CefLifeSpanHandler,
						public CefLoadHandler {
public:
	UiBrowserClient(MainWindow &mainWindow);
	~UiBrowserClient();
	void Render();
	
	CefRefPtr<CefRenderHandler> GetRenderHandler() override;

	CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;

	CefRefPtr<CefLoadHandler> GetLoadHandler() override;

	CefRefPtr<CefRequestHandler> GetRequestHandler() override;

	CefRefPtr<CefDisplayHandler> GetDisplayHandler() override;

	/* CefDisplayHandler callbacks */
	bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
		const CefString& message,
		const CefString& source,
		int line);

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

	bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
		CefProcessId source_process,
		CefRefPtr<CefProcessMessage> message) {
		return false;
	}

private:
	IMPLEMENT_REFCOUNTING(UiBrowserClient);

	CefRefPtr<CefBrowser> mBrowser;

	CefRefPtr<UiRenderHandler> mRenderHandler;

	CefRefPtr<UiRequestHandler> mRequestHandler;
};
