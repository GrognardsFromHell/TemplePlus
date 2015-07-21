
#pragma once

#include <include/cef_request_handler.h>

class UiRequestHandler : public CefRequestHandler {
public:
	UiRequestHandler();
	~UiRequestHandler();

	CefRefPtr<CefResourceHandler> GetResourceHandler(CefRefPtr<CefBrowser> browser, 
		CefRefPtr<CefFrame> frame, 
		CefRefPtr<CefRequest> request);

private:
	IMPLEMENT_REFCOUNTING(UiRequestHandler);
};
