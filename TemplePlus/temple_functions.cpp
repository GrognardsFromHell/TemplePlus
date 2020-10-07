
#include "stdafx.h"
#include <temple/dll.h>
#include "temple_functions.h"
#include "tig/tig_mouse.h"
#include "tig/tig_msg.h"

//#include "spell.h"
#include "common.h"
#include "config/config.h"
#include "util/fixes.h"
#include "rng.h"
#include <dungeon_master.h>

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
	auto dmFudge = dmSys.GetDiceRollForcing();
	int32_t result = dieBonus;

	switch (dmFudge){
	case 0:
		for (uint32_t i = 0; i < dieNum; i++) {
			result += rngSys.GetInt(1, dieType);
		}
		return result;
	case 1:
		result += 1 * dieNum;
		return result;
	case 2:
		result +=  (int)ceil( ( (1.0 + dieType) * dieNum) / 2); // average, rounded up for even dice
		return result;
	case 3:
		result += (int)ceil(((1.0 + dieType) * dieNum) / 2); // average, rounded up for even dice
		if (dieType >=2){
			result += 2 * dieNum;
		}
		return result;
	case 4:
		result += dieNum * dieType;
		return result;
	default: 
		break;
	}
	for (uint32_t i = 0; i < dieNum; i++){
		result += rngSys.GetInt(1, dieType);
	}
	return result;
}

TempleFuncs::TempleFuncs()
{
	rebase(StringHash, 0x101EBB00);

	
	rebase(sub_100664B0, 0x100664B0);

}

#pragma endregion


#pragma region TempleFuncs replacements
int32_t _diceRoll(uint32_t dieNum, uint32_t dieType, int32_t dieBonus)
{
	return templeFuncs.diceRoll(dieNum, dieType, dieBonus);
}
#pragma endregion