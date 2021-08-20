#pragma once

#include <memory>
#include <timeapi.h>

/* 0x101E3500 */
inline uint32_t TigGetSystemTime() {
	return timeGetTime();
}

/* 0x101E3510 */
inline int TigElapsedSystemTime(int refTime) {
	return timeGetTime() - refTime;
}