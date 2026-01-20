
#include "stdafx.h"

#include <infrastructure/meshes.h>

#include "anim_slot.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/timeevents.h"
#include "gamesystems/objects/objsystem.h"
#include "python/python_integration_spells.h"
#include "critter.h"
#include "action_sequence.h"
#include "combat.h"
#include "location.h"
#include "objlist.h"

using GoalCallback = int(__cdecl *)(AnimSlot&);

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
			logger->debug("Anim sys for {} ending turn...", slot.param1.obj);
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
	return (objects.IsCritter(slot.param1.obj) && critterSys.IsConcealed(slot.param1.obj));
}

// Originally @ 0x100125F0
int GoalIsRotatedTowardNextPathNode(AnimSlot & slot) {

	static auto GoalIsRotatedTowardNextPathNode = temple::GetPointer<int(AnimSlot *slot)>(0x100125f0);
	return GoalIsRotatedTowardNextPathNode(&slot);

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
  logger->debug("GSF82 for {}, current goal {} ({}). Flags: {:x}, currentState: {:x}", slot.animObj, animGoalTypeNames[slot.pCurrentGoal->goalType], slot.currentGoal, slot.flags, slot.currentState);
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
	//logger->debug("GSF 106 for {}, goal {}, flags {:x}, currentState {:x}", slot.animObj, animGoalTypeNames[slot.pCurrentGoal->goalType], (uint32_t)slot.flags, (uint32_t)slot.currentState);
	auto obj = slot.param1.obj;
	assert(slot.param1.obj);

	auto aasHandle = objects.GetAnimHandle(obj);
	assert(aasHandle);

	if (objects.getInt32(obj, obj_f_spell_flags) & SpellFlags::SF_10000) {
		//logger->debug("GSF 106: return FALSE due to obj_f_spell_flags 0x10000");
		return FALSE;
	}

	auto animId = aasHandle->GetAnimId();
	auto normalAnim = animId.GetNormalAnimType();

	if ((!objects.IsCritter(obj) ||
		!(objects.getInt32(obj, obj_f_critter_flags) &
		(OCF_PARALYZED | OCF_STUNNED)) ||
		!animId.IsWeaponAnim() &&
		(normalAnim == gfx::NormalAnimType::Death ||
			normalAnim == gfx::NormalAnimType::Death2 ||
			normalAnim == gfx::NormalAnimType::Death3))) {
		static auto anim_frame_advance_maybe =
			temple::GetPointer<BOOL(objHndl, AnimSlot & runSlot,
				uint32_t animHandle, uint32_t * eventOut)>(
					0x10016530);
		uint32_t eventOut = 0;
		anim_frame_advance_maybe(obj, slot, aasHandle->GetHandle(), &eventOut);

		// This is a special hack for Conjuration conjuring. It seems to have a
		// never-ending 'animation' that always sets the action flag. So instead
		// we'll set the 0x8 flag ourselves simulate a complete animation cycle.
		// The spell casting animgoal will then switch to the casting animation
		// after that time. The actual 'animation' is just standing motionless
		// for some reason, so we don't really need to worry about waiting for a
		// complete cycle
		//
		// We don't actually want to restart the animation, because it seems
		// to stack up the spell particles.
		//
		// Perhaps this is just a bugged animation. If it could be fixed, this
		// hack could be removed.
		if (normalAnim == gfx::NormalAnimType::ConjurationConjuring) {
			slot.flags |= AnimSlotFlag::ASF_UNK4;
		}

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



		if (animId.IsWeaponAnim()) {
			if (animId.GetWeaponAnim() == gfx::WeaponAnim::Idle) {
				//	// We will continue anyway down below, because the character is idling, so log a message
				if (!(eventOut & 2)) {
					logger->info("Ending wait for animation action/end in goal {}, because the idle animation would never end.",
						slot.pCurrentGoal->goalType);
				}
				looping = true;
			}
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
	//logger->debug("GSF83 for {}, current goal {} ({})", slot.animObj, animGoalTypeNames[slot.pCurrentGoal->goalType], slot.currentGoal);
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
	gfx::EncodedAnimId spellAnimId(animId.GetSpellAnimType());
	return spellAnimId.IsCastingAnimation() ? TRUE : FALSE;
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


// Originally @ 0x1000c9c0
int GoalStateCallback1(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000c9c0);
	return org(slot);
}

// Originally @ 0x1000ccf0
int GoalStateCallback3(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000ccf0);
	return org(slot);
}

// Originally @ 0x1000ce10
int GoalSetOffAndDestroyParam1(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000ce10);
	return org(slot);
}

