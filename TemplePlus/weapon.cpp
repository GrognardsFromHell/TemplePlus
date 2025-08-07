#include "stdafx.h"
#include "common.h"
#include "config/config.h"
#include "weapon.h"
#include "obj.h"
#include "util/fixes.h"

WeaponSystem weapons;

static class WeaponReplacements : public TempleFix {
	void apply() override {
		// replace unload function to allow for more loadable weapons
		replaceFunction<int (__cdecl)(objHndl)>(0x100657D0, [](objHndl weapon) {
			weapons.SetUnloaded(weapon);
			return 0;
		});
	}
} replacements;

std::string WeaponSystem::GetName(WeaponTypes wpnType)
{
	//Use the weapon focus feat to the the name of the feat
	auto weaponFocusFeat = static_cast<feat_enums>(static_cast<int>(FEAT_WEAPON_FOCUS_GAUNTLET) + wpnType);
	std::string weaponFocusFeatName = feats.GetFeatName(weaponFocusFeat);
	int nIdx1 = weaponFocusFeatName.find("(");
	int nIdx2 = weaponFocusFeatName.find(")");
	std::string strWeaponName = weaponFocusFeatName.substr(nIdx1 + 1, nIdx2 - nIdx1 - 1);
	return strWeaponName;
}

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
	switch (wpnType){
		case wt_dagger:
		case wt_sickle:
		case wt_club:
		case wt_shortspear:
		case wt_quarterstaff:
		case wt_spear:
		case wt_dart:
		case wt_sling:
		case wt_scimitar:
		case wt_longspear:
			return 1;
		default:
			return 0;
	}
	return 0;
}


uint32_t WeaponSystem::IsMonkWeapon(WeaponTypes wpnType){
	switch (wpnType){
		case wt_dagger:
		case wt_club:
		case wt_quarterstaff:
		case wt_light_crossbow:
		case wt_sling:
		case wt_heavy_crossbow:
		case wt_javelin:
		case wt_handaxe:
		case wt_kama:
		case wt_nunchaku:
		case wt_siangham:
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
	switch (wpnType){
	case wt_hand_crossbow:
	case wt_rapier:
	case wt_short_sword:
	case wt_sap:
	case wt_shortbow:
	case wt_composite_shortbow:
		return 1;
	default:
		return 0;
	}
	return 0;
}


uint32_t WeaponSystem::IsWizardWeapon(WeaponTypes wpnType)
{
	switch (wpnType){
		case wt_dagger:
		case wt_club:
		case wt_quarterstaff:
		case wt_light_crossbow:
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
	case wt_rapier:
	case wt_shortbow:
	case wt_composite_shortbow:
	case wt_longbow:
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
	case wt_short_sword:
	case wt_longsword:
	case wt_rapier:
	case wt_shortbow:
	case wt_composite_shortbow:
	case wt_whip:
		return 1;
	default:
		return 0;
	}
	return 0;
}

int WeaponSystem::GetAmmoProtoId(WeaponAmmoType ammoType)
{
	static int table[18] = {
		3001,
		3002,
		3003,
		3004,
		3005,
		3006,
		3007,
		3008,
		3009,
		3010,
		3011,
		3012,
		3013,
		3014,
		3015,
		3016,
		3017,
		3018,
	};
	Expects(ammoType >= 0 && ammoType < WeaponAmmoType::wat_unk18);
	return table[ammoType];
}

bool WeaponSystem::IsSlashingOrBludgeoning(objHndl weapon){
	if (!weapon)
		return false;
	auto weaponType = objects.GetWeaponType(weapon);
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
		return true;
	default:
		return false;
	}
	
	/*if (IsSlashingWeapon(wpnType) || IsBludgeoningWeapon(wpnType))
		return true;*/
	return false;
}

bool WeaponSystem::IsSlashingWeapon(WeaponTypes wpnType){
	auto p = wpnProps.find(wpnType);
	if (p == wpnProps.end()) return false;
	return p->second.damType == DamageType::Slashing
		|| p->second.damType == DamageType::SlashingAndBludgeoning
		|| p->second.damType == DamageType::PiercingAndSlashing
		|| p->second.damType == DamageType::SlashingAndBludgeoningAndPiercing;
}

bool WeaponSystem::IsPiercingWeapon(WeaponTypes wpnType)
{
	auto p = wpnProps.find(wpnType);
	if (p == wpnProps.end()) return false;
	return p->second.damType == DamageType::Piercing
		|| p->second.damType == DamageType::PiercingAndSlashing
		|| p->second.damType == DamageType::BludgeoningAndPiercing
		|| p->second.damType == DamageType::SlashingAndBludgeoningAndPiercing;
}

bool WeaponSystem::IsBludgeoningWeapon(WeaponTypes wpnType)
{
	auto p = wpnProps.find(wpnType);
	if (p == wpnProps.end()) return false;
	return p->second.damType == DamageType::Bludgeoning
		|| p->second.damType == DamageType::SlashingAndBludgeoning
		|| p->second.damType == DamageType::BludgeoningAndPiercing
		|| p->second.damType == DamageType::SlashingAndBludgeoningAndPiercing;
}

