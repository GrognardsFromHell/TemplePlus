#include "stdafx.h"
#include "location.h"

LocationSys locSys;

float LocationSys::distBtwnLocAndOffs(LocAndOffsets loca, LocAndOffsets locb)
{
	float dx = 0;
	float dy = 0;
	int32_t dyi = loca.location.locy - locb.location.locy;
	int32_t dxi = loca.location.locx - locb.location.locx;
	dx += (loca.off_x - locb.off_x) + intToFloat(dxi)*tileToOffset;
	dy += (loca.off_y - locb.off_y) + intToFloat(dyi)*tileToOffset;
	 return sqrt(pow(dx, 2) + pow(dy, 2)) ;
}

float LocationSys::intToFloat(int32_t x)
{
	float result;
	__asm{
		fild x;
		fstp result;
	}
	return result;
}

LocationSys::LocationSys()
{
	rebase(getLocAndOff, 0x10040080);
	macRebase(TOEEdistBtwnLocAndOffs, 1002A0A0)
}