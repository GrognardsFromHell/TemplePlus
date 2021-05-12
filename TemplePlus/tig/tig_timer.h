#pragma once

#include <memory>
#include <timeapi.h>


uint32_t GetSystemTime() {
	return timeGetTime();
}