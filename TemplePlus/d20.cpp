#include "stdafx.h"
#include "common.h"
#include "d20.h"
#include "temple_functions.h"
#include "obj.h"
#include <temple/dll.h>
#include "feat.h"
#include "util/fixes.h"
#include "spell.h"
#include "dispatcher.h"
#include "condition.h"
#include "inventory.h"
#include "python/python_debug.h"
#include "pathfinding.h"
#include "location.h"
#include "action_sequence.h"
#include "critter.h"
#include "anim.h"
#include "tab_file.h"
#include "combat.h"
#include "float_line.h"
#include "weapon.h"
#include "party.h"
#include "ui/ui_dialog.h"


static_assert(sizeof(D20SpellData) == (8U), "D20SpellData structure has the wrong size!"); //shut up compiler, this is ok
static_assert(sizeof(D20Actn) == 0x58, "D20Action struct has the wrong size!");
static_assert(sizeof(D20ActionDef) == 0x30, "D20ActionDef struct has the wrong size!");


int (__cdecl *OrgD20Init)(GameSystemConf* conf);

class D20Replacements : public TempleFix {
public:
	const char* name() override {
		return "D20 Function Replacements";
	}

	void apply() override {
		
		OrgD20Init = (int(__cdecl *)(GameSystemConf* conf))replaceFunction(0x1004C8A0, _D20Init);


		replaceFunction(0x1004CA00, _D20StatusInitItemConditions);
		replaceFunction(0x1004CC00, _D20Query);
		replaceFunction(0x1004CC60, _d20QueryWithData);
		replaceFunction(0x1004CD40, _d20QueryReturnData);
		replaceFunction(0x1004DFC0, _GetAttackWeapon);

		replaceFunction(0x1004E6B0, _d20SendSignal);
		
		replaceFunction(0x1004F910, _D20StatusInitFromInternalFields);
		replaceFunction(0x1004FDB0, _D20StatusInit);
		replaceFunction(0x1004FF30, _D20StatusRefresh);
		
		replaceFunction(0x10077850, D20SpellDataExtractInfo);
		replaceFunction(0x10077830, D20SpellDataSetSpontCast);
		replaceFunction(0x10077800, _d20ActnSetSpellData); 
		

		
		replaceFunction(0x10080220, _CanLevelup);
		
		replaceFunction(0x10089F80, _globD20aSetTypeAndData1);
		replaceFunction(0x1008A450, _GlobD20ActnSetSpellData);
		replaceFunction(0x1008A530, _globD20aSetPerformer);

		replaceFunction(0x1008CE30, _PerformStandardAttack);
		
		replaceFunction(0x100949E0, _GlobD20ActnInit);
		

		
		replaceFunction(0x10093810, _D20ActnInitUsercallWrapper); // function takes esi as argument
		
		replaceFunction(0x100FD2D0, _D20StatusInitFeats);
		replaceFunction(0x100FD790, _D20StatusInitRace);
		replaceFunction(0x100FEE60, _D20StatusInitClass); 
		
	}
} d20Replacements;


static struct D20SystemAddresses : temple::AddressTable {

	void(__cdecl*  GlobD20ActnSetTarget)(objHndl objHnd, LocAndOffsets * loc);
	uint32_t(__cdecl* LocationCheckStdAttack)(D20Actn*, TurnBasedStatus*, LocAndOffsets*);
	uint32_t (__cdecl*ActionCostStandardAttack)(D20Actn *d20, TurnBasedStatus *tbStat, ActionCostPacket *acp);
	uint32_t(__cdecl*sub_1008EDF0)(D20Actn * d20a, int flags);
	uint32_t(__cdecl*AddToSeqStdAttack)(D20Actn*, ActnSeq*, TurnBasedStatus*);
	uint32_t(__cdecl*AiCheckStdAttack)(D20Actn*, TurnBasedStatus*);
	uint32_t(__cdecl*ActionCheckStdAttack)(D20Actn*, TurnBasedStatus*);
	int(__cdecl*TargetWithinReachOfLoc)(objHndl obj, objHndl target, LocAndOffsets* loc);
	D20SystemAddresses()
	{
		rebase(GlobD20ActnSetTarget,0x10092E50); 
		rebase(LocationCheckStdAttack, 0x1008C910);
		rebase(ActionCostStandardAttack, 0x100910F0);
		rebase(sub_1008EDF0, 0x1008EDF0);
		rebase(AddToSeqStdAttack, 0x100955E0);
		rebase(AiCheckStdAttack, 0x1008C4F0);
		rebase(ActionCheckStdAttack, 0x1008C910);

		rebase(TargetWithinReachOfLoc, 0x100B86C0);
	}
} addresses;

#pragma region D20System Implementation
D20System d20Sys;
D20ActionDef d20ActionDefsNew[1000];
TabFileStatus _d20actionTabFile;

