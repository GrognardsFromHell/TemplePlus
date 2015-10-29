#include "stdafx.h"
#include "common.h"
#include "skill.h"
#include "bonus.h"
#include "util/fixes.h"

class SkillSystemReplacements : public TempleFix
{
	macTempleFix(Skill System)
	{
		
	}
} skillSysReplacements;

#pragma region SkillSystem Implementation
LegacySkillSystem skillSys;

LegacySkillSystem::LegacySkillSystem()
{
	bonus = &bonusSys;
	macRebase(skillPropsTable, 102CBA30)
}

#pragma endregion