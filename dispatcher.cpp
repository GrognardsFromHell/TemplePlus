#include "stdafx.h"
#include "dispatcher.h"
#include "d20.h"
#include "temple_functions.h"
#include "obj.h"
#include "condition.h"
#include "bonus.h"
#include "action_sequence.h"
#include "turn_based.h"

class DispatcherReplacements : public TempleFix {
public:
	const char* name() override {
		return "Dispatcher System Function Replacements";
	}

	void apply() override {
		logger->info("Replacing basic Dispatcher functions");
		replaceFunction(0x1004D700, _DispIO14hCheckDispIOType1);
		replaceFunction(0x100E1E30, _DispatcherRemoveSubDispNodes);
		replaceFunction(0x100E2400, _DispatcherClearField);
		replaceFunction(0x100E2720, _DispatcherClearAttribs);
		replaceFunction(0x100E2740, _DispatcherClearItemConds);
		replaceFunction(0x100E2760, _DispatcherClearConds);
		replaceFunction(0x100E2120, _DispatcherProcessor);
		replaceFunction(0x100E1F10, _DispatcherInit);
		replaceFunction(0x1004DBA0, _DispIO_Size32_Type21_Init);
		replaceFunction(0x1004D3A0, _Dispatch62);
		replaceFunction(0x1004D440, _Dispatch63);
		macReplaceFun(1004E790,_dispatchTurnBasedStatusInit)
		macReplaceFun(1004ED70, _dispatch1ESkillLevel)
	}
} dispatcherReplacements;


#pragma region Dispatcher System Implementation
DispatcherSystem dispatch;

void DispatcherSystem::DispatcherProcessor(Dispatcher* dispatcher, enum_disp_type dispType, uint32_t dispKey, DispIO* dispIO)
{
	_DispatcherProcessor(dispatcher, dispType, dispKey, dispIO);
}

Dispatcher * DispatcherSystem::DispatcherInit(objHndl objHnd)
{
	return _DispatcherInit(objHnd);
}

bool DispatcherSystem::dispatcherValid(Dispatcher* dispatcher)
{
	return (dispatcher != nullptr && dispatcher != (Dispatcher*)-1);
}

void  DispatcherSystem::DispatcherClearField(Dispatcher * dispatcher, CondNode ** dispCondList)
{
	_DispatcherClearField(dispatcher, dispCondList);
}

void  DispatcherSystem::DispatcherClearAttribs(Dispatcher * dispatcher)
{
	_DispatcherClearAttribs(dispatcher);
}

void  DispatcherSystem::DispatcherClearItemConds(Dispatcher * dispatcher)
{
	_DispatcherClearItemConds(dispatcher);
}

void  DispatcherSystem::DispatcherClearConds(Dispatcher *dispatcher)
{
	_DispatcherClearConds(dispatcher);
}

int32_t DispatcherSystem::dispatch1ESkillLevel(objHndl objHnd, SkillEnum skill, BonusList* bonOut, objHndl objHnd2, int32_t flag)
{
	DispIO390h dispIO;
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (!dispatcherValid(dispatcher)) return 0;

	dispIO.dispIOType = dispIOType10;
	dispIO.returnVal = flag;
	dispIO.bonOut = bonOut;
	dispIO.obj = objHnd2;
	if (!bonOut)
	{
		dispIO.bonOut = &dispIO.bonlist;
		bonusSys.initBonusList(&dispIO.bonlist);
	}
	DispatcherProcessor(dispatcher, dispTypeSkillLevel, skill + 20, &dispIO);
	return bonusSys.getOverallBonus(dispIO.bonOut);
	
}

float DispatcherSystem::Dispatch29hGetMoveSpeed(objHndl objHnd, void* iO) // including modifiers like armor restirction
{
	float result = 30.0;
	uint32_t objHndLo = (uint32_t)(objHnd & 0xffffFFFF);
	uint32_t objHndHi = (uint32_t)((objHnd >>32) & 0xffffFFFF);
	macAsmProl;
	__asm{
		mov ecx, this;
		mov esi, [ecx]._Dispatch29hMovementSthg;
		mov eax, iO;
		push eax;
		mov eax, objHndHi;
		push eax;
		mov eax, objHndLo;
		push eax;
		call esi;
		add esp, 12;
		fstp result;
	}
	macAsmEpil
	return result;
}

void DispatcherSystem::dispIOTurnBasedStatusInit(DispIOTurnBasedStatus* dispIOtbStat)
{
	dispIOtbStat->dispIOType = dispIOTypeTurnBasedStatus;
	dispIOtbStat->tbStatus = nullptr;
}


void DispatcherSystem::dispatchTurnBasedStatusInit(objHndl objHnd, DispIOTurnBasedStatus* dispIOtB)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (dispatcherValid(dispatcher))
	{
		DispatcherProcessor(dispatcher, dispTypeTurnBasedStatusInit, 0, dispIOtB);
		if (dispIOtB->tbStatus)
		{
			if (dispIOtB->tbStatus->hourglassState > 0)
			{
				d20Sys.d20SendSignal(objHnd, DK_SIG_BeginTurn, 0, 0);
			}
		}
	}
}
#pragma endregion





