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

	void apply() override
	{
	};
};

extern GameTimeSystem gameTimeSys;


