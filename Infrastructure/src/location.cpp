#include "infrastructure/location.h"

#include <ostream>
#include <string>

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

std::ostream& operator<<(std::ostream& os, const locXY& loc) {
	return os
		<< std::to_string(loc.locx)
		+ "," + std::to_string(loc.locy);
}

std::ostream& operator<<(std::ostream& os, const LocAndOffsets& loc) {
	return os
		<< std::to_string(loc.location.locx)
		+ "," + std::to_string(loc.location.locy)
		+ "," + std::to_string(loc.off_x)
		+ "," + std::to_string(loc.off_y);
}

std::ostream& operator<<(std::ostream& os, const LocFull& loc) {
	return os
		<< std::to_string(loc.location.location.locx)
		+ "," + std::to_string(loc.location.location.locy)
		+ "," + std::to_string(loc.location.off_x)
		+ "," + std::to_string(loc.location.off_y)
		+ "," + std::to_string(loc.off_z);
}
