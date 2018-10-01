
#include "stdafx.h"

#include <graphics/math.h>
#include <infrastructure/meshes.h>

#include "common.h"
#include "dispatcher.h"
#include "condition.h"
#include "temple_functions.h"
#include "obj.h"
#include "bonus.h"
#include "radialmenu.h"
#include "combat.h"
#include "critter.h"
#include "location.h"
#include "damage.h"
#include "float_line.h"
#include "ui/ui_dialog.h"
#include "party.h"
#include "weapon.h"
#include "action_sequence.h"
#include "ui/ui_item_creation.h"
#include "util/fixes.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include <infrastructure/elfhash.h>
#include "particles.h"
#include "gamesystems/particlesystems.h"
#include "animgoals/anim.h"
#include "python/python_integration_spells.h"
#include "history.h"
#include "gamesystems/objects/objevent.h"
#include "ui/ui_party.h"
#include "sound.h"
#include "d20_class.h"
#include "gamesystems/d20/d20stats.h"
#include "d20_race.h"

#define CB int(__cdecl)(DispatcherCallbackArgs)
using DispCB = int(__cdecl )(DispatcherCallbackArgs);

ConditionSystem conds;
CondStructNew conditionDisableAoO;
CondStructNew conditionGreaterTwoWeaponFighting;
CondStructNew condGreaterTWFRanger;
CondStructNew condDivineMight;
CondStructNew condDivineMightBonus;
CondStructNew condRecklessOffense;
CondStructNew condKnockDown;
CondStructNew condDeadlyPrecision;
CondStructNew condPersistentSpell;

CondStructNew condGreaterWeaponSpecialization;

CondStructNew ConditionSystem::mCondDisarm;
CondStructNew ConditionSystem::mCondDisarmed;


//items
CondStructNew ConditionSystem::mCondNecklaceOfAdaptation;

// monsters
CondStructNew condRend;

CondStructNew ConditionSystem::mCondCaptivatingSong;
CondStructNew ConditionSystem::mCondCaptivated;

CondStructNew ConditionSystem::mCondHezrouStench;
CondStructNew ConditionSystem::mCondHezrouStenchHit;

int ConditionPreventWithArg(DispatcherCallbackArgs args);
int ConditionOverrideBy(DispatcherCallbackArgs args);


struct ConditionSystemAddresses : temple::AddressTable
{
	void(__cdecl*SetPermanentModArgsFromDataFields)(Dispatcher* dispatcher, CondStruct* condStruct, int* condArgs);
	int(__cdecl*RemoveSpellCondition)(DispatcherCallbackArgs args); 
	int(__cdecl*RemoveSpellMod)(DispatcherCallbackArgs args);
	ConditionSystemAddresses()
	{
		rebase(SetPermanentModArgsFromDataFields, 0x100E1B90);
		rebase(RemoveSpellCondition, 0x100D7620);
		rebase(RemoveSpellMod, 0x100CBAB0);
	}

} addresses;



class SpellCallbacks {
#define SPELL_CALLBACK(name) static int __cdecl name(DispatcherCallbackArgs args);
public:

	static int __cdecl ArmorSpellFailure(DispatcherCallbackArgs args);

	static int __cdecl SkillBonus(DispatcherCallbackArgs args);
	static int __cdecl BeginHezrouStench(DispatcherCallbackArgs args);

	static int __cdecl ConcentratingActionSequenceHandler(DispatcherCallbackArgs args); // handles "Stop Concentration" due to action taken
	static int __cdecl ConcentratingActionRecipientHandler(DispatcherCallbackArgs args); // handles "Stop Concentration" due to action received


	static int __cdecl EnlargePersonWeaponDice(DispatcherCallbackArgs args);

