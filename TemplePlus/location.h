#pragma once

#include "common.h"
#include <temple/dll.h>

const float tileToOffset = 12.0;

struct LocationSys : temple::AddressTable
{
	float distBtwnLocAndOffs(LocAndOffsets, LocAndOffsets);
	void RegularizeLoc(LocAndOffsets* toLocTweaked); //  alters the location and offsets so that the offsets are within the tile
	void GetOverallOffset(LocAndOffsets loc, float* absX, float* absY);
	BOOL ShiftLocationByOneSubtile(LocAndOffsets* loc, char direction, LocAndOffsets* locOut);
	void(__cdecl * getLocAndOff)(objHndl objHnd, LocAndOffsets * locAndOff);
	void(__cdecl* SubtileToLocAndOff)(int64_t subtile, LocAndOffsets* locFromSubtile);
	int64_t(__cdecl * subtileFromLoc)(LocAndOffsets * loc);
	BOOL(__cdecl* ShiftSubtileOnceByDirection)(Subtile subtile, int direction, Subtile * shiftedTile);
	void(__cdecl * TOEEdistBtwnLocAndOffs)(LocAndOffsets, LocAndOffsets); // outputs to the FPU (st0);is basically  sqrt(dx^2+dy^2)/ 12 where a tile is sqrt(800)xsqrt(800);  I think it's in Inches because some functions divide this result by 12 (inches->feet)
	float (__cdecl * Distance3d)(LocAndOffsets loc1, LocAndOffsets loc2); // is basically  sqrt(dx^2+dy^2)/ 12 where a tile is sqrt(800)xsqrt(800);  I think it's in Inches because some functions divide this result by 12 (inches->feet)
	float intToFloat(int32_t x);

	// Distance between two objects in feet
	float (__cdecl *DistanceToObj)(objHndl from, objHndl to);

	// Distance between from and loc in inches (without the obj radius)
	float DistanceToLoc(objHndl from, LocAndOffsets loc);

	float InchesToFeet(float inches);

	LocationSys();
};

/*
	Calculates the angle in radians between two points in the tile coordinate system.
	The angle can be used to make an object that is at fromPoint face the location at toPoint,
	given that rotation 0 means "look directory north".
*/
float AngleBetweenPoints(LocAndOffsets fromPoint, LocAndOffsets toPoint);

extern LocationSys locSys;