int WeaponSystem::GetBaseHardness(objHndl item)
{
	if (!item)
		return -1;
	if (objects.GetType(item) == obj_t_weapon)
	{
		auto weaponType = objects.GetWeaponType(item);
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

// Note: this does not completely match the original version.
// It actually determines whether the weapon uses the ammo. The original
// reports whether the weapon can be used to make a ranged attack while
// the ammo is equipped. For the latter, thrown weapons always report
// success. However, this is the wrong behavior for determining whether
// the ammo's enhancement bonus should apply to attacks with the weapon.
bool WeaponSystem::AmmoMatchesWeapon(objHndl weapon, objHndl ammoItem)
{
	if (objects.GetType(weapon) != obj_t_weapon)
		return 0;
	auto ammoType = objects.getInt32(weapon, obj_f_weapon_ammo_type);
	if (!ammoItem)
		return 0;
	return ammoType == objects.getInt32(ammoItem, obj_f_ammo_type);
}

bool WeaponSystem::IsReachWeaponType(WeaponTypes weapType){
	switch (weapType){
	case wt_glaive:
	case wt_guisarme:
	case wt_longspear:
	case wt_ranseur:
	case wt_spike_chain:
		return true;
	default:
		return false;
	}
}

bool WeaponSystem::IsMeleeWeapon(WeaponTypes wpnType)
{
	switch (wpnType) {
	case wt_light_crossbow:
	case wt_dart:
	case wt_sling:
	case wt_heavy_crossbow:
	case wt_javelin:
	case wt_shortbow:
	case wt_composite_shortbow:
	case wt_longbow:
	case wt_composite_longbow:
	case wt_hand_crossbow:
	case wt_shuriken:
	case wt_repeating_crossbow:
	case wt_net:
	case wt_ray:
	case wt_grenade:
		return false;

	default:
		return true;
	}
}

// Checks if a weapon requires loading and is not loaded.
bool WeaponSystem::IsUnloaded(objHndl weapon)
{
	if (!weapon) return false;

	if (objects.GetType(weapon) != obj_t_weapon) return false;

	bool unloaded = !(objects.getInt32(weapon, obj_f_weapon_flags) & OWF_WEAPON_LOADED);

	switch (objects.GetWeaponType(weapon))
	{
	case wt_light_crossbow:
	case wt_heavy_crossbow:
		return unloaded;
	case wt_sling:
		return config.stricterRulesEnforcement && unloaded;
	// TODO: repeating crossbows
	default:
		return false;
	}
}

bool WeaponSystem::IsLoadable(objHndl weapon, bool strict)
{
	if (!weapon || objects.GetType(weapon) != obj_t_weapon) return false;

	switch (objects.GetWeaponType(weapon))
	{
	case wt_light_crossbow:
	case wt_heavy_crossbow:
		return true;
	case wt_sling:
		return strict || config.stricterRulesEnforcement;
		// TODO: repeating/hand crossbows
	default:
		return false;
	}
}

void WeaponSystem::SetLoaded(objHndl weapon)
{
	if (!IsLoadable(weapon)) return;

	auto flags = objects.getInt32(weapon, obj_f_weapon_flags);
	objects.setInt32(weapon, obj_f_weapon_flags, flags | OWF_WEAPON_LOADED);
}

void WeaponSystem::SetUnloaded(objHndl weapon)
{
	// set unloaded as a default state even without strict rules
	if (!IsLoadable(weapon, true)) return;

	auto flags = objects.getInt32(weapon, obj_f_weapon_flags);
	objects.setInt32(weapon, obj_f_weapon_flags, flags & ~OWF_WEAPON_LOADED);
}

bool WeaponSystem::IsRangedWeapon(WeaponTypes wpnType)
{
	switch (wpnType) {
		case wt_dagger:
		case wt_shortspear:
		case wt_spear:
		case wt_light_crossbow:
		case wt_dart:
		case wt_sling:
		case wt_heavy_crossbow:
		case wt_javelin:
		case wt_throwing_axe:
		case wt_light_hammer:
		case wt_trident:
		case wt_shortbow:
		case wt_composite_shortbow:
		case wt_longbow:
		case wt_composite_longbow:
		case wt_hand_crossbow:
		case wt_shuriken:
		case wt_repeating_crossbow:
		case wt_net:
		case wt_ray:
		case wt_grenade:
			return true;

		default:
			return false;
	}
}


bool WeaponSystem::IsThrownOnlyWeapon(WeaponTypes wpnType)
{
	switch (wpnType) {
	case wt_javelin:
	case wt_dart:
	case wt_shuriken:
	case wt_net:
	case wt_grenade:
		return true;

	default:
		return false;
	}
}

WeaponSystem::WeaponSystem(){
	wpnProps[wt_javelin].damType = DamageType::Piercing;
	wpnProps[wt_dagger].damType = DamageType::PiercingAndSlashing;
	wpnProps[wt_short_sword].damType = DamageType::Piercing;
	wpnProps[wt_longsword].damType = DamageType::Slashing;
	wpnProps[wt_dart].damType = DamageType::Piercing;
	wpnProps[wt_dwarven_waraxe].damType = DamageType::Slashing;
	wpnProps[wt_quarterstaff].damType = DamageType::Bludgeoning;
	wpnProps[wt_light_crossbow].damType = DamageType::Piercing;
	wpnProps[wt_morningstar].damType = DamageType::BludgeoningAndPiercing;
	wpnProps[wt_shuriken].damType = DamageType::Piercing;
	// todo: wakizashi, cutlass, or just generalize the fucking thing
}
