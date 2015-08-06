
#include "stdafx.h"

#include <include/cef_parser.h>

#include "ui_request_handler.h"
#include "ui_resource_handler.h"

UiRequestHandler::UiRequestHandler() {
}

UiRequestHandler::~UiRequestHandler() {
}

CefRefPtr<CefResourceHandler> UiRequestHandler::GetResourceHandler(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefRequest> request) {

	request->GetURL();

	CefURLParts parts;
	if (!CefParseURL(request->GetURL(), parts)) {
		return nullptr; // We don't know how to parse this URL
	}

	// We only consider http
	if (wcscmp(parts.scheme.str, L"http")) {
		return nullptr;
	}

	// and localhost
	if (wcscmp(parts.host.str, L"localhost")) {
		return nullptr;
	}

	// and port 80
	if (parts.port.length != 0 && wcscmp(parts.port.str, L"80")) {
		return nullptr;
	}

	return new UiResourceHandler;
}
