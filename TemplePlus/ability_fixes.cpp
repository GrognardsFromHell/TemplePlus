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
	static int MonsterSplittingHpChange(DispatcherCallbackArgs args);
	static int MonsterOozeSplittingOnDamage(DispatcherCallbackArgs args);
	static int MonsterSubtypeFire(DispatcherCallbackArgs args);
	static int GrappledMoveSpeed(DispatcherCallbackArgs args);
	static int DiplomacySkillSynergy(DispatcherCallbackArgs args);

	static int BootsOfSpeedNewday(DispatcherCallbackArgs args);
	static int BootsOfSpeedBeginRound(DispatcherCallbackArgs args);

	static int CombatExpertiseAcBonus(DispatcherCallbackArgs args);
	static int TacticalAbusePrevention(DispatcherCallbackArgs args); // Combat Expertise / Fight Defensively
	static int __cdecl FinesseToHit(DispatcherCallbackArgs args);

	static int __cdecl CombatReflexesAooReset(DispatcherCallbackArgs args);
	static int __cdecl AOOWillTake(DispatcherCallbackArgs args);
	static int __cdecl AOOPerformed(DispatcherCallbackArgs args);
	
	static int SpellFocusDcMod(DispatcherCallbackArgs args);

	static int PurityOfBodyPreventDisease(DispatcherCallbackArgs args); // Fixes Monk's Purity of Body; currently it prevents Magical Disease (Contagion) rather than normal disease (as by Monster Melee Disease e.g. dire rats)


	void apply() override {

		//replaceFunction(0x100FC050, SpellFocusDcMod);

		replaceFunction(0x100F8AF0, CombatReflexesAooReset);
		replaceFunction(0x100F8A10, AOOWillTake);
		replaceFunction(0x100F8A70, AOOPerformed);

		replaceFunction(0x100F7ED0, TacticalAbusePrevention);
		replaceFunction(0x100F7E70, CombatExpertiseAcBonus);

		replaceFunction(0x100CB890, GrappledMoveSpeed); // fixed Grappled when the frog is dead
		{ // Remove the Grappled condition when the frog (caster) is dead on the following queries
			SubDispDefNew sdd;
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
			sdd.dispKey = DK_QUE_SneakAttack;
			write(0x102E6158, &sdd, sizeof(sdd));
			sdd.dispKey = DK_QUE_Helpless;
			write(0x102E616C, &sdd, sizeof(sdd));
			sdd.dispKey = DK_QUE_CannotCast;
			write(0x102E6180, &sdd, sizeof(sdd));
			sdd.dispKey = DK_QUE_Critter_Is_Grappling;
			write(0x102E61A8, &sdd, sizeof(sdd));
			write(0x102E15C0 + offsetof(CondStruct, subDispDefs) + (1) * sizeof(SubDispDefNew), &sdd, sizeof(sdd));

		}

		replaceFunction(0x10101EB0, BootsOfSpeedNewday);
		replaceFunction(0x101020A0, BootsOfSpeedBeginRound); // fixes the bug that fucked things up

		replaceFunction(0x100FDE00, MonsterSubtypeFire); // fixes 2x damage factor for vulnerability to cold (should be 1.5)

		replaceFunction(0x100F7550, MonsterSplittingHpChange);
		replaceFunction(0x100F7490, MonsterOozeSplittingOnDamage);

		replaceFunction(0x100EEE70, DiplomacySkillSynergy); // fixes skill synergy bonuses to diplomacy not stacking

		replaceFunction(0x100FAFB0, PurityOfBodyPreventDisease);

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

		// Fix for negative str mod and weapon sizing.
		replaceFunction(0x100F80C0, FinesseToHit);
	}
} abilityConditionFixes;


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

	// inherit factions (e.g. moathouse respawn Blood Amniotes need this since the factions are defined at the MOB level)
	{
		auto factionArr = obj->GetInt32Array(obj_f_npc_faction);
		int numFactions = factionArr.GetSize();

		for (auto i = 0; i < numFactions; i++) {
			auto newFac = factionArr[i];
			if (!newFac)
				continue;
			if (!objects.factions.FactionHas(newMonster, newFac)) {
				objects.factions.FactionAdd(newMonster, newFac);
			}
		}
	}
	

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

	/*auto attackMade = args.GetCondArg(1);
	if (!attackMade){
		return 0;
	}*/
	auto attackMadeStoredValue = args.GetCondArg(1);
	if (!attackMadeStoredValue) {
		return 0;
	}
	expertiseAmt = attackMadeStoredValue;

	GET_DISPIO(dispIOTypeAttackBonus, DispIoAttackBonus);
	dispIo->bonlist.AddBonusFromFeat(expertiseAmt, 8, 114, FEAT_COMBAT_EXPERTISE);

	return 0;
}

int __cdecl  AbilityConditionFixes::CombatReflexesAooReset(DispatcherCallbackArgs args)
{
	int numAoosRem = 1;

	//Fixes atari bug 571 where only dex bonus AOOs could be taken for combat reflexes instead of dex bonus + 1
	if (feats.HasFeatCount(args.objHndCaller, FEAT_COMBAT_REFLEXES) > 0) {
		const auto dexScore = objects.StatLevelGet(args.objHndCaller, Stat::stat_dexterity);
		auto extraAoos = objects.GetModFromStatLevel(dexScore);
		extraAoos = std::max(extraAoos, 0);

		// Enable hydra combat reflexes special case.
		auto heads = d20Sys.D20QueryPython(args.objHndCaller, "Hydra Heads");
		if (heads > 0) extraAoos = heads;
		numAoosRem += extraAoos;
	}

	args.SetCondArg(0, numAoosRem);
	args.SetCondArg(1, 0);
	args.SetCondArg(2, 0);
	return 0;
}

