#pragma once
#include "stdafx.h"
#include "common.h"
#include "idxtables.h"

struct LevelPacket;

class D20LevelSystem
{
public:
	uint32_t maxLevel;
	uint32_t LevelPacketInit(LevelPacket* lvlPkt);
	uint32_t LevelPacketDealloc(LevelPacket *lvlPkt);
	uint32_t GetLevelPacket(Stat classEnum, objHndl ObjHnd, uint32_t levelAdjustSthg, uint32_t classLevel, LevelPacket *lvlPkt);
	bool CanLevelup(objHndl objHnd);
	D20LevelSystem()
	{
		maxLevel = 20;
	}
};

extern D20LevelSystem d20LevelSys;


#pragma pack(push,1)
struct LevelPacket
{
	objHndl objHnd;
	uint32_t levelAdjustmentSthg;
	uint32_t classLevel;
	Stat classEnum;
	IdxTable<feat_enums> featsIdxTable; // this gets filled from the ClassPacket
	uint32_t hitDiceWithConBonus; // in the triplet format
	int32_t field_28[10]; // init to -1's; spell related; idx is spellLevel
	int32_t spellCountFromClass[10]; // init to -1's; idx is spellLevel
	uint32_t spellCountBonusFromStatMod[10]; // init to 0; idx is spellLevel
	uint32_t fortitudeSaveBase;
	uint32_t reflexSaveBase;
	uint32_t willSaveBase;
	uint32_t baseAttackBonus;
	uint32_t skillPoints;
	uint32_t baseFeatNum_Maybe;
	uint32_t classFeatsNum;
};
#pragma pack(pop)