#pragma once

#include <graphics/math.h>

#include <fmt/format.h>

#include <cstdint>

// This is the number of pixels per tile on the x and y axis. Equal to sqrt(800) (which is sqrt(20 * 20 + 20 * 20))
constexpr float INCH_PER_TILE = 28.284271247461900976033774484194f;
constexpr float INCH_PER_HALFTILE = (INCH_PER_TILE / 2.0f);
constexpr float INCH_PER_SUBTILE = (INCH_PER_TILE / 3.0f);
constexpr int INCH_PER_FEET = 12;

#pragma pack(push, 1)

struct locXY {
	uint32_t locx;
	uint32_t locy;

	static locXY fromField(uint64_t location) {
		return *(locXY*)&location;
	}

	uint64_t ToField() const {
		return *((uint64_t*)this);
	}

	operator uint64_t() const {
		return *(uint64_t*)this;
	}

	XMFLOAT2 ToInches2D(float offsetX = 0, float offsetY = 0) const;

	XMFLOAT3 ToInches3D(float offsetX = 0, float offsetY = 0, float offsetZ = 0) const;
};

struct LocAndOffsets {
	locXY location;
	float off_x;
	float off_y;

	XMFLOAT2 ToInches2D() const {
		return location.ToInches2D(off_x, off_y);
	}

	void Normalize();

	XMFLOAT3 ToInches3D(float offsetZ = 0) const {
		return location.ToInches3D(off_x, off_y, offsetZ);
	}

	
	static LocAndOffsets FromInches(float x, float y);

	static LocAndOffsets FromInches(XMFLOAT2 pos) {
		return FromInches(pos.x, pos.y);
	}

	static LocAndOffsets FromInches(XMFLOAT3 pos) {
		return FromInches(pos.x, pos.z);
	}

	static LocAndOffsets create(locXY locXy, float offx, float offy);
	/*LocAndOffsets();
	LocAndOffsets(locXY loc);
	LocAndOffsets(locXY loc, float offx, float offy);*/

	void Regularize(); // ensures the floating point offset corresponds to less than half a tile

	static const LocAndOffsets null;
};

struct LocFull {
	LocAndOffsets location;
	float off_z;
};

#pragma pack(pop)

void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const locXY &loc);
void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const LocAndOffsets &loc);
void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const LocFull &loc);

