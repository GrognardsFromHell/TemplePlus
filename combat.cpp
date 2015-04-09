#include "stdafx.h"
#include "combat.h"
#include "tig/tig_mes.h"

CombatSystem combat;

uint32_t IsCombatActive()
{
	return *combat.combatModeActive;
}

uint32_t Combat_GetMesfileIdx_CombatMes()
{
	return *combat.combatMesfileIdx;
}