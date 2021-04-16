#pragma once

#include "common.h"
#include "util/fixes.h"

struct GameTime {
	int timeInDays = 0;
	int timeInMs = 0;

	GameTime(uint64_t t) {
		timeInMs = (t >> 32) & 0xFFFFFFFF;
		timeInDays = t & 0xFFFFFFFF;
	}

	GameTime() {
	}

	GameTime(int days, int ms) : timeInDays(days), timeInMs(ms) {
	}

	// return 1 if t > t2, -1 if t < t2, 0 if equal
	static int Compare(const GameTime& t, const GameTime& t2){
		if (t2.timeInDays > t.timeInDays)
			return -1;
		if (t2.timeInDays < t.timeInDays)
			return 1;
		
		// days is equal, compare msec
		if (t2.timeInMs > t.timeInMs)
			return -1;
		return (t2.timeInMs < t.timeInMs) ? 1 : 0;
	};

	static GameTime FromSeconds(int seconds) {
		if (seconds == 0) {
			return{ 0, 1 };
		}
		else {
			return{ 0, seconds * 1000 };
		}
	}
};

// GameTime Function Replacements
class GameTimeSystem : TempleFix
{
public: 
	static GameTime ElapsedGetDelta(GameTime * gtime);
	static GameTime GetElapsed();
	static void GameTimeAdd(GameTime* timeDelta)
	{
		orgGameTimeAdd(timeDelta);
	};

	void apply() override
	{
		orgGameTimeAdd = replaceFunction(0x10060C90, GameTimeAdd);
	};
protected:
	static void(__cdecl*orgGameTimeAdd)(GameTime* timeDelta);

};

extern GameTimeSystem gameTimeSys;


