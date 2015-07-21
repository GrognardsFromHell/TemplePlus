
#include "stdafx.h"

#include "ui_browser_client.h"
#include "ui_render_handler.h"
#include "ui_request_handler.h"

#include "util/exception.h"

UiBrowserClient::UiBrowserClient() {
	mRenderHandler = new UiRenderHandler;
	mRequestHandler = new UiRequestHandler;
}

UiBrowserClient::~UiBrowserClient() {
}

CefRefPtr<CefRenderHandler> UiBrowserClient::GetRenderHandler() {
	return mRenderHandler;
}

CefRefPtr<CefLifeSpanHandler> UiBrowserClient::GetLifeSpanHandler() {
	return this;
}

CefRefPtr<CefLoadHandler> UiBrowserClient::GetLoadHandler() {
	return this;
}

CefRefPtr<CefRequestHandler> UiBrowserClient::GetRequestHandler() {
	return mRequestHandler;
}

void UiBrowserClient::OnLoadError(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefLoadHandler::ErrorCode errorCode,
	const CefString& errorText,
	const CefString& failedUrl) {

	logger->error("Failed to load web page {}: {} {}", failedUrl.ToString(), (int) errorCode, errorText.ToString());

}

void UiBrowserClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
	if (mBrowser) {
		throw new TempleException("Multiple browsers were created!");
	}
	mBrowser = browser;
}

void UiBrowserClient::Render() {
	mRenderHandler->Render();
}
