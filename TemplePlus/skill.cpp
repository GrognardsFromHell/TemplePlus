#include "stdafx.h"
#include "common.h"
#include "skill.h"
#include "bonus.h"
#include "util/fixes.h"
#include "float_line.h"

// enables autohypnosis skill slot
class AutohypnosisFix : public TempleFix {
public:

	void apply() override {
		writeHex(0x102CBBA7, "00");
	}
} autohypnosisFix;

#pragma region SkillSystem Implementation
LegacySkillSystem skillSys;

bool LegacySkillSystem::IsEnabled(SkillEnum skillEnum) const{
	return !(skillPropsTable[skillEnum].classFlags & 0x80000000);
}

BOOL LegacySkillSystem::SkillRoll(objHndl performer, SkillEnum skillEnum, int dc, int* resultDeltaFromDc, int flags) const
{
	return temple::GetRef<BOOL(__cdecl)(objHndl, SkillEnum, int, int*, int)>(0x1007D530)(performer, skillEnum, dc, resultDeltaFromDc, flags);
}

void LegacySkillSystem::FloatError(const objHndl& obj, int errorOffset){
	MesLine mesline(1000 + errorOffset);
	auto skillMes = temple::GetRef<MesHandle>(0x10AB7158);
	mesFuncs.GetLine_Safe(skillMes, &mesline);
	floatSys.floatMesLine(obj, 1, FloatLineColor::White, mesline.value);
}

const char * LegacySkillSystem::GetSkillHelpTopic(SkillEnum skillEnum){
	MesLine mesline(10200 + skillEnum);
	auto skillRulesMes = temple::GetRef<MesHandle>(0x10AB72B8);
	if (mesFuncs.GetLine(skillRulesMes, &mesline))
		return mesline.value;
	return nullptr;
}

LegacySkillSystem::LegacySkillSystem(){
	bonus = &bonusSys;
	macRebase(skillPropsTable, 102CBA30)
}

#pragma endregion