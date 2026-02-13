#pragma once

#include "common.h"
#include <map>
#include <temple/dll.h>


struct WeaponTypeProperties
{
	DamageType damType = DamageType::Unspecified;
};

struct WeaponSystem : temple::AddressTable
{
	std::string GetName(WeaponTypes wpnType);

	uint32_t IsSimple(WeaponTypes wpnType);
	uint32_t IsMartial(WeaponTypes wpnType);
	uint32_t IsExotic(WeaponTypes wpnType);
	uint32_t IsDruidWeapon(WeaponTypes wpnType);
	uint32_t IsMonkWeapon(WeaponTypes wpnType);
	uint32_t IsRogueWeapon(uint32_t wielderSize, WeaponTypes wpnType);
	uint32_t IsWizardWeapon(WeaponTypes wpnType);
	uint32_t IsElvenWeapon(WeaponTypes wpnType);
	uint32_t IsBardWeapon(WeaponTypes wpnType);

	int GetAmmoProtoId(WeaponAmmoType ammoType);

	bool IsSlashingOrBludgeoning(objHndl weapon);
	bool IsSlashingOrBludgeoning(WeaponTypes wpnType);
	bool IsSlashingWeapon(WeaponTypes wpnType);
	bool IsPiercingWeapon(WeaponTypes wpnType);
	bool IsBludgeoningWeapon(WeaponTypes wpnType);

	int GetBaseHardness(objHndl item);
	int GetBaseHardness(WeaponTypes weapon);
	bool AmmoMatchesWeapon(objHndl weapon, objHndl ammoItem);
	bool IsReachWeaponType(WeaponTypes weap_type);
	bool IsRangedWeapon(WeaponTypes wpnType);
	bool IsThrownOnlyWeapon(WeaponTypes wpnType);
	bool IsMeleeWeapon(WeaponTypes wpnType);
	bool IsUnloaded(objHndl weapon);
	bool IsLoadable(objHndl weapon, bool strict = false);

	void SetLoaded(objHndl weapon);
	void SetUnloaded(objHndl weapon);

	std::map<WeaponTypes, WeaponTypeProperties> wpnProps;
	WeaponSystem();
};

extern WeaponSystem weapons;
