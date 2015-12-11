#include "stdafx.h"
#include "common.h"
#include "weapon.h"
#include "obj.h"

WeaponSystem weapons;

uint32_t WeaponSystem::IsSimple(WeaponTypes wpnType)
{
	if (wpnType == wt_longspear || (wpnType <= wt_javelin && wpnType >= wt_gauntlet))
	{
		return 1;
	}
	return 0;
}

uint32_t WeaponSystem::IsMartial(WeaponTypes wpnType)
{
	if (wpnType <= wt_composite_longbow && wpnType != wt_longspear && wpnType >= wt_throwing_axe)
	{
		return 1;
	}
	if (wpnType == wt_kukri){ return 1; }
	return 0;
}


uint32_t WeaponSystem::IsExotic(WeaponTypes wpnType)
{
	if (wpnType >= wt_halfling_kama && wpnType <= wt_grenade && wpnType != wt_kukri)
	{
		return 1;
	}
	return 0;
}

uint32_t WeaponSystem::IsDruidWeapon(WeaponTypes wpnType)
{
	switch (wpnType)
	{
	case wt_dagger:
		return 1;
	case wt_sickle:
		return 1;
	case wt_club:
		return 1;
	case wt_shortspear:
		return 1;
	case wt_quarterstaff:
		return 1;
	case wt_spear:
		return 1;
	case wt_dart:
		return 1;
	case wt_sling:
		return 1;
	case wt_scimitar:
		return 1;
	case wt_longspear:
		return 1;
	default:
		return 0;
	}
	return 0;
}


uint32_t WeaponSystem::IsMonkWeapon(WeaponTypes wpnType)
{
	switch (wpnType)
	{
	case wt_dagger:
		return 1;
	case wt_club:
		return 1;
	case wt_quarterstaff:
		return 1;
	case wt_light_crossbow:
		return 1;
	case wt_sling:
		return 1;
	case wt_heavy_crossbow:
		return 1;
	case wt_javelin:
		return 1;
	case wt_handaxe:
		return 1;
	case wt_kama:
		return 1;
	case wt_nunchaku:
		return 1;
	case wt_siangham:
		return 1;
	case wt_shuriken:
		return 1;
	default:
		return 0;
	}
	return 0;
}


uint32_t WeaponSystem::IsRogueWeapon(uint32_t wielderSize, WeaponTypes wpnType)
{
	// TODO: looks like Troika intended to differentiate by the Wielder's Size? Was not implemented
	if (weapons.IsSimple(wpnType)){ return 1; }
	switch (wpnType)
	{
	case wt_hand_crossbow:
		return 1;
	case wt_rapier:
		return 1;
	case wt_short_sword:
		return 1;
	case wt_sap:
		return 1;
	case wt_shortbow:
		return 1;
	case wt_composite_shortbow:
		return 1;
	default:
		return 0;
	}
	return 0;
}


uint32_t WeaponSystem::IsWizardWeapon(WeaponTypes wpnType)
{
	switch (wpnType)
	{
	case wt_dagger:
		return 1;
	case wt_club:
		return 1;
	case wt_quarterstaff:
		return 1;
	case wt_light_crossbow:
		return 1;
	case wt_heavy_crossbow:
		return 1;
	default:
		return 0;
	}
	return 0;
}

uint32_t WeaponSystem::IsElvenWeapon(WeaponTypes wpnType)
{
	switch (wpnType)
	{
	case wt_longsword:
		return 1;
	case wt_rapier:
		return 1;
	case wt_shortbow:
		return 1;
	case wt_composite_shortbow:
		return 1;
	case wt_longbow:
		return 1;
	case wt_composite_longbow:
		return 1;
	default:
		return 0;
	}
	return 0;
}

uint32_t WeaponSystem::IsBardWeapon(WeaponTypes wpnType)
{
	if (weapons.IsSimple(wpnType))
	{
		return 1;
	}
	switch (wpnType)
	{
	case wt_sap:
		return 1;
	case wt_short_sword:
		return 1;
	case wt_longsword:
		return 1;
	case wt_rapier:
		return 1;
	case wt_shortbow:
		return 1;
	case wt_composite_shortbow:
		return 1;
	case wt_whip:
		return 1;
	default:
		return 0;
	}
	return 0;
}

