#include "stdafx.h"
#include "common.h"
#include "ai.h"
#include "d20.h"
#include "combat.h"
#include "obj.h"
#include "action_sequence.h"
#include "spell.h"
#include "temple_functions.h"
#include "pathfinding.h"
#include "objlist.h"
#include "gamesystems/objects/objsystem.h"

#pragma region AI System Implementation
#include "location.h"
#include "critter.h"
#include "weapon.h"
#include "python/python_integration_obj.h"
#include "python/python_object.h"
#include "util/fixes.h"
#include "party.h"
#include "ui/ui_picker.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/timeevents.h"
#include "python/python_header.h"
#include "condition.h"
#include "rng.h"

struct AiSystem aiSys;
AiParamPacket * AiSystem::aiParams;

const auto TestSizeOfAiTactic = sizeof(AiTactic); // should be 2832 (0xB10 )
const auto TestSizeOfAiStrategy = sizeof(AiStrategy); // should be 808 (0x324)

struct AiSystemAddresses : temple::AddressTable
{
	int (__cdecl*UpdateAiFlags)(objHndl obj, int aiFightStatus, objHndl target, int * soundMap);
	void(__cdecl*AiProcess)(objHndl obj);
	AiSystemAddresses()
	{
		rebase(UpdateAiFlags, 0x1005DA00);
		rebase(AiProcess, 0x1005EEC0);
	}

	
}addresses;

AiSystem::AiSystem()
{
	combat = &combatSys;
	d20 = &d20Sys;
	actSeq = &actSeqSys;
	spell = &spellSys;
	pathfinding = &pathfindingSys;
	rebase(aiStrategies,0x11868F3C); 
	rebase(aiStrategiesNum,0x11868F40); 
	rebase(aiTacticDefs,0x102E4398); 
	rebase(_AiRemoveFromList, 0x1005A070);
	rebase(_FleeAdd, 0x1005DE60);
	rebase(_ShitlistAdd, 0x1005CC10);
	rebase(_StopAttacking, 0x1005E6A0);
	rebase(_AiSetCombatStatus, 0x1005DA00);
	rebase(aiParams, 0x10AA4BD0);
	RegisterNewAiTactics();
}

void AiSystem::aiTacticGetConfig(int tacIdx, AiTactic* aiTacOut, AiStrategy* aiStrat)
{
	SpellPacketBody * spellPktBody = &aiTacOut->spellPktBody;
	spell->spellPacketBodyReset(&aiTacOut->spellPktBody);
	aiTacOut->aiTac = aiStrat->aiTacDefs[tacIdx];
	aiTacOut->field4 = aiStrat->field54[tacIdx];
	aiTacOut->tacIdx = tacIdx;
	int32_t spellEnum = aiStrat->spellsKnown[tacIdx].spellEnum;
	spellPktBody->spellEnum = spellEnum;
	spellPktBody->spellEnumOriginal = spellEnum;
	if (spellEnum != -1)
	{
		aiTacOut->spellPktBody.caster = aiTacOut->performer;
		aiTacOut->spellPktBody.spellClass = aiStrat->spellsKnown[tacIdx].classCode;
		aiTacOut->spellPktBody.spellKnownSlotLevel = aiStrat->spellsKnown[tacIdx].spellLevel;
		spell->SpellPacketSetCasterLevel(spellPktBody);
		d20->D20ActnSetSpellData(&aiTacOut->d20SpellData, spellEnum, spellPktBody->spellClass, spellPktBody->spellKnownSlotLevel, 0xFF, spellPktBody->metaMagicData);
	}
}

uint32_t AiSystem::AiStrategyParse(objHndl objHnd, objHndl target)
{
	#pragma region preamble
	AiTactic aiTac;
	combat->enterCombat(objHnd);
	auto stratIdx = objects.getInt32(objHnd, obj_f_critter_strategy);
	Expects(stratIdx >= 0);
	AiStrategy* aiStrat = &(*aiStrategies)[stratIdx];
	if (!actSeq->TurnBasedStatusInit(objHnd)) return 0;

	actSeq->curSeqReset(objHnd);
	d20->GlobD20ActnInit();
	spell->spellPacketBodyReset(&aiTac.spellPktBody);
	aiTac.performer = objHnd;
	aiTac.target = target;
	#pragma endregion

	AiCombatRole role = GetRole(objHnd);
	if (role != AiCombatRole::caster)
	{

		// check if disarmed, if so, try to pick up weapon
		if (d20Sys.d20Query(aiTac.performer, DK_QUE_Disarmed))
		{
			logger->info("AiStrategy: \t {} attempting to pickup weapon...", description.getDisplayName(objHnd));
			if (PickUpWeapon(&aiTac))
			{
				actSeq->sequencePerform();
				return 1;
			}
		}

		// wake up friends put to sleep; will do this if friend is within reach or otherwise at a 40% chance
		if (WakeFriend(&aiTac))
		{
			actSeq->sequencePerform();
			return 1;
		}

	}

	// loop through tactics defined in strategy.tab
	for (uint32_t i = 0; i < aiStrat->numTactics; i++)
	{
		aiTacticGetConfig(i, &aiTac, aiStrat);
		logger->info("AiStrategy: \t {} attempting {}...", description._getDisplayName(objHnd, objHnd), aiTac.aiTac->name);
		auto aiFunc = aiTac.aiTac->aiFunc;
		if (!aiFunc) continue;
		if (aiFunc(&aiTac)) {
			logger->info("AiStrategy: \t AI tactic succeeded; performing.");
			actSeq->sequencePerform();
			return 1;
		}
	}

	// if no tactics defined (e.g. frogs), do target closest first to avoid all kinds of sillyness
	if (aiStrat->numTactics == 0)
	{
		TargetClosest(&aiTac);
	}

	// if none of those work, use default
	aiTac.aiTac = &aiTacticDefs[0];
	aiTac.field4 = 0;
	aiTac.tacIdx = -1;
	logger->info("AiStrategy: \t {} attempting default...", description._getDisplayName(objHnd, objHnd));
	if (aiTac.target)
		logger->info("Target: {}", description.getDisplayName(aiTac.target));
	assert(aiTac.aiTac != nullptr);
	if (Default(&aiTac))
	{
		actSeq->sequencePerform();
		return 1;
	}

	if (!aiTac.target || !combatSys.IsWithinReach(objHnd, aiTac.target)){
		logger->info("AiStrategy: \t Default FAILED. Attempting to find pathable party member as target...");
		objHndl pathablePartyMember = pathfindingSys.CanPathToParty(objHnd);
		if (pathablePartyMember)
		{
			aiTac.target = pathablePartyMember;
			if (aiTac.aiTac->aiFunc(&aiTac))
			{
				logger->info("AiStrategy: \t Default tactic succeeded; performing.");
				actSeq->sequencePerform();
				return 1;
			}
		}
	}
	

	// if that doesn't work either, try to Break Free (NPC might be held back by Web / Entangle)
	if (d20Sys.d20Query(aiTac.performer, DK_QUE_Is_BreakFree_Possible))
	{
		logger->info("AiStrategy: \t {} attempting to break free...", description.getDisplayName(objHnd));
		if (BreakFree(&aiTac))
		{
			actSeq->sequencePerform();
			return 1;
		}
	}

	return 0;
}

uint32_t AiSystem::AiStrategDefaultCast(objHndl objHnd, objHndl target, D20SpellData* spellData, SpellPacketBody* spellPkt)
{
	AiTactic aiTac;
	combat->enterCombat(objHnd);
	auto stratIdx = objects.getInt32(objHnd, obj_f_critter_strategy);
	Expects(stratIdx >= 0);
	AiStrategy* aiStrat = &(*aiStrategies)[stratIdx];
	if (!actSeq->TurnBasedStatusInit(objHnd)) return 0;

	actSeq->curSeqReset(objHnd);
	d20->GlobD20ActnInit();
	spell->spellPacketBodyReset(&aiTac.spellPktBody);
	aiTac.performer = objHnd;
	aiTac.target = target;

	// loop through tactics defined in strategy.tab
	for (uint32_t i = 0; i < aiStrat->numTactics; i++)
	{
		aiTacticGetConfig(i, &aiTac, aiStrat);
		logger->info("AiStrategyDefaultCast: \t {} attempting {}...", description._getDisplayName(objHnd, objHnd), aiTac.aiTac->name);
		auto aiFunc = aiTac.aiTac->aiFunc;
		if (!aiFunc) continue;
		if (aiFunc(&aiTac)) {
			logger->info("AiStrategyDefaultCast: \t AI tactic succeeded; performing.");
			actSeq->sequencePerform();
			return 1;
		}
	}

	logger->info("AiStrategyDefaultCast: \t AI tactics failed, trying DefaultCast.");
	aiTac.d20SpellData = *spellData;
	aiTac.aiTac = &aiTacticDefs[1]; // default spellcast
	aiTac.tacIdx = -1;
	aiTac.spellPktBody = *spellPkt;

	auto aiFunc = aiTac.aiTac->aiFunc;
	if (aiFunc && aiFunc(&aiTac)) {
		logger->info("AiStrategyDefaultCast: \t DefaultCast succeeded; performing.");
		actSeq->sequencePerform();
		return 1;
	}
	logger->info("AiStrategyDefaultCast: \t DefaultCast failed, trying to perform again with initial target.");
	aiTac.target = target;
	if (aiFunc && aiFunc(&aiTac)) {
		logger->info("AiStrategyDefaultCast: \t DefaultCast succeeded; performing.");
		actSeq->sequencePerform();
		return 1;
	}

	// if nothing else, try to breakfree
	if (d20Sys.d20Query(aiTac.performer, DK_QUE_Is_BreakFree_Possible))
	{
		logger->info("AiStrategy: \t {} attempting to break free...", description.getDisplayName(objHnd));
		if (BreakFree(&aiTac))
		{
			actSeq->sequencePerform();
			return 1;
		}
	}

	return 0;
}

bool AiSystem::HasAiFlag(objHndl npc, AiFlag flag) const {
	auto obj = objSystem->GetObject(npc);
	auto aiFlags = obj->GetInt64(obj_f_npc_ai_flags64);
	auto flagBit = (uint64_t)flag;
	return (aiFlags & flagBit) != 0;
}

