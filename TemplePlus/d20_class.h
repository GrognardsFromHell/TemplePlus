#pragma once
#include "common.h"
#include "idxtables.h"
#include <map>
#include "spell_structs.h"
#include "skill.h"
#include "obj.h"

//#include <EASTL/hash_map.h>


enum BardicMusicSongType : int {
	BM_INSPIRE_COURAGE = 1,
	BM_COUNTER_SONG = 2,
	BM_FASCINATE = 3,
	BM_INSPIRE_COMPETENCE = 4,
	BM_SUGGESTION = 5,
	BM_INSPIRE_GREATNESS = 6,
	BM_SONG_OF_FREEDOM = 7,
	BM_INSPIRE_HEROICS = 8
};

enum WildShapeProtoIdx : int {
	WS_Deactivate = 999 + (1 << 24),
	WS_Wolf = 0,
	WS_Dire_Lizard,
	WS_Brown_Bear,
	WS_Polar_Bear,
	WS_Legendary_Rat,
	WS_Dire_Bear,
	WS_Giant_Snake,
	WS_Hill_Giant,
	WS_Elem_Large_Air,
	WS_Elem_Large_Earth,
	WS_Elem_Large_Fire,
	WS_Elem_Large_Water,
	WS_Elem_Huge_Air,
	WS_Elem_Huge_Earth,
	WS_Elem_Huge_Fire,
	WS_Elem_Huge_Water
};

enum class BABProgressionType{
	Martial = 0, // +1 every level
	SemiMartial = 1, // + 3/4
	NonMartial = 2 // + 1/2
};

struct ClassPacket;

struct D20ClassSpec {
	Stat classEnum;
	BABProgressionType babProgression;


	bool fortitudeSaveIsFavored;
	bool reflexSaveIsFavored;
	bool willSaveIsFavored;
	int hitDice; // HD side (4,6,8 etc)
	int skillPts; // skill point per level
	SpellListType spellListType;
	std::map<SkillEnum, bool> classSkills; // dictionary denoting if a skill is a class skill
	std::map<int, std::vector<feat_enums>> classFeats; // dictionary 

};

struct D20ClassSystem : temple::AddressTable
{
public:
	Stat vanillaClassEnums[VANILLA_NUM_CLASSES];
	std::map<int, int> classEnums;
	const int ClassLevelMax = 20;
	bool IsNaturalCastingClass(Stat classEnum, objHndl handle = objHndl::null);
	bool IsNaturalCastingClass(uint32_t classEnum);
	bool IsVancianCastingClass(Stat classEnum, objHndl handle = objHndl::null);
	bool IsCastingClass(Stat classEnum);
	bool IsLateCastingClass(Stat classEnum); // for classes like Ranger / Paladin that start casting on level 4
	bool IsArcaneCastingClass(Stat stat, objHndl handle = objHndl::null);
	static bool HasDomainSpells(Stat classEnum);

	void ClassPacketAlloc(ClassPacket *classPkt); // allocates the three IdxTables within ClassPacket
	void ClassPacketDealloc(ClassPacket *classPkt);
	uint32_t GetClassPacket(Stat classEnum, ClassPacket *classPkt); // fills the struct with content based on classEnum (e.g. Barbarian Feats in the featsIdxTable). Also STUB FOR PRESTIGE CLASSES! TODO
	int GetBaseAttackBonus(Stat classCode, uint32_t classLvl); // gets the class's BAB
	int GetSkillPts(Stat classEnum);
	

	struct WildShapeSpec {
		int protoId;
		uint32_t minLvl;
		int monCat;
		WildShapeSpec() { protoId = 0; minLvl = 1; monCat = mc_type_animal; }
		WildShapeSpec(int ProtoId, int MinLvl) :protoId(ProtoId), minLvl(MinLvl), monCat(mc_type_animal) {	};
		WildShapeSpec(int ProtoId, int MinLvl, int MonCat) :protoId(ProtoId), minLvl(MinLvl), monCat(MonCat) {	};

	};

	std::map<WildShapeProtoIdx, WildShapeSpec> wildShapeProtos =
	{ { WS_Wolf,{ 14050, 2 } }, // 2HD
	{ WS_Dire_Lizard ,{ 14450, 5 } }, // 5HD
	{ WS_Brown_Bear,{ 14053, 7 } }, // large, 6HD
	{ WS_Polar_Bear ,{ 14054, 8 } }, // large, 8HD
	{ WS_Legendary_Rat ,{ 14451, 6 } },
	{ WS_Dire_Bear ,{ 14506, 12 } },
	{ WS_Giant_Snake ,{ 14449, 10 } },
	{ WS_Hill_Giant ,{ 14217, 10000 } }, // this is added via a special option
	{ WS_Elem_Large_Air,{ 14292 , 16, mc_type_elemental } },
	{ WS_Elem_Large_Earth,{ 14296 , 16, mc_type_elemental } },
	{ WS_Elem_Large_Fire,{ 14298 , 16, mc_type_elemental } },
	{ WS_Elem_Large_Water,{ 14302 , 16, mc_type_elemental } },
	{ WS_Elem_Huge_Air,{ 14508 , 20, mc_type_elemental } },
	{ WS_Elem_Huge_Earth,{ 14509 , 20 , mc_type_elemental } },
	{ WS_Elem_Huge_Fire,{ 14510 , 20, mc_type_elemental } },
	{ WS_Elem_Huge_Water,{ 14511 , 20, mc_type_elemental } }
	};

	D20ClassSystem(){
		Stat _charClassEnums[VANILLA_NUM_CLASSES] = 
			{ stat_level_barbarian, stat_level_bard, 
			stat_level_cleric, stat_level_druid, stat_level_fighter, stat_level_monk, stat_level_paladin, stat_level_ranger, stat_level_rogue, stat_level_sorcerer, stat_level_wizard };
		memcpy(vanillaClassEnums, _charClassEnums, VANILLA_NUM_CLASSES * sizeof(uint32_t));
	}
	void GetClassSpecs(); // gets class specs from python files

	int ClericMaxSpellLvl(uint32_t clericLvl) const;
	int NumDomainSpellsKnownFromClass(objHndl dude, Stat classCode);
	static int GetNumSpellsFromClass(objHndl obj, Stat classCode, int spellLvl, uint32_t classLvl);

	// skills
	BOOL IsClassSkill(SkillEnum skillEnum, Stat classCode);
protected:
	std::map<int, D20ClassSpec> classSpecs;
};

extern D20ClassSystem d20ClassSys;

#pragma pack(push, 1)
struct ClassPacket{
	IdxTable<uint32_t> keyStats; // stats considered key to this class (e.g. int, dex and con for wizards)
	IdxTable<uint32_t> alignments; // allowed alignments for this class
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

