#include "stdafx.h"
#include "anim.h"
#include "util/fixes.h"
#include "temple_functions.h"
#include "timeevents.h"
#include "util/config.h"
#include "obj.h"
#include "pathfinding.h"
#include "dice.h"

#include <map>
#include <set>
#include <fstream>
#include "python/pythonglobal.h"
#include "party.h"

#pragma pack(push, 1)

struct AnimSlotId {
	int slotIndex;
	int uniqueId;
	int field_8;

	string toString() const {
		return format("[{}:{}r{}]", slotIndex, uniqueId, field_8);
	}
};

// Allows for direct use of AnimSlotId in format() strings
ostream &operator<<(ostream &str, const AnimSlotId &id) {
	str << id.toString();
	return str;
}

enum AnimGoalType : uint32_t {
	ag_animate = 0x0,
	ag_animate_loop = 0x1,
	ag_anim_idle = 0x2,
	ag_anim_fidget = 0x3,
	ag_move_to_tile = 0x4,
	ag_run_to_tile = 0x5,
	ag_attempt_move = 0x6,
	ag_move_to_pause = 0x7,
	ag_move_near_tile = 0x8,
	ag_move_near_obj = 0x9,
	ag_move_straight = 0xA,
	ag_attempt_move_straight = 0xB,
	ag_open_door = 0xC,
	ag_attempt_open_door = 0xD,
	ag_unlock_door = 0xE,
	ag_jump_window = 0xF,
	ag_pickup_item = 0x10,
	ag_attempt_pickup = 0x11,
	ag_pickpocket = 0x12,
	ag_attack = 0x13,
	ag_attempt_attack = 0x14,
	ag_talk = 0x15,
	ag_pick_weapon = 0x16,
	ag_chase = 0x17,
	ag_follow = 0x18,
	ag_follow_to_location = 0x19,
	ag_flee = 0x1A,
	ag_throw_spell = 0x1B,
	ag_attempt_spell = 0x1C,
	ag_shoot_spell = 0x1D,
	ag_hit_by_spell = 0x1E,
	ag_hit_by_weapon = 0x1F,
	ag_dying = 0x20,
	ag_destroy_obj = 0x21,
	ag_use_skill_on = 0x22,
	ag_attempt_use_skill_on = 0x23,
	ag_skill_conceal = 0x24,
	ag_projectile = 0x25,
	ag_throw_item = 0x26,
	ag_use_object = 0x27,
	ag_use_item_on_object = 0x28,
	ag_use_item_on_object_with_skill = 0x29,
	ag_use_item_on_tile = 0x2A,
	ag_use_item_on_tile_with_skill = 0x2B,
	ag_knockback = 0x2C,
	ag_floating = 0x2D,
	ag_close_door = 0x2E,
	ag_attempt_close_door = 0x2F,
	ag_animate_reverse = 0x30,
	ag_move_away_from_obj = 0x31,
	ag_rotate = 0x32,
	ag_unconceal = 0x33,
	ag_run_near_tile = 0x34,
	ag_run_near_obj = 0x35,
	ag_animate_stunned = 0x36,
	ag_animate_kneel_magic_hands = 0x37,
	ag_attempt_move_near = 0x38,
	ag_knock_down = 0x39,
	ag_anim_get_up = 0x3A,
	ag_attempt_move_straight_knockback = 0x3B,
	ag_wander = 0x3C,
	ag_wander_seek_darkness = 0x3D,
	ag_use_picklock_skill_on = 0x3E,
	ag_please_move = 0x3F,
	ag_attempt_spread_out = 0x40,
	ag_animate_door_open = 0x41,
	ag_animate_door_closed = 0x42,
	ag_pend_closing_door = 0x43,
	ag_throw_spell_friendly = 0x44,
	ag_attempt_spell_friendly = 0x45,
	ag_animate_loop_fire_dmg = 0x46,
	ag_attempt_move_straight_spell = 0x47,
	ag_move_near_obj_combat = 0x48,
	ag_attempt_move_near_combat = 0x49,
	ag_use_container = 0x4A,
	ag_throw_spell_w_cast_anim = 0x4B,
	ag_attempt_spell_w_cast_anim = 0x4C,
	ag_throw_spell_w_cast_anim_2ndary = 0x4D,
	ag_back_off_from = 0x4E,
	ag_attempt_use_pickpocket_skill_on = 0x4F,
	ag_use_disable_device_skill_on_data = 0x50,
	ag_unknown,
	ag_count = 82
};

