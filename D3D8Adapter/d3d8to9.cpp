
#include "stdafx.h"
#include "d3d.h"
#include "d3d8to9_rootobj.h"
#include "dxerr.h"

bool useDirect3d9Ex = false;

void SetDirect3d9Ex(bool enable)
{
	useDirect3d9Ex = enable;
}

void LogD3dError(const char *method, HRESULT result)
{
	logger->warn("Direct3D Error @ {}: {}", method, DXGetErrorString(result));
}
