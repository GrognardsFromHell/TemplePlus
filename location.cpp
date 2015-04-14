#include "stdafx.h"
#include "location.h"

LocationSys locSys;

LocationSys::LocationSys()
{
	rebase(getLocAndOff, 0x10040080);
}