bool AiSystem::IsRunningOff(objHndl handle) const
{
	return objects.IsNPC(handle) && HasAiFlag(handle, AiFlag::RunningOff);
}

void AiSystem::SetAiFlag(objHndl npc, AiFlag flag) {
	auto obj = objSystem->GetObject(npc);
	auto aiFlags = obj->GetInt64(obj_f_npc_ai_flags64);
	aiFlags |= (uint64_t) flag;
	obj->SetInt64(obj_f_npc_ai_flags64, aiFlags);
}

void AiSystem::ClearAiFlag(objHndl npc, AiFlag flag) {
	auto obj = objSystem->GetObject(npc);
	auto aiFlags = obj->GetInt64(obj_f_npc_ai_flags64);
	aiFlags &= ~ (uint64_t)flag;
	obj->SetInt64(obj_f_npc_ai_flags64, aiFlags);
}

AiParamPacket AiSystem::GetAiParams(objHndl obj)
{
	auto aiParamIdx = gameSystems->GetObj().GetObject(obj)->GetInt32(obj_f_npc_ai_data);
	Expects(aiParamIdx >= 0 && aiParamIdx < 100000);
	return aiParams[aiParamIdx];

}

void AiSystem::ShitlistAdd(objHndl npc, objHndl target) {
	_ShitlistAdd(npc, target);
}

void AiSystem::ShitlistRemove(objHndl npc, objHndl target) {
	_AiRemoveFromList(npc, target, 0); // 0 is the shitlist

	auto focus = GetCombatFocus(npc);
	if (focus == target) {
		SetCombatFocus(npc, objHndl::null);
	}
	auto hitMeLast = GetWhoHitMeLast(npc);
	if (hitMeLast == target) {
		SetWhoHitMeLast(npc, objHndl::null);
	}

	_AiSetCombatStatus(npc, 0, objHndl::null, 0); // I think this means stop attacking?
}

BOOL AiSystem::AiListFind(objHndl aiHandle, objHndl tgt, int typeToFind){

	// ensure is NPC handle
	if (!aiHandle)
		return FALSE;
	auto obj = objSystem->GetObject(aiHandle);
	if (!obj->IsNPC())
		return FALSE;

	auto typeListCount = obj->GetInt32Array(obj_f_npc_ai_list_type_idx).GetSize();
	if (!typeListCount)
		return FALSE;

	auto aiListCount = obj->GetObjectIdArray(obj_f_npc_ai_list_idx).GetSize();
	auto N = min(aiListCount, typeListCount);

	for (auto i=0; i < N; i++){
		auto aiListType = obj->GetInt32(obj_f_npc_ai_list_type_idx, i);
		if (aiListType != typeToFind)
			continue;
		
		auto aiListItem = obj->GetObjHndl(obj_f_npc_ai_list_idx, i);
		if (aiListItem == tgt)
			return TRUE;
	}

	return FALSE;
}

void AiSystem::FleeAdd(objHndl npc, objHndl target) {
	_FleeAdd(npc, target);
}

void AiSystem::StopAttacking(objHndl npc) {
	_StopAttacking(npc);
}

void AiSystem::ProvokeHostility(objHndl agitator, objHndl provokedNpc, int rangeType, int flags)
{
	temple::GetRef<void(__cdecl)(objHndl, objHndl, int, int)>(0x1005E8D0)(agitator, provokedNpc, rangeType, flags);
}

int AiSystem::GetAllegianceStrength(objHndl aiHandle, objHndl tgt){
	if (aiHandle == tgt)
		return 4;
	if (combat->IsBrawlInProgress())
		return 0;
	auto leader = critterSys.GetLeader(aiHandle);
	if (leader && (leader == tgt || party.IsInParty(tgt) && (party.IsInParty(leader) || party.IsInParty(aiHandle))))
		return 3;
	else{
		if (objSystem->GetObject(aiHandle)->GetInt32(obj_f_spell_flags) & SpellFlags::SF_SPELL_FLEE || d20Sys.d20Query(aiHandle, DK_QUE_Critter_Is_Charmed))
			return 0;
		if (critterSys.AllegianceShared(aiHandle, tgt))
			return 1;
		return 0;
		// there's a stub here for value 2
	}
	return 0;
}

BOOL AiSystem::CannotHear(objHndl handle, objHndl tgt, int tileRangeIdx){
	return temple::GetRef<BOOL(__cdecl)(objHndl, objHndl, int)>(0x10059A10)(handle, tgt, tileRangeIdx);
}

objHndl AiSystem::GetCombatFocus(objHndl npc) {
	auto obj = objSystem->GetObject(npc);
	return obj->GetObjHndl(obj_f_npc_combat_focus);
}

objHndl AiSystem::GetWhoHitMeLast(objHndl npc) {
	auto obj = objSystem->GetObject(npc);
	return obj->GetObjHndl(obj_f_npc_who_hit_me_last);
}

BOOL AiSystem::ConsiderTarget(objHndl obj, objHndl tgt)
{
	if (!tgt || tgt == obj)
		return 0;

	auto tgtObj = gameSystems->GetObj().GetObject(tgt);
	if ( tgtObj->GetFlags() & (OF_INVULNERABLE | OF_DONTDRAW | OF_OFF | OF_DESTROYED) ){
		return 0;
	}

	auto objBody = gameSystems->GetObj().GetObject(obj);

	auto leader = critterSys.GetLeader(obj);
	if (!tgtObj->IsCritter()){
		auto isBusted = temple::GetRef<BOOL(__cdecl)(objHndl)>(0x1001E730);
		if (isBusted(tgt))
			return 0;
	} 
	
	else	{
		auto npcAiListFindAlly = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x1005C200);
		if (npcAiListFindAlly(obj, tgt))
			return 0;
		if (critterSys.IsDeadNullDestroyed(tgt))
			return 0;
		if (critterSys.IsDeadOrUnconscious(tgt)){
			if (objBody->GetInt32(obj_f_critter_flags) & OCF_NON_LETHAL_COMBAT)
				return 0;
			auto aiFindSuitableCritter = temple::GetRef<objHndl(__cdecl)(objHndl)>(0x1005CED0);
			auto suitableCrit = aiFindSuitableCritter(obj);
			if (suitableCrit && suitableCrit != tgt)
				return 0;
		}
		if (tgt == leader)
			return 0;

		auto tgtLeader = critterSys.GetLeader(tgt);
		if (tgtLeader && tgtLeader == leader)
			return 0;

		auto isCharmedBy = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x10057C50);
		if (isCharmedBy(tgt, obj)){
			auto targetIsPcPartyNotDead = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x1005B7D0);
			return targetIsPcPartyNotDead(obj, tgt);
		}

		if (critterSys.IsFriendly(obj, tgt))
			return 0;
	}

	if (locSys.DistanceToObj(obj, tgt) > 125.0)
		return 0;
	if (leader){
		int64_t tileDelta = locSys.GetTileDeltaMax(leader, tgt);
		if (tileDelta > 20)
			return 0;
	}
	return 1;
}

int AiSystem::CannotHate(objHndl aiHandle, objHndl triggerer, objHndl aiLeader){
	auto obj = objSystem->GetObject(aiHandle);
	if (obj->GetInt32(obj_f_spell_flags) & SpellFlags::SF_SPELL_FLEE && critterSys.GetLeaderForNpc(aiHandle))
		return 0;
	if (!triggerer || !objSystem->GetObject(triggerer)->IsCritter())
		return 0;
	if (critterSys.GetLeader(triggerer) == aiLeader)
		return 4;
	if (critterSys.AllegianceShared(aiHandle, triggerer))
		return 3;
	if (d20Sys.d20QueryWithData(aiHandle, DK_QUE_Critter_Has_Condition, conds.GetByName("sp-Sanctuary Save Failed"), 0) != TRUE
		|| d20Sys.d20QueryWithData(triggerer, DK_QUE_Critter_Has_Condition, conds.GetByName("sp-Sanctuary"), 0) != TRUE)
		return 0;
	else{
		auto triggererSanctuaryHandle = d20Sys.d20QueryReturnData(triggerer, DK_QUE_Critter_Has_Condition, conds.GetByName("sp-Sanctuary"), 0);
		auto sancHandle = d20Sys.d20QueryReturnData(aiHandle, DK_QUE_Critter_Has_Condition, conds.GetByName("sp-Sanctuary Save Failed"), 0);
		if (sancHandle == triggererSanctuaryHandle)
			return 5;
	}

	return 0;
}

int AiSystem::WillKos(objHndl aiHandle, objHndl triggerer){

	if (!ConsiderTarget(aiHandle, triggerer))
		return FALSE;

	auto leader = critterSys.GetLeaderForNpc(aiHandle);
	if (!pythonObjIntegration.ExecuteObjectScript(triggerer, aiHandle, ObjScriptEvent::WillKos))
		return FALSE;

	if (AiListFind(aiHandle, triggerer, 1))
		return FALSE;

	AiFightStatus aiFightStatus;
	objHndl curTgt = objHndl::null;
	GetAiFightStatus(aiHandle, &aiFightStatus, &curTgt);

	if (aiFightStatus == AIFS_SURRENDERED && curTgt == triggerer)
		return FALSE;

	if (!leader && !party.IsInParty(aiHandle)){
		auto npcFlags = objSystem->GetObject(aiHandle)->GetNPCFlags();
		if ((npcFlags & NpcFlag::ONF_KOS) && !(npcFlags & NpcFlag::ONF_KOS_OVERRIDE)) {

			if (!critterSys.AllegianceShared(aiHandle, triggerer) && (objSystem->GetObject(triggerer)->IsPC() || !factions.HasNullFaction(triggerer))) {
				return 1;
			}


			if (d20Sys.d20Query(triggerer, DK_QUE_Critter_Is_Charmed)) {
				objHndl charmer;
				charmer = d20Sys.d20QueryReturnData(triggerer, DK_QUE_Critter_Is_Charmed);
				auto res = WillKos(aiHandle, charmer);
				if (res != 0)
				{
					return res;
				}
					
			}
		}

		// check AI Params hostility threshold
		if (objSystem->GetObject(triggerer)->IsPC()) {
			auto aiPar = GetAiParams(aiHandle);
			auto reac = critterSys.GetReaction(aiHandle, triggerer);
			if (reac <= aiPar.hostilityThreshold){
				return 2;
			}
				
			return 0;
		}
	}

	if (!objSystem->GetObject(triggerer)->IsNPC())
		return FALSE;
	GetAiFightStatus(triggerer, &aiFightStatus, &curTgt);
	if (aiFightStatus != AIFS_FIGHTING || !curTgt)
		return FALSE;
	if (GetAllegianceStrength(aiHandle, curTgt) == 0) {
		return FALSE; // there's a stub for more extensive logic here...
	}
	leader = critterSys.GetLeader(aiHandle);
	if (!CannotHate(aiHandle, triggerer, leader)){
		return 4;
	}
		
	return FALSE;

}