	static int __cdecl HezrouStenchObjEvent(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchCountdown(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchTurnbasedStatus(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchAooPossible(DispatcherCallbackArgs args);


	static int __cdecl HezrouStenchAbilityCheckMod(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchSavingThrowLevel(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchDealingDamage(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchToHit2(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchEffectTooltip(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchCureNausea(DispatcherCallbackArgs args);
	static int __cdecl RemoveSpell(DispatcherCallbackArgs args);
	static int __cdecl HasCondition(DispatcherCallbackArgs args);

	static int __cdecl SpellDismissRadialSub(DispatcherCallbackArgs args); // allows dismissal of specific spells
	static int __cdecl SpellAddDismissCondition(DispatcherCallbackArgs args); // prevents dups
	static int __cdecl SpellDismissSignalHandler(DispatcherCallbackArgs args); // fixes issue with dismissing multiple spells
	static int __cdecl DismissSignalHandler(DispatcherCallbackArgs args); // fixes issue with lingering Dismiss Spell holdouts

	static int __cdecl SpellRemoveMod(DispatcherCallbackArgs args); // fixes issue with dismissing multiple spells
	static int __cdecl AoeSpellRemove(DispatcherCallbackArgs args);

	
} spCallbacks;


class GenericCallbacks
{
#define CBFunc(fname) static int __cdecl fname ## (DispatcherCallbackArgs args)
public:
	static int QuerySetReturnVal1(DispatcherCallbackArgs args);
	static int QuerySetReturnVal0(DispatcherCallbackArgs);
	static int ActionInvalidQueryTrue(DispatcherCallbackArgs);

	static int EffectTooltipDuration(DispatcherCallbackArgs args); // SubDispDef data1 denotes the effect type idx, data2 denotes combat.mes line; appends duration
	static int EffectTooltipGeneral(DispatcherCallbackArgs args);
	static int TooltipUnrepeated(DispatcherCallbackArgs); // SubDispDef data1 denotes combat.mes line

	// particle systems
	static int EndParticlesFromArg(DispatcherCallbackArgs args);
	static int PlayParticlesSavePartsysId(DispatcherCallbackArgs args);
	

	static int AddEtherealDamageImmunity(DispatcherCallbackArgs args);
	static int EtherealOnAdd(DispatcherCallbackArgs args);
	static int EtherealOnD20StatusInit(DispatcherCallbackArgs args);
	static int EtherealDamageDealingNull(DispatcherCallbackArgs);
	static int EtherealOnRemove(DispatcherCallbackArgs);

	static int __cdecl SpellResistanceQuery(DispatcherCallbackArgs args);
	static int __cdecl SpellResistanceTooltip(DispatcherCallbackArgs args);

	static int __cdecl TripAooRadial(DispatcherCallbackArgs args);
	static int __cdecl TripAooQuery(DispatcherCallbackArgs args);
	static int __cdecl ImprovedTripBonus(DispatcherCallbackArgs args);

	static int __cdecl PowerAttackDamageBonus(DispatcherCallbackArgs args);
	static int __cdecl PowerAttackToHitPenalty(DispatcherCallbackArgs args);
	static int __cdecl GlobalWieldedTwoHandedQuery(DispatcherCallbackArgs args);

	static int __cdecl GlobalHpChanged(DispatcherCallbackArgs args);


	static int __cdecl HasCondition(DispatcherCallbackArgs args);

	static int __cdecl CastDefensivelyAooTrigger(DispatcherCallbackArgs args);
	static int __cdecl CastDefensivelySpellInterrupted(DispatcherCallbackArgs args);
	
	static int __cdecl D20ModCountdownHandler(DispatcherCallbackArgs args);

	static int __cdecl MonsterRegenerationOnDamage(DispatcherCallbackArgs args);

	static int __cdecl PreferOneHandedWieldRadialMenu(DispatcherCallbackArgs args);
	static int __cdecl PreferOneHandedWieldQuery(DispatcherCallbackArgs args);

} genericCallbacks;


class ItemCallbacks
{
public:
	static int __cdecl SkillBonus(DispatcherCallbackArgs args);

	static int __cdecl UseableItemRadialEntry(DispatcherCallbackArgs args);
	static int __cdecl UseableItemActionCheck(DispatcherCallbackArgs args);

	static int __cdecl BucklerToHitPenalty(DispatcherCallbackArgs args);
	static int __cdecl BucklerAcPenalty(DispatcherCallbackArgs args);
	static int __cdecl WeaponMerciful(DispatcherCallbackArgs);
	static int __cdecl WeaponSeekingAttackerConcealmentMissChance(DispatcherCallbackArgs args);
	static int __cdecl WeaponSpeed(DispatcherCallbackArgs args);
	static int __cdecl WeaponVicious(DispatcherCallbackArgs args);
	static int __cdecl WeaponViciousBlowback(DispatcherCallbackArgs args); // TODO (need to replace the damage calculation function to do this right...)
	static int __cdecl WeaponWounding(DispatcherCallbackArgs args);
	static int __cdecl WeaponThundering(DispatcherCallbackArgs args);

	static int __cdecl WeaponDamageBonus(DispatcherCallbackArgs args);
} itemCallbacks;


class ClassAbilityCallbacks
{
#define FeatFunc(fname) static int __cdecl Feat ## fname ## (DispatcherCallbackArgs args)
public:
	// note: conditions obtained from feats always arg0 set to the feat enum (automatically)
	static int __cdecl FeatDamageReduction(DispatcherCallbackArgs args);
	static int __cdecl FeatEmptyBody(DispatcherCallbackArgs args); // radial menu builder
	static int __cdecl FeatEmptyBodyInit(DispatcherCallbackArgs args);
	static int __cdecl FeatEmptyBodyReduceRounds(DispatcherCallbackArgs);

	static int __cdecl FeatQuiveringPalmRadial(DispatcherCallbackArgs args); // radial menu builder
	static int __cdecl FeatQuiveringPalmInit(DispatcherCallbackArgs);
	static int __cdecl FeatQuiveringPalmPerform(DispatcherCallbackArgs args);
	static int __cdecl FeatQuiveringPalmAvailable(DispatcherCallbackArgs args);



	// Timed effect callbacks; assumption: num of rounds remaining is at arg[2]
	static int __cdecl GetNumRoundsRemaining(DispatcherCallbackArgs args);
	static int __cdecl TimedEffectCountdown(DispatcherCallbackArgs args);

	// Diamond Soul
	static int __cdecl FeatDiamondSoulInit(DispatcherCallbackArgs args);
	static int __cdecl FeatDiamondSoulSpellResistanceMod(DispatcherCallbackArgs args);

	// Aura Of Courage
	static int __cdecl CouragedAuraSavingThrow(DispatcherCallbackArgs args);


	static int FeatBrewPotionRadialMenu(DispatcherCallbackArgs args);
	static int FeatScribeScrollRadialMenu(DispatcherCallbackArgs args);
	static int FeatCraftWandRadial(DispatcherCallbackArgs args);
	static int FeatCraftRodRadial(DispatcherCallbackArgs args);
	static int FeatCraftWondrousRadial(DispatcherCallbackArgs args);
	static int FeatCraftStaffRadial(DispatcherCallbackArgs args);
	static int FeatForgeRingRadial(DispatcherCallbackArgs args);
	static int FeatCraftMagicArmsAndArmorRadial(DispatcherCallbackArgs args);

	static int CraftWandOnAdd(DispatcherCallbackArgs args);

	static int __cdecl FeatIronWillSave(DispatcherCallbackArgs args);

	static int ItemCreationBuildRadialMenuEntry(DispatcherCallbackArgs args, ItemCreationType itemCreationType, char* helpSystemString, MesHandle combatMesLine);
	
	
	static int DruidWildShapeInit(DispatcherCallbackArgs args);
	static int DruidWildShapedInit(DispatcherCallbackArgs args);
	static int DruidWildShapeReset(DispatcherCallbackArgs args);
	static int DruidWildShapeD20StatusInit(DispatcherCallbackArgs args);
	static int DruidWildShapeGetNumAttacks(DispatcherCallbackArgs args);
	static int DruidWildShapeRadialMenu(DispatcherCallbackArgs args);
	static int DruidWildShapeCheck(DispatcherCallbackArgs args);
	static int DruidWildShapePerform(DispatcherCallbackArgs args);
	static int DruidWildShapeScale(DispatcherCallbackArgs args);


	static int BardicMusicBeginRound(DispatcherCallbackArgs args);
	static int BardMusicRadial(DispatcherCallbackArgs args);
	static int BardMusicCheck(DispatcherCallbackArgs args);
	static int BardMusicActionFrame(DispatcherCallbackArgs args);
	static int BardicMusicInspireRefresh(DispatcherCallbackArgs args); // refreshes the duration
	static int BardicMusicInspireBeginRound(DispatcherCallbackArgs args); // ticks away the duration
	static int BardicMusicInspireOnAdd(DispatcherCallbackArgs args);
	static int BardicMusicGreatnessToHitBonus(DispatcherCallbackArgs args);
	static int BardicMusicGreatnessSaveBonus(DispatcherCallbackArgs args);
	static int BardicMusicTooltip(DispatcherCallbackArgs args);
	static int BardicMusicEffectTooltip(DispatcherCallbackArgs args);
	static int BardicMusicGreatnessTakingTempHpDamage(DispatcherCallbackArgs args);
	static int BardicMusicHeroicsSaveBonus(DispatcherCallbackArgs args);
	static int BardicMusicHeroicsAC(DispatcherCallbackArgs args);
	static int BardicMusicSuggestionFearQuery(DispatcherCallbackArgs args);

	static int SneakAttackDamage(DispatcherCallbackArgs args);
} classAbilityCallbacks;

class RaceAbilityCallbacks
{
public:
	static int __cdecl HalflingThrownWeaponAndSlingBonus(DispatcherCallbackArgs args);
} raceCallbacks;

class ConditionFunctionReplacement : public TempleFix {
public:
	static int LayOnHandsPerform(DispatcherCallbackArgs arg);
	static int RemoveDiseasePerform(DispatcherCallbackArgs arg); // also used in WholenessOfBodyPerform
	void HookSpellCallbacks();
	static int TurnUndeadHook(objHndl, Stat shouldBeClassCleric, DispIoD20ActionTurnBased* evtObj);
	static int TurnUndeadCheck(DispatcherCallbackArgs args);
	static int TurnUndeadPerform(DispatcherCallbackArgs args);

	static bool StunningFistHook(objHndl objHnd, objHndl caster, int DC, int saveType, int flags);
	
	//Old version of the function to be used within the replacement
	int (*oldTurnUndeadPerform)(DispatcherCallbackArgs) = nullptr;
	
	void apply() override {
		logger->info("Replacing Condition-related Functions");

		//conds.RegisterNewConditions();


		replaceFunction(0x100E19C0, _CondStructAddToHashtable);
		replaceFunction(0x100E1A80, _GetCondStructFromHashcode);
		replaceFunction(0x100E1AB0, _CondNodeGetArg);
		replaceFunction(0x100E1AD0, _CondNodeSetArg);
		replaceFunction(0x100E1DD0, _CondNodeAddToSubDispNodeArray);

		replaceFunction(0x100E22D0, _ConditionAddDispatch);
		replaceFunction<uint32_t(__cdecl)(Dispatcher*, CondStruct*)>(0x100E24C0, [](Dispatcher* dis, CondStruct* cond) {
			return _ConditionAddToAttribs_NumArgs0(dis, cond, false);
		});
		replaceFunction<uint32_t(__cdecl)(Dispatcher*, CondStruct*, int, int)>(0x100E2500, [](Dispatcher* dis, CondStruct* cond, int arg1, int arg2) {
			return _ConditionAddToAttribs_NumArgs2(dis, cond, arg1, arg2, false);
		});
		replaceFunction<uint32_t(__cdecl)(Dispatcher*, CondStruct*)>(0x100E24E0, [](Dispatcher* dis, CondStruct* cond) {
			return _ConditionAdd_NumArgs0(dis, cond, false);
		});
		replaceFunction<uint32_t(__cdecl)(Dispatcher*, CondStruct*, int, int)>(0x100E2530, [](Dispatcher* dis, CondStruct* cond, int arg1, int arg2) {
			return _ConditionAdd_NumArgs2(dis, cond, arg1, arg2, false);
		});
		replaceFunction<uint32_t(__cdecl)(Dispatcher*, CondStruct*, int, int, int)>(0x100E2560, [](Dispatcher* dis, CondStruct* cond, int arg1, int arg2, int arg3)	{
			return 	_ConditionAdd_NumArgs3(dis, cond, arg1, arg2, arg3, false);
		});
		replaceFunction<uint32_t(__cdecl)(Dispatcher*, CondStruct*, int, int, int, int)>(0x100E2590, [](Dispatcher* dis, CondStruct* cond, int arg1, int arg2, int arg3, int arg4){
			return _ConditionAdd_NumArgs4(dis, cond, arg1, arg2, arg3, arg4, false);
		});
		replaceFunction(0x100E25C0, InitCondFromCondStructAndArgs);
		replaceFunction(0x100ED030, ConditionRemoveCallback);
		
		replaceFunction(0x100EABB0, BarbarianRageStatBonus);
		replaceFunction(0x100EABE0, BarbarianRageSaveBonus);
		replaceFunction(0x100EAC10, BarbarianRageACPenalty);
		
		replaceFunction(0x100ECF30, ConditionPrevent);
		replaceFunction(0x100ECF60, ConditionPreventWithArg);
		replaceFunction(0x100ECFA0, ConditionOverrideBy);

		replaceFunction(0x100EE050, GlobalGetArmorClass);
		replaceFunction(0x100EE280, GlobalToHitBonus);
		replaceFunction(0x100EE760, GlobalOnDamage);
		
		// fixes for lack of uniqueAnimID registration
		replaceFunction(0x100FA060, LayOnHandsPerform);
		replaceFunction(0x100FB150, RemoveDiseasePerform);
		static int (__cdecl* orgLayOnHandsPerformOnActionFrame)(DispatcherCallbackArgs) = replaceFunction<int(__cdecl)(DispatcherCallbackArgs)>(0x100FA0F0, [](DispatcherCallbackArgs args){
			return orgLayOnHandsPerformOnActionFrame(args);
		});
		


		replaceFunction(0x100F7B60, _FeatConditionsRegister);
		replaceFunction(0x100F7BE0, _GetCondStructFromFeat);

		replaceFunction(0x100F7D60, CombatExpertiseRadialMenu);
		replaceFunction(0x100F7F00, CombatExpertiseSet);
		replaceFunction(0x100F88C0, TwoWeaponFightingBonus);
		replaceFunction(0x100F8940, TwoWeaponFightingBonusRanger);
		replaceFunction(0x100FEBA0, BarbarianDamageResistance);

		
		replaceFunction(0x10101150, ItemSkillBonusCallback);

		// Replace hooks for S_Is_BreakFree_Possible so they also return the spell Id
		int writeVal = (int)QueryRetrun1GetArgs;
		write(0x102E0F1C + 8, &writeVal, sizeof(int*)); // web on
		write(0x102D7958 + 8, &writeVal, sizeof(int*)); // entangle on


		//replaceFunction(0x100C7180, QueryReturn1GetArgs); // caused a crash :(

		HookSpellCallbacks();

		// Armor Spell Failure
		replaceFunction<int(DispatcherCallbackArgs)>(0x100EF060, spCallbacks.ArmorSpellFailure);
			
		// Enlarge Person weapon damage dice modification
		replaceFunction<int(DispatcherCallbackArgs)>(0x100CA2B0, spCallbacks.EnlargePersonWeaponDice);

		// power attack damage bonus and To Hit penalty
		replaceFunction<int(DispatcherCallbackArgs)>(0x100F8540, genericCallbacks.PowerAttackDamageBonus);
		replaceFunction<int(DispatcherCallbackArgs)>(0x100F84C0, genericCallbacks.PowerAttackToHitPenalty);

		// Use Item ActionCheck & Radial
		replaceFunction<int(DispatcherCallbackArgs)>(0x10100B60, itemCallbacks.UseableItemActionCheck);
		replaceFunction<int(DispatcherCallbackArgs)>(0x10100840, itemCallbacks.UseableItemRadialEntry);
		
		// couraged aura
		replaceFunction<int(DispatcherCallbackArgs)>(0x100EB100, classAbilityCallbacks.CouragedAuraSavingThrow);

		// two-handed wielded query
		replaceFunction<int(DispatcherCallbackArgs)>(0x100EF4B0, genericCallbacks.GlobalWieldedTwoHandedQuery);

		// buckler to-hit penalty
		replaceFunction<int(DispatcherCallbackArgs)>(0x10104DA0, itemCallbacks.BucklerToHitPenalty);

		// buckler AC penalty
		replaceFunction<int(DispatcherCallbackArgs)>(0x10104E40, itemCallbacks.BucklerAcPenalty);



		// Druid wild shape
		replaceFunction<int(DispatcherCallbackArgs)>(0x100FBDB0, classAbilityCallbacks.DruidWildShapeReset);
		replaceFunction<int(DispatcherCallbackArgs)>(0x100FBB20, classAbilityCallbacks.DruidWildShapeRadialMenu);
		replaceFunction<int(DispatcherCallbackArgs)>(0x100FBC60, classAbilityCallbacks.DruidWildShapeCheck);
		replaceFunction<int(DispatcherCallbackArgs)>(0x100FBCE0, classAbilityCallbacks.DruidWildShapePerform);

		// Fixes Weapon Damage Bonus for ammo items
		replaceFunction<int(DispatcherCallbackArgs)>(0x100FFE90, itemCallbacks.WeaponDamageBonus);

		// Cast Defensively Aoo Trigger Query, SpellInterrupted Query
		replaceFunction<int(DispatcherCallbackArgs)>(0x100F8BE0, genericCallbacks.CastDefensivelyAooTrigger);
		replaceFunction<int(DispatcherCallbackArgs)>(0x100F8CC0, genericCallbacks.CastDefensivelySpellInterrupted);

		// bardic music
		replaceFunction<int(DispatcherCallbackArgs)>(0x100FE220, classAbilityCallbacks.BardMusicRadial);
		replaceFunction<int(DispatcherCallbackArgs)>(0x100FE470, classAbilityCallbacks.BardMusicCheck);
		replaceFunction<int(DispatcherCallbackArgs)>(0x100FE570, classAbilityCallbacks.BardMusicActionFrame);
		replaceFunction<int(DispatcherCallbackArgs)>(0x100FE820, classAbilityCallbacks.BardicMusicBeginRound);
		replaceFunction<int(DispatcherCallbackArgs)>(0x100EA960, classAbilityCallbacks.BardicMusicGreatnessTakingTempHpDamage);
				
		writeHex(0x102E6608 + 3*sizeof(int), "03" ); // fixes the Competence effect tooltip (was pointing to Inspire Courage)

		// Sneak Attack damage generalization
		replaceFunction<int(DispatcherCallbackArgs)>(0x100F9A10, classAbilityCallbacks.SneakAttackDamage);
		SubDispDefNew sdd(dispTypeDealingDamageWeaponlikeSpell, 0, classAbilityCallbacks.SneakAttackDamage, 0u,0u); // Weapon-like spell damage hook
		write(0x102ED2A8, &sdd, sizeof(SubDispDefNew));

		// D20Mods countdown handler
		replaceFunction<int(DispatcherCallbackArgs)>(0x100EC9B0, genericCallbacks.D20ModCountdownHandler);

		replaceFunction<int(DispatcherCallbackArgs)>(0x100F72E0, genericCallbacks.MonsterRegenerationOnDamage);

		// Turn Undead extension
		redirectCall(0x1004AF5F, TurnUndeadHook);
		oldTurnUndeadPerform = replaceFunction(0x1004AEB0, TurnUndeadPerform);

		replaceFunction<int(DispatcherCallbackArgs)>(0x1004ADE0, TurnUndeadCheck);




		// racial callbacks
		replaceFunction<int(DispatcherCallbackArgs)>(0x100FDC70, raceCallbacks.HalflingThrownWeaponAndSlingBonus);

		replaceFunction<int(DispatcherCallbackArgs)>(0x100CBAB0, spCallbacks.SpellRemoveMod);

		// Stunning Fist extension
		redirectCall(0x100E84B0, StunningFistHook);
	}
} condFuncReplacement;


CondNode::CondNode(CondStruct *cond) {
	memset(this, 0, sizeof(CondNode));
	condStruct = cond;
}


#pragma region Condition Add Functions

CondFeatDictionary::CondFeatDictionary(){
	featEnum = FEAT_NONE;
	featEnumMax = FEAT_NONE;
	condStruct.cs = nullptr;
	condArg = 0;
}

CondFeatDictionary::CondFeatDictionary(CondStructNew* cs, feat_enums Feat, feat_enums FeatMax, uint32_t arg){
	featEnum = Feat;
	featEnumMax = FeatMax;
	condStruct.cs = cs;
	condArg = arg;
}

int32_t _CondNodeGetArg(CondNode* condNode, uint32_t argIdx)
{
	return conds.CondNodeGetArg(condNode, argIdx);
}

void _CondNodeSetArg(CondNode* condNode, uint32_t argIdx, uint32_t argVal)
{
	conds.CondNodeSetArg(condNode, argIdx, argVal);
}

uint32_t _ConditionAddDispatch(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
	assert(condStruct->numArgs >= 0 && condStruct->numArgs <= 8);

	vector<int> args;
	if (condStruct->numArgs > 0) {
		args.push_back(arg1);
	}
	if (condStruct->numArgs > 1) {
		args.push_back(arg2);
	}
	if (condStruct->numArgs > 2) {
		args.push_back(arg3);
	}
	if (condStruct->numArgs > 3) {
		args.push_back(arg4);
	}


	return _ConditionAddDispatchArgs(dispatcher, ppCondNode, condStruct, args);
};

uint32_t _ConditionAddDispatchArgs(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, const vector<int> &args) {
	assert(condStruct->numArgs >= args.size());

	// pre-add section (may abort adding condition, or cause another condition to be deleted first)
	DispIoCondStruct dispIO14h;
	dispIO14h.dispIOType = dispIoTypeCondStruct;
	dispIO14h.condStruct = condStruct;
	dispIO14h.outputFlag = 1;
	dispIO14h.arg1 = 0;
	dispIO14h.arg2 = 0;
	if (args.size() > 0) {
		dispIO14h.arg1 = args[0];
	}
	if (args.size() > 1) {
		dispIO14h.arg2 = args[1];
	}

	_DispatcherProcessor(dispatcher, dispTypeConditionAddPre, 0, (DispIO*)&dispIO14h);

	if (dispIO14h.outputFlag == 0) {
		return 0;
	}

	// adding condition
	auto condNodeNew = new CondNode(condStruct);
	for (unsigned int i = 0; i < condStruct->numArgs; ++i) {
		if (i < args.size()) {
			condNodeNew->args[i] = args[i];
		} else {
			// Fill the rest with zeros
			condNodeNew->args[i] = 0;
		}
	}

	CondNode** ppNextCondeNode = ppCondNode;

	while (*ppNextCondeNode != nullptr) {
		ppNextCondeNode = &(*ppNextCondeNode)->nextCondNode;
	}
	*ppNextCondeNode = condNodeNew;

	_CondNodeAddToSubDispNodeArray(dispatcher, condNodeNew);


	auto dispatcherSubDispNodeType1 = dispatcher->subDispNodes[1];
	while (dispatcherSubDispNodeType1 != nullptr) {
		if (dispatcherSubDispNodeType1->subDispDef->dispKey == 0
			&& (dispatcherSubDispNodeType1->condNode->flags & 1) == 0
			&& condNodeNew == dispatcherSubDispNodeType1->condNode) {
			dispatcherSubDispNodeType1->subDispDef->dispCallback(dispatcherSubDispNodeType1, dispatcher->objHnd, dispTypeConditionAdd, 0, nullptr);
		}

		dispatcherSubDispNodeType1 = dispatcherSubDispNodeType1->next;
	}

	return 1;
};

void _CondNodeAddToSubDispNodeArray(Dispatcher* dispatcher, CondNode* condNode) {
	auto subDispDef = condNode->condStruct->subDispDefs;

	while (subDispDef->dispType != 0) {
		auto subDispNodeNew = (SubDispNode *)malloc(sizeof(SubDispNode));
		subDispNodeNew->subDispDef = subDispDef;
		subDispNodeNew->next = nullptr;
		subDispNodeNew->condNode = condNode;


		auto dispType = subDispDef->dispType;
		assert(dispType >= 0 && dispType < dispTypeCount);

		auto ppDispatcherSubDispNode = &(dispatcher->subDispNodes[dispType]);

		if (*ppDispatcherSubDispNode != nullptr) {
			while ((*ppDispatcherSubDispNode)->next != nullptr) {
				ppDispatcherSubDispNode = &((*ppDispatcherSubDispNode)->next);
			}
			(*ppDispatcherSubDispNode)->next = subDispNodeNew;
		}
		else {
			dispatcher->subDispNodes[subDispDef->dispType] = subDispNodeNew;
		}


		subDispDef += 1;
	}

};


CondStruct* _getCondStruct_RegardVanillaHook(CondStruct* condStruct, bool isInternalUse){
	if (!isInternalUse) {
		// this section is meant for vanilla conditions that may be added by their direct address.
		// The hooked function will go here, whereas internal Temple+ usage won't.
		auto condStructNew = conds.GetByName(condStruct->condName);
		if (condStructNew != condStruct) {
			return condStructNew;
		}
	}
	return condStruct;
}

uint32_t _ConditionAddToAttribs_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct, bool isInternalUse) {

	condStruct = _getCondStruct_RegardVanillaHook(condStruct, isInternalUse);
	return _ConditionAddDispatch(dispatcher, &dispatcher->permanentMods, condStruct, 0, 0, 0, 0);
};

uint32_t _ConditionAddToAttribs_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, bool isInternalUse) {

	condStruct = _getCondStruct_RegardVanillaHook(condStruct, isInternalUse);
	return _ConditionAddDispatch(dispatcher, &dispatcher->permanentMods, condStruct, arg1, arg2, 0, 0);
};

uint32_t _ConditionAdd_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct, bool isInternalUse) {
	condStruct = _getCondStruct_RegardVanillaHook(condStruct, isInternalUse);
	return _ConditionAddDispatch(dispatcher, &dispatcher->conditions, condStruct, 0, 0, 0, 0);
};

uint32_t _ConditionAdd_NumArgs1(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, bool isInternalUse) {
	condStruct = _getCondStruct_RegardVanillaHook(condStruct, isInternalUse);
	return _ConditionAddDispatch(dispatcher, &dispatcher->conditions, condStruct, arg1, 0, 0, 0);
};

uint32_t _ConditionAdd_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, bool isInternalUse) {
	condStruct = _getCondStruct_RegardVanillaHook(condStruct, isInternalUse);
	return _ConditionAddDispatch(dispatcher, &dispatcher->conditions, condStruct, arg1, arg2, 0, 0);
};

uint32_t _ConditionAdd_NumArgs3(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, bool isInternalUse) {
	condStruct = _getCondStruct_RegardVanillaHook(condStruct, isInternalUse);
	return _ConditionAddDispatch(dispatcher, &dispatcher->conditions, condStruct, arg1, arg2, arg3, 0);
};

uint32_t _ConditionAdd_NumArgs4(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, bool isInternalUse) {
	condStruct = _getCondStruct_RegardVanillaHook(condStruct, isInternalUse);
	return _ConditionAddDispatch(dispatcher, &dispatcher->conditions, condStruct, arg1, arg2, arg3, arg4);
}

void InitCondFromCondStructAndArgs(Dispatcher* dispatcher, CondStruct* condStruct, int* condargs)
{
	conds.InitCondFromCondStructAndArgs(dispatcher, condStruct, condargs);
};

#pragma endregion

int ConditionPrevent(DispatcherCallbackArgs args)
{
	DispIoCondStruct * dispIO = _DispIoCheckIoType1((DispIoCondStruct*)args.dispIO);
	if (dispIO == nullptr)
	{
		logger->error("Dispatcher Error! Condition {} fuckup, wrong DispIO type", args.subDispNode->condNode->condStruct->condName);
		return 0; // if we get here then VERY BAD!
	}
	if (dispIO->condStruct == (CondStruct *)args.subDispNode->subDispDef->data1)
	{
		dispIO->outputFlag = 0;
		return 0;
	}

	// Adding the following to accomodate CondStruct replacements (see extend_existing in python_dispatcher.cpp)
	// Explanation:
	// ToEE stores the actual CondStruct memory address instead of cond ID inside data1
	// So we retrieve the name from the condstruct address
	// and then fetch the up-to-date condition
	auto refCond = (CondStruct *)args.subDispNode->subDispDef->data1;
	if (!refCond)
		return 0;
	refCond = conds.GetByName(refCond->condName); // re-retrieve it via the NAME
	if (refCond && dispIO->condStruct == refCond){
		dispIO->outputFlag = 0;
		return 0;
	}

	return 0;
};

int ConditionPreventWithArg(DispatcherCallbackArgs args)
{
	int arg1 = conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	DispIoCondStruct *dispIo = dispatch.DispIoCheckIoType1((DispIoCondStruct *)args.dispIO);
	
	auto refCond = (CondStruct *)args.subDispNode->subDispDef->data1;
	if (dispIo->condStruct == refCond && dispIo->arg1 == arg1){
		dispIo->outputFlag = 0;
		return 0;
	}
	
	if (!refCond)
		return 0;
	refCond = conds.GetByName(refCond->condName); // re-retrieve it via the NAME
	
	if (dispIo->condStruct == refCond && dispIo->arg1 == arg1)
		dispIo->outputFlag = 0;

	return 0;
}

int ConditionOverrideBy(DispatcherCallbackArgs args){
	DispIoCondStruct *dispIo = dispatch.DispIoCheckIoType1((DispIoCondStruct *)args.dispIO);
	if (!dispIo)
		return 0;
	
	auto refCond = (CondStruct *)args.subDispNode->subDispDef->data1;
	if (dispIo->condStruct == refCond ) {
		args.RemoveCondition();
		return 0;
	}

	if (!refCond)
		return 0;
	refCond = conds.GetByName(refCond->condName); // re-retrieve it via the NAME
	if (dispIo->condStruct == refCond) {
		args.RemoveCondition();
		return 0;
	}

	return 0;
}

int CondResetArgs(DispatcherCallbackArgs args)
{
	memset(args.subDispNode->condNode->args, 0, args.subDispNode->condNode->condStruct->numArgs);
	return 0;
}
/*
used in BeginRound callback to update the remaining duration and remove condition when it ends
*/
int ConditionDurationTicker(DispatcherCallbackArgs args)
{
	auto argIdx = args.subDispNode->subDispDef->data1;
	auto condArg = _CondNodeGetArg(args.subDispNode->condNode, args.subDispNode->subDispDef->data1);
	auto dispIo = dispatch.DispIoCheckIoType6(args.dispIO);
	int durationRemaining = condArg - static_cast<int>(dispIo->data1);
	if (durationRemaining >= 0)
	{
		_CondNodeSetArg(args.subDispNode->condNode, argIdx, durationRemaining);
	} else
	{
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
	}
	return 0;
}

int ConditionRemoveCallback(DispatcherCallbackArgs args)
{
	conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
	return 0;
}

int RemoveSpellConditionAndMod(DispatcherCallbackArgs args)
{
	auto argsCopy = args;
	argsCopy.dispKey = DK_SIG_Action_Recipient;
	argsCopy.RemoveSpell();
	argsCopy.RemoveSpellMod();
	return 0;
};

/*
data1 - argIdx (to store partsys ID)
data2 - char * to particle system name
*/
int PlayParticlesSavePartsysId(DispatcherCallbackArgs args)
{
	auto partsysName = reinterpret_cast<char*>(args.subDispNode->subDispDef->data2);
	if (partsysName)
	{
		auto partsysId = gameSystems->GetParticleSys().CreateAtObj(partsysName, args.objHndCaller);
		_CondNodeSetArg(args.subDispNode->condNode,
			args.subDispNode->subDispDef->data1, partsysId);
	}
	return 0;
}

int EndParticlesFromArg(DispatcherCallbackArgs args)
{
	auto partsysId = _CondNodeGetArg(args.subDispNode->condNode, args.subDispNode->subDispDef->data1);
	if (partsysId)
	{
		gameSystems->GetParticleSys().End(partsysId);
		_CondNodeSetArg(args.subDispNode->condNode,
			args.subDispNode->subDispDef->data1, 0);
	}
	return 0;
}

int DivineMightEffectTooltipCallback(DispatcherCallbackArgs args)
{
	void * dispIo = args.dispIO;
	int (__cdecl* callback )(int, int, int)= (int(__cdecl*)(int, int, int))temple::GetPointer(0x100F4760);
	const char shit[] = "Divine Might";
	callback( *((int*)dispIo + 1), args.subDispNode->subDispDef->data1, (int)shit);
	return 0;
};

/*
gets a tooltip string from combat.mes
*/
int GenericCallbacks::TooltipUnrepeated(DispatcherCallbackArgs args)
{
	DispIoTooltip *dispIo = dispatch.DispIoCheckIoType9(args.dispIO);
	auto mesLine = combatSys.GetCombatMesLine(args.subDispNode->subDispDef->data1);
	int numstrings = dispIo->numStrings;
	if (numstrings >= 10)
		return 0;
	int idx = 0;
	for (idx = 0; idx < numstrings && idx < 10 ; idx++)
	{
		if (!strcmp(dispIo->strings[idx], mesLine))
			break;
	}
	if (idx == numstrings) // reached the end and not found
	{
		strncpy(dispIo->strings[numstrings], mesLine, 0x100);
		dispIo->numStrings++;
	}
	return 0;
}

int GenericCallbacks::EndParticlesFromArg(DispatcherCallbackArgs args)
{
	auto psId = args.GetCondArg(args.GetData1());
	if (psId && psId != -1){
		gameSystems->GetParticleSys().End(psId);
		args.SetCondArg(args.GetData1(), 0);
	}
	return 0;
}

int GenericCallbacks::PlayParticlesSavePartsysId(DispatcherCallbackArgs args)
{
	auto psId = gameSystems->GetParticleSys().CreateAtObj((const char*)args.GetData2(), args.objHndCaller);
	args.SetCondArg(args.GetData1(), psId);
	return 0;
};



int __cdecl CondNodeSetArg0FromSubDispDef(DispatcherCallbackArgs args)
{
	conds.CondNodeSetArg(args.subDispNode->condNode, 0, args.subDispNode->subDispDef->data1);
	return 0;
}

int QueryCritterHasCondition(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	if (dispIo->data1 == args.subDispNode->subDispDef->data1 && !dispIo->data2)
	{
		dispIo->return_val = 1;
		dispIo->data1 = _CondNodeGetArg(args.subDispNode->condNode, 0); //spellId
		dispIo->data2 = 0;
	}
	return 0;
}


int GenericCallbacks::QuerySetReturnVal1(DispatcherCallbackArgs args)
{
	DispIoD20Query * dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	dispIo->return_val = 1;
	return 0;
}

int GenericCallbacks::QuerySetReturnVal0(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	dispIo->return_val = 0;
	return 0;
}

int GenericCallbacks::ActionInvalidQueryTrue(DispatcherCallbackArgs args){
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	dispIo->return_val = 1;
	return 0;
}

int GenericCallbacks::AddEtherealDamageImmunity(DispatcherCallbackArgs args){
	auto dispIo = dispatch.DispIoCheckIoType4(args.dispIO);
	dispIo->damage.AddEtherealImmunity();
	return 0;
}

int GenericCallbacks::EtherealOnAdd(DispatcherCallbackArgs args)
{
	floatSys.FloatCombatLine(args.objHndCaller, 210); // Ethereal
	objects.FadeTo(args.objHndCaller, 70, 10, 30, 0);
	return 0;
}

int GenericCallbacks::EtherealOnD20StatusInit(DispatcherCallbackArgs args){
	objects.FadeTo(args.objHndCaller, 70, 10, 30, 0);
	return 0;
}

int GenericCallbacks::EtherealDamageDealingNull(DispatcherCallbackArgs args){
	auto dispIo = dispatch.DispIoCheckIoType4(args.dispIO);
	dispIo->damage.AddEtherealImmunity();
	return 0;
}

int GenericCallbacks::EtherealOnRemove(DispatcherCallbackArgs args)
{
	objects.FadeTo(args.objHndCaller, 255, 0, 5, 0);
	return 0;
}

int GenericCallbacks::SpellResistanceQuery(DispatcherCallbackArgs args){
	auto srMod = args.GetCondArg(1);
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	dispIo->data1 = srMod;
	dispIo->return_val = 1;
	dispIo->data2 = 0;
	return 0;
}

int GenericCallbacks::SpellResistanceTooltip(DispatcherCallbackArgs args){

	auto dispIo = dispatch.DispIoCheckIoType9(args.dispIO);
	auto srMod = args.GetCondArg(1);
	string text( fmt::format("{} [{}]" ,combatSys.GetCombatMesLine(args.GetData1()), srMod) );

	dispIo->Append(text);

	return 0;
}

int GenericCallbacks::TripAooRadial(DispatcherCallbackArgs args){

	// limit this option to characters with Improved Trip, to prevent AoOs during AoOs
	if (!feats.HasFeatCountByClass(args.objHndCaller, FEAT_IMPROVED_TRIP))
		return 0;
	RadialMenuEntryToggle radEntry(5117, args.GetCondArgPtr(1), "TAG_TRIP_ATTACK_OF_OPPORTUNITY" );
	radEntry.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::Options);
	return 0;
}

int GenericCallbacks::TripAooQuery(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	if (args.GetCondArg(1)){
		dispIo->return_val = 1;
	}

	return 0;
}

int GenericCallbacks::ImprovedTripBonus(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType10(args.dispIO);
	if (dispIo->flags & 1){
		dispIo->bonOut->AddBonusWithDesc(4, 0, 114, feats.GetFeatName(FEAT_IMPROVED_TRIP));
	}
	return 0;
}

int GenericCallbacks::PowerAttackDamageBonus(DispatcherCallbackArgs args)
{

	auto powerAttackAmt = args.GetCondArg(0);

	if (!powerAttackAmt)
		return 0;

	args.dispIO->AssertType(dispIOTypeDamage);
	auto dispIo = static_cast<DispIoDamage*>(args.dispIO);

	// ignore ranged weapons
	if (dispIo->attackPacket.flags & D20CAF_RANGED)
		return 0;

	// get wield type
	auto weaponUsed = dispIo->attackPacket.GetWeaponUsed();
	auto wieldType = inventory.GetWieldType(args.objHndCaller, weaponUsed, true);
	auto wieldTypeWeaponModified = inventory.GetWieldType(args.objHndCaller, weaponUsed, false); // the wield type if the weapon is not enlarged along with the critter
	auto modifiedByEnlarge = wieldType != wieldTypeWeaponModified;

	// check offhand
	auto offhandWeapon = inventory.ItemWornAt(args.objHndCaller, EquipSlot::WeaponSecondary);
	auto shield = inventory.ItemWornAt(args.objHndCaller, EquipSlot::Shield);
	auto regardOffhand = (offhandWeapon || shield && !inventory.IsBuckler(shield)) ?true:false; // is there an offhand item (weapon/non-buckler shield)

	// case 1
	switch (wieldType)
	{
	case 0: // light weapon
		switch (wieldTypeWeaponModified)
		{
		case 0:
			dispIo->damage.bonuses.ZeroBonusSetMeslineNum(305);
			return 0;
		case 1: // benefitting from enlargement of weapon
			if (regardOffhand)
				dispIo->damage.bonuses.AddBonusFromFeat(powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
			else
				dispIo->damage.bonuses.AddBonusFromFeat(2 * powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
			return 0;
		case 2:
			if (regardOffhand)
				dispIo->damage.bonuses.AddBonusFromFeat(powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
			else
				dispIo->damage.bonuses.AddBonusFromFeat(2 * powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
			return 0;
		default:
			dispIo->damage.bonuses.AddBonusFromFeat(powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
			return 0;
		}
	case 1: // single handed wield if weapon is unaffected
		switch (wieldTypeWeaponModified)
		{
		case 0: // only in reduce person; going to assume the "beneficial" case that the reduction was made voluntarily and hence you let the weapon stay larger
		case 1: 
			if (regardOffhand)
				dispIo->damage.bonuses.AddBonusFromFeat(powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
			else
				dispIo->damage.bonuses.AddBonusFromFeat(2 * powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
			return 0;
		case 2:
			if (regardOffhand)
				dispIo->damage.bonuses.AddBonusFromFeat(powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
			else
				dispIo->damage.bonuses.AddBonusFromFeat(2 * powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
			return 0;
		default:
			dispIo->damage.bonuses.AddBonusFromFeat(powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
			return 0;
		}
	case 2: // two handed wield if weapon is unaffected
		switch (wieldTypeWeaponModified)
		{
		case 0: 
		case 1: // only in reduce person; going to assume the "beneficial" case that the reduction was made voluntarily and hence you let the weapon stay larger
			if (regardOffhand) // has offhand item, so assume the weapon stayed the old size
				dispIo->damage.bonuses.AddBonusFromFeat(powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
			else
				dispIo->damage.bonuses.AddBonusFromFeat(2 * powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
			return 0;
		case 2:
			if (regardOffhand) // shouldn't really be possible... maybe if player is cheating
			{
				dispIo->damage.bonuses.AddBonusFromFeat(powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
				logger->warn("Illegally wielding weapon along withoffhand!");
			}
			else
				dispIo->damage.bonuses.AddBonusFromFeat(2 * powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
			return 0;
		default:
			dispIo->damage.bonuses.AddBonusFromFeat(powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
			return 0;
		}
	case 3:
		dispIo->damage.bonuses.AddBonusFromFeat(powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
		return 0;
	case 4:
	default:
		dispIo->damage.bonuses.AddBonusFromFeat(powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
		return 0;
	}


	//if (modifiedByEnlarge){
	//	// if using an offhand and also wielding two handed
	//	if (regardOffhand && wieldTypeWeaponModified == 2)
	//		wieldType = wieldTypeWeaponModified;
	//	else if (!wieldType && wieldTypeWeaponModified)
	//		wieldType = wieldTypeWeaponModified;
	//}

	//if(!wieldType && !wieldTypeWeaponModified){
	//	dispIo->damage.bonuses.ZeroBonusSetMeslineNum(305);
	//	return 0;
	//}

	//auto bonAmt = 2*powerAttackAmt;
	//if (wieldType == 4)
	//	bonAmt = powerAttackAmt;
	//else if (wieldType != 2){
	//	if (  regardOffhand) 
	//		bonAmt = powerAttackAmt;
	//}
	//
	//dispIo->damage.bonuses.AddBonusFromFeat(bonAmt, 0, 114, FEAT_POWER_ATTACK);
	//
	//return 0;
}

int GenericCallbacks::PowerAttackToHitPenalty(DispatcherCallbackArgs args)
{
	auto powerAttackAmt = args.GetCondArg(0);

	if (!powerAttackAmt)
		return 0;

	GET_DISPIO(dispIOTypeAttackBonus, DispIoAttackBonus);
	
	// ignore ranged weapons
	if (dispIo->attackPacket.flags & D20CAF_RANGED)
		return 0;

	// get wield type
	auto weaponUsed = dispIo->attackPacket.GetWeaponUsed();
	auto wieldType = inventory.GetWieldType(args.objHndCaller, weaponUsed, true);
	auto wieldTypeWeaponModified = inventory.GetWieldType(args.objHndCaller, weaponUsed, false); // the wield type if the weapon is not enlarged along with the critter
	auto modifiedByEnlarge = wieldType != wieldTypeWeaponModified;

	

	dispIo->bonlist.AddBonusFromFeat(-powerAttackAmt, 0, 114, FEAT_POWER_ATTACK); // todo convenience disabling of to hit penalty when not dual wielding
	return 0;

	//switch (wieldType)
	//{
	//case 0: // light weapon
	//	switch (wieldTypeWeaponModified)
	//	{
	//	case 0:
	//		// is light weapon either way, so no malus (and no damage bonus) applied
	//		return 0;
	//	case 1: // benefitting from enlargement of weapon - so apply to hit malus
	//	case 2:
	//	default:
	//		// add penalty
	//		dispIo->bonlist.AddBonusFromFeat(-powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
	//		return 0;
	//	}
	//case 1: // single handed wield if weapon is unaffected; 
	//case 2: // two handed wield if weapon is unaffected
	//case 3:
	//case 4:
	//default:
	//	// add penalty
	//	dispIo->bonlist.AddBonusFromFeat(-powerAttackAmt, 0, 114, FEAT_POWER_ATTACK);
	//	return 0;
	//}

	return 0;
}

int GenericCallbacks::GlobalWieldedTwoHandedQuery(DispatcherCallbackArgs args)
{
	auto dispIo = static_cast<DispIoD20Query*>(args.dispIO);
	dispIo->AssertType(dispIOTypeQuery);

	auto dispIoAttack = (DispIoDamage*)(dispIo->data1);
	auto weaponUsed = dispIoAttack->attackPacket.GetWeaponUsed();

	if (!weaponUsed)
		return 0;

	auto weapType = (WeaponTypes)gameSystems->GetObj().GetObject(weaponUsed)->GetInt32(obj_f_weapon_type);

	// special case - rapiers are always wielded one handed
	if (weapType == wt_rapier){
		dispIo->return_val = 0;
		return 0;
	}



	auto offhandWeapon = inventory.ItemWornAt(args.objHndCaller, EquipSlot::WeaponSecondary);
	auto shield = inventory.ItemWornAt(args.objHndCaller, EquipSlot::Shield);
	auto isShieldAlloingTwoHandedWield = (shield != objHndl::null) && inventory.IsBuckler(shield); // are you holding the weapon with your buckler hand?
	if (isShieldAlloingTwoHandedWield){
		if (d20Sys.d20Query(args.objHndCaller, DK_QUE_Is_Preferring_One_Handed_Wield))
			isShieldAlloingTwoHandedWield = false;
	}
	auto hasInterferingOffhand = (offhandWeapon != objHndl::null 
								  || (shield != objHndl::null && !isShieldAlloingTwoHandedWield)) ? true : false;

	auto wieldType = inventory.GetWieldType(args.objHndCaller, weaponUsed, true);
	auto wieldTypeWeaponModified = inventory.GetWieldType(args.objHndCaller, weaponUsed, false); // the wield type if the weapon is not enlarged along with the critter
	

	bool isTwohandedWieldable = !hasInterferingOffhand;

	switch (wieldType)
	{
	case 0: // light weapon
		switch (wieldTypeWeaponModified)
		{
		case 0:
			isTwohandedWieldable = false;
			break;
		case 1: // benefitting from enlargement of weapon
		case 2:
		default:
			break;
		}
	case 1: // single handed wield if weapon is unaffected
		switch (wieldTypeWeaponModified)
		{
		case 0: // only in reduce person; going to assume the "beneficial" case that the reduction was made voluntarily and hence you let the weapon stay larger
		case 1:
		case 2:
		default:
			break;
		}
	case 2: // two handed wield if weapon is unaffected
		switch (wieldTypeWeaponModified)
		{
		case 0:
		case 1: // only in reduce person
			break;
		case 2:
			if (hasInterferingOffhand) // shouldn't really be possible to hold two Two Handed Weapons... maybe if player is cheating
			{
				logger->warn("Illegally wielding weapon along withoffhand!");
			}
		default:
			break;
		}
	case 3:
		break;
	case 4:
	default:
		break;
	}

	dispIo->return_val = isTwohandedWieldable;

	return 0;
}

int GenericCallbacks::GlobalHpChanged(DispatcherCallbackArgs args){
	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);

	auto handle = args.objHndCaller;
	auto obj = objSystem->GetObject(handle);

	auto hpCur = objects.StatLevelGet(handle, stat_hp_current);
	auto subdualDam = obj->GetInt32(obj_f_critter_subdual_damage);
	auto lastHitBy = obj->GetObjHndl(obj_f_last_hit_by);
	auto hpChange = dispIo->data2;

	// Kill
	if (hpCur <= -10){
		critterSys.Kill(handle);
		return 0;
	}

	auto isUncon = critterSys.IsDeadOrUnconscious(handle);

	
	auto addDisabled = false;
	auto knockedOut = false;
	auto isDying = false;
	auto hasDiehard = feats.HasFeatCountByClass(args.objHndCaller, FEAT_DIEHARD);

	if (hpCur < 0 && !hasDiehard){
		knockedOut = true;
		if (hpChange < 0){
			isDying = true;
		}
	}
	if (subdualDam >= hpCur){
		if (!hasDiehard)
			knockedOut = true;
		else
			knockedOut = subdualDam >= hpCur + 10;
	}

	// Knock Unconscious
	if (knockedOut){
		d20Sys.D20SignalPython(handle, "Knocked Unconscious");
		if (!isUncon){
			auto animId = Dice::Roll(1, 3, 72); // roll number between 73-75
			gameSystems->GetAnim().PushFallDown(handle, animId);
		}
		if (isDying){
			conds.AddTo(handle, "Dying", {});
			return 0;
		}
		conds.AddTo(handle, "Unconscious", {});
		return 0;
	}
	// Mark Disabled
	else if (hpCur <= 0 ) {
		conds.AddTo(args.objHndCaller, "Disabled", {});
		return 0;
	}

	return 0;
}

int GenericCallbacks::HasCondition(DispatcherCallbackArgs args){
	args.dispIO->AssertType(dispIOTypeQuery);
	auto dispIo = static_cast<DispIoD20Query*>(args.dispIO);
	if (dispIo->data1 == args.GetData1() && !dispIo->data2){
		dispIo->return_val = 1;
		dispIo->data1 = args.GetCondArg(0);
		dispIo->data2 = 0;
	}
	return 0;
}

int GenericCallbacks::CastDefensivelyAooTrigger(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeQuery, DispIoD20Query);



	auto isSet = args.GetCondArg(0);
	auto d20a = (D20Actn*)(dispIo->data1);

	if (d20a->d20ActType== D20A_CAST_SPELL && isSet){
		dispIo->return_val = 0;
		return 0;
	}
	
	
	if (d20a->d20ActType == D20A_USE_ITEM){
		auto invIdx = d20a->d20SpellData.itemSpellData;
		auto item = inventory.GetItemAtInvIdx(args.objHndCaller, invIdx);
		if (!item) {
			dispIo->return_val = 0;
			return 0;
		}
		auto itemObj = gameSystems->GetObj().GetObject(item);
		if (itemObj->type == obj_t_generic || itemObj->type == obj_t_weapon) {
			dispIo->return_val = 0;
			return 0;
		} 
		if (itemObj->type != obj_t_scroll){

			logger->warn("CastDefensivelyAooTrigger: Unexpected item type {}", (int)(itemObj->type));
			int dummy = 1;
		}
		// for the rest of the item types (should only be obj_t_scroll?)
		if (isSet){
			dispIo->return_val = 0;
			return 0;
		}
		// otherwise it will be as default (1)

	}
	
	return 0;
}

int GenericCallbacks::CastDefensivelySpellInterrupted(DispatcherCallbackArgs args){
	auto isSet = args.GetCondArg(0);
	GET_DISPIO(dispIOTypeQuery, DispIoD20Query);

	if (dispIo->return_val == 1)
		return 0; // already interrupted by sthg else

	if (!isSet)
		return 0; // not casting defensively

	if (!combatSys.isCombatActive())
		return 0; // forego this outside of combat

	// check if no threatening melee enemies - if so, disregard casting defensively (since it's just annoying micromanagement!!!)
	auto enemiesCanMelee = combatSys.GetEnemiesCanMelee(args.objHndCaller);
	if (!enemiesCanMelee.size())
		return FALSE;

	auto spellData = (D20SpellData*)(dispIo->data1);
	if (!spellData)
		return 0;

	// odd, but that's where it was in the original code...
	if (feats.HasFeatCountByClass(args.objHndCaller, FEAT_COMBAT_CASTING)){
		conds.AddTo(args.objHndCaller, conds.GetByName("Combat_Casting"), { 0 });
	}

	auto rollRes = skillSys.SkillRoll(args.objHndCaller, SkillEnum::skill_concentration, 15 + spellData->spellSlotLevel, nullptr, 1);
	if (!rollRes){
		histSys.CreateRollHistoryLineFromMesfile(25, args.objHndCaller, objHndl::null);
		floatSys.FloatCombatLine(args.objHndCaller, 58);
		dispIo->return_val = 1;
	}
	return 0;
}

int GenericCallbacks::D20ModCountdownHandler(DispatcherCallbackArgs args){
	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
	auto durArgIdx = args.GetData2();
	auto durRem = args.GetCondArg(durArgIdx);
	int durNew = durRem - dispIo->data1;
	if (durNew >= 0){
		args.SetCondArg(durArgIdx, durNew);
		return 0;
	}

	auto d20modCountdownEndHandler = temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100E98B0);
	if (args.dispType == dispTypeBeginRound){
		
		if (args.GetData1() != 6){ // invisibility
			d20modCountdownEndHandler(args);
			return 0;
		}


		// Invisibility is getting special-cased because of the Ring of Invisbility
		auto spellId = args.GetCondArg(0);

		if (spellId < 0 || spellId > 100000){ // when the Invisbility condition is applied frmo the Ring, arg0 is actually a memory address of the condition; this check is a bit hacky but it should do the trick in most cases
			return 0;
		}

		SpellPacketBody spPkt(spellId);
		if (spPkt.spellEnum == 0)
			d20modCountdownEndHandler(args);
	} else
	{
		d20modCountdownEndHandler(args);
	}
	return 0;
}

int GenericCallbacks::MonsterRegenerationOnDamage(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeDamage, DispIoDamage);
	auto arg0 = args.GetCondArg(0);
	auto arg1 = args.GetCondArg(1);

	if (arg0 == 0 && arg1 == 0)
	{
		arg0 = (int)DamageType::Fire;
		arg1 = (int)DamageType::Acid;
		args.SetCondArg(0, arg0);
		args.SetCondArg(1, arg1);
	}

	if (arg0 < 0)
	{
		if ((arg0 & 0x7fffFFFF) == dispIo->damage.attackPowerType)
			return 0;
		arg0 = arg1;
	}

	auto replacedDice = false;
	for (auto i = 0u; i < dispIo->damage.diceCount; i++) {
		if (dispIo->damage.dice[i].type != (DamageType)arg0 
			&& dispIo->damage.dice[i].type != (DamageType)arg1)
		{
			replacedDice = true;
			dispIo->damage.dice[i].type = DamageType::Subdual;
		}
	}
	if (replacedDice)
		dispIo->damage.bonuses.ZeroBonusSetMeslineNum(322);
	return 0;
}

int GenericCallbacks::PreferOneHandedWieldRadialMenu(DispatcherCallbackArgs args)
{
	auto shield = inventory.ItemWornAt(args.objHndCaller, EquipSlot::Shield);
	if (!inventory.IsBuckler(shield))
		return 0;

	RadialMenuEntryToggle radEntry(5124, args.GetCondArgPtr(0), "TAG_RADIAL_MENU_PREFER_ONE_HANDED_WIELD");
	radEntry.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::Options);
	return 0;
}

int GenericCallbacks::PreferOneHandedWieldQuery(DispatcherCallbackArgs args)
{
	GET_DISPIO(dispIOTypeQuery, DispIoD20Query);
	auto isCurrentlyOn = args.GetCondArg(0);

	dispIo->return_val = isCurrentlyOn;
	return 0;
}

int GenericCallbacks::EffectTooltipDuration(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType24(args.dispIO);
	auto numRounds = args.GetCondArg(2);

	dispIo->Append( args.GetData1(), -1, fmt::format("{}\n{}: {}",combatSys.GetCombatMesLine(args.GetData2()), combatSys.GetCombatMesLine(175), numRounds).c_str() );
	return 0;
}

int GenericCallbacks::EffectTooltipGeneral(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeEffectTooltip, DispIoEffectTooltip);
	dispIo->Append(args.GetData1(), -1, nullptr);
	return 0;
}

int QuerySetReturnVal0(DispatcherCallbackArgs args)
{
	DispIoD20Query * dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	dispIo->return_val = 0;
	return 0;
};

int QueryRetrun1GetArgs(DispatcherCallbackArgs args)
{
	DispIoD20Query * dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	dispIo->return_val = 1;
	dispIo->data1 = conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	dispIo->data2 = conds.CondNodeGetArg(args.subDispNode->condNode, 1);
	return 0;
};

int DisarmedRetrieveQuery(DispatcherCallbackArgs args)
{
	DispIoD20Query * dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	dispIo->return_val = 1;
	objHndl weapon;
	ObjectId objId;
	memcpy(&objId, args.subDispNode->condNode->args, sizeof(ObjectId));
	weapon = gameSystems->GetObj().GetHandleById(objId);
	*(objHndl*)&dispIo->data1 = weapon;
	return 0;
};

int __cdecl CondNodeSetArgFromSubDispDef(DispatcherCallbackArgs args)
{
	// sets arg[data1] from data2  
	// e.g. IF data1 = 0, data2 = 15 
	//    THEN it'll set arg0 = 15
	conds.CondNodeSetArg(args.subDispNode->condNode, args.subDispNode->subDispDef->data1,
		args.subDispNode->subDispDef->data2);
	return 0;
};


int __cdecl CondArgDecrement(DispatcherCallbackArgs args)
{
	conds.CondNodeSetArg(args.subDispNode->condNode, args.subDispNode->subDispDef->data1, 
		conds.CondNodeGetArg(args.subDispNode->condNode, args.subDispNode->subDispDef->data1) - 1);
	return 0;
};

int __cdecl ItemSkillBonusCallback(DispatcherCallbackArgs args)
{
	/*
	used by conditions: Skill Circumstance Bonus, Skill Competence Bonus
	*/
	SkillEnum skillEnum = (SkillEnum)conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	int bonValue = conds.CondNodeGetArg(args.subDispNode->condNode, 1);
	int bonType = args.subDispNode->subDispDef->data1;
	if (args.dispKey - 20 == skillEnum)
	{
		int invIdx = conds.CondNodeGetArg(args.subDispNode->condNode, 2);
		objHndl itemHnd = inventory.GetItemAtInvIdx(args.objHndCaller, invIdx);
		DispIoObjBonus * dispIo = dispatch.DispIoCheckIoType10((DispIoObjBonus*)args.dispIO);
		const char * name = description.getDisplayName(itemHnd, args.objHndCaller);
		bonusSys.bonusAddToBonusListWithDescr(dispIo->bonOut, bonValue, bonType, 112, (char*)name);
	}
	return 0;
}


int __cdecl GlobalToHitBonus(DispatcherCallbackArgs args)
{
	DispIoAttackBonus * dispIo = dispatch.DispIoCheckIoType5((DispIoAttackBonus*)args.dispIO);
	dispatch.DispatchToHitBonusBase(args.objHndCaller, dispIo);

	// natural attack - get attack bonus from internal defs
	if (dispIo->attackPacket.dispKey >= (ATTACK_CODE_NATURAL_ATTACK+1) 
		 && !d20Sys.d20Query(args.objHndCaller, DK_QUE_Polymorphed) )
	{
		int attackIdx = dispIo->attackPacket.dispKey - (ATTACK_CODE_NATURAL_ATTACK + 1);
		int bonValue = 0; // temporarily used as an index value for obj_f_attack_bonus_idx field
		for (int i = 0,  j=0; i < 4; i++)
		{
			j += objects.getArrayFieldInt32(args.objHndCaller, obj_f_critter_attacks_idx, i); // number of attacks
			if (attackIdx < j){
				bonValue = i;
				break;
			}
		}
		bonValue = objects.getArrayFieldInt32(args.objHndCaller, obj_f_attack_bonus_idx, bonValue);
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, bonValue, 1, 118); // base attack
	}

	if (dispIo->attackPacket.flags & D20CAF_RANGED) // get to hit modifier from DEX
	{
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 
			objects.GetModFromStatLevel(objects.abilityScoreLevelGet(args.objHndCaller, stat_dexterity, 0)) , 3, 104);
	} else // get to hit mod from STR
	{
		bonusSys.bonusAddToBonusList(&dispIo->bonlist,
			objects.GetModFromStatLevel(objects.abilityScoreLevelGet(args.objHndCaller, stat_strength, 0)), 2, 103);
	}

	int attackCode = dispIo->attackPacket.dispKey;
	if (attackCode < ATTACK_CODE_NATURAL_ATTACK) // apply penalties for Nth attack
	{
		int attackNumber = 1;
		int dualWielding = 0;
		d20Sys.ExtractAttackNumber(args.objHndCaller, attackCode, &attackNumber, &dualWielding);
		if (attackNumber <= 0)
		{
			int dummy = 1;
			assert(attackNumber > 0);
		}
		switch (attackNumber)
		{
		case 1: 
			break;
		case 2:
			bonusSys.bonusAddToBonusList(&dispIo->bonlist, -(attackNumber-1) * 5, 24, 119);
			break;
		default:
			bonusSys.bonusAddToBonusList(&dispIo->bonlist, -(attackNumber - 1) * 5, 25, 120);
		}
		if (dualWielding)
		{
			if (d20Sys.UsingSecondaryWeapon(args.objHndCaller, attackCode))
				bonusSys.bonusAddToBonusList(&dispIo->bonlist, -10, 26, 121); // penalty for dualwield on offhand attack
			else
				bonusSys.bonusAddToBonusList(&dispIo->bonlist, -6, 27, 122); // penalty for dualwield on primary attack

			auto offhand = inventory.ItemWornAt(args.objHndCaller, 4);
			if (offhand)
			{
				if (inventory.GetWieldType(dispIo->attackPacket.attacker, offhand) == 0)
					bonusSys.bonusAddToBonusList(&dispIo->bonlist, 2, 0, 167); // Light Off-hand Weapon
			}
		}
	}

	// helplessness bonus
	if (dispIo->attackPacket.victim
		&& d20Sys.d20Query(dispIo->attackPacket.victim, DK_QUE_Helpless)
		&& !d20Sys.d20Query(dispIo->attackPacket.victim, DK_QUE_Critter_Is_Stunned))
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 4, 30, 136);
	
	// flanking bonus
	if (combatSys.IsFlankedBy(dispIo->attackPacket.victim, dispIo->attackPacket.attacker))
	{
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 2, 0, 201);
		*(int*)(&dispIo->attackPacket.flags ) |= (int)D20CAF_FLANKED;
	}

	// size bonus / penalty
	int sizeCategory = dispatch.DispatchGetSizeCategory(args.objHndCaller);
	int sizeCatBonus = critterSys.GetBonusFromSizeCategory(sizeCategory);
	bonusSys.bonusAddToBonusList(&dispIo->bonlist, sizeCatBonus, 0, 115);

	if (dispIo->attackPacket.flags & D20CAF_RANGED)
	{

		if (dispIo->attackPacket.victim)
		{
			// firing into melee penalty
			objHndl canMeleeList[100];

			int numEnemiesCanMelee = combatSys.GetEnemiesCanMelee(dispIo->attackPacket.victim, canMeleeList);
			if (numEnemiesCanMelee > 0
				&& (numEnemiesCanMelee != 1 || canMeleeList[0]!= args.objHndCaller)
				&& !feats.HasFeatCount(args.objHndCaller, FEAT_PRECISE_SHOT))
				bonusSys.bonusAddToBonusList(&dispIo->bonlist, -4, 0, 150); 
		
			// range penalty 
			objHndl weapon = combatSys.GetWeapon(&dispIo->attackPacket);
			float dist = locSys.DistanceToObj(args.objHndCaller, dispIo->attackPacket.victim);
			if (dist < 0.0) dist = 0.0;
			if (weapon)
			{
				long double weaponRange = (long double)objects.getInt32(weapon, obj_f_weapon_range);
				int rangePenaltyFacotr = (int)(dist / weaponRange);
				if ((int)rangePenaltyFacotr > 0)
					bonusSys.bonusAddToBonusList(&dispIo->bonlist, -2 * rangePenaltyFacotr, 0, 303);
			}
		}
	}

	return 0;
}

int GlobalGetArmorClass(DispatcherCallbackArgs args) // the basic AC value (initial value and some misc. global modifiers)
{
	DispIoAttackBonus * dispIo = dispatch.DispIoCheckIoType5((DispIoAttackBonus*)args.dispIO);
	BonusList *bonlist = &dispIo->bonlist;

	// Armor Class initial value 10
	bonusSys.bonusAddToBonusList(bonlist, 10, 1, 102); 
	
	// add npc natural armor AC bonus
	if (!(dispIo->attackPacket.flags & D20CAF_TOUCH_ATTACK))
	{
		objHndl defender = args.objHndCaller;
		int polymorphedTo = d20Sys.d20Query(args.objHndCaller, DK_QUE_Polymorphed);
		if (polymorphedTo)
			defender = objects.GetProtoHandle(polymorphedTo);
		if (objects.GetType(args.objHndCaller) == obj_t_npc || polymorphedTo)
		{
			bonusSys.bonusAddToBonusList(bonlist, objects.getInt32(defender, obj_f_npc_ac_bonus), 9, 123);
		} else{
			auto race = critterSys.GetRace(defender, false);
			auto racialAcBonus = d20RaceSys.GetNaturalArmor(race);
			bonlist->AddBonus(racialAcBonus, 9, 123);
		}
	}

	// add size bonus / penalty to AC
	int sizeCat = dispatch.DispatchGetSizeCategory(args.objHndCaller);
	int sizeCatBonus = critterSys.GetBonusFromSizeCategory(sizeCat);
	bonusSys.bonusAddToBonusList(bonlist, sizeCatBonus, 0, 115); 

	// dex bonus
	int dexScore = objects.abilityScoreLevelGet(args.objHndCaller, stat_dexterity, 0);
	int dexMod = objects.GetModFromStatLevel(dexScore);
	bonusSys.bonusAddToBonusList(bonlist, dexMod, 3, 104);

	// dodging trap
	if (dispIo->attackPacket.flags & D20CAF_TRAP)
	{
		if (dispIo->attackPacket.victim && feats.HasFeatCount(dispIo->attackPacket.victim, FEAT_UNCANNY_DODGE))
		{
			bonusSys.zeroBonusSetMeslineNum(bonlist, 165); // dex bonus retained due to Uncanny Dodge
			return 0;
		}
		bonusSys.bonusCapAdd(bonlist, 8, 0, 0x99u); // flatfooted
		bonusSys.bonusCapAdd(bonlist, 3, 0, 0x99u);
	}

	if (dispIo->attackPacket.flags & D20CAF_COVER)
	{
		if (dispIo->attackPacket.attacker && feats.HasFeatCount(dispIo->attackPacket.attacker, FEAT_IMPROVED_PRECISE_SHOT))
		{
			bonusSys.zeroBonusSetMeslineNum(bonlist, 335); // Cover negated by Imp. Precise Shot
			return 0;
		}

		if (dispIo->attackPacket.attacker && feats.HasFeatCountByClass(dispIo->attackPacket.attacker, FEAT_IMPROVED_PRECISE_SHOT_RANGER, (Stat)0, 0))
		{
			if (critterSys.IsWearingLightOrNoArmor(dispIo->attackPacket.attacker))
			{
				bonusSys.zeroBonusSetMeslineNum(bonlist, 335); // Cover negated by Imp. Precise Shot
				return 0;
			}
						
		}

		if (dispIo->attackPacket.attacker && feats.HasFeatCount(dispIo->attackPacket.attacker, FEAT_SHARP_SHOOTING))
		{
			bonusSys.bonusAddToBonusList(bonlist, 2, 0, 336);; // Cover (diminished by Sharp-Shooting)
			return 0;
		}
		bonusSys.bonusAddToBonusList(bonlist, 4, 0, 309);
	}
		
	return 0;
}


int __cdecl GlobalOnDamage(DispatcherCallbackArgs args)
{
	DispIoDamage * dispIo = dispatch.DispIoCheckIoType4(args.dispIO);
	objHndl weapon = combatSys.GetWeapon(&dispIo->attackPacket);
	int polymorphedTo = d20Sys.d20Query(args.objHndCaller, DK_QUE_Polymorphed);
	objHndl v34 = args.objHndCaller;
	DispIoAttackDice dispIoAttackDice;
	int attackDice;
	DamageType attackDamageType = DamageType::Bludgeoning;
	const char * weaponName = feats.emptyString;
	int damageMesLine = 100; // ~Weapon~[TAG_WEAPONS]

	if (polymorphedTo)
	{
		v34 = objects.GetProtoHandle(polymorphedTo);
	}

	if (weapon)
	{
		dispIoAttackDice.flags = dispIo->attackPacket.flags;
		dispIoAttackDice.wielder = args.objHndCaller;
		dispIoAttackDice.weapon = weapon;
		attackDice = dispatch.Dispatch60GetAttackDice(args.objHndCaller, &dispIoAttackDice);
		attackDamageType = dispIoAttackDice.attackDamageType;
		weaponName = description.getDisplayName(weapon, args.objHndCaller);
	} 
	else // unarmed
	{
		int attackCode = dispIo->attackPacket.dispKey;
		if (attackCode > ATTACK_CODE_NATURAL_ATTACK ) // natural attack
		{
			int attackIdx = attackCode - (ATTACK_CODE_NATURAL_ATTACK + 1);
			int attackDiceUnarmed = critterSys.GetCritterDamageDice(v34, attackIdx);
			
			damageMesLine = critterSys.GetCritterAttackType(v34, attackIdx) + 114;
			attackDamageType = critterSys.GetCritterAttackDamageType(v34, attackIdx);
			dispIoAttackDice.flags = dispIo->attackPacket.flags;
			dispIoAttackDice.weapon = 0i64;
			dispIoAttackDice.wielder = args.objHndCaller;
			dispIoAttackDice.dicePacked = attackDiceUnarmed;
			dispIoAttackDice.attackDamageType = attackDamageType;
			attackDice = dispatch.Dispatch60GetAttackDice(args.objHndCaller, &dispIoAttackDice);
		} 
		else
		{
			int monkLvl = objects.StatLevelGet(args.objHndCaller, stat_level_monk);
			
			attackDamageType = DamageType::Subdual;
			if (feats.HasFeatCountByClass(args.objHndCaller, FEAT_IMPROVED_UNARMED_STRIKE) > 0) {
				//Note:  Bludgeoning is zero so this will be default if nothing answers the query
				int nDamageType = d20Sys.D20QueryPython(args.objHndCaller, "Unarmed Damage Type");
				attackDamageType = static_cast<DamageType>(nDamageType);
			}
			
			damageMesLine = 113; // Unarmed

			auto beltItem = inventory.ItemWornAt(args.objHndCaller, EquipSlot::Lockpicks);
			if (beltItem){
				auto beltObj = gameSystems->GetObj().GetObject(beltItem);
				if (beltObj->protoId.GetPrototypeId() == 12420){
					monkLvl += 5;
				}
			}

			int attackDiceType = 3;
			int attackDiceCount = 1;
			int dudeSize = objects.StatLevelGet(args.objHndCaller, stat_size);
			if (dudeSize< 5) // small monk
			{
				if (monkLvl <= 0)
				{
					attackDiceType = 2;
				} else if (monkLvl < 4)
				{
					attackDiceType = 4;
				} else 
				if (monkLvl < 8)
				{
					attackDiceType = 6;
				}
				else if (monkLvl < 12)
				{
					attackDiceType = 8;
				}
				else if (monkLvl < 16)
				{
					attackDiceCount = 1;
					attackDiceType = 10;
				} else if (monkLvl < 20)
				{
					attackDiceCount = 2;
					attackDiceType = 6;
				} else // 20 and above
				{
					attackDiceCount = 2;
					attackDiceType = 8;
				}
			}
			else if (dudeSize > 5) // Large Monk
			{
				if (monkLvl <= 0)
				{
					attackDiceType = 4;
				}
				else if (monkLvl < 4)
				{
					attackDiceType = 8;
				}
				else if (monkLvl < 8)
				{
					attackDiceCount = 2;
					attackDiceType = 6;
				}
				else if (monkLvl < 12)
				{
					attackDiceCount = 2;
					attackDiceType = 8;
				}
				else if (monkLvl < 16)
				{
					attackDiceCount = 3;
					attackDiceType = 6;
				}
				else if (monkLvl < 20)
				{
					attackDiceCount = 3;
					attackDiceType = 8;
				}
				else
				{
					attackDiceCount = 4;
					attackDiceType = 8;
				}
			}
			else // normal monk
			{
				if (monkLvl <= 0)
				{
					attackDiceType = 3;
				} else if (monkLvl < 4)
				{
					attackDiceType = 6;
				} else 
				if (monkLvl < 8)
				{
					attackDiceType = 8;
				}
				else if (monkLvl < 12)
				{
					attackDiceType = 10;
				}
				else if (monkLvl < 16)
				{
					attackDiceCount = 2;
					attackDiceType = 6;
				} else if (monkLvl < 20)
				{
					attackDiceCount = 2;
					attackDiceType = 8;
				} else
				{
					attackDiceCount = 2;
					attackDiceType = 10;
				}

			}

			Dice diceUnarmed(attackDiceCount, attackDiceType, 0);
			attackDice = diceUnarmed.ToPacked();

		}

	}
	damage.AddDamageDice(&dispIo->damage, attackDice, attackDamageType, damageMesLine);

	int strScore = objects.StatLevelGet(args.objHndCaller, stat_strength);
	int strMod = objects.GetModFromStatLevel(strScore);
	D20CAF flags = dispIo->attackPacket.flags;
	if (!(flags & D20CAF_RANGED) || (flags & D20CAF_THROWN))
	{
		int attackCode = dispIo->attackPacket.dispKey;
		if (d20Sys.UsingSecondaryWeapon(args.objHndCaller, attackCode))
		{
			if (strMod > 0)
			{
				strMod /= 2;
			}
				
		} else if (d20Sys.d20QueryWithData(args.objHndCaller, DK_QUE_WieldedTwoHanded, (int)dispIo, 0) 
			&& strMod > 0 
			&&  inventory.GetWieldType(args.objHndCaller, weapon, true) )
		{
			strMod += strMod / 2;
		}
		if (attackCode > ATTACK_CODE_NATURAL_ATTACK && strMod > 0 && critterSys.GetDamageIdx(args.objHndCaller, attackCode - (ATTACK_CODE_NATURAL_ATTACK + 1)) > 0)
		{
			strMod /= 2;
		}
		damage.AddDamageBonusWithDescr(&dispIo->damage, strMod, 2, 103, 0);
		return 0;
	}
	if (!weapon)
		return 0;
	auto weaponType = objects.GetWeaponType(weapon);
	if (weaponType == wt_sling || ((weaponType == wt_shortbow ||weaponType == wt_longbow )&& strMod < 0))
	{
		damage.AddDamageBonusWithDescr(&dispIo->damage, strMod, 2, 103, 0);
	}
	return 0;
}


int CaptivatingSongOnConditionAdd(DispatcherCallbackArgs args);

int __cdecl TurnBasedStatusInitNoActions(DispatcherCallbackArgs args){
	auto dispIo = dispatch.DispIoCheckIoType8(args.dispIO);
	if (dispIo){
		auto tbStat = dispIo->tbStatus;
		if (tbStat){
			tbStat->hourglassState = 0;
			dispIo->tbStatus->tbsFlags |= TurnBasedStatusFlags::TBSF_Movement;
			logger->debug("Zeroed actions for {}", description.getDisplayName(args.objHndCaller));
		}
	}
	return 0;
}

int __cdecl CaptivatingSongEffectTooltipDuration(DispatcherCallbackArgs args){
	
	auto dispIo = dispatch.DispIoCheckIoType24(args.dispIO);
	int durationRemaining = _CondNodeGetArg(args.subDispNode->condNode, 0);
	char tooltipString[256];
	sprintf(tooltipString, "\n%d rounds remaining.", durationRemaining);
	auto effectTooltipBase = temple::GetRef<int(__cdecl)(BuffDebuffPacket*, int someIdx, int spellEnum, char*)>(0x100F4680);
	effectTooltipBase(dispIo->bdb, 100, 90000, tooltipString); // will fetch 90000 from spell_ext.mes (Captivated!)
	return 0;
}


class Conditions
{
public:
	static void AddConditionsToTable();
	std::map<feat_enums,CondFeatDictionary> condDict;
} conditions;

void _FeatConditionsRegister()
{

	// In moebiues DLL the condition table was moved and extended
	auto condCount = 84u;
	if (temple::Dll::GetInstance().IsVanillaDll()) {
		conds.FeatConditionDict = temple::GetPointer<CondFeatDictionary>(0x102EEC40);
		condCount = 79u;
	}

	conds.hashmethods.CondStructAddToHashtable(conds.ConditionAttackOfOpportunity);
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionCastDefensively);
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionCombatCasting);
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionDealSubdualDamage );
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionDealNormalDamage);
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionFightDefensively);
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionAnimalCompanionAnimal);
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionAutoendTurn);
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionTurnUndead);
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionGreaterTurning);
	
	// Add the destruction domain to the condition table so it can be accessed in python
	CondStruct * pDestructionDomain = *(conds.ConditionArrayDomains + 3 * Domain_Destruction);
	conds.hashmethods.CondStructAddToHashtable(pDestructionDomain);

	// Craft Wand
	static CondStructNew craftWand("Craft Wand", 0);
	craftWand.AddHook(dispTypeRadialMenuEntry, DK_NONE, classAbilityCallbacks.CraftWandOnAdd);
	craftWand.AddToFeatDictionary(FEAT_CRAFT_WAND);

	// Brew Potion
	static CondStructNew brewPotion("Brew Potion", 0);
	brewPotion.AddHook(dispTypeRadialMenuEntry, DK_NONE, classAbilityCallbacks.FeatBrewPotionRadialMenu);
	brewPotion.AddToFeatDictionary(FEAT_BREW_POTION);

	// Scribe Scroll
	static CondStructNew scribeScroll("Scribe Scroll", 0);
	scribeScroll.AddHook(dispTypeRadialMenuEntry, DK_NONE, classAbilityCallbacks.FeatScribeScrollRadialMenu);
	scribeScroll.AddToFeatDictionary(FEAT_SCRIBE_SCROLL);
	
	// Forge Ring
	static CondStructNew forgeRing("Forge Ring", 0);
	forgeRing.AddHook(dispTypeRadialMenuEntry, DK_NONE, classAbilityCallbacks.FeatForgeRingRadial);
	forgeRing.AddToFeatDictionary(FEAT_FORGE_RING);

	// Craft Staff
	static CondStructNew craftStaff("Craft Staff", 0);
	craftStaff.AddHook(dispTypeRadialMenuEntry, DK_NONE, classAbilityCallbacks.FeatCraftStaffRadial);
	craftStaff.AddToFeatDictionary(FEAT_CRAFT_STAFF);

	// Craft Rod
	static CondStructNew craftRod("Craft Rod", 0);
	craftRod.AddHook(dispTypeRadialMenuEntry, DK_NONE, classAbilityCallbacks.FeatCraftRodRadial);
	craftRod.AddToFeatDictionary(FEAT_CRAFT_ROD);

	// Craft Magic Arms and Armor
	static CondStructNew craftMaa("Craft Magic Arms and Armor", 0);
	craftMaa.AddHook(dispTypeRadialMenuEntry, DK_NONE, classAbilityCallbacks.FeatCraftMagicArmsAndArmorRadial);
	craftMaa.AddToFeatDictionary(FEAT_CRAFT_MAGIC_ARMS_AND_ARMOR);

	for (auto i = 0u; i < condCount; i++){
		conds.hashmethods.CondStructAddToHashtable(conds.FeatConditionDict[i].condStruct.old);
	}
}

uint32_t  _GetCondStructFromFeat(feat_enums featEnum, CondStruct ** condStructOut, uint32_t * argout)
{
	switch (featEnum)
	{
	case FEAT_GREATER_TWO_WEAPON_FIGHTING:
		*condStructOut = (CondStruct*)conds.mCondGreaterTwoWeaponFighting;
		*argout = 0;
		return 1;
	case FEAT_GREATER_TWO_WEAPON_FIGHTING_RANGER:
		*condStructOut = (CondStruct*)conds.mCondGreaterTWFRanger;
		*argout = 0;
		return 1;
	case FEAT_DIVINE_MIGHT:
		*condStructOut = (CondStruct*)conds.mCondDivineMight;
		*argout = 0;
		return 1;
	case FEAT_RECKLESS_OFFENSE:
		*condStructOut = (CondStruct*)conds.mCondRecklessOffense;
		*argout = 0;
		return 1;
	case FEAT_KNOCK_DOWN:
		*condStructOut = (CondStruct*)conds.mCondKnockDown;
		*argout = 0;
		return 1;
	case FEAT_SUPERIOR_EXPERTISE:
		return 0; // willl just be patched inside Combat Expertise
	case FEAT_DEADLY_PRECISION:
		*condStructOut = (CondStruct*)conds.mCondDeadlyPrecision;
		*argout = 0;
		return 1;
	case FEAT_PERSISTENT_SPELL:
		*condStructOut = (CondStruct*)conds.mCondPersistentSpell;
		*argout = 0;
		return 1;
	default:
		break;
	}

	if (featEnum >= FEAT_GREATER_WEAPON_SPECIALIZATION_GAUNTLET && featEnum <= FEAT_GREATER_WEAPON_SPECIALIZATION_GRAPPLE)
	{
		*condStructOut = (CondStruct*)conds.mCondGreaterWeaponSpecialization;
		*argout = featEnum - FEAT_GREATER_WEAPON_SPECIALIZATION_GAUNTLET;
		return 1;
	}
	
	// Search the new Feat-CondStruct dictionary
	auto it = conditions.condDict.find(featEnum);
	if (it != conditions.condDict.end()){
		if (it->second.featEnum == featEnum && it->second.condStruct.cs != nullptr){
			*condStructOut = static_cast<CondStruct*>(it->second.condStruct.old);
			*argout = it->second.condArg;
			return 1;
		}
	};

	feat_enums * featFromDict = & ( conds.FeatConditionDict->featEnum );
	uint32_t iter = 0;
	while (
		( (int32_t)featEnum != featFromDict[0] || featFromDict[1] != -1)
		&&  ( (int32_t)featEnum < (int32_t)featFromDict[0] 
				|| (int32_t)featEnum >= (int32_t)featFromDict[1]  )
		)
	{
		iter += 16;
		featFromDict += 4;
		if (iter >= 0x540){ return 0; }
	}

	*condStructOut = (CondStruct *)*(featFromDict - 1);
	*argout = featEnum + featFromDict[2] - featFromDict[0];
	return 1;
}

uint32_t _CondStructAddToHashtable(CondStruct * condStruct)
{
	return conds.hashmethods.CondStructAddToHashtable(condStruct);
}

CondStruct * _GetCondStructFromHashcode(uint32_t key)
{
	return conds.hashmethods.GetCondStruct(key);
}

CondStruct* ConditionSystem::GetByName(const string& name) {
	auto key = ElfHash::Hash(name.c_str());
	return hashmethods.GetCondStruct(key);
}

CondStruct* ConditionSystem::GetById(const int condId)
{
	return hashmethods.GetCondStruct(condId);
}

void ConditionSystem::DoForAllCondStruct(void(*cb)(CondStruct &condStruct)){
	uint32_t bitmask = (mCondStructHashtable->powerOfTwo - 1);

	for (auto i = 0u; i < mCondStructHashtable->numItems; i++) {
		auto idx = mCondStructHashtable->idxArray[i];
		auto dataEntry = &mCondStructHashtable->dataArray[idx];
		if (*dataEntry) {
			cb(**dataEntry);
		}
	}
	
}

void ConditionSystem::AddToItem(objHndl item, const CondStruct* cond, const vector<int>& args) {
	assert(args.size() == cond->numArgs);

	auto obj = objSystem->GetObject(item);
	auto curCondCount = obj->GetInt32Array(obj_f_item_pad_wielder_condition_array).GetSize();
	auto curCondArgCount = obj->GetInt32Array(obj_f_item_pad_wielder_argument_array).GetSize();

	// Add the condition name hash to the list
	auto key = ElfHash::Hash(cond->condName);
	obj->SetInt32(obj_f_item_pad_wielder_condition_array, curCondCount, key);

	auto idx = curCondArgCount;
	for (auto arg : args) {
		obj->SetInt32(obj_f_item_pad_wielder_argument_array, idx, arg);
		idx++;
	}
}

bool ConditionSystem::AddTo(objHndl handle, const CondStruct* cond, const vector<int>& args) {
	assert(args.size() == cond->numArgs);

	auto dispatcher = objects.GetDispatcher(handle);

	if (!dispatch.dispatcherValid(dispatcher)) {
		logger->info("Dispatcher invalid for {}", objects.description.getDisplayName(handle));
		return false;
	}

	return _ConditionAddDispatchArgs(dispatcher, &dispatcher->conditions, const_cast<CondStruct*>(cond), args) != 0;
}

bool ConditionSystem::AddTo(objHndl handle, const string& name, const vector<int>& args) {
	auto cond = GetByName(name);
	if (!cond) {
		logger->warn("Unable to find condition {}", name);
		return false;
	}

	return AddTo(handle, cond, args);
}

bool ConditionSystem::ConditionAddDispatchArgs(Dispatcher* dispatcher, CondNode** nodes, CondStruct* condStruct, const vector<int>& args)
{
	return _ConditionAddDispatchArgs(dispatcher, nodes, condStruct, args) != 0;

}

int32_t ConditionSystem::CondNodeGetArg(CondNode* condNode, uint32_t argIdx)
{
	if (argIdx < condNode->condStruct->numArgs)
	{
		return condNode->args[argIdx];
	}
	return 0;
}

void ConditionSystem::CondNodeSetArg(CondNode* condNode, uint32_t argIdx, uint32_t argVal)
{
	if (argIdx < condNode->condStruct->numArgs)
	{
		condNode->args[argIdx] = argVal;
	}
}

void ConditionSystem::CondNodeAddToSubDispNodeArray(Dispatcher* dispatcher, CondNode* condNodeNew)
{
	_CondNodeAddToSubDispNodeArray(dispatcher, condNodeNew);
}

void ConditionSystem::InitCondFromCondStructAndArgs(Dispatcher* dispatcher, CondStruct* condStruct, int* condargs)
{
	CondNode **v4; 
	SubDispNode *subDispNode; 
	CondNode *condNode; 

	auto *condNodeNew = new CondNode(condStruct);
	v4 = &dispatcher->conditions;
	while (*v4)
	{
		v4 = & (*v4)->nextCondNode;
	}
	*v4 = condNodeNew;

	for (auto i = 0u; i < condStruct->numArgs; i++) {
		condNodeNew->args[i] = condargs[i];
	}
	conds.CondNodeAddToSubDispNodeArray(dispatcher, condNodeNew);
	for (subDispNode = dispatcher->subDispNodes[dispTypeConditionAddFromD20StatusInit]; subDispNode; subDispNode = subDispNode->next)
	{
		if (subDispNode->subDispDef->dispKey == 0)
		{
			condNode = subDispNode->condNode;
			if (!(condNode->flags & 1) && condNode == condNodeNew)
				subDispNode->subDispDef->dispCallback(subDispNode,dispatcher->objHnd, dispTypeConditionAddFromD20StatusInit, 0, nullptr);
		}
	}
}

void ConditionSystem::InitItemCondFromCondStructAndArgs(Dispatcher * dispatcher, CondStruct * condStruct, int * condargs){
	CondNode **node;
	SubDispNode *subDispNode;
	CondNode *condNode;

	auto *condNodeNew = new CondNode(condStruct);
	node = &dispatcher->itemConds;
	while (*node)
	{
		node = &(*node)->nextCondNode;
	}
	*node = condNodeNew;

	for (auto i = 0u; i < condStruct->numArgs; i++) {
		condNodeNew->args[i] = condargs[i];
	}

	conds.CondNodeAddToSubDispNodeArray(dispatcher, condNodeNew);
	for (subDispNode = dispatcher->subDispNodes[dispTypeConditionAddFromD20StatusInit]; subDispNode; subDispNode = subDispNode->next)
	{
		if (subDispNode->subDispDef->dispKey == 0)
		{
			condNode = subDispNode->condNode;
			if (!(condNode->flags & 1) && condNode == condNodeNew)
				subDispNode->subDispDef->dispCallback(subDispNode, dispatcher->objHnd, dispTypeConditionAddFromD20StatusInit, 0, nullptr);
		}
	}
}

void ConditionSystem::RegisterNewConditions()
{

	CondStructNew * cond;
	char * condName;

#pragma region Feats

	// Disable AoO
	mConditionDisableAoO = &conditionDisableAoO;
	cond = mConditionDisableAoO; 	condName = mConditionDisableAoOName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Disable AoO", sizeof("Disable AoO"));

	cond->condName = mConditionDisableAoOName;
	cond->numArgs = 1;

	DispatcherHookInit(cond, 0, dispTypeD20Query, DK_QUE_AOOPossible, AoODisableQueryAoOPossible,	0, 0);
	DispatcherHookInit(cond, 1, dispTypeRadialMenuEntry, 0, AoODisableRadialMenuInit, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)mConditionDisableAoO, 0);
	DispatcherHookInit(cond, 3, dispType0, 0, 0, 0, 0);

