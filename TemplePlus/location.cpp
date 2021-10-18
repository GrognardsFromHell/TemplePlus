#include "stdafx.h"
#include "location.h"
#include "obj.h"
#include "util/fixes.h"
#include "gamesystems/objects/objsystem.h"

const LocAndOffsets LocAndOffsets::null{ locXY::fromField(0), 0,0 };

LocationSys locSys;

struct LocationAddresses : temple::AddressTable{
	int(__cdecl*GetLocFromScreenLocPrecise)(int64_t x, int64_t y, locXY* location, float* offX, float* offY);

	LocationAddresses(){
		rebase(GetLocFromScreenLocPrecise, 0x10029300);
	}
	
} locAddresses;

typedef int(__cdecl*locfunc)(int64_t x, int64_t y, locXY *loc, float*offx, float*offy);

class LocReplacement : public TempleFix
{
public:

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
	/*
		checks if two locations are identical (up to floating point tolerance)
	*/
	static BOOL LocationCompare(LocAndOffsets * loc1, LocAndOffsets * loc2)
	{
		return loc1 == loc2
			|| loc1->location.locx == loc2->location.locx
			&& loc1->location.locy == loc2->location.locy
			&& fabs(loc1->off_x - loc2->off_x) < 0.0000001192092895507812
			&& fabs(loc1->off_y - loc2->off_y) < 0.0000001192092895507812;
	};

		void apply() override 
	{
		replaceFunction(0x1003FFC0, LocationCompare);
	//	orgGetLocFromScreenLoc=replaceFunction(0x10029300, GetLocFromScreenLoc);
	}
}locRep;
locfunc LocReplacement::orgGetLocFromScreenLoc;

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

int LocationSys::GetLocFromScreenLocPrecise(int x, int y, LocAndOffsets &loc){
	return locAddresses.GetLocFromScreenLocPrecise(x, y, &loc.location, &loc.off_x, &loc.off_y);
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

float LocationSys::DistanceToObj(objHndl from, objHndl to){
	if (!from || !to)
		return 0.0f;
	auto fromObj = objSystem->GetObject(from);
	auto toObj = objSystem->GetObject(to);

	if (fromObj->IsItem()) {
		auto parent = inventory.GetParent(from);
		if (parent == to)
			return 0.0f;
	}
	if (toObj->IsItem()){
		auto parent = inventory.GetParent(to);
		if (parent == from)
			return 0.0f;
	}

	auto fromLoc = fromObj->GetLocationFull();
	auto toLoc = toObj->GetLocationFull();
	auto fromRadius = objects.GetRadius(from);
	auto toRadius = objects.GetRadius(to);
	auto result = (Distance3d(fromLoc, toLoc) - (fromRadius + toRadius)) / INCH_PER_FEET;
	return result;
}

float LocationSys::DistanceToObj_NonNegative(objHndl from, objHndl to)
{
	auto result = DistanceToObj(from, to);
	if (result < 0.f)
		result = 0.f;
	return result;
}

float LocationSys::DistanceToLoc(objHndl from, LocAndOffsets loc) {
	auto objLoc = objects.GetLocationFull(from);
	auto distance = Distance3d(objLoc, loc);
	auto radius = objects.GetRadius(from);
	return distance - radius;
}

/* 0x10023800 */
float LocationSys::DistanceToLocFeet(objHndl obj, LocAndOffsets* loc)
{
	auto objLoc = objects.GetLocationFull(obj);
	auto distance =  Distance3d(objLoc, *loc);
	auto radius = objects.GetRadius(obj);
	return InchesToFeet(distance - radius);
}

/* 0x100B4940 */
float LocationSys::DistanceToLocFeet_NonNegative(objHndl obj, LocAndOffsets* loc)
{
	auto result = DistanceToLocFeet(obj, loc);
	if (result < 0.0f)
		result = 0.0f;
	return result;
}


int64_t LocationSys::GetTileDeltaMaxBtwnLocs(locXY loc1, locXY loc2){
	return temple::GetRef<int64_t(__cdecl)(locXY, locXY)>(0x1002A030)(loc1, loc2);
}

float LocationSys::InchesToFeet(float inches) {
	constexpr float FEET_TO_INCH = (float)( 1 / 12.0);
	return inches * FEET_TO_INCH;
}

LocAndOffsets LocationSys::TrimToLength(LocAndOffsets srcLoc, LocAndOffsets tgtLoc, float lengthInches){
	float srcAbsX, srcAbsY, tgtAbsX, tgtAbsY;
	GetOverallOffset(srcLoc, &srcAbsX, &srcAbsY);
	GetOverallOffset(tgtLoc, &tgtAbsX, &tgtAbsY);
	auto deltaX = tgtAbsX - srcAbsX;
	auto deltaY = tgtAbsY - srcAbsY;
	auto norm = 1.0f / sqrt(deltaX*deltaX + deltaY*deltaY);

	return LocAndOffsets::FromInches(srcAbsX + norm * deltaX * lengthInches, srcAbsY + norm * deltaY * lengthInches);
}

float LocationSys::AngleBetweenPoints(LocAndOffsets &fromPoint, LocAndOffsets &toPoint)
{
	auto fromCoord = fromPoint.ToInches2D();
	auto toCoord = toPoint.ToInches2D();

	// Create the vector from->to
	auto dir = XMFLOAT2(
		toCoord.x - fromCoord.x,
		toCoord.y - fromCoord.y
	);

	auto angle = atan2(dir.y, dir.x);
	return angle; //+ 2.3561945f; // + 135 degrees
}

XMFLOAT2 LocationSys::GetDirectionVector(LocAndOffsets & fromPoint, LocAndOffsets & toPoint)
{
	auto fromCoord = fromPoint.ToInches2D();
	auto toCoord = toPoint.ToInches2D();

	// Create the vector from->to
	auto dir = XMFLOAT2(
		toCoord.x - fromCoord.x,
		toCoord.y - fromCoord.y
	);

	auto normalizer = sqrt(dir.x * dir.x + dir.y * dir.y);
	if (normalizer > 0){
		dir.x = dir.x / normalizer;
		dir.y = dir.y / normalizer;
	}
		

	return dir;
}

LocationSys::LocationSys()
{
	rebase(getLocAndOff, 0x10040080);
	rebase(SubtileToLocAndOff, 0x100400C0);
	rebase(subtileFromLoc,0x10040750); 
	rebase(PointNodeInit, 0x100408A0);

	//rebase(DistanceToObj, 0x100236E0);
	
	rebase(ShiftSubtileOnceByDirection, 0x10029DC0);
	rebase(Distance3d, 0x1002A0A0);
	rebase(translationX, 0x10808D00);
	rebase(translationY, 0x10808D48);
	rebase(GetTileDeltaMax ,0x1001F830);
	rebase(TOEEdistBtwnLocAndOffs, 0x1002A0A0);
	
}

float AngleBetweenPoints(LocAndOffsets fromPoint, LocAndOffsets toPoint) {

	auto fromCoord = fromPoint.ToInches2D();
	auto toCoord = toPoint.ToInches2D();
	
	// Create the vector from->to
	auto dir = XMFLOAT2(
		toCoord.x - fromCoord.x,
		toCoord.y - fromCoord.y
	);

	auto angle = atan2(dir.y, dir.x);
	return angle; //+ 2.3561945f; // + 135 degrees
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