void D20System::NewD20ActionsInit()
{
	tabSys.tabFileStatusInit(d20ActionsTabFile, d20actionTabLineParser);
	if (tabSys.tabFileStatusBasicFormatter(d20ActionsTabFile, "tprules//d20actions.tab"))
	{
		tabSys.tabFileStatusDealloc(d20ActionsTabFile);
	}
	else
	{
		tabSys.tabFileParseLines(d20ActionsTabFile);
	}
	mesFuncs.Open("tpmes//combat.mes", &combatSys.combatMesNew);

	d20Defs[D20A_DIVINE_MIGHT].addToSeqFunc = _AddToSeqSimple;
	d20Defs[D20A_DIVINE_MIGHT].actionCheckFunc = _DivineMightCheck;
	d20Defs[D20A_DIVINE_MIGHT].performFunc = _DivineMightPerform;
	// d20Defs[D20A_DIVINE_MIGHT].actionFrameFunc = _DivineMightPerform;
	d20Defs[D20A_DIVINE_MIGHT].actionCost = _ActionCostNull;
	d20Defs[D20A_DIVINE_MIGHT].flags = D20ADF::D20ADF_None;
	
	D20ActionType d20Type;

	d20Type = D20A_DISARM;
	d20Defs[d20Type].addToSeqFunc = _AddToSeqWithTarget;
	d20Defs[d20Type].aiCheckMaybe = _StdAttackAiCheck;
	d20Defs[d20Type].actionCheckFunc = _ActionCheckDisarm;
	d20Defs[d20Type].locCheckFunc = addresses.LocationCheckStdAttack;
	d20Defs[d20Type].performFunc = _PerformDisarm;
	d20Defs[d20Type].actionFrameFunc = _ActionFrameDisarm;
	d20Defs[d20Type].actionCost = addresses.ActionCostStandardAttack;
	d20Defs[d20Type].pickerFuncMaybe = addresses.sub_1008EDF0;
	d20Defs[d20Type].flags = (D20ADF)(D20ADF_TargetSingleExcSelf | D20ADF_TriggersAoO | D20ADF_QueryForAoO | D20ADF_TriggersCombat
		| D20ADF_Unk8000 | D20ADF_SimulsCompatible ); // 0x28908; // same as Trip // note : queryForAoO is used for resetting a flag


	d20Type = D20A_DISARMED_WEAPON_RETRIEVE;
	d20Defs[d20Type].addToSeqFunc = _AddToSeqSimple;
	d20Defs[d20Type].aiCheckMaybe = 0;
	d20Defs[d20Type].actionCheckFunc = _ActionCheckDisarmedWeaponRetrieve;
	d20Defs[d20Type].locCheckFunc = LocationCheckDisarmedWeaponRetrieve;
	d20Defs[d20Type].performFunc = _PerformDisarmedWeaponRetrieve;
	d20Defs[d20Type].actionFrameFunc = 0;
	d20Defs[d20Type].actionCost = _ActionCostMoveAction;
	d20Defs[d20Type].pickerFuncMaybe = 0;
	d20Defs[d20Type].flags = (D20ADF)( D20ADF_TriggersAoO 	| 0*D20ADF_Unk8000
		| D20ADF_SimulsCompatible | D20ADF_Unk100000); // 0x28908; // largely same as Pick Up Object
	*(int*)&d20Defs[D20A_PICKUP_OBJECT].flags |= (int) (D20ADF_TriggersAoO);

	d20Type = D20A_SUNDER;
	d20Defs[d20Type].addToSeqFunc = _AddToSeqWithTarget;
	d20Defs[d20Type].aiCheckMaybe = _StdAttackAiCheck;
	d20Defs[d20Type].actionCheckFunc = _ActionCheckSunder;
	d20Defs[d20Type].locCheckFunc = addresses.LocationCheckStdAttack;
	d20Defs[d20Type].performFunc = _PerformDisarm;
	d20Defs[d20Type].actionFrameFunc = _ActionFrameSunder;
	d20Defs[d20Type].actionCost = addresses.ActionCostStandardAttack;
	d20Defs[d20Type].pickerFuncMaybe = addresses.sub_1008EDF0;
	d20Defs[d20Type].flags = (D20ADF)(D20ADF_TargetSingleExcSelf | D20ADF_TriggersAoO | D20ADF_TriggersCombat
		| D20ADF_Unk8000 | D20ADF_SimulsCompatible); // 0x28908; // same as Trip

	d20Type = D20A_AID_ANOTHER_WAKE_UP;
	d20Defs[d20Type].addToSeqFunc = _AddToSeqWithTarget;
	d20Defs[d20Type].aiCheckMaybe = _StdAttackAiCheck;
	d20Defs[d20Type].actionCheckFunc = _ActionCheckAidAnotherWakeUp;
	d20Defs[d20Type].locCheckFunc = addresses.LocationCheckStdAttack;
	d20Defs[d20Type].performFunc = _PerformAidAnotherWakeUp;
	d20Defs[d20Type].actionFrameFunc = _ActionFrameAidAnotherWakeUp;
	d20Defs[d20Type].actionCost = addresses.ActionCostStandardAttack;
	d20Defs[d20Type].pickerFuncMaybe = addresses.sub_1008EDF0;
	d20Defs[d20Type].flags = (D20ADF)(D20ADF_TargetSingleExcSelf | D20ADF_Unk8000 | D20ADF_SimulsCompatible); // 0x28908; // same as Trip // note : queryForAoO is used for resetting a flag



	// *(int*)&d20Defs[D20A_USE_POTION].flags |= (int)D20ADF_SimulsCompatible;  // need to modify the SimulsEnqueue script because it also checks for san_start_combat being null
	// *(int*)&d20Defs[D20A_TRIP].flags -= (int)D20ADF_Unk8000;

	//d20Defs[D20A_DISARM] = d20Defs[D20A_STANDARD_ATTACK];
	//d20Defs[d20Type].actionCost = _ActionCostNull; // just for testing - REMOVE!!!
}

D20System::D20System()
{
	pathfinding = &pathfindingSys;
	actSeq = &actSeqSys;
	d20Class = &d20ClassSys;
	d20Status = &d20StatusSys;
	rebase(D20StatusInitFromInternalFields, 0x1004F910);
	rebase(D20ObjRegistryAppend, 0x100DFAD0);
	rebase(d20EditorMode, 0x10AA3284);
	rebase(globD20Action, 0x1186AC00);
	rebase(ToHitProc, 0x100B7160);
	// rebase(d20Defs, 0x102CC5C8);
	d20Defs = (D20ActionDef*)&d20ActionDefsNew;
		d20ActionsTabFile = &_d20actionTabFile;
		d20actionTabLineParser = &_d20actionTabLineParser;
	//rebase(ToEEd20ActionNames, 0x102CD2BC);
	rebase(_d20aTriggerCombatCheck, 0x1008AE90);//ActnSeq * @<eax>
	rebase(_tumbleCheck, 0x1008AA90);
	rebase(_d20aTriggersAOO, 0x1008A9C0);
	rebase(CreateRollHistory, 0x100DFFF0);
	

}