// Originally @ 0x1000ce60
int GoalParam1ObjCloseToParam2Loc(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000ce60);
	return org(slot);
}

// Originally @ 0x1000cf10
int GoalTargetLocWithinRadius(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000cf10);
	return org(slot);
}

// Originally @ 0x1000cfe0
int GoalStateCallback7(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000cfe0);
	return org(slot);
}

// Originally @ 0x1000d060
int GoalStateCallback8(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000d060);
	return org(slot);
}

// Originally @ 0x1000d560
int GoalCalcPathToTarget(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000d560);
	return org(slot);
}

// Originally @ 0x1000db30
int GoalCalcPathToTarget2(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000db30);
	return org(slot);
}

// Originally @ 0x1000dca0
int GoalKnockbackFunc(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000dca0);
	return org(slot);
}

// Originally @ 0x1000dd80
int GoalMoveAwayFromObj(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000dd80);
	return org(slot);
}

// Originally @ 0x1000e2c0
int GoalStunnedExpire(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000e2c0);
	return org(slot);
}

// Originally @ 0x1000e4f0
int GoalHasDoorInPath(AnimSlot &slot) {
	/*static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000e4f0);
	return org(slot);*/

	auto obj = slot.param1.obj;
	Expects(obj);

	auto nextPathPos = slot.path.GetNextNode();
	if (!nextPathPos) {
		return 0;
	}

	// There's no real rhyme or reason to this constant
	auto testHeight = 36.0f;

	using namespace DirectX;
	auto nextPathPosWorld = XMLoadFloat3(&nextPathPos->ToInches3D(testHeight));

	auto currentPos = objects.GetLocationFull(obj);
	auto radius = objects.GetRadius(obj);

	auto currentPosWorld = XMLoadFloat3(&currentPos.ToInches3D(testHeight));

	// Effectively this builds a world position that is one tile ahead of the current
	// position along the critter's trajectory
	auto directionVec = XMVector3Normalize(nextPathPosWorld - currentPosWorld);
	auto testPosVec = currentPosWorld + INCH_PER_TILE * directionVec;
	XMFLOAT3 testPos;
	XMStoreFloat3(&testPos, testPosVec);

	// List every door in a certain radius
	ObjList doorSearch;
	doorSearch.ListRadius(currentPos, radius + 150.0f, OLC_PORTAL);

	for (auto doorObj : doorSearch) {

		auto doorModel = objects.GetAnimHandle(doorObj);
		if (doorModel) {
			auto doorAnimParams = objects.GetAnimParams(doorObj);

			auto distFromDoor = doorModel->GetDistanceToMesh(doorAnimParams, testPos);
			if (distFromDoor < radius) {
				// Store the door we will collide with in the scratchObj slot
				slot.pCurrentGoal->scratch.obj = doorObj;
				return 1;
			}
		}

	}

	return 0;
}

// Originally @ 0x1000e6f0
int GoalFindPathNear(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000e6f0);
	return org(slot);
}

// Originally @ 0x1000e8b0
int GoalFindPathNearObject(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000e8b0);
	return org(slot);
}

// Originally @ 0x1000ec10
int GoalFindPathNearObjectCombat(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000ec10);
	return org(slot);
}

// Originally @ 0x1000efb0
int GoalIsParam1Door(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000efb0);
	return org(slot);
}

// Originally @ 0x1000f000
int GoalPlayDoorLockedSound(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000f000);
	return org(slot);
}

// Originally @ 0x1000f0d0
int GoalIsDoorMagicallyHeld(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000f0d0);
	return org(slot);
}

// Originally @ 0x1000f140
int GoalAttemptOpenDoor(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000f140);
	return org(slot);
}

// Originally @ 0x1000f2c0
int GoalIsDoorLocked(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000f2c0);
	return org(slot);
}

// Originally @ 0x1000f350
int GoalDoorAlwaysFalse(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000f350);
	return org(slot);
}

// Originally @ 0x1000f400
int GoalIsDoorUnlocked(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000f400);
	return org(slot);
}

// Originally @ 0x1000f490
int GoalSetRadiusTo2(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000f490);
	return org(slot);
}

