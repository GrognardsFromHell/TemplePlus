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
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/particlesystems.h"
#include "action_sequence.h"

// Ability Condition Fixes (for buggy abilities, including monster abilities)
class AbilityConditionFixes : public TempleFix {
public:
#define ABFIX(fname) static int fname ## (DispatcherCallbackArgs args);
#define HOOK_ORG(fname) static int (__cdecl* org ##fname)(DispatcherCallbackArgs) = replaceFunction<int(__cdecl)(DispatcherCallbackArgs)>
	static int PoisonedOnBeginRound(DispatcherCallbackArgs args);;
	static int MonsterSplittingHpChange(DispatcherCallbackArgs args);
	static int MonsterOozeSplittingOnDamage(DispatcherCallbackArgs args);
	static int MonsterSubtypeFire(DispatcherCallbackArgs args);
	static int GrappledMoveSpeed(DispatcherCallbackArgs args);
	static int DiplomacySkillSynergy(DispatcherCallbackArgs args);

	static int BootsOfSpeedNewday(DispatcherCallbackArgs args);
	static int BootsOfSpeedBeginRound(DispatcherCallbackArgs args);

	static int CombatExpertiseAcBonus(DispatcherCallbackArgs args);
	static int TacticalAbusePrevention(DispatcherCallbackArgs args); // Combat Expertise / Fight Defensively
	

