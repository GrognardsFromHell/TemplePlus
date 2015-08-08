
#pragma once

#include <include/cef_v8.h>
#include "../ui_threads.h"
#include "../ui_browser_app.h"

/*
	Main class for defining the native code layer that can be
	accessed from JavaScript.
*/
class JsInterop {
public:

	template<typename... ArgType>
	static void CallAsync(const char *funcName, ArgType ...args) {
		UiThreads::PostRenderTask<void>([=]() {
			auto v8Context = UiBrowserApp::GetMainJsContext();
			v8Context->Enter();

			CefV8ValueList valueList;
			AddToValueList(valueList, args...);

			v8Context->GetGlobal()->GetValue(funcName)
				->ExecuteFunction(nullptr, valueList);
			v8Context->Exit();
		});
	}

	template<typename RetType, typename... ArgType>
	static RetType Call(const char *funcName, ArgType ...args) {
		return UiThreads::PostRenderTask<RetType>([=]() {
			auto v8Context = UiBrowserApp::GetMainJsContext();
			v8Context->Enter();

			CefV8ValueList valueList;
			AddToValueList(valueList, args...);

			auto result = v8Context->GetGlobal()->GetValue(funcName)
				->ExecuteFunction(nullptr, valueList);

			auto realResult = FromValue<RetType>(result);

			v8Context->Exit();

			return realResult;
		}).get();
	}

private:
	static CefRefPtr<CefV8Value> ToValue(int val) {
		return CefV8Value::CreateInt(val);
	}

	template<typename RetVal>
	static RetVal FromValue(const CefRefPtr<CefV8Value> &val) {
	}

	template<>
	static bool FromValue<bool>(const CefRefPtr<CefV8Value> &val) {
		return val->GetBoolValue();
	}

	static void AddToValueList(CefV8ValueList &args) {
	}

	template<typename ArgType, typename... MoreArgTypes>
	static void AddToValueList(CefV8ValueList &args, ArgType arg, MoreArgTypes... moreArgs) {
		args.push_back(ToValue(arg));

		AddToValueList(args, moreArgs...);
	}

};