#pragma region Dispatcher Functions

void __cdecl _DispatcherRemoveSubDispNodes(Dispatcher * dispatcher, CondNode * cond)
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


void __cdecl _DispatcherClearField(Dispatcher *dispatcher, CondNode ** dispCondList)
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
		_DispatcherRemoveSubDispNodes(dispatcher, cond);
		allocFuncs.free(cond);
		cond = nextCond;

	}
	*dispCondList = nullptr;
};

void __cdecl _DispatcherClearAttribs(Dispatcher *dispatcher)
{
	_DispatcherClearField(dispatcher, &dispatcher->attributeConds);
};

void __cdecl _DispatcherClearItemConds(Dispatcher *dispatcher)
{
	_DispatcherClearField(dispatcher, &dispatcher->itemConds);
};

void __cdecl _DispatcherClearConds(Dispatcher *dispatcher)
{
	_DispatcherClearField(dispatcher, &dispatcher->otherConds);
};

DispIO14h * _DispIO14hCheckDispIOType1(DispIO14h * dispIO)
{
	if (dispIO->dispIOType == 1){ return dispIO; }
	else { return nullptr; }
};

void _DispatcherProcessor(Dispatcher* dispatcher, enum_disp_type dispType, uint32_t dispKey, DispIO* dispIO) {
	static uint32_t dispCounter = 0;
	if (dispCounter > DISPATCHER_MAX) {
		logger->info("Dispatcher maximum recursion reached!");
		return;
	}
	dispCounter++;

	SubDispNode* subDispNode = dispatcher->subDispNodes[dispType];

	while (subDispNode != nullptr) {

		if ((subDispNode->subDispDef->dispKey == dispKey || subDispNode->subDispDef->dispKey == 0) && ((subDispNode->condNode->flags & 1) == 0)) {

			DispIO20h dispIO20h;
			_DispIO_Size32_Type21_Init((DispIO20h*)&dispIO20h);
			dispIO20h.condNode = (CondNode *)subDispNode->condNode;

			if (dispKey != 10 || dispType != 62) {
				_Dispatch62(dispatcher->objHnd, (DispIO*)&dispIO20h, 10);
			}

			if (dispIO20h.interrupt == 1 && dispType != dispType63) {
				dispIO20h.interrupt = 0;
				dispIO20h.val2 = 10;
				_Dispatch63(dispatcher->objHnd, (DispIO*)&dispIO20h);
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

}

int32_t _dispatch1ESkillLevel(objHndl objHnd, SkillEnum skill, BonusList* bonOut, objHndl objHnd2, int32_t flag)
{
	return dispatch.dispatch1ESkillLevel(objHnd, skill, bonOut, objHnd2, flag);
};

Dispatcher* _DispatcherInit(objHndl objHnd) {
	Dispatcher* dispatcherNew = (Dispatcher *)allocFuncs._malloc_0(sizeof(Dispatcher));
	memset(&dispatcherNew->subDispNodes, 0, dispTypeCount * sizeof(SubDispNode*));
	CondNode* condNode = *(conds.pCondNodeGlobal);
	while (condNode != nullptr) {
		_CondNodeAddToSubDispNodeArray(dispatcherNew, condNode);
		condNode = condNode->nextCondNode;
	}
	dispatcherNew->objHnd = objHnd;
	dispatcherNew->attributeConds = nullptr;
	dispatcherNew->itemConds = nullptr;
	dispatcherNew->otherConds = nullptr;
	return dispatcherNew;
};

void _DispIO_Size32_Type21_Init(DispIO20h* dispIO) {
	dispIO->dispIOType = dispIOType21;
	dispIO->interrupt = 0;
	dispIO->field_8 = 0;
	dispIO->field_C = 0;
	dispIO->val1 = 0;
	dispIO->val2 = 0;
	dispIO->okToAdd = 0;
	dispIO->condNode = nullptr;
}

void _dispatchTurnBasedStatusInit(objHndl objHnd, DispIOTurnBasedStatus* dispIOtB)
{
	dispatch.dispatchTurnBasedStatusInit(objHnd, dispIOtB);
};


uint32_t _Dispatch62(objHndl objHnd, DispIO* dispIO, uint32_t dispKey) {
	Dispatcher* dispatcher = (Dispatcher *)objects.GetDispatcher(objHnd);
	if (dispatcher != nullptr && (int32_t)dispatcher != -1) {
		_DispatcherProcessor(dispatcher, dispType62, dispKey, dispIO);
	}
	return 0;
}


uint32_t _Dispatch63(objHndl objHnd, DispIO* dispIO) {
	Dispatcher* dispatcher = (Dispatcher *)objects.GetDispatcher(objHnd);
	if (dispatcher != nullptr && (int32_t)dispatcher != -1) {
		_DispatcherProcessor(dispatcher, dispType63, 0, dispIO);
	}
	return 0;
}

#pragma endregion 