const char* animGoalTypeNames[ag_count] = {
	"ag_animate",
	"ag_animate_loop",
	"ag_anim_idle",
	"ag_anim_fidget",
	"ag_move_to_tile",
	"ag_run_to_tile",
	"ag_attempt_move",
	"ag_move_to_pause",
	"ag_move_near_tile",
	"ag_move_near_obj",
	"ag_move_straight",
	"ag_attempt_move_straight",
	"ag_open_door",
	"ag_attempt_open_door",
	"ag_unlock_door",
	"ag_jump_window",
	"ag_pickup_item",
	"ag_attempt_pickup",
	"ag_pickpocket",
	"ag_attack",
	"ag_attempt_attack",
	"ag_talk",
	"ag_pick_weapon",
	"ag_chase",
	"ag_follow",
	"ag_follow_to_location",
	"ag_flee",
	"ag_throw_spell",
	"ag_attempt_spell",
	"ag_shoot_spell",
	"ag_hit_by_spell",
	"ag_hit_by_weapon",
	"ag_dying",
	"ag_destroy_obj",
	"ag_use_skill_on",
	"ag_attempt_use_skill_on",
	"ag_skill_conceal",
	"ag_projectile",
	"ag_throw_item",
	"ag_use_object",
	"ag_use_item_on_object",
	"ag_use_item_on_object_with_skill",
	"ag_use_item_on_tile",
	"ag_use_item_on_tile_with_skill",
	"ag_knockback",
	"ag_floating",
	"ag_close_door",
	"ag_attempt_close_door",
	"ag_animate_reverse",
	"ag_move_away_from_obj",
	"ag_rotate",
	"ag_unconceal",
	"ag_run_near_tile",
	"ag_run_near_obj",
	"ag_animate_stunned",
	"ag_animate_kneel_magic_hands",
	"ag_attempt_move_near",
	"ag_knock_down",
	"ag_anim_get_up",
	"ag_attempt_move_straight_knockback",
	"ag_wander",
	"ag_wander_seek_darkness",
	"ag_use_picklock_skill_on",
	"ag_please_move",
	"ag_attempt_spread_out",
	"ag_animate_door_open",
	"ag_animate_door_closed",
	"ag_pend_closing_door",
	"ag_throw_spell_friendly",
	"ag_attempt_spell_friendly",
	"ag_animate_loop_fire_dmg",
	"ag_attempt_move_straight_spell",
	"ag_move_near_obj_combat",
	"ag_attempt_move_near_combat",
	"ag_use_container",
	"ag_throw_spell_w_cast_anim",
	"ag_attempt_spell_w_cast_anim",
	"ag_throw_spell_w_cast_anim_2ndary",
	"ag_back_off_from",
	"ag_attempt_use_pickpocket_skill_on",
	"ag_use_disable_device_skill_on_data",
	"ag_unknown"
};

// Has to be 0x10 in size
union AnimParam {
	objHndl obj;
	LocAndOffsets location;
	int number;
};

struct goal_stack_entry_objref {
	uint64_t objid;
	int field_8;
	int field_C;
};

struct goal_stack_entry_locref {
	ObjectId objid;
	uint64_t location;
	int mapNumber;
	int field_24;
};

