#include "stdafx.h"
#include "util/fixes.h"
#include "dispatcher.h"
#include "condition.h"

#include "gamesystems/objects/objsystem.h"
#include "ui/ui_systems.h"
#include "ui/ui_legacysystems.h"

#define CONDFIX(fname) static int fname ## (DispatcherCallbackArgs args);
#define HOOK_ORG(fname) static int (__cdecl* org ##fname)(DispatcherCallbackArgs) = replaceFunction<int(__cdecl)(DispatcherCallbackArgs)>

class GeneralConditionFixes : public TempleFix {
public:

	static int WeaponKeenQuery(DispatcherCallbackArgs args);

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

			write(0x102E7400, &sdd, sizeof(sdd));
		}

		replaceFunction(0x100FF670, WeaponKeenQuery);

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
