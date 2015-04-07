#include "stdafx.h"
#include "dispatcher.h"
#include "d20.h"
#include "temple_functions.h"
#include "obj.h"
#include "condition.h"


void __cdecl DispatcherRemoveSubDispNodes(Dispatcher * dispatcher, CondNode * cond)
{
	for (uint32_t i = 0; i < dispTypeCount; i++)
	{
		SubDispNode ** ppSubDispNode = &dispatcher->subDispNodes[i];
		while (*ppSubDispNode != nullptr)
		{
			if ((*ppSubDispNode)->condNode == cond)
			{
				SubDispNode * savedNext = (*ppSubDispNode)->next;
				allocFuncs.free(*ppSubDispNode);
				*ppSubDispNode = savedNext;

			}
			else
			{
				ppSubDispNode = &((*ppSubDispNode)->next);
			}

		}
	}

	};


void __cdecl DispatcherClearField(Dispatcher *dispatcher, CondNode ** dispCondList)
{
	CondNode * cond = *dispCondList;
	objHndl obj = dispatcher->objHnd;
	while (cond != nullptr)
	{
		SubDispNode * subDispNode_TypeRemoveCond = dispatcher->subDispNodes[2];
		CondNode * nextCond = cond->nextCondNode;

		while (subDispNode_TypeRemoveCond != nullptr)
		{

			SubDispDef * sdd = subDispNode_TypeRemoveCond->subDispDef;
			if (sdd->dispKey == 0 && (subDispNode_TypeRemoveCond->condNode->flags & 1) == 0
				&& subDispNode_TypeRemoveCond->condNode == cond)
			{
				sdd->dispCallback(subDispNode_TypeRemoveCond, obj, dispTypeConditionRemove, 0, nullptr);
			}
			subDispNode_TypeRemoveCond = subDispNode_TypeRemoveCond->next;
		}
		DispatcherRemoveSubDispNodes(dispatcher, cond);
		allocFuncs.free(cond);
		cond = nextCond;

	}
	*dispCondList = nullptr;
};

void __cdecl DispatcherClearAttribs(Dispatcher *dispatcher)
{
	DispatcherClearField(dispatcher, &dispatcher->attributeConds);
};

void __cdecl DispatcherClearItemConds(Dispatcher *dispatcher)
{
	DispatcherClearField(dispatcher, &dispatcher->itemConds);
};

void __cdecl DispatcherClearConds(Dispatcher *dispatcher)
{
	DispatcherClearField(dispatcher, &dispatcher->otherConds);
};

DispIO14h * DispIO14hCheckDispIOType1(DispIO14h * dispIO)
{
	if (dispIO->dispIOType == 1){ return dispIO; }
	else { return nullptr; }
};

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
			DispIO_Size32_Type21_Init((DispIO20h*)&dispIO20h);
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
			}
			else {
				subDispNode->subDispDef->dispCallback(subDispNode, dispatcher->objHnd, dispType, dispKey, (DispIO*)dispIO);
			}


		}

		subDispNode = subDispNode->next;
	}

	dispCounter--;
	return;

};

Dispatcher* DispatcherInit(objHndl objHnd) {
	Dispatcher* dispatcherNew = (Dispatcher *)allocFuncs._malloc_0(sizeof(Dispatcher));
	memset(&dispatcherNew->subDispNodes, 0, dispTypeCount * sizeof(SubDispNode*));
	CondNode* condNode = *(conds.pCondNodeGlobal);
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


uint32_t Dispatch62(objHndl objHnd, DispIO* dispIO, uint32_t dispKey) {
	Dispatcher* dispatcher = (Dispatcher *)objects.GetDispatcher(objHnd);
	if (dispatcher != nullptr && (int32_t)dispatcher != -1) {
		DispatcherProcessor(dispatcher, dispType62, dispKey, dispIO);
	}
	return 0;
}


uint32_t Dispatch63(objHndl objHnd, DispIO* dispIO) {
	Dispatcher* dispatcher = (Dispatcher *)objects.GetDispatcher(objHnd);
	if (dispatcher != nullptr && (int32_t)dispatcher != -1) {
		DispatcherProcessor(dispatcher, dispType63, 0, dispIO);
	}
	return 0;
}