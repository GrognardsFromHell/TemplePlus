#pragma once

#include "../stdafx.h"
#include "../addresses.h"
#include "../config.h"

inline HRESULT handleD3dError(const char* method, HRESULT result) {
	if (result != S_OK) {
		logger->warn("Direct3D Error @ {}: {}", method, DXGetErrorStringA(result));
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
