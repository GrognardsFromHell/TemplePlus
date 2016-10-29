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

enum ClassDefinitionFlag : int
{
	CDF_BaseClass = 0x1, // denotes whether class is base class (can be taken at level 1, and factors into multiclass calculations; unlike Prestige Classes for instance)
	CDF_CoreClass = 0x2,  // is the class drawn from Core 3.5 rules?
};

struct ClassPacket;

struct D20ClassSpec {
	Stat classEnum;
	ClassDefinitionFlag flags;
	BABProgressionType babProgression;
	Stat deityClass; // emulate deity compatibility of the vanilla classes


	bool fortitudeSaveIsFavored;
	bool reflexSaveIsFavored;
	bool willSaveIsFavored;
	int hitDice; // HD side (4,6,8 etc)
	int skillPts; // skill point per level
	
	std::map<SkillEnum, bool> classSkills; // dictionary denoting if a skill is a class skill
	std::map<feat_enums, int> classFeats; // dictionary denoting which level the feat is granted 
	std::string conditionName; // name of the accompanying condition (e.g. "Bard", "Sorcerer", "Mystic Theurge")

	// spell casting

	std::string spellCastingConditionName; // name of the accompanying Spell Casting condition (e.g. "Bard Spellcasting")
	SpellListType spellListType;
	std::map<int, int> spellList; // mapping Spell Enum -> Spell Level for this class. This information is also stored in spellSystem under spellEntryExt
	SpellReadyingType spellMemorizationType;
	SpellSourceType spellSourceType;
	std::map<int, std::vector<int>> spellsPerDay; // index is class level, vector enumerates spells per day for each spell level
	Stat spellStat; // stat that determines maximum spell level
};

class D20ClassSystem : temple::AddressTable
{
public:
	Stat vanillaClassEnums[VANILLA_NUM_CLASSES];
	std::vector<int> classEnums;
	std::vector<int> baseClassEnums;
	std::vector<Stat> classEnumsWithSpellLists; // prepare a list of classes with spell lists
	const int ClassLevelMax = 20;

	bool ReqsMet(const objHndl &handle, const Stat classCode); // class requirements met
	bool IsCompatibleWithAlignment(Stat classEnum, Alignment al);

	bool IsNaturalCastingClass(Stat classEnum, objHndl handle = objHndl::null);
	bool IsNaturalCastingClass(uint32_t classEnum);
	bool IsVancianCastingClass(Stat classEnum, objHndl handle = objHndl::null);
	bool IsCastingClass(Stat classEnum);
	bool HasSpellList(Stat classEnum); // does this class have its own spell list? (as opposed to extending another's like Mystic Theurge does), e.g. Blackguard, Assassin
	bool IsLateCastingClass(Stat classEnum); // for classes like Ranger / Paladin that start casting on level 4
	bool IsArcaneCastingClass(Stat stat, objHndl handle = objHndl::null); // classes who list SpellSourceType as Arcane; this is mostly used to retrieve spell properties, so Mystic Theurges need not apply since they don't have an independent spell list
	bool IsDivineCastingClass(Stat stat, objHndl handle = objHndl::null); // similar to IsArcaneCastingClass
	static bool HasDomainSpells(Stat classEnum);
	Stat GetSpellStat(Stat classEnum); // default - wisdom
	Stat GetDeityClass(Stat classEnum); // get effective class for deity selection
	int GetMaxSpellLevel(Stat classEnum, int characterLvl);
	std::string GetSpellCastingCondition(Stat classEnum);

	void ClassPacketAlloc(ClassPacket *classPkt); // allocates the three IdxTables within ClassPacket
	void ClassPacketDealloc(ClassPacket *classPkt);
	uint32_t GetClassPacket(Stat classEnum, ClassPacket *classPkt); // fills the struct with content based on classEnum (e.g. Barbarian Feats in the featsIdxTable). Also STUB FOR PRESTIGE CLASSES! TODO
	int GetBaseAttackBonus(Stat classCode, uint32_t classLvl); // gets the class's BAB
	bool IsSaveFavoredForClass(Stat classCode, int saveType);
	int GetSkillPts(Stat classEnum);
	int GetClassHitDice(Stat classEnum);
	

	const char* GetClassShortHelp(Stat classCode);

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
	int GetNumSpellsFromClass(objHndl obj, Stat classCode, int spellLvl, uint32_t classLvl, bool getFromStatMod = true); // returns -1 if none; must have >=0 spells per day before taking into account the stat mod

	// skills
	BOOL IsClassSkill(SkillEnum skillEnum, Stat classCode);

	// Feats
	bool HasFeat(feat_enums featEnum, Stat classEnum, int classLvl); // checks if the class definition has the feat at the selected level.

	// Levelup
	bool IsSelectingFeatsOnLevelup(objHndl handle, Stat classEnum);
	void LevelupGetBonusFeats( objHndl handle, Stat classEnum);

	bool IsSelectingSpellsOnLevelup(objHndl handle, Stat classEnum);
	void LevelupInitSpellSelection(objHndl handle, Stat classEnum, int classLvlNew = -1, int classLvlIncrease = 1);
	bool LevelupSpellsCheckComplete(objHndl handle, Stat classEnum);
	void LevelupSpellsFinalize(objHndl handle, Stat classEnum);
	
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

