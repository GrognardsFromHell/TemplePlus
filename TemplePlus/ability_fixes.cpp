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
#define HOOK_ORG(fname) static int (__cdecl* org ##fname)(DispatcherCallbackArgs) = replaceFunction<int(__cdecl)(DispatcherCallbackArgs)>
	static int PoisonedOnBeginRound(DispatcherCallbackArgs args);;
	

	void apply() override {

		replaceFunction(0x100EA040, PoisonedOnBeginRound);

		// fixes animal companion runoff crash (it didn't null the second part of the obj handle stored in args[1,2]
		static void (__cdecl*orgCompanionRunoff)(SubDispNode*, objHndl , objHndl ) = replaceFunction<void(__cdecl)(SubDispNode*, objHndl, objHndl)>(0x100FC3D0, [](SubDispNode* sdn, objHndl owner, objHndl handle){
			orgCompanionRunoff(sdn, owner, handle);
			conds.CondNodeSetArg(sdn->condNode, 2, 0);
		});

		// fixes Opportunist getting an AOO for your own attack...
		HOOK_ORG(OpportuninstBroadcast)(0x100FADF0, [](DispatcherCallbackArgs args)->int
		{
			auto numAvail = args.GetCondArg(0);
			if (numAvail){
				GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
				auto d20a = (D20Actn*)dispIo->data1;
				if (d20a->d20APerformer != args.objHndCaller)
					return orgOpportuninstBroadcast(args);
			}
			return 0;
		});
	

		// Banshee Charisma Drain (fixes uninitialized values used when saving throw is successful)
		HOOK_ORG(BansheeCharismaDrain)(0x100F67D0, [](DispatcherCallbackArgs args)->int {
			GET_DISPIO(dispIOTypeDamage, DispIoDamage);
			
			if (dispIo->attackPacket.dispKey < ATTACK_CODE_NATURAL_ATTACK + 1)
				return 0;

			auto attacker = dispIo->attackPacket.attacker;
			auto tgt = dispIo->attackPacket.victim;
			//auto saveRes = damage.SavingThrow(tgt, attacker, 26, SavingThrowType::Fortitude, 0); // looks like by the rules there shouldn't even be a save? (Mon. Man. Ghost entry)

			auto curHp = objects.StatLevelGet(attacker, stat_hp_current);
			auto maxHp = objects.StatLevelGet(attacker, stat_hp_max);
			int tempHpGain = 5;
			auto chaDrainDice = 1;
			if (dispIo->attackPacket.flags & D20CAF_CRITICAL){
				tempHpGain = 10;
				chaDrainDice = 2;
			}
			auto chaDrainAmt = Dice(chaDrainDice, 4, 0).Roll();

			auto hpDam = maxHp - curHp;
			// add Temporary HPs 
			if (tempHpGain >= hpDam){
				conds.AddTo(attacker, "Temporary_Hit_Points", {0, 14400, tempHpGain - hpDam});
				tempHpGain -= tempHpGain - hpDam;
			}
			// heal normal damage if applicable
			if (tempHpGain > 0 ){
				damage.Heal(attacker, tgt, Dice(0, 0, tempHpGain), D20A_NONE);
			}
			conds.AddTo(tgt, "Temp_Ability_Loss", {stat_charisma, chaDrainAmt});
		
			return 0;
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
	if (damage.SavingThrow(args.objHndCaller, objHndl::null, dc, SavingThrowType::Fortitude, D20STF_POISON)){
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