	// Greater Two Weapon Fighting
	mCondGreaterTwoWeaponFighting = &conditionGreaterTwoWeaponFighting;
	cond = mCondGreaterTwoWeaponFighting; 	condName = mConditionGreaterTwoWeaponFightingName;
	memset(condName, 0, sizeof(condName));
	memcpy(condName, "Greater Two Weapon Fighting", sizeof("Greater Two Weapon Fighting"));

	cond->condName = mConditionGreaterTwoWeaponFightingName;
	cond->numArgs = 2;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)mCondGreaterTwoWeaponFighting, 0);
	DispatcherHookInit(cond, 1, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)mCondGreaterTWFRanger, 0);
	DispatcherHookInit(cond, 2, dispTypeGetNumAttacksBase, 0, GreaterTwoWeaponFighting, 0, 0); // same callback as Improved TWF (it just adds an extra attack... logic is inside the action sequence / d20 / GlobalToHit functions
	DispatcherHookInit(cond, 3, dispType0, 0, nullptr, 0, 0);

	// Greater TWF Ranger
	mCondGreaterTWFRanger = &condGreaterTWFRanger;
	cond = mCondGreaterTWFRanger; 	condName = mCondGreaterTWFRangerName;
	memset(condName, 0, sizeof(condName));
	memcpy(condName, "Greater Two Weapon Fighting Ranger", sizeof("Greater Two Weapon Fighting Ranger"));

	cond->condName = mCondGreaterTWFRangerName;
	cond->numArgs = 2;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)mCondGreaterTwoWeaponFighting, 0);
	DispatcherHookInit(cond, 1, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)mCondGreaterTWFRanger, 0); // TODO: add TWF_RANGER
	DispatcherHookInit(cond, 2, dispTypeGetNumAttacksBase, 0, GreaterTWFRanger, 0, 0); // same callback as Improved TWF (it just adds an extra attack... logic is inside the action sequence / d20 / GlobalToHit functions
	DispatcherHookInit(cond, 3, dispType0, 0, nullptr, 0, 0);

	// Divine Might Ability
	mCondDivineMight = &condDivineMight;
	cond = mCondDivineMight; 	condName = mCondDivineMightName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Divine Might", sizeof("Divine Might"));

	cond->condName = condName;
	cond->numArgs = 2;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0 ,ConditionPrevent, (uint32_t) cond , 0 );
	DispatcherHookInit(cond, 1, dispTypeConditionAdd, 0, CondNodeSetArg0FromSubDispDef, 1, 0);
	DispatcherHookInit(cond, 2, dispTypeRadialMenuEntry, 0, DivineMightRadial, 0, 0);

	// Divine Might Bonus (gets activated when you choose the action from the Radial Menu)
	mCondDivineMightBonus = &condDivineMightBonus;
	cond = mCondDivineMightBonus; 	condName = mCondDivineMightBonusName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Divine Might Bonus", sizeof("Divine Might Bonus"));

	cond->condName = condName;
	cond->numArgs = 2;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeDealingDamage, 0, DivineMightDamageBonus, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeBeginRound, 0, ConditionRemoveCallback, 0, 0);
	DispatcherHookInit(cond, 3, dispTypeD20Signal, DK_SIG_Killed, ConditionRemoveCallback, 0, 0);
	DispatcherHookInit(cond, 4, dispTypeEffectTooltip, 0, DivineMightEffectTooltipCallback, 81, 0);

	// Reckless Offense
	mCondRecklessOffense = &condRecklessOffense;
	cond = mCondRecklessOffense; 	condName = mCondRecklessOffenseName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Reckless Offense", sizeof("Reckless Offense"));

	cond->condName = condName;
	cond->numArgs = 2;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeRadialMenuEntry, 0, RecklessOffenseRadialMenuInit, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeGetAC, 0, RecklessOffenseAcPenalty, 0, 0);
	DispatcherHookInit(cond, 3, dispTypeToHitBonus2, 0, RecklessOffenseToHitBonus, 0, 0);
	DispatcherHookInit(cond, 4, dispTypeD20Signal, DK_SIG_Attack_Made, TacticalOptionAbusePrevention, 0, 0);
	DispatcherHookInit(cond, 5, dispTypeBeginRound, 0, CondNodeSetArgFromSubDispDef, 1, 0);
	DispatcherHookInit(cond, 6, dispTypeConditionAdd, 0, CondNodeSetArgFromSubDispDef, 0, 0);

	// Knock Down
	mCondKnockDown = &condKnockDown;
	cond = mCondKnockDown; 	condName = mCondKnockDownName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Knock-Down", sizeof("Knock-Down"));

	cond->condName = condName;
	cond->numArgs = 2;

	// Greater Weapon Specialization
	mCondGreaterWeaponSpecialization = &condGreaterWeaponSpecialization;
	cond = mCondGreaterWeaponSpecialization; 	condName = mCondGreaterWeaponSpecializationName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Greater Weapon Specialization", sizeof("Greater Weapon Specialization"));

	cond->condName = condName;
	cond->numArgs = 2;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPreventWithArg, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeDealingDamage, 0, GreaterWeaponSpecializationDamage, 0, 0);

	// Deadly Precision
	mCondDeadlyPrecision = &condDeadlyPrecision;
	cond = mCondDeadlyPrecision; 	condName = mCondDeadlyPrecisionName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Deadly Precision", sizeof("Deadly Precision"));

	cond->condName = condName;
	cond->numArgs = 2;

	// Persistent Spell
	mCondPersistentSpell = &condPersistentSpell;
	cond = mCondPersistentSpell; 	condName = mCondPersistentSpellName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Persistent Spell", sizeof("Persistent Spell"));

	cond->condName = condName;
	cond->numArgs = 2;

	// Disarm
	cond = &mCondDisarm; 	condName = (char*)mCondDisarmName;

	cond->condName = condName;
	cond->numArgs = 2;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeRadialMenuEntry, 0, DisarmRadialMenu, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeD20Signal, DK_SIG_HP_Changed, DisarmHpChanged, 0, 0);
	DispatcherHookInit(cond, 3, dispTypeD20Query, DK_QUE_ActionTriggersAOO, DisarmQueryAoOResetArg, 1, 1);
	DispatcherHookInit(cond, 4, dispTypeD20Query, DK_QUE_Can_Perform_Disarm, DisarmCanPerform, 0, 0);
	//DispatcherHookInit(cond, 5, dispTypeD20Query, DK_QUE_ActionTriggersAOO, QuerySetReturnVal1, 0, 0);

