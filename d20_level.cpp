#include "stdafx.h"
#include "d20_level.h"
#include "common.h"


D20LevelSystem d20LevelSys;

struct D20LevelSystemAddresses : AddressTable
{
	uint32_t (__cdecl *LevelPacketInit)(LevelPacket *lvlPkt);
	uint32_t(__cdecl *LevelPacketDealloc)(LevelPacket* lvlPkt);
	uint32_t(__cdecl *GetLevelPacket)(Stat classEnum, objHndl objHnd, uint32_t levelAdjustSthg, uint32_t classLevel, LevelPacket* lvlPkt);
	D20LevelSystemAddresses()
	{
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