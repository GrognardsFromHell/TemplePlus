#pragma once

const uint32_t SPELL_ENUM_MAX = 802;
const uint32_t SPELL_ENUM_MAX_EXPANDED = 3999;
#define NUM_SPELL_LEVELS 10 // spells are levels 0-9
#define MAX_SPELL_TARGETS 32
#define INV_IDX_INVALID 255  // indicates that a spell is not an item spell

enum SpellStoreType : uint8_t{
	spellStoreNone = 0,
	spellStoreKnown = 1,
	spellStoreMemorized = 2,
	spellStoreCast = 3
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
	Arcane = 0,
	Divine,
	Ability,
	Psionic,
	Any
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
	Theurge // adds to both divine and arcane classes!
};

struct D20SpellData
{
	uint16_t spellEnumOrg;
	MetaMagicData metaMagicData;
	uint8_t spellClassCode;
	uint8_t itemSpellData;
	SpontCastType spontCastType : 4;
	unsigned char spellSlotLevel : 4;
	void Set(uint32_t spellEnum, uint32_t spellClassCode, uint32_t spellLevel, uint32_t invIdx, MetaMagicData metaMagicData);
	void Extract(int* spellEnum, int *spellEnumOrg, int* spellClass, int* spellLevel, int* invIdx, MetaMagicData* mmData);
	D20SpellData(){
		spellEnumOrg = 0;
		metaMagicData = 0;
		spellClassCode = 0;
		itemSpellData = INV_IDX_INVALID;
		spontCastType = SpontCastType::spontCastNone;
		spellSlotLevel = 0;
	}
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

#pragma pack(push,4)
struct SpellStoreData
{
	uint32_t spellEnum;
	uint32_t classCode;
	uint32_t spellLevel;
	SpellStoreState spellStoreState;
	MetaMagicData metaMagicData; // should be stored as 32bit value!
	char pad0;
	uint32_t pad1; // these are actually related to MM indicator icons
	uint32_t pad2;
	uint32_t pad3;
	SpellStoreData():spellEnum(0), classCode(0), spellLevel(0), pad0(0), pad1(0),pad2(0),pad3(0){
	};
	SpellStoreData(int SpellEnum, int SpellLevel, int ClassCode, int mmData, int SpellStoreData):SpellStoreData(){
		spellEnum = SpellEnum;
		classCode = ClassCode;
		spellLevel = SpellLevel;
		metaMagicData = MetaMagicData(mmData);
		spellStoreState = SpellStoreState(SpellStoreData);
	}

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
};
const auto TestSizeOfSpellStoreData = sizeof(SpellStoreData);
#pragma pack(pop)