#pragma endregion
	// Disarmed
	cond = &mCondDisarmed; 	condName = (char*)mCondDisarmedName;

	cond->condName = condName;
	cond->numArgs = 8;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeD20Query, DK_QUE_Disarmed, DisarmedRetrieveQuery, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeD20Signal, DK_SIG_Combat_End, DisarmedReminder, 0, 0);
	DispatcherHookInit(cond, 3, dispTypeD20Signal, DK_SIG_Disarmed_Weapon_Retrieve, DisarmedWeaponRetrieve, 0, 0);
	DispatcherHookInit(cond, 4, dispTypeRadialMenuEntry, 0, DisarmedRetrieveWeaponRadialMenu, 0, 0);
	DispatcherHookInit(cond, 5, dispTypeConditionAdd, 0, DisarmedOnAdd, 0, 0);

#pragma region Monster Abilities
	// Rend

	mCondRend = &condRend;
	cond = mCondRend; 	condName = mCondRendName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Rend", sizeof("Rend"));

	cond->condName = condName;
	cond->numArgs = 8;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeDealingDamage, 0, RendOnDamage, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeBeginRound, 0, CondResetArgs, 0, 0);
	
	// Captivating Song

	cond = &mCondCaptivatingSong; 	condName = (char*)mCondCaptivatingSongName;

	cond->condName = condName;
	cond->numArgs = 8; // 2-7 is the caster objId

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeConditionAdd, 0, CaptivatingSongOnConditionAdd, 1, 0x1028C7C8);

	// Captivated

	cond = &mCondCaptivated; 	condName = (char*)mCondCaptivatedName;

	cond->condName = condName;
	cond->numArgs = 8;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeD20Query, DK_QUE_SneakAttack, genericCallbacks.QuerySetReturnVal1, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeD20Query, DK_QUE_CannotCast, genericCallbacks.QuerySetReturnVal1, 0, 0);
	DispatcherHookInit(cond, 3, dispTypeD20Query, DK_QUE_AOOPossible, QuerySetReturnVal0, 0, 0);
	DispatcherHookInit(cond, 4, dispTypeD20Signal, DK_SIG_Killed, ConditionRemoveCallback, 0, 0);
	DispatcherHookInit(cond, 5, dispTypeTooltip, 0, genericCallbacks.TooltipUnrepeated, 205, 0); // Captivated
	DispatcherHookInit(cond, 6, dispTypeConditionAdd, 0, PlayParticlesSavePartsysId, 1, 0x1028C7C8); // 'Bardic-Fascinate-hit'
	DispatcherHookInit(cond, 7, dispTypeConditionAddFromD20StatusInit, 0, PlayParticlesSavePartsysId, 1, 0x1028C7C8); // 'Bardic-Fascinate-hit'
	DispatcherHookInit(cond, 8, dispTypeConditionRemove, 0, EndParticlesFromArg, 1, 0);
	DispatcherHookInit(cond, 9, dispTypeBeginRound, 0, ConditionDurationTicker, 0, 0);
	DispatcherHookInit(cond, 10, dispTypeTurnBasedStatusInit, 0, TurnBasedStatusInitNoActions, 0, 0);
	DispatcherHookInit(cond, 11, dispTypeEffectTooltip, 0, CaptivatingSongEffectTooltipDuration, 0, 0);


	// Hezrou Stench

	cond = &mCondHezrouStench; 

	cond->condName = "Hezrou Stench";
	cond->numArgs = 4; // 0 - spellId; 1 - duration; 2 - eventId; 3 - partsysId;

	auto aoeSpellRemover = temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100D3430);

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeConditionAdd, 0, spCallbacks.BeginHezrouStench, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeD20Signal, DK_SIG_Spell_End, aoeSpellRemover, 0, 0);
	DispatcherHookInit(cond, 3, dispTypeObjectEvent, 0, spCallbacks.HezrouStenchObjEvent, 0, 0);
	DispatcherHookInit(cond, 4, dispTypeD20Signal, DK_SIG_Combat_End, aoeSpellRemover, 0, 0);
	DispatcherHookInit(cond, 5, dispTypeD20Signal, DK_SIG_Critter_Killed, aoeSpellRemover, 0, 0);

	// Hezrou Stench Nausea / Sickness

	cond = &mCondHezrouStenchHit;

	cond->condName = "Hezrou Stench Hit";
	cond->numArgs = 5; // 0 - spellId; 1 - duration; 2 - eventId; 3 - partsysId; 4 - nausea/sickness (0 = nausea, 1 = sickness)

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeBeginRound, 0, spCallbacks.HezrouStenchCountdown, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeNewDay, DK_NEWDAY_REST, spCallbacks.RemoveSpell, 0, 0);
	DispatcherHookInit(cond, 3, dispTypeObjectEvent, 0, spCallbacks.HezrouStenchObjEvent, 1, 0);
	DispatcherHookInit(cond, 4, dispTypeTurnBasedStatusInit, 0, spCallbacks.HezrouStenchTurnbasedStatus, 0, 0);
	DispatcherHookInit(cond, 5, dispTypeD20Query, DK_QUE_AOOPossible, spCallbacks.HezrouStenchAooPossible, 0, 0);
	DispatcherHookInit(cond, 6, dispTypeSkillLevel, 0, spCallbacks.SkillBonus, -1, -2);
	DispatcherHookInit(cond, 7, dispTypeAbilityCheckModifier, 0, spCallbacks.HezrouStenchAbilityCheckMod, -2, 345);
	DispatcherHookInit(cond, 8, dispTypeSaveThrowLevel, 0, spCallbacks.HezrouStenchSavingThrowLevel, -2, 345);
	DispatcherHookInit(cond, 9, dispTypeDealingDamage2, 0, spCallbacks.HezrouStenchDealingDamage, -2, 345);
	DispatcherHookInit(cond, 10, dispTypeToHitBonus2, 0, spCallbacks.HezrouStenchToHit2, -2, 345);
	DispatcherHookInit(cond, 11, dispTypeEffectTooltip, 0, spCallbacks.HezrouStenchEffectTooltip, 141, 0);
	DispatcherHookInit(cond, 12, dispTypeD20Signal, DK_SIG_Combat_End, spCallbacks.HezrouStenchCureNausea,0,0 );
	DispatcherHookInit(cond, 13, dispTypeD20Query, DK_QUE_Critter_Has_Condition, spCallbacks.HasCondition, (uint32_t)cond, 0);

	// Necklace of Adaptation

	auto itemForceRemoveCallback = temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x10104410);
	auto immunityCheckHandler = temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100ED650);
	auto immunityTriggerCallback = temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100ed5a0);
	cond = &mCondNecklaceOfAdaptation; 

	cond->condName = "Necklace of Adaptation";
	cond->numArgs = 4;
	DispatcherHookInit(cond, 0, dispTypeItemForceRemove, 0, itemForceRemoveCallback, 0, 0);
	DispatcherHookInit(cond, 1, dispTypeSpellImmunityCheck,0, immunityCheckHandler, 4,0);
	DispatcherHookInit(cond, 2, dispTypeImmunityTrigger, DK_IMMUNITY_SPECIAL, immunityTriggerCallback, 0x10, 0);

#pragma endregion

	static CondStructNew preferOneHanded("Prefer One Handed Wield", 1);
	preferOneHanded.AddHook(dispTypeD20Query, DK_QUE_Is_Preferring_One_Handed_Wield, genericCallbacks.PreferOneHandedWieldQuery);
	preferOneHanded.AddHook(dispTypeRadialMenuEntry, DK_NONE, genericCallbacks.PreferOneHandedWieldRadialMenu);

	//mCondCraftWandLevelSet = 
	static CondStructNew craftWandSetLev("Craft Wand Level Set", 2);
	craftWandSetLev.AddHook(dispTypeD20Query, DK_QUE_Craft_Wand_Spell_Level, QueryRetrun1GetArgs, &craftWandSetLev, 0);
	craftWandSetLev.AddHook(dispTypeRadialMenuEntry, DK_NONE, classAbilityCallbacks.FeatCraftWandRadial);

	// Aid Another
	mCondAidAnother = new CondStructNew();
	memset(mCondAidAnother, 0, sizeof(CondStructNew));
	cond = mCondAidAnother;	condName = mCondAidAnotherName;
	sprintf(condName, "Aid Another");

	cond->condName = condName;
	cond->numArgs = 8;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeRadialMenuEntry, 0, AidAnotherRadialMenu, 0, 0);
	// 
	
	/*
	char mCondIndomitableWillName[100];
	CondStructNew *mCondIndomitableWill;
	char mCondTirelessRageName[100];
	CondStructNew *mCondTirelessRage;
	char mCondMightyRageName[100];
	CondStructNew *mCondMightyRage;
	char mCondDisarmName[100];
	CondStructNew *mCondDisarm;
	char mCondImprovedDisarmName[100];
	CondStructNew *mCondImprovedDisarm;

	// monsters
	
	
	*/

	conditions.AddConditionsToTable();

}

void ConditionSystem::AddToFeatDictionary(CondStructNew* condStruct, feat_enums feat, feat_enums featEnumMax, uint32_t condArg2Offset){
	conditions.condDict[feat] = { condStruct, feat, featEnumMax, condArg2Offset };
}

void ConditionSystem::DispatcherHookInit(SubDispDefNew* sdd, enum_disp_type dispType, int key, void* callback, int data1, int data2){
	sdd->dispType = dispType;
	sdd->dispKey = key;
	sdd->dispCallback = ( int(__cdecl*)(DispatcherCallbackArgs))callback;
	sdd->data1.sVal = data1;
	sdd->data2.sVal = data2;
}

void ConditionSystem::DispatcherHookInit(CondStructNew* cond, int hookIdx, enum_disp_type dispType, int key, void* callback, int data1, int data2)
{
	if (cond->subDispDefs[hookIdx].dispType != 0)
	{
		int dummy = 1;
	}
	assert(cond->subDispDefs[hookIdx].dispType == 0);
	DispatcherHookInit(&cond->subDispDefs[hookIdx], dispType, key, callback, data1, data2 );
}

void ConditionSystem::SetPermanentModArgsFromDataFields(Dispatcher* dispatcher, CondStruct* condStruct, int* condArgs)
{
	addresses.SetPermanentModArgsFromDataFields(dispatcher, condStruct, condArgs);
}

void ConditionSystem::DispatcherCondsResetFlag2(Dispatcher* dispatcher)
{
	CondNode *condNode; 
	for (condNode = dispatcher->permanentMods; condNode; condNode = condNode->nextCondNode)
		condNode->flags &= 0xFFFFFFFD;
	for (condNode = dispatcher->itemConds; condNode; condNode = condNode->nextCondNode)
		condNode->flags &= 0xFFFFFFFD;
}

int ConditionSystem::GetActiveCondsNum(Dispatcher* dispatcher)
{ 
	int numConds=0; 

	CondNode *cond = dispatcher->conditions;
	while (cond)
	{
		if (!(cond->flags & 1))
			++numConds;
		cond = cond->nextCondNode;
	}
	return numConds;
}

int ConditionSystem::GetPermanentModsAndItemCondCount(Dispatcher* dispatcher)
{
	int numConds = 0;
	CondNode *cond = dispatcher->permanentMods;

	while (cond)
	{
		if (!(cond->flags & 1))
			++numConds;
		cond = cond->nextCondNode;
	}

	cond = dispatcher->itemConds;
	while(cond)
	{
		if (!(cond->flags & 1))
			++numConds;
		cond = cond->nextCondNode;
	}
	return numConds;
}

int ConditionSystem::ConditionsExtractInfo(Dispatcher* dispatcher, int activeCondIdx, int* hashkeyOut, int* condArgsOut)
{
	CondNode *cond;
	int n; 
	int numArgs; 


	cond = dispatcher->conditions;              
	n = 0;
	while (cond)
	{
		if (!(cond->flags & 1))
		{
			if (n == activeCondIdx)
			{
				*hashkeyOut = conds.hashmethods.GetCondStructHashkey(cond->condStruct);
				int numArgs = cond->condStruct->numArgs;
				for (int i = 0; i < numArgs; i++)
				{
					condArgsOut[i] = cond->args[i];
				}
				return cond->condStruct->numArgs;
			}
			++n;
		}
		cond = cond->nextCondNode;
	}
	if (!cond) return 0;

	*hashkeyOut = conds.hashmethods.GetCondStructHashkey(cond->condStruct);
	numArgs = cond->condStruct->numArgs;
	for (int i = 0; i < numArgs; i++)
	{
		condArgsOut[i] = cond->args[i];
	}
	return cond->condStruct->numArgs;
}

int ConditionSystem::PermanentAndItemModsExtractInfo(Dispatcher* dispatcher, int permModIdx, int* hashkeyOut, int* condArgsOut){
	
	int i=0; 
	CondNode *cond;

	cond = dispatcher->permanentMods;
	while (cond ){
		if (!(cond->flags & 1))	{
			if (i == permModIdx)	{
				*hashkeyOut = conds.hashmethods.GetCondStructHashkey(cond->condStruct);
				int numArgs = cond->condStruct->numArgs;
				for (int i = 0; i < numArgs; i++)
				{
					condArgsOut[i] = cond->args[i];
				}
				return cond->condStruct->numArgs;
			}
			++i;
		}
		cond = cond->nextCondNode;
	}


	cond = dispatcher->itemConds;
	while (cond){
		if (! (cond->flags & 1))	{
			if (i == permModIdx)	{
				*hashkeyOut = conds.hashmethods.GetCondStructHashkey(cond->condStruct);
				int numArgs = cond->condStruct->numArgs;
				for (i = 0; i < numArgs; i++)	{
					condArgsOut[i] = cond->args[i];
				}
				return cond->condStruct->numArgs;
			}
			i++;
		}
		cond = cond->nextCondNode;
	}

	return 0;
}

void ConditionSystem::ConditionRemove(objHndl objHnd, CondNode* cond)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (dispatch.dispatcherValid(dispatcher))
	{
		dispatch.DispatchConditionRemove(dispatcher, cond);
	}
}

int* ConditionSystem::CondNodeGetArgPtr(CondNode* condNode, uint32_t argIdx)
{
	if (argIdx < condNode->condStruct->numArgs)
		return (int*)&condNode->args[argIdx];
	return 0;
}

#pragma region NewConditionCallbacks
 int __cdecl AoODisableRadialMenuInit(DispatcherCallbackArgs args)
{
	RadialMenuEntry radEntry;
	radEntry.maxArg = 1;
	radEntry.minArg = 0;
	radEntry.type = RadialMenuEntryType::Toggle;
	radEntry.actualArg = (int)conds.CondNodeGetArgPtr(args.subDispNode->condNode, 0);
	radEntry.callback = (BOOL (__cdecl*)(objHndl, RadialMenuEntry*))temple::GetPointer(0x100F0200);
	MesLine mesLine;
	mesLine.key = 5105; //disable AoOs
	if (!mesFuncs.GetLine(*combatSys.combatMesfileIdx, &mesLine) )
	{
		//sprintf((char*)temple::GetPointer(0x10EEE228), "Disable Attacks of Opportunity");
		mesLine.value = conds.mConditionDisableAoOName;
	}
	//mesFuncs.GetLine_Safe(*combatSys.combatMesfileIdx, &mesLine);
	radEntry.text = (char*)mesLine.value;
	radEntry.helpId = conds.hashmethods.StringHash("TAG_RADIAL_MENU_DISABLE_AOOS");
	int parentNode = radialMenus.GetStandardNode(RadialMenuStandardNode::Options);
	radialMenus.AddChildNode(args.objHndCaller, &radEntry, parentNode);
	return 0;
}

int __cdecl AoODisableQueryAoOPossible(DispatcherCallbackArgs args)
{
	DispIoD20Query * dispIo = dispatch.DispIoCheckIoType7((DispIoD20Query*)args.dispIO);
	if (dispIo->return_val && conds.CondNodeGetArg(args.subDispNode->condNode, 0))
	{
		dispIo->return_val = 0;
	}
	return 0;
}

int __cdecl GreaterTwoWeaponFighting(DispatcherCallbackArgs args)
{
	DispIoD20ActionTurnBased *dispIo = dispatch.DispIoCheckIoType12((DispIoD20ActionTurnBased*)args.dispIO);
	objHndl mainWeapon = inventory.ItemWornAt(args.objHndCaller, 3);
	objHndl offhand = inventory.ItemWornAt(args.objHndCaller, 4);
	
	if (mainWeapon != offhand)
	{
		if (mainWeapon)
		{
			if (offhand)
			{
				int weapFlags = objects.getInt32(mainWeapon, obj_f_weapon_flags);
				if (!(weapFlags & (4<<8)) && objects.getInt32(offhand, obj_f_type) != obj_t_armor)
					++dispIo->returnVal;
			}
		}
	}
	return 0;

}

int __cdecl GreaterTWFRanger(DispatcherCallbackArgs args)
{
	if (!critterSys.IsWearingLightOrNoArmor(args.objHndCaller))
	{
		return 0;
	}
	return GreaterTwoWeaponFighting(args);
};

int __cdecl TwoWeaponFightingBonus(DispatcherCallbackArgs args)
{
	DispIoAttackBonus *dispIo = dispatch.DispIoCheckIoType5((DispIoAttackBonus*)args.dispIO);
	char *featName;

	feat_enums feat = (feat_enums)conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	int attackCode = dispIo->attackPacket.dispKey;
	int dualWielding = 0;
	int attackNumber = 1;
	if (d20Sys.UsingSecondaryWeapon(args.objHndCaller, attackCode))
	{
		featName = feats.GetFeatName(feat);
		bonusSys.bonusAddToBonusListWithDescr(&dispIo->bonlist, 6, 0, 114, featName);
	}
		else if ( d20Sys.ExtractAttackNumber(args.objHndCaller, attackCode, &attackNumber, &dualWielding), dualWielding != 0)
	{
		featName = feats.GetFeatName(feat);
		bonusSys.bonusAddToBonusListWithDescr(&dispIo->bonlist, 2, 0, 114, featName);
	}
	return 0;
}

int TwoWeaponFightingBonusRanger(DispatcherCallbackArgs args)
{
	DispIoAttackBonus * dispIo = dispatch.DispIoCheckIoType5((DispIoAttackBonus*)args.dispIO);
	if ( !critterSys.IsWearingLightOrNoArmor(args.objHndCaller))
	{
		bonusSys.zeroBonusSetMeslineNum(&dispIo->bonlist, 166);
		return 0;
	}
	
	
	feat_enums feat = FEAT_TWO_WEAPON_FIGHTING;
	char * featName;
	int attackCode = dispIo->attackPacket.dispKey;
	int dualWielding = 0;
	int attackNumber = 1;
	if (d20Sys.UsingSecondaryWeapon(args.objHndCaller, attackCode))
	{
		featName = feats.GetFeatName(feat);
		bonusSys.bonusAddToBonusListWithDescr(&dispIo->bonlist, 6, 0, 114, featName);
	}
	else if (d20Sys.ExtractAttackNumber(args.objHndCaller, attackCode, &attackNumber, &dualWielding), dualWielding != 0)
	{
		featName = feats.GetFeatName(feat);
		bonusSys.bonusAddToBonusListWithDescr(&dispIo->bonlist, 2, 0, 114, featName);
	}
	return 0;

}



int __cdecl DivineMightRadial(DispatcherCallbackArgs args)
{
	if (d20Sys.d20Query(args.objHndCaller, DK_QUE_IsFallenPaladin))
		return 0;

	RadialMenuEntryAction radEntry(5106, D20A_DIVINE_MIGHT, 0, ElfHash::Hash("TAG_DIVINE_MIGHT"));
	radEntry.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::Feats);
	return 0;
}


int __cdecl DivineMightDamageBonus(DispatcherCallbackArgs args)
{
	DispIoDamage * dispIo = dispatch.DispIoCheckIoType4((DispIoDamage*)args.dispIO);
	char * desc = feats.GetFeatName(FEAT_DIVINE_MIGHT);
	int damBonus = conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	damage.AddDamageBonusWithDescr(&dispIo->damage, damBonus, 0, 114, desc);
	return 0;
}


int GreaterWeaponSpecializationDamage(DispatcherCallbackArgs args)
{
	int weaponType; 
	char *featName; 

	feat_enums feat = (feat_enums)conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	WeaponTypes wpnTypeFromCond = (WeaponTypes)conds.CondNodeGetArg(args.subDispNode->condNode, 1);
	DispIoDamage * dispIo = dispatch.DispIoCheckIoType4(args.dispIO);
	objHndl weapon = combatSys.GetWeapon(&dispIo->attackPacket);
	if (weapon)
		weaponType = objects.GetWeaponType(weapon);
	else
		weaponType = 1;
	if (weaponType == wpnTypeFromCond)
	{
		featName = feats.GetFeatName(feat);
		damage.AddDamageBonusWithDescr(&dispIo->damage, 2, 0, 114, featName);
	}
	return 0;
}

int __cdecl RecklessOffenseRadialMenuInit(DispatcherCallbackArgs args)
{
	RadialMenuEntry radEntry;
	radEntry.maxArg = 1;
	radEntry.minArg = 0;
	radEntry.type = RadialMenuEntryType::Toggle;
	radEntry.actualArg = (int)conds.CondNodeGetArgPtr(args.subDispNode->condNode, 0);
	radEntry.callback = (BOOL(__cdecl*)(objHndl, RadialMenuEntry*))temple::GetPointer(0x100F0200);
	MesLine mesLine;
	mesLine.key = 5107; // reckless offense
	if (!mesFuncs.GetLine(*combatSys.combatMesfileIdx, &mesLine))
	{
		//sprintf((char*)temple::GetPointer(0x10EEE228), "Reckless Offense");
		mesLine.value = conds.mCondRecklessOffenseName;
	};
	radEntry.text = (char*)mesLine.value;
	radEntry.helpId = ElfHash::Hash("TAG_FEAT_RECKLESS_OFFENSE");
	int parentNode = radialMenus.GetStandardNode(RadialMenuStandardNode::Feats);
	radialMenus.AddChildNode(args.objHndCaller, &radEntry, parentNode);
	return 0;
}

int RecklessOffenseAcPenalty(DispatcherCallbackArgs args)
{
	if (conds.CondNodeGetArg(args.subDispNode->condNode, 0))
	{
		if (conds.CondNodeGetArg(args.subDispNode->condNode, 1))
		{
			DispIoAttackBonus* dispIo = dispatch.DispIoCheckIoType5(args.dispIO);
			BonusList* bonlist = &dispIo->bonlist;
			bonusSys.bonusAddToBonusList(bonlist, -4, 8, 337);
		}
	}
	return 0;
}

int RecklessOffenseToHitBonus(DispatcherCallbackArgs args)
{
	if (conds.CondNodeGetArg(args.subDispNode->condNode, 0))
	{
		DispIoAttackBonus* dispIo = dispatch.DispIoCheckIoType5(args.dispIO);
		if (!(dispIo->attackPacket.flags & D20CAF_RANGED))
			bonusSys.bonusAddToBonusList(&dispIo->bonlist, 2, 0, 337);
	}
	return 0;
}

int TacticalOptionAbusePrevention(DispatcherCallbackArgs args)
{ // signifies that an attack has been made using that tactical option (so user doesn't toggle it off and shrug off the penalties)
	DispIoD20Signal * dispIo = dispatch.DispIoCheckIoType6(args.dispIO);
	if (!(*(char*)(dispIo->data1 + 32) & 4))
		_CondNodeSetArg(args.subDispNode->condNode, 1, 1);
	return 0;
}

#pragma region Barbarian Stuff

int __cdecl CombatExpertiseRadialMenu(DispatcherCallbackArgs args)
{
	int bab = dispatch.DispatchToHitBonusBase(args.objHndCaller, 0);
	if (bab > 0)
	{
		auto maxArg = feats.HasFeatCount(args.objHndCaller, FEAT_SUPERIOR_EXPERTISE)? bab:min(5, bab);
		RadialMenuEntrySlider radEntry(5007,0, maxArg, args.GetCondArgPtr(0), -1, ElfHash::Hash("TAG_COMBAT_EXPERTISE") );
		int parentNode = radialMenus.GetStandardNode(RadialMenuStandardNode::Feats);
		radialMenus.AddChildNode(args.objHndCaller, &radEntry, parentNode);
	}
	return 0;
}

int CombatExpertiseSet(DispatcherCallbackArgs args)
{
	int bab = dispatch.DispatchToHitBonusBase(args.objHndCaller, 0);
	if (bab > 5 && !feats.HasFeatCount(args.objHndCaller, FEAT_SUPERIOR_EXPERTISE))
		bab = 5;
	DispIoD20Signal * dispIo = dispatch.DispIoCheckIoType6(args.dispIO);
	int bonus = dispIo->data1;
	if (bonus > bab)
		bonus = bab;
	if (bonus < 0)
		bonus = 0;
	conds.CondNodeSetArg(args.subDispNode->condNode, 0, bonus);
	return 0;
}

int BarbarianRageStatBonus(DispatcherCallbackArgs args)
{
	DispIoBonusList * dispIo = dispatch.DispIoCheckIoType2(args.dispIO);
	if (feats.HasFeatCountByClass(args.objHndCaller, FEAT_MIGHTY_RAGE, (Stat)0, 0)) {
		int nBonus = 8;
		nBonus += d20Sys.D20QueryPython(args.objHndCaller, "Additional Rage Stat Bonus");
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, nBonus, 0, 339); // Greater Rage
	}
	else if (feats.HasFeatCountByClass(args.objHndCaller, FEAT_GREATER_RAGE, (Stat)0, 0)) {
		int nBonus = 6;
		nBonus += d20Sys.D20QueryPython(args.objHndCaller, "Additional Rage Stat Bonus");
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, nBonus, 0, 338); // Greater Rage
	}
	else {
		int nBonus = 4;
		nBonus += d20Sys.D20QueryPython(args.objHndCaller, "Additional Rage Stat Bonus");
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, nBonus, 0, 195); // normal rage
	}
	return 0;
}

int BarbarianRageACPenalty(DispatcherCallbackArgs args)
{
	DispIoAttackBonus * dispIo = dispatch.DispIoCheckIoType5(args.dispIO);

	int nPenalty = -2;

	//Value needs to be negated (it is a penalty) since qureies can't return negative values
	nPenalty += -1 * d20Sys.D20QueryPython(args.objHndCaller, "Additional Rage AC Penalty");

	bonusSys.bonusAddToBonusList(&dispIo->bonlist, nPenalty, 0, 195);  //rage ac penalty
	return 0;
}

int BarbarianRageSaveBonus(DispatcherCallbackArgs args)
{
	DispIoSavingThrow * dispIo = dispatch.DispIoCheckIoType3(args.dispIO);
	if (feats.HasFeatCountByClass(args.objHndCaller, FEAT_MIGHTY_RAGE, (Stat)0, 0))
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 4, 0, 339); // Mighty Rage
	else if (feats.HasFeatCountByClass(args.objHndCaller, FEAT_GREATER_RAGE, (Stat)0, 0))
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 3, 0, 338); // Greater Rage
	else
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 2, 0, 195); // normal rage

	if (dispIo->flags & 0x100) // enchantment
	{
		if (feats.HasFeatCountByClass(args.objHndCaller, FEAT_INDOMITABLE_WILL, (Stat)0, 0))
			bonusSys.bonusAddToBonusList(&dispIo->bonlist, 4, 0, 344); // Indomitable Will
	}
	return 0;
}

int BarbarianDamageResistance(DispatcherCallbackArgs args)
{
	DamagePacket *damagePacket; 
	int barbLvl; 
	int damRes = 0;

	damagePacket = &dispatch.DispIoCheckIoType4(args.dispIO)->damage;
	barbLvl = objects.StatLevelGet(args.objHndCaller, stat_level_barbarian);
	if (barbLvl >= 7)
	{
		damRes = 1 + (barbLvl - 7) / 3;
		damage.AddPhysicalDR(damagePacket, damRes, 1, 126u);
	}
	
	return 0;
}


