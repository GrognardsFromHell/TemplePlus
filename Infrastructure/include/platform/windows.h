
#pragma once

/*
	This header file is supposed to help with correctly including the windows header.
*/
#ifndef STRICT
#define STRICT
#endif
#define WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#define NOMINMAX 1
#include <windows.h>

#include <string>

/*
	Helper function for logging windows platform errors.
*/
std::string GetLastWin32Error();

/*
	Helper class that will call CoInitializeEx in the constructor
	and CoUninitialize in the destructor. If initialization fails,
	an exception will be thrown.

	If COM was already initialized on the thread, the destructor
	will not uninitialize it.
*/
class ComInitializer {
public:
	ComInitializer();
	~ComInitializer();
private:
	bool mAlreadyInitialized;
};
