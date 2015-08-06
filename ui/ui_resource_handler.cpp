
#include "stdafx.h"

#include <include/cef_parser.h>

#include "ui_resource_handler.h"

UiResourceHandler::UiResourceHandler() {
}

UiResourceHandler::~UiResourceHandler() {
}

bool UiResourceHandler::ProcessRequest(CefRefPtr<CefRequest> request,
	CefRefPtr<CefCallback> callback) {
	return true;
}

void UiResourceHandler::GetResponseHeaders(CefRefPtr<CefResponse> response,
	int64& response_length,
	CefString& redirectUrl) {

}

bool UiResourceHandler::ReadResponse(void* data_out,
	int bytes_to_read,
	int& bytes_read,
	CefRefPtr<CefCallback> callback) {
	return false;
}

void UiResourceHandler::Cancel() {

}
