#include "stdafx.h"
#include "gametime.h"
#include <temple/dll.h>

constexpr int TIME_1DAY_IN_MSEC = 86400000;

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

GameTime GameTimeSystem::GetElapsed() {
	return *addresses.gameTimeElapsed;
}

void(__cdecl*GameTimeSystem::orgGameTimeAdd)(GameTime* timeDelta);

void GameTime::operator+=(const GameTime& amt)
{
	timeInMs += amt.timeInMs;
	timeInDays += amt.timeInDays;
	if (timeInMs >= TIME_1DAY_IN_MSEC) {
		timeInDays += timeInMs / TIME_1DAY_IN_MSEC;
		timeInMs = timeInMs % TIME_1DAY_IN_MSEC;
	}
}

GameTime GameTime::operator-(const GameTime& t0)
{
	GameTime result;
	result.timeInDays = this->timeInDays - t0.timeInDays;
	result.timeInMs = this->timeInMs - t0.timeInMs;

	if (result.timeInMs <= -TIME_1DAY_IN_MSEC) {
		int deltaInDays = (-result.timeInMs) / TIME_1DAY_IN_MSEC;
		result.timeInDays -= deltaInDays;
		result.timeInMs += deltaInDays * TIME_1DAY_IN_MSEC;
	}
	return result;
}

int GameTime::ToMs()
{
	return timeInMs + timeInDays * TIME_1DAY_IN_MSEC;
}
