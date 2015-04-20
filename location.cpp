#include "stdafx.h"
#include "location.h"

LocationSys locSys;

const float PIXEL_PER_TILE = sqrt(800.0f);

float LocationSys::distBtwnLocAndOffs(LocAndOffsets loca, LocAndOffsets locb)
{
	float dx = 0;
	float dy = 0;
	const float eighthundred = 800;
	float sqrt800 = sqrt(eighthundred);
	int32_t dyi = loca.location.locy - locb.location.locy;
	int32_t dxi = loca.location.locx - locb.location.locx;
	dx += (loca.off_x - locb.off_x) + intToFloat(dxi)*sqrt800;
	dy += (loca.off_y - locb.off_y) + intToFloat(dyi)*sqrt800;
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

float AngleBetweenPoints(LocAndOffsets fromPoint, LocAndOffsets toPoint) {

	auto fromCoord = fromPoint.ToInches2D();
	auto toCoord = fromPoint.ToInches2D();
	
	// Create the vector from->to
	auto dir = toCoord - fromCoord;

	auto angle = atan2(dir.y, dir.x);
	return angle + 2.3561945f; // + 135 degrees
}
