#include "stdafx.h"
#include "obj.h"
#include "addresses.h"
#include "fixes.h"
#include "d20.h"

const uint32_t DISPATCHER_MAX = 250; // max num of simultaneous Dispatches going on (static int counter inside DispatcherProcessor)

Objects objects;

static_assert(validate_size<CondNode, 36>::value, "Condition node structure has incorrect size.");

#pragma region Object Internals
const size_t objHeaderSize = 4; // Constant
const size_t objBodySize = 168; // Passed in to Object_Tables_Init
const size_t objSize = objHeaderSize + objBodySize;
static_assert(validate_size<GameObject, objSize>::value, "Object structure has incorrect size.");



class ObjectDispatch : public TempleFix {
public:
	const char* name() override {
		return "Object";
	}

	void apply() override {
		replaceFunction(0x100E1F10, DispatcherInit);
		replaceFunction(0x1004DBA0, DispIO_Size32_Type21_Init);
		replaceFunction(0x1004D3A0, Dispatch62);
		replaceFunction(0x1004D440, Dispatch63);


		replaceFunction(0x100E2120, DispatcherProcessor);

		replaceFunction(0x100E22D0, ConditionAddDispatch);
		replaceFunction(0x100E24C0, ConditionAddToAttribs_NumArgs0);
		replaceFunction(0x100E2500, ConditionAddToAttribs_NumArgs2);
		replaceFunction(0x100E24E0, ConditionAdd_NumArgs0);
		replaceFunction(0x100E2530, ConditionAdd_NumArgs2);
		replaceFunction(0x100E2560, ConditionAdd_NumArgs3);
		replaceFunction(0x100E2590, ConditionAdd_NumArgs4);
	}
};







// Root hashtable for all objects
struct ObjectMasterTableRow {
	GameObject objects[0x2000];
};

struct ObjectMasterTable {
	ObjectMasterTableRow* rows[256];
};

const uint32_t ObjFieldDefCount = 430;

struct ObjFieldDef {
	int protoPropIndex;
	int field4;
	int bitmapIndex1;
	uint32_t bitmapMask;
	int bitmapIndex2;
	uint32_t IsStoredInPropCollection;
	uint32_t FieldTypeCode;
};

static struct ObjInternal : AddressTable {
	ObjFieldDef** fieldDefs;

	ObjectMasterTable** masterTable;

	// Starts at 1, Max is 256
	int* masterTableSize;

	int *poolSize;

	ObjInternal() {
		rebase(fieldDefs, 0x10B3D7D8);
		rebase(masterTable, 0x10BCAC50);
		rebase(masterTableSize, 0x10BCAC4C);
		rebase(poolSize, 0x10BCAC30);
	}
} objInternal;
#pragma endregion

void DispatcherProcessor(Dispatcher* dispatcher, enum_disp_type dispType, uint32_t dispKey, DispIO* dispIO) {
	static uint32_t dispCounter = 0;
	if (dispCounter > DISPATCHER_MAX) {
		return;
	}
	dispCounter++;

	SubDispNode* subDispNode = dispatcher->subDispNodes[dispType];

	while (subDispNode != nullptr) {

		if ((subDispNode->subDispDef->dispKey == dispKey || subDispNode->subDispDef->dispKey == 0) && ((subDispNode->condNode->flags & 1) == 0)) {

			DispIO20h dispIO20h;
			DispIO_Size32_Type21_Init((DispIO20h*) &dispIO20h);
			dispIO20h.condNode = (CondNode *)subDispNode->condNode;

			if (dispKey != 10 || dispType != 62) {
				Dispatch62(dispatcher->objHnd, (DispIO*)&dispIO20h, 10);
			}

			if (dispIO20h.interrupt == 1 && dispType != dispType63) {
				dispIO20h.interrupt = 0;
				dispIO20h.val2 = 10;
				Dispatch63(dispatcher->objHnd, (DispIO*)&dispIO20h);
				if (dispIO20h.interrupt == 0) {
					subDispNode->subDispDef->dispCallback(subDispNode, dispatcher->objHnd, dispType, dispKey, (DispIO*)dispIO);
				}
			} else {
				subDispNode->subDispDef->dispCallback(subDispNode, dispatcher->objHnd, dispType, dispKey, (DispIO*)dispIO);
			}


		}

		subDispNode = subDispNode->next;
	}

	dispCounter--;
	return;

};

CondNode::CondNode(CondStruct *cond) {
	memset(this, 0, sizeof(CondNode));
	condStruct = cond;
}

Objects::Objects() {
	rebase(_GetInternalFieldInt32, 0x1009E1D0);
	rebase(_GetInternalFieldInt64, 0x1009E2E0);
	rebase(_StatLevelGet, 0x10074800);
	rebase(_SetInternalFieldInt32, 0x1010B4A0);
}

Dispatcher* DispatcherInit(objHndl objHnd) {
	Dispatcher* dispatcherNew = (Dispatcher *)allocFuncs._malloc_0(sizeof(Dispatcher));
	memset(&dispatcherNew->subDispNodes, 0, dispTypeCount * sizeof(SubDispNode*));
	CondNode* condNode = pCondNodeGlobal;
	while (condNode != nullptr) {
		CondNodeAddToSubDispNodeArray(dispatcherNew, condNode);
		condNode = condNode->nextCondNode;
	}
	dispatcherNew->objHnd = objHnd;
	dispatcherNew->attributeConds = nullptr;
	dispatcherNew->itemConds = nullptr;
	dispatcherNew->otherConds = nullptr;
	return dispatcherNew;
};

