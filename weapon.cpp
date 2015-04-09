#include "stdafx.h"
#include "common.h"
#include "weapon.h"

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