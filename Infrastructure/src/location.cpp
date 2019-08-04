#include "infrastructure/location.h"

#include <ostream>
#include <string>
//#include "../../TemplePlus/location.h"

XMFLOAT2 locXY::ToInches2D(float offsetX, float offsetY) const {
	return{
		locx * INCH_PER_TILE + offsetX + INCH_PER_HALFTILE,
		locy * INCH_PER_TILE + offsetY + INCH_PER_HALFTILE
	};
}

XMFLOAT3 locXY::ToInches3D(float offsetX, float offsetY, float offsetZ) const {
	return{
		locx * INCH_PER_TILE + offsetX + INCH_PER_HALFTILE,
		offsetZ,
		locy * INCH_PER_TILE + offsetY + INCH_PER_HALFTILE
	};
}

LocAndOffsets LocAndOffsets::FromInches(float x, float y) {
	float tileX = x / INCH_PER_TILE;
	float tileY = y / INCH_PER_TILE;

	LocAndOffsets result;
	result.location.locx = (int)tileX;
	result.location.locy = (int)tileY;
	result.off_x = (tileX - floor(tileX)) * INCH_PER_TILE - INCH_PER_HALFTILE;
	result.off_y = (tileY - floor(tileY)) * INCH_PER_TILE - INCH_PER_HALFTILE;
	return result;

}

LocAndOffsets LocAndOffsets::create(locXY locXy, float offx, float offy)
{
	LocAndOffsets loc;
	loc.location = locXy;
	loc.off_x = offx;
	loc.off_y = offy;
	return loc;
}

void LocAndOffsets::Regularize(){
	auto loc = this;
	if (abs(loc->off_x) > 14.142136f)
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

//LocAndOffsets::LocAndOffsets()
//{
//	location.locx = 0;
//	location.locy = 0;
//	off_x = 0.0;
//	off_y = 0.0;
//}
//
//LocAndOffsets::LocAndOffsets(locXY loc)
//{
//	location = loc;
//	off_x = 0.0;
//	off_y = 0.0;
//}
//
//LocAndOffsets::LocAndOffsets(locXY loc, float offx, float offy)
//{
//	location = loc;
//	off_x = offx;
//	off_y = offy;
//}

void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const locXY &loc) {
	f.writer().write("{},{}", loc.locx, loc.locy);
}

void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const LocAndOffsets &loc) {
	f.writer().write("{},{},{},{}", loc.location.locx, loc.location.locy, loc.off_x, loc.off_y);
}

void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const LocFull &loc) {
	f.writer().write("{},{}", loc.location, loc.off_z);
}
