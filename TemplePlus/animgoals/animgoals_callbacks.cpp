
#include "stdafx.h"

#include <infrastructure/meshes.h>

#include "anim_slot.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "python/python_integration_spells.h"
#include "critter.h"
#include "action_sequence.h"
#include "combat.h"
#include "location.h"

static bool ContinueWithAnimation(objHndl handle, AnimSlot &slot, int animHandle, int *eventOut = nullptr) {
	static auto anim_continue_with_animation = temple::GetPointer<BOOL(objHndl objHandle, AnimSlot *runSlot, int animHandle, int *eventOut)>(0x10016530);
	int eventOutDummy = 0;
	if (!eventOut) {
		eventOut = &eventOutDummy;
	}
	return anim_continue_with_animation(handle, &slot, animHandle, eventOut) == 1;
}

// Originally @ 0x1000FF10
int GoalIsAlive(AnimSlot& slot) {
	//logger->debug("GoalStateFunc35");
	if (slot.param1.obj)
	{
		if (!gameSystems->GetObj().GetObject(slot.param1.obj)->IsCritter()
			|| !critterSys.IsDeadNullDestroyed(slot.param1.obj))
			return 1;
	}

	return 0;
}

// Originally @ 0x10010D60
int GoalActionPerform2(AnimSlot& slot) {
	//logger->debug("GSF54 ag_attempt_attack action frame");
	assert(slot.param1.obj);
	assert(slot.param2.obj);

	if (!(gameSystems->GetObj().GetObject(slot.param2.obj)->GetFlags() & (OF_OFF | OF_DESTROYED)))
	{
		actSeqSys.ActionFrameProcess(slot.param1.obj);
	}
	return TRUE;
}

// Originally @ 0x1001C100
int GoalAttackEndTurnIfUnreachable(AnimSlot& slot)
{
	//logger->debug("GoalStateFunc133");
	auto sub_10017BF0 = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl)>(0x10017BF0);
	auto result = sub_10017BF0(slot.param1.obj, slot.param2.obj);
	if (!result)
	{
		if (combatSys.isCombatActive())
		{
			logger->debug("Anim sys for {} ending turn...", description.getDisplayName(slot.param1.obj));
			combatSys.CombatAdvanceTurn(slot.param1.obj);
		}
	}

	return result;
}

// Originally @ 0x1000E270
int GoalIsProne(AnimSlot& slot)
{
	//logger->debug("GoalStateFunc IsCritterProne");
	if (slot.param1.obj) {
		return objects.IsCritterProne(slot.param1.obj);
	}

	logger->debug("Anim Assertion failed: obj != OBJ_HANDLE_NULL");
	return FALSE;
}

// Originally @ 0x1000E250
int GoalIsConcealed(AnimSlot& slot)
{
	//logger->debug("GoalState IsConcealed");
	return (critterSys.IsConcealed(slot.param1.obj));
}

