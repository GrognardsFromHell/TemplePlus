#include "stdafx.h"
#include "location.h"
#include "obj.h"
#include "util/fixes.h"

LocationSys locSys;

typedef int(__cdecl*locfunc)(int64_t x, int64_t y, locXY *loc, float*offx, float*offy);

class LocReplacement : public TempleFix
{
public: const char* name() override 
{ return "Location" "Function Replacements";}

		static locfunc orgGetLocFromScreenLoc;
		/*
			gets the game location tile from the screen pixel (screenX,screenY). often used for the center of the screen (W/2,H/2) and the corner (0,0).
		*/
		static BOOL GetLocFromScreenLoc(int64_t screenX, int64_t screenY, locXY *loc, float*offx, float*offy)
{
	int64_t xx = (screenX - *locSys.locTransX) / 2;
	int64_t yy = (int64_t)((screenY - *locSys.locTransY) / 2 * sqrt(2));
	int result = orgGetLocFromScreenLoc(screenX,screenY,loc,offx,offy);
	return result;
};

		void apply() override 
	{
	//	orgGetLocFromScreenLoc=replaceFunction(0x10029300, GetLocFromScreenLoc);
	}
}locRep;
locfunc LocReplacement::orgGetLocFromScreenLoc;

const float PIXEL_PER_TILE = sqrt(800.0f);
const float PIXEL_PER_TILE_HALF = PIXEL_PER_TILE / 2;

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

void LocationSys::RegularizeLoc(LocAndOffsets* loc)
{
	if ( abs(loc->off_x) > 14.142136f)
	{
		while (loc->off_x >= 14.142136f)
		{
			loc->off_x -= 28.284271f;
			loc->location.locx++;
		}


		while (loc->off_x < -14.142136f)
		{
			loc->off_x += 28.284271f;
			loc->location.locx--;
		}
	}

	if (abs(loc->off_y) > 14.142136f)
	{
		while (loc->off_y >= 14.142136f)
		{
		loc->off_y -= 28.284271f;
		loc->location.locy++;
		}


		while (loc->off_y < -14.142136f)
		{
		loc->off_y += 28.284271f;
		loc->location.locy--;
		}
	}
}

void LocationSys::GetOverallOffset(LocAndOffsets loc, float* absX, float* absY)
{
	*absX = loc.location.locx * PIXEL_PER_TILE + PIXEL_PER_TILE_HALF + loc.off_x;
	*absY = loc.location.locy * PIXEL_PER_TILE + PIXEL_PER_TILE_HALF + loc.off_y;
}

BOOL LocationSys::ShiftLocationByOneSubtile(LocAndOffsets* loc, ScreenDirections direction, LocAndOffsets* locOut)
{
	*locOut = *loc;
	if (direction < ScreenDirections::DirectionsNum)
	{
		switch (direction)
		{
		case ScreenDirections::Top: 
			locOut->off_x -= 9.4280901f;
			locOut->off_y -= 9.4280901f;
			break;
		case ScreenDirections::TopRight: 
			locOut->off_x -= 9.4280901f;
			break;
		case ScreenDirections::Right:
			locOut->off_x -= 9.4280901f;
			locOut->off_y += 9.4280901f;
			break;
		case ScreenDirections::BottomRight:
			locOut->off_y += 9.4280901f;
			break;
		case ScreenDirections::Bottom:
			locOut->off_x += 9.4280901f;
			locOut->off_y += 9.4280901f;
			break;
		case ScreenDirections::BottomLeft:
			locOut->off_x += 9.4280901f;
			break;
		case ScreenDirections::Left:
			locOut->off_x += 9.4280901f;
			locOut->off_y -= 9.4280901f;
			break;
		case ScreenDirections::TopLeft:
			locOut->off_y -= 9.4280901f;
				break;
		default:
			break;
		}
	}
	RegularizeLoc(locOut);
	return 1;
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

void LocationSys::ToTranslation(int x, int y, int &xOut, int &yOut) {
	xOut = (int) *translationX + (y - x - 1) * 20i64;
	yOut = (int) *translationY + (x + y) * 14i64;
}

void LocationSys::TileToScreen(int x, int y, int& xOut, int& yOut) {
	xOut = (y - x - 1) * 20;
	yOut = (x + y) * 14;
}

void LocationSys::GetScrollTranslation(int& xOut, int& yOut) {
	xOut = (int) *translationX;
	yOut = (int) *translationY;
}

float LocationSys::DistanceToLoc(objHndl from, LocAndOffsets loc) {
	auto objLoc = objects.GetLocationFull(from);
	auto distance = Distance3d(objLoc, loc);
	auto radius = objects.GetRadius(from);
	return distance - radius;
}

float LocationSys::InchesToFeet(float inches) {
	return inches / 12.0f;
}

LocationSys::LocationSys()
{
	rebase(getLocAndOff, 0x10040080);
	rebase(SubtileToLocAndOff, 0x100400C0);
	rebase(subtileFromLoc,0x10040750); 
	rebase(ShiftSubtileOnceByDirection, 0x10029DC0);
	rebase(TOEEdistBtwnLocAndOffs,0x1002A0A0); 
	rebase(DistanceToObj, 0x100236E0);
	rebase(Distance3d, 0x1002A0A0);
	rebase(translationX, 0x10808D00);
	rebase(translationY, 0x10808D48);
	
}

float AngleBetweenPoints(LocAndOffsets fromPoint, LocAndOffsets toPoint) {

	auto fromCoord = fromPoint.ToInches2D();
	auto toCoord = fromPoint.ToInches2D();
	
	// Create the vector from->to
	auto dir = XMFLOAT2(
		toCoord.x - fromCoord.x,
		toCoord.y - fromCoord.y
	);

	auto angle = atan2(dir.y, dir.x);
	return angle + 2.3561945f; // + 135 degrees
}

bool operator!=(const LocAndOffsets& to, const LocAndOffsets& rhs)
{
	if (to.location.locx != rhs.location.locx 
		|| to.location.locy != rhs.location.locy
		|| abs(to.off_x - rhs.off_x) > LOCATION_OFFSET_TOL
		|| abs(to.off_y - rhs.off_y) > LOCATION_OFFSET_TOL)
		return true;
	return false;
}

bool operator==(const LocAndOffsets& to, const LocAndOffsets& rhs)
{
	if (to != rhs)
		return false;
	return true;
}