#pragma once

#include "common.h"
#include "skill.h"

#define DISPATCHER_MAX  250 // max num of simultaneous Dispatches going on (static int counter inside _DispatcherProcessor)
#include "spell_structs.h"

struct CondStructNew;
struct BuffDebuffPacket;
struct DispIO;
struct DispIoCondStruct; // 1
struct DispIoBonusList; // 2
struct DispIoSavingThrow; // 3
struct DispIoDamage; // 4
struct DispIoAttackBonus; // 5
struct DispIoD20Signal; // 6
struct DispIoD20Query; // 7
struct DispIOTurnBasedStatus; // 8
struct DispIoTooltip; // 9
struct DispIoObjBonus; // 10
struct DispIoDispelCheck; // 11
struct DispIoD20ActionTurnBased; // 12
struct DispIoMoveSpeed; //13
struct DispIOBonusListAndSpellEntry; // 14
struct DispIoObjEvent; // 17
struct DispIoSpellsPerDay; // 18
struct DispIoAbilityLoss; // 19
struct DispIoAttackDice; // 20
struct DispIoImmunity; //23
struct DispIoEffectTooltip; // 24
struct EvtObjSpellCaster; // 34;
struct EvtObjActionCost; // 35
struct D20Actn;
struct DamagePacket;

struct SpellEntry;
struct SpellPacketBody;

struct TurnBasedStatus;
struct BonusList;
struct CondNode;

struct SubDispDef;
struct CondStruct;
struct DispatcherCallbackArgs;
struct Dispatcher;


struct DispatcherSystem : temple::AddressTable
{
	Dispatcher* DispatcherInit(objHndl objHnd);
	bool dispatcherValid(Dispatcher * dispatcher);
	void DispatcherProcessor(Dispatcher * dispatcher, enum_disp_type dispType, uint32_t dispKey, DispIO * dispIO);
	void DispatcherProcessorForItems(CondStruct* cond, int condArgs[64], enum_disp_type dispType, D20DispatcherKey key, DispIO* dispIo);

	void  DispatcherClearField(Dispatcher * dispatcher, CondNode ** dispCondList);
	void  DispatcherClearPermanentMods(Dispatcher * dispatcher);
	void  DispatcherClearItemConds(Dispatcher * dispatcher);
	void  DispatcherClearConds(Dispatcher *dispatcher);
	
	int DispatchForCritter(objHndl handle, DispIoBonusList*, enum_disp_type dispType, D20DispatcherKey dispKey);
	void DispatchForItem(objHndl item, enum_disp_type dispType, D20DispatcherKey key, DispIO* dispIo);
	void DispatchForWearable(objHndl item, enum_disp_type dispType, D20DispatcherKey key, DispIO* dispIo);
	int Dispatch10AbilityScoreLevelGet(objHndl handle, Stat stat, DispIoBonusList * dispIo); // use objects.abilityScoreLevelGet instead (to include NPC stat boost mod)
	int32_t dispatch1ESkillLevel(objHndl objHnd, SkillEnum skill, BonusList * bonOut, objHndl objHnd2, int32_t flag);
	float Dispatch29hGetMoveSpeed(objHndl objHnd, DispIoMoveSpeed * dispIo = nullptr);
	float Dispatch40GetBaseMoveSpeed(objHndl objHnd, DispIoMoveSpeed * dispIo = nullptr);
	void dispIOTurnBasedStatusInit(DispIOTurnBasedStatus* dispIOtbStat);
	void dispatchTurnBasedStatusInit(objHndl objHnd, DispIOTurnBasedStatus* dispIOtB);

	int DispatchSavingThrow(objHndl handle, DispIoSavingThrow* evtObj, enum_disp_type enumDisp, D20DispatcherKey d20DispatcherKey);
	int Dispatch13SavingThrow(objHndl handle, SavingThrowType saveType, DispIoSavingThrow* evtObj);
	int Dispatch14SavingThrowMod(objHndl handle, SavingThrowType saveType, DispIoSavingThrow* evtObj);
	int Dispatch44FinalSaveThrow(objHndl handle, SavingThrowType saveType, DispIoSavingThrow* evtObj);
	

#pragma region event object checkers

