#pragma once
#include "stdafx.h"
#include "common.h"

struct LocationSys : AddressTable
{
	void(__cdecl * getLocAndOff)(objHndl objHnd, LocAndOffsets * locAndOff);
	LocationSys();
};

extern LocationSys locSys;