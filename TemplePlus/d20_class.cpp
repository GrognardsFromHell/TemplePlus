#include "stdafx.h"
#include "d20_class.h"
#include "d20_level.h"
#include "obj.h"

D20ClassSystem d20ClassSys;

struct D20ClassSystemAddresses : temple::AddressTable
{
	void(__cdecl *ClassPacketAlloc)(ClassPacket *classPkt); // allocates the three IdxTables within
	void(__cdecl *ClassPacketDealloc)(ClassPacket *classPkt);
	uint32_t(__cdecl * GetClassPacket)(Stat classEnum, ClassPacket* classPkt); // fills the struct with content based on classEnum (e.g. Barbarian Feats in the featsIdxTable). Also STUB FOR PRESTIGE CLASSES! TODO
	D20ClassSystemAddresses()
	{
		rebase(ClassPacketAlloc,   0x100F5730); 
		rebase(ClassPacketDealloc, 0x100F5780);
		rebase(GetClassPacket,     0x100F65E0);  
	}

} addresses;


bool D20ClassSystem::isNaturalCastingClass(Stat classEnum)
{
	if (classEnum == stat_level_bard || classEnum == stat_level_sorcerer) return 1;
	return 0;
}

bool D20ClassSystem::isNaturalCastingClass(uint32_t classEnum)
{
	Stat castedClassEnum = static_cast<Stat>(classEnum);
	if (classEnum == stat_level_bard || classEnum == stat_level_sorcerer) return 1;
	return 0;
}

bool D20ClassSystem::isVancianCastingClass(Stat classEnum)
{
	if (classEnum == stat_level_cleric || classEnum == stat_level_druid || classEnum == stat_level_paladin || classEnum == stat_level_ranger || classEnum == stat_level_wizard) return 1;
	return 0;
}

bool D20ClassSystem::IsCastingClass(Stat classEnum)
{
	if (classEnum == stat_level_cleric || classEnum == stat_level_druid || classEnum == stat_level_paladin || classEnum == stat_level_ranger || classEnum == stat_level_wizard || classEnum == stat_level_bard || classEnum == stat_level_sorcerer) return 1;
	return 0;
}

bool D20ClassSystem::IsLateCastingClass(Stat classEnum)
{
	if (classEnum == stat_level_paladin || classEnum == stat_level_ranger)
		return 1;
	return 0;
}

void D20ClassSystem::ClassPacketAlloc(ClassPacket* classPkt)
{
	addresses.ClassPacketAlloc(classPkt);
}

void D20ClassSystem::ClassPacketDealloc(ClassPacket* classPkt)
{
	addresses.ClassPacketDealloc(classPkt);
}

uint32_t D20ClassSystem::GetClassPacket(Stat classEnum, ClassPacket* classPkt)
{
	return addresses.GetClassPacket(classEnum, classPkt);
}

int D20ClassSystem::ClericMaxSpellLvl(uint32_t clericLvl) const
{
	return max(static_cast<uint32_t>(0), clericLvl % 2 + clericLvl / 2);
}

int D20ClassSystem::NumDomainSpellsKnownFromClass(objHndl dude, Stat classCode)
{
	if (classCode != stat_level_cleric)
		return 0;
	auto clericLvl = objects.StatLevelGet(dude, stat_level_cleric);
	return ClericMaxSpellLvl(clericLvl) * 2;
}

int D20ClassSystem::GetNumSpellsFromClass(objHndl obj, Stat classCode, int spellLvl, uint32_t classLvl)
{
	LevelPacket lvlPkt;
	d20LevelSys.GetLevelPacket(classCode, obj, 0, classLvl, &lvlPkt);
	if (classCode == stat_level_bard){
		if (spellLvl > 7)
			spellLvl = 7;
	}
	else if (classCode <= stat_level_monk || classCode > stat_level_ranger){
		if (spellLvl > 10)
			spellLvl = 10;
	}
	else{
		if (spellLvl > 5)
			spellLvl = 5;
	}
	auto spellCountFromClass = lvlPkt.spellCountFromClass[spellLvl];
	if (spellCountFromClass >=0)
	{
		spellCountFromClass += lvlPkt.spellCountBonusFromStatMod[spellLvl];
	}

	return spellCountFromClass;
}
