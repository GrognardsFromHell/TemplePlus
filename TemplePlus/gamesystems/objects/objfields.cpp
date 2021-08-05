
#include "stdafx.h"

#include "objfields.h"

#include <fstream>

#pragma region Static Field Mappings
static const std::unordered_map<obj_f, ObjectFieldType> sFieldTypeMapping = {
	{ obj_f_begin, ObjectFieldType::BeginSection },
	{ obj_f_location, ObjectFieldType::Int64 },
	{ obj_f_offset_x, ObjectFieldType::Float32 },
	{ obj_f_offset_y, ObjectFieldType::Float32 },
	{ obj_f_2D_shadow_art, ObjectFieldType::Int32 },
	{ obj_f_blit_flags, ObjectFieldType::Int32 },
	{ obj_f_blit_color, ObjectFieldType::Int32 },
	{ obj_f_transparency, ObjectFieldType::Int32 },
	{ obj_f_model_scale, ObjectFieldType::Int32 },
	{ obj_f_light_flags, ObjectFieldType::Int32 },
	{ obj_f_light_material, ObjectFieldType::Int32 },
	{ obj_f_light_color, ObjectFieldType::Int32 },
	{ obj_f_light_radius, ObjectFieldType::Float32 },
	{ obj_f_light_angle_start, ObjectFieldType::Float32 },
	{ obj_f_light_angle_end, ObjectFieldType::Float32 },
	{ obj_f_light_type, ObjectFieldType::Int32 },
	{ obj_f_light_facing_X, ObjectFieldType::Float32 },
	{ obj_f_light_facing_Y, ObjectFieldType::Float32 },
	{ obj_f_light_facing_Z, ObjectFieldType::Float32 },
	{ obj_f_light_offset_X, ObjectFieldType::Float32 },
	{ obj_f_light_offset_Y, ObjectFieldType::Float32 },
	{ obj_f_light_offset_Z, ObjectFieldType::Float32 },
	{ obj_f_flags, ObjectFieldType::Int32 },
	{ obj_f_spell_flags, ObjectFieldType::Int32 },
	{ obj_f_name, ObjectFieldType::Int32 },
	{ obj_f_description, ObjectFieldType::Int32 },
	{ obj_f_size, ObjectFieldType::Int32 },
	{ obj_f_hp_pts, ObjectFieldType::Int32 },
	{ obj_f_hp_adj, ObjectFieldType::Int32 },
	{ obj_f_hp_damage, ObjectFieldType::Int32 },
	{ obj_f_material, ObjectFieldType::Int32 },
	{ obj_f_scripts_idx, ObjectFieldType::ScriptArray },
	{ obj_f_sound_effect, ObjectFieldType::Int32 },
	{ obj_f_category, ObjectFieldType::Int32 },
	{ obj_f_rotation, ObjectFieldType::Float32 },
	{ obj_f_speed_walk, ObjectFieldType::Float32 },
	{ obj_f_speed_run, ObjectFieldType::Float32 },
	{ obj_f_base_mesh, ObjectFieldType::Int32 },
	{ obj_f_base_anim, ObjectFieldType::Int32 },
	{ obj_f_radius, ObjectFieldType::Float32 },
	{ obj_f_3d_render_height, ObjectFieldType::Float32 },
	{ obj_f_conditions, ObjectFieldType::Int32Array },
	{ obj_f_condition_arg0, ObjectFieldType::Int32Array },
	{ obj_f_permanent_mods, ObjectFieldType::Int32Array },
	{ obj_f_initiative, ObjectFieldType::Int32 },
	{ obj_f_dispatcher, ObjectFieldType::Int32 },
	{ obj_f_subinitiative, ObjectFieldType::Int32 },
	{ obj_f_secretdoor_flags, ObjectFieldType::Int32 },
	{ obj_f_secretdoor_effectname, ObjectFieldType::Int32 },
	{ obj_f_secretdoor_dc, ObjectFieldType::Int32 },
	{ obj_f_pad_i_7, ObjectFieldType::Int32 },
	{ obj_f_pad_i_8, ObjectFieldType::Int32 },
	{ obj_f_pad_i_9, ObjectFieldType::Int32 },
	{ obj_f_pad_i_0, ObjectFieldType::Int32 },
	{ obj_f_offset_z, ObjectFieldType::Float32 },
	{ obj_f_rotation_pitch, ObjectFieldType::Float32 },
	{ obj_f_pad_f_3, ObjectFieldType::Float32 },
	{ obj_f_pad_f_4, ObjectFieldType::Float32 },
	{ obj_f_pad_f_5, ObjectFieldType::Float32 },
	{ obj_f_pad_f_6, ObjectFieldType::Float32 },
	{ obj_f_pad_f_7, ObjectFieldType::Float32 },
	{ obj_f_pad_f_8, ObjectFieldType::Float32 },
	{ obj_f_pad_f_9, ObjectFieldType::Float32 },
	{ obj_f_pad_f_0, ObjectFieldType::Float32 },
	{ obj_f_pad_i64_0, ObjectFieldType::Int64 },
	{ obj_f_pad_i64_1, ObjectFieldType::Int64 },
	{ obj_f_pad_i64_2, ObjectFieldType::Int64 },
	{ obj_f_pad_i64_3, ObjectFieldType::Int64 },
	{ obj_f_pad_i64_4, ObjectFieldType::Int64 },
	{ obj_f_last_hit_by, ObjectFieldType::Obj },
	{ obj_f_pad_obj_1, ObjectFieldType::Obj },
	{ obj_f_pad_obj_2, ObjectFieldType::Obj },
	{ obj_f_pad_obj_3, ObjectFieldType::Obj },
	{ obj_f_pad_obj_4, ObjectFieldType::Obj },
	{ obj_f_permanent_mod_data, ObjectFieldType::Int32Array },
	{ obj_f_attack_types_idx, ObjectFieldType::Int32Array },
	{ obj_f_attack_bonus_idx, ObjectFieldType::Int32Array },
	{ obj_f_strategy_state, ObjectFieldType::Int32Array },
	{ obj_f_pad_ias_4, ObjectFieldType::Int32Array },
	{ obj_f_pad_i64as_0, ObjectFieldType::Int64Array },
	{ obj_f_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_pad_i64as_2, ObjectFieldType::Int64Array },
	{ obj_f_pad_i64as_3, ObjectFieldType::Int64Array },
	{ obj_f_pad_i64as_4, ObjectFieldType::Int64Array },
	{ obj_f_pad_objas_0, ObjectFieldType::ObjArray },
	{ obj_f_pad_objas_1, ObjectFieldType::ObjArray },
	{ obj_f_pad_objas_2, ObjectFieldType::ObjArray },
	{ obj_f_end, ObjectFieldType::EndSection },
	{ obj_f_portal_begin, ObjectFieldType::BeginSection },
	{ obj_f_portal_flags, ObjectFieldType::Int32 },
	{ obj_f_portal_lock_dc, ObjectFieldType::Int32 },
	{ obj_f_portal_key_id, ObjectFieldType::Int32 },
	{ obj_f_portal_notify_npc, ObjectFieldType::Int32 },
	{ obj_f_portal_pad_i_1, ObjectFieldType::Int32 },
	{ obj_f_portal_pad_i_2, ObjectFieldType::Int32 },
	{ obj_f_portal_pad_i_3, ObjectFieldType::Int32 },
	{ obj_f_portal_pad_i_4, ObjectFieldType::Int32 },
	{ obj_f_portal_pad_i_5, ObjectFieldType::Int32 },
	{ obj_f_portal_pad_obj_1, ObjectFieldType::Obj },
	{ obj_f_portal_pad_ias_1, ObjectFieldType::Int32Array },
	{ obj_f_portal_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_portal_end, ObjectFieldType::EndSection },
	{ obj_f_container_begin, ObjectFieldType::BeginSection },
	{ obj_f_container_flags, ObjectFieldType::Int32 },
	{ obj_f_container_lock_dc, ObjectFieldType::Int32 },
	{ obj_f_container_key_id, ObjectFieldType::Int32 },
	{ obj_f_container_inventory_num, ObjectFieldType::Int32 },
	{ obj_f_container_inventory_list_idx, ObjectFieldType::ObjArray },
	{ obj_f_container_inventory_source, ObjectFieldType::Int32 },
	{ obj_f_container_notify_npc, ObjectFieldType::Int32 },
	{ obj_f_container_pad_i_1, ObjectFieldType::Int32 },
	{ obj_f_container_pad_i_2, ObjectFieldType::Int32 },
	{ obj_f_container_pad_i_3, ObjectFieldType::Int32 },
	{ obj_f_container_pad_i_4, ObjectFieldType::Int32 },
	{ obj_f_container_pad_i_5, ObjectFieldType::Int32 },
	{ obj_f_container_pad_obj_1, ObjectFieldType::Obj },
	{ obj_f_container_pad_obj_2, ObjectFieldType::Obj },
	{ obj_f_container_pad_ias_1, ObjectFieldType::Int32Array },
	{ obj_f_container_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_container_pad_objas_1, ObjectFieldType::ObjArray },
	{ obj_f_container_end, ObjectFieldType::EndSection },
	{ obj_f_scenery_begin, ObjectFieldType::BeginSection },
	{ obj_f_scenery_flags, ObjectFieldType::Int32 },
	{ obj_f_scenery_pad_obj_0, ObjectFieldType::Obj },
	{ obj_f_scenery_respawn_delay, ObjectFieldType::Int32 },
	{ obj_f_scenery_pad_i_0, ObjectFieldType::Int32 },
	{ obj_f_scenery_pad_i_1, ObjectFieldType::Int32 },
	{ obj_f_scenery_teleport_to, ObjectFieldType::Int32 },
	{ obj_f_scenery_pad_i_4, ObjectFieldType::Int32 },
	{ obj_f_scenery_pad_i_5, ObjectFieldType::Int32 },
	{ obj_f_scenery_pad_obj_1, ObjectFieldType::Int32 },
	{ obj_f_scenery_pad_ias_1, ObjectFieldType::Int32Array },
	{ obj_f_scenery_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_scenery_end, ObjectFieldType::EndSection },
	{ obj_f_projectile_begin, ObjectFieldType::BeginSection },
	{ obj_f_projectile_flags_combat, ObjectFieldType::Int32 },
	{ obj_f_projectile_flags_combat_damage, ObjectFieldType::Int32 },
	{ obj_f_projectile_parent_weapon, ObjectFieldType::Obj },
	{ obj_f_projectile_parent_ammo, ObjectFieldType::Obj },
	{ obj_f_projectile_part_sys_id, ObjectFieldType::Int32 },
	{ obj_f_projectile_acceleration_x, ObjectFieldType::Float32 },
	{ obj_f_projectile_acceleration_y, ObjectFieldType::Float32 },
	{ obj_f_projectile_acceleration_z, ObjectFieldType::Float32 },
	{ obj_f_projectile_pad_i_4, ObjectFieldType::Int32 },
	{ obj_f_projectile_pad_obj_1, ObjectFieldType::Obj },
	{ obj_f_projectile_pad_obj_2, ObjectFieldType::Obj },
	{ obj_f_projectile_pad_obj_3, ObjectFieldType::Obj },
	{ obj_f_projectile_pad_ias_1, ObjectFieldType::Int32Array },
	{ obj_f_projectile_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_projectile_pad_objas_1, ObjectFieldType::ObjArray },
	{ obj_f_projectile_end, ObjectFieldType::EndSection },
	{ obj_f_item_begin, ObjectFieldType::BeginSection },
	{ obj_f_item_flags, ObjectFieldType::Int32 },
	{ obj_f_item_parent, ObjectFieldType::Obj },
	{ obj_f_item_weight, ObjectFieldType::Int32 },
	{ obj_f_item_worth, ObjectFieldType::Int32 },
	{ obj_f_item_inv_aid, ObjectFieldType::Int32 },
	{ obj_f_item_inv_location, ObjectFieldType::Int32 },
	{ obj_f_item_ground_mesh, ObjectFieldType::Int32 },
	{ obj_f_item_ground_anim, ObjectFieldType::Int32 },
	{ obj_f_item_description_unknown, ObjectFieldType::Int32 },
	{ obj_f_item_description_effects, ObjectFieldType::Int32 },
	{ obj_f_item_spell_idx, ObjectFieldType::SpellArray },
	{ obj_f_item_spell_idx_flags, ObjectFieldType::Int32 },
	{ obj_f_item_spell_charges_idx, ObjectFieldType::Int32 },
	{ obj_f_item_ai_action, ObjectFieldType::Int32 },
	{ obj_f_item_wear_flags, ObjectFieldType::Int32 },
	{ obj_f_item_material_slot, ObjectFieldType::Int32 },
	{ obj_f_item_quantity, ObjectFieldType::Int32 },
	{ obj_f_item_pad_i_1, ObjectFieldType::Int32 },
	{ obj_f_item_pad_i_2, ObjectFieldType::Int32 },
	{ obj_f_item_pad_i_3, ObjectFieldType::Int32 },
	{ obj_f_item_pad_i_4, ObjectFieldType::Int32 },
	{ obj_f_item_pad_i_5, ObjectFieldType::Int32 },
	{ obj_f_item_pad_i_6, ObjectFieldType::Int32 },
	{ obj_f_item_pad_obj_1, ObjectFieldType::Obj },
	{ obj_f_item_pad_obj_2, ObjectFieldType::Obj },
	{ obj_f_item_pad_obj_3, ObjectFieldType::Obj },
	{ obj_f_item_pad_obj_4, ObjectFieldType::Obj },
	{ obj_f_item_pad_obj_5, ObjectFieldType::Obj },
	{ obj_f_item_pad_wielder_condition_array, ObjectFieldType::Int32Array },
	{ obj_f_item_pad_wielder_argument_array, ObjectFieldType::Int32Array },
	{ obj_f_item_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_item_pad_i64as_2, ObjectFieldType::Int64Array },
	{ obj_f_item_pad_objas_1, ObjectFieldType::ObjArray },
	{ obj_f_item_pad_objas_2, ObjectFieldType::ObjArray },
	{ obj_f_item_end, ObjectFieldType::EndSection },
	{ obj_f_weapon_begin, ObjectFieldType::BeginSection },
	{ obj_f_weapon_flags, ObjectFieldType::Int32 },
	{ obj_f_weapon_range, ObjectFieldType::Int32 },
	{ obj_f_weapon_ammo_type, ObjectFieldType::Int32 },
	{ obj_f_weapon_ammo_consumption, ObjectFieldType::Int32 },
	{ obj_f_weapon_missile_aid, ObjectFieldType::Int32 },
	{ obj_f_weapon_crit_hit_chart, ObjectFieldType::Int32 },
	{ obj_f_weapon_attacktype, ObjectFieldType::Int32 },
	{ obj_f_weapon_damage_dice, ObjectFieldType::Int32 },
	{ obj_f_weapon_animtype, ObjectFieldType::Int32 },
	{ obj_f_weapon_type, ObjectFieldType::Int32 },
	{ obj_f_weapon_crit_range, ObjectFieldType::Int32 },
	{ obj_f_weapon_pad_i_1, ObjectFieldType::Int32 },
	{ obj_f_weapon_pad_i_2, ObjectFieldType::Int32 },
	{ obj_f_weapon_pad_obj_1, ObjectFieldType::Obj },
	{ obj_f_weapon_pad_obj_2, ObjectFieldType::Obj },
	{ obj_f_weapon_pad_obj_3, ObjectFieldType::Obj },
	{ obj_f_weapon_pad_obj_4, ObjectFieldType::Obj },
	{ obj_f_weapon_pad_obj_5, ObjectFieldType::Obj },
	{ obj_f_weapon_pad_ias_1, ObjectFieldType::Int32Array },
	{ obj_f_weapon_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_weapon_end, ObjectFieldType::EndSection },
	{ obj_f_ammo_begin, ObjectFieldType::BeginSection },
	{ obj_f_ammo_flags, ObjectFieldType::Int32 },
	{ obj_f_ammo_quantity, ObjectFieldType::Int32 },
	{ obj_f_ammo_type, ObjectFieldType::Int32 },
	{ obj_f_ammo_pad_i_1, ObjectFieldType::Int32 },
	{ obj_f_ammo_pad_i_2, ObjectFieldType::Int32 },
	{ obj_f_ammo_pad_obj_1, ObjectFieldType::Obj },
	{ obj_f_ammo_pad_ias_1, ObjectFieldType::Int32Array },
	{ obj_f_ammo_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_ammo_end, ObjectFieldType::EndSection },
	{ obj_f_armor_begin, ObjectFieldType::BeginSection },
	{ obj_f_armor_flags, ObjectFieldType::Int32 },
	{ obj_f_armor_ac_adj, ObjectFieldType::Int32 },
	{ obj_f_armor_max_dex_bonus, ObjectFieldType::Int32 },
	{ obj_f_armor_arcane_spell_failure, ObjectFieldType::Int32 },
	{ obj_f_armor_armor_check_penalty, ObjectFieldType::Int32 },
	{ obj_f_armor_pad_i_1, ObjectFieldType::Int32 },
	{ obj_f_armor_pad_ias_1, ObjectFieldType::Int32Array },
	{ obj_f_armor_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_armor_end, ObjectFieldType::EndSection },
	{ obj_f_money_begin, ObjectFieldType::BeginSection },
	{ obj_f_money_flags, ObjectFieldType::Int32 },
	{ obj_f_money_quantity, ObjectFieldType::Int32 },
	{ obj_f_money_type, ObjectFieldType::Int32 },
	{ obj_f_money_pad_i_1, ObjectFieldType::Int32 },
	{ obj_f_money_pad_i_2, ObjectFieldType::Int32 },
	{ obj_f_money_pad_i_3, ObjectFieldType::Int32 },
	{ obj_f_money_pad_i_4, ObjectFieldType::Int32 },
	{ obj_f_money_pad_i_5, ObjectFieldType::Int32 },
	{ obj_f_money_pad_ias_1, ObjectFieldType::Int32Array },
	{ obj_f_money_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_money_end, ObjectFieldType::EndSection },
	{ obj_f_food_begin, ObjectFieldType::BeginSection },
	{ obj_f_food_flags, ObjectFieldType::Int32 },
	{ obj_f_food_pad_i_1, ObjectFieldType::Int32 },
	{ obj_f_food_pad_i_2, ObjectFieldType::Int32 },
	{ obj_f_food_pad_ias_1, ObjectFieldType::Int32Array },
	{ obj_f_food_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_food_end, ObjectFieldType::EndSection },
	{ obj_f_scroll_begin, ObjectFieldType::BeginSection },
	{ obj_f_scroll_flags, ObjectFieldType::Int32 },
	{ obj_f_scroll_pad_i_1, ObjectFieldType::Int32 },
	{ obj_f_scroll_pad_i_2, ObjectFieldType::Int32 },
	{ obj_f_scroll_pad_ias_1, ObjectFieldType::Int32Array },
	{ obj_f_scroll_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_scroll_end, ObjectFieldType::EndSection },
	{ obj_f_key_begin, ObjectFieldType::BeginSection },
	{ obj_f_key_key_id, ObjectFieldType::Int32 },
	{ obj_f_key_pad_i_1, ObjectFieldType::Int32 },
	{ obj_f_key_pad_i_2, ObjectFieldType::Int32 },
	{ obj_f_key_pad_ias_1, ObjectFieldType::Int32Array },
	{ obj_f_key_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_key_end, ObjectFieldType::EndSection },
	{ obj_f_written_begin, ObjectFieldType::BeginSection },
	{ obj_f_written_flags, ObjectFieldType::Int32 },
	{ obj_f_written_subtype, ObjectFieldType::Int32 },
	{ obj_f_written_text_start_line, ObjectFieldType::Int32 },
	{ obj_f_written_text_end_line, ObjectFieldType::Int32 },
	{ obj_f_written_pad_i_1, ObjectFieldType::Int32 },
	{ obj_f_written_pad_i_2, ObjectFieldType::Int32 },
	{ obj_f_written_pad_ias_1, ObjectFieldType::Int32Array },
	{ obj_f_written_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_written_end, ObjectFieldType::EndSection },
	{ obj_f_bag_begin, ObjectFieldType::BeginSection },
	{ obj_f_bag_flags, ObjectFieldType::Int32 },
	{ obj_f_bag_size, ObjectFieldType::Int32 },
	{ obj_f_bag_end, ObjectFieldType::EndSection },
	{ obj_f_generic_begin, ObjectFieldType::BeginSection },
	{ obj_f_generic_flags, ObjectFieldType::Int32 },
	{ obj_f_generic_usage_bonus, ObjectFieldType::Int32 },
	{ obj_f_generic_usage_count_remaining, ObjectFieldType::Int32 },
	{ obj_f_generic_pad_ias_1, ObjectFieldType::Int32Array },
	{ obj_f_generic_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_generic_end, ObjectFieldType::EndSection },
	{ obj_f_critter_begin, ObjectFieldType::BeginSection },
	{ obj_f_critter_flags, ObjectFieldType::Int32 },
	{ obj_f_critter_flags2, ObjectFieldType::Int32 },
	{ obj_f_critter_abilities_idx, ObjectFieldType::AbilityArray },
	{ obj_f_critter_level_idx, ObjectFieldType::Int32Array },
	{ obj_f_critter_race, ObjectFieldType::Int32 },
	{ obj_f_critter_gender, ObjectFieldType::Int32 },
	{ obj_f_critter_age, ObjectFieldType::Int32 },
	{ obj_f_critter_height, ObjectFieldType::Int32 },
	{ obj_f_critter_weight, ObjectFieldType::Int32 },
	{ obj_f_critter_experience, ObjectFieldType::Int32 },
	{ obj_f_critter_pad_i_1, ObjectFieldType::Int32 },
	{ obj_f_critter_alignment, ObjectFieldType::Int32 },
	{ obj_f_critter_deity, ObjectFieldType::Int32 },
	{ obj_f_critter_domain_1, ObjectFieldType::Int32 },
	{ obj_f_critter_domain_2, ObjectFieldType::Int32 },
	{ obj_f_critter_alignment_choice, ObjectFieldType::Int32 },
	{ obj_f_critter_school_specialization, ObjectFieldType::Int32 },
	{ obj_f_critter_spells_known_idx, ObjectFieldType::SpellArray },
	{ obj_f_critter_spells_memorized_idx, ObjectFieldType::SpellArray },
	{ obj_f_critter_spells_cast_idx, ObjectFieldType::SpellArray },
	{ obj_f_critter_feat_idx, ObjectFieldType::Int32Array },
	{ obj_f_critter_feat_count_idx, ObjectFieldType::Int32Array },
	{ obj_f_critter_fleeing_from, ObjectFieldType::Obj },
	{ obj_f_critter_portrait, ObjectFieldType::Int32 },
	{ obj_f_critter_money_idx, ObjectFieldType::Int32Array },
	{ obj_f_critter_inventory_num, ObjectFieldType::Int32 },
	{ obj_f_critter_inventory_list_idx, ObjectFieldType::ObjArray },
	{ obj_f_critter_inventory_source, ObjectFieldType::Int32 },
	{ obj_f_critter_description_unknown, ObjectFieldType::Int32 },
	{ obj_f_critter_follower_idx, ObjectFieldType::ObjArray },
	{ obj_f_critter_teleport_dest, ObjectFieldType::Int64 },
	{ obj_f_critter_teleport_map, ObjectFieldType::Int32 },
	{ obj_f_critter_death_time, ObjectFieldType::Int32 },
	{ obj_f_critter_skill_idx, ObjectFieldType::Int32Array },
	{ obj_f_critter_reach, ObjectFieldType::Int32 },
	{ obj_f_critter_subdual_damage, ObjectFieldType::Int32 },
	{ obj_f_critter_pad_i_4, ObjectFieldType::Int32 },
	{ obj_f_critter_pad_i_5, ObjectFieldType::Int32 },
	{ obj_f_critter_sequence, ObjectFieldType::Int32 },
	{ obj_f_critter_hair_style, ObjectFieldType::Int32 },
	{ obj_f_critter_strategy, ObjectFieldType::Int32 },
	{ obj_f_critter_pad_i_3, ObjectFieldType::Int32 },
	{ obj_f_critter_monster_category, ObjectFieldType::Int64 },
	{ obj_f_critter_pad_i64_2, ObjectFieldType::Int64 },
	{ obj_f_critter_pad_i64_3, ObjectFieldType::Int64 },
	{ obj_f_critter_pad_i64_4, ObjectFieldType::Int64 },
	{ obj_f_critter_pad_i64_5, ObjectFieldType::Int64 },
	{ obj_f_critter_damage_idx, ObjectFieldType::Int32Array },
	{ obj_f_critter_attacks_idx, ObjectFieldType::Int32Array },
	{ obj_f_critter_seen_maplist, ObjectFieldType::Int64Array },
	{ obj_f_critter_pad_i64as_2, ObjectFieldType::Int64Array },
	{ obj_f_critter_pad_i64as_3, ObjectFieldType::Int64Array },
	{ obj_f_critter_pad_i64as_4, ObjectFieldType::Int64Array },
	{ obj_f_critter_pad_i64as_5, ObjectFieldType::Int64Array },
	{ obj_f_critter_end, ObjectFieldType::EndSection },
	{ obj_f_pc_begin, ObjectFieldType::BeginSection },
	{ obj_f_pc_flags, ObjectFieldType::Int32 },
	{ obj_f_pc_pad_ias_0, ObjectFieldType::Int32Array },
	{ obj_f_pc_pad_i64as_0, ObjectFieldType::Int64Array },
	{ obj_f_pc_player_name, ObjectFieldType::String },
	{ obj_f_pc_global_flags, ObjectFieldType::Int32Array },
	{ obj_f_pc_global_variables, ObjectFieldType::Int32Array },
	{ obj_f_pc_voice_idx, ObjectFieldType::Int32 },
	{ obj_f_pc_roll_count, ObjectFieldType::Int32 },
	{ obj_f_pc_pad_i_2, ObjectFieldType::Int32 },
	{ obj_f_pc_weaponslots_idx, ObjectFieldType::Int32Array },
	{ obj_f_pc_pad_ias_2, ObjectFieldType::Int32Array },
	{ obj_f_pc_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_pc_end, ObjectFieldType::EndSection },
	{ obj_f_npc_begin, ObjectFieldType::BeginSection },
	{ obj_f_npc_flags, ObjectFieldType::Int32 },
	{ obj_f_npc_leader, ObjectFieldType::Obj },
	{ obj_f_npc_ai_data, ObjectFieldType::Int32 },
	{ obj_f_npc_combat_focus, ObjectFieldType::Obj },
	{ obj_f_npc_who_hit_me_last, ObjectFieldType::Obj },
	{ obj_f_npc_waypoints_idx, ObjectFieldType::Int64Array },
	{ obj_f_npc_waypoint_current, ObjectFieldType::Int32 },
	{ obj_f_npc_standpoint_day_INTERNAL_DO_NOT_USE, ObjectFieldType::Int64 },
	{ obj_f_npc_standpoint_night_INTERNAL_DO_NOT_USE, ObjectFieldType::Int64 },
	{ obj_f_npc_faction, ObjectFieldType::Int32Array },
	{ obj_f_npc_retail_price_multiplier, ObjectFieldType::Int32 },
	{ obj_f_npc_substitute_inventory, ObjectFieldType::Obj },
	{ obj_f_npc_reaction_base, ObjectFieldType::Int32 },
	{ obj_f_npc_challenge_rating, ObjectFieldType::Int32 },
	{ obj_f_npc_reaction_pc_idx, ObjectFieldType::ObjArray },
	{ obj_f_npc_reaction_level_idx, ObjectFieldType::Int32Array },
	{ obj_f_npc_reaction_time_idx, ObjectFieldType::Int32Array },
	{ obj_f_npc_generator_data, ObjectFieldType::Int32 },
	{ obj_f_npc_ai_list_idx, ObjectFieldType::ObjArray },
	{ obj_f_npc_save_reflexes_bonus, ObjectFieldType::Int32 },
	{ obj_f_npc_save_fortitude_bonus, ObjectFieldType::Int32 },
	{ obj_f_npc_save_willpower_bonus, ObjectFieldType::Int32 },
	{ obj_f_npc_ac_bonus, ObjectFieldType::Int32 },
	{ obj_f_npc_add_mesh, ObjectFieldType::Int32 },
	{ obj_f_npc_waypoint_anim, ObjectFieldType::Int32 },
	{ obj_f_npc_pad_i_3, ObjectFieldType::Int32 },
	{ obj_f_npc_pad_i_4, ObjectFieldType::Int32 },
	{ obj_f_npc_pad_i_5, ObjectFieldType::Int32 },
	{ obj_f_npc_ai_flags64, ObjectFieldType::Int64 },
	{ obj_f_npc_pad_i64_2, ObjectFieldType::Int64 },
	{ obj_f_npc_pad_i64_3, ObjectFieldType::Int64 },
	{ obj_f_npc_pad_i64_4, ObjectFieldType::Int64 },
	{ obj_f_npc_pad_i64_5, ObjectFieldType::Int64 },
	{ obj_f_npc_hitdice_idx, ObjectFieldType::Int32Array },
	{ obj_f_npc_ai_list_type_idx, ObjectFieldType::Int32Array },
	{ obj_f_npc_pad_ias_3, ObjectFieldType::Int32Array },
	{ obj_f_npc_pad_ias_4, ObjectFieldType::Int32Array },
	{ obj_f_npc_pad_ias_5, ObjectFieldType::Int32Array },
	{ obj_f_npc_standpoints, ObjectFieldType::Int64Array },
	{ obj_f_npc_pad_i64as_2, ObjectFieldType::Int64Array },
	{ obj_f_npc_pad_i64as_3, ObjectFieldType::Int64Array },
	{ obj_f_npc_pad_i64as_4, ObjectFieldType::Int64Array },
	{ obj_f_npc_pad_i64as_5, ObjectFieldType::Int64Array },
	{ obj_f_npc_end, ObjectFieldType::EndSection },
	{ obj_f_trap_begin, ObjectFieldType::BeginSection },
	{ obj_f_trap_flags, ObjectFieldType::Int32 },
	{ obj_f_trap_difficulty, ObjectFieldType::Int32 },
	{ obj_f_trap_pad_i_2, ObjectFieldType::Int32 },
	{ obj_f_trap_pad_ias_1, ObjectFieldType::Int32Array },
	{ obj_f_trap_pad_i64as_1, ObjectFieldType::Int64Array },
	{ obj_f_trap_end, ObjectFieldType::EndSection },
	{ obj_f_total_normal, ObjectFieldType::None },
	{ obj_f_transient_begin, ObjectFieldType::None },
	{ obj_f_render_color, ObjectFieldType::Int32 },
	{ obj_f_render_colors, ObjectFieldType::Int32 },
	{ obj_f_render_palette, ObjectFieldType::Int32 },
	{ obj_f_render_scale, ObjectFieldType::Int32 },
	{ obj_f_render_alpha, ObjectFieldType::AbilityArray },
	{ obj_f_render_x, ObjectFieldType::Int32 },
	{ obj_f_render_y, ObjectFieldType::Int32 },
	{ obj_f_render_width, ObjectFieldType::Int32 },
	{ obj_f_render_height, ObjectFieldType::Int32 },
	{ obj_f_palette, ObjectFieldType::Int32 },
	{ obj_f_color, ObjectFieldType::Int32 },
	{ obj_f_colors, ObjectFieldType::Int32 },
	{ obj_f_render_flags, ObjectFieldType::Int32 },
	{ obj_f_temp_id, ObjectFieldType::Int32 },
	{ obj_f_light_handle, ObjectFieldType::Int32 },
	{ obj_f_overlay_light_handles, ObjectFieldType::Int32Array },
	{ obj_f_internal_flags, ObjectFieldType::Int32 },
	{ obj_f_find_node, ObjectFieldType::Int32 },
	{ obj_f_animation_handle, ObjectFieldType::Int32 },
	{ obj_f_grapple_state, ObjectFieldType::Int32 },
	{ obj_f_transient_end, ObjectFieldType::None },
	{ obj_f_type, ObjectFieldType::Int32 },
	{ obj_f_prototype_handle, ObjectFieldType::Obj }
};