void AiSystem::SetCombatFocus(objHndl npc, objHndl target) {
	auto obj = objSystem->GetObject(npc);
	obj->SetObjHndl(obj_f_npc_combat_focus, target);
}

void AiSystem::SetWhoHitMeLast(objHndl npc, objHndl target) {
	auto obj = objSystem->GetObject(npc);
	obj->SetObjHndl(obj_f_npc_who_hit_me_last, target);
}

void AiSystem::GetAiFightStatus(objHndl handle, AiFightStatus* status, objHndl* target)
{
	auto obj = gameSystems->GetObj().GetObject(handle);
	auto critFlags = (CritterFlag)obj->GetInt32(obj_f_critter_flags);

	if (critFlags & OCF_FLEEING){
		if (target){
			*target = obj->GetObjHndl(obj_f_critter_fleeing_from);
		}
		*status = AIFS_FLEEING;
		return;
	}

	if (critFlags & OCF_SURRENDERED){
		if (target)
			*target = obj->GetObjHndl(obj_f_critter_fleeing_from);
		*status = AIFS_SURRENDERED;
		return;
	}

	auto aiFlags = static_cast<AiFlag>(obj->GetInt64(obj_f_npc_ai_flags64));
	if (aiFlags & AiFlag::Fighting)
	{
		if (target)
			*target = obj->GetObjHndl(obj_f_npc_combat_focus);
		*status = AIFS_FIGHTING;
		return;
	}
	if (aiFlags & AiFlag::FindingHelp)
	{
		if (target)
			*target = obj->GetObjHndl(obj_f_npc_combat_focus);
		*status = AIFS_FINDING_HELP;
		return;
	}

	if (target)
		*target = 0i64;
	*status = AIFS_NONE;
	return;
}

void AiSystem::FightOrFlight(objHndl obj, objHndl tgt)
{
	auto shouldFlee = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x1005C570);
	if (shouldFlee(obj, tgt))
	{
		UpdateAiFlags(obj, AIFS_FLEEING, tgt, nullptr);
	} else
	{
		UpdateAiFlags(obj, AIFS_FIGHTING, tgt, nullptr);
	}
}

void AiSystem::FightStatusProcess(objHndl obj, objHndl newTgt)
{
	if (critterSys.IsDeadNullDestroyed(obj))
	{
		return;
	}

	static auto CheckNewTgt = [](objHndl _obj, objHndl _curTgt, objHndl _newTgt)->objHndl
	{
		if (!_curTgt)
			return _newTgt;
		if (!_newTgt)
			return _curTgt;
		if (!objects.IsCritter(_newTgt))
			return _curTgt;
		if (!objects.IsCritter(_curTgt) 
			|| !critterSys.IsDeadOrUnconscious(_newTgt)	&& critterSys.IsDeadOrUnconscious(_curTgt)
			|| locSys.DistanceToObj(_obj, _newTgt) <= 125.0 && (locSys.DistanceToObj(_obj, _curTgt) > 125.0 || critterSys.IsFriendly(_obj, _curTgt)) 
			)
			return _newTgt;
		else
			return _curTgt;
	};
	static auto WithinFleeDistance = [](objHndl _obj, objHndl _tgt)->bool
	{
		if (gameSystems->GetObj().GetObject(_obj)->GetInt32(obj_f_spell_flags) & SpellFlags::SF_SPELL_FLEE)
			return true;
		AiParamPacket aiPar = aiSys.GetAiParams(_obj);
		auto distTo = locSys.DistanceToObj(_obj, _tgt);
		if (aiPar.fleeDistanceFeet < distTo)
			return true;
		else
			return false;
	};

	AiFightStatus status; 
	objHndl curTgt;
	GetAiFightStatus(obj, &status, &curTgt);
	switch (status ){
	case AIFS_NONE:
		FightOrFlight(obj, newTgt);
		break;
	case AIFS_FIGHTING:
		if (newTgt == curTgt || CheckNewTgt(obj, curTgt, newTgt) == newTgt)	{
			FightOrFlight(obj, newTgt);
		}
		break;
	case AIFS_FLEEING:
		if (curTgt != newTgt && (!curTgt || WithinFleeDistance(obj, curTgt)))
			FightOrFlight(obj, newTgt);
		break;
	case AIFS_SURRENDERED:
		if (newTgt == curTgt || CheckNewTgt(obj, curTgt, newTgt) == newTgt) {
			FightOrFlight(obj, newTgt);
		} else
		{
			if (!critterSys.IsDeadOrUnconscious(obj))
			{
				auto getFleeVoiceLine = temple::GetRef<void(__cdecl)(objHndl, objHndl, char*, int*)>(0x100374E0);
				char fleeText[1000];
				int soundId;
				getFleeVoiceLine(obj, newTgt, fleeText, &soundId);
				auto dialogPlayer = temple::GetRef<void(__cdecl*)(objHndl, objHndl, char*, int)>(0x10AA73B0);
				dialogPlayer(obj, newTgt, fleeText, soundId);
			}
			FleeProcess(obj, newTgt);
		}
		break;
	default:
		break;
	}
	combatSys.AddToInitiative(obj);
	return;
}

void AiSystem::FleeProcess(objHndl obj, objHndl fleeingFrom)
{
	auto fleeProc = temple::GetRef<void(__cdecl)(objHndl, objHndl)>(0x1005A1F0);
	fleeProc(obj, fleeingFrom);
}

int AiSystem::TargetClosest(AiTactic* aiTac)
{
	objHndl performer = aiTac->performer;
	int performerIsIntelligent = (objects.StatLevelGet(performer, stat_intelligence) >= 3);
	//objHndl target; 
	LocAndOffsets performerLoc; 
	float dist = 1000000000.0, reach = critterSys.GetReach(performer, D20A_UNSPECIFIED_ATTACK);
	bool hasGoodTarget = false;

	
	locSys.getLocAndOff(aiTac->performer, &performerLoc);
	/*
	if (combatSys.GetClosestEnemy(aiTac->performer, &performerLoc, &target, &dist, 0x21))
	{
		aiTac->target = target;
		
	}
	*/

	logger->debug("{} targeting closest...", objects.description.getDisplayName(performer));

	// ObjList objlist;
	// objlist.ListVicinity(performerLoc.location, OLC_CRITTERS);

	combatSys.groupInitiativeList->GroupSize;

	auto args = PyTuple_New(2);
	PyTuple_SET_ITEM(args, 0, PyObjHndl_Create(performer));

	for ( uint32_t i = 0; i < combatSys.groupInitiativeList->GroupSize; i++)
	{
		// objHndl dude = objlist.get(i);
		objHndl combatant = combatSys.groupInitiativeList->GroupMembers[i];
		PyTuple_SET_ITEM(args, 1, PyObjHndl_Create(combatant));

		auto result = pythonObjIntegration.ExecuteScript("combat", "ShouldIgnoreTarget", args);
		//auto result2 = pythonObjIntegration.ExecuteScript("combat", "TargetClosest", args);
		int ignoreTarget = PyInt_AsLong(result);
		Py_DECREF(result);


		if (!critterSys.IsFriendly(combatant, performer)
			&& !critterSys.IsDeadOrUnconscious(combatant)
			&& !ignoreTarget)
		{
			auto distToCombatant = locSys.DistanceToObj(performer, combatant);
			//logger->debug("Checking line of attack for target: {}", description.getDisplayName(combatant));
			bool hasLineOfAttack = combatSys.HasLineOfAttack(performer, combatant);
			if (d20Sys.d20Query(combatant, DK_QUE_Critter_Is_Invisible)
				&& !d20Sys.d20Query(performer, DK_QUE_Critter_Can_See_Invisible))
			{
				distToCombatant = static_cast<float>((distToCombatant + 5.0) * 2.5); // makes invisibile chars less likely to be attacked; also takes into accout stuff like Hide From Animals (albeit in a shitty manner)
			}
			bool isGoodTarget = distToCombatant <= reach && hasLineOfAttack;
		
			if (isGoodTarget)
			{
				if (distToCombatant < dist ) // best
				{
					aiTac->target = combatant;
					dist = distToCombatant;
					hasGoodTarget = true;
		}
				else if (!hasGoodTarget) // is a good target within reach, not necessarily the closest so far, but other good targets haven't been found yet
				{
					aiTac->target = combatant;
					dist = distToCombatant;
					hasGoodTarget = true;
	}
			}
			else if (distToCombatant < dist && !hasGoodTarget)
			{
				aiTac->target = combatant;
				dist = distToCombatant;
			} 
		}

	}
	logger->info("{} targeted.", objects.description.getDisplayName(aiTac->target, aiTac->performer));

	return 0;
}

