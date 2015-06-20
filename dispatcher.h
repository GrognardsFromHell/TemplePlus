#pragma once

#include "common.h"
#include "skill.h"

#define DISPATCHER_MAX  250 // max num of simultaneous Dispatches going on (static int counter inside _DispatcherProcessor)



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
struct DispIoBonusAndObj; // 10
struct DispIoDispelCheck; // 11
struct DispIoD20ActionTurnBased; // 12
struct DispIOBonusListAndSpellEntry; // 14
struct D20Actn;

struct SpellEntry;


struct TurnBasedStatus;
struct BonusList;
struct CondNode;

struct SubDispDef;
struct CondStruct;
struct DispatcherCallbackArgs;
struct Dispatcher;


struct DispatcherSystem : AddressTable
{
	Dispatcher* DispatcherInit(objHndl objHnd);
	bool dispatcherValid(Dispatcher * dispatcher);
	void DispatcherProcessor(Dispatcher * dispatcher, enum_disp_type dispType, uint32_t dispKey, DispIO * dispIO);
	void  DispatcherClearField(Dispatcher * dispatcher, CondNode ** dispCondList);
	void  DispatcherClearPermanentMods(Dispatcher * dispatcher);
	void  DispatcherClearItemConds(Dispatcher * dispatcher);
	void  DispatcherClearConds(Dispatcher *dispatcher);
	
	int32_t dispatch1ESkillLevel(objHndl objHnd, SkillEnum skill, BonusList * bonOut, objHndl objHnd2, int32_t flag);
	float Dispatch29hGetMoveSpeed(objHndl objHnd, void *);
	void dispIOTurnBasedStatusInit(DispIOTurnBasedStatus* dispIOtbStat);
	void dispatchTurnBasedStatusInit(objHndl objHnd, DispIOTurnBasedStatus* dispIOtB);

#pragma region event object checkers

	DispIoCondStruct* DispIoCheckIoType1(DispIoCondStruct* dispIo); // used for ConditionAdd (3); 1,2 and 4 dispatch will null eventObjs
	DispIoBonusList* DispIoCheckIoType2(DispIoBonusList* dispIoBonusList); // used for stat level (10), stat base (66) and Cur/Max HP
	DispIoSavingThrow* DispIoCheckIoType3(DispIoSavingThrow* dispIoBonusList); // 
	DispIoDamage * DispIoCheckIoType4(DispIoDamage* dispIo);
	DispIoAttackBonus * DispIoCheckIoType5(DispIoAttackBonus* dispIo);
	DispIoD20Signal* DispIoCheckIoType6(DispIoD20Signal* dispIo);
	DispIoD20Query* DispIoCheckIoType7(DispIoD20Query* dispIo);
	DispIOTurnBasedStatus* DispIoCheckIoType8(DispIOTurnBasedStatus* dispIo);
	DispIoTooltip* DispIoCheckIoType9(DispIoTooltip* dispIo);
	DispIoBonusAndObj* DispIoCheckIoType10(DispIoBonusAndObj* dispIo);
	DispIoDispelCheck* DispIOCheckIoType11(DispIoDispelCheck* dispIo);
	DispIoD20ActionTurnBased* DispIOCheckIoType12(DispIoD20ActionTurnBased* dispIo);
	DispIOBonusListAndSpellEntry* DispIOCheckIoType14(DispIOBonusListAndSpellEntry* dispIo);
	void PackDispatcherIntoObjFields(objHndl objHnd, Dispatcher* dispatcher);
	int DispatchAttackBonus(objHndl objHnd, objHndl victim, DispIoAttackBonus* dispIo, enum_disp_type dispType, int key);
	int DispatchToHitBonusBase(objHndl objHndCaller, DispIoAttackBonus* dispIo);
	int DispatchGetSizeCategory(objHndl objHndCaller);
#pragma endregion

	uint32_t(__cdecl * dispatcherForCritters)(objHndl, DispIO *, enum_disp_type, uint32_t dispKey);
	void DispIoDamageInit(DispIoDamage *dispIoDamage);
	int32_t DispatchDamage(objHndl objHnd, DispIoDamage* dispIoDamage, enum_disp_type enumDispType, D20DispatcherKey d20DispatcherKey);
	int DispatchD20ActionCheck(D20Actn* d20Actn, TurnBasedStatus* turnBasedStatus, enum_disp_type dispType);
	DispatcherSystem()
	{
		rebase(_Dispatch29hMovementSthg,0x1004D080); 
		rebase(dispatcherForCritters, 0x1004DD00);
	
	};
private:
	void(__cdecl *_Dispatch29hMovementSthg)(objHndl objHnd, void *);
};

extern DispatcherSystem dispatch;

#pragma pack(push, 1)

#pragma region Dispatcher Structs

struct DispIO {
	enum_dispIO_type dispIOType;
};

struct CondNode : TempleAlloc {
	CondStruct* condStruct;
	CondNode* nextCondNode;
	uint32_t flags; // 1 - expired; 2 - got arg data from info stored in field
	uint32_t args[6];

	explicit CondNode(CondStruct *cond);
};

struct SubDispNode : TempleAlloc {
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
	uint32_t data1;
	uint32_t data2;
};

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

