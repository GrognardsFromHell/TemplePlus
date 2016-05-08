#include "stdafx.h"
#include "common.h"
#include "skill.h"
#include "bonus.h"
#include "util/fixes.h"

#pragma region SkillSystem Implementation
LegacySkillSystem skillSys;

BOOL LegacySkillSystem::SkillRoll(objHndl performer, SkillEnum skillEnum, int dc, int* resultDeltaFromDc, int flags) const
{
	return temple::GetRef<BOOL(__cdecl)(objHndl, SkillEnum, int, int*, int)>(0x1007D530)(performer, skillEnum, dc, resultDeltaFromDc, flags);
}

LegacySkillSystem::LegacySkillSystem()
{
	bonus = &bonusSys;
	macRebase(skillPropsTable, 102CBA30)
}

#pragma endregion