	DispIoCondStruct* DispIoCheckIoType1(DispIoCondStruct* dispIo); // used for ConditionAdd (3); 1,2 and 4 dispatch will null eventObjs
	DispIoCondStruct* DispIoCheckIoType1(DispIO* dispIo);
	DispIoBonusList* DispIoCheckIoType2(DispIoBonusList* dispIoBonusList); // used for stat level (10), stat base (66) and Cur/Max HP
	DispIoBonusList* DispIoCheckIoType2(DispIO* dispIoBonusList); // used for stat level (10), stat base (66) and Cur/Max HP
	DispIoSavingThrow* DispIoCheckIoType3(DispIoSavingThrow* dispIoBonusList);  
	DispIoSavingThrow* DispIoCheckIoType3(DispIO* dispIoBonusList);
	DispIoDamage * DispIoCheckIoType4(DispIoDamage* dispIo);
	DispIoDamage * DispIoCheckIoType4(DispIO* dispIo);
	DispIoAttackBonus * DispIoCheckIoType5(DispIoAttackBonus* dispIo);
	DispIoAttackBonus * DispIoCheckIoType5(DispIO* dispIo);
	DispIoD20Signal* DispIoCheckIoType6(DispIoD20Signal* dispIo);
	DispIoD20Signal* DispIoCheckIoType6(DispIO* dispIo);
	DispIoD20Query* DispIoCheckIoType7(DispIoD20Query* dispIo);
	DispIoD20Query* DispIoCheckIoType7(DispIO* dispIo);
	DispIOTurnBasedStatus* DispIoCheckIoType8(DispIOTurnBasedStatus* dispIo);
	DispIOTurnBasedStatus* DispIoCheckIoType8(DispIO* dispIo);
	DispIoTooltip* DispIoCheckIoType9(DispIO* dispIo);
	DispIoTooltip* DispIoCheckIoType9(DispIoTooltip* dispIo);
	DispIoObjBonus* DispIoCheckIoType10(DispIoObjBonus* dispIo);
	DispIoObjBonus* DispIoCheckIoType10(DispIO* dispIo);
	DispIoDispelCheck* DispIOCheckIoType11(DispIoDispelCheck* dispIo);
	static DispIoD20ActionTurnBased* DispIoCheckIoType12(DispIoD20ActionTurnBased* dispIo);
	DispIoD20ActionTurnBased* DispIoCheckIoType12(DispIO* dispIo);
	DispIoMoveSpeed * DispIoCheckIoType13(DispIoMoveSpeed* dispIo);
	DispIoMoveSpeed * DispIoCheckIoType13(DispIO* dispIo);
	static DispIoObjEvent* DispIoCheckIoType17(DispIO* dispIo);
	DispIoAttackDice* DispIoCheckIoType20(DispIO* dispIo);
	DispIoImmunity* DispIoCheckIoType23(DispIoImmunity* dispIo);
	DispIoImmunity* DispIoCheckIoType23(DispIO* dispIo);
	DispIoEffectTooltip* DispIoCheckIoType24(DispIoEffectTooltip* dispIo);
	DispIoEffectTooltip* DispIoCheckIoType24(DispIO* dispIo);
	DispIOBonusListAndSpellEntry* DispIOCheckIoType14(DispIOBonusListAndSpellEntry* dispIo);
	void PackDispatcherIntoObjFields(objHndl objHnd, Dispatcher* dispatcher);
	int DispatchDispelCheck(objHndl handle, int spellId, int flags, int returnValue);
	int DispatchAttackBonus(objHndl objHnd, objHndl victim, DispIoAttackBonus* dispIo, enum_disp_type dispType, int key);
	int DispatchToHitBonusBase(objHndl objHndCaller, DispIoAttackBonus* dispIo);
	int DispatchGetSizeCategory(objHndl objHndCaller, bool base = false);
	void DispatchConditionRemove(Dispatcher* dispatcher, CondNode* cond);
	unsigned int Dispatch35CasterLevelModify(objHndl obj, SpellPacketBody* spellPkt);
	void DispatchSpellResistanceCasterLevelCheck(objHndl caster, objHndl target, BonusList *bonlist, SpellPacketBody*spellPkt);
	void DispatchTargetSpellDCBonus(objHndl caster, objHndl target, BonusList *bonlist, SpellPacketBody*spellPkt);
	bool DispatchIgnoreDruidOathCheck(objHndl character, objHndl item);
	void DispatchMetaMagicModify(objHndl obj, MetaMagicData& mmData, unsigned char spellLevel, uint16_t spellEnum, uint32_t spellClass);
	void DispatchSpecialAttack(objHndl obj, int attack, objHndl target);
	double DispatchRangeBonus(objHndl obj, objHndl weaponUsed);
	int DispatchSpellListLevelExtension(objHndl obj, Stat casterClass);
	int DispatchSpellsPerDay(objHndl obj, Stat casterClass, int spellLevel, int effectiveLvl);
	int DispatchGetBaseCasterLevel(objHndl obj, Stat casterClass);
	int DispatchGetCasterLevelStage2(objHndl handle, Stat casterClass, int initialVal);