struct AnimSlotGoalStackEntry {
	AnimGoalType goalType;
	int unk1;
	AnimParam self;
	AnimParam target;
	AnimParam block;
	AnimParam scratch;
	AnimParam parent;
	AnimParam targetTile;
	AnimParam range;
	AnimParam animId;
	AnimParam animIdPrevious;
	AnimParam animData;
	AnimParam spellData;
	AnimParam skillData;
	AnimParam flagsData;
	AnimParam scratchVal1;
	AnimParam scratchVal2;
	AnimParam scratchVal3;
	AnimParam scratchVal4;
	AnimParam scratchVal5;
	AnimParam scratchVal6;
	AnimParam soundHandle;
	uint32_t soundStreamId;
	uint32_t soundStreamId2; // Probably padding
	uint32_t padding[2];
	TimeEventObjInfo selfTracking;
	TimeEventObjInfo targetTracking;
	TimeEventObjInfo blockTracking;
	TimeEventObjInfo scratchTracking;
	TimeEventObjInfo parentTracking;
};

const auto TestSizeOfAnimSlotGoalStackEntry = sizeof(AnimSlotGoalStackEntry);

struct AnimSlot {
	AnimSlotId id;
	int flags; // See AnimSlotFlag
	int currentState;
	int field_14;
	int field_18;
	int field_1C;
	objHndl animObj;
	int currentGoal;
	int field_2C;
	AnimSlotGoalStackEntry goals[8];
	AnimSlotGoalStackEntry* pCurrentGoal;
	uint32_t unk1;
	uint32_t unk2;
	uint32_t unk3;
	uint32_t padding[60];
	PathQueryResult path;
	AnimParam param1; // Used as parameters for goal states
	AnimParam param2; // Used as parameters for goal states
	uint32_t someGoalType;
	uint32_t unknown[5];
	uint64_t gametimeSth;
	uint32_t unknown2;
	uint32_t uniqueActionId; // ID assigned when triggered by a D20 action
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AnimStateTransition {
	uint32_t newState;
	// Delay in milliseconds or one of the constants below
	int delay;

	// Delay is read from runSlot->animDelay
	static const uint32_t DelaySlot = -2;
	// Delay is read from 0x10307534 (written by some goal states)
	static const uint32_t DelayCustom = -3;
	// Specifies a random 0-300 ms delay
	static const uint32_t DelayRandom = -4;
};

struct AnimGoalState {
	bool (__cdecl *callback)(AnimSlot& slot);
	int argInfo1;
	int argInfo2;
	int refToOtherGoalType;
	AnimStateTransition afterFailure;
	AnimStateTransition afterSuccess;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AnimGoal {
	int statecount;
	AnimGoalPriority priority;
	int field_8;
	int field_C;
	int field_10;
	int relatedGoal1;
	int relatedGoal2;
	int relatedGoal3;
	AnimGoalState states[16];
	AnimGoalState state_special;
};
#pragma pack(pop)

enum AnimGoalDataItem {
	AGDATA_SELF_OBJ = 0, // Type: 1 (Object)
	AGDATA_TARGET_OBJ, // Type: 1
	AGDATA_BLOCK_OBJ, // Type: 1
	AGDATA_SCRATCH_OBJ, // Type: 1
	AGDATA_PARENT_OBJ, // Type: 1
	AGDATA_TARGET_TILE, // Type: 2 (Location)
	AGDATA_RANGE_DATA, // Type: 2
	AGDATA_ANIM_ID, // Type: 0 (just a 32-bit number it seems)
	AGDATA_ANIM_ID_PREVIOUS, // Type: 0
	AGDATA_ANIM_DATA, // Type: 0
	AGDATA_SPELL_DATA, // Type: 0
	AGDATA_SKILL_DATA, // Type: 0
	AGDATA_FLAGS_DATA, // Type: 0
	AGDATA_SCRATCH_VAL1, // Type: 0
	AGDATA_SCRATCH_VAL2, // Type: 0
	AGDATA_SCRATCH_VAL3, // Type: 0
	AGDATA_SCRATCH_VAL4, // Type: 0
	AGDATA_SCRATCH_VAL5, // Type: 0
	AGDATA_SCRATCH_VAL6, // Type: 0
	AGDATA_SOUND_HANDLE // Type: 0
};

const char* AnimGoalDataNames[] = {
	"AGDATA_SELF_OBJ",
	"AGDATA_TARGET_OBJ",
	"AGDATA_BLOCK_OBJ",
	"AGDATA_SCRATCH_OBJ",
	"AGDATA_PARENT_OBJ",
	"AGDATA_TARGET_TILE",
	"AGDATA_RANGE_DATA",
	"AGDATA_ANIM_ID",
	"AGDATA_ANIM_ID_PREV",
	"AGDATA_ANIM_DATA",
	"AGDATA_SPELL_DATA",
	"AGDATA_SKILL_DATA",
	"AGDATA_FLAGS_DATA",
	"AGDATA_SCRATCH_VAL1",
	"AGDATA_SCRATCH_VAL2",
	"AGDATA_SCRATCH_VAL3",
	"AGDATA_SCRATCH_VAL4",
	"AGDATA_SCRATCH_VAL5",
	"AGDATA_SCRATCH_VAL6",
	"AGDATA_SOUND_HANDLE"
};

enum AnimSlotFlag {
	ASF_ACTIVE = 1,
	ASF_UNK2 = 2,
	ASF_UNK1 = 0x10000
};

static void assertStructSizes() {
	static_assert(validate_size<ObjectId, 0x18>::value, "Goal Stack entry has incorrect size.");
	static_assert(validate_size<AnimParam, 0x10>::value, "Anim Param union has the wrong size.");
	static_assert(validate_size<AnimSlotGoalStackEntry, 0x220>::value, "Goal Stack entry has incorrect size.");
	static_assert(validate_size<AnimSlot, 0x2C98>::value, "AnimSlot has incorrect size");
}

static class AnimAddressTable : AddressTable {
public:
	AnimGoal** goals;

