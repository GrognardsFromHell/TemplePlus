#include "stdafx.h"

#include "templeplus.h"

#include "ui/renderer/ui_render_process.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd) {

	// CEF starts the secondary processes with a --type parameter
	UiRenderProcess renderProcess;
	if (renderProcess.IsRenderProcess(lpCmdLine)) {		
		int retCode;
		renderProcess.Run(hInstance, retCode);
		return retCode;
	}

	TemplePlus templePlus;

	return templePlus.Run(hInstance, lpCmdLine);	
}
