#include "stdafx.h"
#include "common.h"
#include <tig/tig_tabparser.h>

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
#include "gamesystems/gamesystems.h"

#pragma region AI System Implementation
#include "location.h"
#include "critter.h"
#include "weapon.h"
#include "python/python_integration_obj.h"
#include "python/python_object.h"
#include "pybind11/pybind11.h"
#include "pybind11/embed.h"
#include <pybind11/cast.h>
#include <pybind11/stl.h>

#include "util/fixes.h"
#include "party.h"
#include "ui/ui_picker.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/timeevents.h"
#include "python/python_header.h"
#include "condition.h"
#include "rng.h"
#include "turn_based.h"
#include "gamesystems/mapsystem.h"
#include "animgoals/anim.h"
#include "gamesystems/legacysystems.h"
#include "ui/ui_systems.h"
#include "ui/ui_legacysystems.h"
#include "infrastructure/vfs.h"
#include "util/streams.h"
#include "infrastructure/elfhash.h"
#include "mod_support.h"
#include "legacyscriptsystem.h"
#include "maps.h"


namespace py = pybind11;

template <> class py::detail::type_caster<objHndl> {
public:
	bool load(handle src, bool) {
		value = PyObjHndl_AsObjHndl(src.ptr());
		success = true;
		return true;
	}

	static handle cast(const objHndl& src, return_value_policy /* policy */, handle /* parent */) {
		return PyObjHndl_Create(src);
	}

	PYBIND11_TYPE_CASTER(objHndl, _("objHndl"));
protected:
	bool success = false;
};

struct AiSystem aiSys;
AiParamPacket * AiSystem::aiParams;

const auto TestSizeOfAiTactic = sizeof(AiTactic); // should be 2832 (0xB10 )
const auto TestSizeOfAiStrategy = sizeof(AiStrategy); // should be 808 (0x324)
constexpr int AI_PREFAB_STRAT_MAX = 10000;


PYBIND11_EMBEDDED_MODULE(tpai, m) {

	m.doc() = "Temple+ AI module, used for pythonizing the AI.";


	py::class_<AiTacticDef>(m, "AiTacticDef")
		.def("get_name", [](AiTacticDef& self)->std::string {
			return self.name;
			})
		.def("execute", [](AiTacticDef& self, AiTactic& aiTac)->uint32_t {
				return self.aiFunc(&aiTac);
			})
		;
	py::class_<AiTactic>(m, "AiTactic")
		.def_readwrite("performer", &AiTactic::performer)
		.def_readwrite("target", &AiTactic::target)
		.def_readwrite("id", &AiTactic::tacIdx)
		.def_readwrite("spell_pkt", &AiTactic::spellPktBody)
		.def_readwrite("d20_spell_data", &AiTactic::d20SpellData)
		;

	py::class_<AiParamPacket>(m, "AiParams")
		.def("__init__", [](AiParamPacket & self, objHndl handle) {
		self.GetForCritter(handle);}
		)
		.def_readwrite("hostility_threshold", &AiParamPacket::hostilityThreshold)
		;
}

void AiParamPacket::GetForCritter(objHndl handle){
	*this = aiSys.GetAiParams(handle);
}

AiSystem::AiSystem()
{
	combat = &combatSys;
	d20 = &d20Sys;
	actSeq = &actSeqSys;
	spell = &spellSys;
	pathfinding = &pathfindingSys;
	rebase(aiTacticDefs,0x102E4398); 
	rebase(_AiRemoveFromList, 0x1005A070);
	rebase(_FleeAdd, 0x1005DE60);
	rebase(_StopFleeing, 0x1005DEA0);
	rebase(_ShitlistAdd, 0x1005CC10);
	rebase(_StopAttacking, 0x1005E6A0);
	rebase(aiParams, 0x10AA4BD0);
	RegisterNewAiTactics();
}

AiTactic::AiTactic()
{
	memset(this, 0, sizeof(AiTactic));
}


AiStrategy* AiSystem::GetAiStrategy(uint32_t stratId){
	// Check if "normal" strategy ID
	if (stratId < AI_PREFAB_STRAT_MAX ){

		if (stratId >= aiStrategies.size()){
			logger->warn("Strategy ID {} not found!", stratId);
			return &aiStrategies[0];
		}
		
		return &aiStrategies[stratId];
	}
	
	// otherwise - fetch custom strategy
	auto customStrat = aiCustomStrats->get(stratId);
	if (customStrat == nullptr){
		logger->warn("Custom strat ID {} not found!", stratId);
		return &aiStrategies[0];
	}
	return customStrat;
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
	else {
		aiTacOut->field4 = aiStrat->spellsKnown[tacIdx].pad2;
		aiTacOut->field24 = aiStrat->spellsKnown[tacIdx].pad3;
	}
}

uint32_t AiSystem::StrategyParse(objHndl objHnd, objHndl target)
{
	#pragma region preamble
	AiTactic aiTac;
	combat->enterCombat(objHnd);
	
	auto critterStratIdx = (uint32_t)objects.getInt32(objHnd, obj_f_critter_strategy);
	AiStrategy * aiStrat = GetAiStrategy(critterStratIdx);
	
	if (!actSeq->TurnBasedStatusInit(objHnd)) return 0;
	logger->debug("AiStrategy: \t Executing start \"{}\" ({})", aiStrat->name, critterStratIdx);
	actSeq->curSeqReset(objHnd);
	d20->GlobD20ActnInit();
	spell->spellPacketBodyReset(&aiTac.spellPktBody);
	aiTac.performer = objHnd;
	aiTac.target = target;

	#pragma endregion
	
	py::tuple args = py::make_tuple(py::cast(objHnd), py::cast<objHndl>(target));
	auto pyResult = pythonObjIntegration.ExecuteScript("d20_ai.combat_strategy", "execute_strategy", args.ptr());
	if (PyInt_Check(pyResult)) {
		auto result = _PyInt_AsInt(pyResult);
		return result != 0 ? TRUE : FALSE;
	}

	AiCombatRole role = GetRole(objHnd);
	if (role != AiCombatRole::caster)
	{

		// check if disarmed, if so, try to pick up weapon
		if (d20Sys.d20Query(aiTac.performer, DK_QUE_Disarmed))
		{
			logger->info("AiStrategy: \t {} attempting to pickup weapon...", objHnd);
			if (PickUpWeapon(&aiTac))
			{
				actSeq->sequencePerform();
				return TRUE;
			}
		}

		// wake up friends put to sleep; will do this if friend is within reach or otherwise at a 40% chance
		if (WakeFriend(&aiTac))
		{
			actSeq->sequencePerform();
			return TRUE;
		}

	}

	// loop through tactics defined in strategy.tab
	for (uint32_t i = 0; i < aiStrat->numTactics; i++)
	{
		aiTacticGetConfig(i, &aiTac, aiStrat);
		logger->info("AiStrategy: \t {} attempting {}...", objHnd, aiTac.aiTac->name);
		auto aiFunc = aiTac.aiTac->aiFunc;
		if (!aiFunc) continue;
		if (aiFunc(&aiTac)) {
			logger->info("AiStrategy: \t AI tactic succeeded; performing.");
			actSeq->sequencePerform();
			return TRUE;
		}
	}

	// if no tactics defined (e.g. frogs), do target closest first to avoid all kinds of sillyness
	if (aiStrat->numTactics == 0){
		TargetClosest(&aiTac);
	}

	// if none of those work, use default
	aiTac.aiTac = &aiTacticDefs[0];
	aiTac.field4 = 0;
	aiTac.tacIdx = -1;
	logger->info("AiStrategy: \t {} attempting default...", objHnd);
	if (aiTac.target)
		logger->info("Target: {}", aiTac.target);
	assert(aiTac.aiTac != nullptr);
	if (Default(&aiTac))
	{
		actSeq->sequencePerform();
		return TRUE;
	}

	if (!aiTac.target || !combatSys.IsWithinReach(objHnd, aiTac.target)){
		logger->info("AiStrategy: \t Default FAILED. Attempting to find pathable party member as target...");
		objHndl pathablePartyMember = pathfindingSys.CanPathToParty(objHnd);
		if (pathablePartyMember)
		{
			aiTac.target = pathablePartyMember;
			logger->info("New target: {}", pathablePartyMember);
			if (aiTac.aiTac->aiFunc(&aiTac)){
				logger->info("AiStrategy: \t Default tactic succeeded; performing.");
				actSeq->sequencePerform();
				return TRUE;
			}
		}
	}
	
	// if that doesn't work either, try to Break Free (NPC might be held back by Web / Entangle)
	if (d20Sys.d20Query(aiTac.performer, DK_QUE_Is_BreakFree_Possible))
	{
		logger->info("AiStrategy: \t {} attempting to break free...", objHnd);
		if (BreakFree(&aiTac)){
			actSeq->sequencePerform();
			return TRUE;
		}
	}

	// if that's not the issue either:
	if (role == AiCombatRole::sniper){
		if (ImprovePosition(&aiTac)){
			actSeq->sequencePerform();
			return TRUE;
		}
	}
	return FALSE;
}

uint32_t AiSystem::AiStrategDefaultCast(objHndl objHnd, objHndl target, D20SpellData* spellData, SpellPacketBody* spellPkt)
{
	AiTactic aiTac;
	combat->enterCombat(objHnd);
	
	auto critterStratIdx = (uint32_t)objects.getInt32(objHnd, obj_f_critter_strategy);
	AiStrategy * aiStrat = GetAiStrategy(critterStratIdx);

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
		logger->info("AiStrategyDefaultCast: \t {} attempting {}...", objHnd, aiTac.aiTac->name);
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
		logger->info("AiStrategy: \t {} attempting to break free...", objHnd);
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

	aiSys.UpdateAiFlags(npc, AIFS_NONE, objHndl::null, nullptr); // note: this also unsets OCF_COMBAT_MODE within AiPacket::ProcessCombat (if AIFS_NONE, it does DoWaypoints(), which exits combat mode)
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

	for (auto i=0u; i < N; i++){
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
	auto obj = objSystem->GetObject(npc);
	if (!obj->IsNPC())
		return;

	UpdateAiFlags(npc, AIFS_FLEEING, target, nullptr);
}

void AiSystem::StopAttacking(objHndl npc) {
	_StopAttacking(npc);
}

void AiSystem::StopFleeing(objHndl npc) {
	_StopFleeing(npc);
}

void AiSystem::ProvokeHostility(objHndl agitator, objHndl provokedNpc, int rangeType, int flags){

	if (!agitator || !provokedNpc)
		return;

	auto provokedObj = objSystem->GetObject(provokedNpc);
	if (provokedObj->IsCritter()){
		auto critFlags2 = provokedObj->GetInt32(obj_f_critter_flags2);
		if ( (critFlags2 & CritterFlags2::OCF2_NIGH_INVULNERABLE) || critterSys.IsDeadNullDestroyed(provokedNpc))
			return;
	}

	if (provokedObj->GetFlags() & (OF_INVULNERABLE | OF_DONTDRAW | OF_OFF)
		|| provokedObj->GetInt32(obj_f_name) == 6719
		|| agitator == provokedNpc || combatSys.IsBrawlInProgress()){
		return;
	}

	auto agitatorObj = objSystem->GetObject(agitator);

	if (agitatorObj->IsPC() && provokedObj->IsPC())
		return;

	if (!agitatorObj->IsCritter()){ 
		aiSys.UpdateAiFlags(provokedNpc, AIFS_FIGHTING, agitator, nullptr);
		return;
	}

	if (flags & 4){ // never happens AFAIK
		auto agitatorLeader = critterSys.GetLeader(agitator);
		if (agitatorLeader){
			aiSys.TryLockOnTarget(agitator, agitatorLeader, provokedNpc, 1, 0, 1);
		}
		return;
	}

	if (!(flags & 2) && agitator != provokedNpc){
		ObjList objList;
		objList.ListFollowers(agitator);
		for (auto i=0; i < objList.size(); i++){
			auto follower = objList[i];
			if (!follower) continue;
			aiSys.TryLockOnTarget(follower, agitator, provokedNpc, 1, flags & 1, 0);
		}
	}

	if (!provokedObj->IsCritter())
		return;

	auto provokedNpcLeader = critterSys.GetLeaderForNpc(provokedNpc);
	if (!provokedNpcLeader){
		provokedNpcLeader = provokedNpc;
	}
	if (critterSys.IsConcealed(provokedNpcLeader)){
		critterSys.SetConcealedWithFollowers(provokedNpcLeader, 0);
	}

	if (provokedObj->IsNPC()){
		auto npcFlags = provokedObj->GetNPCFlags();
		npcFlags &= (~(ONF_KOS_OVERRIDE));
		provokedObj->SetNPCFlags( npcFlags );
	}

	if (!(flags & 2)){
		if (provokedObj->IsPC()){
			temple::GetRef<void(__cdecl)(objHndl, objHndl)>(0x10057790)(provokedNpc, agitator);
		}

		if (!(flags & 1)){
			aiSys.AlertAllies(provokedNpc, agitator, rangeType);
		}
	}

	if (!provokedObj->IsNPC()){
		return;
	}

	provokedNpcLeader = critterSys.GetLeader(provokedNpc);
	if (!(flags & 1) && agitator!= provokedNpcLeader) // todo should it begetLeaderFprNpc? is in partt?
	{
		provokedObj->SetObjHndl(obj_f_npc_who_hit_me_last, agitator);
	}
	auto aiShitlistAddWithFollowers = temple::GetRef<void(__cdecl)(objHndl, objHndl)>(0x1005CCA0);
	aiShitlistAddWithFollowers(provokedNpc, agitator);
	if (agitatorObj->IsNPC()) {
		aiShitlistAddWithFollowers(agitator, provokedNpc);
	}
	else if (agitatorObj->IsPC()) {

		auto aiPar = aiSys.GetAiParams(provokedNpc);

		if (provokedNpcLeader == agitator) {
			auto npcRefuseFollowingCheck = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x10058A30);
			if (npcRefuseFollowingCheck(provokedNpc, agitator) && critterSys.RemoveFollower(agitator, 0))
			{
				uiSystems->GetParty().Update();
				auto npcFlags = provokedObj->GetInt32(obj_f_npc_flags);
				agitatorObj->SetNPCFlags(npcFlags | ONF_JILTED);
			}
			else if (rngSys.GetInt(1, 3) == 1 && !critterSys.IsDeadOrUnconscious(provokedNpc))
			{
				auto uiDlgSoundPlayer = temple::GetRef<void(__cdecl*)(objHndl, objHndl, char*, int)>(0x10AA73B0);
				if (uiDlgSoundPlayer)
				{
					char ffText[1000]; int soundId;
					auto getFriendlyFireVoiceLine = temple::GetRef<void(__cdecl)(objHndl, objHndl, char*, int*)>(0x10037450);
					getFriendlyFireVoiceLine(provokedNpc, agitator, ffText, &soundId);
					uiDlgSoundPlayer(provokedNpc, agitator, ffText, soundId);
				}
			}
		}
		else if (flags & 1)
		{
			objects.AdjustReaction(provokedNpc, agitator, -10);
		}
		else
		{
			auto curReaction = objects.GetReaction(provokedNpc, agitator);
			if (curReaction > aiPar.hostilityThreshold)
				objects.AdjustReaction(provokedNpc, agitator, aiPar.hostilityThreshold - curReaction);
		}
	}

	if (!(flags & 1) || !critterSys.NpcAllegianceShared(agitator, provokedNpc) && agitator != provokedNpcLeader)
	{
		aiSys.FightStatusProcess(provokedNpc, agitator);
	}

	
	// legacy: temple::GetRef<void(__cdecl)(objHndl, objHndl, int, int)>(0x1005E8D0)(agitator, provokedNpc, rangeType, flags);
}

void AiSystem::TryLockOnTarget(objHndl handle, objHndl leader, objHndl tgt, int isAlways1, int someFlag, int skipAiStatusUpdate){
	if (leader == tgt)
		return;
	auto tgtObj = objSystem->GetObject(tgt);
	if (tgtObj->IsCritter()){
		if (handle == tgt || critterSys.IsDeadNullDestroyed(handle))
			return;

		if (aiSys.CannotHate(handle, tgt, leader)){
			if (isAlways1 && !critterSys.IsCombatModeActive(tgt) && !someFlag){
				auto uiDlgSoundPlayer = temple::GetRef<void*>(0x10AA73B0);
				if (uiDlgSoundPlayer){
					// had a floating "IsConcious" check here...
					objects.AdjustReaction(handle, leader, -5);
				}
			}
			return;
		}

		if (someFlag){
			if (!isAlways1){
				objects.AdjustReaction(handle, leader, -5);
			}
			return;
		}

		if (!skipAiStatusUpdate){
			aiSys.FightStatusProcess(handle, tgt);
			return;
		}
	}
	else{ // trap object can apply here I think
		if (someFlag){
			return;
		}
			
		if (!skipAiStatusUpdate){
			aiSys.UpdateAiFlags(handle, AIFS_FIGHTING, tgt, nullptr);
			return;
		}
	}

	auto lockOnTarget = temple::GetRef<void(__cdecl)(objHndl, objHndl)>(0x1005DF80);
	lockOnTarget(handle, tgt);

}