// Originally @ 0x1000f550
int GoalUseObject(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000f550);
	return org(slot);
}

// Originally @ 0x1000f860
int GoalUseItemOnObj(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000f860);
	return org(slot);
}

// Originally @ 0x1000f9a0
int GoalUseItemOnObjWithSkillDummy(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000f9a0);
	return org(slot);
}

// Originally @ 0x1000fbc0
int GoalUseItemOnLoc(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000fbc0);
	return org(slot);
}

// Originally @ 0x1000fce0
int GoalUseItemOnLocWithSkillDummy(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000fce0);
	return org(slot);
}

// Originally @ 0x1000fec0
int GoalSetNoFlee(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000fec0);
	return org(slot);
}

// Originally @ 0x1000ff60
int GoalPlaySoundScratch5(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000ff60);
	return org(slot);
}

// Originally @ 0x1000fff0
int GoalAttemptAttackCheck(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1000fff0);
	return org(slot);
}

// Originally @ 0x10010160
int GoalCritterShouldNotAutoAnimate(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10010160);
	return org(slot);
}

// Originally @ 0x100101d0
int GoalAttackerHasRangedWeapon(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100101d0);
	return org(slot);
}

// Originally @ 0x10010250
int GoalReturnTrue(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10010250);
	return org(slot);
}

// Originally @ 0x10010410
int GoalCastConjureEnd(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10010410);
	return org(slot);
}

// Originally @ 0x100104a0
int GoalDestroyParam1(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100104a0);
	return org(slot);
}

// Originally @ 0x10010500
int GoalWasInterrupted(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10010500);
	return org(slot);
}

// Originally @ 0x100105f0
int GoalStartConjurationAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100105f0);

	auto obj = slot.param1.obj;
	if (!obj) {
		logger->error("GoalStartConjurationAnim: Null object param.");
		gameSystems->GetAnim().Debug();
	}
	auto aasHandle = objects.GetAnimHandle(obj);
	auto aasId = aasHandle ? aasHandle->GetAnimId() : 0;
	if (!aasId) {
		logger->error("GoalStartConjurationAnim: Null AAS handle.");
		gameSystems->GetAnim().Debug();
	}

	auto &aas = gameSystems->GetAAS();
	//auto model = aas.BorrowByHandle(aasId);
	//auto encodedId = model->GetAnimId();
	auto encodedId = aasHandle->GetAnimId();
	
	gfx::EncodedAnimId encodedSpellAnimId(encodedId.GetSpellAnimType());
	if (!encodedSpellAnimId.IsCastingAnimation()) {
		logger->error("GoalStartConjurationAnim: Is not conjuration anim.");
		gameSystems->GetAnim().Debug();
	}
	;
	objects.SetAnimId(obj, encodedId.ConjurationToCastAnimation());
	
	slot.path.someDelay = 33;
	slot.gametimeSth = gameSystems->GetTimeEvent().GetAnimTime();
	slot.flags |= AnimSlotFlag::ASF_UNK5;
	slot.flags &= ~(AnimSlotFlag::ASF_UNK3 | AnimSlotFlag::ASF_UNK4);

	return 1;
	//return org(slot);
}

// Originally @ 0x10010760
int GoalAreOnSameTile(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10010760);
	return org(slot);
}

// Originally @ 0x10010aa0
int GoalActionPerform(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10010aa0);
	return org(slot);
}

// Originally @ 0x10010b50
int GoalCheckSlotFlag40000(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10010b50);
	return org(slot);
}

// Originally @ 0x10010b70
int GoalCheckParam2AgainstStateFlagData(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10010b70);
	return org(slot);
}

// Originally @ 0x10010b90
int GoalPickLock(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10010b90);
	return org(slot);
}

// Originally @ 0x10010cd0
int GoalAttemptTrapDisarm(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10010cd0);
	return org(slot);
}

// Originally @ 0x10010e00
int GoalHasReachWithMainWeapon(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10010e00);
	return org(slot);
}

// Originally @ 0x10010f70
int GoalThrowItem(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10010f70);
	return org(slot);
}

// Originally @ 0x100110a0
int GoalNotPreventedFromTalking(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100110a0);
	return org(slot);
}

// Originally @ 0x100111e0
int GoalIsWithinTalkingDistance(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100111e0);
	return org(slot);
}

