#pragma once
#include "common.h"
#include "idxtables.h"


struct ClassPacket;

struct D20ClassSystem : temple::AddressTable
{
public:
	Stat classEnums[NUM_CLASSES];
	bool isNaturalCastingClass(Stat classEnum);
	bool isNaturalCastingClass(uint32_t classEnum);
	bool isVancianCastingClass(Stat classEnum);
	bool IsCastingClass(Stat classEnum);
	bool IsLateCastingClass(Stat classEnum); // for classes like Ranger / Paladin that start casting on level 4
	void ClassPacketAlloc(ClassPacket *classPkt); // allocates the three IdxTables within ClassPacket
	void ClassPacketDealloc(ClassPacket *classPkt);
	uint32_t GetClassPacket(Stat classEnum, ClassPacket *classPkt); // fills the struct with content based on classEnum (e.g. Barbarian Feats in the featsIdxTable). Also STUB FOR PRESTIGE CLASSES! TODO
	

	D20ClassSystem()
	{
		Stat _charClassEnums[NUM_CLASSES] = 
			{ stat_level_barbarian, stat_level_bard, 
			stat_level_cleric, stat_level_druid, stat_level_fighter, stat_level_monk, stat_level_paladin, stat_level_ranger, stat_level_rogue, stat_level_sorcerer, stat_level_wizard };
		memcpy(classEnums, _charClassEnums, NUM_CLASSES * sizeof(uint32_t));
	}

	
};

extern D20ClassSystem d20ClassSys;

#pragma pack(push, 1)
struct ClassPacket
{
	IdxTable<uint32_t> idxTab1; // unknown
	IdxTable<uint32_t> idxTab2; // unknown
	IdxTable<feat_enums> featsIdxTable;
	uint32_t fortitudeSaveIsFavored;
	uint32_t reflexSaveIsFavored;
	uint32_t willSaveIsFavored;
	uint32_t hitDice; // packed in that triplet format XdY+Z
	Stat classEnum;
	uint32_t skillPointsPerLevel_Maybe;
	uint32_t skillPointsMultiplier_Maybe;
};

const auto TestSizeOfClassPacket = sizeof(ClassPacket); // should be 76 ( 0x4C )
#pragma pack(pop)