
#include "stdafx.h"
#include "d3d.h"
#include "d3d8to9_rootobj.h"
#include <platform/dxerr.h>

bool useDirect3d9Ex = false;

void SetDirect3d9Ex(bool enable)
{
	useDirect3d9Ex = enable;
}
