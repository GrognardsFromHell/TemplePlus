
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
	rebase(dicePack,0x10038C50); 
	rebase(UpdatePartyUI, 0x10134CB0);
	rebase(PartyMoney, 0x1002B750);
	rebase(DebitPartyMoney, 0x1002C020);

	rebase(PyObjFromObjHnd, 0x100AF1D0);
	rebase(GetProtoHandle, 0x1003AD70);


	rebase(ObjStatBaseGet, 0x10074CF0);
	rebase(ObjStatBaseDispatch, 0x1004E810);

	rebase(ObjGetBABAfterLevelling, 0x100749B0);
	rebase(XPReqForLevel, 0x100802E0);
	rebase(ObjXPGainProcess, 0x100B5480);

	rebase(sub_10152280, 0x10152280);
	rebase(CraftMagicArmsAndArmorSthg, 0x10150B20);


	rebase(_ItemWorthFromEnhancements, 0x101509C0);
	rebase(ItemCreationPrereqSthg_sub_101525B0, 0x101525B0);

	rebase(CombatAdvanceTurn, 0x100634E0);
	
	rebase(sub_100664B0, 0x100664B0);

}

#pragma endregion


#pragma region TempleFuncs replacements
int32_t _diceRoll(uint32_t dieNum, uint32_t dieType, int32_t dieBonus)
{
	return templeFuncs.diceRoll(dieNum, dieType, dieBonus);
}
#pragma endregion