	void apply() override {

		replaceFunction(0x100F7ED0, TacticalAbusePrevention);
		replaceFunction(0x100F7E70, CombatExpertiseAcBonus);

		replaceFunction(0x100CB890, GrappledMoveSpeed); // fixed Grappled when the frog is dead
		{
			SubDispDefNew sdd;
			sdd.dispKey = DK_QUE_Critter_Is_Grappling;
			sdd.dispType = dispTypeD20Query;
			sdd.dispCallback = [](DispatcherCallbackArgs args){
				auto spellId = args.GetCondArg(0);
				SpellPacketBody spPkt(spellId);
				if (!spPkt.caster
					|| objSystem->IsValidHandle(spPkt.caster) && critterSys.IsDeadOrUnconscious(spPkt.caster)){
					conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
					return 0;
				}
					
				GET_DISPIO(dispIOTypeQuery, DispIoD20Query);
				dispIo->return_val = 1;
				return 0;
			};

			write(0x102E6158, &sdd, sizeof(sdd));
			write(0x102E616C, &sdd, sizeof(sdd));
			write(0x102E6180, &sdd, sizeof(sdd));
			write(0x102E61A8, &sdd, sizeof(sdd));
			write(0x102E15C0 + offsetof(CondStruct, subDispDefs) + (1) * sizeof(SubDispDefNew), &sdd, sizeof(sdd));

		}

		replaceFunction(0x10101EB0, BootsOfSpeedNewday);
		replaceFunction(0x101020A0, BootsOfSpeedBeginRound); // fixes the bug that fucked things up

		replaceFunction(0x100FDE00, MonsterSubtypeFire); // fixes 2x damage factor for vulnerability to cold (should be 1.5)

		replaceFunction(0x100EA040, PoisonedOnBeginRound);

		replaceFunction(0x100F7550, MonsterSplittingHpChange);
		replaceFunction(0x100F7490, MonsterOozeSplittingOnDamage);

		replaceFunction(0x100EEE70, DiplomacySkillSynergy); // fixes skill synergy bonuses to diplomacy not stacking

		// fixes animal companion runoff crash (it didn't null the second part of the obj handle stored in args[1,2]
		static void (__cdecl*orgCompanionRunoff)(SubDispNode*, objHndl , objHndl ) = replaceFunction<void(__cdecl)(SubDispNode*, objHndl, objHndl)>(0x100FC3D0, [](SubDispNode* sdn, objHndl owner, objHndl handle){
			orgCompanionRunoff(sdn, owner, handle);
			conds.CondNodeSetArg(sdn->condNode, 2, 0);
		});

		// fixes Opportunist getting an AOO for your own attack...
		replaceFunction<int(__cdecl)(DispatcherCallbackArgs)>(0x100FADF0, [](DispatcherCallbackArgs args){
			auto numAvail = args.GetCondArg(0);
			if (!numAvail)
				return 0;
			
			GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
			auto d20a = (D20Actn*)dispIo->data1;
			if (d20a->d20APerformer == args.objHndCaller) // fixes vanilla bug where you got an AOO for your making your own attack
				return 0;
			
			auto tgt = d20a->d20ATarget;
			if (!tgt) // fixed missing check on target (e.g. this would fire on move actions)
				return 0;

			if (!d20Sys.d20QueryWithData(args.objHndCaller, DK_QUE_AOOPossible, tgt))
				return 0;
			if (!d20Sys.d20QueryWithData(args.objHndCaller, DK_QUE_AOOWillTake, tgt))
				return 0;
			if (combatSys.AffiliationSame(args.objHndCaller, tgt))
				return 0;
			if (!combatSys.CanMeleeTarget(args.objHndCaller, tgt))
				return 0;

			if (!d20a->IsMeleeHit())
				return 0;
			args.SetCondArg(0, numAvail - 1);
			actSeqSys.DoAoo(args.objHndCaller, tgt);
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

int AbilityConditionFixes::MonsterSplittingHpChange(DispatcherCallbackArgs args){
	auto protoId = objSystem->GetProtoId(args.objHndCaller);
	auto obj = objSystem->GetObject(args.objHndCaller);
	auto loc = obj->GetLocationFull();
	auto hpCur = objects.StatLevelGet(args.objHndCaller, stat_hp_current);
	auto baseHp = obj->GetInt32(obj_f_hp_pts);
	auto hpDam = obj->GetInt32(obj_f_hp_damage);

	auto hpMod = (hpCur + hpDam) - baseHp;

	if (hpCur <= 10)
		return 0;

	obj->SetInt32(obj_f_hp_pts, baseHp / 2);
	obj->SetInt32(obj_f_hp_damage, (hpDam + hpMod) / 2);


	auto protoHandle = objSystem->GetProtoHandle(protoId);
	auto newMonster = objSystem->CreateObject(protoHandle, loc.location);
	auto newMonObj = objSystem->GetObject(newMonster);
	newMonObj->SetInt32(obj_f_hp_pts, baseHp / 2);
	newMonObj->SetInt32(obj_f_hp_damage, (hpDam + hpMod) / 2);
	newMonObj->SetInt32(obj_f_critter_flags, newMonObj->GetInt32(obj_f_critter_flags) | OCF_EXPERIENCE_AWARDED);

	gameSystems->GetParticleSys().CreateAtObj("hit-Acid-medium", args.objHndCaller);
	conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);

	return 0;
}

int AbilityConditionFixes::MonsterOozeSplittingOnDamage(DispatcherCallbackArgs args){

	auto curHp = objects.StatLevelGet(args.objHndCaller, stat_hp_current);
	if (curHp <= 10)
		return 0;

	GET_DISPIO(dispIOTypeDamage, DispIoDamage);
	auto isSplitting = false;
	auto protoId = objSystem->GetProtoId(args.objHndCaller);
	if (protoId == 14142 && dispIo->damage.GetOverallDamageByType(DamageType::Electricity) > 0){
		isSplitting = true;
		dispIo->damage.AddModFactor(0.0f, DamageType::Electricity, 132);
	}

	if (dispIo->damage.GetOverallDamageByType(DamageType::Slashing) > 0) {
		isSplitting = true;
		dispIo->damage.AddModFactor(0.0f, DamageType::Slashing, 132);
	}

	if (dispIo->damage.GetOverallDamageByType(DamageType::Piercing) > 0) {
		isSplitting = true;
		dispIo->damage.AddModFactor(0.0f, DamageType::Piercing, 132);
	}

	if (dispIo->damage.GetOverallDamageByType(DamageType::PiercingAndSlashing) > 0) {
		isSplitting = true;
		dispIo->damage.AddModFactor(0.0f, DamageType::PiercingAndSlashing, 132);
	}


	if (isSplitting){
		conds.AddTo(args.objHndCaller, "Monster Splitting", {});
	}

	return 0;
}

int AbilityConditionFixes::MonsterSubtypeFire(DispatcherCallbackArgs args){

	GET_DISPIO(dispIOTypeDamage, DispIoDamage);
	auto immuneDamType = (DamageType)args.GetData1();
	auto vulnerDamType = (DamageType)args.GetData2();

	if (!(dispIo->attackPacket.flags & D20CAF_SAVE_SUCCESSFUL))
		dispIo->damage.AddModFactor(1.5, vulnerDamType, 136); // Vulnerable
	dispIo->damage.AddModFactor(0.0, immuneDamType, 104); // Invulnerable
	return 0;
}

int AbilityConditionFixes::GrappledMoveSpeed(DispatcherCallbackArgs args){
	auto spellId = args.GetCondArg(0);
	SpellPacketBody spPkt(spellId);
	if (!spPkt.caster
		|| critterSys.IsDeadOrUnconscious(spPkt.caster))
		return 0;
	
	GET_DISPIO(dispIOTypeMoveSpeed, DispIoMoveSpeed);
	dispIo->bonlist->SetOverallCap(1, args.GetData1(), 0, args.GetData2());
	dispIo->bonlist->SetOverallCap(2, args.GetData1(), 0, args.GetData2());

	return 0;
}

int AbilityConditionFixes::DiplomacySkillSynergy(DispatcherCallbackArgs args){
	GET_DISPIO(dispIoTypeObjBonus, DispIoObjBonus);

	if (critterSys.SkillBaseGet(args.objHndCaller, SkillEnum::skill_bluff) >= 5)
		dispIo->bonOut->AddBonus(2, 0, 140);
	if (critterSys.SkillBaseGet(args.objHndCaller, SkillEnum::skill_sense_motive) >= 5)
		dispIo->bonOut->AddBonus(2, 0, 302);
	return 0;
}

int AbilityConditionFixes::BootsOfSpeedNewday(DispatcherCallbackArgs args){
	auto arg1 = args.GetCondArg(1);
	args.SetCondArg(0, arg1);
	return 0;
}

int AbilityConditionFixes::BootsOfSpeedBeginRound(DispatcherCallbackArgs args){

	auto roundsRem = args.GetCondArg(0);
	auto isOn = args.GetCondArg(3);
	if (!isOn)
		return 0;
	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
	int roundsNew = roundsRem - (int)dispIo->data1;
	if (roundsNew >= 0){
		args.SetCondArg(0, roundsNew);
		return 1;
	}
	// conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode); // this was a bug in vanilla! should just reset the isOn arg, as below
	args.SetCondArg(0, 0);
	args.SetCondArg(3, 0);
	
	auto partsysId = args.GetCondArg(4);
	gameSystems->GetParticleSys().End(partsysId);
	return 0;
}

int AbilityConditionFixes::CombatExpertiseAcBonus(DispatcherCallbackArgs args){

	auto expertiseAmt = args.GetCondArg(0);
	if (!expertiseAmt){
		return 0;
	}

	auto attackMade = args.GetCondArg(1);
	if (!attackMade){
		return 0;
	}

	GET_DISPIO(dispIOTypeAttackBonus, DispIoAttackBonus);
	dispIo->bonlist.AddBonusFromFeat(expertiseAmt, 8, 114, FEAT_COMBAT_EXPERTISE);

	return 0;
}

int AbilityConditionFixes::TacticalAbusePrevention(DispatcherCallbackArgs args)
{
	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
	auto damPkt = (DispIoDamage*)dispIo->data1;
	if (damPkt->attackPacket.flags & D20CAF_RANGED){
		return 0;
	}
	args.SetCondArg(1, 1);
	return 0;
}
