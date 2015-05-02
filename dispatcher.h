#pragma once

#include "common.h"
#include "skill.h"

#define DISPATCHER_MAX  250 // max num of simultaneous Dispatches going on (static int counter inside _DispatcherProcessor)

struct DispIOBonusListAndSpellEntry;
struct SpellEntry;
struct DispIOTurnBasedStatus;
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
	DispIOBonusListAndSpellEntry* DispIOCheckIoType14(DispIO* dispIo);
	uint32_t(__cdecl * dispatcherForCritters)(objHndl, DispIO *, enum_disp_type, uint32_t dispKey);
	DispatcherSystem()
	{
		macRebase(_Dispatch29hMovementSthg, 1004D080)
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
	int numArgs;
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

struct DispIOAC : DispIO {
	DispIOAC() {
		dispIOType = dispIOTypeAC;
	}
};

struct DispIO10h : DispIO
{
	uint32_t return_val;
	uint32_t data1;
	uint32_t data2;

	DispIO10h()
	{
		dispIOType = dispIOTypeQuery;
		return_val = 0;
		data1 = 0;
		data2 = 0;
	}
};

struct DispIO14h : DispIO {
	CondStruct* condStruct;
	uint32_t outputFlag;
	uint32_t arg1;
	uint32_t arg2;

	DispIO14h() {
		dispIOType = dispIOType0;
		condStruct = nullptr;
		outputFlag = 0;
		arg1 = 0;
		arg2 = 0;
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
		dispIOType = dispIOType0;
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
DispIO14h * _DispIO14hCheckDispIOType1(DispIO14h * dispIO);
void _DispIO_Size32_Type21_Init(DispIO20h* dispIO);

void _dispatchTurnBasedStatusInit(objHndl objHnd, DispIOTurnBasedStatus* dispIOtB);
uint32_t _Dispatch62(objHndl, DispIO*, uint32_t dispKey);
uint32_t _Dispatch63(objHndl objHnd, DispIO* dispIO);

void _DispatcherProcessor(Dispatcher* dispatcher, enum_disp_type dispType, uint32_t dispKey, DispIO * dispIO);
int32_t _dispatch1ESkillLevel(objHndl objHnd, SkillEnum skill, BonusList* bonOut, objHndl objHnd2, int32_t flag);

DispIOBonusListAndSpellEntry * __cdecl _DispIOCheckIoType14(DispIO *dispIO);

#pragma endregion