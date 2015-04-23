#include "stdafx.h"
#include "common.h"


struct FloatLineSystem : AddressTable
{
	void(__cdecl * floatMesLine)(objHndl, int categoryBit, int colorId, const char *text);

	FloatLineSystem();
};

extern  FloatLineSystem floatSys;