
#pragma once
#include "obj.h"
#include "skill.h"


#pragma pack(push, 1)
struct AnimSlotId {
	int slotIndex;
	int uniqueId;
	int field_8;

	string toString() const {
		return format("[{}:{}r{}]", slotIndex, uniqueId, field_8);
	}
};
#pragma pack(pop)

// Allows for direct use of AnimSlotId in format() strings
inline ostream &operator<<(ostream &str, const AnimSlotId &id) {
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

std::string GetAnimGoalTypeName(AnimGoalType type);

ostream &operator<<(ostream &str, AnimGoalType type);

enum AnimGoalPriority {
	AGP_NONE = 0,
	AGP_1 = 1,
	AGP_2 = 2,
	AGP_3 = 3,
	AGP_4 = 4,
	AGP_5 = 5,
	AGP_HIGHEST = 6,
	AGP_7 = 7,	
	AGP_MAX = 8
};

class AnimationGoals {
public:

	/*
		Pushes a goal for the obj that will rotate it to the given rotation using its rotation speed.
		Please note that the target rotation is absolute, not relative.
	*/
	bool PushRotate(objHndl obj, float rotation);

	/*
		Pushes a goal for the actor to use a certain skill on the given target.
	*/
	bool PushUseSkillOn(objHndl actor, objHndl target, SkillEnum skill, objHndl scratchObj = 0, int goalFlags = 0);

	/*
		Pushes a goal for the actor to run near a certain tile.
	*/
	bool PushRunNearTile(objHndl actor, LocAndOffsets target, int radiusFeet);

	/*
		Pushes a goal to play the unconceal animation and unconceal the critter.
	*/
	bool PushUnconceal(objHndl actor);

	/*
		Interrupts currently running animation goals up to the given priority.
	*/
	bool Interrupt(objHndl actor, AnimGoalPriority priority, bool all = false);

	void PushFallDown(objHndl actor, int unk);

	int PushAttackAnim(objHndl actor, objHndl target, int unk1, int hitAnimIdx, int playCrit, int useSecondaryAnim);
	int GetAnimIdSthgSub_1001ABB0(objHndl objHndl);
	int PushAttemptAttack(objHndl attacker, objHndl defender);
};

extern AnimationGoals animationGoals;
