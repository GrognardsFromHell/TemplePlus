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

# pragma region Standard Structs

#pragma pack(push, 1)


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

#pragma endregion