void AiSystem::TargetLockUnset(objHndl handle){
	if (!handle)
		return;
	auto obj = objSystem->GetObject(handle);
	if (!obj->IsNPC())
		return;
	auto critFlags2 = obj->GetInt32(obj_f_critter_flags2);
	if (critFlags2 & CritterFlags2::OCF2_TARGET_LOCK){
		critFlags2 &= ~OCF2_TARGET_LOCK;
		obj->SetInt32(obj_f_critter_flags2, critFlags2);
	}
}

BOOL AiSystem::RefuseFollowCheck(objHndl handle, objHndl leader){
	auto objBody = objSystem->GetObject(handle);
	if (objBody->GetInt32(obj_f_spell_flags) & SpellFlags::SF_SPELL_FLEE
		&& critterSys.GetLeaderForNpc(handle)
		|| (critterSys.GetLeader(handle), objBody->GetNPCFlags() & ONF_FORCED_FOLLOWER) ){
		return FALSE;	
	}
	auto aiParams = aiSys.GetAiParams(handle);
	auto reactionLvl = critterSys.GetReaction(handle, leader);
	return reactionLvl > aiParams.reactionLvlToRefuseFollowingPc ? 0 : 3;
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
		if (critterSys.NpcAllegianceShared(aiHandle, tgt))
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

	py::tuple args = py::make_tuple(py::cast<objHndl>(obj), py::cast<objHndl>(tgt));
	auto pyResult = pythonObjIntegration.ExecuteScript("d20_ai.targeting", "consider_target", args.ptr());
	if (PyInt_Check(pyResult)) {
		auto result = _PyInt_AsInt(pyResult);
		return result;
	}

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
			//// Make AI party members ignore unconscious critters if nothing else is left...
			//if (party.IsInParty(obj)){
			//	if (combatSys.AllCombatantsFarFromParty()){
			//		return 0;
			//	}
			//}
			auto suitableCrit = aiSys.FindSuitableTarget(obj);
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

	if (locSys.DistanceToObj(obj, tgt) > 125.0){
		return 0;
	}
		
	if (leader){
		int64_t tileDelta = locSys.GetTileDeltaMax(leader, tgt);
		if (tileDelta > 20){ 
			// added so your summons work at range...
			if (locSys.DistanceToObj(obj, tgt) > 15.0f )
				return 0;
		}
	}
	return 1;
}

objHndl AiSystem::GetFriendsCombatFocus(objHndl handle, objHndl friendHandle, objHndl leader){
	auto tgtObj = objSystem->GetObject(friendHandle);
	if (tgtObj->IsNPC()) {
		AiFightStatus aifs;
		objHndl targetsFocus = objHndl::null;
		aiSys.GetAiFightStatus(friendHandle, &aifs, &targetsFocus);

		//// Make AI ignore friend's target if it's unconscious (otherwise putting foes to sleep caused "enter combat" loops when more than 1 AI follower was in party)
		//if (targetsFocus && critterSys.IsDeadOrUnconscious(targetsFocus)){
		//	return objHndl::null;
		//}

		if (aiSys.ConsiderTarget(handle, targetsFocus) && (aifs == AIFS_FIGHTING || aifs == AIFS_FLEEING || aifs == AIFS_SURRENDERED)) {

			auto allegianceStr = aiSys.GetAllegianceStrength(handle, friendHandle);

			if (allegianceStr && !aiSys.CannotHate(handle, targetsFocus, leader)) {
				auto isUnconcealed = !critterSys.IsMovingSilently(targetsFocus)	&& !critterSys.IsConcealed(targetsFocus);

				// check simple LOS/hearing
				if (!aiSys.CannotHear(handle, targetsFocus, isUnconcealed)	|| !critterSys.HasLineOfSight(handle, targetsFocus)) {
					return targetsFocus;
				}
				else {
					// new in Temple+ : check pathfinding short distances (to simulate sensing nearby critters)
					auto pathFlags = PathQueryFlags::PQF_HAS_CRITTER | PQF_IGNORE_CRITTERS
						| PathQueryFlags::PQF_800 | PathQueryFlags::PQF_TARGET_OBJ
						| PathQueryFlags::PQF_ADJUST_RADIUS | PathQueryFlags::PQF_ADJ_RADIUS_REQUIRE_LOS
						| PathQueryFlags::PQF_DONT_USE_PATHNODES | PathQueryFlags::PQF_A_STAR_TIME_CAPPED;

					if (!config.alertAiThroughDoors) {
						pathFlags |= PathQueryFlags::PQF_DOORS_ARE_BLOCKING;
					}

					if (pathfindingSys.CanPathTo(handle, targetsFocus, (PathQueryFlags)pathFlags, 40)) {
						return targetsFocus;
					}
					else if (!party.IsInParty(handle)) {
						auto partyTgt = pathfindingSys.CanPathToParty(handle);
						if (partyTgt) {
							return partyTgt;
						}
					}
				}
			}

		}
	}

	return objHndl::null;
}

objHndl AiSystem::FindSuitableTarget(objHndl handle){
	auto & aiSearchingTgt = temple::GetRef<BOOL>(0x10AA73B4);
	if (aiSearchingTgt)
		return objHndl::null;

	// begin search section
	aiSearchingTgt = 1;

	auto objToTurnTowards = objHndl::null;
	ObjList objList;
	objList.ListRangeTiles(handle, 18, OLC_CRITTERS);

	auto numCritters = objList.size();
	auto critterList = objList.GetListResult();
	std::vector<int64_t> tileDeltas;
	tileDeltas.resize(numCritters);

	auto leader = critterSys.GetLeader(handle);

	// sort by distance?
	if (numCritters > 1){
		for (auto i=0; i < numCritters; i++){
			auto dude = critterList[i];
			auto tileDelta = locSys.GetTileDeltaMax(handle, dude);
			tileDeltas[i] = tileDelta;
			if (critterSys.IsDeadOrUnconscious(dude))
				tileDeltas[i] += 1000;
			if (!party.IsInParty(dude)) { // Temple+: added to prioritize party over other NPCs
				tileDeltas[i] += 100;
			}
		}
		for (auto i=1; i < numCritters; i++){
			auto target = critterList[i];
			auto tileDelta = tileDeltas[i];
			auto j = i;
			for ( ; j > 0; j--){

				if (tileDeltas[j-1] <= tileDelta)
					break;
				tileDeltas[j] = tileDeltas[j - 1];
				critterList[j] = critterList[j - 1];
			}
			tileDeltas[j] = tileDelta;
			critterList[j] = target;
		}
	}

	auto kosCandidate = objHndl::null;
	for (auto i =0; i < numCritters; i++){
		auto target = critterList[i];
		
		// Added 2019-01 because it was silly
		if (target == handle)
			continue;

		int isUnconcealed = !critterSys.IsMovingSilently(target) 
			&& !critterSys.IsConcealed(target);

		if (!aiSys.CannotHear(handle, target, isUnconcealed)
			|| !critterSys.HasLineOfSight(handle, target)){
			
			auto tgtObj = objSystem->GetObject(target);
			if (tgtObj->IsPC() && !isUnconcealed){
				objToTurnTowards = target;
			}
			if (aiSys.WillKos(handle, target)){
				kosCandidate = target;
				break;
			}

			auto friendsCombatFocus = aiSys.GetFriendsCombatFocus(handle, target, leader);
			if (friendsCombatFocus){
				kosCandidate = friendsCombatFocus;
				break;
			}
		}

		target = temple::GetRef<objHndl(__cdecl)(objHndl, objHndl)>(0x1005CB60)(handle, target);
		if (target){
			isUnconcealed = !critterSys.IsMovingSilently(target)
				&& !critterSys.IsConcealed(target);
			if (!aiSys.CannotHear(handle, target, isUnconcealed)
				|| !critterSys.HasLineOfSight(handle, target)){
				kosCandidate = target;
				break;
			}
		}
	}


	if (!kosCandidate){
		if (objToTurnTowards){
			auto rotationTo = objects.GetRotationTowards(handle, objToTurnTowards);
			gameSystems->GetAnim().PushRotate(handle, rotationTo);
		}
	}

	aiSearchingTgt = 0;

	return kosCandidate;

}

