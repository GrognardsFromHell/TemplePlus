#pragma once

#include <d3dx9math.h>
#include <graphics/math.h>

#include "obj_structs.h"
#include "d20_defs.h"
#include "temple_enums.h"

#define macAsmProl {\
	__asm push ecx\
	__asm push esi\
	__asm push ebx\
	__asm push edi};

#define macAsmEpil {\
	__asm pop edi\
	__asm pop ebx\
	__asm pop esi\
	__asm pop ecx}\

#define macRebase(funName , b) rebase(funName, 0x##b); 
#define macReplaceFun(b, funName) replaceFunction(0x##b, funName); 
#define macAsmThis(funName) {\
	__asm mov ecx, this\
	__asm mov esi, [ecx]._##funName }

#define macTempleFix(sysName) public:\
	const char* name() override {\
			return #sysName "Function Replacements";}\
			void apply() override \

#define BonusListMax 40

// This is the number of pixels per tile on the x and y axis. Derived from sqrt(800)
constexpr float INCH_PER_TILE = 28.284271247461900976033774484194f;
constexpr float INCH_PER_HALFTILE = (INCH_PER_TILE / 2.0f);
constexpr float INCH_PER_SUBTILE = (INCH_PER_TILE / 3.0f);
constexpr int INCH_PER_FEET = 12;

// this is the number of tiles per sector in each direction (so the total is this squared i.e. 4096 in toee)
#define SECTOR_SIDE_SIZE 64

# pragma region Standard Structs

#pragma pack(push, 1)

typedef XMFLOAT2 vector2f;
typedef XMFLOAT3 vector3f;

struct locXY {
	uint32_t locx;
	uint32_t locy;

	static locXY fromField(uint64_t location) {
		return *(locXY*)&location;
	}

	uint64_t ToField() {
		return *((uint64_t*)this);
	}

	operator uint64_t() const {
		return *(uint64_t*)this;
	}

	vector2f ToInches2D(float offsetX = 0, float offsetY = 0) {
		return{
			locx * INCH_PER_TILE + offsetX,
			locy * INCH_PER_TILE + offsetY
		};
	}

	vector3f ToInches3D(float offsetX = 0, float offsetY = 0, float offsetZ = 0) {
		return {
			locx * INCH_PER_TILE + offsetX,
			offsetZ,
			locy * INCH_PER_TILE + offsetY
		};
	}
};

struct SectorLoc
{
	uint64_t raw;

	uint64_t x()
	{
		return raw & 0x3ffFFFF;
	}

	uint64_t y()
	{
		return raw >> 26 ;
	}

	uint64_t ToField() {
		return this->raw;
	}

	operator uint64_t() const {
			return this->raw;
		}

	operator int64_t() const {
		return (int64_t)this->raw;
	}

	void GetFromLoc(locXY loc)
	{
		raw = loc.locx / SECTOR_SIDE_SIZE
			 + ( (loc.locy / SECTOR_SIDE_SIZE) << 26 );
	}

	SectorLoc()
	{
		raw = 0;
	}

	SectorLoc(locXY loc)
	{
		raw = loc.locx / SECTOR_SIDE_SIZE
			+ ((loc.locy / SECTOR_SIDE_SIZE) << 26);
	}

	SectorLoc(uint64_t sectorLoc) {
		raw = sectorLoc;
	}

	SectorLoc(int sectorX, int sectorY) {
		raw = (sectorX & 0x3ffFFFF) | (sectorY << 26);
	}

	locXY GetBaseTile()
	{
		locXY loc;
		loc.locx = (int) x() * SECTOR_SIDE_SIZE;
		loc.locy = (int) y() * SECTOR_SIDE_SIZE;
		return loc;
	}

	bool operator ==(SectorLoc secLoc) {
		return raw == secLoc.raw;
	}

};

struct Subtile // every tile is subdivided into 3x3 subtiles
{
	int32_t x;
	int32_t y;

	static Subtile fromField(int64_t location) {
		return *(Subtile*)&location;
	}

	int64_t ToField() {
		return *((int64_t*)this);
	}

	operator int64_t() const {
		return *(int64_t*)this;
	}

	operator Subtile() const
	{
		return *(Subtile*)this;
	}
};

struct TileRect
{
	int64_t x1;
	int64_t y1;
	int64_t x2;
	int64_t y2;
};

struct LocAndOffsets {
	locXY location;
	float off_x;
	float off_y;
	
	vector2f ToInches2D() {
		return location.ToInches2D(off_x, off_y);
	}

	// Same as ToInches2d, but translates to center of tile
	vector2f ToCenterOfTileAbs();

	// Same as ToInches3d, but translates to center of tile
	XMFLOAT3 ToCenterOfTileAbs3D(float offsetZ = 0);

	vector3f ToInches3D(float offsetZ = 0) {
		return location.ToInches3D(off_x, off_y, offsetZ);
	}
	
	static LocAndOffsets FromInches(float x, float y) {
		float tileX = x / INCH_PER_TILE;
		float tileY = y / INCH_PER_TILE;

		LocAndOffsets result;		
		result.location.locx = (int)tileX;
		result.location.locy = (int)tileY;
		result.off_x = (tileX - floor(tileX) - 0.5f) * INCH_PER_TILE;
		result.off_y = (tileY - floor(tileY) - 0.5f) * INCH_PER_TILE;
		return result;
	}

};

inline vector2f LocAndOffsets::ToCenterOfTileAbs() {
	auto result = ToInches2D();
	result.x += INCH_PER_HALFTILE;
	result.y += INCH_PER_HALFTILE;
	return result;
}

inline XMFLOAT3 LocAndOffsets::ToCenterOfTileAbs3D(float offsetZ) {
	auto abs2d(ToInches2D());
	XMFLOAT3 result{ abs2d.x, offsetZ, abs2d.y };
	result.x += INCH_PER_HALFTILE;
	result.z += INCH_PER_HALFTILE;
	return result;
}

inline std::ostream& operator<<(std::ostream& os, const LocAndOffsets & loc) {

	return os
		<< std::to_string(loc.location.locx)
		+ "," + std::to_string(loc.location.locy)
		+ "," + std::to_string(loc.off_x)
		+ "," + std::to_string(loc.off_y);
}

struct LocFull {
	LocAndOffsets location;
	float off_z;
};

struct GroupArray {
	objHndl GroupMembers[32];
	uint32_t GroupSize;
	int (__cdecl*sortFunc)(void*, void*); // used for comparing two items (e.g. alphabetic sorting)
};

struct JumpPointPacket {
	int jmpPntId;
	char * pMapName;
	_mapNum mapNum;
	uint32_t field_C;
	locXY location;
};



#pragma pack(pop)


struct BonusEntry
{
	int32_t bonValue;
	uint32_t bonType; // gets comapred with 0, 8 and 21 in 100E6490; I think these types allow bonuses to stack
	char * bonusMesString; // parsable string for the help system e.g. "~Item~[TAG_ITEM]"
	char * bonusDescr; // e.g. "Magic Full Plate +1"
};

struct BonusCap
{
	int capValue;
	int bonType;
	char *bonCapperString;
	char * bonCapDescr;
};

struct BonusList
{
	BonusEntry bonusEntries[40];
	uint32_t bonCount;
	BonusCap bonCaps[10];
	uint32_t bonCapperCount;
	uint32_t zeroBonusReasonMesLine[10]; // a line from the bonus.mes that is auto assigned a 0 value (I think it will print ---). Probably for overrides like racial immunity and stuff.
	uint32_t zeroBonusCount;
	int32_t overallCapHigh; // init to largest  positive int; controlls what the sum of all the modifiers of various types cannot exceed
	uint32_t field358; // looks unused
	char * overallCapHighBonusMesString;
	char * overallCapHighDescr;
	int32_t overallCapLow; //init to most negative int
	uint32_t field368; // looks unused
	char * overallCapLowBonusMesString;
	char * overallCapLowDescr;
	uint32_t bonFlags; // init 0; 0x1 - overallCapHigh set; 0x2 - overallCapLow set; 0x4 - force cap override (otherwise it can only impose restrictions i.e. it will only change the cap if it's lower than the current one)

	BonusList()
	{
		this->bonCount = 0;
		this->bonCapperCount = 0;
		this->zeroBonusCount = 0;
		this->bonFlags = 0;
		this->overallCapHigh = 0x7fffFFFF;
		this->overallCapLow = 0x80000001;
	}

	int GetEffectiveBonusSum() const;

	/**
	 * Returns true if the given bonus is suppressed by another bonus (i.e. of the same type and
	 * the bonus type does not stack). If true is returned, the idx of the supressing bonus is returned
	 * in suppressedByIdx.
	 */
	bool IsBonusSuppressed(size_t bonusIdx, size_t* suppressedByIdx) const;
	
	/**
	 * Returns true if the given bonus is capped by one of the caps in this bonus list and returns the
	 * index of the lowest cap in cappedByIdx, if the pointer is not null.
	 */
	bool IsBonusCapped(size_t bonusIdx, size_t* cappedByIdx) const;

};

const int TestSizeOfBonusList = sizeof(BonusList); // should be 888 (0x378)


struct AttackPacket
{
	objHndl attacker;
	objHndl victim;
	D20ActionType d20ActnType;
	int dispKey;
	D20CAF flags;
	int field_1C;
	objHndl weaponUsed;
	objHndl ammoItem;
};

enum ScreenDirections : char {
	Top = 0,
	TopRight = 1,
	Right,
	BottomRight,
	Bottom,
	BottomLeft,
	Left,
	TopLeft,
	DirectionsNum // 8
};
#pragma endregion