	int DispatchLevelupSystemEvent(objHndl handle, Stat casterClass, D20DispatcherKey evtType);
	void DispatchLevelupSpellsFinalize(objHndl handle, Stat casterClass);

	int Dispatch45SpellResistanceMod(objHndl handle, DispIOBonusListAndSpellEntry* dispIo);
	void Dispatch48BeginRound(objHndl obj, int numRounds) const;
	bool Dispatch64ImmunityCheck(objHndl handle, DispIoImmunity* dispIo);
	void Dispatch68ItemRemove(objHndl handle);
#pragma endregion

	void DispIoDamageInit(DispIoDamage *dispIoDamage);
	int32_t DispatchDamage(objHndl objHnd, DispIoDamage* dispIoDamage, enum_disp_type enumDispType, D20DispatcherKey d20DispatcherKey);
	void DispatchSpellDamage(objHndl obj, DamagePacket* damage, objHndl target, SpellPacketBody *spellPkt);
	int DispatchD20ActionCheck(D20Actn* d20Actn, TurnBasedStatus* turnBasedStatus, enum_disp_type dispType);
	int Dispatch60GetAttackDice(objHndl obj, DispIoAttackDice * dispIo);
	int Dispatch61GetLevel(objHndl obj, Stat stat, BonusList* bonlist = nullptr, objHndl someObj = objHndl::null, LevelDrainType omit = LevelDrainType::NoDrainedLevel); // get level after accounting for level drain effects

	int DispatchGetBonus(objHndl critter, DispIoBonusList* bonusListOut, enum_disp_type dispType, D20DispatcherKey key);

	
	int DispatchItemQuery(objHndl item, D20DispatcherKey d20DispatcherKey);

	DispatcherSystem()
	{
		rebase(_Dispatch29hMovementSthg,0x1004D080); 
	}

	


private:
	void(__cdecl *_Dispatch29hMovementSthg)(objHndl objHnd, void *);
};

extern DispatcherSystem dispatch;

#pragma pack(push, 1)

#pragma region Dispatcher Structs

struct DispIO {
	enum_dispIO_type dispIOType;

	void AssertType(enum_dispIO_type eventObjType) const;
};

struct CondNode : temple::TempleAlloc {
	CondStruct* condStruct;
	CondNode* nextCondNode;
	uint32_t flags; // 1 - expired; 2 - got arg data from info stored in field
	uint32_t args[10];

	explicit CondNode(CondStruct *cond);
	bool IsExpired() { return (flags & 1) != 0; };
};

