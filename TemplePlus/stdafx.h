#define _CRT_SECURE_NO_WARNINGS

#include <platform/d3d.h>

#include <timeapi.h>

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

#include <infrastructure/infrastructure.h>
using fmt::format;

#include "MinHook.h"

#define Py_NO_ENABLE_SHARED
#include "Python.h"
#undef _GNU_SOURCE // Defined by python for some reason
#undef LONG_LONG