void DispIO_Size32_Type21_Init(DispIO20h* dispIO) {
	dispIO->dispIOType = dispIOType21;
	dispIO->interrupt = 0;
	dispIO->field_8 = 0;
	dispIO->field_C = 0;
	dispIO->val1 = 0;
	dispIO->val2 = 0;
	dispIO->okToAdd = 0;
	dispIO->condNode = nullptr;
};




ObjectDispatch objectDispatch;


uint32_t Dispatch62(objHndl objHnd, DispIO* dispIO, uint32_t dispKey) {
	Dispatcher* dispatcher = (Dispatcher *)templeFuncs.Obj_Get_Field_32bit(objHnd, obj_f_dispatcher);
	if (dispatcher != nullptr && (int32_t)dispatcher != -1) {
		DispatcherProcessor(dispatcher, dispType62, dispKey, dispIO);
	}
	return 0;
}


uint32_t Dispatch63(objHndl objHnd, DispIO* dispIO) {
	Dispatcher* dispatcher = (Dispatcher *)templeFuncs.Obj_Get_Field_32bit(objHnd, obj_f_dispatcher);
	if (dispatcher != nullptr && (int32_t)dispatcher != -1) {
		DispatcherProcessor(dispatcher, dispType63, 0, dispIO);
	}
	return 0;
}

uint32_t ConditionAddDispatch(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
	
	assert(condStruct->numArgs >= 0 && condStruct->numArgs <= 6);



	// pre-add section (may abort adding condition, or cause another condition to be deleted first)
	DispIO14h dispIO14h;
	dispIO14h.dispIOType = dispIOType1;
	dispIO14h.condStruct = condStruct;
	dispIO14h.outputFlag = 1;
	dispIO14h.arg1 = arg1;
	dispIO14h.arg2 = arg2;

	DispatcherProcessor(dispatcher, dispTypeConditionAddPre, 0, (DispIO*)&dispIO14h);

	if (dispIO14h.outputFlag == 0) {
		return 0;
	}

	// adding condition
	auto condNodeNew = new CondNode(condStruct);
	auto numArgs = condStruct->numArgs;
	if (numArgs >= 1) {
		condNodeNew->args[0] = arg1;
	}
	if (numArgs >= 2) {
		condNodeNew->args[1] = arg2;
	}
	if (numArgs >= 3) {
		condNodeNew->args[2] = arg3;
	}
	if (numArgs >= 4) {
		condNodeNew->args[3] = arg4;
	}
	
	CondNode** ppNextCondeNode = ppCondNode;

	while (*ppNextCondeNode != nullptr) {
		ppNextCondeNode = &(*ppNextCondeNode)->nextCondNode;
	}
	*ppNextCondeNode = condNodeNew;

	CondNodeAddToSubDispNodeArray(dispatcher, condNodeNew);


	auto dispatcherSubDispNodeType1 = dispatcher->subDispNodes[1];
	while (dispatcherSubDispNodeType1 != nullptr) {
		if (dispatcherSubDispNodeType1->subDispDef->dispKey == 0
			&& (dispatcherSubDispNodeType1->condNode->flags & 1) == 0
			&& condNodeNew == dispatcherSubDispNodeType1->condNode) {
			dispatcherSubDispNodeType1->subDispDef->dispCallback(dispatcherSubDispNodeType1, dispatcher->objHnd, dispTypeConditionAdd, 0, nullptr);
		}

		dispatcherSubDispNodeType1 = dispatcherSubDispNodeType1->next;
	}

	return 1;
};

void CondNodeAddToSubDispNodeArray(Dispatcher* dispatcher, CondNode* condNode) {
	auto subDispDef = &(condNode->condStruct->subDispDefs);

	while (subDispDef->dispType != 0) {
		auto subDispNodeNew = (SubDispNode *)allocFuncs._malloc_0(sizeof(SubDispNode));
		subDispNodeNew->subDispDef = subDispDef;
		subDispNodeNew->next = nullptr;
		subDispNodeNew->condNode = condNode;


		auto dispType = subDispDef->dispType;
		assert(dispType >= 0 && dispType < dispTypeCount);

		auto ppDispatcherSubDispNode = &(dispatcher->subDispNodes[dispType]);

		if (*ppDispatcherSubDispNode != nullptr) {
			while ((*ppDispatcherSubDispNode)->next != nullptr) {
				ppDispatcherSubDispNode = &((*ppDispatcherSubDispNode)->next);
			}
			(*ppDispatcherSubDispNode)->next = subDispNodeNew;
		} else {
			dispatcher->subDispNodes[subDispDef->dispType] = subDispNodeNew;
		}


		subDispDef += 1;
	}

};


uint32_t ConditionAddToAttribs_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct) {
	return ConditionAddDispatch(dispatcher, &dispatcher->attributeConds, condStruct, 0, 0, 0, 0);
};

uint32_t ConditionAddToAttribs_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2) {
	return ConditionAddDispatch(dispatcher, &dispatcher->attributeConds, condStruct, arg1, arg2, 0, 0);
};

uint32_t ConditionAdd_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct) {
	return ConditionAddDispatch(dispatcher, &dispatcher->otherConds, condStruct, 0, 0, 0, 0);
};

uint32_t ConditionAdd_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2) {
	return ConditionAddDispatch(dispatcher, &dispatcher->otherConds, condStruct, arg1, arg2, 0, 0);
};

uint32_t ConditionAdd_NumArgs3(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
	return ConditionAddDispatch(dispatcher, &dispatcher->otherConds, condStruct, arg1, arg2, arg3, 0);
};

uint32_t ConditionAdd_NumArgs4(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
	return ConditionAddDispatch(dispatcher, &dispatcher->otherConds, condStruct, arg1, arg2, arg3, arg4);
};
