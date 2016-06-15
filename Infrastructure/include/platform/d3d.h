
#pragma once

#include "windows.h"

#include <d3d11.h>
#include <d3d11_1.h>
#include <atlcomcli.h>

void LogD3dError(const char *methodName, HRESULT result);
void ThrowD3dAssertion(const char *methodName, HRESULT result);

inline HRESULT _HandleD3dError(const char* method, HRESULT result) {
	if (result != S_OK) {
		LogD3dError(method, result);
	}
	return result;
}

#define D3DLOG(CMD) _HandleD3dError(#CMD, CMD)

inline void _assertD3dSuccess(const char* method, HRESULT result) {
	if (result != S_OK) {
		ThrowD3dAssertion(method, result);
	}
}

#define D3DVERIFY(CMD) _assertD3dSuccess(#CMD, CMD)

#include <DirectXMath.h>
