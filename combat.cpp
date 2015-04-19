#include "stdafx.h"
#include "common.h"
#include "combat.h"
#include "tig/tig_mes.h"


class CombatSystemReplacements : public TempleFix
{
public:
	const char* name() override {
		return "Combat System Replacements";
	}
	void apply() override{
		replaceFunction(0x100628D0, _isCombatActive);
	}
} combatSysReplacements;



#pragma region Combat System Implementation
CombatSystem combatSys;

bool CombatSystem::isCombatActive()
{
	return *combatSys.combatModeActive != 0;
}

void CombatSystem::enterCombat(objHndl objHnd)
{
	_enterCombat(objHnd);
}
#pragma endregion


uint32_t _isCombatActive()
{
	return *combatSys.combatModeActive;
}

uint32_t Combat_GetMesfileIdx_CombatMes()
{
	return *combatSys.combatMesfileIdx;
}