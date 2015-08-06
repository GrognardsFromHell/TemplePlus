
#include "stdafx.h"
#include "ui_render_process.h"
#include "ui_render_app.h"

bool UiRenderProcess::IsRenderProcess(LPSTR commandLine)
{
	return (strstr(commandLine, "--type=") != nullptr);
}

bool UiRenderProcess::Run(HINSTANCE instance, int & retCode)
{
	CefMainArgs mainArgs(instance);
	CefRefPtr<UiRenderApp> renderApp = new UiRenderApp;

	if ((retCode = CefExecuteProcess(mainArgs, renderApp, nullptr)) >= 0) {
		return true;
	}
	return false;
}