int AiSystem::CannotHate(objHndl aiHandle, objHndl triggerer, objHndl aiLeader){
	auto obj = objSystem->GetObject(aiHandle);
	if (obj->GetInt32(obj_f_spell_flags) & SpellFlags::SF_SPELL_FLEE && critterSys.GetLeaderForNpc(aiHandle))
		return 0;
	if (!triggerer || !objSystem->GetObject(triggerer)->IsCritter())
		return 0;
	if (aiLeader && critterSys.GetLeader(triggerer) == aiLeader)
		return 4;
	if (critterSys.NpcAllegianceShared(aiHandle, triggerer))
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
	
	auto isInParty = party.IsInParty(aiHandle);
	if (!leader && !isInParty){
		auto npcFlags = objSystem->GetObject(aiHandle)->GetNPCFlags();
		if (npcFlags & NpcFlag::ONF_KOS && !(npcFlags & NpcFlag::ONF_KOS_OVERRIDE)) {

			if (!critterSys.NpcAllegianceShared(aiHandle, triggerer) && (objSystem->GetObject(triggerer)->IsPC() || !factions.HasNullFaction(triggerer))) {
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
	/*if (isInParty && critterSys.IsDeadOrUnconscious(triggerer)){
		if (combatSys.AllCombatantsFarFromParty()){
			return FALSE;
		}
	}*/
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

void AiSystem::AlertAllies(objHndl handle, objHndl alertFrom, int rangeIdx){

	auto rangeTiles = temple::GetRef<int[]>(0x102BD4D0)[rangeIdx];
	auto tileDelta = locSys.GetTileDeltaMax(alertFrom, handle);

	const int aiTileDeltaMax = 20;

	// alert around the attacked critter (alertFrom object)
	if (tileDelta < 2 * aiTileDeltaMax){
		ObjList objList;
		objList.ListRangeTiles(alertFrom, rangeTiles, OLC_NPC);
		for (auto i=0; i < objList.size(); i++){
			auto resHandle = objList[i];
			if (!resHandle)
				continue;
			if (rangeIdx != 3 || combatSys.HasLineOfAttack(handle, resHandle)){
				aiSys.AlertAlly(resHandle, alertFrom, handle, rangeIdx);
			}
		}
	}

	// alert around the attacker
	if (tileDelta > 1){
		ObjList objList;
		objList.ListRangeTiles(handle, rangeTiles, OLC_NPC);
		for (auto i = 0; i < objList.size(); i++) {
			auto resHandle = objList[i];
			if (!resHandle)
				continue;
			if (rangeIdx != 3 || combatSys.HasLineOfAttack(handle, resHandle)) {
				aiSys.AlertAlly(resHandle, alertFrom, handle, rangeIdx);
			}
		}
	}
}

void AiSystem::AlertAlly(objHndl handle, objHndl alertFrom, objHndl alertDispatcher, int rangeIdx){
	if (handle == alertDispatcher || handle == alertFrom)
		return;
	auto resObj = gameSystems->GetObj().GetObject(handle);
	if (resObj->GetFlags() & (OF_OFF | OF_DESTROYED | OF_DONTDRAW))
		return;

	if (aiSys.GetAllegianceStrength(handle, alertDispatcher)){
		if (!critterSys.HasLineOfSight(handle, alertDispatcher)
			|| !aiSys.CannotHear(handle, alertDispatcher, rangeIdx) || rangeIdx == 3){
			aiSys.FightStatusProcess(handle, alertFrom);
		}
		return;
	}

	if (aiSys.GetAllegianceStrength(handle, alertFrom)){
		if (!critterSys.HasLineOfSight(handle, alertFrom)
			|| !aiSys.CannotHear(handle, alertFrom, rangeIdx)) {
			aiSys.FightStatusProcess(handle, alertDispatcher);
		}
		return;
	}
	// the code below caused problems when attacking Mickey in the Tavern (made everyone go hostile on you)
	/*
	 if ( !( objSystem->GetObject(handle)->GetInt32(obj_f_critter_flags) & OCF_NO_FLEE)){
		AiFightStatus aifs;
		aiSys.GetAiFightStatus(handle, &aifs, nullptr);
		if (aifs == AIFS_NONE){
			if (!critterSys.HasLineOfSight(handle, alertDispatcher)
				|| !aiSys.CannotHear(handle, alertDispatcher, rangeIdx)){
				aiSys.UpdateAiFlags(handle, AIFS_FLEEING, alertFrom, nullptr);
			}
		}
	}
	 */
	
}

void AiSystem::AlertAllies2(objHndl handle, objHndl alertFrom)
{
	ObjList objList;
	const int ALLY_ALERTING_DISTANCE = 24;
	// test cases:
	// moathouse frogs - 12 is enough to pull them all in
	objList.ListRangeTiles(handle, ALLY_ALERTING_DISTANCE, OLC_CRITTERS);
	for (auto i = 0; i < objList.size(); i++) {
		auto resHandle = objList[i];
		if (!resHandle)
			break;
		if (resHandle == handle)
			continue;


		auto resObj = gameSystems->GetObj().GetObject(resHandle);
		if (resObj->GetFlags() & (OF_OFF | OF_DESTROYED | OF_DONTDRAW))
			continue;
		if (critterSys.IsDeadOrUnconscious(resHandle)) {
			continue;
		}

		if (party.IsInParty(resHandle))
			continue;

		if (tbSys.IsInInitiativeList(resHandle) || critterSys.IsCombatModeActive(resHandle))
			continue;

		if (!combatSys.HasLineOfAttack(resHandle, handle)) {


			if (locSys.DistanceToObj(handle, resHandle) > 30) {
				continue;
			}
			// check pathfinding short distances
			auto pathFlags = PathQueryFlags::PQF_HAS_CRITTER | PQF_IGNORE_CRITTERS
				| PathQueryFlags::PQF_800 | PathQueryFlags::PQF_TARGET_OBJ
				| PathQueryFlags::PQF_ADJUST_RADIUS | PathQueryFlags::PQF_ADJ_RADIUS_REQUIRE_LOS
				| PathQueryFlags::PQF_DONT_USE_PATHNODES | PathQueryFlags::PQF_A_STAR_TIME_CAPPED;

			if (!config.alertAiThroughDoors) {
				pathFlags |= PathQueryFlags::PQF_DOORS_ARE_BLOCKING;
			}

			if (!pathfindingSys.CanPathTo(handle, resHandle, (PathQueryFlags)pathFlags, 40)) {
				//logger->debug("Failed to alert {} because of PF distance", resHandle);
				continue;
			}
		}

		if (aiSys.GetAllegianceStrength(resHandle, handle)) { // check that they have a faction in common
			aiSys.ProvokeHostility(alertFrom, resHandle, 3, 0);
			continue;
		}

		if (factions.HasNullFaction(resHandle) && factions.HasNullFaction(handle)) {
			if (aiSys.WillKos(resHandle, alertFrom)) {
				aiSys.ProvokeHostility(alertFrom, resHandle, 3, 0);
			}
		}

	}

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
	if (critterSys.IsDeadNullDestroyed(obj)){
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

	if (! ( objects.GetFlags(obj) & (OF_OFF | OF_DESTROYED) )
		 && combatSys.isCombatActive() && !critterSys.IsDeadNullDestroyed(obj)){
		combatSys.AddToInitiative(obj);
	}
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
	int performerIsIntelligent = objects.StatLevelGet(performer, stat_intelligence) >= 3;
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

	logger->debug("{} targeting closest...", performer);

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
			//logger->debug("Checking line of attack for target: {}", combatant);
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
	logger->info("{} targeted.", aiTac->target);

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

BOOL AiSystem::TargetFriendHurt(AiTactic * aiTac)
{
	auto performer = aiTac->performer;
	auto lowest = 70;
	auto N = combatSys.GetInitiativeListLength();
	aiTac->target = 0;

	for (auto i = 0; i < N; i++) {
		auto combatant = combatSys.GetInitiativeListMember(i);
		if (!combatant || !critterSys.IsFriendly(performer, combatant))
			continue;
		
		auto hpPct = critterSys.GetHpPercent(combatant); 
		
		if (hpPct < lowest) {
			lowest = hpPct;
			aiTac->target = combatant;
		}

	}

	return FALSE;
}

int AiSystem::TargetThreatened(AiTactic* aiTac)
{
	objHndl performer = aiTac->performer;
	int performerIsIntelligent = objects.StatLevelGet(performer, stat_intelligence) >= 3;
//	objHndl target;
	LocAndOffsets performerLoc;
	float dist = 1000000000.0;


	locSys.getLocAndOff(aiTac->performer, &performerLoc);

	logger->info("{} targeting threatened...", aiTac->performer);

	ObjList objlist;
	objlist.ListVicinity(performerLoc.location, OLC_CRITTERS);

	auto args = PyTuple_New(2);
	PyTuple_SET_ITEM(args, 0, PyObjHndl_Create(performer));
	
	objHndl ignoredTarget = objHndl::null;
	float ignoredDist = 1000000000.0;
	for (int i = 0; i < objlist.size(); i++)
	{
		objHndl dude = objlist.get(i);
		PyTuple_SET_ITEM(args, 1, PyObjHndl_Create(dude));

		auto result = pythonObjIntegration.ExecuteScript("combat", "ShouldIgnoreTarget", args);
		int ignoreTarget = PyInt_AsLong(result);
		Py_DECREF(result);
		
		if (!critterSys.IsFriendly(dude, performer)
			&& !critterSys.IsDeadNullDestroyed(dude)
			&& combatSys.IsWithinReach(performer, dude) 
			)
		{
			if (!ignoreTarget && locSys.DistanceToObj(performer, dude)  < dist){
				aiTac->target = dude;
				dist = locSys.DistanceToObj(performer, dude);
			}
			else if (ignoreTarget && locSys.DistanceToObj(performer, dude)  < ignoredDist){
				ignoredTarget = dude;
				ignoredDist = locSys.DistanceToObj(performer, dude);
			}
		}
	}
	Py_DECREF(args);
	if (dist > 900000000.0)
	{
		// check if already moved - if so, use the threatened target (todo: regard spellcasting)
		auto curSeq = *actSeqSys.actSeqCur;
		if ( (curSeq->tbStatus.tbsFlags & (TBSF_Movement | TBSF_Movement2)) == (TBSF_Movement | TBSF_Movement2)){
			if (ignoredTarget){
				aiTac->target = ignoredTarget;
				logger->info("{} targeted because there was no other legit target and am out of moves.", aiTac->target);
				return FALSE;
			}
		}
		aiTac->target = 0; //should it override it?
		//hooked_print_debug_message(" no target found. Attempting Target Closest instead.");
		logger->info("no target found. ");
		//TargetClosest(aiTac);
	} 
	else
	{
		logger->info("{} targeted.", aiTac->target);
	}
	

	return FALSE;
}
BOOL AiSystem::UsePotion(AiTactic* aiTac) {

	auto performer = aiTac->performer;
	auto hpCur = objects.StatLevelGet(performer, stat_hp_current);
	auto hpMax = objects.StatLevelGet(performer, stat_hp_max);
	auto hpPct = hpCur * 1.0 / hpMax;

	auto objInventory = inventory.GetInventory(performer);
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


BOOL AiSystem::UseItem(AiTactic* aiTac) {

	objHndl item = objHndl::null;
	if (aiTac->field24 && aiTac->field4) {
		uint64_t h1 = (uint64_t)aiTac->field24 << 32 | aiTac->field4;
		item = h1;
		if (!item || !objSystem->IsValidHandle(item))
			item = objHndl::null;
	}
	auto result = combatSys.UseItem(aiTac->performer, item, aiTac->target);
	if (!result)
		return TRUE;
	return FALSE;
}

int AiSystem::Approach(AiTactic* aiTac)
{
	int initialActNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	if (!aiTac->target)
		return 0;
	if (combatSys.IsWithinReach(aiTac->performer, aiTac->target))
		return 0;

	// check if 5' step is a good choice
	auto isWorth = Is5FootStepWorth(aiTac);

	if (isWorth) {
		d20Sys.GlobD20ActnSetTypeAndData1(D20A_5FOOTSTEP, 0);
		d20Sys.GlobD20ActnSetTarget(aiTac->target, nullptr);
		if (actSeqSys.ActionAddToSeq() == AEC_OK
			&& !actSeqSys.ActionSequenceChecksWithPerformerLocation())
			return TRUE;
		actSeqSys.ActionSequenceRevertPath(initialActNum);
	}

	actSeqSys.curSeqReset(aiTac->performer);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_UNSPECIFIED_MOVE, 0);
	d20Sys.GlobD20ActnSetTarget(aiTac->target, 0);
	actSeqSys.ActionAddToSeq();
	if (actSeqSys.ActionSequenceChecksWithPerformerLocation() != AEC_OK){
		actSeqSys.ActionSequenceRevertPath(initialActNum);
		return FALSE;
	}
	return TRUE;
}

int AiSystem::ApproachSingle(AiTactic* aiTac)
{
	int initialActNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	if (!aiTac->target)
		return 0;
	if (combatSys.IsWithinReach(aiTac->performer, aiTac->target))
		return 0;

	// check if 5' step is a good choice
	auto isWorth = Is5FootStepWorth(aiTac);

	if (isWorth) {
		d20Sys.GlobD20ActnSetTypeAndData1(D20A_5FOOTSTEP, 0);
		d20Sys.GlobD20ActnSetTarget(aiTac->target, nullptr);
		if (actSeqSys.ActionAddToSeq() == AEC_OK
			&& !actSeqSys.ActionSequenceChecksWithPerformerLocation())
			return TRUE;
		actSeqSys.ActionSequenceRevertPath(initialActNum);
	}

	actSeqSys.curSeqReset(aiTac->performer);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_MOVE, 0);
	d20Sys.GlobD20ActnSetTarget(aiTac->target, 0);
	actSeqSys.ActionAddToSeq();
	if (actSeqSys.ActionSequenceChecksWithPerformerLocation() != AEC_OK) {
		actSeqSys.ActionSequenceRevertPath(initialActNum);
		return FALSE;
	}
	return TRUE;
}

int AiSystem::CastParty(AiTactic* aiTac)
{
	auto initialActNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	if (!aiTac->target){
		return FALSE;
	}
	
	int castDefensively = 0;
	auto enemiesCanMelee = combatSys.GetEnemiesCanMelee(aiTac->performer);
	if (enemiesCanMelee.size() > 0)
		castDefensively = 1;
	d20Sys.d20SendSignal(aiTac->performer, DK_SIG_SetCastDefensively, castDefensively, 0);
	LocAndOffsets targetLoc =	objects.GetLocationFull(aiTac->target);
	
	auto partyLen = party.GroupListGetLen();
	for (auto i = 0u; i < partyLen; i++){
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
		return FALSE;
	}
	return TRUE;
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

BOOL AiSystem::ImprovePosition(AiTactic* aiTac){

	if (!aiTac->target)
		return FALSE;

	auto performer = aiTac->performer;
	auto tgt = aiTac->target;

	auto hasLineOfAttack = combatSys.HasLineOfAttack(performer, tgt);
	if (hasLineOfAttack)
		return FALSE;

	// need to get a map of LOS to critter
	// basically a fog of war map for an individual...
	// use this map for pathfinding (rather than making a LOS check for every A* step...)
	// 
	auto curSeq = *actSeqSys.actSeqCur;
	actSeqSys.curSeqReset(aiTac->performer);
	auto initialActNum = curSeq->d20ActArrayNum;

	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_UNSPECIFIED_MOVE, /*required range: */ 30);
	d20Sys.GlobD20ActnSetTarget(aiTac->target, 0);
	auto addToSeqError = (ActionErrorCode)actSeqSys.ActionAddToSeq();

	auto curNum = curSeq->d20ActArrayNum;

	if (addToSeqError == AEC_OK ){

		// Check if AoO's were added - if so cut the movement before reaching those
		for (auto i = initialActNum; i < curNum; i++) {
			auto &d20a = curSeq->d20ActArray[i];
			if (d20a.d20ActType == D20A_AOO_MOVEMENT || d20a.d20ActType == D20A_READIED_INTERRUPT) {
				curNum = i;
				actSeqSys.ActionSequenceRevertPath(i);
				break;
			}
		}

		// removed this part - this is now accomplished by the PF system itself
		if (false)
		if (curNum > 0) {


			if (curSeq->d20ActArrayNum != curNum){
				
			}
			auto path = curSeq->d20ActArray[curSeq->d20ActArrayNum - 1].path;

			if (!path){
				logger->info("ImprovePosition: Unspecified Move failed. ");
				actSeqSys.ActionSequenceRevertPath(initialActNum);
				return FALSE;
			}

			auto lowerBound = 0.0f;
			auto upperBound = path->GetPathResultLength();

			auto truncationDistance = upperBound;
			auto newDest = path->to;
			hasLineOfAttack = combatSys.HasLineOfAttackFromPosition(newDest, tgt);

			if (hasLineOfAttack) {
				while (upperBound > lowerBound + 2.0f) {
					truncationDistance = (upperBound + lowerBound) / 2;
					pathfindingSys.TruncatePathToDistance(path, &newDest, truncationDistance);
					hasLineOfAttack = combatSys.HasLineOfAttackFromPosition(newDest, tgt);
				
					if (hasLineOfAttack) {
						upperBound = truncationDistance;
					}
					else {
						lowerBound = truncationDistance;
					}
				}
				/*auto destClear = pathfindingSys.PathDestIsClear(performer, &newDest);
				while (!destClear && upperBound > 2.0f) {
					upperBound -= 2.0f;
					pathfindingSys.TruncatePathToDistance(path, &newDest, upperBound);
					destClear = pathfindingSys.PathDestIsClear(performer, &newDest);
				}
				
				if (upperBound <= 0.0f) {
					logger->info("ImprovePosition: Unspecified Move failed. ");
					actSeqSys.ActionSequenceRevertPath(initialActNum);
					return FALSE;
				}*/
				

				auto truncPath = pathfindingSys.FetchAvailablePQRCacheSlot();
				if (pathfindingSys.GetPartialPath(path, truncPath, 0, upperBound)) {
					logger->info("ImprovePosition: truncated path to length {} ft", upperBound);
					path->occupiedFlag &= ~1;
					curSeq->d20ActArray[curSeq->d20ActArrayNum - 1].path = truncPath;
					curSeq->d20ActArray[curSeq->d20ActArrayNum - 1].destLoc = truncPath->to;
				}
				else {
					truncPath->occupiedFlag &= ~1;
				}

			}

		}

	}
		
	

	auto performError = actSeqSys.ActionSequenceChecksWithPerformerLocation();

	if (addToSeqError != AEC_OK || performError != AEC_OK )
	{
		logger->info("ImprovePosition: Unspecified Move failed. AddToSeqError: {}  Location Checks Error: {}", addToSeqError, performError);
		actSeqSys.ActionSequenceRevertPath(initialActNum);
		return FALSE;
	}
	if (curSeq->d20ActArray[curSeq->d20ActArrayNum - 1].path){
		logger->info("{} improving position to {} ({} to {})", performer, tgt, curSeq->d20ActArray[curSeq->d20ActArrayNum - 1].path->from, curSeq->d20ActArray[curSeq->d20ActArrayNum - 1].path->to);
	}
	return TRUE;
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

/* 0x1005DA00 */
int AiSystem::UpdateAiFlags(objHndl handle, AiFightStatus aiFightStatus, objHndl target, int* soundMap){
	//logger->trace("{} entering ai state: {}, target: {}", handle, (int)aiFightStatus, target);
	auto obj = objSystem->GetObject(handle);
	if (aiFightStatus == AIFS_NONE)
	{
		auto asdf = 1;
	}
	auto critterFlags = obj->GetInt32(obj_f_critter_flags);
	auto critterFlags2 = obj->GetInt32(obj_f_critter_flags2);
	auto npcFlags = obj->GetNPCFlags();
	auto aiFlags = obj->GetInt64(obj_f_npc_ai_flags64);

	auto cheatsAiNpcIsFightingAllowed = temple::GetRef<BOOL>(0x102BD4E0);


	// handle Fleeing / Surrendered first because they may be converted to Fighting
	if (aiFightStatus == AIFS_FLEEING || aiFightStatus == AIFS_SURRENDERED){

		if (aiFightStatus == AIFS_FLEEING
			&& critterFlags & CritterFlag::OCF_NO_FLEE 
			&& !d20Sys.d20Query(handle, DK_QUE_Critter_Is_Afraid)){
			aiFightStatus = AIFS_FIGHTING;
		}
		else{
			auto dialogPlayer = temple::GetRef<void(__cdecl*)(objHndl, objHndl, char*, int)>(0x10AA73B0);
			if (dialogPlayer && !(critterFlags & (OCF_SURRENDERED | OCF_FLEEING))
				&& !critterSys.IsDeadOrUnconscious(handle)){
				auto getFleeVoiceLine = temple::GetRef<void(__cdecl)(objHndl, objHndl, char*, int*)>(0x100374E0);
				char fleeText[1000];
				int soundId;
				getFleeVoiceLine(handle, target, fleeText, &soundId);
				dialogPlayer(handle, target, fleeText, soundId);
			}
			critterFlags &= ~(OCF_SURRENDERED | OCF_FLEEING);
			if (aiFightStatus == AIFS_FLEEING)
				critterFlags |= OCF_FLEEING;
			else{
				critterFlags |= OCF_SURRENDERED;
				combatSys.CritterExitCombatMode(handle);
			}
			obj->SetObjHndl(obj_f_critter_fleeing_from, target);
		}
	}

	if (aiFightStatus == AIFS_FIGHTING){
		if (!cheatsAiNpcIsFightingAllowed || (npcFlags & ONF_NO_ATTACK))
			aiFightStatus = AIFS_NONE;
	}

	if (aiFightStatus != AIFS_SURRENDERED && aiFightStatus != AIFS_FLEEING){
		critterFlags &= ~(OCF_SURRENDERED | OCF_FLEEING);
		aiFlags &= ~(AiFlag::HasSpokenFlee);
	}

	// update the flags
	obj->SetInt32(obj_f_critter_flags, critterFlags);
	obj->SetInt64(obj_f_npc_ai_flags64, aiFlags);

	if (aiFightStatus != AIFS_FIGHTING){
		if (aiFightStatus == AIFS_FINDING_HELP && !(aiFlags & AiFlag::FindingHelp)){
			obj->SetObjHndl(obj_f_npc_combat_focus, target);
			aiFlags |= AiFlag::FindingHelp;
			obj->SetInt64(obj_f_npc_ai_flags64, aiFlags);
		}
		if (npcFlags & ONF_BACKING_OFF ){
			npcFlags &= ~(ONF_BACKING_OFF);
			obj->SetInt32(obj_f_npc_flags, npcFlags);
		}
		if (aiFlags & AiFlag::Fighting){
			aiFlags &= ~(AiFlag::Fighting);
			obj->SetInt64(obj_f_npc_ai_flags64, aiFlags);
			if (aiFightStatus != AIFS_FLEEING) {
				obj->SetInt32(obj_f_npc_flags, npcFlags | ONF_DEMAINTAIN_SPELLS);
			}
			pythonObjIntegration.ExecuteObjectScript(target, handle, ObjScriptEvent::ExitCombat);
		}
		return aiFightStatus;
	}
	
	// AIFS_FIGHTING
	if (critterFlags2 & OCF2_TARGET_LOCK){
		return AIFS_FIGHTING;
	}
	if (!(aiFlags & AiFlag::Fighting)){
		aiFlags |= AiFlag::CheckWield | AiFlag::CheckGrenade | AiFlag::Fighting;
		obj->SetInt64(obj_f_npc_ai_flags64, aiFlags);
		pythonObjIntegration.ExecuteObjectScript(target, handle, ObjScriptEvent::EnterCombat);
		if (obj->GetFlags() & OF_OFF)
			return AIFS_NONE;
		
		/* 
		 * Adding because of changes to shouldRemove
		 */
		if (objects.IsCritter(target)) { // in case it's a trap
			combatSys.enterCombat(handle, false);
		}
		/*
		 *
		 */

		if (soundMap){
			*soundMap = critterSys.SoundmapCritter(handle, 5);
		}
	}
	if (obj->IsPC()){ // set reaction level to hostility level
		auto reactionLvl = critterSys.GetReaction(handle, target);
		auto aiParams = GetAiParams(handle);
		if (reactionLvl > aiParams.hostilityThreshold){
			objects.AdjustReaction(handle, target, aiParams.hostilityThreshold - reactionLvl);
		}
	}
	obj->SetObjHndl(obj_f_npc_combat_focus, target);

	static bool isAlertingAllies = false;
	if (!isAlertingAllies){
		isAlertingAllies = true;
		aiSys.AlertAllies(target, handle, 1);
		isAlertingAllies = false;
	}

	return aiFightStatus;
}

void AiSystem::StrategyTabLineParseTactic(AiStrategy* aiStrat, const char* tacName, const char* middleString, const char* spellString)
{ // this functions matches the tactic strings (3 strings) to a tactic def
	if (!*tacName){
		return;
	}

	int tacIdx = 0;

	// first check the vanilla tactic defs (44 types)
	constexpr int VANILLA_NUM_TACTICS = 44;
	for (int i = 0; i < VANILLA_NUM_TACTICS && _stricmp(tacName, aiTacticDefs[i].name); i++){
		++tacIdx;
	}

	if (tacIdx < VANILLA_NUM_TACTICS) {
		aiStrat->aiTacDefs[aiStrat->numTactics] = &aiTacticDefs[tacIdx];
		aiStrat->field54[aiStrat->numTactics] = 0;
		aiStrat->spellsKnown[aiStrat->numTactics].spellEnum = -1;
		if (*spellString)
			spell->ParseSpellSpecString(&aiStrat->spellsKnown[aiStrat->numTactics], (char *)spellString);
		++aiStrat->numTactics;
		return;
	}

	// if none matched, try the new Temple+ tactics
	tacIdx = 0;
	for (int i = 0; i < 100 && aiTacticDefsNew[i].name && _stricmp(tacName, aiTacticDefsNew[i].name); i++)
	{
		tacIdx++;
	}
	if (aiTacticDefsNew[tacIdx].name && tacIdx < 100){
		aiStrat->aiTacDefs[aiStrat->numTactics] = &aiTacticDefsNew[tacIdx];
		aiStrat->field54[aiStrat->numTactics] = 0;
		aiStrat->spellsKnown[aiStrat->numTactics].spellEnum = -1;
		if (*spellString)
			spell->ParseSpellSpecString(&aiStrat->spellsKnown[aiStrat->numTactics], (char*)spellString);
		else
			StrategyTabLineParseTacticMiddleString(aiStrat, aiStrat->numTactics, middleString);
		++aiStrat->numTactics;
		return;
	}
	logger->warn("Error: No Such Tactic {} for Strategy {}", tacName, aiStrat->name);
	
	return;
}

void AiSystem::StrategyTabLineParseTacticMiddleString(AiStrategy* aiStrat, int idx, const char* middleString)
{
	if (*middleString) {
		/**
			middleString can have next values:
			- string, like "G_BAE066DE_A0A2_4078_8AF0_F0A26A5434FA", e.g. id.ToString(), which will look up for objHndl and save to arg0, arg1
			- uint64, e.g. locXY
			- uint32 uint32, e.g. locx locy

			Parsing result will assign two uin32: SpellStoreData.pad2 and SpellStoreData.pad3 to be treated as arg0 and arg1.

			SpellStoreData.pad2 will hold arg0, which will be copied to AiTactic.field4 and used in tactic func like AiTargetObj
			SpellStoreData.pad3 will hold arg1, which will be copied to AiTactic.field24 and used in tactic func like AiTargetObj

			usage of arg0, arg1:
			- locXY (locx, locy)
			- handle (lower, upper)
		*/
		uint64_t val = _atoi64(middleString);
		if (val) {
			aiStrat->spellsKnown[idx].pad2 = (uint32_t)val;
			auto secondStr = strchr(middleString, ' ');
			if (secondStr) {
				aiStrat->spellsKnown[idx].pad3 = atoi(secondStr);
			}
			else {
				aiStrat->spellsKnown[idx].pad3 = (uint32_t)(val >> 32);
			}
		}
		else 
		if (objSystem) {
			objHndl found = objSystem->FindObjectByIdStr(format("{}", middleString));
			if (found) {
				aiStrat->spellsKnown[idx].pad2 = found.GetHandleLower();
				aiStrat->spellsKnown[idx].pad3 = found.GetHandleUpper();
			}
		}
	}
}

void AiSystem::ParseStrategyLine(AiStrategy & newStrategy, const std::vector< string>& strings)
{
	newStrategy.numTactics = 0;
	if (!strings.size())
		return;
	if (strings[0].size()){
		newStrategy.name = fmt::format("{}", strings[0]);
	}
	
	for (auto i=1u; i+2 < strings.size(); i+=3)
	{
		StrategyTabLineParseTactic(&newStrategy, strings[i].c_str(), strings[i+1].c_str(), strings[i+2].c_str());
	} 
}

int AiSystem::StrategyTabLineParser(const TigTabParser* tabFile, int n, char** strings)
{
	int numCols; 
	char **str; 
	
	AiStrategy newStrategy;
	
	newStrategy.name = fmt::format("{}", *strings);
	newStrategy.numTactics = 0;
	numCols = 3;
	str = strings + 1;
	auto i = 0;
	do
	{
		if (numCols > tabFile->maxColumns )
			break;

		StrategyTabLineParseTactic(&newStrategy, str[0], str[1], str[2]);
		numCols += 3;
		str += 3;
		++i;
	} while (i < 20);
	aiStrategies.push_back(newStrategy);
	return 0;
}

void AiSystem::InitCustomStrategies(){
	
	{
		AiStrategy pcCaster;
		pcCaster.numTactics = 0;
		pcCaster.name = fmt::format("Charmed PC Caster");
		StrategyTabLineParseTactic(&pcCaster, "target closest","","");
		StrategyTabLineParseTactic(&pcCaster, "defaultCast", "", "");

		aiStrategies.push_back(pcCaster);
	}

	{
		AiStrategy pcFighter;
		pcFighter.numTactics = 0;
		pcFighter.name = fmt::format("Charmed PC Fighter");
		StrategyTabLineParseTactic(&pcFighter, "target closest", "", "");
		aiStrategies.push_back(pcFighter);
	}

	{
		aiCustomStrats = new IdxTableWrapper<AiStrategy>(&mAiStrategiesCustom);
		aiCustomStrats->newTable(sizeof AiStrategy, __FILE__, __LINE__);
	}
}

void AiSystem::SetCustomStrategy(objHndl handle, const std::vector<std::string>& stringVector, int save)
{
	AiStrategy aiStrat;
	ParseStrategyLine(aiStrat, stringVector);
	auto id = ElfHash::Hash(aiStrat.name);
	if (id >= 0 && id < AI_PREFAB_STRAT_MAX){
		logger->error("bad strategy id!"); 
		return;
	}

	aiCustomStrats->put(id, aiStrat);
	if (save)
		mAiStrategiesCustomSrc.push_back(stringVector);
	objSystem->GetObject(handle)->SetInt32(obj_f_critter_strategy,id);
}

bool AiSystem::CustomStrategiesSave()
{
	VfsOutputStream tios("Save\\Current\\custom_strats.bin");
	
	for (auto &strat : mAiStrategiesCustomSrc) {
		
		for (auto it: strat){
			tios.WriteStringPrefixed(it);
		}
		tios.WriteStringPrefixed("\n");
	}

	return true;
}

bool AiSystem::CustomStrategiesLoad()
{
	std::string stratPath = { "Save\\Current\\custom_strats.bin" };
	if (!vfs->FileExists(stratPath)) // support old saves without custom strats
		return true;

	auto data = vfs->ReadAsBinary(stratPath);
	MemoryInputStream tios2(data);

	auto pos = 0u;
	while (pos < data.size()){
		std::vector<std::string> stringVector;
		
		
		while (true){
			std::string s = tios2.ReadStringPrefixed();
			if (s.size() && !strcmp(s.c_str(), "\n")){
				break;
			}
			stringVector.push_back(s);
		}
		
		AiStrategy aiStrat;
		ParseStrategyLine(aiStrat, stringVector);
		auto id = ElfHash::Hash(aiStrat.name);
		aiCustomStrats->put(id, aiStrat);
		mAiStrategiesCustomSrc.push_back(stringVector);
		pos = tios2.GetPos();
	}
	return true;
}

int AiSystem::AiOnInitiativeAdd(objHndl obj)
{
	if (party.IsInParty(obj) && objects.IsPlayerControlled(obj)){
		return 0;
	}

	auto critterStratIdx = (uint32_t)objects.getInt32(obj, obj_f_critter_strategy);
	AiStrategy * aiStrat = GetAiStrategy(critterStratIdx);  

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
	if (critterSys.IsWieldingRangedWeapon(obj))
		return AiCombatRole::sniper;
	return AiCombatRole::general;
}

BOOL AiSystem::AiFiveFootStepAttempt(AiTactic* aiTac)
{
	objHndl threateners[40];
	auto actNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	auto numThreateners = combatSys.GetEnemiesCanMelee(aiTac->performer, threateners);
	if (!numThreateners)
		return TRUE;

	// check if those threateners are ignorable
	auto shouldIgnoreThreateners = true;
	auto args = PyTuple_New(2);
	PyTuple_SET_ITEM(args, 0, PyObjHndl_Create(aiTac->performer));
	for (auto i=0; i < numThreateners; i++){
		PyTuple_SET_ITEM(args, 1, PyObjHndl_Create(threateners[i]));

		auto result = pythonObjIntegration.ExecuteScript("combat", "ShouldIgnoreTarget", args);
		int ignoreTarget = PyInt_AsLong(result);
		Py_DECREF(result);

		if (!ignoreTarget){
			shouldIgnoreThreateners = false;
			break;
		}
	}

	if (shouldIgnoreThreateners)
		return TRUE;

	// got a reason to be afraid!
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
				return TRUE;
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

	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiGoto;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "goto");

	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiHalt;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "halt");

	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiStop;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "stop");

	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiTargetObj;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "target obj");

	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiTotalDefence;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "total defence");

	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiPythonAction;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "python action");

	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiD20Action;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "d20 action");
	
	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiUseItem;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "use item");

	n++;
	aiTacticDefsNew[n].name = new char[100];
	aiTacticDefsNew[n].aiFunc = _AiApproachSingle;
	memset(aiTacticDefsNew[n].name, 0, 100);
	sprintf(aiTacticDefsNew[n].name, "approach single");
}