void __cdecl BarbarianTirelessRageCheck(objHndl obj)
{
	if (objects.StatLevelGet(obj, stat_level_barbarian) < 17)
		conds.AddTo(obj, "Barbarian_Fatigued", {0,0});
};

// Barbarian Tireless Rage patch
class BarbarianTirelessRagePatch : public TempleFix
{
public:
	void apply() override {
		redirectCall(0x100EADBF, BarbarianTirelessRageCheck);
	}

} barbarianTirelessRagePatch;

#pragma endregion

int __cdecl DisarmRadialMenu(DispatcherCallbackArgs args){

	RadialMenuEntryAction radEntry(5109, D20A_DISARM, 0, ElfHash::Hash("TAG_DISARM"));
	radEntry.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::Offense);
	return 0;
}

int __cdecl DisarmHpChanged(DispatcherCallbackArgs args)
{
	DispIoD20Signal * dispIo = dispatch.DispIoCheckIoType6(args.dispIO);
	int hpChange = dispIo->data1;
	if (hpChange < 0)
	{
		if (conds.CondNodeGetArg(args.subDispNode->condNode, 1) )
			conds.CondNodeSetArg(args.subDispNode->condNode, 0, 1);
	}
	return 0;
};


int __cdecl DisarmQueryAoOResetArg(DispatcherCallbackArgs args)
{
	DispIoD20Query * dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	D20Actn * d20a = (D20Actn*)dispIo->data1;
	if (d20a->d20ActType == D20A_DISARM)
	{
		if (!feats.HasFeatCountByClass(args.objHndCaller, FEAT_IMPROVED_DISARM))
			dispIo->return_val = 1;
		conds.CondNodeSetArg(args.subDispNode->condNode, args.subDispNode->subDispDef->data1,
			args.subDispNode->subDispDef->data2);
	}
		
	// sets arg[data1] from data2  
	// e.g. IF data1 = 0, data2 = 15 
	//    THEN it'll set arg0 = 15
	return 0;
};

int __cdecl DisarmCanPerform(DispatcherCallbackArgs args)
{
	DispIoD20Query * dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	conds.CondNodeSetArg(args.subDispNode->condNode, 1, 0);
	if (conds.CondNodeGetArg(args.subDispNode->condNode, 0) == 0)
	{
		dispIo->return_val = 1;
	}
	conds.CondNodeSetArg(args.subDispNode->condNode, 0, 0);
	return 0;
};

int DisarmedReminder(DispatcherCallbackArgs args)
{
	if (args.subDispNode->condNode->args[7] < 2 && party.IsInParty(args.objHndCaller) 
		&& !critterSys.IsDeadOrUnconscious(args.objHndCaller))
	{
		char blargh[1000];
		memcpy(blargh, "I was disarmed.", sizeof("I was disarmed."));
		uiDialog.ShowTextBubble(args.objHndCaller, args.objHndCaller, { blargh }, -1);
		args.subDispNode->condNode->args[7]++;
	}
	return 0;
}

int DisarmedOnAdd(DispatcherCallbackArgs args)
{
	objHndl weapon;
	((int*)&weapon)[0] = conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	((int*)&weapon)[1] = conds.CondNodeGetArg(args.subDispNode->condNode, 1);
	ObjectId objId = objects.GetId(weapon);
	memcpy(args.subDispNode->condNode->args, &objId, sizeof(ObjectId));
	return 0;
}

int DisarmedWeaponRetrieve(DispatcherCallbackArgs args)
{
	objHndl weapon = objHndl::null;
	DispIoD20Signal * dispIo = dispatch.DispIoCheckIoType6(args.dispIO);
	D20Actn* d20a = (D20Actn*)dispIo->data1;
	if (d20a->d20ATarget && objects.GetType(d20a->d20ATarget) == obj_t_weapon)
	{
		weapon = d20a->d20ATarget;
	}
	else
	{
		ObjectId objId;
		memcpy(&objId, args.subDispNode->condNode->args, sizeof(ObjectId));
		weapon = gameSystems->GetObj().GetHandleById(objId);
	}
	if (!weapon || (inventory.GetParent(weapon) && combatSys.isCombatActive())|| objects.GetType(weapon) != obj_t_weapon)
	{
		objects.floats->FloatCombatLine(args.objHndCaller, 195); //fail!
		if (args.subDispNode->condNode->args[6] < 2)
			args.subDispNode->condNode->args[6]++;
		else
			conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0; 
	}

	//((int*)&weapon)[0]= conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	//((int*)&weapon)[1] = conds.CondNodeGetArg(args.subDispNode->condNode, 1);
	if (!inventory.ItemWornAt(args.objHndCaller, 203))
		inventory.ItemGetAdvanced(weapon, args.objHndCaller, 203, 0);
	else
		inventory.ItemGetAdvanced(weapon, args.objHndCaller, -1, 0);
	objects.floats->FloatCombatLine(args.objHndCaller, 201);
	conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
	return 0;
};

int DisarmedRetrieveWeaponRadialMenu(DispatcherCallbackArgs args){

	RadialMenuEntryAction radEntry(5111, D20A_DISARMED_WEAPON_RETRIEVE, 0, "TAG_DISARM");
	radEntry.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::Items);
	return 0;
}

int __cdecl SunderRadialMenu(DispatcherCallbackArgs args){

	RadialMenuEntryAction radEntry(5110, D20A_SUNDER, 0, "TAG_SUNDER");
	radEntry.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::Offense);
	return 0;
}



int RendOnDamage(DispatcherCallbackArgs args)
{
	DispIoDamage * dispIo = dispatch.DispIoCheckIoType4(args.dispIO);
	objHndl weapon = combatSys.GetWeapon(&dispIo->attackPacket);
	if (weapon)
		return 0;
	
	

	DamagePacket * dmgPacket = &dispIo->damage;
	auto attackDescr = dmgPacket->dice[0].typeDescription; // e.g. Claw etc.
	auto hasDeliveredDamage = args.GetCondArg(0);
	auto previousAttackDescr = (char*)args.GetCondArg(1);
	auto previousTarget = args.GetCondArgObjHndl(2);
	if (hasDeliveredDamage && attackDescr == previousAttackDescr && previousTarget == dispIo->attackPacket.victim)
	{
		Dice dice(2, 6, 9);
		dispIo->damage.AddDamageDice(dice.ToPacked(), DamageType::PiercingAndSlashing, 133);
		//damage.AddDamageDice(&dispIo->damage, dice.ToPacked(), DamageType::PiercingAndSlashing, 133);
		floatSys.FloatCombatLine(args.objHndCaller, 203);
		conds.CondNodeSetArg(args.subDispNode->condNode, 0, 0);
	}
	else
	{
		args.SetCondArg(0, 1);
		args.SetCondArg(1, (int)attackDescr);
		args.SetCondArgObjHndl(2, dispIo->attackPacket.victim);
	}
		
	return 0;
}


int ClassAbilityCallbacks::FeatCraftWondrousRadial(DispatcherCallbackArgs args){
	return ItemCreationBuildRadialMenuEntry(args, CraftWondrous, "TAG_CRAFT_WONDROUS", 5070);
};

int ClassAbilityCallbacks::FeatCraftStaffRadial(DispatcherCallbackArgs args){
	return ItemCreationBuildRadialMenuEntry(args, CraftStaff, "TAG_CRAFT_STAFF", 5103);
};

int ClassAbilityCallbacks::FeatForgeRingRadial(DispatcherCallbackArgs args){
	return ItemCreationBuildRadialMenuEntry(args, ForgeRing, "TAG_FORGE_RING", 5104);
};

int ClassAbilityCallbacks::FeatCraftMagicArmsAndArmorRadial(DispatcherCallbackArgs args){
	return ItemCreationBuildRadialMenuEntry(args, CraftMagicArmsAndArmor, "TAG_CRAFT_MAA", 5071);
}

int ConditionFunctionReplacement::LayOnHandsPerform(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType12(args.dispIO);
	auto d20a = dispIo->d20a;
	bool animResult = false;
	if (critterSys.IsUndead(d20a->d20ATarget)){
		d20a->d20Caf |= D20CAF_TOUCH_ATTACK;
		if (d20a->d20Caf & D20CAF_RANGED)
			return 0;
		d20Sys.ToHitProc(d20a);
		animResult = gameSystems->GetAnim().PushAttemptAttack(d20a->d20APerformer, d20a->d20ATarget) != 0;
	} else	{
		animResult = gameSystems->GetAnim().PushAnimate(d20a->d20APerformer, 86) != 0;
	}

	
	if (animResult)
	{
		// fixes lack of animation ID
		d20a->animID = gameSystems->GetAnim().GetActionAnimId(d20a->d20APerformer);
		d20a->d20Caf |= D20CAF_NEED_ANIM_COMPLETED;
	}
		

	return 0;
}

int ConditionFunctionReplacement::RemoveDiseasePerform(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType12(args.dispIO);
	auto d20a = dispIo->d20a;
	auto animResult = gameSystems->GetAnim().PushAnimate(d20a->d20APerformer, 86);
	
	if (animResult){
		// fixes lack of animation ID
		d20a->animID = gameSystems->GetAnim().GetActionAnimId(d20a->d20APerformer);
		d20a->d20Caf |= D20CAF_NEED_ANIM_COMPLETED;
	}
	return 0;
}

void ConditionFunctionReplacement::HookSpellCallbacks()
{

	replaceFunction(0x100D3100, SpellCallbacks::ConcentratingActionSequenceHandler);
	replaceFunction(0x100D32B0, SpellCallbacks::ConcentratingActionRecipientHandler);

	// QueryCritterHasCondition for sp-Spiritual Weapon
	int writeVal = dispTypeD20Query;
	SubDispDefNew sdd;
	sdd.dispType = dispTypeD20Query;
	sdd.data1.sVal = 0x102DFC00;
	sdd.data2.usVal = 0;
	sdd.dispCallback = QueryCritterHasCondition;
	sdd.dispKey = DK_QUE_Critter_Has_Condition;
	write(0x102DFD48, &sdd, sizeof(SubDispDefNew));

	// QueryCritterHasCondition for sp-Sleep
	sdd.dispType = dispTypeD20Query;
	sdd.data1.sVal = 0x102DEB08;
	sdd.data2.usVal = 0;
	sdd.dispCallback = QueryCritterHasCondition;
	sdd.dispKey = DK_QUE_Critter_Has_Condition;
	write(0x102DEC00, &sdd, sizeof(SubDispDefNew));

	//  DK_SIG_AID_ANOTHER_WAKE_UP for sp-Sleep
	sdd.dispType = dispTypeD20Signal;
	sdd.data1.usVal = 0;
	sdd.data2.usVal = 0;
	sdd.dispCallback = RemoveSpellConditionAndMod;
	sdd.dispKey = DK_SIG_AID_ANOTHER_WAKE_UP;
	write(0x102DEB9C, &sdd, sizeof(SubDispDefNew)); // overwriting S_Teleport_Reconnect since it does nothing (return_0 callback)


	// EffectTooltip for Stinking Cloud
	sdd.dispType = dispTypeEffectTooltip;
	sdd.dispKey = 0;
	sdd.data1.usVal = 141;
	sdd.data2.usVal = 0;
	sdd.dispCallback = [](DispatcherCallbackArgs args)->int
	{
		auto dispIo = dispatch.DispIoCheckIoType24(args.dispIO);
		auto remainingDuration = args.GetCondArg(1);
		auto spellId = args.GetCondArg(0);
		SpellPacketBody spellPkt(spellId);

		auto text = fmt::format("\n {}: {}/{}", combatSys.GetCombatMesLine(175), remainingDuration, spellPkt.duration);
		dispIo->Append(args.GetData1(), spellPkt.spellEnum, text.c_str());
		return 0;
	};
	write(0x102DFF50, &sdd, sizeof(SubDispDefNew));

	// Radial menu Spell Dismiss options
	{
		SubDispDefNew subd;
		subd.dispKey = DK_NONE;
		subd.dispType = dispTypeRadialMenuEntry;
		subd.dispCallback = spCallbacks.SpellDismissRadialSub;
		write(0x102E6988, &subd, sizeof(subd));

		// disable the generic "Dismiss Spell" entry
		replaceFunction<int(DispatcherCallbackArgs)>(0x100EEDD0, [](DispatcherCallbackArgs){
			return 0;
		});

		// prevent dups
		replaceFunction<int(DispatcherCallbackArgs)>(0x100CBD60, spCallbacks.SpellAddDismissCondition);

		// signal handler
		replaceFunction<int(DispatcherCallbackArgs)>(0x100DE3F0, spCallbacks.SpellDismissSignalHandler);
		replaceFunction<int(DispatcherCallbackArgs)>(0x100E95C0, SpellCallbacks::DismissSignalHandler);
	}

	
	
}


bool ConditionFunctionReplacement::StunningFistHook(objHndl objHnd, objHndl caster, int DC, int saveType, int flags)
{

	//Preform the saving throw and return the result
	bool result = damage.SavingThrow(objHnd, caster, DC, static_cast<SavingThrowType>(saveType), flags);

	if (!result) {
		//On a failed saving throw send the signal that a stunning fist effect was applied
		dispatch.DispatchSpecialAttack(caster, static_cast<int>(EvtObjSpecialAttack::STUNNING_FIST), objHnd);
	}

	return result;
}


int ConditionFunctionReplacement::TurnUndeadHook(objHndl handle, Stat shouldBeClassCleric, DispIoD20ActionTurnBased * evtObj){

	auto turnType = evtObj->d20a->data1;
	auto result = objects.StatLevelGet(handle, stat_level_cleric); // the vanilla code we're replacing did this

	result += d20Sys.D20QueryPython(handle, "Turn Undead Level", turnType);

	return result;
}

int ConditionFunctionReplacement::TurnUndeadPerform(DispatcherCallbackArgs args)
{
	auto dispIo = static_cast<DispIoD20ActionTurnBased*>(args.dispIO);
	dispIo->AssertType(dispIOTypeD20ActionTurnBased);

	auto turnType = args.GetCondArg(0);

	auto result = condFuncReplacement.oldTurnUndeadPerform(args);  //Just call the old version now

	// Send the signal if this was the turn type used
	if (dispIo->d20a->data1 == turnType) {
		d20Sys.D20SignalPython(args.objHndCaller, "Turn Undead Perform", turnType);
	}

	return result;
}

int ConditionFunctionReplacement::TurnUndeadCheck(DispatcherCallbackArgs args)
{
	auto dispIo = static_cast<DispIoD20ActionTurnBased*>(args.dispIO);
	dispIo->AssertType(dispIOTypeD20ActionTurnBased);

	auto d20a = dispIo->d20a;
	auto turnType = args.GetCondArg(0);

	if (turnType == d20a->data1) {
		auto charges = args.GetCondArg(1);

		// Check if the turn undead ability has been disabled in python
		auto result = d20Sys.D20QueryPython(args.objHndCaller, "Turn Undead Disabled");
		if (result > 0) {
			dispIo->returnVal = dispIo->returnVal = AEC_INVALID_ACTION;
		} else {
			if (charges > 0) {
				dispIo->returnVal = 0;
			}
			else {
				dispIo->returnVal = dispIo->returnVal = AEC_OUT_OF_CHARGES;
			}
		}
	}

	return 0;
}

#pragma region Spell Callbacks

int SpellCallbacks::ArmorSpellFailure(DispatcherCallbackArgs args){

	GET_DISPIO(dispIOTypeQuery, DispIoD20Query);

	if (dispIo->return_val){
		return 0;
	}

	auto d20SpellData = (D20SpellData*)(dispIo->data1);
	int spellEnum, spellClass, spellLvl, invIdx;
	MetaMagicData mmData;
	d20SpellData->Extract(&spellEnum, nullptr, &spellClass, &spellLvl, &invIdx, &mmData);

	if (spellSys.isDomainSpell(spellClass))
		return 0;

	SpellStoreData spData(spellEnum, spellLvl, spellClass, mmData);
	if (!(spData.GetSpellComponentFlags() & SpellComponent_Somatic))
		return 0;

	if (!spellSys.IsArcaneSpellClass(spellClass))
		return 0;

	auto classCode = spellSys.GetCastingClass(spellClass);
	auto failChance = 0;
	for (auto i = (int)EquipSlot::Helmet; i < EquipSlot::Count; i++){
		auto itemFailChance = d20Sys.d20QueryWithData(args.objHndCaller, DK_QUE_Get_Arcane_Spell_Failure, classCode, i);
		if (itemFailChance > 0)
			failChance += itemFailChance;
	}

	if (failChance <= 0)
		return 0;

	auto rollRes = Dice::Roll(1, 100);
	if (rollRes <= failChance){
		floatSys.FloatCombatLine(args.objHndCaller, 57); // Miscast (Armor)!
		dispIo->return_val = 1;
		auto histId = histSys.RollHistoryType5Add(args.objHndCaller, objHndl::null, failChance, 59, rollRes, 57, 192); // Arcane Spell Failure due to Armor
		histSys.CreateRollHistoryString(histId);
		histSys.CreateRollHistoryLineFromMesfile(29, args.objHndCaller, objHndl::null); // [ACTOR] ~loses spell~[TAG_ARCANE_SPELL_FAILURE] due to armor.
		return 0;
	}

	auto histId = histSys.RollHistoryType5Add(args.objHndCaller, objHndl::null, failChance, 59, rollRes, 62, 192); // Arcane Spell Failure due to Armor
	histSys.CreateRollHistoryString(histId);

	return 0;
}

int SpellCallbacks::SkillBonus(DispatcherCallbackArgs args){

	SkillEnum skillEnum = (SkillEnum)args.GetData1();
	int bonValue = args.GetData2();

	auto spellId = args.GetCondArg(0);
	SpellPacketBody spellPkt(spellId);

	int bonType = 0; // will stack if 0

	if (args.dispKey == skillEnum + 20 || skillEnum == -1) {
		auto dispIo = dispatch.DispIoCheckIoType10((DispIoObjBonus*)args.dispIO);
		auto spellName = spellSys.GetSpellName(spellPkt.spellEnum);
		dispIo->bonOut->AddBonusWithDesc(bonValue, bonType, 113, (char*)spellName); // 113 is ~Spell~[TAG_SPELLS] in bonus.mes
	}
	return 0;
}

int SpellCallbacks::BeginHezrouStench(DispatcherCallbackArgs args){

	auto spellId = args.GetCondArg(0);
	SpellPacketBody spellPkt(spellId);
	if (!spellId)
		return 0;

	SpellEntry spellEntry(spellPkt.spellEnum);

	auto evtId = objEvents.EventAppend(args.objHndCaller, 0, 1, OLC_CRITTERS, 12.0f * spellEntry.radiusTarget, 0.0, XM_PI * 2);

	args.SetCondArg(2, evtId);
	if (!spellPkt.UpdateSpellsCastRegistry()){
		logger->warn("BeginHezrouStench: Unable to update spell cast registry!");
		return 0;
	}

	spellPkt.spellObjs[0].obj = spellPkt.aoeObj;
	spellPkt.spellObjs[0].partySysId = args.GetCondArg(3);
	spellPkt.UpdateSpellsCastRegistry();
	pySpellIntegration.UpdateSpell(spellPkt.spellId);


	return 0;
}

int SpellCallbacks::ConcentratingActionSequenceHandler(DispatcherCallbackArgs args){
	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
	auto actSeq = (ActnSeq*)dispIo->data1;
	if (!actSeq)
		return 1;
	auto spellId = args.GetCondArg(0);
	SpellPacketBody spellPkt(spellId);
	if (!spellPkt.spellEnum){
		logger->debug("Cannot fetch spell packet");
		return 0;
	}

	if (spellPkt.spellEnum == 303){ // Meld Into Stone hardcoding...
		return 0;
	}

	for (auto i=0; i < actSeq->d20ActArrayNum; i++){
		auto &d20a = actSeq->d20ActArray[i];
		auto actionFlags = d20Sys.GetActionFlags(d20a.d20ActType);
		if (!(actionFlags & D20ADF::D20ADF_Breaks_Concentration))
			continue;
		if (d20a.d20ActType == D20A_CAST_SPELL && d20a.spellId == spellId)
			break;
		if (d20a.d20Caf & D20CAF_FREE_ACTION) // added in Temple+ - free actions won't take up your standard action
			continue;
		DispatcherCallbackArgs dca;
		dca.dispIO = nullptr;
		dca.dispType = dispTypeD20Signal;
		dca.dispKey = DK_SIG_Remove_Concentration;
		dca.subDispNode = args.subDispNode;
		dca.objHndCaller = args.objHndCaller;
		SpellRemoveMod(dca);
	}
	return 0;
}

int SpellCallbacks::ConcentratingActionRecipientHandler(DispatcherCallbackArgs args)
{
	if (!args.dispIO){
		return args.dispType == dispTypeBeginRound;
	}

	if (args.dispKey == DK_SIG_Killed){
		return 1;
	}
		
	if (args.dispIO->dispIOType != enum_dispIO_type::dispIoTypeSendSignal)
		return 0;

	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
	auto d20a = (D20Actn*)dispIo->data1;
	auto spellId = args.GetCondArg(0);
	if (!d20a)
		return 1;

	if (d20a->d20ActType != D20A_CAST_SPELL)
		return 0;

	auto d20aSpellId = d20a->spellId;
	if (spellId == d20aSpellId)
		return 0;

	SpellPacketBody spellPkt(d20aSpellId);
	if (!spellPkt.spellEnum){
		logger->debug("ConcentratingActionRecipientHandler: error, unable to retrieve spell packet");
		return 0;
	}

	SpellPacketBody conceSpellPkt(spellId);
	if (!conceSpellPkt.spellEnum) {
		logger->debug("ConcentratingActionRecipientHandler: error, unable to retrieve spell packet of concentrated spell");
		return 0;
	}

	if (conceSpellPkt.spellEnum == 303) //meld into stone hack
	{
		return 0;
	}

	if (skillSys.SkillRoll(args.objHndCaller, SkillEnum::skill_concentration, spellPkt.dc,nullptr, 1)){
		return 0;
	}

	histSys.CreateRollHistoryLineFromMesfile(32, args.objHndCaller, objHndl::null);
	combatSys.FloatCombatLine(args.objHndCaller, 54);
	auto args2 = args;
	args2.dispKey = DK_SIG_Remove_Concentration;
	args2.dispIO = nullptr;
	args2.dispType = enum_disp_type::dispTypeD20Signal;
	args2.RemoveSpellMod();


	return 0;
}

int SpellCallbacks::EnlargePersonWeaponDice(DispatcherCallbackArgs args)
{
	args.dispIO->AssertType(dispIOType20);
	auto dispIo = static_cast<DispIoAttackDice*>(args.dispIO);


	if (!dispIo->weapon)
		return 0;

	auto dice = Dice::FromPacked(dispIo->dicePacked);
	auto diceCount = dice.GetCount();
	auto diceSide = dice.GetSides();
	auto diceMod = dice.GetModifier();


	// get wield type
	auto weaponUsed = dispIo->weapon;
	auto wieldType = inventory.GetWieldType(args.objHndCaller, weaponUsed, true);
	auto wieldTypeWeaponModified = inventory.GetWieldType(args.objHndCaller, weaponUsed, false); // the wield type if the weapon is not enlarged along with the critter

	// check offhand
	auto offhandWeapon = inventory.ItemWornAt(args.objHndCaller, EquipSlot::WeaponSecondary);
	auto shield = inventory.ItemWornAt(args.objHndCaller, EquipSlot::Shield);
	auto regardOffhand = offhandWeapon || shield && !inventory.IsBuckler(shield) ? true : false;


	bool enlargeWeapon = true; // by default enlarge the weapon
	// case 1
	switch (wieldType)
	{
	case 0: // light weapon
		switch (wieldTypeWeaponModified)
		{
		case 2: // shouldn't really be possible, but just in case...
			if (regardOffhand)
				enlargeWeapon = false;
			break;		
		default:
			break;
		}
		break;
	case 1: // single handed wield if weapon is unaffected
		switch (wieldTypeWeaponModified)
		{
		case 0: // only in reduce person; going to assume the "beneficial" case that the reduction was made voluntarily and hence you let the weapon stay larger
		case 1: // weapon can be enlarged
			break;
		case 2: // this is the main case - weapon gets enlarged along with the character so it's now a THW
			if (regardOffhand) // if holding something in offhand, hold off on increasing the damage
				enlargeWeapon = false;
			break;
		default:
			break;
		}
		break;
	case 2: // two handed wield if weapon is unaffected
		switch (wieldTypeWeaponModified)
		{
		case 0: // these cases shouldn't exist for Enlarge ...
		case 1: // only in reduce person; going to assume the "beneficial" case that the reduction was made voluntarily and hence you let the weapon stay larger
			if (regardOffhand) // has offhand item, so assume the weapon stayed the old size
				enlargeWeapon = false;
			break;
		case 2:
			if (regardOffhand) // shouldn't really be possible... maybe if player is cheating
			{
				enlargeWeapon = false; 
				logger->warn("Illegally wielding weapon along withoffhand!");
			}
			break;
		default:
			break;
		}
	case 3:
	case 4:
	default:
		break;
	}


	if (!enlargeWeapon)
	{
		return 0;
	}
		

	switch (dice.GetSides())
	{
	case 2:
		diceSide = 3;
		break;
	case 3:
		diceSide = 4;
		break;
	case 4:
		diceSide = 6;
		break;
	case 6:
		if (diceCount == 1)
			diceSide = 8;
		else if (diceCount <= 3)
			diceCount++;
		else
			diceCount += 2;
		break;
	case 8:
		if (diceCount == 1){
			diceCount = 2;
			diceSide = 6;
		}
		else if (diceCount <= 3)
		{
			diceCount++;
		} 
		else if (diceCount<=6 )	{
			diceCount += 2;
		}
		else
			diceCount += 4;
		break;
	case 10:
		diceCount *= 2;
		diceSide = 8;
		break;
	case 12:
		diceCount = 3;
		diceSide = 6;
		break;
	default:
		break;
	}

	dispIo->dicePacked = Dice(diceCount, diceSide, diceMod).ToPacked();

	return 0;
}

int SpellCallbacks::HezrouStenchObjEvent(DispatcherCallbackArgs args){

	DispIoObjEvent* dispIo = dispatch.DispIoCheckIoType17(args.dispIO);
	auto condEvtId = args.GetCondArg(2);
	if (dispIo->evtId == condEvtId) {

		auto spellId = args.GetCondArg(0);
		SpellPacketBody spellPkt(spellId);
		if (!spellPkt.spellId) {
			logger->warn("HezrouStenchObjEvent: Unable to fetch spell! ID {}", spellId);
			return 0;
		}

		/*
		AoE Entered;
		- add the target to the Spell's Target List
		- Do a saving throw
		*/
		if (args.dispKey == DK_OnEnterAoE)
		{
			/*
				Hezrou Stench condition (the one applied to the SpellObject)
			*/
			if (args.GetData1() == 0)
			{
				// if already has the condition - skip
				if (d20Sys.d20QueryWithData(dispIo->tgt, DK_QUE_Critter_Has_Condition, 	(CondStruct*)&conds.mCondHezrouStenchHit,0))
					return 0;
				if (d20Sys.d20Query(dispIo->tgt, DK_QUE_Critter_Is_Immune_Poison))
					return 0;
				if (critterSys.IsCategorySubtype(dispIo->tgt, MonsterSubcategoryFlag::mc_subtype_demon))
					return 0;
				if (critterSys.IsCategoryType(dispIo->tgt, MonsterCategory::mc_type_elemental))
					return 0;


				pySpellIntegration.SpellSoundPlay(&spellPkt, SpellEvent::SpellStruck);
				pySpellIntegration.SpellTrigger(spellId, SpellEvent::AreaOfEffectHit);


				// Hezrou Stench does not provoke Spell Resistance
				/*if (spellSys.CheckSpellResistance(&spellPkt, dispIo->tgt) == 1)
					return 0;*/


				auto partsysId = gameSystems->GetParticleSys().CreateAtObj("sp-Stinking Cloud Hit", dispIo->tgt);
				spellPkt.AddTarget(dispIo->tgt, partsysId, 1);
				// save succeeds - apply Sickened
				if (damage.SavingThrowSpell(dispIo->tgt, spellPkt.caster, 24, SavingThrowType::Fortitude, 0, spellPkt.spellId)) {
					conds.AddTo(dispIo->tgt, "Hezrou Stench Hit", { static_cast<int>(spellPkt.spellId), spellPkt.durationRemaining, static_cast<int>(dispIo->evtId), partsysId,1 });
					floatSys.FloatSpellLine(dispIo->tgt, 20026, FloatLineColor::Red);
				}
				// save failed - apply nauseated
				else {
					conds.AddTo(dispIo->tgt, "Hezrou Stench Hit", { static_cast<int>(spellPkt.spellId), spellPkt.durationRemaining, static_cast<int>(dispIo->evtId), partsysId, 0 });
					combatSys.FloatCombatLine(dispIo->tgt, 150, FloatLineColor::Red);
				}
			} 
			
			/*
				"Hezrou Stench Hit" condition (the one applied to the critter from the above)
			*/
			else if (args.GetData1() == 1) 
			{
				if (args.GetCondArg(4) == 2) // "cured"
				{
					args.SetCondArg(4, 1); // re-establish sickness when stepping into Hezrou AoE
				}
			}
		
		}

		/*
		AoE exited;
		- If Nauseated (identified by arg3= 0), apply a 1d4 duration
		- If Sickened, changed to "cured" (arg3 = 2)
		*/
		else if (args.dispKey == DK_OnLeaveAoE)
		{
			if (args.GetData1() == 1){

				if (args.GetCondArg(4) == 1) // sickened
				{
					args.SetCondArg(4, 2);
					// histSys.CreateFromFreeText(fmt::format("{} exited Stinking Cloud; Nauseated for {} more rounds.\n", description.getDisplayName(dispIo->tgt), rollResult).c_str());
				}
				else if (args.GetCondArg(4) == 0) // nauseated
				{
					auto rollResult = Dice::Roll(1, 4, 0); // new duration
					args.SetCondArg(1, rollResult);
					histSys.CreateFromFreeText(fmt::format("{} exited Hezrou Stench; Nauseated for {} more rounds.\n", description.getDisplayName(dispIo->tgt), rollResult).c_str());
				}
			}

		}

		if (!spellPkt.UpdateSpellsCastRegistry()) {
			logger->warn("HezrouStenchObjEvent: Unable to save update SpellPacket!");
			return 0;
		}
		pySpellIntegration.UpdateSpell(spellId);
	}
	return 0;
}

int SpellCallbacks::HezrouStenchCountdown(DispatcherCallbackArgs args)
{

	auto effectType = args.GetCondArg(4);
	/*
	count down for nauseated only
	*/
	if (effectType != 0)
	{
		return 0;
	}

	auto dispIo = dispatch.DispIoCheckIoType6(args.dispIO);
	
	// new duration
	int durationRem = (int)args.GetCondArg(1) - (int)dispIo->data1;
	args.SetCondArg(1, durationRem);

	// if duration drops below 0, change to "Cured" status
	// (the assumption is that this will only happen for the 1d4 countdown, i.e. after you leave the hezrou area)
	if (durationRem < 0){
		args.SetCondArg(4, 2);
	}
	return 0;
}

int SpellCallbacks::HezrouStenchTurnbasedStatus(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType8(args.dispIO);
	if (dispIo && args.GetCondArg(4) == 0) {
		auto tbStat = dispIo->tbStatus;
		if (tbStat) {
			if (tbStat->hourglassState > 1)
				tbStat->hourglassState = 1;
		}
	}
	return 0;
}

int SpellCallbacks::HezrouStenchAooPossible(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	// if nauseated
	if (dispIo->return_val && args.GetCondArg(4) == 0){
		dispIo->return_val = 0;
	}
	return 0;
}

