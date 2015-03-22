#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <windowsx.h>

#include "MinHook.h"
#include "Shlobj.h"
#include "Shobjidl.h"
#pragma comment(lib, "shell32.lib")

#include "Shlwapi.h"
#pragma comment(lib, "shlwapi.lib")

#include <string>
#include <functional>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>

using namespace std;
using namespace boost::filesystem;
using namespace boost;

#define LOG(T) BOOST_LOG_TRIVIAL(T)

#include <DxErr.h>
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "D3dx9.lib")

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
#include <D3dx9tex.h>
