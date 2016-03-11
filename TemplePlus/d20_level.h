#pragma once
#include "common.h"
#include "idxtables.h"
#include "config/config.h"
#include "spell_structs.h"

struct LevelPacket;
struct LevelupPacket;

#define XP_REQ_TABLE_MAX_LEVEL 100

class D20LevelSystem
{
public:
	uint32_t xpReqTable[XP_REQ_TABLE_MAX_LEVEL]; //  xp required to reach a certain level, starting from level 0 (will be 0,0,1000,3000,6000,...)
	// uint32_t maxLevel; // now read from config
	uint32_t LevelPacketInit(LevelPacket* lvlPkt);
	uint32_t LevelPacketDealloc(LevelPacket *lvlPkt);
	uint32_t GetLevelPacket(Stat classEnum, objHndl ObjHnd, uint32_t levelAdjustSthg, uint32_t classLevel, LevelPacket *lvlPkt);
	bool CanLevelup(objHndl objHnd);
	D20LevelSystem()
	{
		//maxLevel = 20;
		
		memset(xpReqTable, 0, sizeof(xpReqTable));
		xpReqTable[2] = 1000;
		for (int i = 3; i < XP_REQ_TABLE_MAX_LEVEL; i++)
		{
			xpReqTable[i] = 1000 * (i - 1)*i / 2;
		}
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
	int32_t field_28[NUM_SPELL_LEVELS]; // init to -1's; spell related; idx is spellLevel
	int32_t spellCountFromClass[NUM_SPELL_LEVELS]; // init to -1's; idx is spellLevel
	uint32_t spellCountBonusFromStatMod[NUM_SPELL_LEVELS]; // init to 0; idx is spellLevel
	uint32_t fortitudeSaveBase;
	uint32_t reflexSaveBase;
	uint32_t willSaveBase;
	uint32_t baseAttackBonus;
	uint32_t skillPoints;
	uint32_t baseFeatNum_Maybe;
	uint32_t classFeatsNum;
	LevelPacket();
	~LevelPacket();
	int GetLevelPacket(Stat classCode, objHndl obj, int lvlAdj, int classLvl);
};
#pragma pack(pop)