BOOL AiSystem::TargetDamaged(AiTactic * aiTac){

	auto performer = aiTac->performer;
	auto lowest = 1.1;
	auto N = combatSys.GetInitiativeListLength();
	for (auto i = 0; i < N; i++) {
		auto combatant = combatSys.GetInitiativeListMember(i);
		if (!combatant || combatant == aiTac->performer)
			continue;

		if (critterSys.IsDeadOrUnconscious(combatant) || critterSys.IsFriendly(performer, combatant))
			continue;

		auto hpCur = objects.StatLevelGet(combatant, stat_hp_current);
		auto hpMax = objects.StatLevelGet(combatant, stat_hp_max);
		auto hpPct = hpCur * 1.0 / hpMax;
		auto priority = hpPct;
		if (d20Sys.d20Query(combatant, DK_QUE_Critter_Is_Invisible)
			&& !d20Sys.d20Query(performer, DK_QUE_Critter_Can_See_Invisible))
			priority += 5.0;
		if (priority < lowest) {
			lowest = priority;
			aiTac->target = combatant;
		}
			
	}

	return FALSE;
}

int AiSystem::TargetThreatened(AiTactic* aiTac)
{
	objHndl performer = aiTac->performer;
	int performerIsIntelligent = (objects.StatLevelGet(performer, stat_intelligence) >= 3);
//	objHndl target;
	LocAndOffsets performerLoc;
	float dist = 1000000000.0;


	locSys.getLocAndOff(aiTac->performer, &performerLoc);

	logger->info("{} targeting threatened...", objects.description.getDisplayName(aiTac->performer));

	ObjList objlist;
	objlist.ListVicinity(performerLoc.location, OLC_CRITTERS);

	auto args = PyTuple_New(2);
	PyTuple_SET_ITEM(args, 0, PyObjHndl_Create(performer));
	
	for (int i = 0; i < objlist.size(); i++)
	{
		objHndl dude = objlist.get(i);
		PyTuple_SET_ITEM(args, 1, PyObjHndl_Create(dude));

		auto result = pythonObjIntegration.ExecuteScript("combat", "ShouldIgnoreTarget", args);
		int ignoreTarget = PyInt_AsLong(result);
		Py_DECREF(result);
		

		if (!critterSys.IsFriendly(dude, performer)
			&& !critterSys.IsDeadNullDestroyed(dude)
			&& locSys.DistanceToObj(performer, dude)  < dist
			&& combatSys.IsWithinReach(performer, dude) 
			&& !ignoreTarget)
		{
			aiTac->target = dude;
			dist = locSys.DistanceToObj(performer, dude);
		}
	}
	Py_DECREF(args);
	if (dist > 900000000.0)
	{
		aiTac->target = 0;
		//hooked_print_debug_message(" no target found. Attempting Target Closest instead.");
		logger->info("no target found. ");
		//TargetClosest(aiTac);
	} 
	else
	{
		logger->info("{} targeted.", objects.description.getDisplayName(aiTac->target, aiTac->performer));
	}
	

	return 0;
}
BOOL AiSystem::UsePotion(AiTactic * aiTac){

	auto performer = aiTac->performer;
	auto hpCur = objects.StatLevelGet(performer, stat_hp_current);
	auto hpMax = objects.StatLevelGet(performer, stat_hp_max);
	float hpPct = hpCur * 1.0 / hpMax;

	auto objInventory =	inventory.GetInventory(performer);
	if (!objInventory.size())
		return FALSE;

	// check if critter can get whacked by AoO
	auto threateningCombatants = combatSys.GetEnemiesCanMelee(performer);
	if (threateningCombatants.size()) {
		logger->info("Skipping Use Potion action because threatened by critters.");
		return FALSE;
	}

	for (auto itemHandle : objInventory) {
		auto itemObj = objSystem->GetObject(itemHandle);
		if (itemObj->type != obj_t_food)
			continue;

		if (!itemObj->GetSpellArray(obj_f_item_spell_idx).GetSize())
			continue;

		auto spData = itemObj->GetSpell(obj_f_item_spell_idx, 0);
		auto spEnum = spData.spellEnum;

		if (spEnum <= 0)
			continue;

		auto spellAiTypeIsHealing =
			spellSys.SpellHasAiType(spEnum, AiSpellType::ai_action_heal_heavy)
			|| spellSys.SpellHasAiType(spEnum, AiSpellType::ai_action_heal_light)
			|| spellSys.SpellHasAiType(spEnum, AiSpellType::ai_action_heal_medium);
		if (spellAiTypeIsHealing) {
			if (hpPct >= 0.5)
				continue;
			// prevent usage of light healing when already fairly healthy...
			if (spellSys.SpellHasAiType(spEnum, AiSpellType::ai_action_heal_light) && hpCur >= 25)
				continue;
		}
		
		auto doUseItemAction = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl, objHndl)>(0x10098C90);
		if (doUseItemAction(performer, performer, itemHandle))
			return TRUE;

	}

	return FALSE;
}
;

int AiSystem::Approach(AiTactic* aiTac)
{
	int initialActNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	if (!aiTac->target)
		return 0;
	if (combatSys.IsWithinReach(aiTac->performer, aiTac->target))
		return 0;
	actSeqSys.curSeqReset(aiTac->performer);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_UNSPECIFIED_MOVE, 0);
	d20Sys.GlobD20ActnSetTarget(aiTac->target, 0);
	actSeqSys.ActionAddToSeq();
	if (actSeqSys.ActionSequenceChecksWithPerformerLocation())
	{
		actSeqSys.ActionSequenceRevertPath(initialActNum);
		return 0;
	}
	return 1;
}

int AiSystem::CastParty(AiTactic* aiTac)
{
	auto initialActNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	if (!aiTac->target)
		return 0;
	objHndl enemiesCanMelee[40];
	int castDefensively = 0;
	if (combatSys.GetEnemiesCanMelee(aiTac->performer, enemiesCanMelee) > 0)
		castDefensively = 1;
	d20Sys.d20SendSignal(aiTac->performer, DK_SIG_SetCastDefensively, castDefensively, 0);
	LocAndOffsets targetLoc =	objects.GetLocationFull(aiTac->target);
	auto partyLen = party.GroupListGetLen();
	for (auto i = 0u; i < partyLen; i++)
	{
		aiTac->spellPktBody.targetListHandles[i] = party.GroupListGetMemberN(i);
	}
	aiTac->spellPktBody.targetCount = partyLen;

	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_CAST_SPELL, 0);
	actSeqSys.ActSeqCurSetSpellPacket(&aiTac->spellPktBody, 1); // ignore LOS changed to 1, was originally 0
	d20Sys.GlobD20ActnSetSpellData(&aiTac->d20SpellData);
	d20Sys.GlobD20ActnSetTarget(aiTac->target, &targetLoc); // originally fetched a concious party member, seems like a bug so I changed it to the target
	actSeqSys.ActionAddToSeq();
	if (actSeqSys.ActionSequenceChecksWithPerformerLocation() != AEC_OK)
	{
		actSeqSys.ActionSequenceRevertPath(initialActNum);
		return 0;
	}
	return 1;
}

int AiSystem::PickUpWeapon(AiTactic* aiTac)
{
	int d20aNum;

	d20aNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;

	if (inventory.ItemWornAt(aiTac->performer, 203) || inventory.ItemWornAt(aiTac->performer, 204))
	{
		return 0;
	}
	if (!d20Sys.d20Query(aiTac->performer, DK_QUE_Disarmed))
		return 0;
	objHndl weapon{ d20Sys.d20QueryReturnData(aiTac->performer, DK_QUE_Disarmed) };

	if (weapon && !inventory.GetParent(weapon))
	{
		aiTac->target = weapon;
	} else
	{
		LocAndOffsets loc;
		locSys.getLocAndOff(aiTac->performer, &loc);

		ObjList objList;
		objList.ListTile(loc.location, OLC_WEAPON);

		if (objList.size() > 0)
		{
			weapon = objList.get(0);
			aiTac->target = weapon;
		} else
		{
			aiTac->target = 0i64;
		}
	}
	
	
	
	if (!aiTac->target || objects.IsCritter(aiTac->target))
	{
		return 0;
	}


	if (!combatSys.IsWithinReach(aiTac->performer, aiTac->target))
		return 0;
	actSeqSys.curSeqReset(aiTac->performer);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_DISARMED_WEAPON_RETRIEVE, 0);
	d20Sys.GlobD20ActnSetTarget(aiTac->target, 0);
	actSeqSys.ActionAddToSeq();
	if (actSeqSys.ActionSequenceChecksWithPerformerLocation())
	{
		actSeqSys.ActionSequenceRevertPath(d20aNum);
		return 0;
	}
	return 1;
}

int AiSystem::BreakFree(AiTactic* aiTac)
{
	int d20aNum;
	objHndl performer = aiTac->performer;
	objHndl target;
	LocAndOffsets performerLoc;
	float dist = 0.0;

	d20aNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	if (!d20Sys.d20Query(performer, DK_QUE_Is_BreakFree_Possible))
		return 0;
	int spellId = (int) d20Sys.d20QueryReturnData(performer, DK_QUE_Is_BreakFree_Possible);

	locSys.getLocAndOff(aiTac->performer, &performerLoc);
	if (combatSys.GetClosestEnemy(aiTac->performer, &performerLoc, &target, &dist, 0x21))
	{
		if (combatSys.IsWithinReach(aiTac->performer, target))
			return 0;
	}
	actSeqSys.curSeqReset(aiTac->performer);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_BREAK_FREE, spellId);
	//d20Sys.GlobD20ActnSetTarget(aiTac->performer, 0);
	actSeqSys.ActionAddToSeq();
	if (actSeqSys.ActionSequenceChecksWithPerformerLocation())
	{
		actSeqSys.ActionSequenceRevertPath(d20aNum);
		return 0;
	}

	return 1;
}

