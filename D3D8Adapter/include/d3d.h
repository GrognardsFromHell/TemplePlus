
#pragma once

#include <platform/windows.h>
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

#include <platform/d3d.h>
