#include "stdafx.h"
#include "common.h"
#include "dispatcher.h"
#include "condition.h"
#include "temple_functions.h"
#include "obj.h"

ConditionSystem conds;

class ConditionFunctionReplacement : public TempleFix {
public:
	const char* name() override {
		return "Condition Function Replacement";
	}

	void apply() override {
		logger->info("Replacing Condition-related Functions");
		replaceFunction(0x100E22D0, _ConditionAddDispatch);
		replaceFunction(0x100E24C0, _ConditionAddToAttribs_NumArgs0);
		replaceFunction(0x100E2500, _ConditionAddToAttribs_NumArgs2);
		replaceFunction(0x100E24E0, _ConditionAdd_NumArgs0);
		replaceFunction(0x100E2530, _ConditionAdd_NumArgs2);
		replaceFunction(0x100E2560, _ConditionAdd_NumArgs3);
		replaceFunction(0x100E2590, _ConditionAdd_NumArgs4);
		replaceFunction(0x100ECF30, ConditionPrevent);
		replaceFunction(0x100E1DD0, _CondNodeAddToSubDispNodeArray);
		replaceFunction(0x100F7BE0, _GetCondStructFromFeat);
		replaceFunction(0x100E1A80, _GetCondStructFromHashcode);
		replaceFunction(0x100E19C0, _CondStructAddToHashtable);
	}
} condFuncReplacement;


CondNode::CondNode(CondStruct *cond) {
	memset(this, 0, sizeof(CondNode));
	condStruct = cond;
}


#pragma region Condition Add Functions

uint32_t _ConditionAddDispatch(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
	assert(condStruct->numArgs >= 0 && condStruct->numArgs <= 6);

	vector<int> args;
	if (condStruct->numArgs > 0) {
		args.push_back(arg1);
	}
	if (condStruct->numArgs > 1) {
		args.push_back(arg2);
	}
	if (condStruct->numArgs > 2) {
		args.push_back(arg2);
	}
	if (condStruct->numArgs > 3) {
		args.push_back(arg3);
	}
	if (condStruct->numArgs > 4) {
		args.push_back(arg4);
	}

	return _ConditionAddDispatchArgs(dispatcher, ppCondNode, condStruct, args);
};

bool _ConditionAddDispatchArgs(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, const vector<int> &args) {
	assert(condStruct->numArgs >= args.size());

	// pre-add section (may abort adding condition, or cause another condition to be deleted first)
	DispIO14h dispIO14h;
	dispIO14h.dispIOType = dispIOType1;
	dispIO14h.condStruct = condStruct;
	dispIO14h.outputFlag = 1;
	dispIO14h.arg1 = 0;
	dispIO14h.arg2 = 0;
	if (args.size() > 0) {
		dispIO14h.arg1 = args[0];
	}
	if (args.size() > 1) {
		dispIO14h.arg2 = args[1];
	}

	_DispatcherProcessor(dispatcher, dispTypeConditionAddPre, 0, (DispIO*)&dispIO14h);

	if (dispIO14h.outputFlag == 0) {
		return false;
	}

	// adding condition
	auto condNodeNew = new CondNode(condStruct);
	for (int i = 0; i < condStruct->numArgs; ++i) {
		if (i < args.size()) {
			condNodeNew->args[i] = args[i];
		} else {
			// Fill the rest with zeros
			condNodeNew->args[i] = 0;
		}
	}

	CondNode** ppNextCondeNode = ppCondNode;

	while (*ppNextCondeNode != nullptr) {
		ppNextCondeNode = &(*ppNextCondeNode)->nextCondNode;
	}
	*ppNextCondeNode = condNodeNew;

	_CondNodeAddToSubDispNodeArray(dispatcher, condNodeNew);


	auto dispatcherSubDispNodeType1 = dispatcher->subDispNodes[1];
	while (dispatcherSubDispNodeType1 != nullptr) {
		if (dispatcherSubDispNodeType1->subDispDef->dispKey == 0
			&& (dispatcherSubDispNodeType1->condNode->flags & 1) == 0
			&& condNodeNew == dispatcherSubDispNodeType1->condNode) {
			dispatcherSubDispNodeType1->subDispDef->dispCallback(dispatcherSubDispNodeType1, dispatcher->objHnd, dispTypeConditionAdd, 0, nullptr);
		}

		dispatcherSubDispNodeType1 = dispatcherSubDispNodeType1->next;
	}

	return true;
};

