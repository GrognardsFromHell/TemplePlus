
#pragma once

#include "system.h"

extern void* templeImageBase;

template<uint32_t address>
inline void* temple_address() {
	static_assert(address > 0x10000000 && address < 0x20000000,
		"address is not within temple.dll address space");
	BOOST_ASSERT(templeImageBase != 0);
	return ((char*)templeImageBase) + (address - 0x10000000);
}

inline void* temple_address(uint32_t address) {
	BOOST_ASSERT(address > 0x10000000 && address < 0x20000000);
	BOOST_ASSERT(templeImageBase != 0);
	return ((char*)templeImageBase) + (address - 0x10000000);
}

template<uint32_t address, typename T>
inline T temple_get() {
	return *(T*)temple_address<address>();
}

template<typename T>
inline T temple_get(uint32_t address) {
	return *(T*)temple_address(address);
}

template<typename T>
inline void temple_set(uint32_t address, T value) {
	*(T*)temple_address(address) = value;
}

template<uint32_t address, typename T>
inline void temple_set(T value) {
	*(T*)temple_address(address) = value;
}
