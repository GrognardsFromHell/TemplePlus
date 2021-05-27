#pragma once

#include <memory>
#include <timeapi.h>


uint32_t TigGetSystemTime() {
	return timeGetTime();
}

/* 0x101E3510 */
int TigElapsedSystemTime(int refTime) {
	return timeGetTime() - refTime;
}