struct SubDispNode : temple::TempleAlloc {
	SubDispDef* subDispDef;
	CondNode* condNode;
	SubDispNode* next;
};

struct SubDispDef {
	enum_disp_type dispType;
	uint32_t dispKey;
	void(__cdecl *dispCallback)(SubDispNode* subDispNode, objHndl objHnd, enum_disp_type dispType, uint32_t dispKey, DispIO* dispIO);
	uint32_t data1;
	uint32_t data2;
};

struct SubDispDefNew {
	enum_disp_type dispType;
	uint32_t dispKey;
	int(__cdecl *dispCallback)(DispatcherCallbackArgs args);
	typedef union {
		uint32_t usVal;
		int sVal;
		CondStructNew* condStruct;
		const char* cs;
	} Data;
	Data data1;
	Data data2;

	SubDispDefNew();
	SubDispDefNew(enum_disp_type type, uint32_t key, int(__cdecl *callback)(DispatcherCallbackArgs), CondStructNew* data1, uint32_t data2);
	SubDispDefNew(enum_disp_type type, uint32_t key, int(__cdecl *callback)(DispatcherCallbackArgs), uint32_t data1, uint32_t data2);
	//SubDispDefNew(enum_disp_type type, uint32_t key, int(__cdecl *callback)(DispatcherCallbackArgs), uint32_t data1, const char* cs);
};

const int testSizeofSubDispDef = sizeof(SubDispDefNew);

/*
	This defines a condition and what it does, while CondNode represents an instance of this or another condition.
*/
struct CondStruct {
	char* condName;
	unsigned int numArgs;
	/*
		This is a variable length array of dispatcher hooks that this condition has. Terminated by null.
	*/
	SubDispDef subDispDefs[1];
};

struct CondStructNew{
	const char* condName;
	unsigned int numArgs;
	SubDispDefNew subDispDefs[100]; // assumes the last one is 0!
	int numHooks = 0; 

	CondStructNew();
	CondStructNew(std::string Name, int NumArgs, bool preventDuplicate = true); // use preventDuplicate = true for "unique" conditions or conditions that should never stack / apply twice
	CondStructNew(CondStruct & existingCondStruct); // Extends existing cond struct
	void AddHook(enum_disp_type dispType, D20DispatcherKey dispKey, int(*callback)(DispatcherCallbackArgs) );
	void AddHook(enum_disp_type dispType, D20DispatcherKey dispKey, int(*callback)(DispatcherCallbackArgs), uint32_t data1, uint32_t data2);
	void AddHook(enum_disp_type dispType, D20DispatcherKey dispKey, int(*callback)(DispatcherCallbackArgs), CondStructNew* data1, uint32_t data2);
	void AddHook(enum_disp_type dispType, D20DispatcherKey dispKey, int(*callback)(DispatcherCallbackArgs), CondStruct* data1, uint32_t data2);
	// void AddHook(enum_disp_type dispType, D20DispatcherKey dispKey, int(*callback)(DispatcherCallbackArgs), uint32_t data1, const char *data2); // this fucks things up :(
	//void AddPyHook(enum_disp_type dispType, D20DispatcherKey dispKey, PyObject* pycallback, PyObject* pydataTuple);
	//void AddPyHook(enum_disp_type dispType, D20DispatcherKey dispKey, pybind11::function pycallback, pybind11::tuple pydataTuple);
	void Register();
	void ExtendExisting(const std::string &condName);
	void AddToFeatDictionary(feat_enums feat, feat_enums featEnumMax = FEAT_INVALID, uint32_t condArg2Offset = 0);

	// standard callbacks
	void AddAoESpellRemover();
	
};

