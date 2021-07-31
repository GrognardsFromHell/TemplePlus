
#pragma once

#include <cstdint>

#include "animgoal.h"

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
	ag_dodge = 0x20, // this is where the enums start to be off compared to the debug string in the dll (they added ag_dodge and didn't update the list)
	ag_dying,
	ag_destroy_obj,
	ag_use_skill_on,
	ag_attempt_use_skill_on,
	ag_skill_conceal,
	ag_projectile,
	ag_throw_item,
	ag_use_object,
	ag_use_item_on_object,
	ag_use_item_on_object_with_skill,
	ag_use_item_on_tile,
	ag_use_item_on_tile_with_skill,
	ag_knockback,
	ag_floating,
	ag_close_door,
	ag_attempt_close_door,
	ag_animate_reverse,
	ag_move_away_from_obj,
	ag_rotate,
	ag_unconceal,
	ag_run_near_tile,
	ag_run_near_obj,
	ag_animate_stunned,
	ag_animate_kneel_magic_hands,
	ag_attempt_move_near,
	ag_knock_down,
	ag_anim_get_up,
	ag_attempt_move_straight_knockback,
	ag_wander,
	ag_wander_seek_darkness,
	ag_use_picklock_skill_on,
	ag_please_move,
	ag_attempt_spread_out,
	ag_animate_door_open,
	ag_animate_door_closed,
	ag_pend_closing_door,
	ag_throw_spell_friendly,
	ag_attempt_spell_friendly,
	ag_animate_loop_fire_dmg,
	ag_attempt_move_straight_spell,
	ag_move_near_obj_combat,
	ag_attempt_move_near_combat,
	ag_use_container,
	ag_throw_spell_w_cast_anim,
	ag_attempt_spell_w_cast_anim,
	ag_throw_spell_w_cast_anim_2ndary,
	ag_back_off_from,
	ag_attempt_use_pickpocket_skill_on,
	ag_use_disable_device_skill_on_data,
	ag_count = 82
};

// Allows for direct use of AnimSlotId in format() strings
std::string_view GetAnimGoalTypeName(AnimGoalType type);

namespace fmt {
    template<>
    struct formatter<AnimGoalType> : simple_formatter {
        template<typename FormatContext>
        auto format(const AnimGoalType &type, FormatContext &ctx) {
            return format_to(ctx.out(), GetAnimGoalTypeName(type));
        }
    };
}

class AnimationGoals {
public:

	AnimationGoals();
	
	const AnimGoal &GetByType(AnimGoalType type) const;

	bool IsValidType(AnimGoalType type) const;

private:
	std::unordered_map<AnimGoalType, AnimGoal> goals_;
};
