
#pragma once

#include "windows.h"

#include <d3d9.h>

void LogD3dError(const char *methodName, HRESULT result);

inline HRESULT _HandleD3dError(const char* method, HRESULT result) {
	if (result != S_OK) {
		LogD3dError(method, result);
	}
	return result;
}

#define D3DLOG(CMD) _HandleD3dError(#CMD, CMD)
