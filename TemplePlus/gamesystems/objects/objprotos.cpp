
#include "stdafx.h"

#include "obj.h"
#include "objsystem.h"
#include "temple_enums.h"

#include "gamesystems/gamesystems.h"

void SetProtoDefaultProperties(objHndl handle) {
	/*
	auto& objs = gameSystems->GetObj();

	objs.SetInt32(handle, obj_f_2D_shadow_art, -1);
	objs.SetInt32(handle, obj_f_blit_color, 0xFFFFFF);
	objs.SetInt32(handle, obj_f_transparency, 255);
	objs.SetInt32(handle, obj_f_model_scale, 100);
	objs.SetInt32(handle, obj_f_light_color, 0xFFFFFF);
	
	auto flags = objects.GetFlags(handle);
	auto type = objects.GetType(handle);

	switch (type)
	{
	case obj_t_weapon:
	case obj_t_ammo:
	case obj_t_armor:
	case obj_t_money:
	case obj_t_food:
	case obj_t_scroll:
	case obj_t_key:
	case obj_t_written:
	case obj_t_generic:
		if (type == obj_t_key) {
			objs.SetInt32(handle, obj_f_item_weight, 0);
		} else if (type == obj_t_money) {
			objs.SetInt32(handle, obj_f_item_weight, 1);
		} else {
			objs.SetInt32(handle, obj_f_item_weight, 10);
		}
		flags = (ObjectFlag)(flags | OF_NO_BLOCK | OF_SHOOT_THROUGH | OF_SEE_THROUGH | OF_FLAT);
		objects.SetFlags(handle, flags);
		break;
	case obj_t_pc:
	case obj_t_npc:
		if (type == obj_t_pc) {
			Object_Set_Field_With_Subindex__by_Handle(ObjHnd, obj_f_pc_weaponslots_idx, 0, 0);
			v1 = 1;
			do
				Object_Set_Field_With_Subindex__by_Handle(ObjHnd, obj_f_pc_weaponslots_idx, v1++, -1);
			while (v1 < 21);
		}
		proto_set_pc_defaults(handle);
		proto_set_pc_defaults2(handle);
		flags = (ObjectFlag)(flags | OF_PROVIDES_COVER | OF_SHOOT_THROUGH | OF_SEE_THROUGH);
		objects.SetFlags(handle, flags);
		break;
	default:
		break;
	}

	switch (type)
	{
	case obj_t_portal:
		objs.SetInt32(handle, obj_f_hp_pts, 100);
		flags = (ObjectFlag)(flags | OF_PROVIDES_COVER);
		objects.SetFlags(handle, flags);
		break;
	case obj_t_container:
	case obj_t_scenery:
		objs.SetInt32(handle, obj_f_hp_pts, 100);
		flags = (ObjectFlag)(flags | OF_PROVIDES_COVER | OF_SHOOT_THROUGH | OF_SEE_THROUGH);
		objects.SetFlags(handle, flags);
		break;
	case obj_t_projectile:
		flags = (ObjectFlag)(flags | OF_NO_BLOCK | OF_SHOOT_THROUGH | OF_SEE_THROUGH);
		objects.SetFlags(handle, flags);
		break;
	case obj_t_weapon:
		proto_set_item_defaults(0, handle);
		objs.SetInt32(handle, obj_f_weapon_ammo_type, 10000);
		v7 = dice_pack(1, 4, 0);
		objs.SetInt32(handle, obj_f_weapon_damage_dice, v7);
		objs.SetInt32(handle, obj_f_weapon_crit_hit_chart, 2);
		objs.SetInt32(handle, obj_f_weapon_crit_range, 1);
		objs.SetInt32(handle, obj_f_weapon_type, 0);
		objs.SetInt32(handle, obj_f_weapon_missile_aid, -1);
		break;
	case obj_t_ammo:
		proto_set_item_defaults(1, handle);
		break;
	case obj_t_armor:
		proto_set_item_defaults(2, handle);
		break;
	case obj_t_money:
		proto_set_item_defaults(3, handle);
		objs.SetInt32(handle, obj_f_money_quantity, 1);
		break;
	case obj_t_food:
		proto_set_item_defaults(4, handle);
		break;
	case obj_t_scroll:
		proto_set_item_defaults(5, handle);
		break;
	case obj_t_key:
		proto_set_item_defaults(6, handle);
		break;
	case obj_t_written:
		proto_set_item_defaults(8, handle);
		break;
	case obj_t_generic:
		proto_set_item_defaults(9, handle);
		break;
	case obj_t_trap:
		objs.SetInt32(handle, obj_f_hp_pts, 100);
		flags = (ObjectFlag)(flags | OF_DONTLIGHT | OF_NO_BLOCK | OF_SHOOT_THROUGH | OF_SEE_THROUGH | OF_FLAT);
		objects.SetFlags(handle, flags);
		break;
	default:
		return;
	}*/
}
