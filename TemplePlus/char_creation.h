#include "common.h"
#include "util/config.h"
#include "util/fixes.h"

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