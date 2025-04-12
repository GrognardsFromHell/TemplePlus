#pragma once

#include "common.h"
#include "config/config.h"

struct D20Actn;
const uint32_t SPELL_ENUM_MAX_VANILLA = 802;
const uint32_t SPELL_ENUM_MAX_EXPANDED = 3999;

#define NUM_SPELL_LEVELS 10 // spells are levels 0-9
#define NUM_SPELL_LEVELS_VANILLA 6 // 0-5
#define MAX_SPELL_TARGETS 32
#define INV_IDX_INVALID 255  // indicates that a spell is not an item spell
#define SPELL_ENUM_LABEL_START 803 // the range of 803-812 is reserved for "spell labels" in the chargen / levelup spell UI
#define SPELL_ENUM_VACANT 802 // used for vacant spellbook slots
#define SPELL_ENUM_NEW_SLOT_START 1605 // for bard/sorc new spells; range is in 1605-1614

enum SpellStoreType : uint8_t{
	spellStoreNone = 0,
	spellStoreKnown = 1,
	spellStoreMemorized = 2,
	spellStoreCast = 3,
	spellStoreAtWill // New! Todo implementation
};

enum MetaMagicFlags : uint8_t
{
	MetaMagic_Maximize = 1,
	MetaMagic_Quicken = 2,
	MetaMagic_Silent = 4,
	MetaMagic_Still = 8
};

enum SpellComponentFlag : uint32_t
{
	SpellComponent_Verbal = 1,
	SpellComponent_Somatic = 2,
	SpellComponent_XpCost = 4,
	SpellComponent_GpCost = 0x8,
};

struct MetaMagicData{
	unsigned char metaMagicFlags : 4; // 1 - Maximize Spell ; 2 - Quicken Spell ; 4 - Silent Spell;  8 - Still Spell
	unsigned char metaMagicEmpowerSpellCount : 4;
	unsigned char metaMagicEnlargeSpellCount : 4;
	unsigned char metaMagicExtendSpellCount : 4;
	unsigned char metaMagicHeightenSpellCount : 4;
	unsigned char metaMagicWidenSpellCount : 4;
	operator  uint32_t()	{
		return (*(uint32_t*)this) & 0xFFFFFF;
	}

	MetaMagicData()	{
		metaMagicFlags = 0;
		metaMagicEmpowerSpellCount = 0;
		metaMagicEnlargeSpellCount = 0;
		metaMagicExtendSpellCount = 0;
		metaMagicHeightenSpellCount = 0;
		metaMagicWidenSpellCount = 0;

	}

	MetaMagicData(unsigned int raw)
	{
		metaMagicFlags = raw & 0xF;
		metaMagicEmpowerSpellCount =  (raw & 0xF0) >> 4;
		metaMagicEnlargeSpellCount =  (raw & 0xF00) >> 8;
		metaMagicExtendSpellCount =   (raw & 0xF000) >> 12;
		metaMagicHeightenSpellCount = (raw & 0xF0000) >> 16;
		metaMagicWidenSpellCount =    (raw & 0xF00000) >> 20;
	}

	
};
const uint32_t TestSizeOfMetaMagicData = sizeof(MetaMagicData);

struct SpellStoreState{
	SpellStoreType spellStoreType;
	uint8_t usedUp; // relevant only for spellStoreMemorized
	SpellStoreState(){
		spellStoreType = SpellStoreType::spellStoreNone;
		usedUp = 0;
	}
	SpellStoreState(int storeStateRaw){
		spellStoreType = (SpellStoreType)(uint8_t)(storeStateRaw & 0xFF);
		usedUp = (uint8_t)(storeStateRaw >> 8);
	}
};

enum SpontCastType : unsigned char {
	spontCastNone = 0,
	spontCastGoodCleric = 2,
	spontCastEvilCleric = 4,
	spontCastDruid = 8
};