	AnimSlot* slots;
	const int slotsCount = 512;
	uint32_t* nextUniqueId;
	uint32_t* slotsInUse;

	/*
		Set to true when ToEE cannot allocate an animation slot. This causes
		the anim system to try and interrupt as many animations as possible.
	*/
	bool* allSlotsUsed;

	/*
		While processing the timer event for a slot, this will contain the slots index.
		Otherwise -1.
	*/
	int *currentlyProcessingSlotIdx;

	/*
		Size of the queue that contains the objects/slot ids that have completed animations 
		and should trigger action processing.
	*/
	int *completeQueueSize;

	/*
		Size 20 queue of unique animation ids (action-variety) that have completed.
	*/
	int *completeQueueAnimIds;

	/*
		Size 20 queue of object ids for which animations were completed.
	*/
	objHndl *completeQueueObjects;

	/*
		Some goal states want to set a dynamic delay for transitioning into the next
		state. They use this global variable to transport this value into the state
		machine controller below.
	*/
	int *customDelayInMs;

	void (__cdecl *GetAnimName)(int animId, char* animNameOut);
	void (__cdecl *PushGoalDying)(objHndl obj, int rotation);
	void (__cdecl *InterruptAllGoalsOfPriority)(AnimGoalPriority priority);
	void (__cdecl *InterruptAllForTbCombat)();
	
	/*
		Interrupts animations in the given animation slot. Exact behaviour is not known yet.
	*/
	void(__cdecl *Interrupt)(const AnimSlotId &id, AnimGoalPriority priority);

	void (__cdecl *DrainCompletedQueue)();
	
	/*
		Will validate object references in the anim slot and set param1 and param2
		to whatever the given state requests. The state is optional. If it is null,
		only the object references will be validated.
	*/
	bool (__cdecl *PrepareSlotForGoalState)(AnimSlot &slot, const AnimGoalState *state);

	void (__cdecl *PopGoal)(AnimSlot &slot, uint32_t &popFlags, const AnimGoal **newGoal, AnimSlotGoalStackEntry **newCurrentGoal, bool &keepProcessing);

