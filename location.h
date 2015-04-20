#pragma once
#include "stdafx.h"
#include "common.h"

const float tileToOffset = 12.0;

struct LocationSys : AddressTable
{
	float distBtwnLocAndOffs(LocAndOffsets, LocAndOffsets);
	void(__cdecl * getLocAndOff)(objHndl objHnd, LocAndOffsets * locAndOff);
	void(__cdecl * TOEEdistBtwnLocAndOffs)(LocAndOffsets, LocAndOffsets); // outputs to the FPU (st0);is basically  sqrt(dx^2+dy^2)/ 12 where a tile is sqrt(800)xsqrt(800);  I think it's in Inches because some functions divide this result by 12 (inches->feet)
	float intToFloat(int32_t x);
	LocationSys();
};

/*
	Calculates the angle in radians between two points in the tile coordinate system.
	The angle can be used to make an object that is at fromPoint face the location at toPoint,
	given that rotation 0 means "look directory north".
*/
float AngleBetweenPoints(LocAndOffsets fromPoint, LocAndOffsets toPoint);

extern LocationSys locSys;