int SpellCallbacks::HezrouStenchAbilityCheckMod(DispatcherCallbackArgs args)
{
	if (args.GetCondArg(4) > 1)
		return 0;

	auto dispIo = dispatch.DispIoCheckIoType10(args.dispIO);

	dispIo->bonOut->AddBonus(args.GetData1(), 0, args.GetData2());

	return 0;

}

int SpellCallbacks::HezrouStenchSavingThrowLevel(DispatcherCallbackArgs args)
{
	if (args.GetCondArg(4) > 1)
		return 0;

	auto dispIo = dispatch.DispIoCheckIoType3(args.dispIO);
	dispIo->bonlist.AddBonus(args.GetData1(), 0, args.GetData2());
	return 0;
}

int SpellCallbacks::HezrouStenchDealingDamage(DispatcherCallbackArgs args)
{
	if (args.GetCondArg(4) > 1)
		return 0;

	auto dispIo = dispatch.DispIoCheckIoType4(args.dispIO);
	dispIo->damage.bonuses.AddBonus(args.GetData1(), 0, args.GetData2());
	return 0;
}

int SpellCallbacks::HezrouStenchToHit2(DispatcherCallbackArgs args)
{
	if (args.GetCondArg(4) > 1)
		return 0;

	auto dispIo = dispatch.DispIoCheckIoType5(args.dispIO);
	dispIo->bonlist.AddBonus(args.GetData1(), 0, args.GetData2());
	return 0;
}

int SpellCallbacks::HezrouStenchEffectTooltip(DispatcherCallbackArgs args)
{

	if (args.GetCondArg(4) > 1)
		return 0;

	auto dispIo = dispatch.DispIoCheckIoType24(args.dispIO);
	auto spellId = args.GetCondArg(0);
	SpellPacketBody spellPkt(spellId);

	/*
		nauseated
	*/
	if (args.GetCondArg(4) == 0){

		auto remainingDuration = args.GetCondArg(1);
		if (remainingDuration < 5){
			dispIo->Append(args.GetData1(), spellPkt.spellEnum, fmt::format("\n {}: {}", combatSys.GetCombatMesLine(175), remainingDuration).c_str());
			return 0;
		}
	}
	dispIo->Append(args.GetData1(), spellPkt.spellEnum, nullptr);
	
	return 0;
}

int SpellCallbacks::HezrouStenchCureNausea(DispatcherCallbackArgs args)
{
	if (args.GetCondArg(4) == 1)
		combatSys.FloatCombatLine(args.objHndCaller, 206, FloatLineColor::White);
	else if (args.GetCondArg(4) == 0)
		combatSys.FloatCombatLine(args.objHndCaller, 207, FloatLineColor::White);
	args.SetCondArg(4, 2);
	return 0;
}

int SpellCallbacks::RemoveSpell(DispatcherCallbackArgs args)
{
	conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
	return 0;
}

int SpellCallbacks::HasCondition(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	if (dispIo->data1 == args.GetData1())
		dispIo->return_val = 1;
	return 0;
}

int SpellCallbacks::SpellDismissRadialSub(DispatcherCallbackArgs args)
{
	auto spellId = args.GetCondArg(0);
	SpellPacketBody spPkt(spellId);
	static auto helpId = ElfHash::Hash("TAG_DISMISS_SPELL");
	RadialMenuEntryAction radEntry(-1, D20A_DISMISS_SPELLS, spellId, helpId);
	if (!spPkt.spellEnum){
		logger->warn("SpellDismissRadialSub: Caught ended spell, terminating.");
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0;
	}

	radEntry.text = (char*)spellSys.GetSpellMesline(spPkt.spellEnum);
	if (spPkt.targetCount == 1 && spPkt.targetListHandles[0] != spPkt.caster && spPkt.targetListHandles[0] && objects.IsCritter(spPkt.targetListHandles[0])){
		auto text = fmt::format("{} ({})", radEntry.text, description.getDisplayName(spPkt.targetListHandles[0]));
		auto id = ElfHash::Hash(text);
		 radialMenus.radMenuStrings[id] = text;
		 radEntry.text = (char*)radialMenus.radMenuStrings[id].c_str();
	}
	radEntry.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::SpellsDismiss);
	return 0;
}

int SpellCallbacks::SpellAddDismissCondition(DispatcherCallbackArgs args)
{
	auto spellId = args.GetCondArg(0);
	SpellPacketBody spPkt(spellId);
	if (!spPkt.spellEnum || !spPkt.caster){
		logger->error("Cannot add Dismiss condition to Caster");
		return 0;
	}
	if (d20Sys.d20QueryReturnData(spPkt.caster, DK_QUE_Critter_Can_Dismiss_Spells) != spellId)
		conds.AddTo(spPkt.caster, "Dismiss", { spellId,0,0 });

	return 0;
}

int SpellCallbacks::SpellDismissSignalHandler(DispatcherCallbackArgs args) {
	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
	if (!dispIo) {
		return 1;
	}

	auto spellId = args.GetCondArg(0);
	SpellPacketBody spPkt(spellId);
	if (!spPkt.spellEnum)
		return 0;

	if (dispIo->data1 != spellId)
		return 0;

	if (spPkt.spellEnum == 315 || args.GetData1() == 1 || spPkt.targetCount > 0){
		floatSys.FloatSpellLine(args.objHndCaller, 20000, FloatLineColor::White); // a spell has expired
		args.RemoveSpell();
		args.RemoveSpellMod();
	}

	return 0;
}

int SpellCallbacks::DismissSignalHandler(DispatcherCallbackArgs args){
	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
	auto spellId = args.GetCondArg(0);
	if (dispIo->data2 == 0 && spellId == dispIo->data1){ // used to check dispIo->data1 == 0 too; doesn't seem to make sense, why would spellId be 0?
	
		SpellPacketBody spPkt(spellId);
		if (!spPkt.spellEnum){
			conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
			return 0;
		}
		if (spPkt.aoeObj && spPkt.aoeObj != args.objHndCaller){
			d20Sys.d20SendSignal(spPkt.aoeObj, DK_SIG_Dismiss_Spells, spPkt.spellId, 0);
		}
		// Spell objects. Added in Temple+ for Wall spells
		for (auto i = 0u; i < static_cast<unsigned int>(spPkt.numSpellObjs) && i < 128; i++) {
			auto spellObj = spPkt.spellObjs[i].obj;
			if (!spellObj || spellObj == args.objHndCaller) continue;
			d20Sys.d20SendSignal(spellObj, DK_SIG_Dismiss_Spells, spPkt.spellId, 0);
		}

		for (auto i=0u; i < spPkt.targetCount; i++){
			auto tgt = spPkt.targetListHandles[i];
			if (!tgt || tgt == args.objHndCaller)
				continue;
			d20Sys.d20SendSignal(spPkt.targetListHandles[i], DK_SIG_Dismiss_Spells, spPkt.spellId, 0);
		}

		// in case the dismiss didn't take care of that (e.g. grease)
		if (spPkt.aoeObj) {
			d20Sys.d20SendSignal(spPkt.aoeObj, DK_SIG_Spell_End, spPkt.spellId, 0);
		}

		// Spell objects. Added in Temple+ for Wall spells
		for (auto i = 0u; i < static_cast<unsigned int>(spPkt.numSpellObjs) && i < 128; i++) {
			auto spellObj = spPkt.spellObjs[i].obj;
			if (!spellObj) continue;
			d20Sys.d20SendSignal(spellObj, DK_SIG_Spell_End, spPkt.spellId, 0);
		}

		
		// adding this speciically for grease because I want to be careful
		auto SP_GREASE_ENUM = 200;
		if (spPkt.spellEnum == SP_GREASE_ENUM){
			conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		}
		
		// By now all effects should have been removed. Cross your fingers!
		d20Sys.d20SendSignal(args.objHndCaller, DK_SIG_Spell_End, spPkt.spellId, 0);

	}
	return 0;
}

void DispatcherCallbackArgs::RemoveSpellMod() {
	spCallbacks.SpellRemoveMod(*this);
}

int SpellCallbacks::SpellRemoveMod(DispatcherCallbackArgs args){
	
	DispIoD20Signal *evtObj = nullptr;
	if (args.dispIO)
		evtObj = dispatch.DispIoCheckIoType6(args.dispIO);
		

	if (args.dispKey == DK_SIG_Sequence){
		logger->warn("Caught a DK_SIG_Sequence, make sure we are removing spell_mod properly...");
	}

	switch (args.dispKey){
	case DK_SIG_Killed:
	case DK_SIG_Critter_Killed:
	case DK_SIG_Sequence:
	case DK_SIG_Spell_Cast:
	case DK_SIG_Action_Recipient:
	case DK_SIG_Remove_Concentration:
	case DK_SIG_TouchAttackAdded:
	case DK_SIG_Teleport_Prepare:
	case DK_SIG_Teleport_Reconnect:
	case DK_SIG_Combat_End:
		break;
	default:
		if (evtObj && evtObj->data1 != args.GetCondArg(0))
			return 0;
		break;
	}

	auto spellId = args.GetCondArg(0);
	SpellPacketBody spPkt(spellId);
	switch (args.GetData1()){
	case 2:
		if (args.dispKey == DK_SIG_Remove_Concentration){
			if (spPkt.spellEnum != 0){
				floatSys.FloatCombatLine(args.objHndCaller, 5060); // Stop Concentration
				d20Sys.d20SendSignal(args.objHndCaller, DK_SIG_Concentration_Broken, spellId, 0);

				if (spPkt.caster && spPkt.caster != args.objHndCaller){
					d20Sys.d20SendSignal(spPkt.caster, DK_SIG_Concentration_Broken, spellId, 0);
				}

				for (auto i=0u; i < spPkt.targetCount; i++){
					if (args.objHndCaller != spPkt.targetListHandles[i])
						d20Sys.d20SendSignal(spPkt.targetListHandles[i], DK_SIG_Concentration_Broken, spellId, 0);
				}
				// added in Temple+: Concentration_Broken on the spell objects
				for (auto i=0; i < spPkt.numSpellObjs && i < 128; i++){
					auto spObj = spPkt.spellObjs[i].obj;
					if (!spObj || spObj == args.objHndCaller) continue;
						d20Sys.d20SendSignal(spObj, DK_SIG_Concentration_Broken, spellId, 0);
				}
			}
		}
		break;
	case 29:
	case 69:
	case 70:
	case 71:
	case 72:
	case 121:
	case 171:
	case 232:
		temple::GetRef<void(__cdecl)(objHndl)>(0x1004D1F0)(args.objHndCaller); // Build radial menu
		break;
	default:
		break;
	}

	conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
	
	return 0;
}

int SpellCallbacks::AoeSpellRemove(DispatcherCallbackArgs args){
	auto spellId = args.GetCondArg(0);
	SpellPacketBody pkt(spellId);
	if (!pkt.spellEnum)
		return 0;

	auto partsysIdToPlay = -1;
	switch (args.GetData1()){
	case 0x26:
		gameSystems->GetParticleSys().CreateAtObj("sp-consecrate-END", args.objHndCaller);
		break;
	case 0x35:
		gameSystems->GetParticleSys().CreateAtObj("sp-Desecrate-END", args.objHndCaller);
		break;
	case 0x66:
		gameSystems->GetParticleSys().CreateAtObj("sp-Fog Cloud-END", args.objHndCaller);
		break;
	case 0x8B:
		gameSystems->GetParticleSys().CreateAtObj("sp-Invisibility Sphere-END", args.objHndCaller);
		break;
	case 0x9D:
		gameSystems->GetParticleSys().CreateAtObj("sp-Minor Globe of Invulnerability-END", args.objHndCaller);
		break;
	case 0x9F:
		gameSystems->GetParticleSys().CreateAtObj("sp-Mind Fog-END", args.objHndCaller);
		break;
	case 0xD2:
		gameSystems->GetParticleSys().CreateAtObj("sp-Solid Fog-END", args.objHndCaller);
		break;
	case 0xED:
		gameSystems->GetParticleSys().CreateAtObj("sp-Wind Wall-END", args.objHndCaller);
		break;
	default:
		break;
	}

	gameSystems->GetParticleSys().End(pkt.spellObjs[0].partySysId);
	for (auto i = 1; i < pkt.numSpellObjs; i++){
		gameSystems->GetParticleSys().End(pkt.spellObjs[i].partySysId);
	}

	auto evtId = args.GetCondArg(2);
	objEvents.objEvtTable->remove(evtId);
	args.RemoveSpellMod();
	return 0;
}


#pragma endregion

#pragma region Item Callbacks
int ItemCallbacks::SkillBonus(DispatcherCallbackArgs args)
{
	auto skillEnum = args.GetCondArg(0);
	auto bonValue = args.GetCondArg(1);
	auto bonType = args.GetData1();
	if (args.dispKey - 20 == skillEnum)
	{
		auto invIdx = args.GetCondArg( 2);
		auto item = inventory.GetItemAtInvIdx(args.objHndCaller, invIdx);
		auto dispIo =dispatch.DispIoCheckIoType10(args.dispIO);
		auto itemName = description.getDisplayName(item, args.objHndCaller);
		dispIo->bonOut->AddBonusWithDesc(bonValue, bonType, 112, const_cast<char*>(itemName));
	}
	return 0;
}

int ItemCallbacks::UseableItemRadialEntry(DispatcherCallbackArgs args){
	auto invIdx = args.GetCondArg(2);
	auto itemHandle = inventory.GetItemAtInvIdx(args.objHndCaller, invIdx);
	auto itemObj = gameSystems->GetObj().GetObject(itemHandle);
	auto objType = itemObj->type;
	int useMagicDeviceSkillBase = critterSys.SkillBaseGet(args.objHndCaller, skill_use_magic_device);

	if (objType != obj_t_food && !inventory.IsIdentified(itemHandle))
		return 0;
	
	auto charges = itemObj->GetInt32(obj_f_item_spell_charges_idx);
	if (charges == 0)
		return 0;

	auto itemFlags = itemObj->GetItemFlags();

	auto spIdx = args.GetCondArg(0);

	auto spData = itemObj->GetSpell(obj_f_item_spell_idx, spIdx);

	auto handle = args.objHndCaller;
	auto obj = objSystem->GetObject(handle);



	if ( objType == obj_t_scroll || (itemFlags & OIF_NEEDS_SPELL) && (itemObj->type == obj_t_generic || itemObj->type == obj_t_weapon) ){
		auto isOk = false;

		if (useMagicDeviceSkillBase || critterSys.HashMatchingClassForSpell(args.objHndCaller, spData.spellEnum))
			isOk = true;

		// clerics with magic domain
		else if (spellSys.IsArcaneSpellClass(spData.classCode)) {
			auto clrLvl = objects.StatLevelGet(handle, stat_level_cleric);
			if (clrLvl > 0 && max(1, clrLvl / 2) >= (int)spData.spellLevel && critterSys.HasDomain(handle, Domain_Magic))
				isOk = true;
		}

		if (!isOk)
			return 0;
		
	}

	if (objType == obj_t_scroll && !spellSys.CheckAbilityScoreReqForSpell(args.objHndCaller, spData.spellEnum, -1) && !useMagicDeviceSkillBase)
		return 0;

	RadialMenuEntry radEntry;
	auto actType = D20A_USE_ITEM;
	if (objType == obj_t_food){
		if (inventory.IsMagicItem(itemHandle))
			actType = D20A_USE_POTION;
		else
			actType = D20A_USE_ITEM;
	} 
	else{
		actType = D20A_USE_ITEM;
	}
	radEntry.d20ActionType = actType;

	radEntry.d20ActionData1 = invIdx;
	radEntry.d20SpellData.Set(spData.spellEnum, spData.classCode, spData.spellLevel, invIdx, (MetaMagicData)0);
	radEntry.text = const_cast<char*>(description.getDisplayName(itemHandle, args.objHndCaller));


	auto chargesRem = charges;
	if (objType == obj_t_scroll || objType == obj_t_food){
		chargesRem = inventory.GetQuantity(itemHandle);
	}
	
	if (chargesRem > 1){
		auto text = fmt::format("{} ({})", radEntry.text, chargesRem);
		auto textId = ElfHash::Hash(text);
		radialMenus.radMenuStrings[textId] = text;
		radEntry.text = (char*)radialMenus.radMenuStrings[textId].c_str();
	}

	RadialMenuStandardNode parentType;
	switch(objType)
	{
	case obj_t_scroll:
		parentType = RadialMenuStandardNode ::Scrolls;
		break;
	case obj_t_food:
		parentType = RadialMenuStandardNode::Potions;
		break;
	default:
		parentType = args.GetCondArg(1) != 3 ? RadialMenuStandardNode::Items : RadialMenuStandardNode::Wands;
		break;
	}
	
	radEntry.helpId = ElfHash::Hash(spellSys.GetSpellEnumTAG(spData.spellEnum));
	if (!spellSys.SpellHasMultiSelection(spData.spellEnum)){
		radEntry.AddChildToStandard(args.objHndCaller, parentType);
	} 
	else
	{
		radEntry.type = RadialMenuEntryType::Parent;
		radEntry.d20ActionType = D20A_NONE;
		auto parentNodeIdx = radialMenus.AddParentChildNode(handle, &radEntry, radialMenus.GetStandardNode(parentType));
		std::vector<SpellMultiOption> multiOptions;
		if (!spellSys.GetMultiSelectOptions(spData.spellEnum, multiOptions))
		{
			logger->error("Spell multiselect options not found!");
		}

		// populate options
		auto numOptions = multiOptions.size();
		for (auto i = 0u; i<numOptions; i++) {
			auto &op = multiOptions[i];
			RadialMenuEntry radChild;
			radChild.SetDefaults();

			radChild.d20SpellData.Set(spData.spellEnum, spData.classCode, spData.spellLevel, invIdx, (MetaMagicData)0);
			radChild.d20ActionType = actType;
			radChild.d20ActionData1 = invIdx;
			radChild.helpId = ElfHash::Hash(spellSys.GetSpellEnumTAG(spData.spellEnum));
			radialMenus.SetCallbackCopyEntryToSelected(&radChild);


			if (op.isProto) {
				auto protoId = multiOptions[i].value;
				radChild.minArg = protoId;

				auto protoHandle = objSystem->GetProtoHandle(protoId);
				auto protoObj = objSystem->GetObject(protoHandle);
				radChild.text = (char*)description.GetDescriptionString(protoObj->GetInt32(obj_f_description));

			}
			else {
				MesLine line(multiOptions[i].value);
				mesFuncs.GetLine_Safe(*spellSys.spellsRadialMenuOptionsMes, &line);
				radChild.text = (char*)line.value;
				radChild.minArg = i + 1;
			}

			radChild.AddAsChild(handle, parentNodeIdx);
		}

		auto radnow = radialMenus.GetForObj(handle);
		auto asd = 1;
	}
	

	// add to Copy Scroll
	if (objType == obj_t_scroll && objects.StatLevelGet(args.objHndCaller, stat_level_wizard) >= 1
		&& critterSys.HashMatchingClassForSpell(args.objHndCaller, spData.spellEnum)
		&& spellSys.IsArcaneSpellClass(spData.classCode) )
	{
		// check if spell is not known
		std::vector<int> spellClasses, spellLevels;
		spellSys.SpellKnownQueryGetData(args.objHndCaller, spData.spellEnum, spellClasses, spellLevels);
		
		auto alreadyKnows = false;

		for (auto i=0u; i < spellClasses.size(); i++){
			if (spellClasses[i] == spellSys.GetSpellClass(stat_level_wizard))
				return 0;
		}

		auto spLvl = spellSys.GetSpellLevelBySpellClass(spData.spellEnum, spellSys.GetSpellClass(stat_level_wizard));

		if (spLvl >= 0){
			radEntry.text = const_cast<char*>(description.getDisplayName(itemHandle, args.objHndCaller));
			radEntry.type = RadialMenuEntryType::Action;
			radEntry.helpId = ElfHash::Hash(spellSys.GetSpellEnumTAG(spData.spellEnum));
			radEntry.d20ActionType = D20A_COPY_SCROLL;
			radEntry.d20ActionData1 = inventory.GetInventoryLocation(itemHandle);
			radEntry.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::CopyScroll);
		}
		
	}
	
	return 0;
}

int ItemCallbacks::UseableItemActionCheck(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeD20ActionTurnBased, DispIoD20ActionTurnBased);

	auto invIdx = args.GetCondArg(2);

	// check if this is the referenced item
	if (dispIo->d20a->data1 != invIdx)
		return 0;


	auto itemHandle = inventory.GetItemAtInvIdx(args.objHndCaller, invIdx);
	auto itemObj = gameSystems->GetObj().GetObject(itemHandle);
	auto objType = itemObj->type;
	int useMagicDeviceSkillBase = critterSys.SkillBaseGet(args.objHndCaller, skill_use_magic_device);

	// ensure is identified
	if (objType != obj_t_food && !inventory.IsIdentified(itemHandle)){
		dispIo->returnVal = AEC_INVALID_ACTION;
		return IEC_Cannot_Wield_Magical;
	}
		
	// check item charges
	auto charges = itemObj->GetInt32(obj_f_item_spell_charges_idx);
	if (charges == 0){
		dispIo->returnVal = AEC_OUT_OF_CHARGES;
		return IEC_Cannot_Wield_Magical;
	}
	

	// check if caster needs and has spell/class
	auto itemFlags = itemObj->GetItemFlags();

	auto spIdx = args.GetCondArg(0);
	auto spData = itemObj->GetSpell(obj_f_item_spell_idx, spIdx);

	auto handle = args.objHndCaller;
	auto obj = objSystem->GetObject(handle);

	if (objType == obj_t_scroll || (itemFlags & OIF_NEEDS_SPELL) && (itemObj->type == obj_t_generic || itemObj->type == obj_t_weapon)) {
		auto isOk = false;
		

		if (useMagicDeviceSkillBase || critterSys.HashMatchingClassForSpell(args.objHndCaller, spData.spellEnum))
			isOk = true;

		// clerics with magic domain
		else if (spellSys.IsArcaneSpellClass(spData.classCode)) {  
			auto clrLvl = objects.StatLevelGet(handle, stat_level_cleric);
			if (clrLvl > 0 && max(1, clrLvl / 2) >= (int)spData.spellLevel && critterSys.HasDomain(handle, Domain_Magic))
				isOk = true;
		}

		if (!isOk){
			dispIo->returnVal = AEC_INVALID_ACTION;
			return IEC_Cannot_Wield_Magical;
		}
	}

	if (objType == obj_t_scroll && !spellSys.CheckAbilityScoreReqForSpell(args.objHndCaller, spData.spellEnum, -1) && !useMagicDeviceSkillBase){
		dispIo->returnVal = AEC_INVALID_ACTION;
		return IEC_Cannot_Wield_Magical;
	}

	return 0;
}

int ItemCallbacks::BucklerToHitPenalty(DispatcherCallbackArgs args)
{
	auto invIdx = args.GetCondArg(2);

	auto dispIo = static_cast<DispIoAttackBonus*>(args.dispIO);
	dispIo->AssertType(dispIOTypeAttackBonus);

	auto weaponUsed = dispIo->attackPacket.GetWeaponUsed();

	if ((dispIo->attackPacket.flags & D20CAF_RANGED))
		return 0;

	if (!weaponUsed)
		return 0;

	if (d20Sys.d20QueryWithData(args.objHndCaller, DK_QUE_WieldedTwoHanded, reinterpret_cast<uint32_t>(dispIo), 0)
		|| dispIo->attackPacket.IsOffhandAttack())
	{
		dispIo->bonlist.AddBonus(-1, 0, 327);
	}


	return 0;
}


int __cdecl ItemCallbacks::BucklerAcPenalty(DispatcherCallbackArgs args)
{
	//Check if the penalty is turned off through python
	if (d20Sys.D20QueryPython(args.objHndCaller, "Disable Buckler Penalty") == 0) {

		auto dispIo = static_cast<DispIoAttackBonus*>(args.dispIO);
		dispIo->AssertType(dispIOTypeAttackBonus);

		auto weaponUsed = dispIo->attackPacket.GetWeaponUsed();

		if ((dispIo->attackPacket.flags & D20CAF_RANGED))
			return 0;

		if (!weaponUsed)
			return 0;

		auto offhandObject = inventory.ItemWornAt(args.objHndCaller, EquipSlot::WeaponSecondary);

		if (offhandObject) {
			auto objectFlag = objects.getInt32(offhandObject, obj_f_type);
			if (objectFlag == obj_t_weapon) {
				args.SetCondArg(1, 1);  //Two Weapon Fighting, remove buckler bonus
			}
		} else {
			auto twoHanded = d20Sys.d20QueryWithData(args.objHndCaller, DK_QUE_WieldedTwoHanded, reinterpret_cast<uint32_t>(dispIo), 0);
			if (twoHanded) {
				args.SetCondArg(1, 1);  //Using a two handed weapon or using a one handed weapon in two hands, remove buckler bonus
			}
		}
	}

	return 0;
}


int ItemCallbacks::WeaponMerciful(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeDamage, DispIoDamage);

	auto attacker = dispIo->attackPacket.attacker;
	if (!attacker)
		return 0;

	auto invIdx = args.GetCondArg(2);
	auto item = inventory.GetItemAtInvIdx(attacker, invIdx);
	auto dice = Dice(1, 6);

	auto weapUsed = dispIo->attackPacket.GetWeaponUsed();
	if (!weapUsed)
		return 0;

	if (weapUsed == item) {
		dispIo->damage.AddDamageDice(dice.ToPacked(), DamageType::Subdual, 121);
	}
	return 0;
}

int ItemCallbacks::WeaponSpeed(DispatcherCallbackArgs args){

	args.dispIO->AssertType(dispIOTypeD20ActionTurnBased);
	auto dispIo = static_cast<DispIoD20ActionTurnBased*>(args.dispIO);
	if (dispIo->bonlist) {
		dispIo->bonlist->AddBonus(1, 34, 346); // Weapon of Speed
	}

	return 0;
}

int ItemCallbacks::WeaponSeekingAttackerConcealmentMissChance(DispatcherCallbackArgs args){
	args.dispIO->AssertType(dispIoTypeObjBonus);
	auto dispIo = static_cast<DispIoObjBonus*>(args.dispIO);
	dispIo->bonOut->AddCap(0, 0, 347);
	return 0;
}

int ItemCallbacks::WeaponVicious(DispatcherCallbackArgs args){

	GET_DISPIO(dispIOTypeDamage, DispIoDamage);

	auto attacker = dispIo->attackPacket.attacker;
	if (!attacker)
		return 0;

	auto invIdx = args.GetCondArg(2);
	auto item = inventory.GetItemAtInvIdx(attacker, invIdx);
	auto dice = Dice(2, 6);

	auto weapUsed = dispIo->attackPacket.GetWeaponUsed();
	if (!weapUsed)
		return 0;

	if (weapUsed == item){
		auto itemName = description.getDisplayName(item);
		dispIo->damage.AddDamageDice(dice.ToPacked(), DamageType::Magic, 121, itemName);

		//auto counterDam = Dice(1, 6);
		// damage.DealDamage(attacker, 0i64, counterDam, DamageType::Magic, (int)AttackPowerType::Unspecified, 0, 100, D20A_NONE);
	}
	return 0;
}

int ItemCallbacks::WeaponViciousBlowback(DispatcherCallbackArgs args)
{
	GET_DISPIO(dispIOTypeDamage, DispIoDamage);

	auto attacker = dispIo->attackPacket.attacker;
	if (!attacker)
		return 0;

	auto invIdx = args.GetCondArg(2);
	auto item = inventory.GetItemAtInvIdx(attacker, invIdx);
	auto dice = Dice(2, 6);

	auto weapUsed = dispIo->attackPacket.GetWeaponUsed();
	if (!weapUsed)
		return 0;

	if (weapUsed == item) {
	//	auto counterDam = Dice(1, 6);
	//	damage.DealDamage(attacker, 0i64, counterDam, DamageType::Magic, (int)AttackPowerType::Unspecified, 0, 100, D20A_NONE);
		//not that simply unfortunately !
	}
	return 0;
}

int ItemCallbacks::WeaponWounding(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeDamage, DispIoDamage);

	auto attacker = dispIo->attackPacket.attacker;
	if (!attacker)
		return 0;
	auto victim = dispIo->attackPacket.victim;

	auto invIdx = args.GetCondArg(2);
	auto item = inventory.GetItemAtInvIdx(attacker, invIdx);

	auto weapUsed = dispIo->attackPacket.GetWeaponUsed();
	if (!weapUsed)
		return 0;

	if (weapUsed == item) {
		if (d20Sys.d20Query(victim, DK_QUE_Critter_Is_Immune_Critical_Hits))
			return 0;

		histSys.CreateFromFreeText(fmt::format("{} takes 1 Con damage.\n", description.getDisplayName(victim)).c_str());
		conds.AddTo(victim, "Damage_Ability_Loss", { 2,1 });
	}
	return 0;


	
}

int ItemCallbacks::WeaponThundering(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeDamage, DispIoDamage);

	auto attacker = dispIo->attackPacket.attacker;
	if (!attacker)
		return 0;
	auto victim = dispIo->attackPacket.victim;

	auto invIdx = args.GetCondArg(2);
	auto item = inventory.GetItemAtInvIdx(attacker, invIdx);

	auto weapUsed = dispIo->attackPacket.GetWeaponUsed();
	if (!weapUsed)
		return 0;

	if (weapUsed == item) {
		
		if (dispIo->attackPacket.flags & D20CAF_CRITICAL){
			auto critMultiplier = gameSystems->GetObj().GetObject(item)->GetInt32(obj_f_weapon_crit_hit_chart);
			dispIo->damage.AddDamageDice(Dice(critMultiplier - 1, 8).ToPacked(), DamageType::Sonic, 121);
			sound.PlaySoundAtObj(100000, victim); // thunderclap.mp3
			if (!damage.SavingThrow(victim, attacker, 14, SavingThrowType::Fortitude, 0)){
				conds.AddTo(victim, "sp-Deafness", {0,0,0});
			}
		}

	}
	return 0;
}

int ItemCallbacks::WeaponDamageBonus(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeDamage, DispIoDamage);

	auto attacker = dispIo->attackPacket.attacker;
	if (!attacker)
		return 0;
	auto victim = dispIo->attackPacket.victim;

	auto invIdx = args.GetCondArg(2);
	auto item = inventory.GetItemAtInvIdx(attacker, invIdx);

	auto weapUsed = dispIo->attackPacket.GetWeaponUsed();
	if (!weapUsed)
		return 0;

	if (!item)
		return 0;

	if (item == weapUsed 
		|| item == dispIo->attackPacket.ammoItem && weapons.AmmoMatchesWeapon(weapUsed, item)){

		auto damBonus = args.GetCondArg(0);
		auto weapName = description.getDisplayName(item);
		dispIo->damage.AddDamageBonus(damBonus, 12, 147, weapName);
	}
	return 0;
}
#pragma endregion 


#pragma region Class Ability Callbacks
int ClassAbilityCallbacks::FeatDamageReduction(DispatcherCallbackArgs args){
	auto drAmt = args.GetCondArg(1);
	auto bypasserBitmask = args.GetData1();
	auto dispIo = dispatch.DispIoCheckIoType4(args.dispIO);
	damage.AddPhysicalDR(&dispIo->damage, drAmt, bypasserBitmask, 126); // 126 is ~Damage Reduction~[TAG_SPECIAL_ABILITIES_DAMAGE_REDUCTION]
	return 0;
}

