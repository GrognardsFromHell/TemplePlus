#include "stdafx.h"
#include "common.h"


struct FloatLineSystem : AddressTable
{
	void(__cdecl * floatMesLine)(objHndl, int32_t, int32_t, const char* );

	FloatLineSystem();
};

extern  FloatLineSystem floatSys;