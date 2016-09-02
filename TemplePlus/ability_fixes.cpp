#include "stdafx.h"
#include "util/fixes.h"
#include "dispatcher.h"
#include "combat.h"
#include "poison.h"
#include "condition.h"
#include "damage.h"
#include "d20.h"
#include "float_line.h"
#include "history.h"

// Ability Condition Fixes (for buggy abilities, including monster abilities)
class AbilityConditionFixes : public TempleFix {
public:
#define ABFIX(fname) static int fname ## (DispatcherCallbackArgs args);

	static int PoisonedOnBeginRound(DispatcherCallbackArgs args);;
	

	void apply() override {

		replaceFunction(0x100EA040, PoisonedOnBeginRound);
		static void (__cdecl*orgCompanionRunoff)(SubDispNode*, objHndl , objHndl ) = replaceFunction<void(__cdecl)(SubDispNode*, objHndl, objHndl)>(0x100FC3D0, [](SubDispNode* sdn, objHndl owner, objHndl handle){
			orgCompanionRunoff(sdn, owner, handle);
			conds.CondNodeSetArg(sdn->condNode, 2, 0);
		});
	
	}
} abilityConditionFixes;

int AbilityConditionFixes::PoisonedOnBeginRound(DispatcherCallbackArgs args){
	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);

	// decrement duration
	auto dur = args.GetCondArg(1);
	auto durRem = dur - (int)dispIo->data1;
	if (durRem >= 0){
		args.SetCondArg(1, durRem);
		return 0;
	}

	auto poisonId = args.GetCondArg(0);
	auto &poisonSpecs = temple::GetRef<PoisonSpec[36]>(0x1028C080);
	auto pspec = poisonSpecs[poisonId];

	// make saving throw
	auto dc = pspec.dc;
	if (dc <= 0){
		dc = args.GetCondArg(2);
	}
	if (dc < 0 || dc > 100) // failsafe
		dc = 15;

	if (pspec.delayedEffect == -10 || pspec.delayedEffect == -9) {
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}

	// success - remove condition
	if (damage.SavingThrow(args.objHndCaller, objHndl::null, dc, SavingThrowType::Fortitude, 8)){
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}

	// failure

	// check delay poison
	if (d20Sys.d20QueryWithData(args.objHndCaller, DK_QUE_Critter_Has_Condition, conds.GetByName("sp-Delay Poison"), 0)){
		floatSys.FloatSpellLine(args.objHndCaller, 20033, FloatLineColor::White);
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}
		
	floatSys.FloatCombatLine(args.objHndCaller, 56);
	histSys.CreateRollHistoryLineFromMesfile(21, args.objHndCaller, objHndl::null); // "X takes poison damage!"
	floatSys.FloatCombatLine(args.objHndCaller, 96);

	auto rollRes = Dice(pspec.delayedDice.count, pspec.delayedDice.sides, pspec.delayedDice.sides).Roll();
	conds.AddTo(args.objHndCaller, "Temp_Ability_Loss", {pspec.delayedEffect + (pspec.delayedEffect < 0 ? 6:0), rollRes});

	if (pspec.delayedSecondEffect != -10){
		rollRes = Dice(pspec.delayedSecDice.count, pspec.delayedSecDice.sides, pspec.delayedSecDice.sides).Roll();
		conds.AddTo(args.objHndCaller, "Temp_Ability_Loss", { pspec.delayedSecondEffect + (pspec.delayedSecondEffect < 0 ? 6 : 0), rollRes });

	}

	conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
	return 0;
}
