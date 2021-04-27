#include "stdafx.h"
#include "util/fixes.h"
#include "dispatcher.h"
#include "condition.h"

#include "gamesystems/objects/objsystem.h"
#include "ui/ui_systems.h"
#include "ui/ui_legacysystems.h"
#include "ui/ui_char.h"
#include <critter.h>

#define CONDFIX(fname) static int fname ## (DispatcherCallbackArgs args);
#define HOOK_ORG(fname) static int (__cdecl* org ##fname)(DispatcherCallbackArgs) = replaceFunction<int(__cdecl)(DispatcherCallbackArgs)>

class GeneralConditionFixes : public TempleFix {
public:

	static int WeaponKeenQuery(DispatcherCallbackArgs args);
	static int TempNegativeLevelOnAdd(DispatcherCallbackArgs args); // fixes critters dying due to neg HP

	void apply() override {

		{ // Fix for duplicate Blackguard tooltip when fallen paladin
			SubDispDefNew sdd;
			sdd.dispKey = DK_NONE;
			sdd.dispType = dispTypeEffectTooltip;
			sdd.dispCallback = [](DispatcherCallbackArgs args) {
				
				GET_DISPIO(dispIOTypeEffectTooltip, DispIoEffectTooltip);

				if (objects.StatLevelGet(args.objHndCaller, stat_level_blackguard))
					return 0;
				dispIo->Append(args.GetData1(), -1, nullptr);
				return 0;
			};
			sdd.data1.sVal = 0xAF;
			write(0x102E7400, &sdd, sizeof(sdd));
		}
		
		replaceFunction(0x100FF670, WeaponKeenQuery);
		replaceFunction(0x100EF540, TempNegativeLevelOnAdd); // fixes NPCs with no class levels instantly dying due to temp negative level
	}
} genCondFixes;

int GeneralConditionFixes::WeaponKeenQuery(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeQuery, DispIoD20Query);
	dispIo->return_val = 2; // default
	
	auto itemHndl = uiSystems->GetChar().GetTooltipItem();
	if (!itemHndl || !objSystem->IsValidHandle(itemHndl))
		return 0;

	// meh, it doesn't have a handle
	/*auto invIdx = args.GetCondArg(2);
	auto itemHndl = inventory.GetItemAtInvIdx(args.objHndCaller, invIdx);
	if (!itemHndl || !objSystem->IsValidHandle(itemHndl))
		return 0;
		*/
	auto item = objSystem->GetObject(itemHndl);
	
	auto critRange = item->GetInt32(obj_f_weapon_crit_range);
	dispIo->return_val = critRange;

	return 0;
}

int GeneralConditionFixes::TempNegativeLevelOnAdd(DispatcherCallbackArgs args)
{
	auto highestLvl = 0;
	auto highestClass = 0;

	if (args.GetData1() == 273) { // aligned weapon enchantment (Holy/Unholy/Axiomatic/Chaotic)
		auto alignmentMask = args.GetData2();
		auto critterAlignment = objects.StatLevelGetBase(args.objHndCaller, Stat::stat_alignment);
		if ((alignmentMask & critterAlignment) != alignmentMask)
			return 0;
	}

	critterSys.CritterHpChanged(args.objHndCaller, objHndl::null, -5);

	// fix for negative levels killing critters without class levels - 
	// instead of class levels, get the hit dice num (taking into account previous negative levels etc)
	//auto lvl = dispatch.Dispatch61GetLevel(args.objHndCaller, Stat::stat_level, nullptr, objHndl::null);
	auto hd = objects.GetHitDiceNum(args.objHndCaller, /*getBase=*/ false); 
	if (hd  <= 0) {
		critterSys.Kill(args.objHndCaller); // buuug
	}

	// extend this to new classes as well
	for (auto classId : d20ClassSys.classEnums) {
		auto classLvl = dispatch.Dispatch61GetLevel(args.objHndCaller, (Stat)classId);
		if (classLvl > highestLvl) {
			highestLvl = classLvl;
			highestClass = classId;
		}
	}

	args.SetCondArg(0, highestClass);
	critterSys.CritterHpChanged(args.objHndCaller, objHndl::null, 0); // hmmm?

	return 0;
}
