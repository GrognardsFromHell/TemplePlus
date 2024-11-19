#include "stdafx.h"
#include "d20_level.h"
#include "common.h"
#include "obj.h"
#include "util/fixes.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/deity/legacydeitysystem.h"
#include "d20_race.h"


D20LevelSystem d20LevelSys;

struct D20LevelSystemAddresses : temple::AddressTable
{
	uint32_t * xpReqTable;
	uint32_t (__cdecl *LevelPacketInit)(LevelPacket *lvlPkt);
	uint32_t(__cdecl *LevelPacketDealloc)(LevelPacket* lvlPkt);
	uint32_t(__cdecl *GetLevelPacket)(Stat classEnum, objHndl objHnd, uint32_t levelAdjustSthg, uint32_t classLevel, LevelPacket* lvlPkt);

	D20LevelSystemAddresses()
	{
		rebase(xpReqTable, 0x102AAF00); // xp needed to reach level N, starting from N=0 (0 , 0 , 1000, 3000, ...)

		rebase(LevelPacketDealloc, 0x100F4780);
		rebase(LevelPacketInit, 0x100F5520);
		rebase(GetLevelPacket, 0x100F5140);
	};
} addresses;



class D20LevelHooks : public TempleFix
{
public: 
	
	void apply() override 
	{
		// hooked the address of the wizardSpellsPerLvl
		auto writeVal = reinterpret_cast<int>(d20LevelSys.mWizardSpellsPerLevel);

		write(0x100F4E4C + 3, &writeVal, sizeof(int));

		// AddSkillPoints
		replaceFunction<void(objHndl, int, int)>(0x1007DAA0, [](objHndl obj, int skillEnum, int numAdd){
			d20LevelSys.AddSkillPoints(obj, skillEnum, numAdd);
		});

		replaceFunction<int(__cdecl)(int)>(0x100802E0, [](int lvl)->int {
				if (lvl > config.maxLevel+1) {
					lvl = config.maxLevel+1;
				}
				return d20LevelSys.GetXpRequireForLevel(lvl);
			});
	}
} d20LevelHooks;

uint32_t D20LevelSystem::LevelPacketInit(LevelPacket* lvlPkt)
{
	return addresses.LevelPacketInit(lvlPkt);
}

uint32_t D20LevelSystem::LevelPacketDealloc(LevelPacket* lvlPkt)
{
	return addresses.LevelPacketDealloc(lvlPkt);
}

uint32_t D20LevelSystem::GetLevelPacket(Stat classEnum, objHndl objHnd, uint32_t levelAdjustSthg, uint32_t classLevel, LevelPacket* lvlPkt)
{
	return addresses.GetLevelPacket(classEnum, objHnd, levelAdjustSthg, classLevel, lvlPkt);
}

bool D20LevelSystem::CanLevelup(objHndl objHnd)
{
	auto ecl = critterSys.GetEffectiveLevel(objHnd);
	
	if (d20Sys.d20Query(objHnd, DK_QUE_ExperienceExempt) || ecl >= (int)config.maxLevel){
		return 0;
	}
	return objects.getInt32(objHnd, obj_f_critter_experience) >= xpReqTable[ecl + 1];
		//addresses.xpReqTable[lvl + 1];

}

void D20LevelSystem::AddSkillPoints(objHndl handle, int skillEnum, int numAdded){

	
	auto levClass = stat_level_fighter; // default
	auto obj = objSystem->GetObject(handle);
	auto numClasses = obj->GetInt32Array(obj_f_critter_level_idx).GetSize();

	if (numClasses) {
		levClass = (Stat)obj->GetInt32(obj_f_critter_level_idx, numClasses - 1);
	}

	auto skill = (SkillEnum)skillEnum;

	if (d20ClassSys.IsClassSkill(skill, levClass) ||
		(levClass == stat_level_cleric && deitySys.IsDomainSkill(handle, skill))
		|| d20Sys.D20QueryPython(handle, "Is Class Skill", skill)) {
		numAdded *= 2;
	}

	auto skillPtNew = numAdded + obj->GetInt32(obj_f_critter_skill_idx, skill);
	if (obj->IsPC()) {
		auto expectedMax = 2 * objects.StatLevelGet(handle, stat_level) + 6;
		if (skillPtNew > expectedMax)
			logger->warn("PC {} has more skill points than they should (has: {} , expected: {}", handle, skillPtNew, expectedMax);
	}
	obj->SetInt32(obj_f_critter_skill_idx, skill, skillPtNew);
}

/* 0x100802E0 */
uint32_t D20LevelSystem::GetXpRequireForLevel(uint32_t level)
{
	if (level < 0 || level >= XP_REQ_TABLE_MAX_LEVEL)
		return 0;
	return xpReqTable[level];

}


uint32_t D20LevelSystem::GetPenaltyXPForDrainedLevel(uint32_t level)
{
	auto xp0 = GetXpRequireForLevel(level);
	auto xp1 = GetXpRequireForLevel(level+1);

	return (xp0 >=0 && xp1 > 0 && xp1 > xp0 ) ? (xp0 + xp1) / 2 : 0;
}

int D20LevelSystem::GetSurplusXp(objHndl handle){
	auto lvl = objects.StatLevelGet(handle, stat_level);
	if (lvl > 1){
		lvl = critterSys.GetEffectiveLevel(handle);
	}
	int xpReq = (int)GetXpRequireForLevel(lvl);
	return gameSystems->GetObj().GetObject(handle)->GetInt32(obj_f_critter_experience) - xpReq;
}

int D20LevelSystem::GetSpellsPerLevel(const objHndl handle, Stat classCode, int spellLvl, int casterLvl){
	// todo: extend to non-core
	LevelPacket lvlPkt;
	auto spellLvlCapped = spellLvl;
	if (classCode == stat_level_bard){
		if (spellLvlCapped > 7)
			spellLvlCapped = 7;
	}
	else if (classCode == stat_level_paladin){
		if (spellLvlCapped > 5)
			spellLvlCapped = 5;
	}
	else if (spellLvlCapped > 10){
		spellLvlCapped = 10;
	}

	lvlPkt.GetLevelPacket(classCode, handle, 0, casterLvl);
	auto spPerLvl = lvlPkt.sorcBardSpellCount[spellLvlCapped];

	switch (spPerLvl + 40){
	case 0:
	case 10: 
	case 20:
	case 30:
		spPerLvl = 0;
		break;
	default:
		break;
	}
	return spPerLvl;
}

void D20LevelSystem::GenerateSpellsPerLevelTables()
{

	memset(mWizardSpellsPerLevel, 0,  sizeof(mWizardSpellsPerLevel));

	memcpy(mWizardSpellsPerLevel, temple::GetPointer<int>(0x102EA460), 20 * NUM_SPELL_LEVELS*sizeof(int));

}

LevelPacket::LevelPacket(){
	d20LevelSys.LevelPacketInit(this);
}

LevelPacket::~LevelPacket()
{
	d20LevelSys.LevelPacketDealloc(this);
}

int LevelPacket::GetLevelPacket(Stat classCode, objHndl obj, int lvlAdj, int classLvl)
{
	return d20LevelSys.GetLevelPacket(classCode, obj, lvlAdj, classLvl, this);
}

uint32_t _CanLevelup(objHndl objHnd)
{
	return d20LevelSys.CanLevelup(objHnd);
}