void _CondNodeAddToSubDispNodeArray(Dispatcher* dispatcher, CondNode* condNode) {
	auto subDispDef = condNode->condStruct->subDispDefs;

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


uint32_t _ConditionAddToAttribs_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct) {
	return _ConditionAddDispatch(dispatcher, &dispatcher->attributeConds, condStruct, 0, 0, 0, 0);
};

uint32_t _ConditionAddToAttribs_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2) {
	return _ConditionAddDispatch(dispatcher, &dispatcher->attributeConds, condStruct, arg1, arg2, 0, 0);
};

uint32_t _ConditionAdd_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct) {
	return _ConditionAddDispatch(dispatcher, &dispatcher->otherConds, condStruct, 0, 0, 0, 0);
};

uint32_t _ConditionAdd_NumArgs1(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1) {
	return _ConditionAddDispatch(dispatcher, &dispatcher->otherConds, condStruct, arg1, 0, 0, 0);
};

uint32_t _ConditionAdd_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2) {
	return _ConditionAddDispatch(dispatcher, &dispatcher->otherConds, condStruct, arg1, arg2, 0, 0);
};

uint32_t _ConditionAdd_NumArgs3(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
	return _ConditionAddDispatch(dispatcher, &dispatcher->otherConds, condStruct, arg1, arg2, arg3, 0);
};

uint32_t _ConditionAdd_NumArgs4(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
	return _ConditionAddDispatch(dispatcher, &dispatcher->otherConds, condStruct, arg1, arg2, arg3, arg4);
};

#pragma endregion

uint32_t ConditionPrevent(DispatcherCallbackArgs args)
{
	DispIO14h * dispIO = _DispIO14hCheckDispIOType1((DispIO14h*)args.dispIO);
	if (dispIO == nullptr)
	{
		logger->error("Dispatcher Error! Condition {} fuckup, wrong DispIO type", args.subDispNode->condNode->condStruct->condName);
		return 0; // if we get here then VERY BAD!
	}
	if (dispIO->condStruct == (CondStruct *)args.subDispNode->subDispDef->data1)
	{
		dispIO->outputFlag = 0;
	}
	return 0;
};

uint32_t  _GetCondStructFromFeat(feat_enums featEnum, CondStruct ** condStructOut, uint32_t * arg2Out)
{
	feat_enums * featFromDict = & ( conds.FeatConditionDict->featEnum );
	uint32_t iter = 0;
	while (
		( (int32_t)featEnum != featFromDict[0] || featFromDict[1] != -1)
		&&  ( (int32_t)featEnum < (int32_t)featFromDict[0] 
				|| (int32_t)featEnum >= (int32_t)featFromDict[1]  )
		)
	{
		iter += 16;
		featFromDict += 4;
		if (iter >= 0x540){ return 0; }
	}

	*condStructOut = (CondStruct *)*(featFromDict - 1);
	*arg2Out = featEnum + featFromDict[2] - featFromDict[0];
	return 1;
}

uint32_t _CondStructAddToHashtable(CondStruct * condStruct)
{
	return conds.hashmethods.CondStructAddToHashtable(condStruct);
}

CondStruct * _GetCondStructFromHashcode(uint32_t key)
{
	return conds.hashmethods.GetCondStruct(key);
}

CondStruct* ConditionSystem::GetByName(const string& name) {
	auto key = templeFuncs.StringHash(name.c_str());
	return hashmethods.GetCondStruct(key);
}

void ConditionSystem::AddToItem(objHndl item, const CondStruct* cond, const vector<int>& args) {
	assert(args.size() == cond->numArgs);

	auto curCondCount = templeFuncs.Obj_Get_IdxField_NumItems(item, obj_f_item_pad_wielder_condition_array);
	auto curCondArgCount = templeFuncs.Obj_Get_IdxField_NumItems(item, obj_f_item_pad_wielder_argument_array);

	// Add the condition name hash to the list
	auto key = templeFuncs.StringHash(cond->condName);
	templeFuncs.Obj_Set_IdxField_byValue(item, obj_f_item_pad_wielder_condition_array, curCondCount, key);

	auto idx = curCondArgCount;
	for (auto arg : args) {
		templeFuncs.Obj_Set_IdxField_byValue(item, obj_f_item_pad_wielder_argument_array, idx, arg);
		idx++;
	}
}

bool ConditionSystem::AddTo(objHndl handle, const CondStruct* cond, const vector<int>& args) {
	assert(args.size() == cond->numArgs);

	auto dispatcher = objects.GetDispatcher(handle);

	if (!dispatcher) {
		return false;
	}

	return _ConditionAddDispatchArgs(dispatcher, &dispatcher->otherConds, const_cast<CondStruct*>(cond), args) != 0;
}
