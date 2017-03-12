
#pragma once

#include <cstdint>

struct TPGUID {
	uint32_t Data1;
	int16_t Data2;
	int16_t Data3;
	int8_t Data4[8];
};

bool operator<(const TPGUID& lhs, const TPGUID& rhs);
bool operator==(const TPGUID& lhs, const TPGUID& rhs);
