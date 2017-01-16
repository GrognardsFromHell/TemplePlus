#include "stdafx.h"
#include "rng.h"
#include <temple/dll.h>


LegacyRNG rngSys;

int LegacyRNG::GetInt(int min, int max){
	return temple::GetRef<int(__cdecl)(int, int)>(0x10038DF0)(min,max);
}
