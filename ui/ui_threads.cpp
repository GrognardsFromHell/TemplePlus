
#include <stdafx.h>
#include "ui_threads.h"

#include <include/cef_app.h>

void UiThreads::RunTask(CefThreadId thread, CefRefPtr<CefTask> task) {
	CefPostTask(thread, task);
	CefDoMessageLoopWork();
}