// Originally @ 0x100112d0
int GoalInitiateDialog(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100112d0);
	return org(slot);
}

// Originally @ 0x10011370
int GoalOpenDoorCleanup(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10011370);
	return org(slot);
}

// Originally @ 0x10011420
int GoalCloseDoorCleanup(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10011420);
	return org(slot);
}

// Originally @ 0x100114d0
int GoalIsDoorSticky(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100114d0);
	return org(slot);
}

// Originally @ 0x10011530
int GoalIsLiveCritterNear(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10011530);
	return org(slot);
}

// Originally @ 0x10011600
int GoalSetRunningFlag(AnimSlot &slot) {
	// static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10011600);
	Expects(slot.animObj);
	slot.flags |= AnimSlotFlag::ASF_RUNNING;
	return TRUE;
}

// Originally @ 0x10011660
int GoalEnterCombat(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10011660);
	return org(slot);
}

// Originally @ 0x100117f0
int GoalLeaveCombat(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100117f0);
	return org(slot);
}

// Originally @ 0x10011a30
int GoalPlayDodgeAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10011a30);
	return org(slot);
}

// Originally @ 0x10011be0
int GoalPlayAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10011be0);
	return org(slot);
}

// Originally @ 0x10011cf0
int GoalSaveParam1InScratch(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10011cf0);
	return org(slot);
}

// Originally @ 0x10011d20
int GoalSaveStateDataInSkillData(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10011d20);
	return org(slot);
}

// Originally @ 0x10011d40
int GoalSaveStateDataOrSpellRangeInRadius(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10011d40);
	return org(slot);
}

// Originally @ 0x10011dc0
int GoalSetTargetLocFromObj(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10011dc0);
	return org(slot);
}

// Originally @ 0x10011e70
int GoalSetRadiusTo4(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10011e70);
	return org(slot);
}

// Originally @ 0x10011e90
int GoalSetRadiusToAiSpread(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10011e90);
	return org(slot);
}

// Originally @ 0x10011f20
int GoalIsCloserThanDesiredSpread(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10011f20);
	return org(slot);
}

// Originally @ 0x10012040
int GoalTurnTowardsOrAway(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10012040);
	return org(slot);
}

// Originally @ 0x100121b0
int GoalPlayRotationAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100121b0);
	return org(slot);
}

// Originally @ 0x100122a0
int GoalRotate(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100122a0);
	return org(slot);
}

// Originally @ 0x100127b0
int GoalIsRotatedTowardTarget(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100127b0);
	return org(slot);
}

// Originally @ 0x10012910
int GoalSetRotationToParam2(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10012910);
	return org(slot);
}

// Originally @ 0x10012a00
int GoalSetRotationToFaceTargetObj(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10012a00);
	return org(slot);
}

// Originally @ 0x10012b60
int GoalSetRotationToFaceTargetLoc(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10012b60);
	return org(slot);
}

// Originally @ 0x10012ca0
int GoalProjectileCleanup(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10012ca0);
	return org(slot);
}

// Originally @ 0x10012cf0
int GoalAnimateCleanup(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10012cf0);
	return org(slot);
}

// Originally @ 0x10012d10
int GoalAnimateForever(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10012d10);
	return org(slot);
}

// Originally @ 0x10012fa0
int GoalLoopWhileCloseToParty(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10012fa0);
	return org(slot);
}

// Originally @ 0x10013080
int GoalFreeSoundHandle(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10013080);
	return org(slot);
}

// Originally @ 0x100130f0
int GoalIsAliveAndConscious(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100130f0);
	return org(slot);
}

// Originally @ 0x10013250
int GoalBeginMoveStraight(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10013250);
	return org(slot);
}

// Originally @ 0x10013af0
int GoalUpdateMoveStraight(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10013af0);
	return org(slot);
}

// Originally @ 0x100140c0
int GoalSetNoBlockIfNotInParty(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100140c0);
	return org(slot);
}

// Originally @ 0x10014170
int GoalDyingCleanup(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10014170);
	return org(slot);
}

// Originally @ 0x100147d0
int GoalMoveAlongPath(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100147d0);
	return org(slot);
}

// Originally @ 0x10014f10
int GoalIsNotStackFlagsData20(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10014f10);
	return org(slot);
}

// Originally @ 0x10014f30
int GoalJiggleAlongYAxis(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10014f30);
	return org(slot);
}

