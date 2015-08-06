
#include "stdafx.h"
#include "ui_browser_app.h"
#include "ui_resourcebundlehandler.h"

UiBrowserApp::UiBrowserApp() {
	mResourceBundleHandler = new UiResourceBundleHandler;
}

UiBrowserApp::~UiBrowserApp() {
}

void UiBrowserApp::OnBeforeCommandLineProcessing(const CefString& process_type, 
	CefRefPtr<CefCommandLine> command_line) {

	//command_line->AppendSwitch("disable-gpu");
	//command_line->AppendSwitch("disable-gpu-compositing");
	command_line->AppendSwitch("single-process");
	command_line->AppendSwitch("in-process-gpu");

}

void UiBrowserApp::OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) {
}

CefRefPtr<CefResourceBundleHandler> UiBrowserApp::GetResourceBundleHandler() {
	return mResourceBundleHandler;
}

class LambdaV8Handler : public CefV8Handler {
public:
	typedef std::function<CefRefPtr<CefV8Value>(const CefV8ValueList &)> Func;

	LambdaV8Handler(const Func &func) : mFunc(func) {

	}

	// Inherited via CefV8Handler
	virtual bool Execute(const CefString & name, CefRefPtr<CefV8Value> object, const CefV8ValueList & arguments, CefRefPtr<CefV8Value>& retval, CefString & exception) override
	{
		auto result = mFunc(arguments);
		retval = result;
		return true;
	}

private:
	Func mFunc;

	IMPLEMENT_REFCOUNTING(LambdaV8Handler);
};

void UiBrowserApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
	context->Enter();

	auto global = context->GetGlobal();
	
	CefRefPtr<CefV8Handler> handler = new LambdaV8Handler([=](const CefV8ValueList &args) {
		auto msg = CefProcessMessage::Create("JSIPC");
		auto msgArgs = msg->GetArgumentList();
		browser->SendProcessMessage(PID_BROWSER, msg);
		return CefV8Value::CreateUndefined();
	});
	auto fun = CefV8Value::CreateFunction("send_msg", handler);
	global->SetValue(fun->GetFunctionName(), fun, V8_PROPERTY_ATTRIBUTE_READONLY);

	context->Exit();
}