int AiSystem::GoMelee(AiTactic* aiTac)
{
	logger->info("Attempting Go Melee...");
	auto performer = aiTac->performer;
	auto tbStat = actSeqSys.curSeqGetTurnBasedStatus();
	auto weapon = critterSys.GetWornItem(performer, EquipSlot::WeaponPrimary);
	if (!weapon)
		return 0;
	if (!inventory.IsRangedWeapon(weapon))
		return 0;
	objHndl * inventoryArray;
	auto invenCount = inventory.GetInventory(performer, &inventoryArray);
	
	for (int i = 0; i < invenCount; i++)
	{
		if (objects.GetType(inventoryArray[i]) == obj_t_weapon)
		{
			auto weapFlags = objects.getInt32(inventoryArray[i], obj_f_weapon_flags);
			if ( (weapFlags & OWF_RANGED_WEAPON) == 0 )
			{
				inventory.ItemUnwieldByIdx(performer, 203);
				inventory.ItemUnwieldByIdx(performer, 204);
				inventory.ItemPlaceInIdx(inventoryArray[i], 203);
				tbStat->hourglassState = actSeqSys.GetHourglassTransition(tbStat->hourglassState, 1);
				free(inventoryArray);
				logger->info("Go Melee succeeded.");
				return 1;
			}

		}
	}
	
	free(inventoryArray);
	return 0;
	
}

int AiSystem::Sniper(AiTactic* aiTac)
{
	auto weapon = critterSys.GetWornItem(aiTac->performer, EquipSlot::WeaponPrimary);
	if (!weapon)
		return 0;
	if (!inventory.IsRangedWeapon(weapon))
		return 0;
	if (!combatSys.AmmoMatchesItemAtSlot(aiTac->performer, EquipSlot::WeaponPrimary))
	{
		GoMelee(aiTac);
		return 0;
	}
	if (!AiFiveFootStepAttempt(aiTac))
		GoMelee(aiTac);
	return Default(aiTac);
}

int AiSystem::CoupDeGrace(AiTactic* aiTac){
	int actNum; 
	objHndl origTarget = aiTac->target;

	actNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	aiTac->target = 0i64;
	combatSys.GetClosestEnemy(aiTac, 1);
	auto performer = gameSystems->GetObj().GetObject(aiTac->performer);
	auto perfLoc = performer->GetLocationFull();
	
	objHndl threateners[40];
	auto numThreateners = combatSys.GetThreateningCrittersAtLoc(aiTac->performer, &perfLoc, threateners);

	if (aiTac->target){
		if (critterSys.IsDeadOrUnconscious(aiTac->target)) { // if the CDG is due to unconscious target...
			if(!combatSys.CanMeleeTarget(aiTac->performer, aiTac->target)) // if it's not in melee range, forget it
				return 0;
			if (numThreateners > 0) // if the AI is threatened, forget it too (since it incurs an AOO)
				return 0;
		}
		if (Approach(aiTac)
			|| (d20Sys.GlobD20ActnInit(),
			d20Sys.GlobD20ActnSetTypeAndData1(D20A_COUP_DE_GRACE, 0),
			d20Sys.GlobD20ActnSetTarget(aiTac->target, 0),
			actSeqSys.ActionAddToSeq(),
			!actSeqSys.ActionSequenceChecksWithPerformerLocation()))
		{
			return 1;
		}
		actSeqSys.ActionSequenceRevertPath(actNum);
		return 0;
		
	}

	aiTac->target = origTarget;
	return 0;


}

int AiSystem::ChargeAttack(AiTactic* aiTac)
{
	int actNum0 = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	if (!aiTac->target)
		return 0;
	actSeqSys.curSeqReset(aiTac->performer);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_CHARGE, 0);
	objHndl weapon = inventory.ItemWornAt(aiTac->performer, 3);
	if (!weapon)
	{
		//TurnBasedStatus tbStatCopy;
		D20Actn d20aCopy;
		//memcpy(&tbStatCopy, &(*actSeqSys.actSeqCur)->tbStatus, sizeof(TurnBasedStatus));
		memcpy(&d20aCopy, d20Sys.globD20Action, sizeof(D20Actn));
		int natAtk = dispatch.DispatchD20ActionCheck(&d20aCopy, 0, dispTypeGetCritterNaturalAttacksNum);
		if (natAtk > 0)
			d20Sys.GlobD20ActnSetTypeAndData1(D20A_CHARGE, ATTACK_CODE_NATURAL_ATTACK + 1);
		else
			d20Sys.GlobD20ActnSetTypeAndData1(D20A_CHARGE, 0);
	}
	else
		d20Sys.GlobD20ActnSetTypeAndData1(D20A_CHARGE, 0);

	d20Sys.GlobD20ActnSetTarget(aiTac->target, 0);
	actSeqSys.ActionAddToSeq();
	if (actSeqSys.ActionSequenceChecksWithPerformerLocation())
	{
		actSeqSys.ActionSequenceRevertPath(actNum0);
		return 0;
	}
	return 1;
}


int AiSystem::UpdateAiFlags(objHndl obj, AiFightStatus aiFightStatus, objHndl target, int* soundMap)
{
	return addresses.UpdateAiFlags(obj, aiFightStatus, target, soundMap);
}

void AiSystem::StrategyTabLineParseTactic(AiStrategy* aiStrat, char* tacName, char* middleString, char* spellString)
{ // this functions matches the tactic strings (3 strings) to a tactic def
	int tacIdx = 0;
	if (*tacName)
	{
		for (int i = 0; i < 44 && _stricmp(tacName, aiTacticDefs[i].name); i++){
			++tacIdx;
		}
		if (tacIdx < 44)
		{
			aiStrat->aiTacDefs[aiStrat->numTactics] = &aiTacticDefs[tacIdx];
			aiStrat->field54[aiStrat->numTactics] = 0;
			aiStrat->spellsKnown[aiStrat->numTactics].spellEnum = -1;
			if (*spellString)
				spell->ParseSpellSpecString(&aiStrat->spellsKnown[aiStrat->numTactics], (char *)spellString);
			++aiStrat->numTactics;
			return;
		}
		tacIdx = 0;
		for (int i = 0; i < 100 && aiTacticDefsNew[i].name && _stricmp(tacName, aiTacticDefsNew[i].name); i++)
		{
			tacIdx++;
		}
		if (aiTacticDefsNew[tacIdx].name && tacIdx < 100)
		{
			aiStrat->aiTacDefs[aiStrat->numTactics] = &aiTacticDefsNew[tacIdx];
			aiStrat->field54[aiStrat->numTactics] = 0;
			aiStrat->spellsKnown[aiStrat->numTactics].spellEnum = -1;
			if (*spellString)
				spell->ParseSpellSpecString(&aiStrat->spellsKnown[aiStrat->numTactics], (char *)spellString);
			++aiStrat->numTactics;
			return;
		}
		logger->warn("Error: No Such Tactic {} for Strategy {}", tacName, aiStrat->name);
		return;
		
		
	}
	return;
}

int AiSystem::StrategyTabLineParser(TabFileStatus* tabFile, int n, char** strings)
{
	AiStrategy *aiStrat; 
	const char *v4; 
	signed int i; 
	char v6; 
	char *stratName; 
	unsigned int v8; 
	char **v9; 

	aiStrat = *aiStrategies;
	aiStrat = &aiStrat[n];
	
	v4 = *strings;
	i = 0;
	do
		v6 = *v4++;
	while (v6);
	stratName = (char *)malloc(v4 - ( (*strings )+ 1) + 1);
	aiStrat->name = stratName;
	strcpy((char *)stratName, *strings);
	aiStrat->numTactics = 0;
	v8 = 3;
	v9 = strings + 2;
	do
	{
		if (tabFile->numTabsMax < v8)
			break;
		StrategyTabLineParseTactic(aiStrat, *(v9 - 1), *v9, v9[1]);
		v8 += 3;
		v9 += 3;
		++i;
	} while (i < 20);
	++(*aiStrategiesNum);
	return 0;
}

int AiSystem::AiOnInitiativeAdd(objHndl obj)
{
	if (party.IsInParty(obj) && objects.IsPlayerControlled(obj)){
		return 0;
	}

	int critterStratIdx = objects.getInt32(obj, obj_f_critter_strategy);
	
	assert(critterStratIdx >= 0 && (uint32_t) critterStratIdx < *aiStrategiesNum);
	AiStrategy *aiStrats = *aiStrategies;
	AiStrategy * aiStrat = &aiStrats[critterStratIdx];
	AiTactic aiTac;
	aiTac.performer = obj;

	for (auto i = 0u; i < aiStrat->numTactics; i++)
	{
		aiTacticGetConfig(i, &aiTac, aiStrat);
		auto func = aiTac.aiTac->onInitiativeAdd;
		if (func)
		{
			if (func(&aiTac))
				return 1;
		}
	}
	return 0;
}

AiCombatRole AiSystem::GetRole(objHndl obj)
{
	if (critterSys.IsCaster(obj))
		return AiCombatRole::caster;
	return AiCombatRole::general;
}

BOOL AiSystem::AiFiveFootStepAttempt(AiTactic* aiTac)
{
	objHndl threateners[40];
	auto actNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	if (!combatSys.GetEnemiesCanMelee(aiTac->performer, threateners))
		return 1;
	float overallOffX, overallOffY;
	LocAndOffsets loc;
	locSys.getLocAndOff(aiTac->performer, &loc);
	locSys.GetOverallOffset(loc, &overallOffX, &overallOffY);
	for (float angleDeg = 0.0; angleDeg <= 360.0;  angleDeg += 45.0 )
	{

		auto angleRad = static_cast<float>(angleDeg * 0.017453292); // to radians
		auto cosTheta = cosf(angleRad);
		auto sinTheta = sinf(angleRad);
		auto fiveFootStepX = overallOffX - cosTheta * 60.0f; // five feet radius
		auto fiveFootStepY = overallOffY + sinTheta * 60.0f;
		auto fiveFootLoc = LocAndOffsets::FromInches(fiveFootStepX, fiveFootStepY);
		if (!combatSys.GetThreateningCrittersAtLoc(aiTac->performer, &fiveFootLoc, threateners))
		{
			d20Sys.GlobD20ActnSetTypeAndData1(D20A_5FOOTSTEP, 0);
			d20Sys.GlobD20ActnSetTarget(objHndl::null, &fiveFootLoc);
			if (!actSeqSys.ActionAddToSeq()
				&& actSeqSys.GetPathTargetLocFromCurD20Action(&fiveFootLoc)
				&& !combatSys.GetThreateningCrittersAtLoc(aiTac->performer, &fiveFootLoc, threateners)
				&& !actSeqSys.ActionSequenceChecksWithPerformerLocation())
				return 1;
			actSeqSys.ActionSequenceRevertPath(actNum);
		}
	}
	return 0;
}