// Originally @ 0x100125F0
int GoalIsRotatedTowardNextPathNode(AnimSlot & slot) {
	if (!slot.pCurrentGoal) {
		slot.pCurrentGoal = &slot.goals[slot.currentGoal];
	}

	if (slot.path.nodeCount <= 0)
		return 1;

	// get the mover's location
	auto obj = gameSystems->GetObj().GetObject(slot.param1.obj);
	auto objLoc = obj->GetLocationFull();
	float objAbsX, objAbsY;
	locSys.GetOverallOffset(objLoc, &objAbsX, &objAbsY);

	// get node loc
	if (slot.path.currentNode > 200 || slot.path.currentNode < 0) {
		logger->info("Anim: Illegal current node detected!");
		return 1;
	}

	auto nodeLoc = slot.path.nodes[slot.path.currentNode];
	float nodeAbsX, nodeAbsY;
	locSys.GetOverallOffset(nodeLoc, &nodeAbsX, &nodeAbsY);

	if (objAbsX == nodeAbsX && objAbsY == nodeAbsY) {
		if (slot.path.currentNode + 1 >= slot.path.nodeCount)
			return 1;
		nodeLoc = slot.path.nodes[slot.path.currentNode + 1];
		locSys.GetOverallOffset(nodeLoc, &nodeAbsX, &nodeAbsY);
	}

	auto &rot = slot.pCurrentGoal->scratchVal2.floatNum;
	rot = (float)(M_PI_2 + M_PI * 0.75 - atan2(nodeAbsY - objAbsY, nodeAbsX - objAbsX));
	if (rot < 0.0) {
		rot += (float)6.2831853;//M_PI * 2;
	}
	if (rot > 6.2831853) {
		rot -= (float) 6.2831853;//M_PI * 2;
	}


	auto objRot = obj->GetFloat(obj_f_rotation);


	if (sin(objRot - rot) > 0.017453292)    // 1 degree
		return 0;

	if (cos(objRot) - cos(rot) > 0.017453292) // in case it's a 180 degrees difference
		return 0;

	return 1;
}

// Originally @ 0x10012C70
int GoalIsSlotFlag10NotSet(AnimSlot& slot)
{ //10012C70
  /*if (slot.pCurrentGoal && slot.pCurrentGoal->goalType != ag_anim_idle) {
  logger->debug("GSF82 for {}, current goal {} ({}). Flags: {:x}, currentState: {:x}", description.getDisplayName(slot.animObj), animGoalTypeNames[slot.pCurrentGoal->goalType], slot.currentGoal, slot.flags, slot.currentState);
  if(slot.pCurrentGoal->goalType == ag_hit_by_weapon)
  {
  int u = 1;
  }
  }*/
  //return (slot.flags & AnimSlotFlag::ASF_UNK5) == 0? TRUE: FALSE;
	return (~(slot.flags >> 4)) & 1;
	//return ~(slot.flags >> 4) & 1;
}

// Originally @ 0x10011880
int GoalPlayGetHitAnim(AnimSlot& slot)
{
	//logger->debug("GSF65");
	if (!slot.param1.obj)
	{
		logger->warn("Error in GSF65");
		return FALSE;
	}
	auto obj = gameSystems->GetObj().GetObject(slot.param1.obj);
	auto locFull = obj->GetLocationFull();
	float worldX, worldY;
	locSys.GetOverallOffset(locFull, &worldX, &worldY);

	auto obj2 = gameSystems->GetObj().GetObject(slot.param2.obj);
	auto loc2 = obj2->GetLocationFull();
	float worldX2, worldY2;
	locSys.GetOverallOffset(loc2, &worldX2, &worldY2);

	auto rot = obj->GetFloat(obj_f_rotation);

	auto newRot = atan2(worldY2 - worldY, worldX2 - worldX) + M_PI * 3 / 4 - rot;
	while (newRot > M_PI * 2) newRot -= M_PI * 2;
	while (newRot < 0) newRot += M_PI * 2;

	auto newRotAdj = newRot - M_PI / 4;

	gfx::WeaponAnim weaponIdParam = gfx::WeaponAnim::FrontHit;
	if (newRotAdj < M_PI / 4)
		weaponIdParam = gfx::WeaponAnim::FrontHit;
	else if (newRotAdj < M_PI * 3 / 4)
		weaponIdParam = gfx::WeaponAnim::LeftHit;
	else if (newRotAdj < M_PI * 5 / 4)
		weaponIdParam = gfx::WeaponAnim::BackHit;
	else if (newRotAdj < M_PI * 7 / 4)
		weaponIdParam = gfx::WeaponAnim::RightHit;

	auto weaponAnimId = critterSys.GetAnimId(slot.param1.obj, weaponIdParam);
	objects.SetAnimId(slot.param1.obj, weaponAnimId);

	return TRUE;

}