// The section headers (excluding transient)
static constexpr auto sSectionCount = 20;
static const std::array<obj_f, sSectionCount> sSectionStarts {
	obj_f_begin,	
	obj_f_portal_begin,
	obj_f_container_begin,
	obj_f_scenery_begin,
	obj_f_projectile_begin,
	obj_f_item_begin,
	obj_f_weapon_begin,
	obj_f_ammo_begin,
	obj_f_armor_begin,
	obj_f_money_begin,
	obj_f_food_begin,
	obj_f_scroll_begin,
	obj_f_key_begin,
	obj_f_written_begin,
	obj_f_bag_begin,
	obj_f_generic_begin,
	obj_f_critter_begin,
	obj_f_pc_begin,
	obj_f_npc_begin,
	obj_f_trap_begin
};

static size_t GetSectionStartIdx(obj_f sectionStart) {
	for (size_t i = 0; i < sSectionStarts.size(); ++i) {
		if (sSectionStarts[i] == sectionStart) {
			return i;
		}
	}
	throw TempleException("Unknown section start: {}", GetObjectFieldName(sectionStart));
}
#pragma endregion

ObjectFields::ObjectFields() {
	int curProtoPropIdx, curArrayIdx;
	int objProtoPropCount, objArrayCount;
	int critterArrayCount, critterProtoPropCount;
	int itemArrayCount, itemProtoPropCount;

	std::array<int, sSectionCount> sectionFirstProtoPropIdx;

	// Create the full property definitions
	for (size_t i = 0; i < mFieldDefs.size(); ++i) {
		auto field = (obj_f)i;

		// Determine the name
		auto fieldName = GetObjectFieldName(field);
		if (!fieldName) {
			throw TempleException("No name for field {}", i);
		}
		mFieldDefs[i].name = fieldName;

		// Determine the type
		auto tIt = sFieldTypeMapping.find(field);
		if (tIt == sFieldTypeMapping.end()) {
			throw TempleException("No type for {}", fieldName);
		}
		auto fieldType = tIt->second;
		mFieldDefs[i].type = fieldType;

		// Determine the width in the property collection
		mFieldDefs[i].storedInPropColl = GetPropCollSize(field, fieldType);
	}

	// For all normal fields, calculate property bitmap indices
	for (obj_f field = obj_f_begin; field < obj_f_total_normal; field = (obj_f)((int) field + 1)) {

		auto fieldType = mFieldDefs[field].type;

		// Handle section markers
		if (fieldType == ObjectFieldType::BeginSection) {

			switch (field) {
			case obj_f_begin:
				curProtoPropIdx = 0;
				curArrayIdx = 0;
				break;
			case obj_f_portal_begin:
			case obj_f_container_begin:
			case obj_f_scenery_begin:
			case obj_f_projectile_begin:
			case obj_f_item_begin:
			case obj_f_critter_begin:
			case obj_f_trap_begin:
				curProtoPropIdx = objProtoPropCount;
				curArrayIdx = objArrayCount;
				break;
			case obj_f_pc_begin:
			case obj_f_npc_begin:
				curProtoPropIdx = critterProtoPropCount;
				curArrayIdx = critterArrayCount;
				break;
			case obj_f_scroll_begin:
			case obj_f_key_begin:
			case obj_f_written_begin:
			case obj_f_bag_begin:
			case obj_f_generic_begin:
			case obj_f_weapon_begin:
			case obj_f_ammo_begin:
			case obj_f_armor_begin:
			case obj_f_money_begin:
			case obj_f_food_begin:
				curProtoPropIdx = itemProtoPropCount;
				curArrayIdx = itemArrayCount;
			default:
				break;
			}

			auto idx = GetSectionStartIdx(field);
			sectionFirstProtoPropIdx[idx] = curProtoPropIdx;

		} else if (fieldType == ObjectFieldType::EndSection) {
			switch (field)
			{
			case obj_f_end:
				objArrayCount = curArrayIdx;
				objProtoPropCount = curProtoPropIdx;
				break;
			case obj_f_item_end:
				itemArrayCount = curArrayIdx;
				itemProtoPropCount = curProtoPropIdx;
				break;
			case obj_f_critter_end:
				critterArrayCount = curArrayIdx;
				critterProtoPropCount = curProtoPropIdx;
				break;
			}
		} else {
			int fieldIdx = (int) field - 1;

			//  Calculate the bitmap block that contains the bit for this field
			int bitmapBlockIdx = fieldIdx / 32;
			int bitmapBitIdx = fieldIdx % 32;

			mFieldDefs[field].bitmapBlockIdx = bitmapBlockIdx;
			mFieldDefs[field].bitmapBitIdx = bitmapBitIdx;
			mFieldDefs[field].bitmapMask = 1 << bitmapBitIdx;
			mFieldDefs[field].protoPropIdx = curProtoPropIdx++;

			if (IsArrayType(fieldType)) {
				mFieldDefs[field].arrayIdx = curArrayIdx++;
			}
		}
	}


	// For each object type, calculate the number of bitmap blocks that are needed
	for (uint32_t i = 0; i < ObjectTypeCount; ++i) {
		ObjectType type = (ObjectType)i;

		// Search for the highest prop idx among the type's fields
		int highestIdx = -1;
		size_t count = 0;
		IterateTypeFields(type, [&](obj_f field) {
			if (mFieldDefs[field].bitmapBlockIdx > highestIdx) {
				highestIdx = mFieldDefs[field].bitmapBlockIdx;
			}
			count++;
			return true;
		});
		mPropCollSizePerType[type] = count;
		mBitmapBlocksPerType[type] = highestIdx + 1;
	}

}