struct DispatcherCallbackArgs {
	SubDispNode* subDispNode;
	objHndl objHndCaller;
	enum_disp_type dispType;
	uint32_t dispKey;
	DispIO* dispIO;
	int32_t GetCondArg(uint32_t argIdx);
	objHndl GetCondArgObjHndl(uint32_t argIdx);
	void* GetCondArgPtr(uint32_t argIdxI);
	int GetData1() const; // gets the data1 value from the subDispDef
	int GetData2() const;
	void SetCondArg(uint32_t argIdx, int value);
	void SetCondArgObjHndl(uint32_t argIdx, const objHndl& handle);
	void RemoveCondition(); // note: this is the low level function
	void RemoveSpellMod();
	void RemoveSpell(); // general spell remover
	void SetExpired(); // set expired flag on condnode
	
};

struct DispIoCondStruct : DispIO { // DispIoType = 1
	CondStruct* condStruct;
	uint32_t outputFlag;
	uint32_t arg1;
	uint32_t arg2;

	DispIoCondStruct() {
		dispIOType = dispIoTypeCondStruct;
		condStruct = nullptr;
		outputFlag = 0;
		arg1 = 0;
		arg2 = 0;
	}
};



struct DispIoBonusList : DispIO { // DispIoType = 2  used for fetching ability scores (dispType 10, 66), and Cur/Max HP 
	BonusList bonlist;
	/*
	Dispatch46GetSpellDcBase (0x1004FF90) sets this to 1
	Checked in 0x100C5C30 vs 2 for eagle's splendor spell (used to ignore the bonus from the spell);
	  I haven't seen that flag set anywhere though
	  Temple+ : added flag 0x4 for statLevelBase query that includes permanent effect items (e.g. "Attribute Enhancement Bonus" condition)
	*/
	uint32_t flags; 
	DispIoBonusList(){
		dispIOType = dispIOTypeBonusList;
		flags = 0;
	}
};


struct DispIoSavingThrow : DispIO { // DispIoType = 3
	uint32_t returVal;
	objHndl obj;
	uint64_t flags; // see D20SavingThrowFlag looks like: 2 - trap, 0x10 - Spell, 0x20 thru 0x1000 - spell schools (abjuration thru transmutation, e.g. 0x100 - enchantment), 0x100000 - fear/morale effect?
	BonusList bonlist;
	int rollResult;

	DispIoSavingThrow();
};

struct DispIoAttackBonus : DispIO { // DispIoType 5
	int field_4;
	AttackPacket attackPacket;
	BonusList bonlist;
	DispIoAttackBonus();
	int Dispatch(objHndl obj, objHndl obj2, enum_disp_type dispType, D20DispatcherKey key);
};


struct DispIoD20Signal : DispIO // DispIoType 6
{
	uint32_t return_val;
	uint32_t data1;
	uint32_t data2;

	DispIoD20Signal(){
		dispIOType = dispIoTypeSendSignal;
		return_val = 0;
		data1 = 0;
		data2 = 0;
	}
};


struct DispIoD20Query : DispIO // DispIoType 7
{
	int return_val; // changed to int type to avoid python casting madness
	uint32_t data1;
	uint32_t data2;

	DispIoD20Query(){
		dispIOType = dispIOTypeQuery;
		return_val = 0;
		data1 = 0;
		data2 = 0;
	}
};


struct DispIOTurnBasedStatus : DispIO // type 8
{
	TurnBasedStatus * tbStatus;
	DispIOTurnBasedStatus(){
		dispIOType = dispIOTypeTurnBasedStatus;
		tbStatus = nullptr;
	};
};

struct DispIoTooltip : DispIO // DispIoType 9 ; tooltip additional text when hovering over an object in the game
{
	char strings[10][256];
	uint32_t numStrings;
	void Append(std::string& cs);
};
const auto TestSizeOfDispIoTooltip = sizeof(DispIoTooltip); // should be 2568  (0xA08)


struct DispIoObjBonus : DispIO // type 10
{
	uint32_t flags;
	BonusList * bonOut;
	uint32_t pad;
	objHndl obj; //optional
	BonusList bonlist;
	DispIoObjBonus();
};
const int TestSizeOfDispIoObjBonus = sizeof(DispIoObjBonus); // should be 912 (0x390)

