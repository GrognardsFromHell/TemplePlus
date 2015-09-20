
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Unknwnbase.h>

namespace d3d8
{
#include "d3d8/d3d8.h"
}

#undef D3DMATRIX_DEFINED
#undef D3DFVF_POSITION_MASK
#undef D3DFVF_RESERVED2
#undef D3DSP_REGNUM_MASK
#undef DIRECT3D_VERSION
#undef D3D_SDK_VERSION
#include <d3d9.h>

void LogD3dError(const char *methodName, HRESULT result);

inline HRESULT handleD3dError(const char* method, HRESULT result) {
	if (result != S_OK) {
		LogD3dError(method, result);
	}
	return result;
}

#define D3DLOG(CMD) handleD3dError(#CMD, CMD)