// Originally @ 0x10014ff0
int GoalStartJigglingAlongYAxis(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10014ff0);
	return org(slot);
}

// Originally @ 0x100150a0
int GoalEndJigglingAlongYAxis(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100150a0);
	return org(slot);
}

// Originally @ 0x10015150
int GoalIsNotStackFlagsData40(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10015150);
	return org(slot);
}

// Originally @ 0x10015170
int GoalSetSlotFlags4(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10015170);
	return org(slot);
}

// Originally @ 0x10015240
int GoalActionPerform3(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10015240);
	return org(slot);
}

// Originally @ 0x10017100
int GoalSpawnFireball(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10017100);
	return org(slot);
}

// Originally @ 0x10017170
int GoalPleaseMove(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10017170);
	return org(slot);
}

// Originally @ 0x10017460
int GoalIsTargetWithinRadius(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10017460);
	return org(slot);
}

// Originally @ 0x10017570
int GoalWander(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10017570);
	return org(slot);
}

// Originally @ 0x10017810
int GoalWanderSeekDarkness(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10017810);
	return org(slot);
}

// Originally @ 0x10017b30
int GoalIsDoorFullyClosed(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10017b30);
	return org(slot);
}

// Originally @ 0x10017dd0
int GoalTriggerSpell(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10017dd0);
	return org(slot);
}

// Originally @ 0x10017f80
int GoalUnconcealCleanup(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10017f80);
	return org(slot);
}

// Originally @ 0x10018050
int GoalResetToIdleAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10018050);
	return org(slot);
}

// Originally @ 0x10018160
int GoalResetToIdleAnimUnstun(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10018160);
	return org(slot);
}

// Originally @ 0x10018290
int GoalThrowItemCleanup(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10018290);
	return org(slot);
}

// Originally @ 0x10018400
int GoalThrowItemPlayAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10018400);
	return org(slot);
}

// Originally @ 0x10018730
int GoalStartIdleAnimIfCloseToParty(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10018730);
	return org(slot);
}

// Originally @ 0x10018810
int GoalStartFidgetAnimIfCloseToParty(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10018810);
	return org(slot);
}

// Originally @ 0x100188f0
int GoalContinueWithAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100188f0);
	return org(slot);
}

// Originally @ 0x100189b0
int GoalContinueWithAnim2(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100189b0);
	return org(slot);
}

// Originally @ 0x10018a70
int GoalPlayDoorOpenAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10018a70);
	return org(slot);
}

// Originally @ 0x10018b90
int GoalContinueWithDoorOpenAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10018b90);
	return org(slot);
}

// Originally @ 0x10018c50
int GoalPlayDoorCloseAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10018c50);
	return org(slot);
}

// Originally @ 0x10018d40
int GoalContinueWithDoorCloseAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10018d40);
	return org(slot);
}

// Originally @ 0x10018e00
int GoalPickLockPlayPushDoorOpenAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10018e00);
	return org(slot);
}

// Originally @ 0x10018ee0
int GoalPickLockContinueWithAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10018ee0);
	return org(slot);
}

// Originally @ 0x10018fa0
int GoalDyingPlaySoundAndRipples(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10018fa0);
	return org(slot);
}

// Originally @ 0x10019070
int GoalDyingContinueAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10019070);
	return org(slot);
}

// Originally @ 0x10019130
int GoalAnimateFireDmgContinueAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10019130);
	return org(slot);
}

// Originally @ 0x100191f0
int GoalStunnedPlayAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100191f0);
	return org(slot);
}

// Originally @ 0x10019330
int GoalStunnedContinueAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10019330);
	return org(slot);
}

// Originally @ 0x10019470
int GoalPlayGetUpAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10019470);
	return org(slot);
}

// Originally @ 0x10019540
int GoalPlayUnconcealAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10019540);
	return org(slot);
}

// Originally @ 0x10019630
int GoalPlayMoveAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10019630);
	return org(slot);
}

// Originally @ 0x10019920
int GoalPlayWaterRipples(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10019920);
	return org(slot);
}

// Originally @ 0x100199b0
int GoalContinueMoveStraight(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x100199b0);
	return org(slot);
}

// Originally @ 0x10019c20
int GoalApplyKnockback(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10019c20);
	return org(slot);
}

// Originally @ 0x10019e10
int GoalDyingReturnTrue(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10019e10);
	return org(slot);
}

