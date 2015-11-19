
#include "stdafx.h"
#include <temple/dll.h>
#include "temple_functions.h"
#include "tig/tig_mouse.h"
#include "tig/tig_msg.h"

//#include "spell.h"
#include "common.h"
#include "util/config.h"
#include "util/fixes.h"

TempleFuncs templeFuncs;

class TempleFuncReplacements : public TempleFix
{
	public: const char* name() override { return "Temple General Function" "Function Replacements";} void apply() override 
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
	rebase(encodeTriplet,0x10038C50); 
	rebase(UpdatePartyUI, 0x10134CB0);
	rebase(PartyMoney, 0x1002B750);
	rebase(DebitPartyMoney, 0x1002C020);



# pragma region Obj Get/Set General

	rebase(Obj_Get_Field_32bit, 0x1009E1D0);
	rebase(Obj_Get_Field_64bit, 0x1009E2E0);
	rebase(Obj_Get_Field_Float, 0x1009E260);
	rebase(Obj_Get_Field_ObjHnd__fastout, 0x1009E360);


	rebase(Obj_Get_IdxField_NumItems, 0x1009E7E0);

	rebase(Obj_Get_IdxField_32bit, 0x1009E5C0);
	rebase(Obj_Get_IdxField_64bit, 0x1009E640);
	rebase(Obj_Get_IdxField_ObjHnd, 0x1009E6D0);
	rebase(Obj_Get_ArrayElem_Generic, 0x1009E770);

	rebase(Obj_Set_Field_32bit, 0x100A0190);
	rebase(Obj_Set_Field_64bit, 0x100A0200);
	rebase(Obj_Set_Field_ObjHnd, 0x100A0280);
	rebase(Obj_Set_IdxField_byValue, 0x100A1310);
	rebase(Obj_Set_IdxField_byPtr, 0x100A1540);
	rebase(Obj_Set_IdxField_ObjHnd, 0x100A14A0);

#pragma endregion

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

	rebase(TurnProcessing, 0x100634E0);
	
	rebase(sub_100664B0, 0x100664B0);

}

#pragma endregion


#pragma region TempleFuncs replacements
int32_t _diceRoll(uint32_t dieNum, uint32_t dieType, int32_t dieBonus)
{
	return templeFuncs.diceRoll(dieNum, dieType, dieBonus);
}
#pragma endregion