void AiSystem::RegisterNewAiTactics()
{
	memset(aiTacticDefsNew, 0, sizeof(AiTacticDef) * AI_TACTICS_NEW_SIZE);

	int n = 0;

	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiAsplode;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "asplode");


	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiWakeFriend;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "wake friend");

	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiAsplode;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "use magic item");
	
	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiAsplode;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "improve position five foot step");

	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiAsplode;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "disarm");

	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiAsplode;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "sorcerer cast once single");

	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiAsplode;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "coordinated strategy");

	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiAsplode;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "cast best offensive");

	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiAsplode;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "cast best crowd control");
}

int AiSystem::GetStrategyIdx(const char* stratName) const
{
	int result = -1;
	for (auto i = 0u; i < *aiStrategiesNum; i++)
	{
		if (_stricmp(stratName, (*aiStrategies)[i].name) == 0)
		{
			return i;
		}
	}

	return result;
	
}

int AiSystem::GetAiSpells(AiSpellList* aiSpell, objHndl obj, AiSpellType aiSpellType)
{
	aiSpell->spellEnums.clear();
	aiSpell->spellData.clear();
	auto objBod = objSystem->GetObject(obj);
	auto spellsMemo = objBod->GetSpellArray(obj_f_critter_spells_memorized_idx);
	for (auto i = 0u; i < spellsMemo.GetSize(); i++)
	{
		auto spellData = spellsMemo[i];
		if (spellData.spellStoreState.usedUp & 1)
			continue;
		SpellEntry spellEntry;
		if (!spellSys.spellRegistryCopy(spellData.spellEnum, &spellEntry))
			continue;
		if (spellEntry.aiTypeBitmask == 0)
			continue;
		if (!((spellEntry.aiTypeBitmask & (1 << aiSpellType)) == (1 << aiSpellType)))
			continue;
		
		
		bool spellAlreadyFound = false;
		for (auto j = 0u; j < aiSpell->spellEnums.size();j++)
		{
			if (aiSpell->spellEnums[j] == spellData.spellEnum)
			{
				spellAlreadyFound = true;
				break;
			}
		}

		if (!spellAlreadyFound)
		{
			aiSpell->spellEnums.push_back(spellData.spellEnum);
			D20SpellData d20SpellData;
			d20SpellData.spellEnumOrg = spellData.spellEnum;
			d20SpellData.spellClassCode = spellData.classCode;
			d20SpellData.metaMagicData = spellData.metaMagicData;
			d20SpellData.itemSpellData = -1;
			d20SpellData.spellSlotLevel = spellData.spellLevel; // hey, I think this was missing / wrong in the original code!

			aiSpell->spellData.push_back(d20SpellData);
		}
	}

	return 1;
}

int AiSystem::ChooseRandomSpell(AiPacket* aiPkt)
{
	auto aiNpcFightingStatus = temple::GetRef<int>(0x102BD4E0);
	if (!aiNpcFightingStatus)
		return 0;

	auto obj = aiPkt->obj;
	AiSpellList aiSpell;

	if (!critterSys.GetNumFollowers(aiPkt->obj, 0))
	{
		GetAiSpells(&aiSpell, obj, AiSpellType::ai_action_summon);
		if ( ChooseRandomSpellFromList(aiPkt, &aiSpell)){
			return 1;
		}
	}

	auto aiDataIdx = objects.getInt32(obj, obj_f_npc_ai_data);
	Expects(aiDataIdx >= 0 && aiDataIdx <= 150);
	AiParamPacket aiParam = aiParams[aiDataIdx];
	
	if (aiParam.defensiveSpellChance > rngSys.GetInt(1,100))
	{
		GetAiSpells(&aiSpell, obj, AiSpellType::ai_action_defensive);
		if (ChooseRandomSpellFromList(aiPkt, &aiSpell)) {
			return 1;
		}
	}
	if (aiParam.offensiveSpellChance > rngSys.GetInt(1,100))
	{
		GetAiSpells(&aiSpell, obj, AiSpellType::ai_action_offensive);
		if (ChooseRandomSpellFromList(aiPkt, &aiSpell)) {
			return 1;
		}
	}
	return 0;
}

unsigned int AiSystem::Asplode(AiTactic* aiTac)
{
	auto performer = aiTac->performer;
	critterSys.KillByEffect(performer);
	return 0;
}

unsigned AiSystem::WakeFriend(AiTactic* aiTac)
{
	objHndl performer = aiTac->performer;
	objHndl sleeper = objHndl::null;
	int performerIsIntelligent = (objects.StatLevelGet(performer, stat_intelligence) >= 3);
	if (!performerIsIntelligent)
		return 0;

	LocAndOffsets performerLoc;
	float sleeperDist = 1000000000.0, enemyDist = 100000.0;


	locSys.getLocAndOff(aiTac->performer, &performerLoc);

	ObjList objlist;
	objlist.ListVicinity(performerLoc.location, OLC_CRITTERS);

	auto args = PyTuple_New(1);

	for (int i = 0; i < objlist.size(); i++)
	{
		objHndl dude = objlist.get(i);
		PyTuple_SET_ITEM(args, 0, PyObjHndl_Create(dude));

		auto result = pythonObjIntegration.ExecuteScript("combat", "IsSleeping", args);

		int isSleeping = PyInt_AsLong(result);
		Py_DECREF(result);

		if (critterSys.IsFriendly(dude, performer))
		{
			if (isSleeping && locSys.DistanceToObj(performer, dude)  < sleeperDist)
			{
				sleeper = dude;
				sleeperDist = locSys.DistanceToObj(performer, dude);
		}
	}
		
	}
	if (!sleeper)
		return 0;

	// if a sleeper is within reach, do it
	bool shouldWake = combatSys.IsWithinReach(performer, sleeper) != 0;

	// if not:
	// first of all check if anyone threatens the ai actor
	// if no threats, then 40% chance that you'll try to wake up
	if (!shouldWake )
	{
		int enemyCount;
		auto enemies = combatSys.GetHostileCombatantList(performer, &enemyCount);
		bool isThreatened = false;
		for (int i = 0; i < enemyCount; i++)
		{
			//logger->debug("Enemy under test: {}", description.getDisplayName(enemies[i]));
			if (combatSys.CanMeleeTarget(enemies[i], performer))
			{
				isThreatened = true;
				break;
			}
		}
		delete [] enemies;
		if (!isThreatened)
			shouldWake = (rngSys.GetInt(1, 100) <= 40);
	}

	if (!shouldWake)
		return 0;

	// do wake action
		int actNum;
		objHndl origTarget = aiTac->target;

		actNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	aiTac->target = sleeper;

		if (Approach(aiTac)
			|| (d20Sys.GlobD20ActnInit(),
				d20Sys.GlobD20ActnSetTypeAndData1(D20A_AID_ANOTHER_WAKE_UP, 0),
				d20Sys.GlobD20ActnSetTarget(aiTac->target, 0),
				actSeqSys.ActionAddToSeq(),
				!actSeqSys.ActionSequenceChecksWithPerformerLocation()))
		{
			return 1;
		}
		actSeqSys.ActionSequenceRevertPath(actNum);

		aiTac->target = origTarget;
		return 0;

}

int AiSystem::Default(AiTactic* aiTac)
{
	if (!aiTac->target)
		return 0;
	if (actSeqSys.curSeqGetTurnBasedStatus()->hourglassState < 2){
		int dummy = 1;
		//return 0;
	}
		
	auto curSeq = *actSeqSys.actSeqCur;
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_UNSPECIFIED_ATTACK, 0);
	d20Sys.GlobD20ActnSetTarget(aiTac->target, 0);
	ActionErrorCode addToSeqError = (ActionErrorCode)actSeqSys.ActionAddToSeq();
	//if (addToSeqError)
	//{
	//	logger->info("AI Default failed, error code: {}", (int)addToSeqError);
	//}
	int performError = actSeqSys.ActionSequenceChecksWithPerformerLocation();
	if (performError == AEC_OK && addToSeqError == AEC_OK){
		return 1;
	} 
	else{
		logger->info("AI Default SequenceCheck failed, error codes are AddToSeq :{}, Location Checs: {}", static_cast<int>(addToSeqError), static_cast<int>(performError));
	}
	if (!critterSys.IsWieldingRangedWeapon(aiTac->performer))
	{
		if (combatSys.IsWithinReach(aiTac->performer, aiTac->target))
			return 0;

		logger->info("AI Action Perform: Resetting sequence; Do Unspecified Move Action");
		actSeqSys.curSeqReset(aiTac->performer);
		auto initialActNum = curSeq->d20ActArrayNum;
		d20Sys.GlobD20ActnInit();
		d20Sys.GlobD20ActnSetTypeAndData1(D20A_UNSPECIFIED_MOVE, 0);
		d20Sys.GlobD20ActnSetTarget(aiTac->target, 0);
		addToSeqError = (ActionErrorCode)actSeqSys.ActionAddToSeq();
		performError = actSeqSys.ActionSequenceChecksWithPerformerLocation();
		
		if (addToSeqError != AEC_OK || performError != AEC_OK )
		{
			logger->info("AI Default: Unspecified Move failed. AddToSeqError: {}  Location Checks Error: {}", addToSeqError, performError);
			actSeqSys.ActionSequenceRevertPath(initialActNum);
			return 0;
		}
	}
	return performError == AEC_OK && addToSeqError == AEC_OK;
}

