#pragma once


class LegacyRNG
{
public:
	int GetInt(int min, int max);
};


extern LegacyRNG rngSys;