// Originally @ 0x10019e70
int GoalAttemptMoveCleanup(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10019e70);
	return org(slot);
}

// Originally @ 0x10019f00
int GoalAttackPlayWeaponHitEffect(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10019f00);
	return org(slot);
}

// Originally @ 0x1001a080
int GoalAttackContinueWithAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1001a080);
	return org(slot);
}

// Originally @ 0x1001a170
int GoalAttackPlayIdleAnim(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1001a170);
	return org(slot);
}

// Originally @ 0x1001bf70
int GoalMoveNearUpdateRadiusToReach(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x1001bf70);
	return org(slot);
}

// Originally @ 0x101f5850
int AlwaysSucceed(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x101f5850);
	return org(slot);
}

// Originally @ 0x10262530
int AlwaysFail(AnimSlot &slot) {
	static auto org = temple::GetRef<std::remove_pointer<GoalCallback>::type>(0x10262530);
	return org(slot);
}
 
int GoalTestAction(AnimSlot &slot) {
	return (slot.flags & AnimSlotFlag::ASF_ACTION) ? TRUE : FALSE;
}

int GoalTestCompletedOnce(AnimSlot &slot) {
	return (slot.flags & AnimSlotFlag::ASF_UNK4) ? TRUE : FALSE;
}

// tests whether the flag indicating an animation in progress is set
int GoalTestAnimating(AnimSlot &slot) {
	return (slot.flags & AnimSlotFlag::ASF_ANIMATING) ? TRUE : FALSE;
}

int GoalUnsetAction(AnimSlot &slot) {
	slot.flags &= ~(AnimSlotFlag::ASF_ACTION);
	return 1;
}

int GoalTestAnimatingCasting(AnimSlot &slot) {
	auto obj = slot.param1.obj;

	if (!obj) {
		logger->error("GoalBeginCastingAnim: null object param");
		gameSystems->GetAnim().Debug();
		return FALSE;
	}

	auto aasHandle = objects.GetAnimHandle(obj);

	if (!aasHandle) {
		logger->error("GoalBeginCastingAnim: null AAS handle");
		gameSystems->GetAnim().Debug();
		return FALSE;
	}

	auto animId = aasHandle->GetAnimId();

	gfx::EncodedAnimId spellAnimId(animId.GetSpellAnimType());
	return spellAnimId.IsCastingAnimation() ? TRUE : FALSE;
}

// Sets flag 0x8 indicating completed conjuration animation
// Yields TRUE when completing conjuration, and FALSE when completing
// casting.
int GoalSpellAnimCompleted(AnimSlot &slot) {
	slot.flags |= AnimSlotFlag::ASF_UNK4;

	return !GoalTestAnimatingCasting(slot);
}

int GoalBeginSpellAnim(AnimSlot &slot) {
	auto obj = slot.param1.obj;
	auto prevId = slot.pCurrentGoal->animIdPrevious.number;

	bool animating = slot.flags & AnimSlotFlag::ASF_ANIMATING;
	bool repeated = slot.flags & AnimSlotFlag::ASF_UNK4;

	// if this is the first time through, don't do anything
	if (animating && !repeated) return TRUE;

	bool action = slot.flags & AnimSlotFlag::ASF_ACTION;
	bool cast = action && repeated;

	if (!obj) {
		logger->error("GoalBeginCastingAnim: null object param");
		gameSystems->GetAnim().Debug();
		return FALSE;
	}

	auto aasHandle = objects.GetAnimHandle(obj);
	auto aasId = aasHandle ? aasHandle->GetAnimId() : 0;
	if (!aasId) {
		logger->error("GoalBeginCastingAnim: null AAS handle");
		gameSystems->GetAnim().Debug();
		return FALSE;
	}

	auto &aas = gameSystems->GetAAS();
	gfx::EncodedAnimId encodedId(prevId - (cast ? 65 : 64));

	objects.SetAnimId(obj, encodedId);

	static auto PlayRipples = temple::GetRef<char(__cdecl)(objHndl)>(0x100166F0);
	PlayRipples(obj);
	slot.path.someDelay = 33;
	slot.gametimeSth = gameSystems->GetTimeEvent().GetAnimTime();
	slot.flags &= ~(AnimSlotFlag::ASF_ACTION);
	slot.flags |= AnimSlotFlag::ASF_ANIMATING;

	return TRUE;
}