struct CondStructNew
{
	const char* condName;
	unsigned int numArgs;
	SubDispDefNew subDispDefs[100];
};

struct DispatcherCallbackArgs {
	SubDispNode* subDispNode;
	objHndl objHndCaller;
	enum_disp_type dispType;
	uint32_t dispKey;
	DispIO* dispIO;
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
	BonusList* bonlist;
	uint32_t flags;
};


struct DispIoSavingThrow : DispIO { // DispIoType = 3
	uint32_t returVal;
	objHndl obj;
	uint32_t flags;
	int field_14;
	BonusList bonlist;
	int rollResult;
};

struct DispIoAttackBonus : DispIO { // DispIoType 5
	int field_4;
	AttackPacket attackPacket;
	BonusList bonlist;
	DispIoAttackBonus() {
		dispIOType = dispIOTypeAttackBonus;
	}
};


struct DispIoD20Signal : DispIO // DispIoType 6
{
	uint32_t return_val;
	uint32_t data1;
	uint32_t data2;

	DispIoD20Signal()
	{
		dispIOType = dispIoTypeSendSignal;
		return_val = 0;
		data1 = 0;
		data2 = 0;
	}
};


struct DispIoD20Query : DispIO // DispIoType 7
{
	uint32_t return_val;
	uint32_t data1;
	uint32_t data2;

	DispIoD20Query()
	{
		dispIOType = dispIOTypeQuery;
		return_val = 0;
		data1 = 0;
		data2 = 0;
	}
};

struct DispIoTooltip : DispIO // DispIoType 9 ; tooltip additional text when hovering over an object in the game
{
	char strings[10][256];
	uint32_t numStrings;
};
const auto TestSizeOfDispIoTooltip = sizeof(DispIoTooltip); // should be 2568  (0xA08)

struct DispIoType21 : DispIO { // DispIoType 21
	uint32_t interrupt;
	uint32_t field_8;
	uint32_t field_C;
	uint32_t SDDKey1;
	uint32_t val2;
	uint32_t okToAdd; // or spellId???
	CondNode* condNode;

	DispIoType21() {
		dispIOType = dispIoTypeNull;
		condNode = nullptr;
		SDDKey1 = 0;
		val2 = 0;
		interrupt = 0;
	}
};

struct DispIoBonusAndObj : DispIO // type 10
{
	uint32_t returnVal;
	BonusList * bonOut;
	uint32_t pad;
	objHndl obj; //optional
	BonusList bonlist;
};

struct DispIOTurnBasedStatus : DispIO // type 8
{
	TurnBasedStatus * tbStatus;
};

const int TestSizeOfDispIO390h = sizeof(DispIoBonusAndObj); // should be 912 (0x390)

struct DispIOBonusListAndSpellEntry: DispIO { // Type 14
	BonusList * bonList;
	SpellEntry * spellEntry;
	uint32_t field_C; // unused?
};

struct DispIoDispelCheck : DispIO // type 11
{
	uint32_t spellId; // of the Dispel Spell (Break Enchantment, Dispel Magic etc.)
	uint32_t flags;  // 0x80 - Dispel Magic   0x40 - Break Enchantment  0x20 - slippery mind 0x10 - 0x2 DispelAlignment stuff
	uint32_t returnVal;
};

struct DispIoD20ActionTurnBased : DispIO{ // dispIoType = 12; matches dispTypes 36, 37, 
	int returnVal;
	D20Actn * d20a;
	TurnBasedStatus * tbStatus;
};

struct Dispatcher :TempleAlloc {
	objHndl objHnd;
	CondNode* permanentMods;
	CondNode* itemConds;
	CondNode* conditions;
	SubDispNode* subDispNodes[dispTypeCount];
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
DispIoBonusAndObj * _DispIoCheckIoType10(DispIoBonusAndObj *dispIo);
DispIoDispelCheck * _DispIoCheckIoType11(DispIoDispelCheck* dispIo);
DispIoD20ActionTurnBased * _DispIoCheckIoType12(DispIoD20ActionTurnBased* dispIo);
DispIOBonusListAndSpellEntry * __cdecl _DispIoCheckIoType14(DispIOBonusListAndSpellEntry *dispIO);
void __cdecl _PackDispatcherIntoObjFields(objHndl objHnd, Dispatcher*dispatcher);

void DispIOType21Init(DispIoType21* dispIO);

void _dispatchTurnBasedStatusInit(objHndl objHnd, DispIOTurnBasedStatus* dispIOtB);
uint32_t _Dispatch62(objHndl, DispIO*, uint32_t dispKey);
uint32_t _Dispatch63(objHndl objHnd, DispIO* dispIO);

void _DispatcherProcessor(Dispatcher* dispatcher, enum_disp_type dispType, uint32_t dispKey, DispIO * dispIO);
int32_t _DispatchDamage(objHndl objHnd, DispIoDamage* dispIo, enum_disp_type dispType, D20DispatcherKey key);
int32_t _dispatch1ESkillLevel(objHndl objHnd, SkillEnum skill, BonusList* bonOut, objHndl objHnd2, int32_t flag);



#pragma endregion