enum AiSpellType : unsigned {
	ai_action_summon = 0,
	ai_action_offensive = 1,
	ai_action_defensive = 2,
	ai_action_flee = 3,
	ai_action_heal_heavy = 4,
	ai_action_heal_medium = 5,
	ai_action_heal_light = 6,
	ai_action_cure_poison = 7,
	ai_action_resurrect = 8
};

enum class SpellSourceType : int {
	Ability = 0,
	Arcane = 1,
	Divine = 2,
	Psionic = 3,
	Any = 4
};

enum class SpellReadyingType : int {
	Vancian = 0, // memorization slots
	Innate, // bards / sorcerers etc.
	Any
};

enum class SpellListType : int {
	None = 0,
	Any, // for prestige classes that stack spell progression with anything
	Arcane,
	Bardic, // subset of Arcane
	Clerical, // subset of Divine 
	Divine,
	Druidic, // subset of divine
	Paladin, // subset of divine
	Psionic, 
	Ranger, // subset of divine
	Special, // "independent" list
	Extender // extends an existing spell list
};

#pragma pack(push,4)
struct SpellStoreData
{
	uint32_t spellEnum;
	uint32_t classCode;
	uint32_t spellLevel;
	SpellStoreState spellStoreState;
	uint16_t padSpellStore;
	MetaMagicData metaMagicData; // should be stored as 32bit value!
	char pad0;
	uint32_t pad1; // these are actually related to MM indicator icons
	uint32_t pad2;
	uint32_t pad3;
	SpellStoreData():spellEnum(0), classCode(0), spellLevel(0), pad0(0), pad1(0),pad2(0),pad3(0){
	};
	SpellStoreData(int SpellEnum, int SpellLevel, int ClassCode, int mmData, int SpellStoreData);/* :SpellStoreData() {
		spellEnum = SpellEnum;
		classCode = ClassCode;
		spellLevel = SpellLevel;
		metaMagicData = MetaMagicData(mmData);
		spellStoreState = SpellStoreState(SpellStoreData);
		padSpellStore = 0;
	}*/

	SpellStoreData(int SpellEnum, int SpellLevel, int ClassCode, MetaMagicData mmData, SpellStoreState SpellStoreData) :SpellStoreData() {
		spellEnum = SpellEnum;
		classCode = ClassCode;
		spellLevel = SpellLevel;
		metaMagicData = mmData;
		spellStoreState = SpellStoreData;
	}

	SpellStoreData(int SpellEnum, int SpellLevel, int ClassCode, MetaMagicData mmData) :SpellStoreData() {
		spellEnum = SpellEnum;
		classCode = ClassCode;
		spellLevel = SpellLevel;
		metaMagicData = mmData;
		spellStoreState = SpellStoreState(0);
	}
	bool operator < (const SpellStoreData& b);
	SpellComponentFlag GetSpellComponentFlags(); // regards metamagic data
};

bool operator < (const SpellStoreData& sp1, const SpellStoreData& sp2);
const auto TestSizeOfSpellStoreData = sizeof(SpellStoreData);
#pragma pack(pop)

struct D20SpellData
{
	uint16_t spellEnumOrg;
	MetaMagicData metaMagicData;
	uint8_t spellClassCode;
	uint8_t itemSpellData;
	SpontCastType spontCastType : 4;
	unsigned char spellSlotLevel : 4;
	void Set(uint32_t spellEnum, uint32_t spellClassCode, uint32_t spellLevel, uint32_t invIdx, MetaMagicData metaMagicData);
	void Extract(int* spellEnum, int *spellEnumOrg, int* spellClass, int* spellLevel, int* invIdx, MetaMagicData* mmData); // regards alt-node (shift clicking spells)
	D20SpellData() {
		spellEnumOrg = 0;
		metaMagicData = 0;
		spellClassCode = 0;
		itemSpellData = INV_IDX_INVALID;
		spontCastType = SpontCastType::spontCastNone;
		spellSlotLevel = 0;
	}
	D20SpellData(int spellEnum) : D20SpellData() {
		spellEnumOrg = spellEnum;
	}