int AiSystem::GetStrategyIdx(const char* stratName) const
{
	int result = -1;
	for (auto i = 0u; i < aiStrategies.size(); i++){
		if (_stricmp(stratName, aiStrategies[i].name.c_str()) == 0)
		{
			return i;
		}
	}

	return result;
	
}

void AiSystem::AiListRemove(const objHndl & handle, const objHndl & tgt, int aiType){
	auto obj = objSystem->GetObject(handle);
	auto aiList = obj->GetObjectIdArray(obj_f_npc_ai_list_idx);
	int aiListCount = aiList.GetSize();
	auto lastIdx = aiListCount - 1;
	for (int i=0; i < aiListCount; i++){

		auto aiListItem     = obj->GetObjHndl(obj_f_npc_ai_list_idx, i);
		auto aiListItemType = obj->GetInt32(obj_f_npc_ai_list_type_idx, i);

		if (!(aiListItem == tgt && aiListItemType == aiType || !aiListItem))
			continue;

		if (i < lastIdx){
			auto lastItem      = obj->GetObjHndl(obj_f_npc_ai_list_idx, lastIdx);
			auto lastItemType  = obj->GetInt32(obj_f_npc_ai_list_type_idx, lastIdx);
			obj->SetObjHndl(obj_f_npc_ai_list_idx, i, lastItem);
			obj->SetInt32(obj_f_npc_ai_list_type_idx, i, lastItemType);
		}
		obj->RemoveObjectId(obj_f_npc_ai_list_idx, lastIdx);
		obj->RemoveInt32(obj_f_npc_ai_list_type_idx, lastIdx--);
		aiListCount--; i--;
	}
}

int AiSystem::GetAiSpells(AiSpellList* aiSpell, objHndl obj, AiSpellType aiSpellType)
{
	aiSpell->spellEnums.clear();
	aiSpell->spellData.clear();
	auto objBod = objSystem->GetObject(obj);
	{
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
			if (!((spellEntry.aiTypeBitmask & 1 << aiSpellType) == 1 << aiSpellType))
				continue;


			bool spellAlreadyFound = false;
			for (auto j = 0u; j < aiSpell->spellEnums.size(); j++)
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
	}

	auto spellsKnown = objBod->GetSpellArray(obj_f_critter_spells_known_idx);
	for (auto i = 0u; i < spellsKnown.GetSize(); i++)
	{
		auto spellData = spellsKnown[i];

		auto spEnum = spellData.spellEnum;
		SpellEntry spellEntry(spEnum);
		if (!spellEntry.spellEnum || spellEntry.aiTypeBitmask == 0)
			continue;
		if (!((spellEntry.aiTypeBitmask & 1 << aiSpellType) == 1 << aiSpellType))
			continue;
		if (spellSys.isDomainSpell(spellData.classCode))
			continue;
		auto casterClass = spellSys.GetCastingClass(spellData.classCode);
		if (d20ClassSys.IsVancianCastingClass(casterClass))
			continue;

		bool spellAlreadyFound = false;
		for (auto j = 0u; j < aiSpell->spellEnums.size(); j++){
			if (aiSpell->spellEnums[j] == spEnum){
				spellAlreadyFound = true;
				break;
			}
		}
		if (spellAlreadyFound)
			continue;

		if (!spellSys.spellCanCast(obj, spEnum, spellData.classCode, spellData.spellLevel))
			continue;
		if (spellSys.IsNaturalSpellsPerDayDepleted(obj, spellData.spellLevel, spellData.classCode))
			continue;

		
		aiSpell->spellEnums.push_back(spellData.spellEnum);
		D20SpellData d20SpellData;
		d20SpellData.spellEnumOrg = spellData.spellEnum;
		d20SpellData.spellClassCode = spellData.classCode;
		d20SpellData.metaMagicData = spellData.metaMagicData;
		d20SpellData.itemSpellData = -1;
		d20SpellData.spellSlotLevel = spellData.spellLevel; // hey, I think this was missing / wrong in the original code!

		aiSpell->spellData.push_back(d20SpellData);
		
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
	int performerIsIntelligent = objects.StatLevelGet(performer, stat_intelligence) >= 3;
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
			//logger->debug("Enemy under test: {}", enemies[i]);
			if (combatSys.CanMeleeTarget(enemies[i], performer))
			{
				isThreatened = true;
				break;
			}
		}
		delete [] enemies;
		if (!isThreatened)
			shouldWake = rngSys.GetInt(1, 100) <= 40;
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

int AiSystem::AiGoto(AiTactic* aiTac)
{
	locXY loc = { aiTac->field4, aiTac->field24 };
	if (!loc.ToField()) return 0;

	int initialActNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	LocAndOffsets locAndOffsets = { loc, 0, 0 };

	actSeqSys.curSeqReset(aiTac->performer);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_UNSPECIFIED_MOVE, 0);
	d20Sys.GlobD20ActnSetTarget(objHndl::null, &locAndOffsets);
	actSeqSys.ActionAddToSeq();
	if (actSeqSys.ActionSequenceChecksWithPerformerLocation() != AEC_OK) {
		actSeqSys.ActionSequenceRevertPath(initialActNum);
		return FALSE;
	}
	return TRUE;
}

int AiSystem::AiHalt(AiTactic* aiTac)
{
	if (actSeqSys.curSeqGetTurnBasedStatus()->hourglassState > 2) {
		actSeqSys.curSeqGetTurnBasedStatus()->hourglassState = 2;
	}
	actSeqSys.curSeqGetTurnBasedStatus()->surplusMoveDistance = 0;
	return FALSE;
}

int AiSystem::AiStop(AiTactic* aiTac)
{
	actSeqSys.curSeqGetTurnBasedStatus()->hourglassState = 0;
	actSeqSys.curSeqGetTurnBasedStatus()->surplusMoveDistance = 0;
	return TRUE;
}

int AiSystem::AiTargetObj(AiTactic* aiTac)
{
	if (!aiTac->field24 && !aiTac->field4) return FALSE;
	objHndl handle;
	uint64_t h1 = (uint64_t)aiTac->field24 << 32 | aiTac->field4;
	handle = h1;
	if (!handle || !objSystem->IsValidHandle(handle)) 
		return FALSE;

	logger->info("AiTargetObj: \t {} set target from {} to {}", aiTac->performer, aiTac->target, handle);
	aiTac->target = handle;
	return FALSE;
}

int AiSystem::AiTotalDefence(AiTactic* aiTac)
{
	int initialActNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	actSeqSys.curSeqReset(aiTac->performer);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_TOTAL_DEFENSE, 0);
	d20Sys.GlobD20ActnSetTarget(objHndl::null, 0);
	actSeqSys.ActionAddToSeq();
	auto result = actSeqSys.ActionSequenceChecksWithPerformerLocation();
	logger->info("AiTotalDefence: \t {} attempting total defence => {}", aiTac->performer, result);
	if (result != AEC_OK) {
		actSeqSys.ActionSequenceRevertPath(initialActNum);
		return FALSE;
	}
	return TRUE;
}

int AiSystem::AiPythonAction(AiTactic* aiTac)
{
	auto actEnum = (D20DispatcherKey)aiTac->field4;
	if (!actEnum) 
		return FALSE;

	int initialActNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	actSeqSys.curSeqReset(aiTac->performer);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_PYTHON_ACTION, 0);
	d20Sys.globD20Action->SetPythonActionEnum(actEnum);
	d20Sys.GlobD20ActnSetTarget(aiTac->target, 0);
	actSeqSys.ActionAddToSeq();
	if (actSeqSys.ActionSequenceChecksWithPerformerLocation() != AEC_OK) {
		actSeqSys.ActionSequenceRevertPath(initialActNum);
		return FALSE;
	}
	return TRUE;
}

