#pragma once

#include "temple_functions.h"
#include "spell.h"
#include "feat.h"
#include "fixes.h"
#include "obj.h"
#include "dispatcher.h"


enum SpontCastType : unsigned char {
	spontCastGoodCleric = 2,
	spontCastEvilCleric = 4,
	spontCastDruid = 8
};

struct D20SpellData
{
	uint16_t spellEnumOriginal;
	MetaMagicData metaMagicData;
	uint8_t spellClassCode ;
	uint8_t itemSpellData;
	SpontCastType spontCastType: 4;
	unsigned char spellSlotLevel : 4;
};

const uint32_t TestSizeOfD20SpellData = sizeof(D20SpellData);


void __cdecl D20SpellDataSetSpontCast(D20SpellData*, SpontCastType spontCastType);
void D20SpellDataExtractInfo
(D20SpellData * d20SpellData, uint32_t * spellEnum, uint32_t * spellEnumOriginal,
uint32_t * spellClassCode, uint32_t * spellSlotLevel, uint32_t * itemSpellData,
uint32_t * metaMagicData);


void DispIO_Size32_Type21_Init(DispIO20h* dispIO);
uint32_t Dispatch62(objHndl, DispIO*, uint32_t dispKey);
uint32_t Dispatch63(objHndl objHnd, DispIO* dispIO);
uint32_t ConditionAddDispatch(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
void CondNodeAddToSubDispNodeArray(Dispatcher* dispatcher, CondNode* condNode);
uint32_t ConditionAddToAttribs_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct);
uint32_t ConditionAddToAttribs_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2);
uint32_t ConditionAdd_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct);
uint32_t ConditionAdd_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2);
uint32_t ConditionAdd_NumArgs3(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3);
uint32_t ConditionAdd_NumArgs4(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);


DispIO14h * DispIO14hCheckDispIOType1(DispIO14h * dispIO);
uint32_t ConditionPrevent(DispatcherCallbackArgs args);

void D20StatusInitRace(objHndl objHnd);
void D20StatusInitClass(objHndl objHnd);
void D20StatusInit(objHndl objHnd);
void D20StatusInitDomains(objHndl objHnd);




extern GlobalPrimitive<CondStruct, 0x102E8088> ConditionGlobal;
extern GlobalPrimitive<CondNode *, 0x10BCADA0> pCondNodeGlobal;