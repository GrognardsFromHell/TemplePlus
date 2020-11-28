#include "stdafx.h"
#include "common.h"
#include "skill.h"
#include "bonus.h"
#include "util/fixes.h"
#include "float_line.h"
#include "d20.h"

#pragma region SkillSystem Implementation
LegacySkillSystem skillSys;

class SkillFunctionReplacement : public TempleFix {
public:
	static BOOL SkillRoll(objHndl performer, SkillEnum skillEnum, int dc, int* resultDeltaFromDc, int flags);

	void apply() override; 

private:
	//Old version of the function to be used within the replacement
    int (*oldSkillRoll)(objHndl, SkillEnum, int, int*, int) = nullptr;
} skillFunctionReplacement;

void SkillFunctionReplacement::apply()
{
	logger->info("Replacing Skill-related Functions");

	oldSkillRoll = replaceFunction<BOOL(objHndl, SkillEnum, int, int*, int)>(0x1007D530, SkillRoll);
}

BOOL SkillFunctionReplacement::SkillRoll(objHndl performer, SkillEnum skillEnum, int dc, int* resultDeltaFromDc, int flags)
{
	//Check if the skill requested should be swapped with a different skill roll (some abilities allow this)
	auto swapSkill = d20Sys.D20QueryPython(performer, "Skill Swap", skillEnum);
	
	// A non zero return means that the value - 1 is the skill to swap out with
	if (swapSkill > 0) {
		skillEnum = static_cast<SkillEnum>(swapSkill - 1);
	}

	return skillFunctionReplacement.oldSkillRoll(performer, skillEnum, dc, resultDeltaFromDc, flags);
}

bool LegacySkillSystem::IsEnabled(SkillEnum skillEnum) const{
	return !(skillPropsTable[skillEnum].classFlags & 0x80000000);
}

BOOL LegacySkillSystem::SkillRoll(objHndl performer, SkillEnum skillEnum, int dc, int* resultDeltaFromDc, int flags) const
{
	return skillFunctionReplacement.SkillRoll(performer, skillEnum, dc, resultDeltaFromDc, flags);
}

void LegacySkillSystem::FloatError(const objHndl& obj, int errorOffset){
	MesLine mesline(1000 + errorOffset);
	auto skillMes = temple::GetRef<MesHandle>(0x10AB7158);
	mesFuncs.GetLine_Safe(skillMes, &mesline);
	floatSys.floatMesLine(obj, 1, FloatLineColor::White, mesline.value);
}

const char* LegacySkillSystem::GetSkillName(SkillEnum skillEnum)
{
	return temple::GetRef<const char* []>(0x10AB70B0)[skillEnum];
}

const char * LegacySkillSystem::GetSkillHelpTopic(SkillEnum skillEnum){
	MesLine mesline(10200 + skillEnum);
	auto skillRulesMes = temple::GetRef<MesHandle>(0x10AB72B8);
	if (mesFuncs.GetLine(skillRulesMes, &mesline))
		return mesline.value;
	return nullptr;
}

Stat LegacySkillSystem::GetSkillStat(SkillEnum skillEnum)
{
	return skillPropsTable[skillEnum].stat;
}

LegacySkillSystem::LegacySkillSystem(){
	bonus = &bonusSys;
	macRebase(skillPropsTable, 102CBA30)
}

#pragma endregion