#pragma region D20 Signal and D20 Query

uint32_t D20System::d20Query(objHndl objHnd, D20DispatcherKey dispKey)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (dispatcher == nullptr || (int32_t)dispatcher == -1){ return 0; }
	DispIoD20Query dispIO;
	dispIO.dispIOType = dispIOTypeQuery;
	dispIO.return_val = 0;
	dispIO.data1 = 0;
	dispIO.data2 = 0;
	objects.dispatch.DispatcherProcessor(dispatcher, dispTypeD20Query, dispKey, &dispIO);
	return dispIO.return_val;
}

uint32_t D20System::d20QueryWithData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (dispatcher == nullptr || (int32_t)dispatcher == -1){ return 0; }
	DispIoD20Query dispIO;
	dispIO.dispIOType = dispIOTypeQuery;
	dispIO.return_val = 0;
	dispIO.data1 = arg1;
	dispIO.data2 = arg2;
	objects.dispatch.DispatcherProcessor(dispatcher, dispTypeD20Query, dispKey, &dispIO);
	return dispIO.return_val;
}

uint32_t D20System::d20QueryHasSpellCond(objHndl obj, int spellEnum)
{
	auto cond = spellSys.GetCondFromSpellIdx(spellEnum);
	if (!cond)
		return 0;
	return d20QueryWithData(obj, DK_QUE_Critter_Has_Condition, (uint32_t) cond, 0);
}

void D20System::d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, int32_t arg1, int32_t arg2)
{
	DispIoD20Signal dispIO;
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (!dispatch.dispatcherValid(dispatcher))
	{
		logger->info("d20SendSignal(): Object {} ({}) lacks a Dispatcher", description._getDisplayName(objHnd, objHnd), objHnd);
		return;
	}
	dispIO.dispIOType = dispIoTypeSendSignal;
	dispIO.data1 = arg1;
	dispIO.data2 = arg2;
	dispatch.DispatcherProcessor(dispatcher, dispTypeD20Signal, dispKey, &dispIO);
}

void D20System::d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, D20Actn* arg1, int32_t arg2)
{
	DispIoD20Signal dispIO;
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (!dispatch.dispatcherValid(dispatcher))
	{
		logger->info("d20SendSignal(): Object {} ({}) lacks a Dispatcher", description._getDisplayName(objHnd, objHnd), objHnd);
		return;
	}
	dispIO.dispIOType = dispIoTypeSendSignal;
	dispIO.data1 = (int)arg1;
	dispIO.data2 = arg2;
	dispatch.DispatcherProcessor(dispatcher, dispTypeD20Signal, dispKey, &dispIO);
}

void D20System::d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, objHndl arg) {
	DispIoD20Signal dispIO;
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (!dispatch.dispatcherValid(dispatcher))
	{
		logger->info("d20SendSignal(): Object {} ({}) lacks a Dispatcher", description._getDisplayName(objHnd, objHnd), objHnd);
		return;
	}
	dispIO.dispIOType = dispIoTypeSendSignal;
	*(objHndl*)&dispIO.data1 = arg;
	dispatch.DispatcherProcessor(dispatcher, dispTypeD20Signal, dispKey, &dispIO);
}

#pragma endregion

void D20System::D20ActnInit(objHndl objHnd, D20Actn* d20a)
{
	d20a->d20APerformer = objHnd;
	d20a->d20ActType = D20A_NONE;
	d20a->data1=0 ;
	d20a->d20ATarget=0i64;
	objects.loc->getLocAndOff(objHnd, &d20a->destLoc);
	PathQueryResult * pq = d20a->path;
	d20a->distTraversed = 0;
	d20a->radialMenuActualArg = 0;
	d20a->spellId = 0;
	d20a->d20Caf = 0;

	if (pq && pq >= pathfinding->pathQArray && pq < (pathfinding->pathQArray + pfCacheSize))
	{
		pq->occupiedFlag = 0;
	}
	d20a->path = nullptr;
	d20a->d20SpellData.spellEnumOrg = 0;
	d20a->animID = 0;
	d20a->rollHist1 = -1;
	d20a->rollHist2 = -1;
	d20a->rollHist3 = -1;
	
}

#pragma region Global D20 Action
void D20System::GlobD20ActnSetTypeAndData1(D20ActionType d20type, uint32_t data1)
{
	globD20Action->d20ActType = d20type;
	globD20Action->data1 = data1;
}

void D20System::globD20ActnSetPerformer(objHndl objHnd)
{
	if (objHnd != (*globD20Action).d20APerformer)
	{
		*actSeq->seqSthg_118CD3B8 = -1;
		*actSeq->seqSthg_118A0980 = 1;
		*actSeq->seqSthg_118CD570 = 0;
	}
	(*globD20Action).d20APerformer = objHnd;
}

void D20System::GlobD20ActnSetTarget(objHndl objHnd, LocAndOffsets * loc)
{
	addresses.GlobD20ActnSetTarget(objHnd, loc);
}

void D20System::GlobD20ActnInit()
{
	D20ActnInit(globD20Action->d20APerformer, globD20Action);
}

void D20System::GlobD20ActnSetSpellData(D20SpellData* d20SpellData)
{
	d20Sys.globD20Action->d20SpellData = *d20SpellData;
}
#pragma endregion

void D20System::d20aTriggerCombatCheck(ActnSeq* actSeq, int32_t idx)
{
	__asm{
		push esi;
		push ecx;
		mov ecx, this;
		mov esi, idx;
		push esi;
		mov esi, [ecx]._d20aTriggerCombatCheck;
		mov eax, actSeq;
		call esi;
		add esp, 4;

		pop ecx;
		pop esi;
	}
	//void d20aTriggerCombatCheck(ActnSeq* actSeq, int32_t idx);//1008AE90    ActnSeq * @<eax>
}