int AiSystem::AiD20Action(AiTactic* aiTac)
{
	auto d20type = (D20ActionType)aiTac->field4;
	if (!d20type)
		return FALSE;

	auto data1 = aiTac->field24;
	int initialActNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;
	actSeqSys.curSeqReset(aiTac->performer);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(d20type, data1);
	d20Sys.GlobD20ActnSetTarget(aiTac->target, 0);
	actSeqSys.ActionAddToSeq();
	auto result = actSeqSys.ActionSequenceChecksWithPerformerLocation();
	logger->info("AiD20Action: \t {} attempting {} on {} => {}", aiTac->performer, d20type, aiTac->target, result);
	if (result != AEC_OK) {
		actSeqSys.ActionSequenceRevertPath(initialActNum);
		return FALSE;
	}
	return TRUE;
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
	auto initialActNum = curSeq->d20ActArrayNum;

	
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_UNSPECIFIED_ATTACK, 0);
	d20Sys.GlobD20ActnSetTarget(aiTac->target, 0);
	ActionErrorCode addToSeqError = (ActionErrorCode)actSeqSys.ActionAddToSeq();
	//if (addToSeqError)
	//{
	//	logger->info("AI Default failed, error code: {}", (int)addToSeqError);
	//}
	auto performError = actSeqSys.ActionSequenceChecksWithPerformerLocation();
	if (performError == AEC_OK && addToSeqError == AEC_OK){
		return TRUE;
	} 
	else{
		actSeqSys.ActionSequenceRevertPath(initialActNum); // Temple+: fixed issue with Default tactic affecting next one to do not resetting it here
		logger->info("AI Default SequenceCheck failed, error codes are AddToSeq: {}, Location Checs: {}", addToSeqError, performError);
	}
	if (!critterSys.IsWieldingRangedWeapon(aiTac->performer))
	{
		if (combatSys.IsWithinReach(aiTac->performer, aiTac->target))
			return 0;

		logger->info("AI Action Perform: Resetting sequence; Do Unspecified Move Action");
		actSeqSys.curSeqReset(aiTac->performer);
		initialActNum = curSeq->d20ActArrayNum;

		auto isWorth = Is5FootStepWorth(aiTac);

		if (isWorth) {
			logger->info("AI Default: 5' step deemed worthwhile");
			d20Sys.GlobD20ActnSetTypeAndData1(D20A_5FOOTSTEP, 0);
			d20Sys.GlobD20ActnSetTarget(aiTac->target, nullptr);
			if (actSeqSys.ActionAddToSeq() == AEC_OK
				&& actSeqSys.ActionSequenceChecksWithPerformerLocation() == AEC_OK){
				logger->info("AI Default: Doing 5' step");
				return TRUE;
			}
			logger->info("AI Default: Cancelling 5' step");
			actSeqSys.ActionSequenceRevertPath(initialActNum);
		}

		d20Sys.GlobD20ActnInit();
		d20Sys.GlobD20ActnSetTypeAndData1(D20A_UNSPECIFIED_MOVE, 0);
		d20Sys.GlobD20ActnSetTarget(aiTac->target, 0);
		addToSeqError = (ActionErrorCode)actSeqSys.ActionAddToSeq();
		performError = actSeqSys.ActionSequenceChecksWithPerformerLocation();
		if (addToSeqError != AEC_OK || performError != AEC_OK )
		{
			logger->info("AI Default: Unspecified Move failed. AddToSeqError: {}  Location Checks Error: {}", addToSeqError, performError);
			actSeqSys.ActionSequenceRevertPath(initialActNum);
			return FALSE;
		}
	}
	return performError == AEC_OK && addToSeqError == AEC_OK;
}