// Originally @ 0x1000D150
int GoalIsCurrentPathValid(AnimSlot & slot)
{
	return slot.path.flags & PathFlags::PF_COMPLETE;
}

// Originally @ 0x100185e0
int GoalUnconcealAnimate(AnimSlot &slot) {
	//logger->debug("GSF 106 for {}, goal {}, flags {:x}, currentState {:x}", description.getDisplayName(slot.animObj), animGoalTypeNames[slot.pCurrentGoal->goalType], (uint32_t)slot.flags, (uint32_t)slot.currentState);
	auto obj = slot.param1.obj;
	assert(slot.param1.obj);

	auto aasHandle = objects.GetAnimHandle(obj);
	assert(aasHandle);

	if (objects.getInt32(obj, obj_f_spell_flags) & SpellFlags::SF_10000) {
		//logger->debug("GSF 106: return FALSE due to obj_f_spell_flags 0x10000");
		return FALSE;
	}

	auto animId = aasHandle->GetAnimId();

	if ((!objects.IsCritter(obj) ||
		!(objects.getInt32(obj, obj_f_critter_flags) &
		(OCF_PARALYZED | OCF_STUNNED)) ||
		!animId.IsWeaponAnim() &&
		(animId.GetNormalAnimType() == gfx::NormalAnimType::Death ||
			animId.GetNormalAnimType() == gfx::NormalAnimType::Death2 ||
			animId.GetNormalAnimType() == gfx::NormalAnimType::Death3))) {
		static auto anim_frame_advance_maybe =
			temple::GetPointer<BOOL(objHndl, AnimSlot & runSlot,
				uint32_t animHandle, uint32_t * eventOut)>(
					0x10016530);
		uint32_t eventOut = 0;
		anim_frame_advance_maybe(obj, slot, aasHandle->GetHandle(), &eventOut);

		// This is the ACTION trigger
		if (eventOut & 1) {
			slot.flags |= AnimSlotFlag::ASF_UNK3;
			//logger->debug("GSF 106: Set flag 4, returned TRUE");
			return TRUE;
		}

		// If the animation is a looping animation, it does NOT have a
		// defined end, so we just trigger the end here anyway otherwise
		// this'll loop endlessly
		bool looping = false;
		/*if (animId.IsWeaponAnim() && ( animId.GetWeaponAnim() == gfx::WeaponAnim::Idle || animId.GetWeaponAnim() == gfx::WeaponAnim::CombatIdle)) {*/



		if (animId.IsWeaponAnim() && (animId.GetWeaponAnim() == gfx::WeaponAnim::Idle)) {
			//	// We will continue anyway down below, because the character is idling, so log a message
			if (!(eventOut & 2)) {
				logger->info("Ending wait for animation action/end in goal {}, because the idle animation would never end.",
					slot.pCurrentGoal->goalType);
			}
			looping = true;
		}

		// This is the END trigger
		if (!looping && !(eventOut & 2))
		{
			//logger->debug("GSF 106: eventOut & 2, returned TRUE");
			return TRUE;
		}


		// Clears WaypointDelay flag
		auto gameObj = objSystem->GetObject(obj);
		if (objects.IsNPC(obj)) {
			auto flags = gameObj->GetInt64(obj_f_npc_ai_flags64);
			gameObj->SetInt64(obj_f_npc_ai_flags64, flags & 0xFFFFFFFFFFFFFFFDui64);
		}

		// Clear 0x10 slot flag
		slot.flags &= ~AnimSlotFlag::ASF_UNK5;
	}
	//logger->debug("GSF 106: returned FALSE");
	return FALSE;
}