int32_t D20System::D20ActionTriggersAoO(D20Actn* d20a, TurnBasedStatus* tbStat)
{
	//uint32_t result = 0;
	ActnSeq * actSeq = *actSeqSys.actSeqCur;
	if (actSeq->tbStatus.tbsFlags & 0x10)
		return 0;



	if ( (d20Defs[d20a->d20ActType].flags & D20ADF::D20ADF_QueryForAoO)
		&& d20QueryWithData(d20a->d20APerformer, DK_QUE_ActionTriggersAOO, (int)d20a, 0))
	{
		if (d20a->d20ActType == D20A_DISARM)
			return feats.HasFeatCountByClass(d20a->d20APerformer, FEAT_IMPROVED_DISARM) == 0;
		return 1;
	}
		
	
	if (!(d20Defs[d20a->d20ActType].flags & D20ADF::D20ADF_TriggersAoO))
		return 0;

	if (d20a->d20ActType == D20A_TRIP)
		return feats.HasFeatCountByClass(d20a->d20APerformer, FEAT_IMPROVED_TRIP) == 0;


	if (d20a->d20ActType == D20A_SUNDER)
		return feats.HasFeatCountByClass(d20a->d20APerformer, FEAT_IMPROVED_SUNDER) == 0;


	if (d20a->d20Caf & D20CAF_TOUCH_ATTACK
		|| d20Sys.GetAttackWeapon(d20a->d20APerformer, d20a->data1, (D20CAF)d20a->d20Caf) 
		|| dispatch.DispatchD20ActionCheck(d20a, tbStat, dispTypeGetCritterNaturalAttacksNum))
		return 0;

	return feats.HasFeatCountByClass(d20a->d20APerformer, FEAT_IMPROVED_UNARMED_STRIKE) == 0;

	/*
	__asm{
		push esi;
		push ecx;
		push ebx;
		mov ecx, this;
		mov esi, iO;
		push esi;
		mov ebx, [ecx]._d20aTriggersAOO;
		mov esi, d20a;
		call ebx;
		add esp, 4;
		mov result, eax;
		pop ebx;
		pop ecx;
		pop esi;
	}
	return result; //_d20aTriggersAOO(void * iO); // d20a @<esi> // 1008A9C0
	*/
}

uint32_t D20System::tumbleCheck(D20Actn* d20a)
{
	if (d20QueryWithData(d20a->d20ATarget, DK_QUE_Critter_Has_Spell_Active, 407, 0)) return 0; // spell_sanctuary active
	if (actSeq->isPerforming(d20a->d20APerformer))
	{
		logger->info("movement aoo while performing...\n");
		return 0;
	}
	if (!d20QueryWithData(d20a->d20ATarget, DK_QUE_AOOIncurs, (uint32_t)(d20a->d20APerformer & 0xFFFFffff), (uint32_t)(d20a->d20APerformer >> 32))){ return 0; }
	if (!d20QueryWithData(d20a->d20APerformer, DK_QUE_AOOPossible, (uint32_t)(d20a->d20ATarget & 0xFFFFffff), (uint32_t)(d20a->d20ATarget >> 32))){ return 0; }
	if (!d20QueryWithData(d20a->d20APerformer, DK_QUE_AOOWillTake, (uint32_t)(d20a->d20ATarget & 0xFFFFffff), (uint32_t)(d20a->d20ATarget >> 32))){ return 0; }
	// not fully implemented yet, but that should cover 90% of the cases anyway ;) TODO: complete this function
	return _tumbleCheck(d20a);
}

void D20System::D20ActnSetSpellData(D20SpellData* d20SpellData, uint32_t spellEnumOrg, uint32_t spellClassCode, uint32_t spellSlotLevel, uint32_t itemSpellData, uint32_t metaMagicData)
{
	*(uint32_t *)&d20SpellData->metaMagicData = metaMagicData;
	d20SpellData->spellEnumOrg = spellEnumOrg;
	d20SpellData->spellClassCode = spellClassCode;
	d20SpellData->itemSpellData = itemSpellData;
	d20SpellData->spontCastType = (SpontCastType)0;
	d20SpellData->spellSlotLevel = spellSlotLevel;
}



bool D20System::UsingSecondaryWeapon(D20Actn* d20a)
{
	return UsingSecondaryWeapon(d20a->d20APerformer, d20a->data1);
}

bool D20System::UsingSecondaryWeapon(objHndl obj, int attackCode)
{
	if (attackCode == ATTACK_CODE_OFFHAND + 2 || attackCode == ATTACK_CODE_OFFHAND + 4 || attackCode == ATTACK_CODE_OFFHAND + 6)
	{
		if (attackCode == ATTACK_CODE_OFFHAND + 2)
		{
			return 1;
		}
		if (attackCode == ATTACK_CODE_OFFHAND + 4)
		{
			if (feats.HasFeatCount(obj, FEAT_IMPROVED_TWO_WEAPON_FIGHTING)
				|| feats.HasFeatCountByClass(obj, FEAT_IMPROVED_TWO_WEAPON_FIGHTING_RANGER, (Stat)0,0))
				return 1;
		}
		else if (attackCode == ATTACK_CODE_OFFHAND + 6)
		{
			if (feats.HasFeatCount(obj, FEAT_GREATER_TWO_WEAPON_FIGHTING)
				|| feats.HasFeatCountByClass(obj, FEAT_GREATER_TWO_WEAPON_FIGHTING_RANGER, (Stat)0, 0))
				return 1;
		}
	}
	return 0;
}