bool ObjectFields::DoesTypeSupportField(ObjectType type, obj_f field)
{
	auto fieldType = mFieldDefs[field].type;

	// Section markers are not supported for storage
	if (fieldType == ObjectFieldType::None
		|| fieldType == ObjectFieldType::BeginSection
		|| fieldType == ObjectFieldType::EndSection) {
		return false;
	}

	// Properties that are supported by all object types
	if (field > obj_f_begin && field < obj_f_end
		|| field > obj_f_transient_begin && field < obj_f_transient_end
		|| field == obj_f_type
		|| field == obj_f_prototype_handle) {
		return true;
	}

	switch (type)
	{
	case obj_t_portal:
		return field > obj_f_portal_begin && field < obj_f_portal_end;
	case obj_t_container:
		return field > obj_f_container_begin && field < obj_f_container_end;
	case obj_t_scenery:
		return field > obj_f_scenery_begin && field < obj_f_scenery_end;
	case obj_t_projectile:
		return field > obj_f_projectile_begin && field < obj_f_projectile_end;
	case obj_t_weapon:
		if (field > obj_f_item_begin && field < obj_f_item_end)
			return true;
		return field > obj_f_weapon_begin && field < obj_f_weapon_end;
	case obj_t_ammo:
		if (field > obj_f_item_begin && field < obj_f_item_end)
			return true;
		return field > obj_f_ammo_begin && field < obj_f_ammo_end;
	case obj_t_armor:
		if (field > obj_f_item_begin && field < obj_f_item_end)
			return true;
		return field > obj_f_armor_begin && field < obj_f_armor_end;
	case obj_t_money:
		if (field > obj_f_item_begin && field < obj_f_item_end)
			return true;
		return field > obj_f_money_begin && field < obj_f_money_end;
	case obj_t_food:
		if (field > obj_f_item_begin && field < obj_f_item_end)
			return true;
		return field > obj_f_food_begin && field < obj_f_food_end;
	case obj_t_scroll:
		if (field > obj_f_item_begin && field < obj_f_item_end)
			return true;
		return field > obj_f_scroll_begin && field < obj_f_scroll_end;
	case obj_t_key:
		if (field > obj_f_item_begin && field < obj_f_item_end)
			return true;
		return field > obj_f_key_begin && field < obj_f_key_end;
	case obj_t_written:
		if (field > obj_f_item_begin && field < obj_f_item_end)
			return true;
		return field > obj_f_written_begin && field < obj_f_written_end;
	case obj_t_bag:
		if (field > obj_f_item_begin && field < obj_f_item_end)
			return true;
		return field > obj_f_bag_begin && field < obj_f_bag_end;
	case obj_t_generic:
		if (field > obj_f_item_begin && field < obj_f_item_end)
			return true;
		return field > obj_f_generic_begin && field < obj_f_generic_end;
	case obj_t_pc:
		if (field > obj_f_critter_begin && field < obj_f_critter_end)
			return true;
		return field > obj_f_pc_begin && field < obj_f_pc_end;
	case obj_t_npc:
		if (field > obj_f_critter_begin && field < obj_f_critter_end)
			return true;
		return field > obj_f_npc_begin && field < obj_f_npc_end;
	case obj_t_trap:
		return field > obj_f_trap_begin && field < obj_f_trap_end;
	default:
		return false;
	}
	
}