bool WeaponSystem::IsSlashingOrBludgeoning(objHndl weapon)
{
	if (!weapon)
		return 0;
	WeaponTypes weaponType = (WeaponTypes)objects.getInt32(weapon, obj_f_weapon_type);
	return IsSlashingOrBludgeoning(weaponType);
}

bool WeaponSystem::IsSlashingOrBludgeoning(WeaponTypes wpnType)
{
	switch (wpnType)
	{
	case wt_gauntlet:
	case wt_light_mace:
	case wt_sickle:
	case wt_club:
	case wt_heavy_mace:
	case wt_morningstar:
	case wt_quarterstaff:
	case wt_sling:
	case wt_throwing_axe:
	case wt_light_hammer:
	case wt_handaxe:
	case wt_kukri:
	case wt_sap:
	case wt_battleaxe:
	case wt_light_flail:
	case wt_heavy_flail:
	case wt_longsword:
	case wt_scimitar:
	case wt_warhammer:
	case wt_falchion:
	case wt_glaive:
	case wt_greataxe:
	case wt_greatclub:
	case wt_greatsword:
	case wt_guisarme:
	case wt_halberd:
	case wt_scythe:
	case wt_kama:
	case wt_halfling_kama:
	case wt_nunchaku:
	case wt_bastard_sword:
	case wt_dwarven_waraxe:
	case wt_whip:
	case wt_orc_double_axe:
	case wt_dire_flail:
	case wt_gnome_hooked_hammer:
	case wt_two_bladed_sword:
	case wt_dwarven_urgrosh:
		return 1;
	default:
		return 0;
	}
}

int WeaponSystem::GetBaseHardness(objHndl item)
{
	if (!item)
		return -1;
	if (objects.GetType(item) == obj_t_weapon)
	{
		WeaponTypes weaponType = (WeaponTypes)objects.getInt32(item, obj_f_weapon_type);
		return GetBaseHardness(weaponType);
	}
	if (objects.GetType(item) == obj_t_armor)
		return 100;

	return 10;
}

int WeaponSystem::GetBaseHardness(WeaponTypes weapon)
{
	switch (weapon){
	
	case wt_light_mace:
	case wt_club:
	case wt_shortspear:
	case wt_heavy_mace:
	case wt_morningstar:
	case wt_quarterstaff:
	case wt_spear:
	case wt_light_crossbow:		// 14
	case wt_dart:
	case wt_sling:
	case wt_heavy_crossbow:		// 17
	case wt_javelin:
	case wt_throwing_axe:
	case wt_light_hammer:
	case wt_handaxe:
	case wt_light_lance:
	case wt_light_pick:
			
	case wt_battleaxe:
	case wt_light_flail:
			
	case wt_heavy_pick:
	case wt_rapier:
			
	case wt_trident:
	case wt_warhammer:
			
	case wt_heavy_flail:
	case wt_glaive:
	case wt_greataxe:
	case wt_greatclub:
			
	case wt_guisarme:
	case wt_halberd:
	case wt_longspear:  //43
	case wt_ranseur:
	case wt_scythe:
	case wt_shortbow:
	case wt_composite_shortbow:
	case wt_longbow:
	case wt_composite_longbow:
	case wt_halfling_kama: // 50
			
	case wt_kama: //54

	case wt_dwarven_waraxe:          // racial weapons - 58 and on
	case wt_gnome_hooked_hammer:
	case wt_orc_double_axe:
			
	case wt_dwarven_urgrosh:
	case wt_hand_crossbow:			// 65
			
	case wt_whip:
	case wt_repeating_crossbow:
	case wt_net:
			
		return 5;
	default: 
		return 10;
	}
}

bool WeaponSystem::AmmoMatchesWeapon(objHndl weapon, objHndl ammoItem)
{
	if (objects.GetType(weapon) != obj_t_weapon)
		return 0;
	auto ammoType = objects.getInt32(weapon, obj_f_weapon_ammo_type);
	if (ammoType >= 4 && ammoType < 18) // no ammo required??
		return 1;
	if (!ammoItem)
		return 0;
	return ammoType == objects.getInt32(ammoItem, obj_f_ammo_type);
}