int AiSystem::Flank(AiTactic* aiTac)
{
	auto initialActNum = (*actSeqSys.actSeqCur)->d20ActArrayNum; // used for resetting in case of failure
	auto target = aiTac->target;
	if (!target)
		return 0;
	auto performer = aiTac->performer;

	// if wielding ranged, go melee
	GoMelee(aiTac); // checks internally for weapon etc.

	// preliminary checks
	{
		if (!d20Sys.d20QueryWithData(target, DK_QUE_CanBeFlanked, aiTac->performer))
		{
			logger->info("Target inherently unflankable; next.");
			return 0;
		}
			
		if (combatSys.IsFlankedBy(target, performer))
		{
			logger->info("Already flanking; next.");
			return 0;
		}
		if (actSeqSys.isSimultPerformer(performer) && actSeqSys.simulsAbort(performer))
		{
			actSeqSys.curSeqReset(performer);
			return 1;
		}
	
		// added check for surroundment for performance reasons
		PathQuery pq;
		Path pqr;
		pq.flags = static_cast<PathQueryFlags>(PQF_HAS_CRITTER | PQF_TARGET_OBJ | PQF_ADJUST_RADIUS);
		pq.critter = performer;
		pq.targetObj = target;
		pathfindingSys.PathInit(&pqr, &pq);
		pq.tolRadius += critterSys.GetReach(performer, D20A_UNSPECIFIED_ATTACK);
		if (pathfindingSys.TargetSurrounded(&pqr, &pq))
		{
			logger->info("Target surrounded; next.");
			return 0;
		}
	}

	// cycle through surrounding allies and attempt to move to their opposite location
	objHndl allies[40];
	auto numAllies = combatSys.GetEnemiesCanMelee(target, allies);
	if (numAllies <= 0)
	{
		logger->info("No allies to flank with; next.");
		return 0;
	}
		
	logger->info("Flank preliminary checks passed; {} allies found in melee range", numAllies);

	LocAndOffsets tgtLoc = objects.GetLocationFull(target);
	float tgtAbsX, tgtAbsY, 
		tgtRadius = objects.GetRadius(target), 
		flankDist = objects.GetRadius(performer)+ tgtRadius + 8.0f;

	locSys.GetOverallOffset(tgtLoc, &tgtAbsX, &tgtAbsY);
	for (int i = 0; i < numAllies; i++)
	{
		if (allies[i] == performer)
			continue;

		LocAndOffsets allyLoc = objects.GetLocationFull(allies[i]);
		float allyAbsX, allyAbsY, flankAbsX, flankAbsY, deltaX, deltaY, normalization;
		// get the diametrically opposed location
		locSys.GetOverallOffset(allyLoc, &allyAbsX, &allyAbsY);
		deltaX = allyAbsX - tgtAbsX;
		deltaY = allyAbsY - tgtAbsY;
		normalization = 1.0f / sqrtf(deltaY*deltaY + deltaX*deltaX);
		float xHat = deltaX * normalization, // components of unit vector from target to ally
				yHat = deltaY * normalization;

		flankAbsX = tgtAbsX - xHat * flankDist;
		flankAbsY = tgtAbsY - yHat * flankDist;
		auto flankLoc = LocAndOffsets::FromInches(flankAbsX, flankAbsY);
		if (!pathfindingSys.PathDestIsClear(performer, &flankLoc))
		{
			bool foundFlankLoc = false;
			// try to tweak the angle; the flank check looks for the range of 120 - 240°, so we'll try 135,165,195,225
			float tweakAngles[4] = { -15.0, 15.0, -45.0, 45.0 };
			for (int i = 0; i < 4; i++)
			{
				float tweakAngle = tweakAngles[i] * (float)M_PI / 180.f;
				float xHat2 = xHat * cosf(tweakAngle) + yHat * sinf(tweakAngle),
					yHat2 =  -xHat * sinf(tweakAngle) + yHat * cosf(tweakAngle) ;
				flankAbsX = tgtAbsX - xHat2 * flankDist;
				flankAbsY = tgtAbsY - yHat2 * flankDist;
				flankLoc = LocAndOffsets::FromInches(flankAbsX, flankAbsY);
				if (pathfindingSys.PathDestIsClear(performer, &flankLoc))
				{
					foundFlankLoc = true;
					break;
				}		
			}
			if (!foundFlankLoc)
			{
				if (i == numAllies-1)
					logger->info("No clear flanking position found; next ai tactic.");
				return 0;
			}	
		}
		logger->info("Found flanking position: {} ; attempting move to position.", flankLoc);
		// create a d20 action using that location
		d20Sys.GlobD20ActnSetTypeAndData1(D20A_UNSPECIFIED_MOVE, 0);
		d20Sys.GlobD20ActnSetTarget(objHndl::null, &flankLoc);
		LocAndOffsets pathTgt;
		auto actionCheckResult = actSeqSys.ActionAddToSeq();
		if (actionCheckResult != AEC_OK)
		{
			logger->info("Failed move to position when adding to sequence.");
		}
		else if (actSeqSys.GetPathTargetLocFromCurD20Action(&pathTgt)
			&& combatSys.CanMeleeTargetFromLoc(performer, target, &pathTgt))
		{
			actionCheckResult = actSeqSys.ActionSequenceChecksWithPerformerLocation();
			if (actionCheckResult == AEC_OK)
			{
				return 1;
			}
		} 
		if (actionCheckResult)
			logger->info("Failed move to position due to sequence checks. Error Code: {}", actionCheckResult);
		else
			logger->info("Failed move to position due to CanMeleeTargetAtLoc check! Location was {}", pathTgt);
		actSeqSys.ActionSequenceRevertPath(initialActNum);
	}
	return 0;

	}

int AiSystem::AttackThreatened(AiTactic* aiTac)
{
	if (!aiTac->target || !combatSys.CanMeleeTarget(aiTac->performer, aiTac->target))
		return FALSE;
	return Default(aiTac);
}

void AiSystem::AiProcess(objHndl obj)
{
	return addresses.AiProcess(obj);
}

int AiSystem::AiTimeEventExpires(TimeEvent* evt)
{
	return 1;
}

#pragma endregion

#pragma region AI replacement functions

uint32_t _aiStrategyParse(objHndl objHnd, objHndl target)
{
	return aiSys.AiStrategyParse(objHnd, target);
}

int _AiCoupDeGrace(AiTactic* aiTac)
{
	return aiSys.CoupDeGrace(aiTac);
}

int _AiApproach(AiTactic* aiTac)
{
	return aiSys.Approach(aiTac);
}

int _AiCharge(AiTactic* aiTac)
{
	return aiSys.ChargeAttack(aiTac);
}

int _AiTargetClosest(AiTactic * aiTac)
{
	return aiSys.TargetClosest(aiTac);
}


void _StrategyTabLineParser(TabFileStatus* tabFile, int n, char** strings)
{
	aiSys.StrategyTabLineParser(tabFile, n, strings);
}

int _AiOnInitiativeAdd(objHndl obj)
{
	return aiSys.AiOnInitiativeAdd(obj);
}

int AiSystem::ChooseRandomSpellFromList(AiPacket* aiPkt, AiSpellList* aiSpells){
	if (!aiSpells->spellEnums.size())
		return 0;
	temple::GetRef<objHndl>(0x10AA73C8) = aiPkt->obj;
	temple::GetRef<objHndl>(0x10AA73D0) = aiPkt->target;
	for (int i = 0; i < 5; i++)	{
		auto spellIdx = rngSys.GetInt(0, aiSpells->spellEnums.size() - 1);
		spellSys.spellPacketBodyReset(&aiPkt->spellPktBod);
		unsigned int spellClass;
		unsigned int spellLevels[2];
		d20Sys.ExtractSpellInfo(&aiSpells->spellData[spellIdx],
			&aiPkt->spellPktBod.spellEnum,
			nullptr, &spellClass, spellLevels, nullptr, nullptr);
		auto spellEnum = aiPkt->spellPktBod.spellEnum = aiSpells->spellEnums[spellIdx];
		aiPkt->spellPktBod.caster = aiPkt->obj;
		aiPkt->spellPktBod.spellEnumOriginal = spellEnum;
		aiPkt->spellPktBod.spellKnownSlotLevel = spellLevels[0];
		aiPkt->spellPktBod.spellClass = spellClass;
		spellSys.SpellPacketSetCasterLevel(&aiPkt->spellPktBod);

		SpellEntry spellEntry;
		spellSys.spellRegistryCopy(spellEnum, &spellEntry);
		auto spellRange = spellSys.GetSpellRange(&spellEntry, aiPkt->spellPktBod.casterLevel, aiPkt->spellPktBod.caster);
		aiPkt->spellPktBod.spellRange = spellRange;
		if ( static_cast<UiPickerType>(spellEntry.modeTargetSemiBitmask & 0xFF) == UiPickerType::Area
			&& spellEntry.spellRangeType == SpellRangeType::SRT_Personal)	{
			spellRange = spellEntry.radiusTarget;
		}
		auto tgt = aiPkt->target;
		if (objects.IsCritter(tgt)
			&& d20Sys.d20Query(tgt, DK_QUE_Critter_Is_Grappling) == 1
			|| d20Sys.d20Query(tgt, DK_QUE_Critter_Is_Charmed))	{
			continue;
		}

		if (spellSys.spellCanCast(aiPkt->obj, spellEnum,spellClass, spellLevels[0])){
		
			if (!spellSys.GetSpellTargets(aiPkt->obj,tgt, &aiPkt->spellPktBod, spellEnum))
				continue;
			if (locSys.DistanceToObj(aiPkt->obj, tgt)>spellRange
				&& spellSys.SpellHasAiType(spellEnum, ai_action_offensive)
				|| spellSys.SpellHasAiType(spellEnum, ai_action_defensive)){
				continue;
			}

			aiPkt->aiState2 = 1;
			aiPkt->spellEnum = spellEnum;
			aiPkt->spellData = aiSpells->spellData[spellIdx];
			return 1;
		} 
		else{
			logger->debug("AiCheckSpells(): object {} ({}) cannot cast spell {}", description.getDisplayName( aiPkt->obj), objSystem->GetObject(aiPkt->obj)->id.ToString(),  spellEnum);
		}

	}
	return 0;
}

unsigned int _AiAsplode(AiTactic * aiTac)
{
	return aiSys.Asplode(aiTac);
}

unsigned int _AiWakeFriend(AiTactic * aiTac)
{
	return aiSys.WakeFriend(aiTac);
}

