
#include "stdafx.h"

#include <set>

#include "util/fixes.h"
#include <infrastructure/stringutil.h>

#include "anim.h"
#include "animgoals_stackentry.h"
#include "obj.h"
#include "gamesystems/gamesystems.h"

static int HandleOptionalSlotResult(std::optional<AnimSlotId> slotId, AnimSlotId *slotIdOut) {
	if (slotId) {
		if (slotIdOut) {
			*slotIdOut = *slotId;
		}
		return 1;
	}
	else {
		if (slotIdOut) {
			slotIdOut->Clear();
		}
		return 0;
	}
}

static const char *AnimGoalDataNames[] = {
	"AGDATA_SELF_OBJ",     "AGDATA_TARGET_OBJ",   "AGDATA_BLOCK_OBJ",
	"AGDATA_SCRATCH_OBJ",  "AGDATA_PARENT_OBJ",   "AGDATA_TARGET_TILE",
	"AGDATA_RANGE_DATA",   "AGDATA_ANIM_ID",      "AGDATA_ANIM_ID_PREV",
	"AGDATA_ANIM_DATA",    "AGDATA_SPELL_DATA",   "AGDATA_SKILL_DATA",
	"AGDATA_FLAGS_DATA",   "AGDATA_SCRATCH_VAL1", "AGDATA_SCRATCH_VAL2",
	"AGDATA_SCRATCH_VAL3", "AGDATA_SCRATCH_VAL4", "AGDATA_SCRATCH_VAL5",
	"AGDATA_SCRATCH_VAL6", "AGDATA_SOUND_HANDLE" };

static class AnimGoalsHooks : public TempleFix {
public:

	std::map<uint32_t, std::string_view> goalfunc_names;

