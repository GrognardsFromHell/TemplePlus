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


struct BonusEntry
{
	int32_t value;
	uint32_t bonusType_maybe; // gets comapred with 0, 8 and 21 in 100E6490; I think these types allow bonuses to stack
	char * rollHistoryString;
	uint32_t fieldC;
};

struct BonusList
{
	BonusEntry bonusEntries[40];
	uint32_t count;
	BonusEntry field284[10];
	uint32_t count2;
	uint32_t field328[10];
	uint32_t count3;
	int32_t maxSthg; //init to largest  positive int
	uint32_t field358;
	uint32_t field35C;
	uint32_t field360;
	int32_t minSthg; //init to most negative int
	uint32_t field368;
	uint32_t field36C;
	uint32_t field370;
	uint32_t flags; // init 0; compared to 0x1 , 0x2 at 0x100E6641
	
	BonusList()
	{
		this->count = 0;
		this->count2 = 0;
		this->count3 = 0;
		this->flags = 0;
		this->maxSthg = 0x7fffFFFF;
		this->minSthg = 0x80000001;
	}
};
#pragma pack(pop)

const int TestSizeOfStruct378 = sizeof(BonusList); // should be 888 (0x378)

#pragma endregion