BOOL AiSystem::DefaultCast(AiTactic* aiTac){
	int N_before = actSeqSys.GetCurSeqD20ActionCount();
	if (!aiTac->target)
		return FALSE;

	auto performer = aiTac->performer;

	

	if (!aiTac->d20SpellData.spellEnumOrg){
		AiSpellList offensiveAiSpell;
		AiSpellList defensiveAiSpell;
		
		GetAiSpells(&offensiveAiSpell, performer, AiSpellType::ai_action_offensive);
		GetAiSpells(&defensiveAiSpell, performer, AiSpellType::ai_action_defensive);

		if (!ChooseRandomSpellFromList(aiTac, &offensiveAiSpell)){
			if (!ChooseRandomSpellFromList(aiTac, &defensiveAiSpell))
				return FALSE;
		}

	}

	aiSys.AiFiveFootStepAttempt(aiTac);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_CAST_SPELL, 0);
	actSeqSys.ActSeqCurSetSpellPacket(&aiTac->spellPktBody, 0);
	d20Sys.GlobD20ActnSetSpellData(&aiTac->d20SpellData);

	auto tgtObj = objSystem->GetObject(aiTac->target);
	auto tgtLoc = tgtObj->GetLocationFull();
	d20Sys.GlobD20ActnSetTarget(aiTac->target, &tgtLoc);

	actSeqSys.ActionAddToSeq();
	auto actCheck = actSeqSys.ActionSequenceChecksWithPerformerLocation();

	if (actCheck != AEC_OK) {
		auto actErrorStr = actSeqSys.ActionErrorString(actCheck);
		logger->info("DefaultCast: caster {} cannot cast spell {} because reason: {}", aiTac->performer, spellSys.GetSpellName(aiTac->spellPktBody.spellEnum), actErrorStr);
		actSeqSys.ActionSequenceRevertPath(N_before);
		return FALSE;
	}

	return TRUE;
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
		float allyAbsX, allyAbsY;
		// get the diametrically opposed location
		locSys.GetOverallOffset(allyLoc, &allyAbsX, &allyAbsY);
		float deltaX = allyAbsX - tgtAbsX;
		float deltaY = allyAbsY - tgtAbsY;
		float normalization = 1.0f / sqrtf(deltaY*deltaY + deltaX*deltaX);
		float xHat = deltaX * normalization, // components of unit vector from target to ally
				yHat = deltaY * normalization;

		float flankAbsX = tgtAbsX - xHat * flankDist;
		float flankAbsY = tgtAbsY - yHat * flankDist;
		auto flankLoc = LocAndOffsets::FromInches(flankAbsX, flankAbsY);
		if (!pathfindingSys.PathDestIsClear(performer, &flankLoc))
		{
			bool foundFlankLoc = false;
			// try to tweak the angle; the flank check looks for the range of 120 - 240�, so we'll try 135,165,195,225
			float tweakAngles[4] = { -15.0, 15.0, -45.0, 45.0 };
			for (int j = 0; j < 4; j++)
			{
				float tweakAngle = tweakAngles[j] * (float)M_PI / 180.f;
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

			if (!foundFlankLoc){
				if (i == numAllies-1)
					logger->info("No clear flanking position found; next ai tactic.");
				return FALSE;
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

/* 0x1005EEC0 */
void AiSystem::AiProcess(objHndl obj){

	// Check if Player Controlled (if so, skip)
	if (objects.IsPlayerControlled(obj))
		return;

	auto isCombatActive = combatSys.isCombatActive();

	if ( isCombatActive 
		&& critterSys.isCritterCombatModeActive(obj)
		&& tbSys.turnBasedGetCurrentActor() != obj
		&& actSeqSys.getNextSimulsPerformer() != obj){
		return;
	}

	if (aiSys.IsPcUnderAiControl(obj)){
		if (!AiProcessPc(obj)){
			logger->debug("Combat for {} ending turn (script)...", obj);
		}
		return;
	}

	if (d20Sys.d20Query(obj, DK_QUE_AI_Has_Spell_Override)){ // from Confusion Spell
		int confusionState = (int)d20Sys.d20QueryReturnData(obj, DK_QUE_AI_Has_Spell_Override);
		if (confusionState > 0 && confusionState < 15){
			if (aiSys.AiProcessHandleConfusion(obj, confusionState))
				return;
		}
	}

	if (gameSystems->GetMap().IsClearingMap())
		return;

	if (critterSys.IsDeadOrUnconscious(obj)){
		if (combatSys.isCombatActive()){ // added to reduce log spam for knocked out NPCs
			logger->info("AI for {} ending turn (unconscious)...", obj);
		}
		combatSys.CombatAdvanceTurn(obj);
		return;
	}

	AiPacket aiPacket(obj);
	if (!aiPacket.PacketCreate())
		return;

	auto isCombat = combatSys.isCombatActive();
	auto curActor = tbSys.turnBasedGetCurrentActor();
	auto nextSimuls = actSeqSys.getNextSimulsPerformer();

	if (!isCombat
		|| critterSys.IsCombatModeActive(obj)
		|| curActor == obj 
		|| nextSimuls == obj){
		aiPacket.ProcessCombat();
		return;
	} 
	
	if (aiPacket.aiFightStatus == AIFS_FIGHTING && locSys.DistanceToObj(obj , aiPacket.target) <= 75.0 )
		combatSys.enterCombat(obj);
	
}

bool AiSystem::AiProcessHandleConfusion(objHndl handle, int confusionState){
	if (!d20Sys.d20Query(handle, DK_QUE_Critter_Is_Confused))
		return false;

	auto qdata = d20Sys.d20QueryReturnData(handle, DK_QUE_Critter_Is_Confused);
	int spellId = 0;
	objHndl designated = objHndl::null;

	if (confusionState == 13) {
		designated = qdata;
	} else {
		spellId = static_cast<int>(qdata);
	}

	SpellPacketBody spPkt(spellId);

	if (confusionState == 11){
		switch (Dice::Roll(1,6)-1){
		case 1:
			confusionState = 6; break;
		case 2:
			confusionState = 7; break;
		case 3:
			confusionState = 8; break;
		case 4:
			confusionState = 9; break;
		case 5:
			confusionState = 12; break;
		default:
			confusionState = 5; break;
		}
	}

	if (!spPkt.spellEnum) {
		if (confusionState == 5) confusionState = 7;
	}

	auto confusionTgt = objHndl::null;
	ObjList objList;
	switch(confusionState){
	case 5:
		aiSys.FleeProcess(handle, spPkt.caster);
		break;
	case 7:
		if (!combatSys.isCombatActive()){
			aiSys.FleeProcess(handle, spPkt.caster);
		}
		else{
			aiSys.FleeProcess(handle, combatSys.GetInitiativeListMember(rngSys.GetInt(0, combatSys.GetInitiativeListLength())));
		}
		break;
	case 8: 
		objList.ListVicinity(handle, OLC_CRITTERS);
		for (auto i=0; i <objList.size(); i++){
			if (objList[i]){
				confusionTgt = objList[i];
				break;
			}
		}
		break;
	case 9: 
		objList.ListVicinity(handle, OLC_CRITTERS);
		for (auto i = 0; i <objList.size(); i++) {
			if (critterSys.IsDeadNullDestroyed(objList[i]))
				continue;
			if (!confusionTgt ||
					locSys.DistanceToObj(handle, objList[i]) < locSys.DistanceToObj(handle, confusionTgt)){
					confusionTgt = objList[i];
			}
			
		}
		break;
	case 12: 
		confusionTgt = handle;
		break;
	case 13:
		confusionTgt = designated;
		break;
	case 6: // do whatever was doing last I guess
	case 10:
	case 11:
		break;
	default:
		return false;
	}

	// TODO: It seems like this just doesn't work well. I suspect that if the
	// target would normally be designated as friendly, the AI system ignores
	// it and picks a different target. Needless to say, this doesn't actually
	// play well with confusion, which is supposed to sometimes lead to your
	// party attacking one another.
	if (confusionTgt){
		aiSys.StrategyParse(handle, confusionTgt);
	}
	

	if (combatSys.isCombatActive()){
		auto curActor = tbSys.turnBasedGetCurrentActor();
		if (curActor == handle && !actSeqSys.isPerforming(curActor) && actSeqSys.IsSimulsCompleted() && !actSeqSys.IsLastSimultPopped(curActor)){
			logger->info("AI for {} ending turn (confusion while simuls)", curActor);
			combatSys.CombatAdvanceTurn(curActor);
		}
	}
		
	return true;
}

bool AiSystem::AiProcessPc(objHndl handle)
{

	auto leader = objHndl::null;
	auto fearer = objHndl::null;
	auto tgt = objHndl::null;

	auto findUnfriendly = [](objHndl leader)->objHndl{
		ObjList objList;
		objList.ListVicinity(leader, OLC_CRITTERS);
		for (auto i=0; i <objList.size(); i++){
			auto critter = objList[i];
			if (!critter)
				break;
			if (critterSys.IsFriendly(leader, critter))
				continue;
			return critter;
		}
		return objHndl::null;
	};


	auto isCharmed = d20Sys.d20Query(handle, DK_QUE_Critter_Is_Charmed);
	auto checkFlee = !isCharmed;

	if (!checkFlee && isCharmed){ // check if the leader is hostile (is this a proper check...?)
		leader = d20Sys.d20QueryReturnData(handle, DK_QUE_Critter_Is_Charmed);
		if (leader &&!critterSys.IsFriendly(handle, leader)){
			checkFlee = true;
		}
	}

	auto isFleeing = false;
	if (checkFlee){
		if (d20Sys.d20Query(handle, DK_QUE_Critter_Is_Afraid)) {
			fearer = d20Sys.d20QueryReturnData(handle, DK_QUE_Critter_Is_Afraid);
			FleeProcess(handle, fearer);
			isFleeing = true;
		}
	}


	if (!isFleeing){

		// check for a leader critter
		if (isCharmed)
			leader = d20Sys.d20QueryReturnData(handle, DK_QUE_Critter_Is_Charmed);

		if (!leader && d20Sys.d20Query(handle, DK_QUE_Critter_Is_AIControlled))
			leader = d20Sys.d20QueryReturnData(handle, DK_QUE_Critter_Is_AIControlled);

		if (leader && objSystem->IsValidHandle(leader)){ // if has a valid leader, try to target its foe
			auto leaderObj = objSystem->GetObject(leader);
			if (leaderObj->IsNPC()){ // inherit the leader's combat focus if any
				auto leaderCombatFocus = objSystem->GetObject(leader)->GetObjHndl(obj_f_npc_combat_focus);
				if (leaderCombatFocus)
					tgt = leaderCombatFocus;
			}
			if (!tgt){
				tgt = findUnfriendly(leader);
			}
		}
		
		// default - find a target for yourself
		if (!tgt || tgt == handle){
			tgt = findUnfriendly(handle);
		}

		if (!tgt) {
			logger->debug("AiProcessPc: unable to find a target for AI PC = {}", handle);
		}
		StrategyParse(handle, tgt);
	}

	if (!combatSys.isCombatActive())
		return true;

	auto curActor = tbSys.turnBasedGetCurrentActor();
	if (curActor == handle && !actSeqSys.isPerforming(handle)){
		if (actSeqSys.IsSimulsCompleted() && !actSeqSys.IsLastSimultPopped(handle)){
			logger->info("AI for {} ending turn (simuls unended)...", handle);
			combatSys.CombatAdvanceTurn(handle);
		}
	}

	return true;
	// auto aiProcessPc = temple::GetRef<int(__cdecl)(objHndl)>(0x1005AE10);
}

int AiSystem::AiTimeEventExpires(TimeEvent* evt)
{
	return 1;
}

#pragma endregion

#pragma region AI replacement functions

uint32_t _aiStrategyParse(objHndl objHnd, objHndl target)
{
	return aiSys.StrategyParse(objHnd, target);
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

int _StrategyTabLineParser(const TigTabParser* tabFile, int n, char** strings)
{
	aiSys.StrategyTabLineParser(tabFile, n, strings);
	return 0;
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
		if (spellEntry.IsBaseModeTarget(UiPickerType::Area) && spellEntry.spellRangeType == SpellRangeType::SRT_Personal
			|| spellEntry.IsBaseModeTarget(UiPickerType::Personal) && (spellEntry.flagsTargetBitmask & UiPickerFlagsTarget::Radius) ){
			spellRange = spellEntry.radiusTarget;
		}
		auto tgt = aiPkt->target;
		if (spellEntry.IsBaseModeTarget(UiPickerType::Personal)){
			tgt = aiPkt->obj;
		}

		if (tgt != aiPkt->obj
			&& objects.IsCritter(tgt)
			&& (d20Sys.d20Query(tgt, DK_QUE_Critter_Is_Grappling) == 1
			    || d20Sys.d20Query(tgt, DK_QUE_Critter_Is_Charmed)
			)){
			continue;
		}

		if (spellSys.spellCanCast(aiPkt->obj, spellEnum,spellClass, spellLevels[0])){
		
			if (!spellSys.GetSpellTargets(aiPkt->obj,tgt, &aiPkt->spellPktBod, spellEnum))
				continue;
			if (aiPkt->spellPktBod.targetListHandles[0]){
				tgt = aiPkt->spellPktBod.targetListHandles[0];
			}
			if (locSys.DistanceToObj(aiPkt->obj, tgt)>spellRange && 
				( spellSys.SpellHasAiType(spellEnum, ai_action_offensive)
				  || spellSys.SpellHasAiType(spellEnum, ai_action_defensive) )
				){
				continue;
			}

			aiPkt->aiState2 = 1;
			aiPkt->spellEnum = spellEnum;
			aiPkt->spellData = aiSpells->spellData[spellIdx];
			return 1;
		} 
		else{
			logger->debug("AiCheckSpells(): object {} ({}) cannot cast spell {}", aiPkt->obj, objSystem->GetObject(aiPkt->obj)->id.ToString(),  spellEnum);
		}

	}
	return 0;
}

int AiSystem::ChooseRandomSpellFromList(AiTactic * aiTac, AiSpellList* aiSpells)
{
	if (!aiSpells->spellEnums.size())
		return 0;
	
	auto performer = aiTac->performer;

	for (int i = 0; i < 5; i++) {
		auto spellIdx = rngSys.GetInt(0, aiSpells->spellEnums.size() - 1);
		spellSys.spellPacketBodyReset(&aiTac->spellPktBody);
		unsigned int spellClass;
		unsigned int spellLevels[2];
		d20Sys.ExtractSpellInfo(&aiSpells->spellData[spellIdx],
			&aiTac->spellPktBody.spellEnum,
			nullptr, &spellClass, spellLevels, nullptr, nullptr);
		auto spellEnum = aiTac->spellPktBody.spellEnum = aiSpells->spellEnums[spellIdx];
		aiTac->spellPktBody.caster = performer;
		aiTac->spellPktBody.spellEnumOriginal = spellEnum;
		aiTac->spellPktBody.spellKnownSlotLevel = spellLevels[0];
		aiTac->spellPktBody.spellClass = spellClass;
		spellSys.SpellPacketSetCasterLevel(&aiTac->spellPktBody);

		SpellEntry spellEntry;
		spellSys.spellRegistryCopy(spellEnum, &spellEntry);
		auto spellRange = spellSys.GetSpellRange(&spellEntry, aiTac->spellPktBody.casterLevel, aiTac->spellPktBody.caster);
		aiTac->spellPktBody.spellRange = spellRange;
		if (static_cast<UiPickerType>(spellEntry.modeTargetSemiBitmask & 0xFF) == UiPickerType::Area
			&& spellEntry.spellRangeType == SpellRangeType::SRT_Personal) {
			spellRange = spellEntry.radiusTarget;
		}
		auto tgt = aiTac->target;
		if (objects.IsCritter(tgt)
			&& d20Sys.d20Query(tgt, DK_QUE_Critter_Is_Grappling) == 1
			|| d20Sys.d20Query(tgt, DK_QUE_Critter_Is_Charmed)) {
			continue;
		}

		if (spellSys.spellCanCast(performer, spellEnum, spellClass, spellLevels[0])) {

			if (!spellSys.GetSpellTargets(performer, tgt, &aiTac->spellPktBody, spellEnum))
				continue;
			if (locSys.DistanceToObj(performer, tgt)>spellRange
				&& spellSys.SpellHasAiType(spellEnum, ai_action_offensive)
				|| spellSys.SpellHasAiType(spellEnum, ai_action_defensive)) {
				continue;
			}
			
			aiTac->d20SpellData = aiSpells->spellData[spellIdx];
			return 1;
		}
		else {
			logger->debug("AiCheckSpells(): object {} cannot cast spell {}", performer, spellEnum);
		}

	}
	return 0;
}

BOOL AiSystem::IsPcUnderAiControl(objHndl handle){
	
	if (!handle)
		return FALSE;

	// must be a PC that is not player controlled

	if (objects.IsPlayerControlled(handle))
		return FALSE;

	auto obj = objSystem->GetObject(handle);
	if (!obj->IsPC())
		return FALSE;

	// must be charmed, AI Controlled or Afraid
	auto queryAiControl =
		d20Sys.d20Query(handle, DK_QUE_Critter_Is_Charmed)
	|| d20Sys.d20Query(handle, DK_QUE_Critter_Is_AIControlled)
	|| d20Sys.d20Query(handle, DK_QUE_Critter_Is_Afraid);
	if (!queryAiControl)
		return FALSE;

	if (d20Sys.d20Query(handle, DK_QUE_Critter_Is_Afraid)){
		objHndl fearedObj;
		fearedObj.handle = d20Sys.d20QueryReturnData(handle, DK_QUE_Critter_Is_Afraid);
		if (!fearedObj)
			return FALSE;
		if (locSys.DistanceToObj(handle, fearedObj) > 40.0)
			return FALSE;
		if (!combatSys.HasLineOfAttack(fearedObj, handle))
			return FALSE;
	}

	return TRUE;
}

bool AiSystem::Is5FootStepWorth(AiTactic* aiTac){
	// when is it worth taking a 5' step to your target?
	
	
	

	if (!aiTac->target)
		return false;

	auto initialActNum = (*actSeqSys.actSeqCur)->d20ActArrayNum;

	// a. when you've used up your full round action and it's the only thing left to do
	auto &tbStat = (*actSeqSys.actSeqCur)->tbStatus;
	if (tbStat.hourglassState == 0 && !(tbStat.tbsFlags & (TBSF_Movement | TBSF_Movement2)) ){
		return true;
	}

	// b. when you can path to it in a 5' step (and thus let you take advantage of full attack)
	// check if 5' step is possible, and if so whether you can reach the target with it
	auto distToTgt = locSys.DistanceToObj(aiTac->performer, aiTac->target);
	auto canReachTgtWithStep = false;
	{
		auto canDoStep = false;
		d20Sys.GlobD20ActnSetTypeAndData1(D20A_5FOOTSTEP, 0);
		d20Sys.GlobD20ActnSetTarget(aiTac->target, nullptr);
		
		if (actSeqSys.ActionAddToSeq() == AEC_OK
			&& !actSeqSys.ActionSequenceChecksWithPerformerLocation())
		{
			canDoStep = true;

			auto &fiveFootAction = (*actSeqSys.actSeqCur)->d20ActArray[initialActNum];
			auto pqr = fiveFootAction.path;
			if (pqr && !(fiveFootAction.d20Caf & D20CAF_TRUNCATED)){
				if (critterSys.GetReach(aiTac->performer, D20A_STANDARD_ATTACK) + 5.0 >= distToTgt)
					canReachTgtWithStep = true;
			}

		}
		actSeqSys.ActionSequenceRevertPath(initialActNum);
		if (!canDoStep)
			return false;
	}

	// if target is reachable with 5' step, then hell yeah!
	if (canReachTgtWithStep)
		return true;

	// c. when approaching your target otherwise would incur AoOs (regard tumbling here...)
	// todo


	// todo advanced - consider spring attack...

	return false;
}

unsigned int _AiAsplode(AiTactic * aiTac)
{
	return aiSys.Asplode(aiTac);
}

unsigned int _AiWakeFriend(AiTactic * aiTac)
{
	return aiSys.WakeFriend(aiTac);
}

unsigned int _AiGoto(AiTactic * aiTac)
{
	return aiSys.AiGoto(aiTac);
}

unsigned int _AiHalt(AiTactic * aiTac)
{
	return aiSys.AiHalt(aiTac);
}

unsigned int _AiStop(AiTactic * aiTac)
{
	return aiSys.AiStop(aiTac);
}

unsigned int _AiTargetObj(AiTactic * aiTac)
{
	return aiSys.AiTargetObj(aiTac);
}

unsigned int _AiTotalDefence(AiTactic* aiTac)
{
	return aiSys.AiTotalDefence(aiTac);
}

unsigned int _AiPythonAction(AiTactic* aiTac)
{
	return aiSys.AiPythonAction(aiTac);
}

unsigned int _AiD20Action(AiTactic* aiTac)
{
	return aiSys.AiD20Action(aiTac);
}

unsigned int _AiUseItem(AiTactic* aiTac)
{
	return aiSys.UseItem(aiTac);
}

unsigned int _AiApproachSingle(AiTactic* aiTac)
{
	return aiSys.ApproachSingle(aiTac);
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
	static int CastArea(AiTactic* aiTac);
	static int CastFireball(AiTactic* aiTac);
	static int CastSingle(AiTactic* aiTac);
	

	static int ChooseRandomSpellUsercallWrapper();
	static void SetCritterStrategy(objHndl obj, const char *stratName);

	void apply() override 
	{
		logger->info("Replacing AI functions...");

		/*replaceFunction<BOOL(__cdecl)(objHndl, objHndl)>(0x, []() {
			py::tuple args = py::make_tuple(py::cast(critter1), py::cast<objHndl>(critter2));
			auto pyResult = pythonObjIntegration.ExecuteScript("d20_ai.friendship", "is_friendly", args.ptr());
			if (PyInt_Check(pyResult)) {
				auto result = _PyInt_AsInt(pyResult);
				if (result)
					return TRUE;
			}
			});*/

		// AiFightStatusProcess
		replaceFunction<void(__cdecl)(objHndl, objHndl)>(0x1005CD50, [](objHndl handle, objHndl newTgt){
			aiSys.FightStatusProcess(handle, newTgt);
		});

		// UpdateAiFlags
		replaceFunction<int(__cdecl)(objHndl, AiFightStatus, objHndl, int*)>(0x1005DA00, [](objHndl handle, AiFightStatus aiFightStatus, objHndl target, int* soundMap){
			return aiSys.UpdateAiFlags(handle, aiFightStatus, target, soundMap);
		});

		replaceFunction<void(__cdecl)(objHndl)>(0x1005EEC0, [](objHndl handle){
			aiSys.AiProcess(handle);
		});

		// Will KOS
		replaceFunction<int(__cdecl)(objHndl, objHndl)>(0x1005C920, [](objHndl aiHandle, objHndl triggerer){
			return aiSys.WillKos(aiHandle, triggerer);
		});
		
		replaceFunction(0x100E3270, AiDefault);
		replaceFunction<BOOL(__cdecl)(AiTactic*)>(0x100E4310, [](AiTactic *aiTac){
			return aiSys.DefaultCast(aiTac);
		});
		replaceFunction<BOOL(__cdecl)(AiTactic*)>(0x100E4A40, [](AiTactic*aiTac)->BOOL {
			return aiSys.UsePotion(aiTac);
		});
		replaceFunction(0x100E3A00, _AiTargetClosest);
		replaceFunction(0x100E3B60, AiRage);
		replaceFunction(0x100E41E0, CastSingle);
		replaceFunction(0x100E43F0, AiCastParty);
		replaceFunction(0x100E4510, CastArea);
		replaceFunction(0x100E5C50, CastFireball); // todo: I've added LOS check, but now the AI needs to know how to tweak it in case it's blocked but good targets are around the corner

		replaceFunction(0x100E46C0, AiAttack);
		replaceFunction(0x100E46D0, AiTargetThreatened);
		replaceFunction<BOOL(__cdecl)(AiTactic*)>(0x100E37D0, [](AiTactic*aiTac)->BOOL {
			return aiSys.TargetDamaged(aiTac);
		});
		replaceFunction<BOOL(__cdecl)(AiTactic*)>(0x100E4070, [](AiTactic*aiTac)->BOOL {
			return aiSys.TargetFriendHurt(aiTac);
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
		replaceFunction<void(_cdecl)()>(0x100E5E40, []() { //Strategy.tab init
			TigTabParser tabParse;
			tabParse.Init(_StrategyTabLineParser);
			tabParse.Open("Rules\\strategy.tab");
			tabParse.Process();
			tabParse.Close();

			aiSys.InitCustomStrategies();
		});
		replaceFunction(0x100E5500, _StrategyTabLineParser);
		replaceFunction(0x100E55A0, AiGoMelee);
		replaceFunction(0x100E58D0, AiSniper);
		replaceFunction(0x100E5950, AiFlank);
		replaceFunction(0x100E5DB0, _AiCoupDeGrace);
		

		replaceFunction<void(__cdecl)()>(0x100E27D0, [](){
			// replace AiStrategyExit with a function that does nothing (since we no longer have to free aiStrategies and the names manually)
		});

		replaceFunction(0x1005B810, ChooseRandomSpellUsercallWrapper);


		static int (*orgAiTimeEventExpires)(TimeEvent*) = replaceFunction<int(__cdecl)(TimeEvent*)>(0x1005F090, [](TimeEvent*evt)
		{
			auto handle = evt->params[0].handle;
			if (!handle){
				return TRUE;
			}

			auto obj = objSystem->GetObject(handle);
			if (!obj){
				return TRUE;
			}

			static auto scheduleNormalAiEvent = temple::GetRef<void(__cdecl)(objHndl, GameTime&)>(0x1005BE00);
			auto canDoHeartbeat = true;
			auto isReschedule = true;
			auto isFirstHeartbeat = evt->params[1].int32;

			if (!critterSys.IsDeadNullDestroyed(handle) && isFirstHeartbeat){
				auto aiFlags = obj->GetInt64(obj_f_npc_ai_flags64);
				obj->SetInt64(obj_f_npc_ai_flags64, aiFlags | AiFlag::CheckWield);
				
				// NPC Wander off
				if (!party.IsInParty(handle) 
					&& !critterSys.isCritterCombatModeActive(handle)
					&& !temple::GetRef<BOOL(__cdecl)(objHndl, int)>(0x1005B950)(handle, 1) /* waypoint related */) {
					temple::GetRef<void(__cdecl)(objHndl, int) >(0x1005BC00)(handle, 1); // NPC Wander
				}

				if (pythonObjIntegration.ExecuteObjectScript(handle, handle, ObjScriptEvent::FirstHeartbeat) != TRUE){
					canDoHeartbeat = false;
				}
			}

			if (canDoHeartbeat){
				canDoHeartbeat = temple::GetRef<bool(__cdecl)(objHndl)>(0x10058730)(handle);
			}
			
			if (canDoHeartbeat){
				if (pythonObjIntegration.ExecuteObjectScript(handle, handle, ObjScriptEvent::Heartbeat) == TRUE){
					if (! (obj->GetInt32(obj_f_npc_flags) & NpcFlag::ONF_GENERATOR) ){
						aiSys.AiProcess(handle);
					}
					else{
						static auto generatorGetNextEvent = temple::GetRef<int(__cdecl)(objHndl, GameTime&)>(0x10050740);
						GameTime time;
						if (generatorGetNextEvent(handle, time)){
							scheduleNormalAiEvent(handle, time);
							return TRUE;
						}
						else{
							isReschedule = false;
						}
					}
				}
			}
			else{
				if (!combatSys.isCombatActive() && !critterSys.IsDeadNullDestroyed(handle)){
					gameSystems->GetAnim().Interrupt(handle, AGP_4, false);
				}
				SectorLoc secLoc(obj->GetLocation());
				auto sectorLoaded = gameSystems->GetMapSector().IsSectorLoaded(secLoc);
				if (!temple::GetRef<BOOL(__cdecl)(objHndl)>(0x10058780)(handle) && !sectorLoaded){
					isReschedule = false;
				}
			}

			
			if (!isReschedule){
				if (!combatSys.isCombatActive()){
					if (obj->GetInt32(obj_f_critter_flags) & OCF_ENCOUNTER){
						objects.Destroy(handle);
					}
				}
				return TRUE;
			}
			
			// Reschedule AI event with fixed delay based on distance from party + optional random delay
			auto aiEvtDelay = temple::GetRef<int(__cdecl)(objHndl)>(0x10058850)(handle); // Minimum 250ms
			if (isFirstHeartbeat) {
				aiEvtDelay += rngSys.GetInt(0, 5000);
			}
			GameTime time(0, aiEvtDelay);
			scheduleNormalAiEvent(handle, time);
			return TRUE;
			
			/*auto result =  orgAiTimeEventExpires(evt);
			return result;*/
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
			logger->debug("Generating AI TimeEvent for {}, first heartbeat: {}", obj, doFirstHeartbeat);
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

		replaceFunction<void(__cdecl)(objHndl, objHndl, int, int)>(0x1005E8D0, [](objHndl agitator, objHndl provokedNpc, int rangeIdx, int flags) {
			return aiSys.ProvokeHostility(agitator, provokedNpc, rangeIdx, flags);
			});

		static int(*orgScriptExecute)(objHndl, objHndl, int, int, SAN, void*) = 
			replaceFunction<int(objHndl, objHndl, int, int, SAN, void*)>(0x10025D60, [](objHndl triggerer, objHndl attachee, int a3, int a4, SAN san, void* a6)->int
		{

			/*if (triggerer && san == san_enter_combat) {
				auto tgtObj = gameSystems->GetObj().GetObject(triggerer);
				if (tgtObj->type == obj_t_pc) {
					logger->debug("ScriptExecute: For {}, triggerer {}", attachee, triggerer);
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

int AiReplacements::CastArea(AiTactic* aiTac){
	auto actNumOrg = actSeqSys.GetCurSeqD20ActionCount();
	if (!aiTac->target)
		return FALSE;
	if (d20Sys.d20QueryWithData(aiTac->target, DK_QUE_Critter_Has_Spell_Active, aiTac->spellPktBody.spellEnum, 0)) {
		return FALSE;
	}
	auto performer = aiTac->performer;
	auto enemies = combatSys.GetEnemiesCanMelee(performer);
	auto castDefensively = 0;
	if (enemies.size() > 0) {
		castDefensively = 1;
	}

	d20Sys.d20SendSignal(performer, DK_SIG_SetCastDefensively, castDefensively, 0);

	SpellEntry spEntry(aiTac->d20SpellData.spellEnumOrg);
	if (!spEntry.spellEnum) {
		return FALSE;
	}
	
	PickerArgs pickArgs;
	auto locAndOff = objSystem->GetObject(aiTac->target)->GetLocationFull();
	//uiPicker.PickerArgsInit(&pickArgs);
	if (!spellSys.pickerArgsFromSpellEntry(&spEntry, &pickArgs, performer, aiTac->spellPktBody.casterLevel)){
		return FALSE;
	}

	// Temple+: added LOS check (since otherwise it's only done by UI code)
	if (uiPicker.PickerLosBlocked(performer, locAndOff)) {
		return FALSE;
	}

	uiPicker.GetListRange(&locAndOff, &pickArgs);
	spellSys.ConfigSpellTargetting(&pickArgs, &aiTac->spellPktBody);
	pickArgs.FreeObjlist();
	
	spellSys.GetSpellTargets(performer, aiTac->target, &aiTac->spellPktBody, aiTac->spellPktBody.spellEnum);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_CAST_SPELL, 0);
	actSeqSys.ActSeqCurSetSpellPacket(&aiTac->spellPktBody, 0);
	d20Sys.GlobD20ActnSetSpellData(&aiTac->d20SpellData);
	
	d20Sys.GlobD20ActnSetTarget(objHndl::null, &locAndOff);
	actSeqSys.ActionAddToSeq();
	if (actSeqSys.ActionSequenceChecksWithPerformerLocation())
	{
		actSeqSys.ActionSequenceRevertPath(actNumOrg);
		return FALSE;
	}
	return TRUE;
}

/* 0x100E5C50 */
int AiReplacements::CastFireball(AiTactic* aiTac)
{
	auto actNumOrg = actSeqSys.GetCurSeqD20ActionCount();
	if (!aiTac->target)
		return FALSE;
	
	auto performer = aiTac->performer;
	auto enemies = combatSys.GetEnemiesCanMelee(performer);
	auto castDefensively = 0;
	if (enemies.size() > 0) {
		castDefensively = 1;
	}

	d20Sys.d20SendSignal(performer, DK_SIG_SetCastDefensively, castDefensively, 0);

	SpellEntry spEntry(aiTac->d20SpellData.spellEnumOrg);
	if (!spEntry.spellEnum) {
		return FALSE;
	}

	PickerArgs pickArgs;
	LocAndOffsets locAndOff = LocAndOffsets::null;
	if (aiTac->target) {
		locAndOff = objects.GetLocationFull(aiTac->target);
	}
	if (!aiTac->ChooseFireballLocation(locAndOff)) {
		return FALSE;
	}


	//uiPicker.PickerArgsInit(&pickArgs);
	if (!spellSys.pickerArgsFromSpellEntry(&spEntry, &pickArgs, performer, aiTac->spellPktBody.casterLevel)) {
		return FALSE;
	}

	// Temple+: added LOS check (since otherwise it's only done by UI code)
	if (uiPicker.PickerLosBlocked(performer, locAndOff)) {
		return FALSE;
	}

	uiPicker.GetListRange(&locAndOff, &pickArgs);
	spellSys.ConfigSpellTargetting(&pickArgs, &aiTac->spellPktBody);
	pickArgs.FreeObjlist();

	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_CAST_SPELL, 0);
	actSeqSys.ActSeqCurSetSpellPacket(&aiTac->spellPktBody, 0);
	d20Sys.GlobD20ActnSetSpellData(&aiTac->d20SpellData);

	d20Sys.GlobD20ActnSetTarget(objHndl::null, &locAndOff);
	actSeqSys.ActionAddToSeq();
	if (actSeqSys.ActionSequenceChecksWithPerformerLocation())
	{
		actSeqSys.ActionSequenceRevertPath(actNumOrg);
		return FALSE;
	}
	return TRUE;
}

int AiReplacements::CastSingle(AiTactic* aiTac){
	auto actNumOrg = actSeqSys.GetCurSeqD20ActionCount();
	if (!aiTac->target)
		return FALSE;
	if (d20Sys.d20QueryWithData(aiTac->target,	DK_QUE_Critter_Has_Spell_Active, aiTac->spellPktBody.spellEnum, 0)){
		return FALSE;
	}
	auto performer = aiTac->performer;
	auto enemies = combatSys.GetEnemiesCanMelee(performer);
	auto castDefensively = 0;
	if (enemies.size() > 0){
		castDefensively = 1;
	}

	d20Sys.d20SendSignal(performer, DK_SIG_SetCastDefensively, castDefensively, 0);

	spellSys.GetSpellTargets(performer, aiTac->target, &aiTac->spellPktBody, aiTac->spellPktBody.spellEnum);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20A_CAST_SPELL, 0);
	actSeqSys.ActSeqCurSetSpellPacket(&aiTac->spellPktBody, 0);
	d20Sys.GlobD20ActnSetSpellData(&aiTac->d20SpellData);
	if (aiTac->target){
		auto locAndOff = objSystem->GetObject(aiTac->target)->GetLocationFull();
		d20Sys.GlobD20ActnSetTarget(aiTac->target, &locAndOff);
	}
	actSeqSys.ActionAddToSeq();
	if (actSeqSys.ActionSequenceChecksWithPerformerLocation())
	{
		actSeqSys.ActionSequenceRevertPath(actNumOrg);
		return FALSE;
	}
	return TRUE;
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
	auto objBody = objSystem->GetObject(obj);
	if (objBody->IsPC()){
		//auto critRole=  aiSys.GetRole(obj);
		//auto idx = aiSys.GetStrategyIdx("default"); // default
		//if (critRole == AiCombatRole::caster)
		//	idx = aiSys.GetStrategyIdx("Charmed PC Caster");
		//else
		//	idx = aiSys.GetStrategyIdx("Charmed PC Fighter");
		//objBody->SetInt32(obj_f_critter_strategy, idx);
		//return;
	}

	auto idx = aiSys.GetStrategyIdx(stratName);
	if (idx == -1)
	{
		idx = aiSys.GetStrategyIdx("default");
	}
	objBody->SetInt32(obj_f_critter_strategy, idx);
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
	soundMap = -1;
}

BOOL AiPacket::PacketCreate(){
	if (!WieldBestItem())
		return FALSE;

	if (d20Sys.d20Query(this->obj, DK_QUE_Critter_Is_Afraid)){
		objHndl fearedObj;
		fearedObj.handle = d20Sys.d20QueryReturnData(this->obj, DK_QUE_Critter_Is_Afraid);
		this->target = fearedObj;
		this->aiFightStatus = aiSys.UpdateAiFlags(this->obj, AiFightStatus::AIFS_FLEEING, fearedObj, &this->soundMap);
		return TRUE;
	}

	if (SelectHealSpell()) // select heal spell & target from: self, leader, leader's followers
		return TRUE;

	if (!LookForEquipment()){
		FightStatusUpdate();
	}

	return TRUE;
}

BOOL AiPacket::WieldBestItem(){
	if (critterSys.IsDeadOrUnconscious(obj))
		return FALSE;
	
	if (!critterSys.IsSleeping(obj) && objSystem->GetObject(obj)->GetFlags() & (OF_DONTDRAW | OF_OFF))
		return FALSE;
	auto pc0 = party.GroupPCsGetMemberN(0);
	if (objSystem->GetObject(pc0)->GetFlags() & OF_OFF)
		return FALSE;
	if (objSystem->GetObject(obj)->GetInt32(obj_f_spell_flags) & SpellFlags::SF_10000)
		return FALSE;

	auto critterFlags = critterSys.GetCritterFlags(obj);
	if (critterFlags & CritterFlag::OCF_STUNNED)
		return FALSE;

	auto getDlgTarget = temple::GetPointer<objHndl(objHndl)>(0x10053CA0);
	if (getDlgTarget(obj))
		return FALSE;

	auto animPriority = gameSystems->GetAnim().GetCurrentPriority(obj);
	if (animPriority != 7 && animPriority > 2)
		return FALSE;

	if (critterSys.IsDeadOrUnconscious(obj))
		return FALSE;

	auto npcFlags = objSystem->GetObject(obj)->GetNPCFlags();
	if (npcFlags & ONF_GENERATOR)
		return FALSE;

	auto aiFlags =  objSystem->GetObject(obj)->GetInt64(obj_f_npc_ai_flags64);
	if (aiFlags & AiFlag::RunningOff)
		return FALSE;

	if (aiFlags & AiFlag::CheckWield && !combatSys.isCombatActive()){
		if (!modSupport.IsZMOD()) {
			inventory.WieldBestAll(obj, target);
		}
	    aiFlags =	objSystem->GetObject(obj)->GetInt64(obj_f_npc_ai_flags64);
		aiFlags &= ~(AiFlag::CheckWeapon | AiFlag::CheckWield);
		objSystem->GetObject(obj)->SetInt64(obj_f_npc_ai_flags64, aiFlags);
	}
	else if (aiFlags & AiFlag::CheckWeapon){
		if (!modSupport.IsZMOD()) {
			inventory.WieldBest(obj, 200 + EquipSlot::WeaponPrimary, target);
		}
		aiFlags = objSystem->GetObject(obj)->GetInt64(obj_f_npc_ai_flags64);
		aiFlags &= ~(AiFlag::CheckWeapon );
		objSystem->GetObject(obj)->SetInt64(obj_f_npc_ai_flags64, aiFlags);;
	}

	if (npcFlags & ONF_DEMAINTAIN_SPELLS){
		auto leader = critterSys.GetLeader(obj);
		if (!leader || critterSys.IsDeadNullDestroyed(leader) || !critterSys.IsCombatModeActive(leader))
			objSystem->GetObject(obj)->SetInt32(obj_f_npc_flags, npcFlags & ~(ONF_DEMAINTAIN_SPELLS));
	}

	return TRUE;

}

BOOL AiPacket::SelectHealSpell(){
	if (critterSys.IsSleeping(obj) || aiFightStatus == AIFS_FLEEING)
		return FALSE;

	if (modSupport.IsZMOD())
		return FALSE;

	auto doHealSpell = 1;
	if (critterSys.IsCombatModeActive(obj)){
		AiParamPacket aiParams;
		aiParams.GetForCritter(obj);
		if (rngSys.GetInt(1, 100) > aiParams.healSpellChance)
			doHealSpell = 0;
	}

	if (ShouldUseHealSpellOn(obj, doHealSpell))
		return TRUE;

	auto leader = critterSys.GetLeaderForNpc(obj);
	if (leader || (leader = this->leader, leader)){
		if (ShouldUseHealSpellOn(leader, doHealSpell))
			return TRUE;
	}
	else
	{
		leader = obj;
	}

	ObjList objList;
	objList.ListFollowers(leader);

	if (!objList.size())
		return FALSE;

	for (auto i=0; i < objList.size(); i++){
		auto follower = objList.get(i);
		if (follower && follower != obj && ShouldUseHealSpellOn(follower, doHealSpell))
			return TRUE;
	}

	return FALSE;
}

bool AiPacket::ShouldUseHealSpellOn(objHndl handle, BOOL healSpellRecommended){
	target = handle;
	AiSpellList aiSpells;

	if (critterSys.IsDeadNullDestroyed(handle)){
		
		aiSys.GetAiSpells(&aiSpells, obj, AiSpellType::ai_action_resurrect);
		if (aiSys.ChooseRandomSpellFromList(this, &aiSpells))
			return true;
	}

	auto hpPct = critterSys.GetHpPercent(handle);
	if (hpPct > 30 && !healSpellRecommended)
		return false;

	if (hpPct < 40){
		aiSys.GetAiSpells(&aiSpells, obj, AiSpellType::ai_action_heal_heavy);
		if (aiSys.ChooseRandomSpellFromList(this, &aiSpells))
			return true;
	}
	if (hpPct < 55){
		aiSys.GetAiSpells(&aiSpells, obj, AiSpellType::ai_action_heal_medium);
		if (aiSys.ChooseRandomSpellFromList(this, &aiSpells))
			return true;
	}
	if (d20Sys.d20Query(handle, DK_QUE_Critter_Is_Poisoned)){
		aiSys.GetAiSpells(&aiSpells, obj, AiSpellType::ai_action_cure_poison);
		if (aiSys.ChooseRandomSpellFromList(this, &aiSpells))
			return true;
	}
	if (hpPct < 70 || aiFightStatus == AIFS_NONE && hpPct < 90){
		aiSys.GetAiSpells(&aiSpells, obj, AiSpellType::ai_action_heal_light);
		if (aiSys.ChooseRandomSpellFromList(this, &aiSpells))
			return true;
	}

	return false;
}

BOOL AiPacket::LookForEquipment(){

	if (critterSys.IsSleeping(obj))
		return FALSE;
	auto critterFlags = critterSys.GetCritterFlags(obj);
	if (critterFlags & CritterFlag::OCF_FATIGUE_LIMITING)
		return FALSE;
	auto unk = temple::GetRef<int(__cdecl)()>(0x1009A660)();
	if (unk)
		return FALSE;

	auto lookForAndPickupItem = temple::GetRef<BOOL(__cdecl)(objHndl, ObjectListFilter)>(0x1005B1D0);

	auto aiFlags = objSystem->GetObject(obj)->GetInt64(obj_f_npc_ai_flags64);
	if (aiFlags & AiFlag::LookForAmmo){
		aiFlags &= ~(AiFlag::LookForAmmo);
		objSystem->GetObject(obj)->SetInt64(obj_f_npc_ai_flags64, aiFlags);
		if (lookForAndPickupItem(obj, OLC_AMMO))
			return TRUE;
	}
	if (aiFlags & AiFlag::LookForWeapon) {
		aiFlags &= ~(AiFlag::LookForWeapon);
		objSystem->GetObject(obj)->SetInt64(obj_f_npc_ai_flags64, aiFlags);
		if (lookForAndPickupItem(obj, OLC_WEAPON))
			return TRUE;
	}
	if (aiFlags & AiFlag::LookForArmor) {
		aiFlags &= ~(AiFlag::LookForArmor);
		objSystem->GetObject(obj)->SetInt64(obj_f_npc_ai_flags64, aiFlags);
		if (lookForAndPickupItem(obj, OLC_ARMOR))
			return TRUE;
	}


	return FALSE;
}

void AiPacket::FightStatusUpdate(){
	if (critterSys.IsSleeping(obj))
		return;
	objHndl focus = objHndl::null;
	switch (aiFightStatus){

	case AIFS_FINDING_HELP: // for AI with scout points
		focus = ConsiderCombatFocus();
		if (focus){
			this->target = focus;
			if (!ScoutPointSetState()){
				this->aiFightStatus = aiSys.UpdateAiFlags(obj, AIFS_FIGHTING, target, &this->soundMap);
				return;
			}
			break;
		}
		// slide into next case if focus == null
	case AIFS_NONE:
		focus = aiSys.FindSuitableTarget(obj);
		if (focus){
			this->target = focus;
			if (HasScoutStandpoint()){
				ScoutPointSetState();
				this->aiFightStatus = aiSys.UpdateAiFlags(obj, AIFS_FIGHTING, target, &this->soundMap);
				return;
			}
			ChooseRandomSpell_RegardInvulnerableStatus();
			// Temple+: added if condition
			// Otherwise NPCs with faction issues triggered each other
			if (combatSys.isCombatActive() || party.IsInParty(focus)) {
				logger->debug("FightStatusUpdate: {} AIFS_NONE -> AIFS_FIGHTING, FindSuitableTarget -> {}", obj, focus);
				this->aiFightStatus = aiSys.UpdateAiFlags(obj, AIFS_FIGHTING, target, &this->soundMap);
			}
			break;
		}
		break;
	case AIFS_FIGHTING:
		focus = ConsiderCombatFocus();
		if (focus){
			this->target = focus;
			ChooseRandomSpell_RegardInvulnerableStatus();
			this->aiFightStatus = aiSys.UpdateAiFlags(obj, AIFS_FIGHTING, target, &this->soundMap);
			return;
		}
		focus = objSystem->GetObject(obj)->GetObjHndl(obj_f_npc_who_hit_me_last);
		this->target = focus;
		if (aiSys.ConsiderTarget(obj, focus)){
			ChooseRandomSpell_RegardInvulnerableStatus();
			this->aiFightStatus = aiSys.UpdateAiFlags(obj, AIFS_FIGHTING, target, &this->soundMap);
			return;
		} 
		
		focus = PickRandomFromAiList();
		this->target = focus;
		if (!focus){
			logger->debug("FightStatusUpdate: {} AIFS_FIGHTING -> AIFS_NONE (no combat focus)", obj);
			this->aiFightStatus = aiSys.UpdateAiFlags(obj, AIFS_NONE, objHndl::null, &this->soundMap);
			return;
		}
		ChooseRandomSpell_RegardInvulnerableStatus();
		this->aiFightStatus = aiSys.UpdateAiFlags(obj, AIFS_FIGHTING, target, &this->soundMap);
		return;
	case AIFS_FLEEING:
		if (!this->target)
			this->aiFightStatus = aiSys.UpdateAiFlags(obj, AIFS_NONE, objHndl::null, &this->soundMap);
		break;
	case AIFS_SURRENDERED:
		if (critterSys.GetHpPercent(obj) >= 80 && rngSys.GetInt(1,500) == 1){
			this->aiFightStatus = aiSys.UpdateAiFlags(obj, AIFS_NONE, objHndl::null, &this->soundMap);
		}
		break;
	default:
		return;
	}
}

bool AiPacket::HasScoutStandpoint(){
	auto standPt = critterSys.GetStandPoint(obj, StandPointType::Scout);
	return standPt.location.location.locx != 0 || standPt.location.location.locy != 0;
}

bool AiPacket::ScoutPointSetState(){

	if (!HasScoutStandpoint())
		return false;

	auto standPt = critterSys.GetStandPoint(obj, StandPointType::Scout);
	auto objLoc = objSystem->GetObject(obj)->GetLocationFull();
	if (locSys.GetTileDeltaMaxBtwnLocs(objLoc.location, standPt.location.location) <= 3){
		this->aiState2 = 5;
		return true;
	}
	this->aiState2 = 4;
	return true;
}

void AiPacket::ChooseRandomSpell_RegardInvulnerableStatus(){
	if (config.disableChooseRandomSpell_RegardInvulnerableStatus) {
		this->aiState2 = 0;
		return;
	}
	auto objBody = objSystem->GetObject(obj);
	if (objBody->GetFlags() & OF_INVULNERABLE){
		this->aiState2 = 0;
		return;
	}

	auto critFlags2 = objBody->GetInt32(obj_f_critter_flags2);
	if (critFlags2 & CritterFlags2::OCF2_NIGH_INVULNERABLE){
		this->aiState2 = 0;
		return;
	}

	if (!aiSys.ChooseRandomSpell(this)){
		this->aiState2 = 0;
		return;
	}

}

objHndl AiPacket::PickRandomFromAiList(){
	aiSys.AiListRemove(obj, objHndl::null, 0);
	
	auto objBody = objSystem->GetObject(obj);
	auto aiListCount = objBody->GetObjectIdArray(obj_f_npc_ai_list_idx).GetSize();

	auto randIdx = rngSys.GetInt(0, aiListCount - 1);
	auto result = objHndl::null;

	for (auto i = 0u; i < aiListCount; i++, randIdx++){
		auto aiListItem = objBody->GetObjHndl(obj_f_npc_ai_list_idx, (randIdx )% aiListCount);
		auto aiListItemType = objBody->GetInt32(obj_f_npc_ai_list_type_idx, (randIdx) % aiListCount);
		if (aiListItemType == 0){
			if (aiSys.ConsiderTarget(obj, aiListItem)){
				return aiListItem;
			}
		}
	}

	return result;

	//auto result = temple::GetRef<objHndl(__cdecl)(objHndl)>(0x1005D620)(obj);
	//return result;
}

objHndl AiPacket::ConsiderCombatFocus(){

	auto objBody = objSystem->GetObject(obj);
	auto curCombatFocus = objBody->GetObjHndl(obj_f_npc_combat_focus);
	if (aiSys.ConsiderTarget(obj, curCombatFocus)){
		return curCombatFocus;
	}
	
	aiSys.TargetLockUnset(obj);
	return aiSys.FindSuitableTarget(obj);

	//auto considerCombatFocs = temple::GetRef < objHndl(__cdecl)(objHndl) >(0x1005D580);
	//return considerCombatFocs(obj);
	
}

void AiPacket::ProcessCombat(){
	auto isCombat = combatSys.isCombatActive();
	auto curActor = tbSys.turnBasedGetCurrentActor();
	auto nextSimuls = actSeqSys.getNextSimulsPerformer();

	if (!isCombat || curActor == obj || nextSimuls == obj) {
		auto objBod = objSystem->GetObject(obj);
		if (objBod->GetNPCFlags() & ONF_BACKING_OFF){
			ProcessBackingOff();
		}
		else{
			auto aiState2 = this->aiState2;
			switch (aiState2){
			case 1:
				ThrowSpell();
				break;
			case 2:
				UseItem();
				break;
			case 3:
				gameSystems->GetAnim().PushUseSkillOn(obj, target, this->skillEnum, this->scratchObj, 0);
				break;
			case 4:
				MoveToScoutPoint();
				break;
			case 5:
				ScoutPointAttack();
				break;
			default:
				switch(this->aiFightStatus){
				case AIFS_NONE:
					DoWaypoints();
					break;
				case AIFS_FLEEING:
					temple::GetRef<void(__cdecl)(objHndl, objHndl)>(0x1005A1F0)(obj, target); // process fleeing
					break;
				case AIFS_FIGHTING:
					ProcessFighting();
					break;
				case AIFS_SURRENDERED:
					FleeingStatusRefresh();
					break;
				default:
					break;
				}
			}
		}
	}

	if (!combatSys.isCombatActive())
		return;
	auto curSeq = *(actSeqSys.actSeqCur);
	if (!curSeq){
		logger->error("NULL STATUS???");
		return;
	}
	curActor = tbSys.turnBasedGetCurrentActor();
	if (curActor != obj || actSeqSys.isPerforming(obj))
		return;
	if (!actSeqSys.IsSimulsCompleted())
		return;
	if (!actSeqSys.IsLastSimultPopped(obj)){
		logger->debug("AI for {} ending turn...", obj);
		combatSys.CombatAdvanceTurn(obj);
	}
}

void AiPacket::ProcessBackingOff(){
	AiParamPacket aiParams;
	aiParams.GetForCritter(obj);
	auto objBody = objSystem->GetObject(obj);
	auto npcFlags = objBody->GetNPCFlags();
	if (!(npcFlags & ONF_BACKING_OFF))
		return;
	if (!target)
		return;
	auto minDistFeet = 12.0f;
	if (aiParams.combatMinDistanceFeet > 1){
		minDistFeet = (float)aiParams.combatMinDistanceFeet;
	}
		

	if (locSys.DistanceToObj(obj, target) >= minDistFeet){
		objBody->SetInt32(obj_f_npc_flags, npcFlags & ~(ONF_BACKING_OFF));
		return;
	}

	AnimPathSpec animPathSpec;
	animPathSpec.handle = obj;
	animPathSpec.srcLoc = objBody->GetLocation();
	animPathSpec.flags = 0x800;
	animPathSpec.size = 100;
	animPathSpec.distTiles = (int)( minDistFeet * 0.4246407);
	int8_t deltas[100];
	animPathSpec.deltas = deltas;
	animPathSpec.destLoc = objSystem->GetObject(target)->GetLocation();
	if (!animPathSpec.PathSearch()){
		objBody->SetInt32(obj_f_npc_flags, npcFlags & ~(ONF_BACKING_OFF));
		return;
	}

	if (actSeqSys.TurnBasedStatusInit(obj)) {
		actSeqSys.curSeqReset(obj);
		d20Sys.GlobD20ActnInit();
		d20Sys.GlobD20ActnSetTypeAndData1(D20ActionType::D20A_UNSPECIFIED_MOVE, 0);
		d20Sys.GlobD20ActnSetTarget(obj, nullptr); // bug???? todo
		actSeqSys.ActionAddToSeq();
		actSeqSys.sequencePerform();
	}
}

void AiPacket::ThrowSpell(){
	if (combatSys.isCombatActive()){
		if (actSeqSys.TurnBasedStatusInit(obj)){
			actSeqSys.curSeqReset(obj);
			aiSys.AiStrategDefaultCast(obj, target, &this->spellData, &this->spellPktBod);
		}
		return;
	}

	if (!spellSys.IsSpellHarmful(this->spellData.spellEnumOrg, obj, target))
		return;


	combatSys.enterCombat(obj);

	if (!party.IsInParty(obj) && !party.IsInParty(target))
		return;
		
	combatSys.enterCombat(target);
	if (critterSys.CanSense(target, obj)) {
		combatSys.StartCombat(obj, 0);
	}
	else{
		combatSys.StartCombat(obj, 1);
	}
}

void AiPacket::UseItem(){
	if (!this->spellEnum)
		return;
	if (!actSeqSys.TurnBasedStatusInit(obj))
		return;
	actSeqSys.curSeqReset(obj);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20ActionType::D20A_USE_ITEM, 0);
	d20Sys.GlobD20ActnSetSpellData(&this->spellData);
	actSeqSys.ActionAddToSeq();
	actSeqSys.sequencePerform();
}

void AiPacket::MoveToScoutPoint(){
	auto standPt = critterSys.GetStandPoint(obj, StandPointType::Scout);
	if (!actSeqSys.TurnBasedStatusInit(obj))
		return;
	actSeqSys.curSeqReset(obj);
	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(D20ActionType::D20A_UNSPECIFIED_MOVE, 0);
	d20Sys.GlobD20ActnSetTarget(obj, &standPt.location);
	actSeqSys.ActionAddToSeq();
	actSeqSys.sequencePerform();
}

void AiPacket::ScoutPointAttack(){
	aiSys.ProvokeHostility(target, obj, 3, 0);
	auto objBody = objSystem->GetObject(obj);
	auto aiFlags = objBody->GetInt64(obj_f_npc_ai_flags64);
	objBody->SetInt64(obj_f_npc_ai_flags64, aiFlags & ~(AiFlag::FindingHelp));
	aiSys.UpdateAiFlags(obj, AIFS_FIGHTING, target, &this->soundMap);
}

void AiPacket::DoWaypoints(){
	combatSys.CritterExitCombatMode(obj);
	if (d20Sys.d20Query(obj, DK_QUE_Prone) && !d20Sys.d20Query(obj, DK_QUE_Unconscious)){
		actSeqSys.TurnBasedStatusInit(obj);
		d20Sys.GlobD20ActnInit();
		d20Sys.GlobD20ActnSetTypeAndData1(D20ActionType::D20A_STAND_UP, 0);
		actSeqSys.ActionAddToSeq();
		actSeqSys.sequencePerform();
		return;
	}

	auto objBody = objSystem->GetObject(obj);
	auto npcFlags = objBody->GetNPCFlags();

	if (!this->leader){
		if (npcFlags& ONF_USE_ALERTPOINTS){
			
			if (!temple::GetRef<BOOL(__cdecl)(objHndl, int)>(0x1005BC00)(obj, 0)){
				gameSystems->GetAnim().PushFidget(obj);
				temple::GetRef<void(__cdecl)(objHndl)>(0x10015FD0)(obj);
			}
		}
		else if (!temple::GetRef<BOOL(__cdecl)(objHndl, int)>(0x1005B950)(obj, 0) // waypoint sthg
			&& !temple::GetRef<BOOL(__cdecl)(objHndl, int)>(0x1005BC00)(obj, 0)) // npc wander sthg
		{
			gameSystems->GetAnim().PushFidget(obj);
			temple::GetRef<void(__cdecl)(objHndl)>(0x10015FD0)(obj);
		}
		return;
	}

	if (npcFlags & ONF_AI_WAIT_HERE)
		return;

	if (gameSystems->GetAnim().GetFirstRunSlotId(obj))
		return;

	if (aiSys.RefuseFollowCheck(this->obj, this->leader) && critterSys.RemoveFollower(obj, 0)){
		uiSystems->GetParty().Update();
		npcFlags = objBody->GetNPCFlags();
		objBody->SetInt32(obj_f_npc_flags, npcFlags | ONF_JILTED);
		// looks like there was some commented out code here for playing some sound
	}
	else if (npcFlags & ONF_CHECK_LEADER){
		objBody->SetInt32(obj_f_npc_flags, npcFlags & ~ONF_CHECK_LEADER);
		// looks like there was some commented out code here for playing some sound
	}
}

void AiPacket::ProcessFighting(){
	if (combatSys.isCombatActive()){
		if (actSeqSys.curSeqGetTurnBasedStatus() && actSeqSys.TurnBasedStatusInit(obj)){
			actSeqSys.curSeqReset(obj);
			aiSys.StrategyParse(obj, target);
		}
		return;
	}

	if (party.ObjIsAIFollower(obj) || locSys.DistanceToObj(obj, target) > 75.0)
		return;

	if (!party.IsInParty(obj) && party.IsInParty(target)){
		target = pathfindingSys.CanPathToParty(obj);
		if (!target)
			return;
	}
	combatSys.enterCombat(obj);
	if (party.IsInParty(obj) || party.IsInParty(target)){
		combatSys.enterCombat(target);
		if (critterSys.CanSense(target, obj))
			combatSys.StartCombat(obj, 0);
		else
			combatSys.StartCombat(obj, 1);
	}

}

void AiPacket::FleeingStatusRefresh(){
	auto objBody = objSystem->GetObject(obj);
	auto fleeingFrom = objBody->GetObjHndl(obj_f_critter_fleeing_from);
	if (!fleeingFrom || !objSystem->IsValidHandle(fleeingFrom)
		|| critterSys.IsDeadNullDestroyed(fleeingFrom) || critterSys.IsDeadOrUnconscious(fleeingFrom))
		this->aiFightStatus = aiSys.UpdateAiFlags(obj, AIFS_NONE, objHndl::null, nullptr);
}

/* 
* 0x100E2D70
* adjusts location if friendlies are found
*/ 
void FireballLocationAdj(objHndl caster, XMFLOAT2 pos, XMFLOAT2& posAdj, int flags) {
	auto loc = LocAndOffsets::FromInches(pos);
	
	flags |= 2; // find a friendly
	const float FB_RANGE = 240.0f; // presumed fireball range in inches
	// bug: does not account for metamagicked fireball
	// but then again it's for AI anyway...

	float dist = FB_RANGE;
	objHndl tgt = objHndl::null;
	// first check for friends that are not fireball resistant (Q_AI_Fireball_OK)
	auto getClosestFriend = combatSys.GetClosestEnemy(caster, &loc, &tgt, &dist, flags);
	if (!getClosestFriend || dist >= FB_RANGE) {
		// if not found, try to find ones that ARE fireball resistant
		getClosestFriend = combatSys.GetClosestEnemy(caster, &loc, &tgt, &dist, flags & ~8);
	}

	// if none found or are too far from FB location
	if (!getClosestFriend || dist >= FB_RANGE || !tgt)
	{
		posAdj = pos;
		return;
	}

	// find the direction from friend to current fireball center
	// vector add this to the tgt pos so that target is out of range
	auto tgtPos = objects.GetLocationFull(tgt).ToInches2D();
	auto posV = DirectX::XMLoadFloat2(&pos);
	auto tgtPosV = DirectX::XMLoadFloat2(&tgtPos);
	auto posDiff = DirectX::XMVectorSubtract( posV , tgtPosV);
	posDiff = DirectX::XMVector2Normalize(posDiff);
	posDiff = DirectX::XMVectorScale(posDiff, FB_RANGE + 60.0f);
	posV = DirectX::XMVectorAdd(posDiff, tgtPosV); // new pos
	DirectX::XMStoreFloat2(&posAdj, posV);
	return;
}

float FireballScore(objHndl caster, XMFLOAT2 &pos, SpellPacketBody & pkt, int spellEnum) {
	static auto getFireballScoreOrg = temple::GetRef<float(__cdecl)(objHndl, XMFLOAT2&, SpellPacketBody&, int)>(0x100E2FA0);
	auto result = getFireballScoreOrg(caster, pos, pkt, spellEnum);
	return result;
}

/* 0x100E56C0 */
bool AiTactic::ChooseFireballLocation(LocAndOffsets& locOut)
{
	auto aiTac = this;
	auto performer = aiTac->performer;


	
	float fireballScoreBest = 0.0f;
	auto fireballPosBest = locOut.ToInches2D();
	auto fbPos = fireballPosBest;
	
	auto updateFireballBest = [&]() {
		// Temple+: added LOS check
		if (uiPicker.PickerLosBlocked(performer, LocAndOffsets::FromInches(fbPos))) {
			return;
		}
		auto fbScore = FireballScore(performer, fbPos, aiTac->spellPktBody, aiTac->spellPktBody.spellEnum);
		if (fbScore > fireballScoreBest) {
			fireballScoreBest = fbScore;
			fireballPosBest = fbPos;
		}
	};

	for (auto i = 0; i < combatSys.GetInitiativeListLength(); ++i) {
		auto combatant = combatSys.GetInitiativeListMember(i);
		if (!combatant || critterSys.IsFriendly(performer, combatant))
			continue;
		auto loc = objects.GetLocationFull(combatant);
		auto pos = loc.ToInches2D();
		fbPos = pos;
		updateFireballBest();
		FireballLocationAdj(performer, pos, fbPos, 8);
		updateFireballBest();
	}

	auto getCenterOfMass = temple::GetRef<BOOL(__cdecl)(objHndl, XMFLOAT2&)>(0x100E2EC0);
	if (getCenterOfMass(performer, fbPos)) {
		updateFireballBest();
	}

	FireballLocationAdj(performer, fbPos, fbPos, 8);
	updateFireballBest();


	if (fireballScoreBest <= 0.0f) {
		return false;
	}

	locOut = locOut.FromInches(fireballPosBest);

	return true;
}
