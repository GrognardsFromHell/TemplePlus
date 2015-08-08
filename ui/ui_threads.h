
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

class CefLambdaWithoutReturn : public CefTask {
public:
	CefLambdaWithoutReturn(function<void()> callback) : mCallback(callback) {
	}

	void Execute() override {
		// Execute the callback
		mCallback();
		mPromise.set_value();
	}

	future<void> future() {
		return mPromise.get_future();
	}

private:
	function<void()> mCallback;
	promise<void> mPromise;
	IMPLEMENT_REFCOUNTING(CefLambdaWithoutReturn);
};

class UiThreads {
public:

	template<typename T>
	static future<T> PostUiTask(function<T()> callback) {
		return PostTask(TID_UI, callback);
	}

	template<typename T>
	static future<T> PostRenderTask(function<T()> callback) {
		return PostTask(TID_RENDERER, callback);
	}
	
	template<typename T>
	static future<T> PostTask(CefThreadId thread, function<T()> callback) {
		CefRefPtr<CefLambdaWithReturn<T>> refptr = new CefLambdaWithReturn<T>(callback);
		RunTask(thread, refptr);
		return refptr->future();
	}

	template<>
	static future<void> PostTask<void>(CefThreadId thread, function<void()> callback) {
		CefRefPtr<CefLambdaWithoutReturn> refptr = new CefLambdaWithoutReturn(callback);
		RunTask(thread, refptr);
		return refptr->future();
	}
private:

	static void RunTask(CefThreadId thread, CefRefPtr<CefTask> task);

};
