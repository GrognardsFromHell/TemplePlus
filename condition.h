#include "common.h"
#include "dispatcher.h"


struct ConditionStructs : AddressTable
{
	CondStruct * ConditionGlobal;
	CondNode ** pCondNodeGlobal;
	CondStruct * ConditionMonsterUndead;
	CondStruct * ConditionSubtypeFire;
	CondStruct * ConditionMonsterOoze;
	CondStruct ** ConditionArrayRace;
	CondStruct ** ConditionArrayClasses;
	CondStruct * ConditionTurnUndead;
	CondStruct * ConditionBardicMusic;
	CondStruct * ConditionSchoolSpecialization;
	CondStruct ** ConditionArrayDomains;
	uint32_t * ConditionArrayDomainsArg1;
	uint32_t * ConditionArrayDomainsArg2;


	ConditionStructs()
	{
		rebase(ConditionGlobal, 0x102E8088);
		rebase(pCondNodeGlobal, 0x10BCADA0);
		rebase(ConditionMonsterUndead, 0x102EF9A8);
		rebase(ConditionSubtypeFire, 0x102EFBE8);
		rebase(ConditionMonsterOoze, 0x102EFAF0);
		rebase(ConditionArrayRace, 0x102EFC18);
		rebase(ConditionArrayClasses, 0x102F0634);
		rebase(ConditionTurnUndead, 0x102B0D48);
		rebase(ConditionBardicMusic, 0x102F0520);
		rebase(ConditionSchoolSpecialization, 0x102F0604);
		rebase(ConditionArrayDomains, 0x102B1690);
		rebase(ConditionArrayDomainsArg1, 0x102B1694);
		rebase(ConditionArrayDomainsArg2, 0x102B1698);
	}

};

extern ConditionStructs conds;



uint32_t ConditionAddDispatch(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
void CondNodeAddToSubDispNodeArray(Dispatcher* dispatcher, CondNode* condNode);
uint32_t ConditionAddToAttribs_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct);
uint32_t ConditionAddToAttribs_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2);
uint32_t ConditionAdd_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct);
uint32_t ConditionAdd_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2);
uint32_t ConditionAdd_NumArgs3(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3);
uint32_t ConditionAdd_NumArgs4(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);


uint32_t ConditionPrevent(DispatcherCallbackArgs args);