int ClassAbilityCallbacks::FeatEmptyBody(DispatcherCallbackArgs args){
	
	RadialMenuEntry mainRadEntry;

	mainRadEntry.text = combatSys.GetCombatMesLine(6020); // Empty Body
	mainRadEntry.helpId = ElfHash::Hash("TAG_CLASS_FEATURES_MONK_EMPTY_BODY");
	int parentNode = radialMenus.GetStandardNode(RadialMenuStandardNode::Class);
	int newParent = radialMenus.AddParentChildNode(args.objHndCaller, &mainRadEntry, parentNode);
	mainRadEntry.d20ActionType = D20A_EMPTY_BODY;

	auto duration = args.GetCondArg(2);
	auto actualArg = args.GetCondArgPtr(1);
	RadialMenuEntrySlider setterChild( 6014, 0, duration, actualArg, 6021, ElfHash::Hash("TAG_CLASS_FEATURES_MONK_EMPTY_BODY"));
	radialMenus.AddChildNode(args.objHndCaller, &setterChild, newParent);


	RadialMenuEntryAction activaterChild(6013, D20A_EMPTY_BODY, 0, ElfHash::Hash("TAG_CLASS_FEATURES_MONK_EMPTY_BODY")); // use
	radialMenus.AddChildNode(args.objHndCaller, &activaterChild, newParent);


	return 0;
}

int ClassAbilityCallbacks::FeatEmptyBodyInit(DispatcherCallbackArgs args){
	// init the remaining number of rounds to the Monk's level
	args.SetCondArg(2, objects.StatLevelGet(args.objHndCaller, stat_level_monk));
	return 0;
}

int ClassAbilityCallbacks::FeatEmptyBodyReduceRounds(DispatcherCallbackArgs args)
{
	int numRoundsRem = args.GetCondArg(2);
	auto dispIo = dispatch.DispIoCheckIoType12(args.dispIO);
	if (!dispIo)
	{
		throw TempleException("FeatEmptyBodyReduceRounds: wrong dispatcher type!");
	}

	D20Actn*d20a = dispIo->d20a;
	args.SetCondArg(2, max(0,numRoundsRem - (int)d20a->data1));

	return 0;
}

int ClassAbilityCallbacks::FeatQuiveringPalmRadial(DispatcherCallbackArgs args){

	RadialMenuEntryAction radEntry(5116, D20A_QUIVERING_PALM, 0,ElfHash::Hash("TAG_CLASS_FEATURES_MONK_QUIVERING_PALM"));
	radEntry.AddChildToStandard(args.objHndCaller ,RadialMenuStandardNode::Class);

	return 0;
}

int ClassAbilityCallbacks::FeatQuiveringPalmInit(DispatcherCallbackArgs args){
	args.SetCondArg(2, 1);
	return 0;
}

int ClassAbilityCallbacks::FeatQuiveringPalmPerform(DispatcherCallbackArgs args){
	args.SetCondArg(2, 0);
	return 0;
}

int ClassAbilityCallbacks::FeatQuiveringPalmAvailable(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	if (!dispIo) {
		throw TempleException("GetNumRoundsRemaining: Wrong dispatcher type!");
	}

	if (args.GetCondArg(2))
		dispIo->return_val = 1;

	return 0;
}

int ClassAbilityCallbacks::GetNumRoundsRemaining(DispatcherCallbackArgs args){

	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	if (!dispIo){
		throw TempleException("GetNumRoundsRemaining: Wrong dispatcher type!");
	}

	if (dispIo->data1 == 1) // getting the number of rounds set by slider
	{
		int numRounds = args.GetCondArg(1);
		if (numRounds < 0) {
			numRounds = 1;
		}
		if (numRounds > args.GetCondArg(2))
			numRounds = max(0, (int)args.GetCondArg(2));
		
		dispIo->data2 = numRounds;
	} 
	
	else if (dispIo->data1 == 2) // getting the max possible number of rounds
	{
		auto numRounds = args.GetCondArg(2);
		dispIo->data2 = numRounds;
	}
	return 0;
}

int ClassAbilityCallbacks::TimedEffectCountdown(DispatcherCallbackArgs args){

	if (!args.GetData1()) // if data1 is set to 0, make it a permanent effect
		return 0;

	int numRoundsRem = args.GetCondArg(2);

	if ( numRoundsRem <= 1 ){
		ConditionRemoveCallback(args);
		return 0;
	}
	args.SetCondArg(2, numRoundsRem - 1);
	return 0;
}

int ClassAbilityCallbacks::FeatDiamondSoulInit(DispatcherCallbackArgs args){

	auto monkLvl = objects.StatLevelGet(args.objHndCaller, stat_level_monk);
	args.SetCondArg(1, monkLvl + 10);

	return 0;
}

int ClassAbilityCallbacks::FeatDiamondSoulSpellResistanceMod(DispatcherCallbackArgs args){
	auto srMod = args.GetCondArg(1);
	auto dispIo = dispatch.DispIOCheckIoType14(static_cast<DispIOBonusListAndSpellEntry*>(args.dispIO));
	dispIo->bonList->AddBonus(srMod, 36, 203);
	return 0;
}

int ClassAbilityCallbacks::CouragedAuraSavingThrow(DispatcherCallbackArgs args)
{

	auto dispIo = static_cast<DispIoSavingThrow*>(args.dispIO);
	dispIo->AssertType(dispIOTypeSavingThrow);
	if (!(dispIo->flags & 0x100000))
		return 0;
	
	objHndl auraSource = args.GetCondArgObjHndl(0);

	if (!auraSource) {
		return 0;
	}

	if (auraSource)
	{
		if (d20Sys.d20Query(auraSource, DK_QUE_Unconscious)) {
			conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		}

		// if distance < 10 ft
		if (locSys.DistanceToObj(auraSource, args.objHndCaller) <  10.0)
		{
			dispIo->bonlist.AddBonus(4, 13, 204);
		}
	}
	

	return 0;
}

int ClassAbilityCallbacks::FeatBrewPotionRadialMenu(DispatcherCallbackArgs args){
	return ItemCreationBuildRadialMenuEntry(args, BrewPotion, "TAG_BREW_POTION", 5066);
}

int ClassAbilityCallbacks::FeatScribeScrollRadialMenu(DispatcherCallbackArgs args)
{
	return ItemCreationBuildRadialMenuEntry(args, ScribeScroll, "TAG_SCRIBE_SCROLL", 5067);
};

int ClassAbilityCallbacks::FeatCraftWandRadial(DispatcherCallbackArgs args){

	if (combatSys.isCombatActive()) { return 0; }
	MesLine mesLine;
	RadialMenuEntry radMenuCraftWand;
	mesLine.key = 5068;
	mesFuncs.GetLine_Safe(*combatSys.combatMesfileIdx, &mesLine);
	radMenuCraftWand.text = (char*)mesLine.value;
	radMenuCraftWand.d20ActionType = D20A_ITEM_CREATION;
	radMenuCraftWand.d20ActionData1 = CraftWand;
	radMenuCraftWand.helpId = ElfHash::Hash("TAG_CRAFT_WAND");

	int newParent = radialMenus.AddParentChildNode(args.objHndCaller, &radMenuCraftWand, radialMenus.GetStandardNode(RadialMenuStandardNode::Feats));


	auto setWandLevelMaxArg = min(20, critterSys.GetCasterLevel(args.objHndCaller));
	RadialMenuEntrySlider setWandLevel(6017,1, setWandLevelMaxArg, args.GetCondArgPtr(0), 6019, ElfHash::Hash("TAG_CRAFT_WAND"));
	radialMenus.AddChildNode(args.objHndCaller, &setWandLevel, newParent);

	RadialMenuEntryAction useCraftWand(6018, D20A_ITEM_CREATION, ItemCreationType::CraftWand, "TAG_CRAFT_WAND");
	radialMenus.AddChildNode(args.objHndCaller, &useCraftWand, newParent);

	return 0;
}

int ClassAbilityCallbacks::CraftWandOnAdd(DispatcherCallbackArgs args){
	conds.AddTo(args.objHndCaller, "Craft Wand Level Set", { 1, 0 });
	return 0;
}

int ClassAbilityCallbacks::FeatIronWillSave(DispatcherCallbackArgs args){
	auto dispIo = static_cast<DispIoSavingThrow*>(args.dispIO);
	dispIo->AssertType(dispIOTypeSavingThrow);

	auto featEnum = (feat_enums)args.GetCondArg(0);
	dispIo->bonlist.AddBonusFromFeat(args.GetData1(), 0, 114, featEnum);

	return 0;
}

int ClassAbilityCallbacks::ItemCreationBuildRadialMenuEntry(DispatcherCallbackArgs args, ItemCreationType itemCreationType, char* helpSystemString, MesHandle combatMesLine)
{
	if (combatSys.isCombatActive()) { return 0; }
	MesLine mesLine;
	RadialMenuEntryAction radEntry(combatMesLine, D20A_ITEM_CREATION, itemCreationType, helpSystemString);
	radEntry.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::Feats);

	return 0;
}

int ClassAbilityCallbacks::DruidWildShapeInit(DispatcherCallbackArgs args){
	auto druidLvl = objects.StatLevelGet(args.objHndCaller, stat_level_druid);
	auto numTimes = 1; // number of times can wild shape per day
	if (druidLvl >= 6) {
		switch (druidLvl) {
		case 6:
			numTimes = 2;
			break;
		case 7:
		case 8:
		case 9:
			numTimes = 3;
			break;
		case 10:
		case 11:
		case 12:
		case 13:
			numTimes = 4;
			break;
		case 14:
		case 15:
		case 16:
		case 17:
			numTimes = 5;
			break;
		default: // 18 and above
			numTimes = 6;
			break;
		}

		// elemental num times (new)
		if (druidLvl >= 16) {
			numTimes += (1 << 8);
		}
		if (druidLvl >= 18)
			numTimes += (1 << 8);
		if (druidLvl >= 20)
			numTimes += (1 << 8);
	}



	args.SetCondArg(0, numTimes);

	// Add if the condition has not already been added.  The extender messes up things up if a query is not used and
	// the condition can get added many times.
	auto res = d20Sys.D20QueryPython(args.objHndCaller, "Wild Shaped Condition Added");
	if (!res) {
		conds.AddTo(args.objHndCaller, conds.GetByName("Wild Shaped"), { numTimes, 0,0 });
	}

	return 0;
}

int ClassAbilityCallbacks::DruidWildShapedInit(DispatcherCallbackArgs args)
{
	return 0;
}

int ClassAbilityCallbacks::DruidWildShapeReset(DispatcherCallbackArgs args){
	auto druidLvl = objects.StatLevelGet(args.objHndCaller, stat_level_druid);
	auto numTimes = 1; // number of times can wild shape per day
	if (druidLvl >= 6) {
		switch (druidLvl) {
		case 6:
			numTimes = 2;
			break;
		case 7:
		case 8:
		case 9:
			numTimes = 3;
			break;
		case 10:
		case 11:
		case 12:
		case 13:
			numTimes = 4;
			break;
		case 14:
		case 15:
		case 16:
		case 17:
			numTimes = 5;
			break;
		default: // 18 and above
			numTimes = 6;
			break;
		}

		// elemental num times (new)
		if (druidLvl >=16) {
			numTimes += (1 << 8);
		}
		if (druidLvl >= 18)
			numTimes += (1 << 8);
		if (druidLvl >= 20)
			numTimes += (1 << 8);
	}

	//See if any bonus uses should be added
	auto extraWildShape = d20Sys.D20QueryPython(args.objHndCaller, "Extra Wildshape Uses");
	auto extraElementalWildShape = d20Sys.D20QueryPython(args.objHndCaller, "Extra Wildshape Elemental Uses");
	numTimes += extraWildShape;
	numTimes += (1 << 8) * extraElementalWildShape;
	
	args.SetCondArg(0, numTimes);
	if (args.GetCondArg(2)) {
		args.SetCondArg(2, 0);
		objects.ClearAnim(args.objHndCaller);

		gameSystems->GetParticleSys().CreateAtObj("sp-animal shape", args.objHndCaller);
		d20StatusSys.initItemConditions(args.objHndCaller);
	}
	return 0;
}

int ClassAbilityCallbacks::DruidWildShapeD20StatusInit(DispatcherCallbackArgs args){
	if (args.GetCondArg(2))
		d20StatusSys.initItemConditions(args.objHndCaller);
	return 0;
}

int ClassAbilityCallbacks::DruidWildShapeGetNumAttacks(DispatcherCallbackArgs args){
	auto dispIo = static_cast<DispIoD20ActionTurnBased*>(args.dispIO);
	dispIo->AssertType(dispIOTypeD20ActionTurnBased);

	int protoId = d20Sys.d20Query(args.objHndCaller, DK_QUE_Polymorphed);
	if (!protoId){
		return 0;
	}

	auto protoHandle = gameSystems->GetObj().GetProtoHandle(protoId);
	if (!protoHandle){
		return 0;
	}
	
	dispIo->returnVal = critterSys.GetCritterNumNaturalAttacks(protoHandle);
	return 0;
}

int ClassAbilityCallbacks::DruidWildShapeRadialMenu(DispatcherCallbackArgs args){
	RadialMenuEntryParent wildshapeMain(5076);
	auto wsId = wildshapeMain.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::Class);

	// if wild shape active - add "Deactivate" node
	if (args.GetCondArg(2)) { 
		RadialMenuEntryAction wsDeactivate(5077, D20A_CLASS_ABILITY_SA, WS_Deactivate, "TAG_CLASS_FEATURES_DRUID_WILD_SHAPE");
		wsDeactivate.AddAsChild(args.objHndCaller, wsId);
		return 0;
	}
	
	// else add the WS options
	auto addOption = [args, wsId](WildShapeProtoIdx optionIdx) {

		auto &wsProto = d20ClassSys.wildShapeProtos[optionIdx].protoId;
		auto protoCode = optionIdx + (1 << 24);
		RadialMenuEntryAction wsOption(0, D20A_CLASS_ABILITY_SA, protoCode, "TAG_CLASS_FEATURES_DRUID_WILD_SHAPE");
		
		auto protoHandle = gameSystems->GetObj().GetProtoHandle(wsProto);
		wsOption.text = (char*)description.getDisplayName(protoHandle);
		return wsOption.AddAsChild(args.objHndCaller ,wsId);
	};


	auto druidLvl = objects.StatLevelGet(args.objHndCaller, stat_level_druid);

	for (auto it : d20ClassSys.wildShapeProtos) {
		if (druidLvl >= (int) it.second.minLvl && it.second.monCat == mc_type_animal)
			addOption(it.first);
	}

	if (d20Sys.d20Query(args.objHndCaller, DK_QUE_Wearing_Ring_of_Change)) {
		addOption(WS_Hill_Giant);
	}

	// elementals
	if (druidLvl >= 16) {
		RadialMenuEntryParent wsElem(5118);
		auto elemId = wsElem.AddAsChild(args.objHndCaller, wsId);


		auto addElem = [args, elemId](WildShapeProtoIdx optionIdx) {

			auto &wsProto = d20ClassSys.wildShapeProtos[optionIdx].protoId;
			auto protoCode = optionIdx + (1 << 24);
			RadialMenuEntryAction wsOption(0, D20A_CLASS_ABILITY_SA, protoCode, "TAG_CLASS_FEATURES_DRUID_WILD_SHAPE");

			auto protoHandle = gameSystems->GetObj().GetProtoHandle(wsProto);
			wsOption.text = (char*)description.getDisplayName(protoHandle);
			return wsOption.AddAsChild(args.objHndCaller, elemId);
		};

		for (auto it : d20ClassSys.wildShapeProtos) {
			if (druidLvl >= (int)it.second.minLvl && it.second.monCat == mc_type_elemental)
				addElem(it.first);
		}
	}

	return 0;
}

int ClassAbilityCallbacks::DruidWildShapeCheck(DispatcherCallbackArgs args){
	auto druidLvl = objects.StatLevelGet(args.objHndCaller, stat_level_druid);

	auto dispIo = static_cast<DispIoD20ActionTurnBased*>(args.dispIO);
	dispIo->AssertType(dispIOTypeD20ActionTurnBased);

	auto d20a = dispIo->d20a;
	if (d20a->data1 & (1 << 24)) {
		if (args.GetCondArg(2)) // already polymorphed
			return 0;

		auto idx = d20a->data1 - (1 << 24);
		auto &spec = d20ClassSys.wildShapeProtos[(WildShapeProtoIdx)idx];
		if (druidLvl < (int)spec.minLvl && spec.minLvl != 10000)
			dispIo->returnVal = AEC_INVALID_ACTION;

		auto numTimes = args.GetCondArg(0);
		if (spec.monCat == mc_type_elemental) {
			numTimes = numTimes >> 8;
			if (numTimes <= 0)
				dispIo->returnVal = AEC_OUT_OF_CHARGES;
		}
		else { // normal animal (or plant)
			numTimes = numTimes & 0xFF;
			if (numTimes <= 0)
				dispIo->returnVal = AEC_OUT_OF_CHARGES;
		}
	}

	return 0;
}

int ClassAbilityCallbacks::DruidWildShapePerform(DispatcherCallbackArgs args){
	auto druidLvl = objects.StatLevelGet(args.objHndCaller, stat_level_druid);

	auto dispIo = static_cast<DispIoD20ActionTurnBased*>(args.dispIO);
	dispIo->AssertType(dispIOTypeD20ActionTurnBased);

	auto d20a = dispIo->d20a;

	if (!(d20a->data1 & (1 << 24)))
		return 0;


	auto initObj = [args](int protoId) {
		objects.ClearAnim(args.objHndCaller);
		if (protoId) {
			auto lvl = objects.StatLevelGet(args.objHndCaller, stat_level);
			damage.Heal(args.objHndCaller, args.objHndCaller, Dice(0, 0, lvl), D20A_CLASS_ABILITY_SA);
		}

		gameSystems->GetParticleSys().CreateAtObj("sp-animal shape", args.objHndCaller);
		d20StatusSys.initItemConditions(args.objHndCaller);
		
	};

	auto curWsProto = args.GetCondArg(2);
	if (curWsProto) { // deactivating
		args.SetCondArg(2, 0);
		initObj(0);
		return 0;
	}

	auto numTimes = args.GetCondArg(0);
	auto idx = d20a->data1 - (1 << 24);
	auto &protoSpec = d20ClassSys.wildShapeProtos[(WildShapeProtoIdx)idx];
	if (protoSpec.monCat == mc_type_elemental) {
		if ( (numTimes >> 8) <= 0)
			return 0;
		args.SetCondArg(0, numTimes - (1 << 8));
	}
	else { // normal animal or plant
		if ((numTimes & 0xFF) <= 0)
			return 0;
		args.SetCondArg(0, numTimes - 1);
	}

	args.SetCondArg(1, 600 * druidLvl);
	args.SetCondArg(2, protoSpec.protoId);
	initObj(protoSpec.protoId);

	return 0;
}

int ClassAbilityCallbacks::DruidWildShapeScale(DispatcherCallbackArgs args){
	auto dispIo = static_cast<DispIoMoveSpeed*>(args.dispIO);
	dispIo->AssertType(dispIOTypeMoveSpeed);

	auto protoId = d20Sys.d20Query(args.objHndCaller, DK_QUE_Polymorphed);
	if (!protoId)
		return 0;
	auto protoHandle = gameSystems->GetObj().GetProtoHandle(protoId);
	if (!protoHandle)
		return 0;

	auto protoScale = gameSystems->GetObj().GetObject(protoHandle)->GetInt32(obj_f_model_scale);
	//auto normalScale = gameSystems->GetObj().GetObject(args.objHndCaller)->GetInt32(obj_f_model_scale);

	dispIo->bonlist->bonusEntries[0].bonValue = protoScale; // modifies the initial value


	return 0;
}

// ************************************
// *         Bardic Music             *
// ************************************

#pragma region Bardic Music

int ClassAbilityCallbacks::BardicMusicBeginRound(DispatcherCallbackArgs args){
	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
	auto bmType = (BardicMusicSongType)args.GetCondArg(1);
	if (!bmType)
		return 0;

	auto roundsLasted = args.GetCondArg(2);
	if (dispIo->data1 <= 1){
		args.SetCondArg(2, roundsLasted + 1);
		auto tgt = args.GetCondArgObjHndl(3);
		if (!gameSystems->GetObj().IsValidHandle(tgt)) {
			tgt = objHndl::null;
		}
		switch (bmType)
		{
		case BM_INSPIRE_GREATNESS:
			if (tgt) {
				int bonusRounds = d20Sys.D20QueryPython(args.objHndCaller, "Bardic Ability Duration Bonus");
				conds.AddTo(tgt, "Greatness", { bonusRounds + 5,0,0,0 });
			}
			return 0;
		case BM_INSPIRE_COURAGE: 
			party.ApplyConditionAround(args.objHndCaller, 30, "Inspired_Courage", objHndl::null);
			return 0;
		case BM_COUNTER_SONG: 
			party.ApplyConditionAround(args.objHndCaller, 30, "Countersong", objHndl::null);
			return 0;
		case BM_FASCINATE: 
			if (tgt)
				conds.AddTo(tgt, "Fascinate", {-1, 0});
			return 0;
		case BM_INSPIRE_COMPETENCE: 
			if (tgt)
				conds.AddTo(tgt, "Competence", {0,0});
			return 0;
		case BM_SUGGESTION: 
			//args.SetCondArg(1,0);
			return 0;
		case BM_SONG_OF_FREEDOM: break; // TODO
		case BM_INSPIRE_HEROICS: 
			if (tgt) {
				int bonusRounds = d20Sys.D20QueryPython(args.objHndCaller, "Bardic Ability Duration Bonus");
				conds.AddTo(tgt, "Inspired Heroics", { bonusRounds + 5,0,0,0 });
			}
		default: break;
		}

	} 
	else
	{
		args.SetCondArg(2, 0);
		args.SetCondArg(1, 0);
		auto psId = args.GetCondArg(5);
		if (psId && psId != -1)
			gameSystems->GetParticleSys().End(psId);
		auto tgt = args.GetCondArgObjHndl(3);
		if (gameSystems->GetObj().IsValidHandle(tgt)) {
			d20Sys.d20SendSignal(tgt, DK_SIG_Bardic_Music_Completed, 0 ,0);
		}
	}
	return 0;
}

int ClassAbilityCallbacks::BardMusicRadial(DispatcherCallbackArgs args){
	auto perfSkill = critterSys.SkillBaseGet(args.objHndCaller, SkillEnum::skill_perform);
	auto bardLvl = objects.StatLevelGet(args.objHndCaller, stat_level_bard);
	if (!bardLvl || perfSkill < 3)
		return 0;

	//Ask python for the maximum number of uses of bardic music
	int nMaxBardicMusic = d20Sys.D20QueryPython(args.objHndCaller, "Max Bardic Music");

	RadialMenuEntryParent bmusic(5039);
	bmusic.flags |= 0x6;
	bmusic.minArg = args.GetCondArg(0);
	bmusic.maxArg = nMaxBardicMusic;
	auto bmusicId = bmusic.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::Class);

	RadialMenuEntryAction insCourage(5040, D20A_BARDIC_MUSIC, BM_INSPIRE_COURAGE, "TAG_CLASS_FEATURES_BARD_INSPIRE_COURAGE");
	insCourage.AddAsChild(args.objHndCaller, bmusicId);

	RadialMenuEntryAction counterSong(5043, D20A_BARDIC_MUSIC, BM_COUNTER_SONG, "TAG_CLASS_FEATURES_BARD_COUNTERSONG");
	counterSong.AddAsChild(args.objHndCaller, bmusicId);

	{ // putting this in brackets to prevent copy paste error, grrr
		RadialMenuEntryAction fasci(5042, D20A_BARDIC_MUSIC, BM_FASCINATE, "TAG_CLASS_FEATURES_BARD_FASCINATE");
		//RadialMenuEntryPythonAction fasci(5042, D20A_PYTHON_ACTION, 801, 0, "TAG_CLASS_FEATURES_BARD_FASCINATE");
		fasci.d20SpellData.Set(3003, spellSys.GetSpellClass(stat_level_bard), 1, -1, 0); // Spell 3003 - Bardic Fascinate
		fasci.AddAsChild(args.objHndCaller, bmusicId);
	}

	if (bardLvl >= 3 && perfSkill >= 6){	
		RadialMenuEntryAction insCompetence(5041, D20A_BARDIC_MUSIC, BM_INSPIRE_COMPETENCE, "TAG_CLASS_FEATURES_BARD_INSPIRE_COMPETENCE");
		insCompetence.d20SpellData.Set(3004, spellSys.GetSpellClass(stat_level_bard), 1, -1, 0); // Spell 3004 - Bardic Inspire Competence
		insCompetence.AddAsChild(args.objHndCaller, bmusicId);
	}

	if (bardLvl >= 6 && perfSkill >= 9) {
		RadialMenuEntryAction bardSugg(5121, D20A_BARDIC_MUSIC, BM_SUGGESTION, "TAG_CLASS_FEATURES_BARD_SUGGESTION");
		if (bardLvl >= 18 && perfSkill >= 21)
			bardSugg.d20SpellData.Set(3006, spellSys.GetSpellClass(stat_level_bard), 1, -1, 0); // Spell 3006 - Bard Suggestion Mass
		else
			bardSugg.d20SpellData.Set(3000, spellSys.GetSpellClass(stat_level_bard), 1, -1, 0); // Spell 3000 - Bard Suggestion
		bardSugg.AddAsChild(args.objHndCaller, bmusicId);
	}

	if (bardLvl >= 9 && perfSkill >= 12){
		RadialMenuEntryAction insGreatness(5044, D20A_BARDIC_MUSIC, BM_INSPIRE_GREATNESS, "TAG_CLASS_FEATURES_BARD_INSPIRE_GREATNESS");
		insGreatness.d20SpellData.Set(3002, spellSys.GetSpellClass(stat_level_bard), 1, -1, 0); // Spell 3002 - Bardic Inspire Greatness
		insGreatness.AddAsChild(args.objHndCaller, bmusicId);
	}
	

	if (bardLvl >= 12 && perfSkill >= 15) {
		RadialMenuEntryAction songOfFreedom(5119, D20A_BARDIC_MUSIC, BM_SONG_OF_FREEDOM, "TAG_CLASS_FEATURES_BARD_SONG_OF_FREEDOM");
		songOfFreedom.d20SpellData.Set(3007, spellSys.GetSpellClass(stat_level_bard), 1, -1, 0); // Spell 3007 - Bardic Song of Freedom
		songOfFreedom.AddAsChild(args.objHndCaller, bmusicId);
	}

	if (bardLvl >= 15 && perfSkill >= 18) {
		RadialMenuEntryAction insHeroics(5120, D20A_BARDIC_MUSIC, BM_INSPIRE_HEROICS, "TAG_CLASS_FEATURES_BARD_INSPIRE_HEROICS");
		insHeroics.d20SpellData.Set(3005, spellSys.GetSpellClass(stat_level_bard), 1, -1, 0); // Spell 3005 - Bardic Inspire Heroics
		insHeroics.AddAsChild(args.objHndCaller, bmusicId);
	}




	return 0;
}

int ClassAbilityCallbacks::BardMusicCheck(DispatcherCallbackArgs args){
	auto dispIo = static_cast<DispIoD20ActionTurnBased*>(args.dispIO);
	dispIo->AssertType(dispIOTypeD20ActionTurnBased);

	auto d20a = dispIo->d20a;
	auto perfSkill = critterSys.SkillBaseGet(args.objHndCaller, SkillEnum::skill_perform);
	auto bmType = (BardicMusicSongType)(d20a->data1);
	auto perfSkillSufficient = [bmType, perfSkill]()->bool{
		switch (bmType)	{
		case BM_INSPIRE_COURAGE:
		case BM_COUNTER_SONG:
		case BM_FASCINATE:
			return perfSkill >= 3;
		case BM_INSPIRE_COMPETENCE: 
			return perfSkill >= 6;
		case BM_SUGGESTION: 
			return perfSkill >= 9;
		case BM_INSPIRE_GREATNESS: 
			return perfSkill >= 12;
		case BM_SONG_OF_FREEDOM: 
			return perfSkill >= 15;
		case BM_INSPIRE_HEROICS: 
			return perfSkill >= 18;
		default: 
			return false;
		}
	};
	
	if (!perfSkillSufficient() || (args.GetCondArg(1) == bmType && bmType != BM_SUGGESTION && bmType != BM_SONG_OF_FREEDOM)){
		dispIo->returnVal = AEC_INVALID_ACTION;
		if (args.GetCondArg(1) == bmType)
			floatSys.floatMesLine(args.objHndCaller, 1, FloatLineColor::Red, fmt::format("Already Singing").c_str());
		return 0;
	}

	if (args.GetCondArg(0) <= 0){
		dispIo->returnVal = AEC_OUT_OF_CHARGES;
	}

	return 0;
}

int ClassAbilityCallbacks::BardMusicActionFrame(DispatcherCallbackArgs args){
	auto dispIo = static_cast<DispIoD20ActionTurnBased*>(args.dispIO);
	dispIo->AssertType(dispIOTypeD20ActionTurnBased);

	

	auto d20a = dispIo->d20a;
	auto performer = d20a->d20APerformer;
	auto perfSkill = critterSys.SkillBaseGet(args.objHndCaller, SkillEnum::skill_perform);
	auto bmType = (BardicMusicSongType)(d20a->data1);

	/*
	 decrease usages left, except for Suggestion
	*/
	if (bmType != BM_SUGGESTION)
		args.SetCondArg(0, args.GetCondArg(0) - 1); 

	/*
	handle already performing music
	*/
	if (args.GetCondArg(1)){
		args.SetCondArg(1, 0);
		auto partsysId = args.GetCondArg(5);
		gameSystems->GetParticleSys().End(partsysId);
		auto objHnd = args.GetCondArgObjHndl(3);
		if (gameSystems->GetObj().IsValidHandle(objHnd) && bmType != BM_SUGGESTION){ // make an exception for Suggestion since it shouldn't abort the Fascinate song
			d20Sys.d20SendSignal(objHnd, DK_SIG_Bardic_Music_Completed, 0,0);
		}
	}

	auto partsysId = 0, rollResult =0, chaScore = 0, spellId = 0;
	auto &curSeq = *actSeqSys.actSeqCur;
	switch (bmType){
	case BM_INSPIRE_COURAGE: 
		party.ApplyConditionAround(args.objHndCaller, 30.0, "Inspired_Courage", objHndl::null);
		partsysId = gameSystems->GetParticleSys().CreateAtObj("Bardic-Inspire Courage", args.objHndCaller);
		break;
	case BM_COUNTER_SONG: 
		party.ApplyConditionAround(args.objHndCaller, 30.0, "Countersong", objHndl::null);
		partsysId = gameSystems->GetParticleSys().CreateAtObj("Bardic-Countersong", args.objHndCaller);
		break;
	case BM_FASCINATE: 
		partsysId = gameSystems->GetParticleSys().CreateAtObj("Bardic-Fascinate", args.objHndCaller);
		spellId = spellSys.GetNewSpellId();
		spellSys.RegisterSpell(curSeq->spellPktBody, spellId);
		pySpellIntegration.SpellTrigger(spellId, SpellEvent::SpellEffect);
		// effect now handled via spell
		//skillSys.SkillRoll(performer, SkillEnum::skill_perform, 20, &rollResult, 1);
		//	if (!damage.SavingThrow(d20a->d20ATarget, performer, rollResult, SavingThrowType::Will, D20STF_SPELL_DESCRIPTOR_SONIC ))	{ // might be an offset in the descriptors... should probably be MIND_AFFECTING
		//		conds.AddTo(d20a->d20ATarget, "Fascinate", { 0,0 });
		//	}		
		break;
	case BM_INSPIRE_COMPETENCE: 
		conds.AddTo(curSeq->spellPktBody.targetListHandles[0], "Competence", {0,0});
		partsysId = gameSystems->GetParticleSys().CreateAtObj("Bardic-Inspire Competence", args.objHndCaller);
		break;
	case BM_SUGGESTION: 
		partsysId = gameSystems->GetParticleSys().CreateAtObj("Bardic-Suggestion", args.objHndCaller);
		spellId = spellSys.GetNewSpellId();
		spellSys.RegisterSpell(curSeq->spellPktBody, spellId);
		pySpellIntegration.SpellTrigger(spellId, SpellEvent::SpellEffect);
		break;
	case BM_INSPIRE_GREATNESS: 
		//conds.AddTo(d20a->d20ATarget, "Greatness", {}); // effect now handled via spell
		partsysId = gameSystems->GetParticleSys().CreateAtObj("Bardic-Inspire Greatness", args.objHndCaller); 
		spellId = spellSys.GetNewSpellId();
		spellSys.RegisterSpell(curSeq->spellPktBody, spellId);
		pySpellIntegration.SpellTrigger(spellId, SpellEvent::SpellEffect);
		break;
	case BM_SONG_OF_FREEDOM:
		//partsysId = gameSystems->GetParticleSys().CreateAtObj("Bardic-Inspire Courage", args.objHndCaller);
		spellId = spellSys.GetNewSpellId();
		spellSys.RegisterSpell(curSeq->spellPktBody, spellId);
		pySpellIntegration.SpellTrigger(spellId, SpellEvent::SpellEffect);
		break;
	case BM_INSPIRE_HEROICS: 
		//conds.AddTo(d20a->d20ATarget, "Inspired Heroics", {});
		partsysId = gameSystems->GetParticleSys().CreateAtObj("Bardic-Inspire Courage", args.objHndCaller);
		spellId = spellSys.GetNewSpellId();
		spellSys.RegisterSpell(curSeq->spellPktBody, spellId);
		pySpellIntegration.SpellTrigger(spellId, SpellEvent::SpellEffect);
		break;
	default: 
		break;
	}

	static int bardicMusicSounds []= {0, 20040, 20000, 20020, 20060, 20080, 20060, 20060 , 20040 };
	auto instrType = d20Sys.d20Query(performer, DK_QUE_BardicInstrument);
	sound.PlaySoundAtObj(bardicMusicSounds[bmType] + instrType, performer);
	args.SetCondArg(1, bmType);
	args.SetCondArg(2, 0);
	args.SetCondArg(3, d20a->d20ATarget.GetHandleUpper());
	args.SetCondArg(4, d20a->d20ATarget.GetHandleLower());
	args.SetCondArg(5, partsysId);
	dispIo->returnVal = 0;
	return 0;
}

