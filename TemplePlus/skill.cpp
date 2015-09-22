#include "stdafx.h"
#include "common.h"
#include "skill.h"
#include "bonus.h"

class SkillSystemReplacements : public TempleFix
{
	macTempleFix(Skill System)
	{
		
	}
} skillSysReplacements;

#pragma region SkillSystem Implementation
SkillSystem skillSys;

SkillSystem::SkillSystem()
{
	bonus = &bonusSys;
	macRebase(skillPropsTable, 102CBA30)
}

#pragma endregion