	AnimGoalsHooks() {

		goalfunc_names[0x1000c9c0] = "GoalStateCallback1"sv;
		goalfunc_names[0x1000ccf0] = "GoalStateCallback3"sv; // Always 1
		goalfunc_names[0x1000ce10] = "GoalSetOffAndDestroyParam1"sv;
		goalfunc_names[0x1000ce60] = "GoalParam1ObjCloseToParam2Loc"sv;
		goalfunc_names[0x1000cf10] = "GoalTargetLocWithinRadius"sv;
		goalfunc_names[0x1000cfe0] = "GoalStateCallback7"sv;
		goalfunc_names[0x1000d060] = "GoalStateCallback8"sv;
		goalfunc_names[0x1000d150] = "GoalIsCurrentPathValid"sv;
		goalfunc_names[0x1000d560] = "GoalCalcPathToTarget"sv;
		goalfunc_names[0x1000db30] = "GoalCalcPathToTarget2"sv;
		goalfunc_names[0x1000dca0] = "GoalKnockbackFunc"sv;
		goalfunc_names[0x1000dd80] = "GoalMoveAwayFromObj"sv;
		goalfunc_names[0x1000e250] = "GoalIsConcealed"sv;
		goalfunc_names[0x1000e270] = "GoalIsProne"sv;
		goalfunc_names[0x1000e2c0] = "GoalStunnedExpire"sv;
		goalfunc_names[0x1000e4f0] = "GoalHasDoorInPath"sv;
		goalfunc_names[0x1000e6f0] = "GoalFindPathNear"sv;
		goalfunc_names[0x1000e8b0] = "GoalFindPathNearObject"sv;
		goalfunc_names[0x1000ec10] = "GoalFindPathNearObjectCombat"sv;
		goalfunc_names[0x1000efb0] = "GoalIsParam1Door"sv;
		goalfunc_names[0x1000f000] = "GoalPlayDoorLockedSound"sv;
		goalfunc_names[0x1000f0d0] = "GoalIsDoorMagicallyHeld"sv;
		goalfunc_names[0x1000f140] = "GoalAttemptOpenDoor"sv;
		goalfunc_names[0x1000f2c0] = "GoalIsDoorLocked"sv;
		goalfunc_names[0x1000f350] = "GoalDoorAlwaysFalse"sv;
		goalfunc_names[0x1000f400] = "GoalIsDoorUnlocked"sv;
		goalfunc_names[0x1000f490] = "GoalSetRadiusTo2"sv;
		goalfunc_names[0x1000f550] = "GoalUseObject"sv;
		goalfunc_names[0x1000f860] = "GoalUseItemOnObj"sv;
		goalfunc_names[0x1000f9a0] = "GoalUseItemOnObjWithSkillDummy"sv;
		goalfunc_names[0x1000fbc0] = "GoalUseItemOnLoc"sv;
		goalfunc_names[0x1000fce0] = "GoalUseItemOnLocWithSkillDummy"sv;
		goalfunc_names[0x1000fec0] = "GoalSetNoFlee"sv;
		goalfunc_names[0x1000ff10] = "GoalIsAlive"sv;
		goalfunc_names[0x1000ff60] = "GoalPlaySoundScratch5"sv;
		goalfunc_names[0x1000fff0] = "GoalAttemptAttackCheck"sv;
		goalfunc_names[0x10010160] = "GoalCritterShouldNotAutoAnimate"sv;
		goalfunc_names[0x100101d0] = "GoalAttackerHasRangedWeapon"sv;
		goalfunc_names[0x10010250] = "GoalReturnTrue"sv;
		goalfunc_names[0x10010290] = "GoalReturnFalse"sv;
		goalfunc_names[0x100102c0] = "GoalAttemptSpell"sv;
		goalfunc_names[0x10010410] = "GoalCastConjureEnd"sv;
		goalfunc_names[0x100104a0] = "GoalDestroyParam1"sv;
		goalfunc_names[0x10010500] = "GoalWasInterrupted"sv;
		goalfunc_names[0x10010520] = "GoalIsAnimatingConjuration"sv;
		goalfunc_names[0x100105f0] = "GoalStartConjurationAnim"sv;
		goalfunc_names[0x10010760] = "GoalAreOnSameTile"sv;
		goalfunc_names[0x100107e0] = "GoalPickpocketPerform"sv;
		goalfunc_names[0x10010aa0] = "GoalActionPerform"sv;
		goalfunc_names[0x10010b50] = "GoalCheckSlotFlag40000"sv;
		goalfunc_names[0x10010b70] = "GoalCheckParam2AgainstStateFlagData"sv;
		goalfunc_names[0x10010b90] = "GoalPickLock"sv;
		goalfunc_names[0x10010cd0] = "GoalAttemptTrapDisarm"sv;
		goalfunc_names[0x10010d60] = "GoalActionPerform2"sv;
		goalfunc_names[0x10010e00] = "GoalHasReachWithMainWeapon"sv;
		goalfunc_names[0x10010f70] = "GoalThrowItem"sv;
		goalfunc_names[0x100110a0] = "GoalNotPreventedFromTalking"sv;
		goalfunc_names[0x100111e0] = "GoalIsWithinTalkingDistance"sv;
		goalfunc_names[0x100112d0] = "GoalInitiateDialog"sv;
		goalfunc_names[0x10011370] = "GoalOpenDoorCleanup"sv;
		goalfunc_names[0x10011420] = "GoalCloseDoorCleanup"sv;
		goalfunc_names[0x100114d0] = "GoalIsDoorSticky"sv;
		goalfunc_names[0x10011530] = "GoalIsLiveCritterNear"sv;
		goalfunc_names[0x10011600] = "GoalSetRunningFlag"sv;
		goalfunc_names[0x10011660] = "GoalEnterCombat"sv;
		goalfunc_names[0x100117f0] = "GoalLeaveCombat"sv;
		goalfunc_names[0x10011880] = "GoalPlayGetHitAnim"sv;
		goalfunc_names[0x10011a30] = "GoalPlayDodgeAnim"sv;
		goalfunc_names[0x10011be0] = "GoalPlayAnim"sv;
		goalfunc_names[0x10011cf0] = "GoalSaveParam1InScratch"sv;
		goalfunc_names[0x10011d20] = "GoalSaveStateDataInSkillData"sv;
		goalfunc_names[0x10011d40] = "GoalSaveStateDataOrSpellRangeInRadius"sv;
		goalfunc_names[0x10011dc0] = "GoalSetTargetLocFromObj"sv;
		goalfunc_names[0x10011e70] = "GoalSetRadiusTo4"sv;
		goalfunc_names[0x10011e90] = "GoalSetRadiusToAiSpread"sv;
		goalfunc_names[0x10011f20] = "GoalIsCloserThanDesiredSpread"sv;
		goalfunc_names[0x10012040] = "GoalTurnTowardsOrAway"sv;
		goalfunc_names[0x100121b0] = "GoalPlayRotationAnim"sv;
		goalfunc_names[0x100122a0] = "GoalRotate"sv;
		goalfunc_names[0x100125f0] = "GoalIsRotatedTowardNextPathNode"sv;
		goalfunc_names[0x100127b0] = "GoalIsRotatedTowardTarget"sv;
		goalfunc_names[0x10012910] = "GoalSetRotationToParam2"sv;
		goalfunc_names[0x10012a00] = "GoalSetRotationToFaceTargetObj"sv;
		goalfunc_names[0x10012b60] = "GoalSetRotationToFaceTargetLoc"sv;
		goalfunc_names[0x10012c70] = "GoalIsSlotFlag10NotSet"sv;
		goalfunc_names[0x10012c80] = "GoalSlotFlagSet8If4AndNotSetYet"sv;
		goalfunc_names[0x10012ca0] = "GoalProjectileCleanup"sv;
		goalfunc_names[0x10012cf0] = "GoalAnimateCleanup"sv;
		goalfunc_names[0x10012d10] = "GoalAnimateForever"sv;
		goalfunc_names[0x10012fa0] = "GoalLoopWhileCloseToParty"sv;
		goalfunc_names[0x10013080] = "GoalFreeSoundHandle"sv;
		goalfunc_names[0x100130f0] = "GoalIsAliveAndConscious"sv;
		goalfunc_names[0x10013250] = "GoalBeginMoveStraight"sv;
		goalfunc_names[0x10013af0] = "GoalUpdateMoveStraight"sv;
		goalfunc_names[0x100140c0] = "GoalSetNoBlockIfNotInParty"sv;
		goalfunc_names[0x10014170] = "GoalDyingCleanup"sv;
		goalfunc_names[0x100147d0] = "GoalMoveAlongPath"sv;
		goalfunc_names[0x10014f10] = "GoalIsNotStackFlagsData20"sv;
		goalfunc_names[0x10014f30] = "GoalJiggleAlongYAxis"sv;
		goalfunc_names[0x10014ff0] = "GoalStartJigglingAlongYAxis"sv;
		goalfunc_names[0x100150a0] = "GoalEndJigglingAlongYAxis"sv;
		goalfunc_names[0x10015150] = "GoalIsNotStackFlagsData40"sv;
		goalfunc_names[0x10015170] = "GoalSetSlotFlags4"sv;
		goalfunc_names[0x10015240] = "GoalActionPerform3"sv;
		goalfunc_names[0x10017100] = "GoalSpawnFireball"sv;
		goalfunc_names[0x10017170] = "GoalPleaseMove"sv;
		goalfunc_names[0x10017460] = "GoalIsTargetWithinRadius"sv;
		goalfunc_names[0x10017570] = "GoalWander"sv;
		goalfunc_names[0x10017810] = "GoalWanderSeekDarkness"sv;
		goalfunc_names[0x10017b30] = "GoalIsDoorFullyClosed"sv;
		goalfunc_names[0x10017dd0] = "GoalTriggerSpell"sv;
		goalfunc_names[0x10017f80] = "GoalUnconcealCleanup"sv;
		goalfunc_names[0x10018050] = "GoalResetToIdleAnim"sv;
		goalfunc_names[0x10018160] = "GoalResetToIdleAnimUnstun"sv;
		goalfunc_names[0x10018290] = "GoalThrowItemCleanup"sv;
		goalfunc_names[0x10018400] = "GoalThrowItemPlayAnim"sv;
		goalfunc_names[0x100185e0] = "GoalUnconcealAnimate"sv;
		goalfunc_names[0x10018730] = "GoalStartIdleAnimIfCloseToParty"sv;
		goalfunc_names[0x10018810] = "GoalStartFidgetAnimIfCloseToParty"sv;
		goalfunc_names[0x100188f0] = "GoalContinueWithAnim"sv;
		goalfunc_names[0x100189b0] = "GoalContinueWithAnim2"sv;
		goalfunc_names[0x10018a70] = "GoalPlayDoorOpenAnim"sv;
		goalfunc_names[0x10018b90] = "GoalContinueWithDoorOpenAnim"sv;
		goalfunc_names[0x10018c50] = "GoalPlayDoorCloseAnim"sv;
		goalfunc_names[0x10018d40] = "GoalContinueWithDoorCloseAnim"sv;
		goalfunc_names[0x10018e00] = "GoalPickLockPlayPushDoorOpenAnim"sv;
		goalfunc_names[0x10018ee0] = "GoalPickLockContinueWithAnim"sv;
		goalfunc_names[0x10018fa0] = "GoalDyingPlaySoundAndRipples"sv;
		goalfunc_names[0x10019070] = "GoalDyingContinueAnim"sv;
		goalfunc_names[0x10019130] = "GoalAnimateFireDmgContinueAnim"sv;
		goalfunc_names[0x100191f0] = "GoalStunnedPlayAnim"sv;
		goalfunc_names[0x10019330] = "GoalStunnedContinueAnim"sv;
		goalfunc_names[0x10019470] = "GoalPlayGetUpAnim"sv;
		goalfunc_names[0x10019540] = "GoalPlayUnconcealAnim"sv;
		goalfunc_names[0x10019630] = "GoalPlayMoveAnim"sv;
		goalfunc_names[0x10019920] = "GoalPlayWaterRipples"sv;
		goalfunc_names[0x100199b0] = "GoalContinueMoveStraight"sv;
		goalfunc_names[0x10019c20] = "GoalApplyKnockback"sv;
		goalfunc_names[0x10019e10] = "GoalDyingReturnTrue"sv;
		goalfunc_names[0x10019e70] = "GoalAttemptMoveCleanup"sv;
		goalfunc_names[0x10019f00] = "GoalAttackPlayWeaponHitEffect"sv;
		goalfunc_names[0x1001a080] = "GoalAttackContinueWithAnim"sv;
		goalfunc_names[0x1001a170] = "GoalAttackPlayIdleAnim"sv;
		goalfunc_names[0x1001bf70] = "GoalMoveNearUpdateRadiusToReach"sv;
		goalfunc_names[0x1001c100] = "GoalAttackEndTurnIfUnreachable"sv;
		goalfunc_names[0x101f5850] = "AlwaysSucceed"sv;
		goalfunc_names[0x10262530] = "AlwaysFail"sv;
	}
	
