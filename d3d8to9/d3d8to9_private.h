#pragma once

#include "../system.h"
#include "../addresses.h"

#include <DxErr.h>
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "D3dx9.lib")

namespace d3d8 {
#include "../d3d8/d3d8.h"
}

#undef D3DFVF_POSITION_MASK
#undef D3DFVF_RESERVED2
#undef D3DSP_REGNUM_MASK
#undef DIRECT3D_VERSION
#undef D3D_SDK_VERSION
#include <d3d9.h>
#include <D3dx9tex.h>

extern bool useD3dEx;

inline HRESULT handleError(const char *method, HRESULT result) {
	if (result != S_OK) {
		LOG(info) << "DirectX Error @ " << method << ": " << DXGetErrorStringA(result);
	}
	return result;
}

// Forward declaration for all our adapters
struct Direct3DVertexBuffer8Adapter;
struct Direct3DIndexBuffer8Adapter;
struct Direct3DTexture8Adapter;
struct Direct3DDevice8Adapter;
struct Direct3DSurface8Adapter;
struct Direct3D8Adapter;