const char * ObjectFields::GetFieldName(obj_f field) const
{
	return GetObjectFieldName(field);
}

const char * ObjectFields::GetTypeName(ObjectFieldType type)
{
	assert(type >= ObjectFieldType::None && type <= ObjectFieldType::Float32);

	static const char *PropTypes[] = {
		"None",
		"BeginSection",
		"EndSection",
		"Int32",
		"Int64",
		"AbilityArray",
		"UnkArray",
		"Int32Array",
		"Int64Array",
		"ScriptArray",
		"Unk2Array",
		"String",
		"Obj",
		"ObjArray",
		"SpellArray",
		"Float32"
	};
	return PropTypes[(int)type];
}

size_t ObjectFields::GetPropCollSize(obj_f field, ObjectFieldType type)
{
	// Handle special cases
	if (field == obj_f_critter_abilities_idx) {
		return 6;
	} else if (field == obj_f_render_alpha) {
		return 4;
	} else if (field >= obj_f_transient_begin) {
		return 0;
	}

	// All other fields are just 1 entry, except the section markers
	if (type == ObjectFieldType::None
		|| type == ObjectFieldType::BeginSection
		|| type == ObjectFieldType::EndSection) {
		return 0;
	}

	return 1;
}

bool ObjectFields::IsArrayType(ObjectFieldType type)
{
	return type == ObjectFieldType::AbilityArray
		|| type == ObjectFieldType::UnkArray
		|| type == ObjectFieldType::Int32Array
		|| type == ObjectFieldType::Int64Array
		|| type == ObjectFieldType::ScriptArray
		|| type == ObjectFieldType::Unk2Array
		|| type == ObjectFieldType::ObjArray
		|| type == ObjectFieldType::SpellArray;
}