struct DispIoDispelCheck : DispIO // type 11
{
	// Associated with the flags variable
	enum DispelSpellType {
		DispelMagicSingle = 0x1,  //Dispel Magic (area or single)
		DispelEvil = 0x2,
		DispelChaos = 0x4,
		DispelGood = 0x8,
		DispelLaw = 0x10,
		DispelAlignment = DispelEvil | DispelChaos | DispelGood | DispelLaw,
		SliperyMind = 0x20,
		BreakEnchantment = 0x40,
		DispelMagic = 0x80,  //Area dispel magic
		DispelAir = 0x100,
		DispelEarth = 0x200,
		DispelFire = 0x400,
		DispelWater = 0x800,
		DispelElement = DispelAir | DispelEarth | DispelFire | DispelWater,
	};

	uint32_t spellId; // of the Dispel Spell (Break Enchantment, Dispel Magic etc.)
	uint32_t flags;  // Dispel Type flag
	uint32_t returnVal;
};

struct DispIoD20ActionTurnBased : DispIO { // dispIoType = 12; matches dispTypes 36-38 , 52
	int returnVal;
	D20Actn * d20a;
	TurnBasedStatus * tbStatus;
	BonusList* bonlist; // NEW (extended vanilla) 

	DispIoD20ActionTurnBased();
	explicit DispIoD20ActionTurnBased(D20Actn* d20a);
	void DispatchPerform(D20DispatcherKey key);
	void DispatchPythonAdf(D20DispatcherKey key);
	void DispatchPythonActionCheck(D20DispatcherKey key);
	void DispatchPythonActionAddToSeq(D20DispatcherKey key);
	void DispatchPythonActionPerform(D20DispatcherKey key);
	void DispatchPythonActionFrame(D20DispatcherKey key);
};

struct DispIoMoveSpeed : DispIO  // dispIoType = 13, matches dispTypes 40,41
{
	BonusList* bonlist;
	float factor;
	DispIoMoveSpeed() {
		dispIOType = dispIOTypeMoveSpeed;
		factor = 1.0;
		bonlist = nullptr;
	}

};



struct DispIOBonusListAndSpellEntry: DispIO { // Type 14
	BonusList * bonList;
	SpellEntry * spellEntry;
	uint32_t field_C; // unused?
	DispIOBonusListAndSpellEntry(){
		dispIOType = dispIoTypeBonusListAndSpellEntry;
		bonList = nullptr;
		spellEntry = nullptr;
		field_C = 0;
	}
	int Dispatch(objHndl handle, enum_disp_type evtType);
};

struct DispIoReflexThrow : DispIO { // DispIoType = 15
	int effectiveReduction;
	D20SavingThrowReduction reduction;
	int damageMesLine;
	D20AttackPower attackPower;
	int attackType;
	int throwResult;
	D20SavingThrowFlag	flags;
};

struct DispIoObjEvent : DispIO // type 17
{
	int pad;
	objHndl aoeObj;
	objHndl tgt;
	uint32_t evtId;
	DispIoObjEvent()
	{
		dispIOType = dispIoTypeObjEvent;
		aoeObj = 0i64;
		tgt = 0i64;
		evtId = 0;
		pad = 0;
	}
};

struct DispIoSpellsPerDay : DispIO // type 18
{
	BonusList* bonList;
	int unk;
	int unk2;
	
	// extensions:
	Stat classCode;
	int spellLvl;
	int casterEffLvl;

	DispIoSpellsPerDay();
};



struct DispIoAbilityLoss: DispIO//  type 19
{
	int result;
	Stat statDamaged;
	int fieldC;
	int spellId;
	int flags; // 8 - marked at the beginning of dispatch; 0x10 - checks against this in the Temp/Perm ability damage

	DispIoAbilityLoss(){
		dispIOType = dispIOType19;
		result = 0;
		statDamaged = Stat::stat_strength;
		fieldC = 0;
		spellId = 0;
		flags = 0;
	}

};

