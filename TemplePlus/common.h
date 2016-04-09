#pragma once

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


// this is the number of tiles per sector in each direction (so the total is this squared i.e. 4096 in toee)
#define SECTOR_SIDE_SIZE 64

# pragma region Standard Structs

#include <infrastructure/location.h>

#pragma pack(push, 1)

typedef XMFLOAT2 vector2f;
typedef XMFLOAT3 vector3f;


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
	/*
		Adds a bonus of a particular type.
		Will register in the D20 roll history using the specified line from bonus.mes
	*/
	int AddBonus(int value, int bonType, int mesline);
	int AddBonusWithDesc(int value, int bonType, int mesline, char* descr);
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


inline std::ostream& operator<<(std::ostream& os, const ScreenDirections & direc) {
	switch(direc)
	{
	case ScreenDirections::Top:
		return os << "U";
	case ScreenDirections::TopRight:
		return os << "UR";
	case ScreenDirections::Right:
		return os << "R";
	case ScreenDirections::BottomRight:
		return os << "DR";
	case ScreenDirections::Bottom:
		return os << "D";
	case ScreenDirections::BottomLeft :
		return os << "DL";
	case ScreenDirections::Left:
		return os << "L";
	case ScreenDirections::TopLeft:
		return os << "UL";
	default:
		return os << "NA";
	}
}


struct PointNode
{
	float absX; // world X goes here
	float absZ; // always 0 in toee
	float absY; // world Y goes here
	PointNode();
	PointNode(float x, float y, float z);
};
#pragma endregion