void D20System::ExtractAttackNumber(objHndl obj, int attackCode, int* attackNumber, int * dualWielding)
{
	if (attackCode >= ATTACK_CODE_NATURAL_ATTACK)
	{
		*attackNumber = attackCode - ATTACK_CODE_NATURAL_ATTACK;
		*dualWielding = 0 ;
	}
	else if (attackCode >= ATTACK_CODE_OFFHAND)
	{
		*dualWielding = 1;
		int attackIdx = attackCode - (ATTACK_CODE_OFFHAND+1);
		int numOffhandExtraAttacks = critterSys.NumOffhandExtraAttacks(obj);
		if (d20Sys.UsingSecondaryWeapon(obj, attackCode))
		{
			if (attackIdx % 2 && (attackIdx - 1) / 2 < numOffhandExtraAttacks )
				*attackNumber = 1 + (attackIdx - 1) / 2;
		}
		else
		{
			if ( !(attackIdx % 2 ) && (attackIdx  / 2 < numOffhandExtraAttacks) )
				*attackNumber = 1 + attackIdx  / 2;
			else
				*attackNumber = 1 + numOffhandExtraAttacks + (attackIdx - 2*numOffhandExtraAttacks);
		}
		assert(*attackNumber > 0);
	}
	else // regular case (just primary hand)
	{
		*attackNumber = attackCode - ATTACK_CODE_PRIMARY;
		if (*attackNumber <= 0) // seems to be the case for charge attack
		{
			*attackNumber = 1;
		}
		*dualWielding = 0;
	}
}

objHndl D20System::GetAttackWeapon(objHndl obj, int attackCode, D20CAF flags)
{
	if (flags & D20CAF_TOUCH_ATTACK && !(flags & D20CAF_THROWN_GRENADE))
	{
		return 0i64;
	}

	if (flags & D20CAF_SECONDARY_WEAPON)
		return inventory.ItemWornAt(obj, 4);

	if (UsingSecondaryWeapon(obj, attackCode))
		return inventory.ItemWornAt(obj, 4);

	if (attackCode > ATTACK_CODE_NATURAL_ATTACK)
		return 0i64;

	return inventory.ItemWornAt(obj, 3);
}

int D20System::PerformStandardAttack(D20Actn* d20a)
{
	int v5 = templeFuncs.RNG(0, 2);

	int d20data = d20a->data1;
	int playCritFlag = 0;
	int useSecondaryAnim = 0;
	if (UsingSecondaryWeapon(d20a))
	{
		d20a->d20Caf |= D20CAF_SECONDARY_WEAPON; 
		useSecondaryAnim = 1;
	}
	else if (d20a->data1 >= ATTACK_CODE_NATURAL_ATTACK + 1)
	{
		useSecondaryAnim = templeFuncs.RNG(0, 1);
		v5 = (d20a->data1 - (ATTACK_CODE_NATURAL_ATTACK + 1)) % 3;
	}

	ToHitProc(d20a);

	int caflags = d20a->d20Caf;
	if (caflags & D20CAF_CRITICAL
		|| d20QueryWithData(d20a->d20APerformer, DK_QUE_Play_Critical_Hit_Anim, caflags, caflags >> 32))
		playCritFlag = 1;


	
	if (animationGoals.PushAttackAnim(d20a->d20APerformer, d20a->d20ATarget, 0xFFFFFFFF, v5, playCritFlag, useSecondaryAnim))
	{
		d20a->animID = animationGoals.GetAnimIdSthgSub_1001ABB0(d20a->d20APerformer);
		d20a->d20Caf |= D20CAF_NEED_ANIM_COMPLETED;
	}
	return 0;
}

int D20System::TargetWithinReachOfLoc(objHndl obj, objHndl target, LocAndOffsets* loc)
{
	return addresses.TargetWithinReachOfLoc(obj, target, loc);
}

void D20System::D20ActnSetSetSpontCast(D20SpellData* d20SpellData, SpontCastType spontCastType)
{
	d20SpellData->spontCastType = spontCastType;
	d20SpellData->metaMagicData.metaMagicFlags = 0;
	d20SpellData->metaMagicData.metaMagicEmpowerSpellCount = 0;
	d20SpellData->metaMagicData.metaMagicEnlargeSpellCount = 0;
	d20SpellData->metaMagicData.metaMagicExtendSpellCount = 0;
	d20SpellData->metaMagicData.metaMagicHeightenSpellCount = 0;
	d20SpellData->metaMagicData.metaMagicWidenSpellCount = 0;
}

uint64_t D20System::d20QueryReturnData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1, ::uint32_t arg2)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (!dispatch.dispatcherValid(dispatcher)){ return 0; }
	DispIoD20Query dispIO;
	dispIO.dispIOType = dispIOTypeQuery;
	dispIO.return_val = 0;
	dispIO.data1 = arg1;
	dispIO.data2 = arg2;
	objects.dispatch.DispatcherProcessor(dispatcher, dispTypeD20Query, dispKey, &dispIO);
	return *(uint64_t*)&dispIO.data1;
}
#pragma endregion 





#pragma region D20 Spell Stuff

void D20SpellDataExtractInfo
  (D20SpellData * d20SpellData	, uint32_t * spellEnum		, uint32_t * spellEnumOriginal	, 
   uint32_t * spellClassCode	, uint32_t * spellSlotLevel	, uint32_t * itemSpellData		, 
   uint32_t * metaMagicData)
{
	if ( ! (spellEnumOriginal == nullptr) )
	{
		*spellEnumOriginal = d20SpellData->spellEnumOrg;
	}

	if (!(spellSlotLevel == nullptr ) )
	{
		*spellSlotLevel = d20SpellData->spellSlotLevel;
	}

	if (! (spellEnum == nullptr))
	{
		if ((SpontCastType) d20SpellData->spontCastType == spontCastGoodCleric)
		{
			*spellEnum = spontCastSpellLists.spontCastSpellsGoodCleric[d20SpellData->spellSlotLevel];
		}
		else if ((SpontCastType)d20SpellData->spontCastType == spontCastEvilCleric)
		{
			*spellEnum = spontCastSpellLists.spontCastSpellsEvilCleric[d20SpellData->spellSlotLevel];
		} 
		else if ( (SpontCastType) d20SpellData->spontCastType == spontCastDruid)
		{
			*spellEnum = spontCastSpellLists.spontCastSpellsDruid[d20SpellData->spellSlotLevel];
		}
 else
 {
	 *spellEnum = d20SpellData->spellEnumOrg;
 }
	}

	if (!(spellClassCode == nullptr))
	{
		*spellClassCode = d20SpellData->spellClassCode;
	}

	if (!(itemSpellData == nullptr))
	{
		*itemSpellData = d20SpellData->itemSpellData;
	}

	if (!(metaMagicData == nullptr))
	{
		*metaMagicData = (*((uint32_t*)&(d20SpellData->metaMagicData))) & (0xFFFFFF); // sue me! it WORKS
	}

	return;
}