	AnimAddressTable() {
		rebase(GetAnimName, 0x102629D0);
		rebase(goals, 0x102BD1B0);
		rebase(nextUniqueId, 0x11E61520);
		rebase(slots, 0x118CE520);
		rebase(slotsInUse, 0x10AA4BBC);
		rebase(allSlotsUsed, 0x10AA4BB0);
		rebase(currentlyProcessingSlotIdx, 0x102B2654);
		rebase(completeQueueSize, 0x1030753C);
		rebase(completeQueueObjects, 0x10307428);
		rebase(completeQueueAnimIds, 0x103074D0);
		rebase(customDelayInMs, 0x10307534);

		rebase(PushGoalDying, 0x100157B0);
		rebase(InterruptAllGoalsOfPriority, 0x1000C8D0);
		rebase(InterruptAllForTbCombat, 0x1000C950);
		rebase(Interrupt, 0x10056090);
		rebase(DrainCompletedQueue, 0x10016A30);		
		rebase(PrepareSlotForGoalState, 0x10055700);
		rebase(PopGoal, 0x10016FC0);
	}

} animAddresses;

static string getDelayText(AnimStateTransition trans) {
	string delay = "";
	if (trans.delay == AnimStateTransition::DelayRandom) {
		delay = ", delay: random";
	} else if (trans.delay == AnimStateTransition::DelayCustom) {
		delay = ", delay: custom";
	} else if (trans.delay == AnimStateTransition::DelaySlot) {
		delay = ", delay: slot";
	} else if (trans.delay != 0) {
		delay = format(", delay: {}", trans.delay);
	}
	return delay;
}

static const uint32_t TRANSITION_LOOP = 0x10000000;
static const uint32_t TRANSITION_END = 0x20000000;
static const uint32_t TRANSITION_GOAL = 0x40000000;

static void getTransitionText(string& diagramText, int& j, AnimStateTransition& transition, const char* condition) {
	string delay = getDelayText(transition);
	auto newState = transition.newState;
	if (newState & 0xFF000000) {
		logger->info("New state flags {:x}", newState);
		if (newState & TRANSITION_LOOP) {
			diagramText += format("state{} --> state0 : [{}{}]\n", j, condition, delay);
		} else if (newState & TRANSITION_END) {
			diagramText += format("state{} --> [*] : [{}{}]\n", j, condition, delay);
		} else if (newState & TRANSITION_GOAL) {
			auto newGoal = newState & 0xFFF;
			diagramText += format("state{} --> [*] : [{}{}] to {}\n", j, condition, delay, animGoalTypeNames[newGoal]);
		}
	} else {
		// Normal transition
		diagramText += format("state{} --> state{} : [{}{}]\n", j, newState - 1, condition, delay);
	}
}

/*
	When an event should be re-executed at a later time, but unmodified, this
	method is used. It also checks whether animations should "catch up" (by skipping
	frames essentially), or whether they should be run at whatever speed was intended,
	but visibly slowing down.
*/
static void rescheduleEvent(int delayMs, const AnimSlot &slot, const TimeEvent *oldEvt) {
	TimeEvent evt;
	evt.system = TimeEventSystem::Anim;
	evt.params[0].int32 = slot.id.slotIndex;
	evt.params[1].int32 = slot.id.uniqueId;
	evt.params[2].int32 = 1111; // Some way to identify these rescheduled events???
	
	if (config.animCatchup) {
		timeEvents.ScheduleAbsolute(evt, oldEvt->time, delayMs);
	} else {
		timeEvents.Schedule(evt, delayMs);
	}
}

/*
	Puts an objects and animation slot into the queue for processing completed animations,
	which happens at the end of the time event processing.
*/
static void PushCompletedAnimation(const AnimSlot &slot) {
	int &queueSize = *animAddresses.completeQueueSize;
	if (queueSize < 20) {
		animAddresses.completeQueueAnimIds[queueSize] = slot.uniqueActionId;
		animAddresses.completeQueueObjects[queueSize] = slot.animObj;
		++queueSize;
	} else {
		logger->warn("Queue for objects with complete animations is full!");
	}
}

static void __cdecl anim_timeevent_process(const TimeEvent* evt) {

	if (*animAddresses.allSlotsUsed) {
		animAddresses.InterruptAllGoalsOfPriority(AGP_3);
		*animAddresses.allSlotsUsed = false;
	}

	// The animation slot id we're triggered for
	AnimSlotId triggerId = { evt->params[0].int32, evt->params[1].int32, evt->params[2].int32 };

	assert(triggerId.slotIndex < animAddresses.slotsCount);

	auto& slot = animAddresses.slots[triggerId.slotIndex];

	// This seems like a pretty stupid check since slots cannot "move" 
	// and the first part of their ID must be the slot index
	// Shouldn't this really check for the unique id of the animation instead?
	if (slot.id.slotIndex != triggerId.slotIndex) {
		logger->debug("{} != {}", slot.id, triggerId);
		return;
	}

	// Slot did belong to "us", but it was deactivated earlier
	if (!(slot.flags & ASF_ACTIVE)) {
		animAddresses.DrainCompletedQueue();
		return;
	}

	// Interesting how this reschedules in at least 100ms which seems steep for animation processing
	// Have to check where and why this is set
	if (slot.flags & ASF_UNK1) {
		animAddresses.DrainCompletedQueue();
		
		auto delay = min(slot.path.someDelay, 100);
		rescheduleEvent(delay, slot, evt);
		return;
	}

	if (slot.currentGoal < 0) {
		logger->warn("Found slot {} with goal < 0", slot.id);
		slot.currentGoal = 0;
	}

	// This sets the current stack pointer, although it should already be set. They used
	// a lot of safeguard against themselves basically
	auto &currentGoal = slot.goals[slot.currentGoal];
	slot.pCurrentGoal = &currentGoal;

	bool unkFlag = false;
	const AnimGoal *goal = nullptr;

	// And another safeguard
	if (currentGoal.goalType < 0 || currentGoal.goalType >= ag_count) {
		slot.flags |= ASF_UNK2;
		unkFlag = true;
	} else {
		goal = animAddresses.goals[currentGoal.goalType];
		if (!goal) {
			logger->error("Animation slot {} references null goal {}.", slot.id, currentGoal.goalType);
		}
	}

	// This validates object references found in the animation slot
	if (!animAddresses.PrepareSlotForGoalState(slot, nullptr)) {
		animAddresses.DrainCompletedQueue();
		return;
	}

	// Validates that the object the animation runs for is not destroyed
	if (slot.animObj) {
		if (objects.GetFlags(slot.animObj) & OF_DESTROYED) {
			logger->warn("Processing animation slot {} for destroyed object.", slot.id);
		}
	} else {
		// Animation is no longer associated with an object after validation
		slot.flags |= ASF_UNK2;
		unkFlag = true;
	}

	// TODO: Clean up this terrible control flow
	if (!unkFlag) {

		*animAddresses.currentlyProcessingSlotIdx = slot.id.slotIndex;

		// TODO: processing
		int loopNr = 0;

		while (true) {
			++loopNr;

			// This only applies to in-development i think, since as of now there should be no infi-looping goals
			if (loopNr >= 100) {
				logger->error("Goal {} loops infinitely in animation {}!", slot.pCurrentGoal->goalType, slot.id);				
				templeFuncs.TurnProcessing(slot.animObj);
				*animAddresses.currentlyProcessingSlotIdx = -1;
				animAddresses.Interrupt(slot.id, AGP_HIGHEST);
				animAddresses.DrainCompletedQueue();
				return;
			}

			auto &currentState = goal->states[slot.currentState];

			// Prepare for the current state
			if (!animAddresses.PrepareSlotForGoalState(slot, &currentState)) {
				animAddresses.DrainCompletedQueue();
				return;
			}

			auto stateResult = currentState.callback(slot);

			// Check flags on the slot that may have been set by the callbacks.
			if (slot.flags & ASF_UNK1) {
				unkFlag = true;
			}

			if (!(slot.flags & ASF_ACTIVE)) {
				*animAddresses.currentlyProcessingSlotIdx = -1;
				animAddresses.DrainCompletedQueue();
				return;
			}

			if (slot.flags & ASF_UNK2) {
				*animAddresses.currentlyProcessingSlotIdx = -1;
				if (slot.animObj) {
					// Interrupt everything for the slot
					animAddresses.Interrupt(slot.id, AGP_HIGHEST);
					if (objects.IsCritter(slot.animObj)) {
						PushCompletedAnimation(slot);
					}
				}
				animAddresses.DrainCompletedQueue();
				break; // Previously: 
			}

			auto &transition = stateResult ? currentState.afterSuccess : currentState.afterFailure;
			auto nextState = transition.newState;
			auto delay = transition.delay;

			// Special transitions
			if (nextState & 0xFF000000) {
				if (nextState & 0x10000000) {
					slot.currentState = 0;
					unkFlag = true;
				}
				if (nextState & 0x30000000) {
					auto newState = &currentGoal;
					auto newGoal = &goal;
					auto popFlags = nextState;
					animAddresses.PopGoal(slot, popFlags, newGoal, &newState, unkFlag);
					if (nextState & 0x08000000) {
						animAddresses.PopGoal(slot, popFlags, newGoal, &newState, unkFlag);
					}
				}
				if (nextState & 0x40000000) {
					// TODO logic for this flag. (no jumps at least)
				}
				if ((nextState & 0x90000000) == 0x90000000) {
					// TODO logic for this flag. (no jumps at least)
				}
			} else {
				// Normal jump to another state without special flags
				--nextState; // Jumps are 1-based, although the array is 0-based
				if (slot.currentState == nextState) {
					logger->error("State {} of goal {} transitioned into itself.", slot.currentState, currentGoal.goalType);
				}
				slot.currentState = nextState;
			}

			if (delay) {
				switch (delay) {
				case AnimStateTransition::DelaySlot:
					// Use the delay specified in the slot. Reasoning currently unknown.
					// NOTE: Could mean that it's waiting for pathing to complete
					delay = slot.path.someDelay;
					break;
				case AnimStateTransition::DelayCustom:
					// Used by some goal states to set their desired dynamic delay
					delay = *animAddresses.customDelayInMs;
					break;
				case AnimStateTransition::DelayRandom:
					// Calculates the animation delay randomly in a range from 0 to 300
					delay = RandomIntRange(0, 300);
					break;
				default:
					// Keep predefined delay
					break;
				}

				*animAddresses.currentlyProcessingSlotIdx = -1;

				// I do not think this can be reached with ASF_UNK2 being set. Check that!
				if (!(slot.flags & ASF_UNK2))
				{
					if (slot.flags & ASF_ACTIVE) {
						// This actually seems to be the "GOOD" case
						rescheduleEvent(delay, slot, evt);
						animAddresses.DrainCompletedQueue();
						return;
					}
					animAddresses.DrainCompletedQueue();
					return;
				}

				break;
			}
			if (unkFlag) {
				break;
			}

			// If no delay has been set, the next state is immediately processed
		}
	}

	if (slot.animObj) {
		// Interrupt everything for the slot
		animAddresses.Interrupt(slot.id, AGP_HIGHEST);

		if (objects.IsCritter(slot.animObj)) {
			PushCompletedAnimation(slot);
		}
	}

	animAddresses.DrainCompletedQueue();
}

static class AnimFix : public TempleFix {
public:
	const char* name() override {
		return "Anim Fix";
	}

