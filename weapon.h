#pragma once
#include "common.h"


struct WeaponSystem : AddressTable
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
};

extern WeaponSystem weapons;