void __cdecl D20SpellDataSetSpontCast(D20SpellData* d20SpellData, SpontCastType spontCastType)
{
	if (!(d20SpellData == nullptr))
	{
		d20SpellData->spontCastType = spontCastType;
	}
};

#pragma endregion



#pragma region D20Status Init Functions

void _D20StatusInit(objHndl objHnd)
{
	d20Sys.d20Status->D20StatusInit(objHnd);
	}

void _D20StatusRefresh(objHndl objHnd)
{
	d20StatusSys.D20StatusRefresh(objHnd);
}

void _D20StatusInitFromInternalFields(objHndl objHnd, Dispatcher * dispatcher)
{
	d20StatusSys.D20StatusInitFromInternalFields(objHnd,  dispatcher);
}

void _D20StatusInitRace(objHndl objHnd)
{
	d20Sys.d20Status->initRace(objHnd);
};


void _D20StatusInitClass(objHndl objHnd)
{
	d20Sys.d20Status->initClass(objHnd);
};

void _D20StatusInitDomains(objHndl objHnd)
{
	d20Sys.d20Status->initDomains(objHnd);
}

void _D20StatusInitFeats(objHndl objHnd)
{
	d20Sys.d20Status->initFeats(objHnd);
};

void _D20StatusInitItemConditions(objHndl objHnd)
{
	d20Sys.d20Status->initItemConditions(objHnd);
}

#pragma endregion


uint32_t _D20Query(objHndl objHnd, D20DispatcherKey dispKey)
{
	return d20Sys.d20Query(objHnd, dispKey);
}

void _d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, int32_t arg1, int32_t arg2)
{
	d20Sys.d20SendSignal(objHnd, dispKey, arg1, arg2);
}

void __cdecl _D20aInitCdecl(objHndl objHnd, D20Actn* d20a)
{
	d20Sys.D20ActnInit(objHnd, d20a);
}


void __declspec(naked) _D20ActnInitUsercallWrapper(objHndl objHnd)
{
	__asm{ // esi is D20Actn * d20a
		push ebp; 
		mov ebp, esp; // ebp = &var4  ebp+4 = &retaddr  ebp+8 = &arg1

		push esi; 
		mov eax, [ebp + 12];
		push eax;
		mov eax, [ebp + 8];
		push eax;
		mov eax, _D20aInitCdecl;
		call eax;
		add esp, 8;

		pop esi;
		mov esp, ebp;
		pop ebp;
		retn;
	}
}

void _d20ActnSetSpellData(D20SpellData* d20SpellData, uint32_t spellEnumOrg, uint32_t spellClassCode, uint32_t spellSlotLevel, uint32_t itemSpellData, uint32_t metaMagicData)
{
	d20Sys.D20ActnSetSpellData(d20SpellData, spellEnumOrg, spellClassCode, spellSlotLevel, itemSpellData, metaMagicData);
}

void _globD20aSetTypeAndData1(D20ActionType d20type, uint32_t data1)
{
	d20Sys.GlobD20ActnSetTypeAndData1(d20type, data1);
}

uint32_t _d20QueryWithData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2)
{
	return d20Sys.d20QueryWithData(objHnd, dispKey, arg1, arg2);
}

uint64_t _d20QueryReturnData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2)
{
	return d20Sys.d20QueryReturnData(objHnd, dispKey, arg1, arg2);
}

void _globD20aSetPerformer(objHndl objHnd)
{
	d20Sys.globD20ActnSetPerformer(objHnd);
}

void _GlobD20ActnInit()
{
	d20Sys.GlobD20ActnInit();
}

void _GlobD20ActnSetSpellData(D20SpellData* d20SpellData)
{
	d20Sys.GlobD20ActnSetSpellData(d20SpellData);
}

int _PerformStandardAttack(D20Actn* d20a)
{
	return d20Sys.PerformStandardAttack(d20a);
}

objHndl _GetAttackWeapon(objHndl obj, int attackCode, D20CAF flags)
{
	return d20Sys.GetAttackWeapon(obj, attackCode, flags);
}

uint32_t _d20actionTabLineParser(TabFileStatus*, uint32_t n, const char** strings)
{
	D20ActionType d20ActionType = (D20ActionType)atoi(strings[0]);
	if (d20ActionType <= D20A_USE_POTION)
	{
		for (int i = 0; i < sizeof(D20ActionDef) / sizeof(int); i++)
		{
			*((int*)&d20Sys.d20Defs[d20ActionType] + i) = atoi(strings[i + 1]);
		}
	}
	return 0;

}

int _D20Init(GameSystemConf* conf)
{
	d20Sys.NewD20ActionsInit();
	return OrgD20Init(conf);
}

