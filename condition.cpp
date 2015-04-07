#include "stdafx.h"
#include "common.h"
#include "dispatcher.h"
#include "condition.h"


CondNode::CondNode(CondStruct *cond) {
	memset(this, 0, sizeof(CondNode));
	condStruct = cond;
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
		}
		else {
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