class AiReplacements : public TempleFix
{
public: 
	static int AiDefault(AiTactic* aiTac);
	static int AiAttack(AiTactic* aiTac);
	static int AiGoMelee(AiTactic* aiTac);
	static int AiSniper(AiTactic* aiTac);
	static int AiAttackThreatened(AiTactic* aiTac);
	static int AiTargetThreatened(AiTactic* aiTac);
	static int AiCastParty(AiTactic* aiTac);
	static int AiFlank(AiTactic* aiTac);
	static int AiRage(AiTactic* aiTac);

	static int ChooseRandomSpellUsercallWrapper();
	static void SetCritterStrategy(objHndl obj, const char *stratName);

	void apply() override 
	{
		logger->info("Replacing AI functions...");

		// Will KOS
		replaceFunction<int(__cdecl)(objHndl, objHndl)>(0x1005C920, [](objHndl aiHandle, objHndl triggerer){
			return aiSys.WillKos(aiHandle, triggerer);
		});
		
		replaceFunction(0x100E3270, AiDefault);
		replaceFunction<BOOL(__cdecl)(AiTactic*)>(0x100E4A40, [](AiTactic*aiTac)->BOOL {
			return aiSys.UsePotion(aiTac);
		});
		replaceFunction(0x100E3A00, _AiTargetClosest);
		replaceFunction(0x100E3B60, AiRage);
		replaceFunction(0x100E43F0, AiCastParty);
		replaceFunction(0x100E46C0, AiAttack);
		replaceFunction(0x100E46D0, AiTargetThreatened);
		replaceFunction<BOOL(__cdecl)(AiTactic*)>(0x100E37D0, [](AiTactic*aiTac)->BOOL {
			return aiSys.TargetDamaged(aiTac);
		});
		replaceFunction(0x100E4680, AiAttackThreatened);
		replaceFunction(0x100E48D0, _AiApproach);
		replaceFunction(0x100E4BD0, _AiCharge);		
		replaceFunction(0x100E5080, SetCritterStrategy);
		replaceFunction(0x100E50C0, _aiStrategyParse);
		replaceFunction<uint32_t(__cdecl)(objHndl, objHndl, D20SpellData*, SpellPacketBody*)>(0x100E5290, [](objHndl obj, objHndl target, D20SpellData* spellData, SpellPacketBody* spellPkt)->uint32_t
		{
			return aiSys.AiStrategDefaultCast(obj, target, spellData, spellPkt);
		});
		replaceFunction(0x100E5460, _AiOnInitiativeAdd);
		replaceFunction(0x100E5500, _StrategyTabLineParser);
		replaceFunction(0x100E55A0, AiGoMelee);
		replaceFunction(0x100E58D0, AiSniper);
		replaceFunction(0x100E5950, AiFlank);
		replaceFunction(0x100E5DB0, _AiCoupDeGrace);
		

		replaceFunction(0x1005B810, ChooseRandomSpellUsercallWrapper);


		static int (*orgAiTimeEventExpires)(TimeEvent*) = replaceFunction<int(__cdecl)(TimeEvent*)>(0x1005F090, [](TimeEvent*evt)
		{
			auto result =  orgAiTimeEventExpires(evt);
			return result;
		});

		// AiTimeEventGenerate_MapSectorLoad
		replaceFunction<void(__cdecl)(objHndl, int)>(0x1005BE60, [](objHndl obj, int doFirstHeartbeat){
			auto& cmpAiTimerRefObj = temple::GetRef<objHndl>(0x10AA73C0);
			cmpAiTimerRefObj = obj;

			auto removeTimer = temple::GetRef<void(__cdecl)(TimeEventType, int(*validationCallback)(TimeEvent*))>(0x10060B20);
			removeTimer(TimeEventType::AI, temple::GetRef<int(__cdecl)(TimeEvent*)>(0x100588A0));
			TimeEvent evtNew;
			evtNew.system = TimeEventType::AI;
			evtNew.params[0].handle = obj;
			evtNew.params[1].int32 = doFirstHeartbeat;
			logger->debug("Generating AI TimeEvent for {}, first heartbeat: {}", description.getDisplayName(obj), doFirstHeartbeat);
			if (doFirstHeartbeat)
			{
				int dummy = 1;
			}
			static auto timeeventAddSpecial = temple::GetPointer<BOOL(TimeEvent *createArgs)>(0x10062340);
			timeeventAddSpecial(&evtNew);

		});


		// AI ConsiderTarget
		replaceFunction<BOOL(__cdecl)(objHndl, objHndl)>(0x1005D3F0, [](objHndl obj, objHndl tgt){
			return aiSys.ConsiderTarget(obj, tgt);
		});

		static int(*orgAiFlagsUpdate)(objHndl, AiFightStatus, objHndl, int*) = 
			replaceFunction<int(objHndl, AiFightStatus, objHndl, int*)>(0x1005DA00, [](objHndl obj, AiFightStatus aiFightStatus, objHndl target, int* soundmap )->int
		{

			//if (target && aiFightStatus == AIFS_FIGHTING) {
			//	auto tgtObj = gameSystems->GetObj().GetObject(target);
			//	if (tgtObj->type == obj_t_pc ) {
			//		//logger->debug("AiFlagsUpdate: For {}, triggerer {}", description.getDisplayName(obj), description.getDisplayName(target));
			//		auto scri = gameSystems->GetObj().GetObject(obj)->GetScriptArray(obj_f_scripts_idx)[san_enter_combat];
			//		AiFlag aiFlags = static_cast<AiFlag >(gameSystems->GetObj().GetObject(obj)->GetInt64(obj_f_npc_ai_flags64));
			//		if (!(aiFlags & AiFlag::Fighting ) && scri.scriptId)
			//		{
			//			int dummy = 1;
			//		}
			//	}
			//	else if (tgtObj->type == obj_t_npc)
			//	{
			//		int asd = 0;
			//	}
			//}


			auto result = orgAiFlagsUpdate(obj, aiFightStatus, target, soundmap);
			
			
			return result;

		});

		static int(*orgScriptExecute)(objHndl, objHndl, int, int, SAN, void*) = 
			replaceFunction<int(objHndl, objHndl, int, int, SAN, void*)>(0x10025D60, [](objHndl triggerer, objHndl attachee, int a3, int a4, SAN san, void* a6)->int
		{

			/*if (triggerer && san == san_enter_combat) {
				auto tgtObj = gameSystems->GetObj().GetObject(triggerer);
				if (tgtObj->type == obj_t_pc) {
					logger->debug("ScriptExecute: For {}, triggerer {}", description.getDisplayName(attachee), description.getDisplayName(triggerer));
					auto scri = gameSystems->GetObj().GetObject(attachee)->GetScriptArray(obj_f_scripts_idx)[san_enter_combat];
					if (scri.scriptId)
					{
						int dummy = 1;
					}
				}
			}*/
			if (!triggerer || !attachee){
				int dummy = 1;
				if (!triggerer && !attachee)
					return 0;
			}
			if (san == san_start_combat)
			{
				int dummy = 1;
			}
			auto result = orgScriptExecute(triggerer, attachee, a3, a4, san, a6);

			return result;
		});

	}
} aiReplacements;

int AiReplacements::AiDefault(AiTactic* aiTac)
{
	return aiSys.Default(aiTac);
}

int AiReplacements::AiAttack(AiTactic* aiTac)
{
	return aiSys.Default(aiTac);
}

int AiReplacements::AiTargetThreatened(AiTactic* aiTac)
{
	return aiSys.TargetThreatened(aiTac);
}

int AiReplacements::AiGoMelee(AiTactic* aiTac)
{
	return aiSys.GoMelee(aiTac);
}

int AiReplacements::AiSniper(AiTactic* aiTac)
{
	return aiSys.Sniper(aiTac);
}

int AiReplacements::AiAttackThreatened(AiTactic* aiTac)
{
	return aiSys.AttackThreatened(aiTac);
}

int AiReplacements::AiCastParty(AiTactic* aiTac)
{
	return aiSys.CastParty(aiTac);
}
		
int AiReplacements::AiFlank(AiTactic* aiTac)
{
	return aiSys.Flank(aiTac);
	}

int AiReplacements::AiRage(AiTactic* aiTac)
{
	auto initialActNum = (*actSeqSys.actSeqCur)->d20ActArrayNum; // used for resetting in case of failure
	if (!critterSys.CanBarbarianRage(aiTac->performer))
		return 0;

	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_BARBARIAN_RAGE, 1);
	auto addToSeqError = actSeqSys.ActionAddToSeq();
	if (addToSeqError != AEC_OK)
	{
		auto locCheckError = actSeqSys.ActionSequenceChecksWithPerformerLocation() ;
		if (locCheckError != AEC_OK)
		{
			actSeqSys.ActionSequenceRevertPath(initialActNum);
			return 0;
		}
	}
	return 1;
}

static int _ChooseRandomSpellUsercallWrapper(AiPacket* aiPkt)
{
	return aiSys.ChooseRandomSpell(aiPkt);
}

int __declspec(naked) AiReplacements::ChooseRandomSpellUsercallWrapper()
{
	{ __asm push ecx __asm push esi __asm push ebx __asm push edi}

	__asm
	{
		push ebx;
		call _ChooseRandomSpellUsercallWrapper;
		pop ebx;
	}
	{ __asm pop edi __asm pop ebx __asm pop esi __asm pop ecx }
	__asm retn;
}

void AiReplacements::SetCritterStrategy(objHndl obj, const char* stratName)
{
	auto idx = aiSys.GetStrategyIdx(stratName);
	if (idx == -1)
	{
		idx = aiSys.GetStrategyIdx("default");
	}
	objSystem->GetObject(obj)->SetInt32(obj_f_critter_strategy, idx);
}
#pragma endregion 
AiPacket::AiPacket(objHndl objIn)
{
	target = 0i64;
	obj = objIn;
	aiFightStatus = 0;
	aiState2 = 0;
	spellEnum = 10000;
	skillEnum = (SkillEnum)-1;
	scratchObj = 0i64;
	leader = critterSys.GetLeader(objIn);
	aiSys.GetAiFightStatus(objIn, reinterpret_cast<AiFightStatus*>(&this->aiFightStatus), &this->target);
	field30 = -1;
}