#include "stdafx.h"
#include "location.h"

LocationSys locSys;

float LocationSys::distBtwnLocAndOffs(LocAndOffsets loca, LocAndOffsets locb)
{
	float dx = (loca.off_x - locb.off_x);
	float dy = (loca.off_y - locb.off_y);
	dx += (loca.location.locx - locb.location.locx) * tileToOffset;
	dy += (loca.location.locy - locb.location.locy) * tileToOffset;
	return sqrt(pow(dx,2) + pow(dy,2));
}

LocationSys::LocationSys()
{
	rebase(getLocAndOff, 0x10040080);
	macRebase(TOEEdistBtwnLocAndOffs, 1002A0A0)
}