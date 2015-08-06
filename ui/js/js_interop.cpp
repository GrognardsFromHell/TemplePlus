
#include "stdafx.h"
#include "js_interop.h"

void JsInterop::SendMessage(const string &type, const CefValue &value)
{

	mBrowser->GetFocusedFrame()->ExecuteJavaScript("ExecuteUI('', '')", "", 0);
	
}
