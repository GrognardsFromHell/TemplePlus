#include "stdafx.h"
#include "common.h"
#include "inventory.h"
#include "obj.h"
#include "critter.h"

InventorySystem inventory;

objHndl InventorySystem::FindItemByName(objHndl container, int nameId) {
	return _FindItemByName(container, nameId);
}

objHndl InventorySystem::FindItemByProtoHandle(objHndl container, objHndl protoHandle, bool skipWorn) {
	return _FindItemByProto(container, protoHandle, skipWorn);
}

objHndl InventorySystem::FindItemByProtoId(objHndl container, int protoId, bool skipWorn) {
	auto protoHandle = objects.GetProtoHandle(protoId);
	return _FindItemByProto(container, protoHandle, skipWorn);
}

int InventorySystem::SetItemParent(objHndl item, objHndl parent, int flags)  {
	return _SetItemParent(item, parent, flags);
}

int InventorySystem::IsNormalCrossbow(objHndl weapon)
{
	if (objects.GetType(weapon) == obj_t_weapon)
	{
		int weapType = objects.getInt32(weapon, obj_f_weapon_type);
		if (weapType == wt_heavy_crossbow || weapType == wt_light_crossbow)
			return 1; // TODO: should this include repeating crossbow? I think the context is reloading action in some cases
		// || weapType == wt_hand_crossbow
	}
	return 0;
}

int InventorySystem::IsThrowingWeapon(objHndl weapon)
{
	if (objects.GetType(weapon) == obj_t_weapon)
	{
		WeaponAmmoType ammoType = (WeaponAmmoType)objects.getInt32(weapon, obj_f_weapon_ammo_type);
		if (ammoType > wat_dagger && ammoType <= wat_bottle) // thrown weapons   TODO: should this include daggers??
		{
			return 1;
		}
	}
	return 0;
}

obj_f InventorySystem::GetInventoryListField(objHndl objHnd)
{
	if (objects.IsCritter(objHnd)) 	return obj_f_critter_inventory_list_idx;
	if (objects.IsContainer(objHnd)) return obj_f_container_inventory_list_idx;
	return (obj_f)0;
}