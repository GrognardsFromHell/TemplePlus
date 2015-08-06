
#pragma once

/**
The browser will spawn multiple processes of the game to properly distribute load
and improve JS performance, since there can only be a single V8 context per process
active at a time. On top of that, multi process mode is the *default* mode for CEF
and single process is poorly supported.

*/
class UiRenderProcess {
public:

	
	bool IsRenderProcess(LPSTR commandLine);

	/*
		If this process is to be "consumed" by CEF, this method will return true and the
		appropriate return code in the retCode parameter.
	*/
	bool Run(HINSTANCE instance, int &retCode);

};