uint32_t _DivineMightCheck(D20Actn* d20a, TurnBasedStatus* tbStat)
{
	DispIoD20ActionTurnBased dispIo;
	dispIo.dispIOType = dispIOTypeD20ActionTurnBased;
	dispIo.returnVal = 0;
	dispIo.d20a = d20a;
	dispIo.tbStatus = tbStat;
	auto dispatcher = objects.GetDispatcher(d20a->d20APerformer);
	auto alignmentchoice = objects.getInt32(d20a->d20APerformer, obj_f_critter_alignment_choice);
	if (alignmentchoice == 2)
		d20a->data1 = 1;

	if (dispatch.dispatcherValid(dispatcher))
	{
		dispatch.DispatcherProcessor(dispatcher, dispTypeD20ActionCheck, DK_D20A_TURN_UNDEAD , &dispIo);
	}

	return dispIo.returnVal;
}

uint32_t _DivineMightPerform(D20Actn* d20a)
{
	DispIoD20ActionTurnBased dispIo;
	dispIo.dispIOType = dispIOTypeD20ActionTurnBased;
	dispIo.returnVal = 0;
	dispIo.d20a = d20a;
	dispIo.tbStatus = nullptr;
	auto dispatcher = objects.GetDispatcher(d20a->d20APerformer);
	if (dispatch.dispatcherValid(dispatcher))
	{
		dispatch.DispatcherProcessor(dispatcher, dispTypeD20ActionPerform, DK_D20A_DIVINE_MIGHT, &dispIo); // reduces the turn undead charge by 1
	}

	auto chaScore = objects.StatLevelGet(d20a->d20APerformer, stat_charisma);
	auto chaMod = objects.GetModFromStatLevel(chaScore);
	conds.AddTo(d20a->d20APerformer, "Divine Might Bonus", { chaMod, 0 });
	
	return dispIo.returnVal;
}



uint32_t _ActionCheckDisarm(D20Actn* d20a, TurnBasedStatus* tbStat)
{
	objHndl performer = d20a->d20APerformer;
	if (d20Sys.d20Query(performer, DK_QUE_Prone) || d20Sys.d20Query(performer, DK_QUE_Unconscious))
		return 13;

	objHndl weapon = inventory.ItemWornAt(d20a->d20APerformer, 3);
	int weapFlags; 
		
	if (weapon && (weapFlags = objects.getInt32(weapon, obj_f_weapon_flags), weapFlags & OWF_RANGED_WEAPON)) // ranged weapon - Need Melee Weapon error
	{
		return 12;
	}

	if (d20a->d20ATarget)
	{
		objHndl targetWeapon = inventory.ItemWornAt(d20a->d20ATarget, 3);
		if (!targetWeapon)
		{
			targetWeapon = inventory.ItemWornAt(d20a->d20ATarget, 4);
			if (!targetWeapon)
				return 9;
			if (objects.GetType(targetWeapon) != obj_t_weapon)
				return 9;
		}
	}
	else
		return 9;

	return 0;
}

uint32_t _PerformDisarm(D20Actn* d20a)
{
	
	if (animationGoals.PushAttemptAttack(d20a->d20APerformer, d20a->d20ATarget))
	{
		d20a->animID = animationGoals.GetAnimIdSthgSub_1001ABB0(d20a->d20APerformer);
		d20a->d20Caf |= D20CAF_NEED_ANIM_COMPLETED;
	}
	return 0;
};


uint32_t _ActionFrameDisarm(D20Actn* d20a)
{
	objHndl performer = d20a->d20APerformer;
	int failedOnce = 0;
	if (!d20Sys.d20Query(d20a->d20APerformer, DK_QUE_Can_Perform_Disarm))
	{
		objects.floats->FloatCombatLine(d20a->d20APerformer, 195); //fail!
		failedOnce = 1;
	}
		
	else if (combatSys.DisarmCheck(d20a->d20APerformer, d20a->d20ATarget, d20a))
	{
		objHndl weapon = inventory.ItemWornAt(d20a->d20ATarget, 3);
		objHndl attackerWeapon = inventory.ItemWornAt(performer, 3);
		objHndl attackerOffhand = inventory.ItemWornAt(performer, 4);
		objHndl attackerShield = inventory.ItemWornAt(performer, 11);
		if (!weapon)
			weapon = inventory.ItemWornAt(d20a->d20ATarget, 4);
		if (!attackerWeapon && !attackerOffhand)
		{
			int itemWearFlags;
			if ( (inventory.GetWieldType(d20a->d20APerformer, weapon) != 2 || !attackerShield
				|| ( itemWearFlags = objects.getInt32(attackerShield, obj_f_item_wear_flags), itemWearFlags & OIF_WEAR_BUCKLER) ) 
				&& objects.StatLevelGet(d20a->d20APerformer, stat_level_monk) == 0)
				inventory.ItemGetAdvanced(weapon, d20a->d20APerformer, 203, 0);
			else
			{
				inventory.ItemDrop(weapon);
				inventory.SetItemParent(weapon, d20a->d20APerformer, 32);
			}
			objects.floats->FloatCombatLine(d20a->d20ATarget, 202);
		} 
		else if (weapon)
		{
			inventory.ItemDrop(weapon);
			objects.floats->FloatCombatLine(d20a->d20ATarget, 198);
		}
		
		struct DisarmedArgs
		{
			objHndl weapon;
		} disarmedArgs;
		disarmedArgs.weapon = weapon;

		conds.AddTo(d20a->d20ATarget, "Disarmed", { ((int*)&disarmedArgs)[0], ((int*)&disarmedArgs)[1],0,0,0,0 });
		return 0;
	} 

	

	// counter attempt
	if (!feats.HasFeatCountByClass(d20a->d20APerformer, FEAT_IMPROVED_DISARM))
	{
		D20Actn d20aCopy;
		memcpy(&d20aCopy, d20a, sizeof(D20Actn));
		d20aCopy.d20APerformer = d20a->d20ATarget;
		d20aCopy.d20ATarget = d20a->d20APerformer;
		if (!d20Sys.d20Defs[D20A_DISARM].actionCheckFunc(&d20aCopy, nullptr))
		{
			if( animationGoals.PushAttemptAttack(d20aCopy.d20APerformer, d20aCopy.d20ATarget))
				d20aCopy.animID = animationGoals.GetAnimIdSthgSub_1001ABB0(d20aCopy.d20APerformer);
			if (combatSys.DisarmCheck(d20a->d20ATarget, d20a->d20APerformer, d20a))
			{

				objHndl weapon = inventory.ItemWornAt(d20a->d20APerformer, 3);
				if (!weapon)
					weapon = inventory.ItemWornAt(d20a->d20APerformer, 4);
				if (weapon)
				{
					inventory.ItemDrop(weapon);
				}
				struct DisarmedArgs
				{
					objHndl weapon;
				} disarmedArgs;
				disarmedArgs.weapon = weapon;
				objects.floats->FloatCombatLine(d20a->d20APerformer, 200); // Counter Disarmed!
				conds.AddTo(d20a->d20APerformer, "Disarmed", { ((int*)&disarmedArgs)[0], ((int*)&disarmedArgs)[1], 0,0,0,0 });
				return 0;
			}
			else if (!failedOnce)
			{
				objects.floats->FloatCombatLine(d20a->d20APerformer, 195);
			}
		}
		else if (!failedOnce)
		{
			objects.floats->FloatCombatLine(d20a->d20APerformer, 195);
		}
		
	} else if (!failedOnce)
	{
		objects.floats->FloatCombatLine(d20a->d20APerformer, 195);
	}

	
	

	return 0;
};