struct DispIoAttackDice : DispIO // type 20
{
	BonusList * bonlist;
	D20CAF flags;
	int fieldC;
	objHndl weapon;
	objHndl wielder;
	int dicePacked;
	DamageType attackDamageType;
	DispIoAttackDice()	{
		dispIOType = dispIOType20;
		bonlist = nullptr;
		flags = D20CAF_HIT;
		dicePacked = 0;
		weapon = 0;
		wielder = 0;
	};
};

struct DispIoTypeImmunityTrigger : DispIO { // DispIoType 21
	uint32_t interrupt;
	uint32_t field_8;
	uint32_t field_C; // might have been used to store dispType, judging by the Globe of Invulnerability callback (probably tested this vs. BeginRound)
	uint32_t SDDKey1;
	uint32_t val2;
	uint32_t okToAdd; // or spellId???
	CondNode* condNode;
	enum_disp_type dispType;
	D20DispatcherKey dispKey;

	DispIoTypeImmunityTrigger();
};

struct DispIoImmunity : DispIO // type 23
{
	int returnVal;
	int field8;
	int flag;
	SpellPacketBody * spellPkt;
	SpellEntry spellEntry;
	DispIoImmunity()
	{
		dispIOType = dispIOTypeImmunityHandler;
		returnVal = 0;
		field8 = 0;
		flag = 0;
	}
};



struct DispIoEffectTooltip: DispIO // type 24
{
	BuffDebuffPacket* bdb;
	DispIoEffectTooltip()
	{
		dispIOType = dispIOTypeEffectTooltip;
		bdb = nullptr;
	}

	/*
	 spellEnum = -1 for no spell
	see uiParty.IndicatorTextGet
	*/
	void Append(int effectTypeId, int spellEnum, const char* text) const;
};


struct EvtObjSpellCaster: DispIO // type 34 (NEW!)
{
	BonusList bonlist;
	objHndl handle;
	int arg0;
	int arg1;
	SpellPacketBody* spellPkt;
	EvtObjSpellCaster() { dispIOType = evtObjTypeSpellCaster; handle = objHndl::null; arg0 = 0; arg1 = 0; spellPkt = nullptr; };
};

struct EvtObjMetaMagic : DispIO // type 35 (NEW!)
{
	MetaMagicData mmData;
	int spellLevel;
	uint32_t spellEnum;
	uint32_t spellClass;
};

struct EvtObjSpecialAttack : DispIO // type 36 (NEW!)
{
	enum AttackType {STUNNING_FIST=1, NUM_EFFECTS};

	int attack;  //Uses the attack enum but unfortunately the enum can't be passed through to python
	objHndl target;
};

struct EvtObjRangeIncrementBonus : DispIO // type 37 (NEW!)
{
	objHndl weaponUsed = objHndl::null;
	double rangeBonus = 0.0;
};

struct EvtObjDealingSpellDamage : DispIO // type 38 (NEW!)
{
	DamagePacket* damage = nullptr;
	SpellPacketBody* spellPkt = nullptr;
	objHndl target = objHndl::null;
};

struct EvtObjSpellTargetBonus : DispIO // type 39 (NEW!)
{
	BonusList* bonusList = nullptr;
	SpellPacketBody* spellPkt = nullptr;
	objHndl target = objHndl::null;
};

struct EvtIgnoreDruidOathCheck : DispIO // type 40 (NEW!)
{
	objHndl item = objHndl::null;
	bool ignoreDruidOath = false;
};

struct EvtObjAddMesh : DispIO // type 41 (new)
{
	objHndl handle;
	std::vector<int> addmeshes;
	EvtObjAddMesh(objHndl handle);
	std::vector<int> DispatchGetAddMeshes();
	void Append(int addmeshId); // index into addmesh.mes
};

struct EvtObjActionCost: DispIO
{
	ActionCostPacket acpOrig; // original
	ActionCostPacket acpCur; // current
	D20Actn *d20a;
	TurnBasedStatus *tbStat;

