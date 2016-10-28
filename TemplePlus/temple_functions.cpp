
#include "stdafx.h"
#include <temple/dll.h>
#include "temple_functions.h"
#include "tig/tig_mouse.h"
#include "tig/tig_msg.h"

//#include "spell.h"
#include "common.h"
#include "config/config.h"
#include "util/fixes.h"

TempleFuncs templeFuncs;

class TempleFuncReplacements : public TempleFix
{
	void apply() override 
	{
		replaceFunction(0x10038B60, _diceRoll); 
	}
} templeFuncReplacements;

#pragma region TempleFuncs Implementation

int32_t TempleFuncs::diceRoll(uint32_t dieNum, uint32_t dieType, int32_t dieBonus)
{
	int32_t result = dieBonus;
	for (uint32_t i = 0; i < dieNum; i++)
	{
		result += RNG(1, dieType);
	}
	return result;
}

TempleFuncs::TempleFuncs()
{
	rebase(ProcessSystemEvents, 0x101DF440);
	rebase(StringHash, 0x101EBB00);
	rebase(RNG, 0x10038DF0);
	rebase(UpdatePartyUI, 0x10134CB0);



	rebase(ObjStatBaseDispatch, 0x1004E810);

	
	rebase(sub_100664B0, 0x100664B0);

}

#pragma endregion


#pragma region TempleFuncs replacements
int32_t _diceRoll(uint32_t dieNum, uint32_t dieType, int32_t dieBonus)
{
	return templeFuncs.diceRoll(dieNum, dieType, dieBonus);
}
#pragma endregion