#pragma once
#include "stdafx.h"

enum class ItemCreationType : uint32_t {
	Alchemy = 0,
	Potion,
	Scroll,
	Wand,
	Rod,
	Wondrous,
	ArmsAndArmor_Deprecated,
	Unk7,
	ArmsAndArmor
};


uint32_t ItemWorthAdjustedForCasterLevel(objHndl objHndItem, uint32_t slotLevelNew);