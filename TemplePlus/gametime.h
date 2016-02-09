#pragma once

#include "common.h"
#include "util/fixes.h"

struct GameTime {
	int timeInDays = 0;
	int timeInMs = 0;

	GameTime(uint64_t t) {
		timeInDays = (t >> 32) & 0xFFFFFFFF;
		timeInMs = t & 0xFFFFFFFF;
	}

	GameTime() {
	}

	GameTime(int days, int ms) : timeInDays(days), timeInMs(ms) {
	}

	static GameTime FromSeconds(int seconds) {
		if (seconds == 0) {
			return{ 0, 1 };
		}
		else {
			return{ 0, seconds * 1000 };
		}
	}
};


class GameTimeSystem : TempleFix
{
public: 
	const char* name() override {
		return "GameTime Function Replacements"; 
	} 
	
	static GameTime ElapsedGetDelta(GameTime * gtime);
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