int GoalStateFunc130(AnimSlot& slot)
{
	//logger->debug("GSF 130");
	int eventOut = 0;
	auto handle = slot.param1.obj;
	if (!handle)
	{
		logger->warn("Missing anim object!");
		return FALSE;
	}

	auto obj = gameSystems->GetObj().GetObject(handle);
	auto aasHandle = objects.GetAnimHandle(handle);
	if (!aasHandle || !aasHandle->GetHandle())
	{
		logger->warn("No aas handle!");
		return FALSE;
	}

	if (obj->GetInt32(obj_f_spell_flags) & SpellFlags::SF_10000)
		return FALSE;

	if (obj->IsCritter())
	{
		if (obj->GetInt32(obj_f_critter_flags) & (OCF_PARALYZED | OCF_STUNNED))
			return FALSE;
	}
	ContinueWithAnimation(handle, slot, aasHandle->GetHandle(), &eventOut);

	if (eventOut & 1)
		slot.flags |= AnimSlotFlag::ASF_UNK3;

	if (eventOut & 2) {
		slot.flags &= ~(AnimSlotFlag::ASF_UNK5);
		return FALSE;
	}
	return TRUE;
}

// Originally @ 0x100107E0
int GoalPickpocketPerform(AnimSlot & slot)
{

	auto tgt = slot.param2.obj;
	auto handle = slot.param1.obj;
	int gotCaught = 0;


	if (!tgt
		|| (objects.GetFlags(handle) & (OF_OFF | OF_DESTROYED))
		|| (objects.GetFlags(tgt) & (OF_OFF | OF_DESTROYED)))
		return FALSE;

	critterSys.Pickpocket(handle, tgt, gotCaught);
	if (!gotCaught) {
		slot.flags |= AnimSlotFlag::ASF_UNK12;
	}
	return TRUE;
}


// Originally @ 0x10012c80
int GoalSlotFlagSet8If4AndNotSetYet(AnimSlot &slot) {
	//logger->debug("GSF83 for {}, current goal {} ({})", description.getDisplayName(slot.animObj), animGoalTypeNames[slot.pCurrentGoal->goalType], slot.currentGoal);
	auto flags = slot.flags;
	if (flags & AnimSlotFlag::ASF_UNK3 && !(flags & AnimSlotFlag::ASF_UNK4)) {
		slot.flags = flags | AnimSlotFlag::ASF_UNK4;
		//logger->debug("GSF83 return TRUE");
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// Originally @ 0x10010520
int GoalIsAnimatingConjuration(AnimSlot &slot) {
	//logger->debug("GSF45");
	auto obj = slot.param1.obj;
	assert(obj);
	auto aasHandle = objects.GetAnimHandle(obj);
	assert(aasHandle);

	auto animId = aasHandle->GetAnimId();
	return animId.IsConjuireAnimation() ? TRUE : FALSE;
}

// Originally @ 0x10010290
int GoalReturnFalse(AnimSlot &slot) {
	//logger->debug("GSF41");
	auto spell = spellsCastRegistry.get(slot.param1.number);
	return FALSE;
}

// Originally @ 0x100102c0
int GoalAttemptSpell(AnimSlot &slot) {
	//logger->debug("GSF42");
	auto obj = slot.param1.obj;
	assert(obj);

	auto spellId = slot.param2.number;
	slot.flags |= 0xCu; // Sets 8 and 4

	if (spellId) {
		actSeqSys.ActionFrameProcess(obj);
		pySpellIntegration.SpellTrigger(spellId, SpellEvent::SpellEffect);

		auto spell = spellsCastRegistry.get(spellId);
		if (spell) {
			auto targetCount = spell->spellPktBody.targetCount;
			bool found = false;
			for (uint32_t i = 0; i < targetCount; i++) {
				if (spell->spellPktBody.targetListHandles[i] ==
					spell->spellPktBody.caster) {
					found = true;
					break;
				}
			}

			if (found) {
				DispIOTurnBasedStatus dispIo;
				dispIo.tbStatus = actSeqSys.curSeqGetTurnBasedStatus();
				dispatch.dispatchTurnBasedStatusInit(spell->spellPktBody.caster,
					&dispIo);
			}
		}
	}

	return TRUE;
}
