#pragma once

#include "stdafx.h"
#include "util/addresses.h"
#include "util/fixes.h"
#include "obj_structs.h"
#include "spell_structs.h"
#include "temple_enums.h"
#include "d20_defs.h"
#include "util/fixes.h"

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

# pragma region Standard Structs

#pragma pack(push, 1)
enum SkillEnum : uint32_t;

struct locXY{
	uint32_t locx;
	uint32_t locy;
};

struct LocAndOffsets {
	locXY location;
	float off_x;
	float off_y;
};

struct LocFull {
	LocAndOffsets location;
	float off_z;
};

struct GroupArray {
	objHndl GroupMembers[32];
	uint32_t * GroupSize;
	void* unknownfunc;
};


struct StandPoint {
	uint32_t mapNum;
	uint32_t field4;
	LocAndOffsets LocAndOff;
	_jmpPntID jmpPntID;
	uint32_t field_1C;
};

struct JumpPointPacket{
	_jmpPntID jmpPntID;
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
};

const int TestSizeOfBonusList = sizeof(BonusList); // should be 888 (0x378)
#pragma endregion