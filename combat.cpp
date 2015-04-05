#include "stdafx.h"
#include "combat.h"
#include "tig_mes.h"

GlobalPrimitive<uint32_t, 0x10AA8418> combatModeActive;
GlobalPrimitive<MesHandle, 0x10AA8408> combatsysCombatMesfileIdx;

uint32_t IsCombatActive()
{
	return combatModeActive;
}

uint32_t Combat_GetMesfileIdx_CombatMes()
{
	return combatsysCombatMesfileIdx;
}