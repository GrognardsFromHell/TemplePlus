
#pragma once

/*
	This header file is supposed to help with correctly including the windows header.
*/
#ifndef STRICT
#define STRICT
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <string>

/*
	Helper function for logging windows platform errors.
*/
std::string GetLastWin32Error();
