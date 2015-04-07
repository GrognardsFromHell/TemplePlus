#include "common.h"
#include "dispatcher.h"

uint32_t ConditionAddDispatch(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
void CondNodeAddToSubDispNodeArray(Dispatcher* dispatcher, CondNode* condNode);
uint32_t ConditionAddToAttribs_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct);
uint32_t ConditionAddToAttribs_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2);
uint32_t ConditionAdd_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct);
uint32_t ConditionAdd_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2);
uint32_t ConditionAdd_NumArgs3(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3);
uint32_t ConditionAdd_NumArgs4(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);


uint32_t ConditionPrevent(DispatcherCallbackArgs args);