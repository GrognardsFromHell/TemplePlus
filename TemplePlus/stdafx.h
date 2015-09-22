#define _CRT_SECURE_NO_WARNINGS

// We'd rather use std::min
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <timeapi.h>

#include "d3d.h"

#include "Shlobj.h"
#include "Shobjidl.h"
#pragma comment(lib, "shell32.lib")

#include "Shlwapi.h"
#pragma comment(lib, "shlwapi.lib")

#include <string>
#include <functional>
#include <vector>
#include <chrono>
#include <cassert>
#include <memory>
#include <algorithm>
#include <unordered_map>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

#define _USE_MATH_DEFINES
#include <math.h>

#pragma comment(lib, "D3dx9.lib")

#undef D3DMATRIX_DEFINED
#undef D3DFVF_POSITION_MASK
#undef D3DFVF_RESERVED2
#undef D3DSP_REGNUM_MASK
#undef DIRECT3D_VERSION
#undef D3D_SDK_VERSION
#include <d3d9.h>
#include <D3dx9tex.h>

#include "infrastructure.h"
using fmt::format;

#include "MinHook.h"

#define Py_NO_ENABLE_SHARED
#include "Python.h"
#undef _GNU_SOURCE // Defined by python for some reason
#undef LONG_LONG