	SpellStoreData ToSpellStore() {
		SpellStoreData spData;
		this->Extract((int*)&spData.spellEnum, nullptr, (int*)&spData.classCode, (int*)&spData.spellLevel, nullptr, &spData.metaMagicData);
		return spData;
	};
};

inline void D20SpellData::Set(uint32_t spellEnum, uint32_t SpellClassCode, uint32_t SpellLevel, uint32_t invIdx, MetaMagicData mmData)
{
	spellEnumOrg = spellEnum;
	metaMagicData = mmData;
	spellClassCode = SpellClassCode;
	itemSpellData = invIdx;
	spellSlotLevel = SpellLevel;
	spontCastType = SpontCastType::spontCastNone;
}

const uint32_t TestSizeOfD20SpellData = sizeof(D20SpellData);


enum SpellSchools : uint32_t
{
	School_None = 0,
	School_Abjuration = 1,
	School_Conjuration = 2,
	School_Divination = 3,
	School_Enchantment =4,
	School_Evocation = 5,
	School_Illusion = 6,
	School_Necromancy = 7,
	School_Transmutation = 8,
	// Added in Temple+
	School_Invocation = 9,
	School_Universal = 10
};


enum SpellAnimationFlag : int32_t
{
	SAF_UNK8 = 0x8,
	SAF_ID_ATTEMPTED = 0x10,
	SAF_PROJECTILE_STHG = 0x20,
};

enum SpellRangeType : uint32_t
{
	SRT_Specified = 0,
	SRT_Personal = 1,
	SRT_Touch,
	SRT_Close,
	SRT_Medium,
	SRT_Long,
	SRT_Unlimited,
	SRT_Special_Inivibility_Purge
};

struct SpellEntryLevelSpec
{
	uint32_t spellClass;
	uint32_t slotLevel;
	SpellEntryLevelSpec() { spellClass = 0; slotLevel = 0; }
};

struct SpellEntry {
	uint32_t spellEnum;
	uint32_t spellSchoolEnum;
	uint32_t spellSubSchoolEnum;
	uint32_t spellDescriptorBitmask;
	uint32_t spellComponentBitmask;
	uint32_t costGp;
	uint32_t costXp;
	uint32_t castingTimeType; // 0 - "1 Action", 1 - "Full Round", 2 - "Out of Combat", 3 - "Safe", 4 - "Free Action"
	SpellRangeType spellRangeType;
	uint32_t spellRange;
	uint32_t savingThrowType;
	uint32_t spellResistanceCode;
	SpellEntryLevelSpec spellLvls[10];
	uint32_t spellLvlsNum;
	uint32_t projectileFlag;
	uint64_t flagsTargetBitmask;
	uint64_t incFlagsTargetBitmask;
	uint64_t excFlagsTargetBitmask;
	uint64_t modeTargetSemiBitmask; // UiPickerType
	uint32_t minTarget;
	uint32_t maxTarget;
	int radiusTarget; //note:	if it's negative, then its absolute value is used as SpellRangeType for mode_target personal; if it's positive, it's a specified number(in feet ? )
	int degreesTarget;
	uint32_t aiTypeBitmask; // see AiSpellType in spell_structs.h
	uint32_t pad;

	//UiPickerType GetModeTarget() const;
	SpellEntry();
	explicit SpellEntry(uint32_t spellEnum);
	bool IsBaseModeTarget(UiPickerType type);
	int SpellLevelForSpellClass(int spellClass); // returns -1 if none
	int GetLowestSpellLevel(uint32_t spellEnumIn);  ///Gets lowest level the spell is for any class
	PnPSource GetSource();
	bool IsSourceEnabled();
};

struct ProjectileEntry {
	D20Actn* d20a;
	int pad4;
	objHndl projectile;
	objHndl ammoItem;
};