	void apply() override {

		return; // Currently not used

		map<uint32_t, string> goalFuncNames;
		map<uint32_t, string> goalFuncDescs;
#define MakeName(a, b) goalFuncNames[a] = b
#define MakeDesc(a, b) goalFuncDescs[a] = b
#include "goalfuncnames.h"

		for (int i = 0; i < ag_count; ++i) {
			auto goal = animAddresses.goals[i];
			if (!goal)
				continue;
			auto goalName = animGoalTypeNames[i];

			string diagramText = "@startuml\n";

			diagramText += "[*] --> state0\n";

			for (int j = 0; j < goal->statecount; ++j) {
				auto& state = goal->states[j];

				auto stateText = "**" + goalFuncNames[(uint32_t) state.callback] + "**";
				auto stateDesc = goalFuncDescs[(uint32_t)state.callback];

				auto param1 = state.argInfo1;
				if (param1 != -1) {
					string param1Name;
					if (param1 <= 19) {
						param1Name = AnimGoalDataNames[param1];
					} else {
						param1Name = format("SPECIAL:{}", param1);
					}
					stateText += "\\nParam 1: " + param1Name;
				}

				auto param2 = state.argInfo2;
				if (param2 != -1) {
					string param2Name;
					if (param2 <= 19) {
						param2Name = AnimGoalDataNames[param2];
					} else {
						param2Name = format("SPECIAL:{}", param2);
					}
					stateText += "\\nParam 2:" + param2Name;
				}

				getTransitionText(diagramText, j, state.afterSuccess, "true");
				getTransitionText(diagramText, j, state.afterFailure, "false");

				diagramText += format("state \"{}\" as state{}\n", stateText, j);
			}

			diagramText += "\n@enduml\n";

			CreateDirectoryA("animation_goals", NULL);
			ofstream o(format("animation_goals/{:02d}_{}.txt", i, goalName));
			o << diagramText;
		}

		logger->info("DONE");
	}
} animFix;

static PyObject* animDump(PyObject* ignore, PyObject* args) {

	for (uint32_t i = 0; i < party.GroupPCsLen(); i++) {
		objHndl objHndPC = party.GroupPCsGetMemberN(i);
		if (!objects.IsDeadNullDestroyed(objHndPC)) {
			animAddresses.PushGoalDying(objHndPC, 1);
		}
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef animDumpDescr = {
	"do_anims",
	(PyCFunction)animDump,
	METH_VARARGS,
	NULL
};

// Puts some debugging stuff into python global scope
static class AnimPythonExtension : PythonGlobalExtension {
public:
	void extend(PyObject* globals) override {
		auto animDump = PyCFunction_New(&animDumpDescr, NULL);
		PyDict_SetItemString(globals, "do_anims", animDump);

	}
} extension;

static struct AnimationAdresses : AddressTable {
	
	bool (__cdecl *PushRotate)(objHndl obj, float rotation);

	bool (__cdecl *PushUseSkillOn)(objHndl actor, objHndl target, objHndl scratchObj, SkillEnum skill, int goalFlags);

	bool (__cdecl *PushRunNearTile)(objHndl actor, LocAndOffsets target, int radiusFeet);

	bool (__cdecl *PushUnconceal)(objHndl actor);

	bool (__cdecl *Interrupt)(objHndl actor, AnimGoalPriority priority, bool all);

	void(__cdecl *PushFallDown)(objHndl actor, int unk);

	AnimationAdresses() {
		
		rebase(PushUseSkillOn, 0x1001C690);
		rebase(PushRunNearTile, 0x1001C1B0);
		rebase(PushRotate, 0x100153E0);
		rebase(PushFallDown, 0x100157B0);
		rebase(PushUnconceal, 0x10015E00);
		
		rebase(Interrupt, 0x1000C7E0);
	}

} addresses;

AnimationGoals animationGoals;

bool AnimationGoals::PushRotate(objHndl obj, float rotation) {
	return addresses.PushRotate(obj, rotation);
}

bool AnimationGoals::PushUseSkillOn(objHndl actor, objHndl target, SkillEnum skill, objHndl scratchObj, int goalFlags) {
	return addresses.PushUseSkillOn(actor, target, scratchObj, skill, goalFlags);
}

bool AnimationGoals::PushRunNearTile(objHndl actor, LocAndOffsets target, int radiusFeet) {
	return addresses.PushRunNearTile(actor, target, radiusFeet);
}

bool AnimationGoals::PushUnconceal(objHndl actor) {
	return addresses.PushUnconceal(actor);
}

bool AnimationGoals::Interrupt(objHndl actor, AnimGoalPriority priority, bool all) {
	return addresses.Interrupt(actor, priority, all);
}

void AnimationGoals::PushFallDown(objHndl actor, int unk)
{
	addresses.PushFallDown(actor, unk);
}