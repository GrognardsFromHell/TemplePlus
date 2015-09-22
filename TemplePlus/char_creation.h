#include "stdafx.h"
#include "common.h"
#include "util/config.h"

class CharCreationSystem : TempleFix
{
	public: 
	const char* name() override { 
		return "CharCreation" "Function Replacements";
	} 

	void SetPointBuyPoints(int points);

	void apply() override 
	{
		SetPointBuyPoints(config.pointBuyPoints);
	}
};

extern CharCreationSystem charCreationSyS;