	EvtObjActionCost()
	{
		dispIOType = evtObjTypeActionCost;
	};

	EvtObjActionCost(ActionCostPacket acp, TurnBasedStatus *tbStatIn, D20Actn* d20aIn) : EvtObjActionCost(){
		this->acpOrig = acp;
		this->acpCur = acp;
		this->d20a = d20aIn;
		this->tbStat = tbStatIn;
	}
	void DispatchCost(D20DispatcherKey key = DK_NONE);
};



struct Dispatcher : temple::TempleAlloc {
	objHndl objHnd;
	CondNode* permanentMods;
	CondNode* itemConds;
	CondNode* conditions;
	SubDispNode* subDispNodes[dispTypeCount];
	bool IsValid();
	void Process(enum_disp_type dispTypeInitiativeMod, D20DispatcherKey key, DispIO* dispIo);
};

#pragma endregion

#pragma pack(pop)

#pragma region Dispatcher Functions

Dispatcher* _DispatcherInit(objHndl objHnd);

void  _DispatcherRemoveSubDispNodes(Dispatcher * dispatcher, CondNode * cond);
void  _DispatcherClearField(Dispatcher *dispatcher, CondNode ** dispCondList);
void  _DispatcherClearPermanentMods(Dispatcher *dispatcher);
void  _DispatcherClearItemConds(Dispatcher *dispatcher);
void  _DispatcherClearConds(Dispatcher *dispatcher);

DispIoCondStruct * _DispIoCheckIoType1(DispIoCondStruct * dispIo);
DispIoBonusList * _DispIoCheckIoType2(DispIoBonusList * dispIo);
DispIoSavingThrow* _DispIOCheckIoType3(DispIoSavingThrow* dispIo);
DispIoDamage* _DispIoCheckIoType4(DispIoDamage* dispIo);
DispIoAttackBonus* _DispIoCheckIoType5(DispIoAttackBonus* dispIo);
DispIoD20Signal* _DispIoCheckIoType6(DispIoD20Signal* dispIo);
DispIoD20Query* _DispIoCheckIoType7(DispIoD20Query* dispIo);
DispIOTurnBasedStatus * _DispIoCheckIoType8(DispIOTurnBasedStatus* dispIo);
DispIoTooltip* _DispIoCheckIoType9(DispIoTooltip* dispIo);
DispIoObjBonus * _DispIoCheckIoType10(DispIoObjBonus *dispIo);
DispIoDispelCheck * _DispIoCheckIoType11(DispIoDispelCheck* dispIo);
DispIoD20ActionTurnBased * _DispIoCheckIoType12(DispIoD20ActionTurnBased* dispIo);
DispIOBonusListAndSpellEntry * __cdecl _DispIoCheckIoType14(DispIOBonusListAndSpellEntry *dispIO);
void __cdecl _PackDispatcherIntoObjFields(objHndl objHnd, Dispatcher*dispatcher);

void DispIOType21Init(DispIoTypeImmunityTrigger* dispIO);

void _dispatchTurnBasedStatusInit(objHndl objHnd, DispIOTurnBasedStatus* dispIOtB);
int _DispatchAttackBonus(objHndl objHnd, objHndl victim, DispIoAttackBonus* dispIo, enum_disp_type dispType, int key);
uint32_t _Dispatch62(objHndl, DispIO*, uint32_t dispKey);
uint32_t _Dispatch63(objHndl objHnd, DispIO* dispIO);

void _DispatcherProcessor(Dispatcher* dispatcher, enum_disp_type dispType, D20DispatcherKey dispKey, DispIO * dispIO);
int32_t _DispatchDamage(objHndl objHnd, DispIoDamage* dispIo, enum_disp_type dispType, D20DispatcherKey key);
int32_t _dispatch1ESkillLevel(objHndl objHnd, SkillEnum skill, BonusList* bonOut, objHndl objHnd2, int32_t flag);



#pragma endregion