
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

#include "MinHook.h"

#include <pybind11/pybind11.h>
