#pragma once

#include "common.h"

#include <temple/dll.h>

struct WeaponSystem : temple::AddressTable
{
	uint32_t IsSimple(WeaponTypes wpnType);
	uint32_t IsMartial(WeaponTypes wpnType);
	uint32_t IsExotic(WeaponTypes wpnType);
	uint32_t IsDruidWeapon(WeaponTypes wpnType);
	uint32_t IsMonkWeapon(WeaponTypes wpnType);
	uint32_t IsRogueWeapon(uint32_t wielderSize, WeaponTypes wpnType);
	uint32_t IsWizardWeapon(WeaponTypes wpnType);
	uint32_t IsElvenWeapon(WeaponTypes wpnType);
	uint32_t IsBardWeapon(WeaponTypes wpnType);
	bool IsSlashingOrBludgeoning(objHndl weapon);
	bool IsSlashingOrBludgeoning(WeaponTypes wpnType);
	int GetBaseHardness(objHndl item);
	int GetBaseHardness(WeaponTypes weapon);
	bool AmmoMatchesWeapon(objHndl weapon, objHndl ammoItem);
};

extern WeaponSystem weapons;