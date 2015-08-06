
#pragma once

#include <include/cef_browser.h>

/*
	Main class for defining the native code layer that can be
	accessed from JavaScript.
*/
class JsInterop {
public:

	JsInterop(CefRefPtr<CefBrowser> &browser) : mBrowser(browser) {}

	// Send a message to the JavaScript running in the browser.
	void SendMessage(const string &type, const CefValue &value);

private:
	CefRefPtr<CefBrowser> mBrowser;

};