	static std::string_view GetArgInfoText(int animParamType) {
		if (animParamType < 21) {
			return AnimGoalDataNames[animParamType];
		}
		else if (animParamType == 31) {
			return "SELF_OBJ_PRECISE_LOC";
		}
		else if (animParamType == 32) {
			return "TARGET_OBJ_PRECISE_LOC";
		}
		else if (animParamType == 33) {
			return "NULL_HANDLE";
		}
		else if (animParamType == 34) {
			return "TARGET_LOC_PRECISE";
		}
		else {
			return to_string(animParamType);
		}
	}

	static std::string GetTransitionDelay(AnimStateTransition transition) {
		if (transition.delay == 0) {
			return "";
		}
		auto delayText = std::to_string(transition.delay);
		if (transition.delay == transition.DelaySlot) {
			delayText = "DELAY_SLOT";
		}
		else if (transition.delay == transition.DelayRandom) {
			delayText = "DELAY_RANDOM";
		}
		else if (transition.delay == transition.DelayCustom) {
			delayText = "DELAY_CUSTOM";
		}
		return ", " + delayText;
	}

	static std::string GetTransitionArgs(AnimStateTransition transition) {
		/*int nextState = transition.newState & ~ASTF_MASK;

		// Simple transition into another state
		if (!(transition.newState & ASTF_MASK)) {
			return fmt::format("Transition::State({}{})", transition.newState - 1, GetTransitionDelay(transition));
		}

		if ((transition.newState & ASTF_POP_ALL) == ASTF_POP_ALL) {
			return fmt::format("Transition::EndAll(){}", GetTransitionDelay(transition));
		}

		if ((transition.newState & ASTF_PUSH_GOAL) == ASTF_PUSH_GOAL) {
			auto remainingMask = (transition.newState & ASTF_MASK) & ~ASTF_PUSH_GOAL;
			if (remainingMask != 0 && remainingMask != ASTF_REWIND && remainingMask != ASTF_POP_GOAL) {
				__debugbreak();
			}
			return fmt::format("Transition::PushGoal({}){}", GetAnimGoalTypeName((AnimGoalType) nextState), GetTransitionDelay(transition));
		}*/

		auto newState = transition.newState;

		std::vector<std::string> flags;
		if ((newState & ASTF_GOAL_INVALIDATE_PATH) == ASTF_GOAL_INVALIDATE_PATH) {
			flags.push_back("T_INVALIDATE_PATH");
			newState &= ~ASTF_GOAL_INVALIDATE_PATH;
		}

		if ((newState & ASTF_POP_GOAL) == ASTF_POP_GOAL) {
			flags.push_back("T_POP_GOAL");
			newState &= ~ASTF_POP_GOAL;
		}

		if ((newState & ASTF_POP_GOAL_TWICE) == ASTF_POP_GOAL_TWICE) {
			flags.push_back("T_POP_GOAL_TWICE");
			newState &= ~ASTF_POP_GOAL_TWICE;
		}

		if ((newState & ASTF_PUSH_GOAL) == ASTF_PUSH_GOAL) {
			auto pushGoal = GetAnimGoalTypeName((AnimGoalType)(transition.newState & ~ASTF_MASK));
			flags.push_back(fmt::format("T_PUSH_GOAL({})", pushGoal));
			newState &= ~ASTF_PUSH_GOAL;
			newState &= ASTF_MASK;
		}

		if ((newState & ASTF_POP_ALL) == ASTF_POP_ALL) {
			flags.push_back("T_POP_ALL");
			newState &= ~ASTF_POP_ALL;
		}

		if ((newState & ASTF_REWIND) == ASTF_REWIND) {
			flags.push_back("T_REWIND");
			newState &= ~ASTF_REWIND;
		}

		if ((newState & ASTF_MASK) != 0) {
			flags.push_back(fmt::format("0x{:x}", newState & ASTF_MASK));
			newState &= ~ASTF_MASK;
		}

		if (!(transition.newState & ASTF_MASK)) {
			flags.push_back(fmt::format("T_GOTO_STATE({})", transition.newState - 1));
		}

		std::string flagsString;
		if (flags.empty()) {
			flagsString = "0";
		} else {
			for (size_t i = 0; i < flags.size(); i++) {
				if (i > 0) {
					flagsString += "|";
				}
				flagsString += flags[i];
			}
		}
		
		auto delayText = std::to_string(transition.delay);
		if (transition.delay == transition.DelaySlot) {
			delayText = "DELAY_SLOT";
		} else if (transition.delay == transition.DelayRandom) {
			delayText = "DELAY_RANDOM";
		} else if (transition.delay == transition.DelayCustom) {
			delayText = "DELAY_CUSTOM";
		}

		if (transition.delay != 0) {
			return fmt::format("{}, {}", flagsString, delayText);
		} else {
			return fmt::format("{}", flagsString);
		}
	}