int __cdecl  AbilityConditionFixes::AOOWillTake(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);

	const auto numAoosRem = args.GetCondArg(0);
	if (numAoosRem > 0) {
		const bool allowMultipleAOOs = d20Sys.D20QueryPython(args.objHndCaller, "Allow Multiple AOOs") > 0;
		if (!allowMultipleAOOs) {
			if (args.GetCondArg(1) != dispIo->data1 || args.GetCondArg(2) != dispIo->data2) {
				dispIo->return_val = 1;
			}
		}
		else {
			dispIo->return_val = 1;
		}
	}
	return 0;
}

int __cdecl  AbilityConditionFixes::AOOPerformed(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType6(args.dispIO);
	const auto numAoosRem = args.GetCondArg(0);
	args.SetCondArg(0, numAoosRem - 1);

	const bool allowMultipleAOOs = d20Sys.D20QueryPython(args.objHndCaller, "Allow Multiple AOOs") > 0;
	if (!allowMultipleAOOs) {
		args.SetCondArg(1, dispIo->data1);
		args.SetCondArg(2, dispIo->data2);
	}

	return 0;
}

int AbilityConditionFixes::TacticalAbusePrevention(DispatcherCallbackArgs args)
{
	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
	auto damPkt = (DispIoDamage*)dispIo->data1;
	if (damPkt->attackPacket.flags & D20CAF_RANGED){
		return 0;
	}

	// vanilla:
	//args.SetCondArg(1, 1);
	
	// fixes issue where users could manipulate this at the end of turn:
	// i.e. made an attack (set arg1 to 1), and then manipulate the value of arg0 via radial
	// now arg1 will store the max value used this round
	auto currentSetValue = args.GetCondArg(0);
	auto currentArg1Value = args.GetCondArg(1);
	int newValue = max(currentArg1Value, currentSetValue); 
	
	args.SetCondArg(1, newValue);
	return 0;
}

// Port of 0x100F80C0. Fixes:
//   1. Zeroes out negative str modifier when using weapon finesse, since it
//      would otherwise stack.
//   2. Regard sizing when Enlarge/Reduce Person is active, since it's
//      supposed to do the same to the weapon.
//   3. Check if shield armor check penalty would reduce modifier to below
//      the strength modifier, in which case it's disadvantageous to use
//      finesse.
int AbilityConditionFixes::FinesseToHit(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType5(args.dispIO);
	auto featEnum = static_cast<feat_enums>(args.GetCondArg(0));
	auto weapon = dispIo->attackPacket.GetWeaponUsed();
	auto critter = args.objHndCaller;

	if (!inventory.IsFinesse(critter, weapon)) return 0;

	auto strMod = objects.StatLevelGet(critter, stat_str_mod);
	auto dexMod = objects.StatLevelGet(critter, stat_dex_mod);

	// Armor check penalty from shields applies to finesse.
	auto shield = inventory.ItemWornAt(critter, EquipSlot::Shield);
	auto penalty = GetArmorCheckPenalty(shield);

	// Compare penalized score against strength, just in case the
	// combination is worse.
	if (dexMod + penalty <= strMod) return 0;

	auto featName = feats.GetFeatName(featEnum);

	dispIo->bonlist.AddCapWithDescr(2, 0, 114, featName);
	// If the strength modifier is a penalty, it will not be capped, so
	// modify it back to 0.
	if (strMod < 0) dispIo->bonlist.ModifyBonus(-strMod, 2, 103);

	if (dexMod != 0)
		dispIo->bonlist.AddBonusWithDesc(dexMod, 3, 104, featName);
	if (penalty != 0) {
		auto desc = description.getDisplayName(shield, critter);
		dispIo->bonlist.AddBonusWithDesc(penalty, 0, 125, desc);
	}

	return 0;
}

int AbilityConditionFixes::SpellFocusDcMod(DispatcherCallbackArgs args)
{
	GET_DISPIO(dispIoTypeBonusListAndSpellEntry, DispIOBonusListAndSpellEntry);
	auto feat = (feat_enums)args.GetCondArg(0);
	auto spellSchool = dispIo->spellEntry->spellSchoolEnum - 1;
	if ( feat - FEAT_SPELL_FOCUS_ABJURATION == spellSchool || feat - FEAT_GREATER_SPELL_FOCUS_ABJURATION == spellSchool){
		dispIo->bonList->AddBonusFromFeat(1, 0, 114, feat);
	}
	return 0;
}

int AbilityConditionFixes::PurityOfBodyPreventDisease(DispatcherCallbackArgs args)
{
	GET_DISPIO(dispIoTypeCondStruct, DispIoCondStruct);
	auto incomingCond = dispIo->condStruct;
	auto refCond = (CondStruct*)args.GetData1();
	if (!refCond)
		return 0;
	refCond = conds.GetByName(refCond->condName); // retrieve condition by name
	if (!refCond)
		return 0;
	if (refCond != incomingCond) {
		return 0;
	}
	
	// Incubating_Disease args are: 
	// arg[0] : 0 - natural, 1 - from contagion spell
	// arg[1] : disease type ID (e.g. Filth Fever is 4)
	// arg[2] : Incubation duration in days (randomized by disease type as 1dN)
	constexpr int DISEASE_ORIGIN_NATURAL = 0;
	auto diseaseOriginId = dispIo->arg1;
	auto isNatural = diseaseOriginId == DISEASE_ORIGIN_NATURAL;
	if (!isNatural) {
		return 0;
	}
	dispIo->outputFlag = 0; // interdict natural disease
	return 0;
}
