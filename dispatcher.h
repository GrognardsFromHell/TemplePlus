#pragma once

#include "common.h"
#include "skill.h"

#define DISPATCHER_MAX  250 // max num of simultaneous Dispatches going on (static int counter inside _DispatcherProcessor)

struct DispIoBonusList;
struct DispIoCondStruct;
struct DispIoDispelCheck;
struct DispIoD20ActionTurnBased;
struct D20Actn;
struct DispIOBonusListAndSpellEntry;
struct SpellEntry;
struct DispIOTurnBasedStatus;
struct DispIoDamage;
struct TurnBasedStatus;
struct BonusList;
struct CondNode;
struct DispIO;
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
	void  DispatcherClearAttribs(Dispatcher * dispatcher);
	void  DispatcherClearItemConds(Dispatcher * dispatcher);
	void  DispatcherClearConds(Dispatcher *dispatcher);
	
	int32_t dispatch1ESkillLevel(objHndl objHnd, SkillEnum skill, BonusList * bonOut, objHndl objHnd2, int32_t flag);
	float Dispatch29hGetMoveSpeed(objHndl objHnd, void *);
	void dispIOTurnBasedStatusInit(DispIOTurnBasedStatus* dispIOtbStat);
	void dispatchTurnBasedStatusInit(objHndl objHnd, DispIOTurnBasedStatus* dispIOtB);

#pragma region event object checkers

	DispIoCondStruct* DispIoCheckIoType1(DispIoCondStruct* dispIo); // used for ConditionAdd (3); 1,2 and 4 dispatch will null eventObjs
	DispIoBonusList* DispIoCheckIoType2(DispIoBonusList* dispIoBonusList); // used for stat level (10), stat base (66) and Cur/Max HP
	DispIoDamage * DispIoCheckIoType4(DispIoDamage* dispIo);
	DispIOTurnBasedStatus* DispIoCheckIoType8(DispIOTurnBasedStatus* dispIo);
	DispIoDispelCheck* DispIOCheckIoType11(DispIoDispelCheck* dispIo);
	DispIoD20ActionTurnBased* DispIOCheckIoType12(DispIoD20ActionTurnBased* dispIo);
	DispIOBonusListAndSpellEntry* DispIOCheckIoType14(DispIOBonusListAndSpellEntry* dispIo);

#pragma endregion

	uint32_t(__cdecl * dispatcherForCritters)(objHndl, DispIO *, enum_disp_type, uint32_t dispKey);
	void DispIoDamageInit(DispIoDamage *dispIoDamage);
	int32_t DispatchDamage(objHndl objHnd, DispIoDamage* dispIoDamage, enum_disp_type enumDispType, D20DispatcherKey d20DispatcherKey);
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
	uint32_t flags;
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

/*
	This defines a condition and what it does, while CondNode represents an instance of this or another condition.
*/
struct CondStruct {
	char* condName;
	unsigned int numArgs;
	/*
		This is a variable length array of dispatcher hooks that this condition has.
	*/
	SubDispDef subDispDefs[1];
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

struct DispIOAC : DispIO {
	DispIOAC() {
		dispIOType = dispIOTypeAC;
	}
};

struct DispIoD20Query : DispIO
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



struct DispIO20h : DispIO {
	uint32_t interrupt;
	uint32_t field_8;
	uint32_t field_C;
	uint32_t val1;
	uint32_t val2;
	uint32_t okToAdd;
	CondNode* condNode;

	DispIO20h() {
		dispIOType = dispIoTypeNull;
		condNode = nullptr;
		val1 = 0;
		val2 = 0;
		interrupt = 0;
	}
};

struct DispIO390h : DispIO
{
	uint32_t returnVal;
	BonusList * bonOut;
	uint32_t pad;
	objHndl obj; //optional
	BonusList bonlist;
};

struct DispIOTurnBasedStatus : DispIO
{
	TurnBasedStatus * tbStatus;
};

const int TestSizeOfDispIO390h = sizeof(DispIO390h); // should be 912 (0x390)

struct DispIOBonusListAndSpellEntry: DispIO{
	BonusList * bonList;
	SpellEntry * spellEntry;
	uint32_t field_C; // unused?
};

struct DispIoDispelCheck : DispIO
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
	CondNode* attributeConds;
	CondNode* itemConds;
	CondNode* otherConds;
	SubDispNode* subDispNodes[dispTypeCount];
};

#pragma endregion

#pragma pack(pop)

#pragma region Dispatcher Functions

Dispatcher* _DispatcherInit(objHndl objHnd);

void  _DispatcherRemoveSubDispNodes(Dispatcher * dispatcher, CondNode * cond);
void  _DispatcherClearField(Dispatcher *dispatcher, CondNode ** dispCondList);
void  _DispatcherClearAttribs(Dispatcher *dispatcher);
void  _DispatcherClearItemConds(Dispatcher *dispatcher);
void  _DispatcherClearConds(Dispatcher *dispatcher);

DispIoCondStruct * _DispIoCheckIoType1(DispIoCondStruct * dispIo);
DispIoBonusList * _DispIoCheckIoType2(DispIoBonusList * dispIo);
DispIoDamage* _DispIOCheckIoType4(DispIoDamage* dispIo);
DispIOTurnBasedStatus * _DispIOCheckIoType8(DispIOTurnBasedStatus* dispIo);
DispIoDispelCheck * _DispIOCheckIoType11(DispIoDispelCheck* dispIo);
DispIoD20ActionTurnBased * _DispIoCheckIoType12(DispIoD20ActionTurnBased* dispIo);
DispIOBonusListAndSpellEntry * __cdecl _DispIOCheckIoType14(DispIOBonusListAndSpellEntry *dispIO);

void _DispIO_Size32_Type21_Init(DispIO20h* dispIO);

void _dispatchTurnBasedStatusInit(objHndl objHnd, DispIOTurnBasedStatus* dispIOtB);
uint32_t _Dispatch62(objHndl, DispIO*, uint32_t dispKey);
uint32_t _Dispatch63(objHndl objHnd, DispIO* dispIO);

void _DispatcherProcessor(Dispatcher* dispatcher, enum_disp_type dispType, uint32_t dispKey, DispIO * dispIO);
int32_t _DispatchDamage(objHndl objHnd, DispIoDamage* dispIo, enum_disp_type dispType, D20DispatcherKey key);
int32_t _dispatch1ESkillLevel(objHndl objHnd, SkillEnum skill, BonusList* bonOut, objHndl objHnd2, int32_t flag);



#pragma endregion