bool ObjectFields::IterateTypeFields(ObjectType type, std::function<bool(obj_f)> callback)
{
	IterateFieldRange(obj_f_begin, obj_f_end, callback);

	switch (type)
	{
	case obj_t_portal:
		return IterateFieldRange(obj_f_portal_begin, obj_f_portal_end, callback);
	case obj_t_container:
		return IterateFieldRange(obj_f_container_begin, obj_f_container_end, callback);
	case obj_t_scenery:
		return IterateFieldRange(obj_f_scenery_begin, obj_f_scenery_end, callback);
	case obj_t_projectile:
		return IterateFieldRange(obj_f_projectile_begin, obj_f_projectile_end, callback);
	case obj_t_weapon:
		if (!IterateFieldRange(obj_f_item_begin, obj_f_item_end, callback))
			return false;
		return IterateFieldRange(obj_f_weapon_begin, obj_f_weapon_end, callback);
	case obj_t_ammo:
		if (!IterateFieldRange(obj_f_item_begin, obj_f_item_end, callback))
			return false;
		return IterateFieldRange(obj_f_ammo_begin, obj_f_ammo_end, callback);
	case obj_t_armor:
		if (!IterateFieldRange(obj_f_item_begin, obj_f_item_end, callback))
			return false;
		return IterateFieldRange(obj_f_armor_begin, obj_f_armor_end, callback);
	case obj_t_money:
		if (!IterateFieldRange(obj_f_item_begin, obj_f_item_end, callback))
			return false;
		return IterateFieldRange(obj_f_money_begin, obj_f_money_end, callback);
	case obj_t_food:
		if (!IterateFieldRange(obj_f_item_begin, obj_f_item_end, callback))
			return false;
		return IterateFieldRange(obj_f_food_begin, obj_f_food_end, callback);
	case obj_t_scroll:
		if (!IterateFieldRange(obj_f_item_begin, obj_f_item_end, callback))
			return false;
		return IterateFieldRange(obj_f_scroll_begin, obj_f_scroll_end, callback);
	case obj_t_key:
		if (!IterateFieldRange(obj_f_item_begin, obj_f_item_end, callback))
			return false;
		return IterateFieldRange(obj_f_key_begin, obj_f_key_end, callback);
	case obj_t_written:
		if (!IterateFieldRange(obj_f_item_begin, obj_f_item_end, callback))
			return false;
		return IterateFieldRange(obj_f_written_begin, obj_f_written_end, callback);
	case obj_t_bag:
		if (!IterateFieldRange(obj_f_item_begin, obj_f_item_end, callback))
			return false;
		return IterateFieldRange(obj_f_bag_begin, obj_f_bag_end, callback);
	case obj_t_generic:
		if (!IterateFieldRange(obj_f_item_begin, obj_f_item_end, callback))
			return false;
		return IterateFieldRange(obj_f_generic_begin, obj_f_generic_end, callback);
	case obj_t_pc:
		if (!IterateFieldRange(obj_f_critter_begin, obj_f_critter_end, callback))
			return false;
		return IterateFieldRange(obj_f_pc_begin, obj_f_pc_end, callback);
	case obj_t_npc:
		if (!IterateFieldRange(obj_f_critter_begin, obj_f_critter_end, callback))
			return false;
		return IterateFieldRange(obj_f_npc_begin, obj_f_npc_end, callback);
	case obj_t_trap:
		return IterateFieldRange(obj_f_trap_begin, obj_f_trap_end, callback);
	default:
		return false;
	}
}

bool ObjectFields::IterateFieldRange(obj_f from, obj_f to, std::function<bool(obj_f)> callback)
{
	// The iteration is exclusive of the start and end
	obj_f field = (obj_f)((int)from + 1);

	while (field < to) {

		if (!callback(field)) {
			return false;
		}

		field = (obj_f)((int)field + 1);
	}

	return true;

}

ObjectFields objectFields;