int ClassAbilityCallbacks::BardicMusicInspireRefresh(DispatcherCallbackArgs args){
	GET_DISPIO(dispIoTypeCondStruct, DispIoCondStruct);
	if (dispIo->condStruct == (CondStruct*)args.GetData1()){
		args.SetCondArg(0, dispIo->arg1); // set duration to the old condition value
		args.SetCondArg(1, 1);
		dispIo->outputFlag = 0; // prevent adding a duplicate entry
	}
	return 0;
}

int ClassAbilityCallbacks::BardicMusicInspireBeginRound(DispatcherCallbackArgs args){
	GET_DISPIO(dispIoTypeSendSignal, DispIoD20Signal);
	auto durRemn = args.GetCondArg(0) - (int)dispIo->data1;
	if (durRemn >= 0){
		args.SetCondArg(0, durRemn);
	} else
	{
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
	}
	return 0;
}

int ClassAbilityCallbacks::BardicMusicInspireOnAdd(DispatcherCallbackArgs args)
{
	if (args.GetData2() == BM_INSPIRE_HEROICS)
		args.SetCondArg(1, 0); // enabler flag; gets set to 1 on the Bard's turn (since it requires hearing the bard for a full round)
	else
		args.SetCondArg(1, 1); 
	if (args.GetData2() == BM_INSPIRE_GREATNESS){
		auto conScore = objects.StatLevelGet(args.objHndCaller, stat_constitution);
		auto conMod = objects.GetModFromStatLevel(conScore);
		auto rollRes = Dice(2, 10, conMod).Roll();
		args.SetCondArg(3, rollRes);
	}
	
	return 0;
}

int ClassAbilityCallbacks::BardicMusicGreatnessToHitBonus(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeAttackBonus, DispIoAttackBonus);
	dispIo->bonlist.AddBonus(2, 34, 194);
	
	return 0;
}

int ClassAbilityCallbacks::BardicMusicGreatnessSaveBonus(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeSavingThrow, DispIoSavingThrow);
	dispIo->bonlist.AddBonus(1, 34, 194);
	
	return 0;
}

int ClassAbilityCallbacks::BardicMusicTooltip(DispatcherCallbackArgs args){
	GET_DISPIO(dispIoTypeTooltip, DispIoTooltip);
	if (args.GetData2() != BM_INSPIRE_HEROICS || args.GetCondArg(1)){
		auto text = fmt::format("{}", combatSys.GetCombatMesLine(args.GetData1()));
		dispIo->Append(text);
	}

	if (args.GetData2() == BM_INSPIRE_GREATNESS) { // Temp HP:%d
		if (args.GetCondArg(3)){
			auto text = fmt::format("{}{}", combatSys.GetCombatMesLine(74), args.GetCondArg(3));
			dispIo->Append(text);
		}	
	}
	return 0;
}

int ClassAbilityCallbacks::BardicMusicEffectTooltip(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeEffectTooltip, DispIoEffectTooltip);
	auto bmtype = (BardicMusicSongType)args.GetData2();
	if (bmtype == BM_INSPIRE_HEROICS){
		dispIo->Append(ElfHash::Hash("INSPIRE_HEROICS"), -1, "Inspire Heroics");
	}
	return 0;
}

int ClassAbilityCallbacks::BardicMusicGreatnessTakingTempHpDamage(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeDamage, DispIoDamage);
	auto tempHp = args.GetCondArg(3);

	auto finalDam = dispIo->damage.finalDamage;

	if (tempHp <= 0 || finalDam <= 0)
		return 0;

	logger->debug("Took {} damage, temp_hp = {}", finalDam, tempHp);
	auto hpLeft = tempHp - finalDam;
	if (hpLeft < 0)
		hpLeft = 0;
	args.SetCondArg(3, hpLeft);
	if (hpLeft <= 0){
		floatSys.FloatCombatLineWithExtraString(args.objHndCaller, 50, fmt::format("[{}]", tempHp),"");
		dispIo->damage.AddDamageBonus(-tempHp, 0, 154);
		d20Sys.d20SendSignal(args.objHndCaller, DK_SIG_Temporary_Hit_Points_Removed, args.GetCondArg(0), 0);
		dispIo->damage.finalDamage -= tempHp;
		return 0;
	}

	dispIo->damage.AddDamageBonus(-finalDam, 0, 154);
	dispIo->damage.finalDamage = 0;
	floatSys.FloatCombatLineWithExtraString(args.objHndCaller, 50, fmt::format("[{}]", finalDam), "");
	return 0;
}

int ClassAbilityCallbacks::BardicMusicHeroicsSaveBonus(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeSavingThrow, DispIoSavingThrow);
	if (args.GetCondArg(1)){ // have heard music for full round
		dispIo->bonlist.AddBonus(4, 13, 194);
	}
	return 0;
}

int ClassAbilityCallbacks::BardicMusicHeroicsAC(DispatcherCallbackArgs args){
	GET_DISPIO(dispIOTypeAttackBonus, DispIoAttackBonus);
	if (args.GetCondArg(1)){ // have heard music for full round
		dispIo->bonlist.AddBonus(4, 13, 348);
	}
	return 0;
}

int ClassAbilityCallbacks::BardicMusicSuggestionFearQuery(DispatcherCallbackArgs args){
	auto spellId = args.GetCondArg(2);
	SpellPacketBody spPkt(spellId);
	auto caster = spPkt.caster;
	if (!caster){
		args.SetCondArg(0, 0); // set duration to 0
		return 0;
	}
	if (d20Sys.d20QueryWithData(args.objHndCaller, DK_QUE_Critter_Has_Condition, conds.GetByName("sp-Calm Emotions"), 0u))
		return 0;
	GET_DISPIO(dispIOTypeQuery, DispIoD20Query);
	dispIo->return_val = 1;
	*(objHndl*)&dispIo->data1 = caster;

	return 0;
}

#pragma endregion



int ClassAbilityCallbacks::SneakAttackDamage(DispatcherCallbackArgs args) {
	GET_DISPIO(dispIOTypeDamage, DispIoDamage);
	auto &atkPkt = dispIo->attackPacket;
	auto tgt = atkPkt.victim;

	if (!tgt)
		return 0;

	if (!atkPkt.attacker) {
		logger->error("SneakAttackDamage: Error! Null attacker in attack packet");
		return 0;
	}

	// imprecise attacks cannot sneak attack
	if (atkPkt.flags & D20CAF_NO_PRECISION_DAMAGE)
		return 0;

	// limit to 30'
	if (locSys.DistanceToObj(args.objHndCaller, tgt) > 30)
		return 0;

	// See if it is a critical and if criticals cause sneak attacks
	bool sneakAttackFromCrit = false;
	if (atkPkt.flags & D20CAF_CRITICAL) {
		auto result = d20Sys.D20QueryPython(args.objHndCaller, "Sneak Attack Critical");
		if (result > 0) {
			sneakAttackFromCrit = true;
		}
	}

	if (atkPkt.flags & D20CAF_FLANKED
		|| d20Sys.d20Query(tgt, DK_QUE_SneakAttack)
		|| d20Sys.d20QueryWithData(atkPkt.attacker, DK_QUE_OpponentSneakAttack, (uint32_t)dispIo, 0)
		|| !critterSys.CanSense(tgt, atkPkt.attacker)
		|| sneakAttackFromCrit)
	{
		// get sneak attack dice (NEW! now via query, for prestige class modularity)
		auto sneakAttackDice = d20Sys.D20QueryPython(args.objHndCaller, fmt::format("Sneak Attack Dice"));
		auto sneakAttackDmgBonus = d20Sys.D20QueryPython(args.objHndCaller, fmt::format("Sneak Attack Bonus"));

		if (sneakAttackDice <= 0)
			return 0;

		if (d20Sys.d20Query(tgt, DK_QUE_Critter_Is_Immune_Critical_Hits))	{
			dispIo->damage.bonuses.ZeroBonusSetMeslineNum(325);
			return 0;
		}

		
		auto sneakDmgDice = Dice(sneakAttackDice, 6, sneakAttackDmgBonus);
		if (feats.HasFeatCountByClass(args.objHndCaller, FEAT_DEADLY_PRECISION)) {
			sneakDmgDice = Dice(sneakAttackDice, 5, 1*sneakAttackDice + sneakAttackDmgBonus);
		}
		dispIo->damage.AddDamageDice(sneakDmgDice.ToPacked(), DamageType::Unspecified, 106);
		floatSys.FloatCombatLine(args.objHndCaller, 90); // Sneak Attack!
		histSys.CreateRollHistoryLineFromMesfile(26, args.objHndCaller, tgt);

		d20Sys.D20SignalPython(args.objHndCaller, "Sneak Attack Damage Applied");

		// crippling strike ability loss
		if (feats.HasFeatCountByClass(args.objHndCaller, FEAT_CRIPPLING_STRIKE)){
			histSys.CreateRollHistoryLineFromMesfile(47, args.objHndCaller, tgt);
			conds.AddTo(tgt, "Damage_Ability_Loss", { 0, 2 }); // note: vanilla had a bug (did 1 damage instead of 2)
			floatSys.FloatCombatLine(args.objHndCaller, 96); // Ability Loss
		}
	}
	

	return 0;
}

int ClassAbilityCallbacks::FeatCraftRodRadial(DispatcherCallbackArgs args){
	return ItemCreationBuildRadialMenuEntry(args, CraftRod, "TAG_CRAFT_ROD", 5069);
};
#pragma endregion

int CaptivatingSongOnConditionAdd(DispatcherCallbackArgs args)
{
	
	int spellId = conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	// int duration = conds.CondNodeGetArg(args.subDispNode->condNode, 1);
	int duration = 2;
	if (!spellId)
		return 0;
	SpellPacketBody spellPktBody;
	spellSys.GetSpellPacketBody(spellId, &spellPktBody);
	objHndl singer = spellPktBody.caster;
	ObjectId singerId = objects.GetId(singer);
	memcpy(&args.subDispNode->condNode->args[2], &singerId, sizeof(ObjectId));
	std::vector<int> argg = { duration,0, 0,0,0,0,0,0 };
	//conds.AddTo(args.objHndCaller, "Captivated", { duration,0, 0,0,0,0,0,0 });
	memcpy(&argg[2], &singerId, sizeof(ObjectId));
	conds.AddTo(args.objHndCaller, "Captivated", argg);
	return 0;
}


void Conditions::AddConditionsToTable(){

	static CondStructNew itemSkillBonus("Special Equipment Skill Bonus", 3, false);
	itemSkillBonus.AddHook(dispTypeSkillLevel, DK_SKILL_APPRAISE, itemCallbacks.SkillBonus, 99, 0);

	static CondStructNew diamondSoul("Diamond Soul", 3);
	diamondSoul.AddHook(dispTypeConditionAdd, DK_NONE, classAbilityCallbacks.FeatDiamondSoulInit);
	diamondSoul.AddHook(dispTypeSpellResistanceMod, DK_NONE, classAbilityCallbacks.FeatDiamondSoulSpellResistanceMod);
	diamondSoul.AddHook(dispTypeD20Query, DK_QUE_Critter_Has_Spell_Resistance, genericCallbacks.SpellResistanceQuery);
	diamondSoul.AddHook(dispTypeTooltip, DK_NONE, genericCallbacks.SpellResistanceTooltip, 5048, 0);
	diamondSoul.AddToFeatDictionary(FEAT_MONK_DIAMOND_SOUL);

	static CondStructNew perfectSelf("Perfect Self", 3);
	perfectSelf.AddHook(dispTypeTakingDamage2, DK_NONE, classAbilityCallbacks.FeatDamageReduction, 0x4, 0); // 0x4 denotes Magical attacks
	perfectSelf.AddToFeatDictionary(FEAT_MONK_PERFECT_SELF, FEAT_INVALID, 10);


	static CondStructNew emptyBody("Empty Body", 3);
	emptyBody.AddHook(dispTypeRadialMenuEntry, DK_NONE, classAbilityCallbacks.FeatEmptyBody);
	emptyBody.AddHook(dispTypeD20Query, DK_QUE_Empty_Body_Num_Rounds, classAbilityCallbacks.GetNumRoundsRemaining);
	emptyBody.AddHook(dispTypeConditionAdd, DK_NONE, classAbilityCallbacks.FeatEmptyBodyInit);
	emptyBody.AddHook(dispTypeNewDay, DK_NEWDAY_REST, classAbilityCallbacks.FeatEmptyBodyInit);
	emptyBody.AddHook(dispTypeD20ActionPerform, DK_D20A_EMPTY_BODY, classAbilityCallbacks.FeatEmptyBodyReduceRounds);
	emptyBody.AddToFeatDictionary(FEAT_MONK_EMPTY_BODY, FEAT_INVALID, -1);

	static CondStructNew ethereal("Ethereal", 3);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_Is_Ethereal, genericCallbacks.QuerySetReturnVal1);
	ethereal.AddHook(dispTypeBeginRound, DK_NONE, classAbilityCallbacks.TimedEffectCountdown, 1, 0);
	ethereal.AddHook(dispTypeTakingDamage2, DK_NONE, genericCallbacks.AddEtherealDamageImmunity);
	ethereal.AddHook(dispTypeDealingDamage2, DK_NONE, genericCallbacks.EtherealDamageDealingNull);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_AOOPossible, genericCallbacks.QuerySetReturnVal0);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_AOOWillTake, genericCallbacks.QuerySetReturnVal0);
	ethereal.AddHook(dispTypeEffectTooltip, DK_NONE, genericCallbacks.EffectTooltipDuration, 82, 210);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_Critter_Is_Invisible, genericCallbacks.QuerySetReturnVal1);
	ethereal.AddHook(dispTypeConditionAdd, DK_NONE, genericCallbacks.EtherealOnAdd);
	ethereal.AddHook(dispTypeConditionAddFromD20StatusInit, DK_NONE, genericCallbacks.EtherealOnD20StatusInit);
	ethereal.AddHook(dispTypeConditionRemove, DK_NONE, genericCallbacks.EtherealOnRemove);
	ethereal.AddHook(dispTypeTooltip, DK_NONE, genericCallbacks.TooltipUnrepeated, 210, 0);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_IsActionInvalid_CheckAction, genericCallbacks.ActionInvalidQueryTrue);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_Critter_Is_Immune_Poison, genericCallbacks.QuerySetReturnVal1);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_AOOWillTake, genericCallbacks.QuerySetReturnVal0);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_AOOIncurs, genericCallbacks.QuerySetReturnVal0);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_ActionTriggersAOO,genericCallbacks.QuerySetReturnVal0);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_Critter_Has_Freedom_of_Movement, genericCallbacks.QuerySetReturnVal1);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_CanBeAffected_PerformAction, genericCallbacks.QuerySetReturnVal0);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_CanBeAffected_ActionFrame, genericCallbacks.QuerySetReturnVal0);


	static CondStructNew quivPalm("Quivering Palm", 3 );
	quivPalm.AddHook(dispTypeRadialMenuEntry, DK_NONE, classAbilityCallbacks.FeatQuiveringPalmRadial);
	quivPalm.AddHook(dispTypeD20Query, DK_QUE_Quivering_Palm_Can_Perform, classAbilityCallbacks.FeatQuiveringPalmAvailable);
	quivPalm.AddHook(dispTypeConditionAdd, DK_NONE, classAbilityCallbacks.FeatQuiveringPalmInit);
	quivPalm.AddHook(dispTypeNewDay, DK_NEWDAY_REST, classAbilityCallbacks.FeatQuiveringPalmInit);
	quivPalm.AddHook(dispTypeD20ActionPerform, DK_D20A_QUIVERING_PALM, classAbilityCallbacks.FeatQuiveringPalmPerform);
	quivPalm.AddToFeatDictionary(FEAT_MONK_QUIVERING_PALM);

	static CondStructNew improvedTrip("Improved Trip", 2);
	improvedTrip.AddHook(dispTypeRadialMenuEntry, DK_NONE, genericCallbacks.TripAooRadial);
	improvedTrip.AddHook(dispTypeD20Query, DK_QUE_Trip_AOO, genericCallbacks.TripAooQuery);
	improvedTrip.AddHook(dispTypeAbilityCheckModifier, DK_NONE, genericCallbacks.ImprovedTripBonus);
	improvedTrip.AddToFeatDictionary(FEAT_IMPROVED_TRIP);

	
	

	static CondStructNew weaponSeeking("Weapon Seeking", 3, false);
	weaponSeeking.AddHook(dispTypeGetAttackerConcealmentMissChance, DK_NONE, itemCallbacks.WeaponSeekingAttackerConcealmentMissChance);
	weaponSeeking.AddHook(dispTypeD20Query, DK_QUE_Critter_Has_Condition, genericCallbacks.HasCondition, &weaponSeeking, 0);

	static CondStructNew weaponSpeed("Weapon Speed", 3, false);
	weaponSpeed.AddHook(dispTypeGetBonusAttacks, DK_NONE, itemCallbacks.WeaponSpeed);

	static CondStructNew weaponThundering("Weapon Thundering", 4, false);
	weaponThundering.AddHook(dispTypeDealingDamage, DK_NONE, itemCallbacks.WeaponThundering);

	static CondStructNew weaponVicious("Weapon Vicious", 3, false);
	weaponVicious.AddHook(dispTypeDealingDamage, DK_NONE, itemCallbacks.WeaponVicious);
	weaponVicious.AddHook(dispTypeDealingDamage2, DK_NONE, itemCallbacks.WeaponViciousBlowback);

	static CondStructNew weaponWounding("Weapon Wounding", 3, false);
	weaponWounding.AddHook(dispTypeDealingDamage2, DK_NONE, itemCallbacks.WeaponWounding);

	static CondStructNew weaponMerciful("Weapon Merciful", 4, false);
	weaponMerciful.AddHook(dispTypeDealingDamage, DK_NONE, itemCallbacks.WeaponMerciful);

	static CondStructNew wildShape("Wild Shape", 3); // mainly to replace the lack of D20StatusInit callback
	wildShape.AddHook(dispTypeRadialMenuEntry, DK_NONE, classAbilityCallbacks.DruidWildShapeInit); // otherwise you go into init hell
	wildShape.AddToFeatDictionary(FEAT_WILD_SHAPE);

	static CondStructNew wildShaped("Wild Shaped", 3); // because the wild shape args get overwritten on each init
	wildShaped.AddHook(dispTypeConditionAdd, DK_NONE, classAbilityCallbacks.DruidWildShapedInit);
	wildShaped.AddHook(dispTypeRadialMenuEntry, DK_NONE, classAbilityCallbacks.DruidWildShapeRadialMenu);
	wildShaped.AddHook(dispTypeConditionAddFromD20StatusInit, DK_NONE, classAbilityCallbacks.DruidWildShapeD20StatusInit); // takes care of resetting the item conditions
	wildShaped.AddHook(dispTypeD20ActionCheck, DK_D20A_CLASS_ABILITY_SA, classAbilityCallbacks.DruidWildShapeCheck);
	wildShaped.AddHook(dispTypeD20ActionPerform, DK_D20A_CLASS_ABILITY_SA, classAbilityCallbacks.DruidWildShapePerform);
	wildShaped.AddHook(dispTypeNewDay, DK_NEWDAY_REST, classAbilityCallbacks.DruidWildShapeReset);
	wildShaped.AddHook(dispTypeBeginRound, DK_NONE, temple::GetRef<DispCB>(0x100FBE70));
	wildShaped.AddHook(dispTypeAbilityScoreLevel, DK_STAT_STRENGTH, temple::GetRef<DispCB>(0x100FBF30));
	wildShaped.AddHook(dispTypeAbilityScoreLevel, DK_STAT_DEXTERITY, temple::GetRef<DispCB>(0x100FBF30));
	wildShaped.AddHook(dispTypeAbilityScoreLevel, DK_STAT_CONSTITUTION, temple::GetRef<DispCB>(0x100FBF30));
	wildShaped.AddHook(dispTypeD20Query, DK_QUE_Polymorphed, temple::GetRef<DispCB>(0x100FBF90));
	wildShaped.AddHook(dispTypeGetCritterNaturalAttacksNum, DK_NONE, classAbilityCallbacks.DruidWildShapeGetNumAttacks);
	wildShaped.AddHook(dispTypeD20Query, DK_QUE_CannotCast, temple::GetRef<DispCB>(0x100FBFC0));
	wildShaped.AddHook(dispTypeGetModelScale, DK_NONE, classAbilityCallbacks.DruidWildShapeScale); // NEW! scales the model too


	static CondStructNew ironWill("Iron Will", 1);
	ironWill.AddHook(dispTypeSaveThrowLevel, DK_SAVE_WILL, classAbilityCallbacks.FeatIronWillSave, 2, 0);
	ironWill.AddToFeatDictionary(FEAT_IRON_WILL);


	{
		static CondStructNew bardicGreatness("Greatness", 4);
		bardicGreatness.AddHook(dispTypeConditionAddPre, DK_NONE, classAbilityCallbacks.BardicMusicInspireRefresh, &bardicGreatness, 0);
		bardicGreatness.AddHook(dispTypeBeginRound, DK_NONE, classAbilityCallbacks.BardicMusicInspireBeginRound, 0u, BM_INSPIRE_GREATNESS);
		bardicGreatness.AddHook(dispTypeConditionAdd, DK_NONE, classAbilityCallbacks.BardicMusicInspireOnAdd, 0u, BM_INSPIRE_GREATNESS);
		bardicGreatness.AddHook(dispTypeToHitBonus2, DK_NONE, classAbilityCallbacks.BardicMusicGreatnessToHitBonus);
		bardicGreatness.AddHook(dispTypeSaveThrowLevel, DK_SAVE_FORTITUDE, classAbilityCallbacks.BardicMusicGreatnessSaveBonus);
		bardicGreatness.AddHook(dispTypeTooltip, DK_NONE, classAbilityCallbacks.BardicMusicTooltip, 82, BM_INSPIRE_GREATNESS);
		bardicGreatness.AddHook(dispTypeD20Query, DK_QUE_Has_Temporary_Hit_Points, genericCallbacks.QuerySetReturnVal1);
		bardicGreatness.AddHook(dispTypeEffectTooltip, DK_NONE, genericCallbacks.EffectTooltipGeneral, 5, 0);
		bardicGreatness.AddHook(dispTypeConditionRemove, DK_NONE, genericCallbacks.EndParticlesFromArg, 2, 0);
		bardicGreatness.AddHook(dispTypeConditionAddFromD20StatusInit, DK_NONE, genericCallbacks.PlayParticlesSavePartsysId, 2, (uint32_t)"Bardic-Inspire Greatness-hit");
		bardicGreatness.AddHook(dispTypeConditionAdd, DK_NONE, genericCallbacks.PlayParticlesSavePartsysId, 2, (uint32_t)"Bardic-Inspire Greatness-hit");
		bardicGreatness.AddHook(dispTypeTakingDamage2, DK_NONE, classAbilityCallbacks.BardicMusicGreatnessTakingTempHpDamage);

	}
	

	{
		static CondStructNew bardSuggestion("Bard Suggestion", 4); // Duration, particle ID, spell ID
		bardSuggestion.AddHook(dispTypeBeginRound, DK_NONE, ConditionDurationTicker, 0u, 0u);
		bardSuggestion.AddHook(dispTypeTooltip, DK_NONE, classAbilityCallbacks.BardicMusicTooltip, 52, BM_SUGGESTION);
		bardSuggestion.AddHook(dispTypeD20Query, DK_QUE_Critter_Is_Afraid, classAbilityCallbacks.BardicMusicSuggestionFearQuery);
		bardSuggestion.AddHook(dispTypeConditionAddFromD20StatusInit, DK_NONE, genericCallbacks.PlayParticlesSavePartsysId, 1, (uint32_t)"Bardic-Suggestion-hit");
		bardSuggestion.AddHook(dispTypeConditionAdd, DK_NONE, genericCallbacks.PlayParticlesSavePartsysId, 1, (uint32_t)"Bardic-Suggestion-hit");
		bardSuggestion.AddHook(dispTypeConditionRemove, DK_NONE, genericCallbacks.EndParticlesFromArg, 1, 0);
		bardSuggestion.AddHook(dispTypeD20Signal, DK_SIG_Killed, ConditionRemoveCallback);
		bardSuggestion.AddHook(dispTypeD20Signal, DK_SIG_Teleport_Prepare, ConditionRemoveCallback);
	}

	static CondStructNew bardInspireHeroics("Inspired Heroics", 4);
	bardInspireHeroics.AddHook(dispTypeConditionAddPre, DK_NONE, classAbilityCallbacks.BardicMusicInspireRefresh, &bardInspireHeroics, 0);
	bardInspireHeroics.AddHook(dispTypeBeginRound, DK_NONE, classAbilityCallbacks.BardicMusicInspireBeginRound, 0u, BM_INSPIRE_HEROICS);
	bardInspireHeroics.AddHook(dispTypeConditionAdd, DK_NONE, classAbilityCallbacks.BardicMusicInspireOnAdd);
	bardInspireHeroics.AddHook(dispTypeGetAC, DK_NONE, classAbilityCallbacks.BardicMusicHeroicsAC);
	bardInspireHeroics.AddHook(dispTypeSaveThrowLevel, DK_NONE, classAbilityCallbacks.BardicMusicHeroicsSaveBonus);
	bardInspireHeroics.AddHook(dispTypeTooltip, DK_NONE, classAbilityCallbacks.BardicMusicTooltip, 5122, BM_INSPIRE_HEROICS);
	bardInspireHeroics.AddHook(dispTypeD20Query, DK_QUE_Has_Temporary_Hit_Points, genericCallbacks.QuerySetReturnVal1);
	bardInspireHeroics.AddHook(dispTypeEffectTooltip, DK_NONE, classAbilityCallbacks.BardicMusicEffectTooltip, 0u, BM_INSPIRE_HEROICS); // todo replace icon tooltip function
	bardInspireHeroics.AddHook(dispTypeConditionRemove, DK_NONE, genericCallbacks.EndParticlesFromArg, 2, 0);
	bardInspireHeroics.AddHook(dispTypeConditionAddFromD20StatusInit, DK_NONE, genericCallbacks.PlayParticlesSavePartsysId, 2, (uint32_t)"Bardic-Inspire Courage-hit"); // todo make new pfx
	bardInspireHeroics.AddHook(dispTypeConditionAdd, DK_NONE, genericCallbacks.PlayParticlesSavePartsysId, 2, (uint32_t)"Bardic-Inspire Courage-hit");


	// New Conditions!
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mConditionDisableAoO);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondGreaterTwoWeaponFighting);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondGreaterTWFRanger);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondDivineMight);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondDivineMightBonus);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondRecklessOffense);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondGreaterWeaponSpecialization);

	conds.hashmethods.CondStructAddToHashtable((CondStruct*)&conds.mCondDisarm);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)&conds.mCondDisarmed);
	// conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondSuperiorExpertise); // will just be patched inside Combat Expertise callbacks
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondRend);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)&conds.mCondCaptivatingSong);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)&conds.mCondCaptivated);
	// conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondCraftWandLevelSet);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondAidAnother);

	conds.hashmethods.CondStructAddToHashtable((CondStruct*)&conds.mCondHezrouStench);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)&conds.mCondHezrouStenchHit);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)&conds.mCondNecklaceOfAdaptation);
	/*
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondDeadlyPrecision);

	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondGreaterRage);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondImprovedDisarm);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondIndomitableWill);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondKnockDown);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondMightyRage);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondPersistentSpell);


	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondTirelessRage);
	*/

	auto tpModule = PyImport_ImportModule("tpModifiers");
}

int AidAnotherRadialMenu(DispatcherCallbackArgs args)
{
	//MesLine mesLine;
	RadialMenuEntry radMenuAidAnotherMain;
	
	radMenuAidAnotherMain.text = combatSys.GetCombatMesLine(5112);
	radMenuAidAnotherMain.d20ActionType = D20A_NONE;
	radMenuAidAnotherMain.d20ActionData1 = 0;
	radMenuAidAnotherMain.helpId = ElfHash::Hash("TAG_AID_ANOTHER");

	int newParent = radialMenus.AddParentChildNode(args.objHndCaller, &radMenuAidAnotherMain, radialMenus.GetStandardNode(RadialMenuStandardNode::Tactical));

	//RadialMenuEntryAction defensiveAssist(6018, D20A_ITEM_CREATION, 0, "TAG_AID_ANOTHER");
	// radialMenus.AddChildNode(args.objHndCaller, &defensiveAssist, newParent);

	RadialMenuEntryAction wakeUp(5113, D20A_AID_ANOTHER_WAKE_UP, 0, "TAG_AID_ANOTHER");
	radialMenus.AddChildNode(args.objHndCaller, &wakeUp, newParent);
	


	return 0;
}
#pragma endregion

int RaceAbilityCallbacks::HalflingThrownWeaponAndSlingBonus(DispatcherCallbackArgs args){

	GET_DISPIO(dispIOTypeAttackBonus, DispIoAttackBonus);

	if (dispIo->attackPacket.flags & D20CAF_THROWN){
		dispIo->bonlist.AddBonus(1, 0, 139);
	} 
	else
	{
		auto weapon = dispIo->attackPacket.GetWeaponUsed();
		if (weapon)
		{
			auto wpnType = (WeaponTypes)objSystem->GetObject(weapon)->GetInt32(obj_f_weapon_type);
			if (wpnType == wt_sling){
				dispIo->bonlist.AddBonus(1, 0, 139);
			}
		}
		
	}


	return 0;
}

void CondStructNew::AddAoESpellRemover() {
	AddHook(dispTypeD20Signal, DK_SIG_Spell_End, spCallbacks.AoeSpellRemove);
}