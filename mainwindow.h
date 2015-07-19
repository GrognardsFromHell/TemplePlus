
#pragma once

#include "tig/tig_startup.h"

#include <functional>

typedef std::function<bool(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)> MessageFilterFn;

bool CreateMainWindow(TigConfig* settings);
void SetMessageFilter(MessageFilterFn filter);
