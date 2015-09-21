#include "stdafx.h"
#include "d20_class.h"

D20ClassSystem d20ClassSys;

struct D20ClassSystemAddresses : AddressTable
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