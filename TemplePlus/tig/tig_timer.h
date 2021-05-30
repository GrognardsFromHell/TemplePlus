#pragma once

#include <memory>
#include <timeapi.h>

/* 0x101E3500 */
uint32_t TigGetSystemTime() {
	return timeGetTime();
}

/* 0x101E3510 */
int TigElapsedSystemTime(int refTime) {
	return timeGetTime() - refTime;
}