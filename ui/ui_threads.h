
#pragma once

#include <include/cef_task.h>

template<typename T>
class CefLambdaWithReturn : public CefTask {
public:
	CefLambdaWithReturn(function<T()> callback) : mCallback(callback) {
	}

	void Execute() override {
		// Execute the callback
		mPromise.set_value(mCallback());
	}

	future<T> future() {
		return mPromise.get_future();
	}

private:
	function<T()> mCallback;
	promise<T> mPromise;
	IMPLEMENT_REFCOUNTING(CefLambdaWithReturn);
};

class UiThreads {
public:

	template<typename T>
	static future<T> PostUiTask(function<T()> callback) {
		return PostTask(TID_UI, callback);
	}

	template<typename T>
	static future<T> PostTask(CefThreadId thread, function<T()> callback) {
		CefRefPtr<CefLambdaWithReturn<T>> refptr = new CefLambdaWithReturn<T>(callback);
		CefPostTask(thread, refptr);
		CefDoMessageLoopWork();
		return refptr->future();
	}

};
