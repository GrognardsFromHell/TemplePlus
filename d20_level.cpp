#include "stdafx.h"
#include "d20_level.h"
#include "common.h"
#include "obj.h"


D20LevelSystem d20LevelSys;

struct D20LevelSystemAddresses : temple::AddressTable
{
	uint32_t * xpReqTable;
	uint32_t (__cdecl *LevelPacketInit)(LevelPacket *lvlPkt);
	uint32_t(__cdecl *LevelPacketDealloc)(LevelPacket* lvlPkt);
	uint32_t(__cdecl *GetLevelPacket)(Stat classEnum, objHndl objHnd, uint32_t levelAdjustSthg, uint32_t classLevel, LevelPacket* lvlPkt);
	D20LevelSystemAddresses()
	{
		rebase(xpReqTable, 0x102AAF00); // xp needed to reach level N, starting from N=0 (0 , 0 , 1000, 3000, ...)

		rebase(LevelPacketDealloc, 0x100F4780);
		rebase(LevelPacketInit, 0x100F5520);
		rebase(GetLevelPacket, 0x100F5140);
	};
} addresses;


uint32_t D20LevelSystem::LevelPacketInit(LevelPacket* lvlPkt)
{
	return addresses.LevelPacketInit(lvlPkt);
}

uint32_t D20LevelSystem::LevelPacketDealloc(LevelPacket* lvlPkt)
{
	return addresses.LevelPacketDealloc(lvlPkt);
}

uint32_t D20LevelSystem::GetLevelPacket(Stat classEnum, objHndl objHnd, uint32_t levelAdjustSthg, uint32_t classLevel, LevelPacket* lvlPkt)
{
	return addresses.GetLevelPacket(classEnum, objHnd, levelAdjustSthg, classLevel, lvlPkt);
}

bool D20LevelSystem::CanLevelup(objHndl objHnd)
{
	auto lvl = objects.StatLevelGet(objHnd, stat_level);
	if (d20Sys.d20Query(objHnd, DK_QUE_ExperienceExempt) || lvl < 0 || lvl >= d20LevelSys.maxLevel)
	{
		return 0;
	}
	return objects.getInt32(objHnd, obj_f_critter_experience) >= addresses.xpReqTable[lvl + 1];

}

uint32_t _CanLevelup(objHndl objHnd)
{
	return d20LevelSys.CanLevelup(objHnd);
}