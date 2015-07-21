
#pragma once

#include <include/cef_resource_handler.h>

class UiResourceHandler : public CefResourceHandler {
public:
	UiResourceHandler();
	~UiResourceHandler();

	bool ProcessRequest(CefRefPtr<CefRequest> request,
		CefRefPtr<CefCallback> callback);

	void GetResponseHeaders(CefRefPtr<CefResponse> response,
		int64& response_length,
		CefString& redirectUrl);

	bool ReadResponse(void* data_out,
		int bytes_to_read,
		int& bytes_read,
		CefRefPtr<CefCallback> callback);

	void Cancel();

private:
	IMPLEMENT_REFCOUNTING(UiResourceHandler);
};