#pragma region Retrieve Disarmed Weapon
uint32_t LocationCheckDisarmedWeaponRetrieve(D20Actn* d20a, TurnBasedStatus* tbStat, LocAndOffsets* loc)
{
	if (!combatSys.isCombatActive())
		return 0;
	if (d20a->d20ATarget)
		return d20Sys.TargetWithinReachOfLoc(d20a->d20APerformer, d20a->d20ATarget, loc) != 0 ? 0 : 8;
	objHndl weapon = d20Sys.d20QueryReturnData(d20a->d20APerformer, DK_QUE_Disarmed, 0, 0);
	if (weapon)
		return d20Sys.TargetWithinReachOfLoc(d20a->d20APerformer, weapon, loc) != 0 ? 0 : 8;
	return 9;
};

uint32_t _ActionCheckDisarmedWeaponRetrieve(D20Actn* d20a, TurnBasedStatus* tbStat)
{
	int dummy = 1;
	return 0;
};

uint32_t _PerformDisarmedWeaponRetrieve(D20Actn* d20a)
{
	d20Sys.d20SendSignal(d20a->d20APerformer, DK_SIG_Disarmed_Weapon_Retrieve, (int)d20a, 0);
	return 0;
};

#pragma endregion

uint32_t _ActionCheckSunder(D20Actn* d20a, TurnBasedStatus* tbStat)
{
	objHndl weapon = inventory.ItemWornAt(d20a->d20APerformer, 3);
	int weapFlags;

	if (!weapon)
		return 12;
	weapFlags = objects.getInt32(weapon, obj_f_weapon_flags);
	if ( weapFlags & OWF_RANGED_WEAPON) // ranged weapon - Need Melee Weapon error
	{
		return 12;
	}
	
	if (!weapons.IsSlashingOrBludgeoning(weapon))
		return 17;

	if (d20a->d20ATarget)
	{
		objHndl targetWeapon = inventory.ItemWornAt(d20a->d20ATarget, 3);
		if (!targetWeapon)
		{
			targetWeapon = inventory.ItemWornAt(d20a->d20ATarget, 4);
			if (targetWeapon)
				return 0;
			targetWeapon = inventory.ItemWornAt(d20a->d20ATarget, 11); // shield
			if (!targetWeapon)
				return 9;
		}
	}
	else
		return 9;

	return 0;
}


uint32_t _ActionFrameSunder(D20Actn* d20a)
{

	if (combatSys.SunderCheck(d20a->d20APerformer, d20a->d20ATarget, d20a))
	{

		objHndl weapon = inventory.ItemWornAt(d20a->d20ATarget, 3);
		if (!weapon)
			weapon = inventory.ItemWornAt(d20a->d20ATarget, 4);
		if (weapon)
			inventory.ItemDrop(weapon);
		//objects.floats->FloatCombatLine(d20a->d20ATarget, 198);
	}


	return 0;
}


uint32_t _PerformAidAnotherWakeUp(D20Actn* d20a)
{

	if (animationGoals.PushAttemptAttack(d20a->d20APerformer, d20a->d20ATarget))
	{
		animationGoals.PushUseSkillOn(d20a->d20APerformer, d20a->d20ATarget, SkillEnum::skill_heal);
		d20a->animID = animationGoals.GetAnimIdSthgSub_1001ABB0(d20a->d20APerformer);
		d20a->d20Caf |= D20CAF_NEED_ANIM_COMPLETED;

		//if (!party.IsInParty(d20a->d20APerformer) )
		{
			char blargh[1000];
			memcpy(blargh, "Wake up!", sizeof("Wake up!"));
			uiDialog.ShowTextBubble(d20a->d20APerformer, d20a->d20APerformer, { blargh }, -1);
			
		}
	}
	return 0;
}

uint32_t _ActionFrameAidAnotherWakeUp(D20Actn* d20a)
{
	
	// objects.floats->FloatCombatLine(d20a->d20ATarget, 204); // woken up // not necessary - already gets applied with the removal of the sleep condition I think
	d20Sys.d20SendSignal(d20a->d20ATarget, DK_SIG_AID_ANOTHER_WAKE_UP, d20a, 0);
	
	
	return 0;
}

uint32_t _ActionCheckAidAnotherWakeUp(D20Actn* d20a, TurnBasedStatus* tbStat)
{
	if (!d20a->d20ATarget)
	{
		return 9;
	}
	return 0;
};