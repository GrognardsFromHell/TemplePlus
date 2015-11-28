#include "stdafx.h"
#include "gametime.h"
#include <temple/dll.h>


class GameTimeSystem gameTimeSys;

struct GameTimeAddresses : temple::AddressTable
{
	GameTime * gameTimePlayed;
	GameTime * gameTimeElapsed;
	
	GameTimeAddresses()
	{
		rebase(gameTimePlayed, 0x10AA83B8);
		rebase(gameTimeElapsed, 0x10AA83C0);
	}
} addresses;

GameTime GameTimeSystem::ElapsedGetDelta(GameTime* gtime)
{
	GameTime result;
	result.timeInDays = addresses.gameTimeElapsed->timeInDays - gtime->timeInDays;
	result.timeInMs = addresses.gameTimeElapsed->timeInMs - gtime->timeInMs;

	return result;
}