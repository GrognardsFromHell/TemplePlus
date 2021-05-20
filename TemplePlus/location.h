#pragma once

#include "common.h"
#include <temple/dll.h>

const float tileToOffset = 12.0;
#define LOCATION_OFFSET_TOL 0.001 // when comparing offsets, differences smaller than this will be considered as ignorable computational error

struct LocationSys : temple::AddressTable
{
	int64_t * locTransX;
	int64_t*locTransY;
	float distBtwnLocAndOffs(LocAndOffsets, LocAndOffsets); // in inches
	void RegularizeLoc(LocAndOffsets* toLocTweaked); //  alters the location and offsets so that the offsets are within the tile
	void GetOverallOffset(LocAndOffsets loc, float* absX, float* absY);
	BOOL ShiftLocationByOneSubtile(LocAndOffsets* loc, ScreenDirections direction, LocAndOffsets* locOut);

	void (__cdecl*PointNodeInit)(LocAndOffsets* loc, PointNode* pntNode);
	int GetLocFromScreenLocPrecise(int x, int y, LocAndOffsets&);
	
	void(__cdecl * getLocAndOff)(objHndl objHnd, LocAndOffsets * locAndOff);
	void(__cdecl* SubtileToLocAndOff)(int64_t subtile, LocAndOffsets* locFromSubtile);
	int64_t(__cdecl * subtileFromLoc)(LocAndOffsets * loc);
	BOOL(__cdecl* ShiftSubtileOnceByDirection)(Subtile subtile, int direction, Subtile * shiftedTile);
	void(__cdecl * TOEEdistBtwnLocAndOffs)(LocAndOffsets, LocAndOffsets); // outputs to the FPU (st0);is basically  sqrt(dx^2+dy^2) where a tile is sqrt(800)xsqrt(800);  I think it's in Inches because some functions divide this result by 12 (inches->feet)
	float (__cdecl * Distance3d)(LocAndOffsets loc1, LocAndOffsets loc2); // is basically  sqrt(dx^2+dy^2)*INCH_PER_TILE  [inch]
	float intToFloat(int32_t x);

	void ToTranslation(int x, int y, int &xOut, int &yOut);
	void TileToScreen(int x, int y, int &xOut, int &yOut);
	void GetScrollTranslation(int &xOut, int &yOut);

	// Distance between two objects in feet
	float DistanceToObj(objHndl from, objHndl to);
	float DistanceToObj_NonNegative(objHndl from, objHndl to); // as above but not less than 0.0

	// Distance between from and loc in inches (without the obj radius)
	float DistanceToLoc(objHndl from, LocAndOffsets loc);
	float DistanceToLocFeet(objHndl obj, LocAndOffsets *loc);

	/*
		gets the higher tile delta value (absolute value)
		e.g. if deltaX = 15 and deltaY = -17 it will return 17
	*/
	int64_t (__cdecl*GetTileDeltaMax)(objHndl obj, objHndl obj2);
	int64_t GetTileDeltaMaxBtwnLocs(locXY loc1, locXY loc2);

	float InchesToFeet(float inches);

	int64_t *translationX;
	int64_t *translationY;

	LocAndOffsets TrimToLength(LocAndOffsets srcLoc, LocAndOffsets tgtLoc, float lengthInches);

	/*
	Calculates the angle in radians between two points in the tile coordinate system.
	The angle can be used to make an object that is at fromPoint face the location at toPoint.
	To convert to screen space rotation, which is defined as 0 when facing to the top of the screen and increasing ** clockwise **, do:
	rotation = -angle + 5*pi/4 
	         = -angle - 2.3561945f
	*/
	float AngleBetweenPoints(LocAndOffsets &fromPoint, LocAndOffsets &toPoint);

	XMFLOAT2 GetDirectionVector(LocAndOffsets &fromPoint, LocAndOffsets &toPoint); // returns a normalized vector in the direction from -> to

	LocationSys();
};


float AngleBetweenPoints(LocAndOffsets fromPoint, LocAndOffsets toPoint);

bool operator!=(const LocAndOffsets& to, const LocAndOffsets& rhs);
bool operator==(const LocAndOffsets& to, const LocAndOffsets& rhs);

extern LocationSys locSys;