	std::string GetCallbackFunctionRef(uint32_t address) {
		if (!goalfunc_names[address].empty()) {
			return std::string(goalfunc_names[address]);
		}

		return fmt::format("0x{:x}", address);
	}

	virtual void apply() override {
		using AnimGoalArray = const AnimGoal*[82];
		AnimGoalArray& goals = temple::GetRef<AnimGoalArray>(0x102BD1B0);


		auto fh = fopen("anim_goal_setup.cpp", "wt");

		for (auto entry : goalfunc_names) {
			fmt::print(fh, "int {}(AnimSlot &slot); // Originally @ 0x{:x}\n", entry.second, entry.first);
		}

		for (auto entry : goalfunc_names) {
			fmt::print(fh, "\n// Originally @ 0x{:x}\n", entry.first);
			fmt::print(fh, "int {}(AnimSlot &slot) {{\n", entry.second);
			fmt::print(fh, "\tstatic org = temple::GetRef<GoalCallback>(0x{:x});\n", entry.first);
			fmt::print(fh, "\treturn org(slot);\n");
			fmt::print(fh, "}}\n");
		}

		for (int i = 0; i < ag_count; i++) {
			auto goal = goals[i];
			auto goalName = GetAnimGoalTypeName((AnimGoalType)i);
			if (!goal) {
				fmt::print(fh, "\n");
				fmt::print(fh, "// {} was undefined\n", goalName);
				fmt::print(fh, "\n");
				continue;
			}

			auto builderVar = std::string(GetAnimGoalTypeName((AnimGoalType)i));
			if (builderVar.find("ag_") == 0) {
				builderVar.replace(0, 3, "");
			}

			fmt::print(fh, "// {}\n", goalName);
			fmt::print(fh, "auto {} = AnimGoalBuilder(goals_[{}])", builderVar, goalName);
			fmt::print(fh, "\n\t.SetPriority({})", GetAnimGoalPriorityText(goal->priority));
			if (goal->interruptAll) {
				fmt::print(fh, "\n\t.SetInterruptAll(true)");
			}
			if (goal->field_C) {
				fmt::print(fh, "\n\t.SetFieldC(true)");
			}
			if (goal->field_10) {
				fmt::print(fh, "\n\t.SetField10(true)");
			}
			if (goal->relatedGoal[0] != -1) {
				fmt::print(fh, "\n\t.SetRelatedGoals(");
				for (int j = 0; j < 3; j++) {
					if (goal->relatedGoal[j] == -1) {
						break;
					}
					if (j > 0) {
						fmt::print(fh, ", ");
					}
					fmt::print(fh, "{}", GetAnimGoalTypeName((AnimGoalType) goal->relatedGoal[j]));
				}
				fmt::print(fh, ")");
			}
			fmt::print(fh, ";\n");

			for (int j = -1; j < goal->statecount; j++) {
				AnimGoalState state;
				if (j == -1) {
					state = goal->state_special;
					if (!state.callback) {
						continue;
					}
					fmt::print(fh, "{}.AddCleanup({})", builderVar, GetCallbackFunctionRef((uint32_t)state.callback));
				} else {
					state = goal->states[j];
					fmt::print(fh, "{}.AddState({}) // Index {}", builderVar, GetCallbackFunctionRef((uint32_t)state.callback), j);
				}

				Expects(state.argInfo2 == -1 || state.argInfo1 != -1);

				if (state.argInfo1 != -1) {
					fmt::print(fh, "\n\t.SetArgs({}", GetArgInfoText(state.argInfo1));

					if (state.argInfo2 != -1) {
						fmt::print(fh, ", {}", GetArgInfoText(state.argInfo2));
					}
					fmt::print(fh, ")");
				}

				if (state.flagsData != -1) {
					fmt::print(fh, "\n\t.SetFlagsData({})", state.flagsData);
				}

				// Print transitions
				if (j != -1) {
					fmt::print(fh, "\n\t.OnSuccess({})", GetTransitionArgs(state.afterSuccess));
				}
				if (j != -1) {
					fmt::print(fh, "\n\t.OnFailure({})", GetTransitionArgs(state.afterFailure));
				}

				fmt::print(fh, ";\n");
			}

			fmt::print(fh, "\n");

		}

		fclose(fh);

		// All functions that use 102BD1B0 should now be replaced (the goals array)
		// breakRegion(0x102BD1B0, 0x102BD2F4);

		// anim_first_run_idx_for_obj
		replaceFunction<int(objHndl)>(0x10054e20, [](objHndl handle) {
			return gameSystems->GetAnim().GetFirstRunSlotIdxForObj(handle);
		});

		// anim_next_run_idx_for_obj
		replaceFunction<int(int, objHndl)>(0x10054e70, [](int slotIndex, objHndl handle) {
			return gameSystems->GetAnim().GetNextRunSlotIdxForObj(handle, slotIndex);
		});

		// anim_run_id_for_obj
		replaceFunction<int(objHndl, AnimSlotId *)>(0x1000c430, [](objHndl handle, AnimSlotId *slotIdOut) {

			auto slotId = gameSystems->GetAnim().GetFirstRunSlotId(handle);
			return HandleOptionalSlotResult(slotId, slotIdOut);

		});

		// anim_get_highest_priority
		replaceFunction<AnimGoalPriority(objHndl)>(0x1000c500, [](objHndl handle) {
			return gameSystems->GetAnim().GetCurrentPriority(handle);
		});

		// anim_pop_goal
		replaceFunction<void(AnimSlot &, const uint32_t *, const AnimGoal **, AnimSlotGoalStackEntry **, BOOL *)>(0x10016FC0, 
			[](AnimSlot &slot, const uint32_t *popFlags, const AnimGoal **newGoal, AnimSlotGoalStackEntry **newCurrentGoal, BOOL *stopProcessing) {
			bool stopProcessingBool = *stopProcessing == TRUE;
			gameSystems->GetAnim().PopGoal(slot, *popFlags, newGoal, newCurrentGoal, &stopProcessingBool);
			*stopProcessing = stopProcessingBool ? TRUE : FALSE;
		});

		// anim_break_nodes_to_map
		replaceFunction<int(const char *)>(0x1001af30, [](const char *mapName) {
			gameSystems->GetAnim().SaveToMap(mapName);
			return 0;
		});


		replaceFunction<int(const TimeEvent *)>(0x1001B830, [](const TimeEvent *evt) -> int {
			auto result = 0;

			result = gameSystems->GetAnim().ProcessAnimEvent(evt);
			temple::GetRef<int>(0x102B2654) = gameSystems->GetAnim().mCurrentlyProcessingSlotIdx;

			return result;
		});

		// anim_is_running_goal
		replaceFunction<int(objHndl, AnimGoalType, AnimSlotId*)>(0x10054f30, [](objHndl handle, AnimGoalType goalType, AnimSlotId *idOut) {
			if (goalType == -1) {
				return 0; // Weird...
			}

			return gameSystems->GetAnim().DoesObjHaveGoal(handle, goalType, idOut) ? 1 : 0;
		});

		// anim_get_slot_with_fieldc_goal
		replaceFunction<int(objHndl, AnimSlotId *)>(0x10054fd0, [](objHndl handle, AnimSlotId *idOut) {

			auto id = gameSystems->GetAnim().GetRunSlotWithGoalWithoutFieldC(handle);
			return HandleOptionalSlotResult(id, idOut);

		});

		// anim_has_attack_anim
		replaceFunction<int(objHndl, AnimSlotId *, objHndl)>(0x10055060, [](objHndl handle, AnimSlotId *idOut, objHndl target) {
			auto id = gameSystems->GetAnim().HasAttackAnim(handle, target);
			return HandleOptionalSlotResult(id, idOut);
		});

		// anim_cur_goal_has_field10_1
		replaceFunction<int(AnimSlot *)>(0x100551a0, [](AnimSlot *slot) {
			return gameSystems->GetAnim().CurrentGoalHasField10_1(*slot) ? 1 : 0;
		});

		// anim_make_goal_self_true
		replaceFunction<int(AnimSlotGoalStackEntry *, objHndl, AnimGoalType)>(0x100556c0, [](AnimSlotGoalStackEntry *pGoalData, objHndl handle, AnimGoalType goalType) {
			Expects(pGoalData);
			return pGoalData->InitWithInterrupt(handle, goalType) ? 1 : 0;
		});

		// anim_make_goal_self_false
		replaceFunction<int(AnimSlotGoalStackEntry *, objHndl, AnimGoalType)>(0x100556e0, [](AnimSlotGoalStackEntry *pGoalData, objHndl handle, AnimGoalType goalType) {
			return pGoalData->Init(handle, goalType) ? 1 : 0;
		});

		// anim_get_goal_with_same_objs
		replaceFunction<int(objHndl, AnimSlotGoalStackEntry*, AnimSlotId *)>(0x10055ab0, [](objHndl handle, AnimSlotGoalStackEntry *goalData, AnimSlotId *idOut) {
			auto id = gameSystems->GetAnim().GetSlotForGoalAndObjs(handle, *goalData);
			return HandleOptionalSlotResult(id, idOut);
		});

		// anim_clear_for_obj
		replaceFunction<int(objHndl)>(0x1000c760, [](objHndl handle) {
			return gameSystems->GetAnim().ClearForObject(handle) ? 1 : 0;
		});

		// anim_set_all_goals_cleared_callback
		replaceFunction<void(void(*)(void))>(0x1000c750, [](void(*callback)(void)) {
			gameSystems->GetAnim().SetAllGoalsClearedCallback(callback);
		});

		// animpriv_get_slot
		replaceFunction<int(AnimSlotId *, AnimSlot **)>(0x10016c40, [](AnimSlotId *id, AnimSlot **slotOut) {
			*slotOut = gameSystems->GetAnim().GetSlot(*id);
			return (*slotOut != nullptr) ? 1 : 0;
		});

		// AnimSlotInterruptGoals
		replaceFunction<int(AnimSlotId &, AnimGoalPriority)>(0x10056090, [](AnimSlotId &animId, AnimGoalPriority priority) {
			auto &slot = gameSystems->GetAnim().GetRunSlot(animId.slotIndex);
			return gameSystems->GetAnim().InterruptGoals(slot, priority) ? 1 : 0;
		});

		// anim_interrupt_by_type
		replaceFunction<int(objHndl, AnimGoalType, AnimGoalType)>(0x10056350, [](objHndl handle, AnimGoalType interruptGoals, AnimGoalType keep) {
			gameSystems->GetAnim().InterruptGoalsByType(handle, interruptGoals, keep);
			return 1;
		});

		// Push Goal Impl
		replaceFunction<BOOL(__cdecl)(AnimSlotGoalStackEntry*, AnimSlotId*, int, int)>(0x10056600, [](AnimSlotGoalStackEntry* gdata, AnimSlotId*slotId, int allocSlot, int flags) {
			Expects(gdata);
			if (allocSlot != 1) {
				throw TempleException("Push Goal should never be called with allocSlot=false");
			}
			return gameSystems->GetAnim().PushGoalInternal(*gdata, slotId, flags) ? 1 : 0;
		});

		// animNumActiveGoals_inc
		replaceFunction<void(AnimSlot *, AnimGoal *)>(0x10055bf0, [](AnimSlot *slot, AnimGoal *goal) {
			gameSystems->GetAnim().IncreaseActiveGoalCount(*slot, *goal);
		});

		// animNumActiveGoals_dec
		replaceFunction<void(AnimSlot *, AnimGoal *)>(0x10055ca0, [](AnimSlot *slot, AnimGoal *goal) {
			gameSystems->GetAnim().DecreaseActiveGoalCount(*slot, *goal);
		});

		// anim_subgoal_add_func
		replaceFunction<int(AnimSlotId, AnimSlotGoalStackEntry*)>(0x10056a50, [](AnimSlotId id, AnimSlotGoalStackEntry *goalStackEntry) {
			return gameSystems->GetAnim().AddSubGoal(id, *goalStackEntry) ? 1 : 0;
		});

		replaceFunction<BOOL(__cdecl)(objHndl, LocAndOffsets, PathQueryResult*)>(0x1001A2E0, [](objHndl handle, LocAndOffsets loc, PathQueryResult* pqr) {
			return gameSystems->GetAnim().PushRunToTile(handle, loc, pqr) ? TRUE : FALSE;
		});

		replaceFunction<void(__cdecl)()>(0x1001A930, []() {
			gameSystems->GetAnim().TurnOnRunning();
		});

		// Push Goal
		replaceFunction<BOOL(__cdecl)(AnimSlotGoalStackEntry*, AnimSlotId*)>(0x10056D20, [](AnimSlotGoalStackEntry* gdata, AnimSlotId*slotId) {
			Expects(gdata);
			return gameSystems->GetAnim().PushGoal(*gdata, slotId) ? TRUE : FALSE;
		});

		replaceFunction<void()>(0x10016A30, []() {
			gameSystems->GetAnim().ProcessActionCallbacks();
		});

		// animpriv_alloc_run_slot
		replaceFunction<int(AnimSlotId*)>(0x10055470, [](AnimSlotId *pAnimID) {
			auto slotId = gameSystems->GetAnim().AllocSlot();
			if (slotId) {
				*pAnimID = *slotId;
				return 1;
			} else {
				return 0;
			}
		});

		// anim_is_processing
		replaceFunction<int()>(0x10054e10, []() {
			return gameSystems->GetAnim().IsProcessing() ? 1 : 0;
		});

		// anim_push_fidget
		replaceFunction<int(objHndl)>(0x10015d70, [](objHndl handle) {
			return gameSystems->GetAnim().PushFidget(handle);
		});

		// anim_get_last_anim_run_id_pushed_to
		replaceFunction<int(AnimSlotId *)>(0x10054ee0, [](AnimSlotId *slotIdOut) {
			if (slotIdOut) {
				*slotIdOut = gameSystems->GetAnim().GetLastSlotPushedTo();
			}
			return 1;
		});

	}

} hooks;
