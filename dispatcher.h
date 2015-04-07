#pragma once

#include "common.h"

#define DISPATCHER_MAX  250 // max num of simultaneous Dispatches going on (static int counter inside DispatcherProcessor)

struct SubDispDef;
struct CondStruct;
struct DispatcherCallbackArgs;
struct Dispatcher;

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

struct CondStruct {
	char* condName;
	int numArgs;
	SubDispDef subDispDefs;
};

struct DispatcherCallbackArgs {
	SubDispNode* subDispNode;
	objHndl objHndCaller;
	enum_disp_type dispType;
	uint32_t dispKey;
	DispIO* dispIO;
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

struct Dispatcher :TempleAlloc {
	objHndl objHnd;
	CondNode* attributeConds;
	CondNode* itemConds;
	CondNode* otherConds;
	SubDispNode* subDispNodes[dispTypeCount];
};


Dispatcher* DispatcherInit(objHndl objHnd);

void  DispatcherRemoveSubDispNodes(Dispatcher * dispatcher, CondNode * cond);
void  DispatcherClearField(Dispatcher *dispatcher, CondNode ** dispCondList);
void  DispatcherClearAttribs(Dispatcher *dispatcher);
void  DispatcherClearItemConds(Dispatcher *dispatcher);
void  DispatcherClearConds(Dispatcher *dispatcher);
DispIO14h * DispIO14hCheckDispIOType1(DispIO14h * dispIO);
void DispIO_Size32_Type21_Init(DispIO20h* dispIO);

uint32_t Dispatch62(objHndl, DispIO*, uint32_t dispKey);
uint32_t Dispatch63(objHndl objHnd, DispIO* dispIO);

void DispatcherProcessor(Dispatcher* dispatcher, enum_disp_type dispType, uint32_t dispKey, DispIO* dispIO);