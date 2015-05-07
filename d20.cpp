#include "stdafx.h"
#include "common.h"
#include "d20.h"
#include "temple_functions.h"
#include "obj.h"
#include "util/addresses.h"
#include "feat.h"
#include "util/fixes.h"
#include "spell.h"
#include "dispatcher.h"
#include "condition.h"
#include "inventory.h"
#include "python/python_debug.h"
#include "pathfinding.h"
#include "location.h"
#include "action_sequence.h"
#include "critter.h"


static_assert(sizeof(D20SpellData) == (8U), "D20SpellData structure has the wrong size!"); //shut up compiler, this is ok
static_assert(sizeof(D20Actn) == 0x58, "D20Action struct has the wrong size!");
static_assert(sizeof(D20ActionDef) == 0x30, "D20ActionDef struct has the wrong size!");


class D20Replacements : public TempleFix {
public:
	const char* name() override {
		return "D20 Function Replacements";
	}

	void apply() override {
		replaceFunction(0x10077850, D20SpellDataExtractInfo);
		replaceFunction(0x10077830, D20SpellDataSetSpontCast);
		macReplaceFun(10077800, _d20ActnSetSpellData)

		replaceFunction(0x100FD790, _D20StatusInitRace);
		replaceFunction(0x100FEE60, _D20StatusInitClass);
		replaceFunction(0x1004FDB0, _D20StatusInit);
		replaceFunction(0x100FD2D0, _D20StatusInitFeats);
		replaceFunction(0x1004CA00, _D20StatusInitItemConditions);
		replaceFunction(0x1004CC00, _D20Query);
		replaceFunction(0x1004CC60, _d20QueryWithData);
		macReplaceFun(1004E6B0, _d20SendSignal)
		macReplaceFun(1004CD40, _d20QueryReturnData)
		replaceFunction(0x10093810, _d20aInitUsercallWrapper); // function takes esi as argument

		replaceFunction(0x10089F80, _globD20aSetTypeAndData1);
		macReplaceFun(1008A450, _GlobD20ActnSetSpellData)
		macReplaceFun(1008A530, _globD20aSetPerformer)
		macReplaceFun(100949E0, _globD20ActnInit)
	}
} d20Replacements;


static struct D20SystemAddresses : AddressTable {

	void(__cdecl*  GlobD20ActnSetTarget)(objHndl objHnd, LocAndOffsets * loc);
	D20SystemAddresses()
	{
		macRebase(GlobD20ActnSetTarget, 10092E50)
	}
} addresses;

#pragma region D20System Implementation
D20System d20Sys;

D20System::D20System()
{
	pathfinding = &pathfindingSys;
	actSeq = &actSeqSys;
	d20Class = &d20ClassSys;
	d20Status = &d20StatusSys;
	rebase(D20StatusInitFromInternalFields, 0x1004F910);
	rebase(AppendObjHndToArray10BCAD94, 0x100DFAD0);
	rebase(D20GlobalSthg10AA3284, 0x10AA3284);
	rebase(globD20Action, 0x1186AC00);
	rebase(ToHitProc, 0x100B7160);
	rebase(d20Defs, 0x102CC5C8);
	//rebase(ToEEd20ActionNames, 0x102CD2BC);
	rebase(_d20aTriggerCombatCheck, 0x1008AE90);//ActnSeq * @<eax>
	rebase(_tumbleCheck, 0x1008AA90);
	rebase(_d20aTriggersAOO, 0x1008A9C0);
	rebase(CreateRollHistory, 0x100DFFF0);
}




uint32_t D20System::d20Query(objHndl objHnd, D20DispatcherKey dispKey)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (dispatcher == nullptr || (int32_t)dispatcher == -1){ return 0; }
	DispIoD20Query dispIO;
	dispIO.dispIOType = dispIOTypeQuery;
	dispIO.return_val = 0;
	dispIO.data1 = 0;
	dispIO.data2 = 0;
	objects.dispatch.DispatcherProcessor(dispatcher, dispTypeD20Query, dispKey, &dispIO);
	return dispIO.return_val;
}

uint32_t D20System::d20QueryWithData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (dispatcher == nullptr || (int32_t)dispatcher == -1){ return 0; }
	DispIoD20Query dispIO;
	dispIO.dispIOType = dispIOTypeQuery;
	dispIO.return_val = 0;
	dispIO.data1 = arg1;
	dispIO.data2 = arg2;
	objects.dispatch.DispatcherProcessor(dispatcher, dispTypeD20Query, dispKey, &dispIO);
	return dispIO.return_val;
}

void D20System::d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, int32_t arg1, int32_t arg2)
{
	DispIoD20Query dispIO;
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (!dispatch.dispatcherValid(dispatcher))
	{
		hooked_print_debug_message("d20SendSignal(): Object %s (%I64x) lacks a Dispatcher", description._getDisplayName(objHnd, objHnd), objHnd);
		return;
	}
	dispIO.dispIOType = dispIoTypeSendSignal;
	dispIO.data1 = arg1;
	dispIO.data2 = arg2;
	dispatch.DispatcherProcessor(dispatcher, dispTypeD20Signal, dispKey, &dispIO);
}

void D20System::d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, objHndl arg) {
	DispIoD20Query dispIO;
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (!dispatch.dispatcherValid(dispatcher))
	{
		hooked_print_debug_message("d20SendSignal(): Object %s (%I64x) lacks a Dispatcher", description._getDisplayName(objHnd, objHnd), objHnd);
		return;
	}
	dispIO.dispIOType = dispIoTypeSendSignal;
	*(objHndl*)&dispIO.data1 = arg;
	dispatch.DispatcherProcessor(dispatcher, dispTypeD20Signal, dispKey, &dispIO);
}

void D20System::d20ActnInit(objHndl objHnd, D20Actn* d20a)
{
	d20a->d20APerformer = objHnd;
	d20a->d20ActType = D20A_NONE;
	d20a->data1=0 ;
	d20a->d20ATarget=0;
	d20a->distTraversed = 0;
	d20a->field_34 = 0;
	d20a->spellId = 0;
	objects.loc->getLocAndOff(objHnd, &d20a->destLoc);
	PathQueryResult * pq = d20a->path;
	if (pq && pq >= pathfinding->pathQArray && pq < (pathfinding->pathQArray + pfCacheSize))
	{
		pq->occupiedFlag = 0;
	}
	d20a->path = nullptr;
	d20a->d20SpellData.spellEnumOrg = 0;
	d20a->animID = 0;
	d20a->rollHist1 = -1;
	d20a->rollHist2 = -1;
	d20a->rollHist3 = -1;
	
}

void D20System::GlobD20ActnSetTypeAndData1(D20ActionType d20type, uint32_t data1)
{
	globD20Action->d20ActType = d20type;
	globD20Action->data1 = data1;
}

void D20System::globD20ActnSetPerformer(objHndl objHnd)
{
	if (objHnd != (*globD20Action).d20APerformer)
	{
		*actSeq->seqSthg_118CD3B8 = -1;
		*actSeq->seqSthg_118A0980 = 1;
		*actSeq->seqSthg_118CD570 = 0;
	}
	(*globD20Action).d20APerformer = objHnd;
}

void D20System::GlobD20ActnSetTarget(objHndl objHnd, LocAndOffsets * loc)
{
	addresses.GlobD20ActnSetTarget(objHnd, loc);
}

void D20System::GlobD20ActnInit()
{
	d20ActnInit(globD20Action->d20APerformer, globD20Action);
}

void D20System::d20aTriggerCombatCheck(ActnSeq* actSeq, int32_t idx)
{
	__asm{
		push esi;
		push ecx;
		mov ecx, this;
		mov esi, idx;
		push esi;
		mov esi, [ecx]._d20aTriggerCombatCheck;
		mov eax, actSeq;
		call esi;
		add esp, 4;

		pop ecx;
		pop esi;
	}
	//void d20aTriggerCombatCheck(ActnSeq* actSeq, int32_t idx);//1008AE90    ActnSeq * @<eax>
}

int32_t D20System::d20aTriggersAOOCheck(D20Actn* d20a, void* iO)
{
	uint32_t result = 0;
	__asm{
		push esi;
		push ecx;
		push ebx;
		mov ecx, this;
		mov esi, iO;
		push esi;
		mov ebx, [ecx]._d20aTriggersAOO;
		mov esi, d20a;
		call ebx;
		add esp, 4;
		mov result, eax;
		pop ebx;
		pop ecx;
		pop esi;
	}
	return result; //_d20aTriggersAOO(void * iO); // d20a @<esi> // 1008A9C0
}

uint32_t D20System::tumbleCheck(D20Actn* d20a)
{
	if (d20QueryWithData(d20a->d20ATarget, DK_QUE_Critter_Has_Spell_Active, 407, 0)) return 0; // spell_sanctuary active
	if (actSeq->isPerforming(d20a->d20APerformer))
	{
		hooked_print_debug_message("movement aoo while performing...\n");
		return 0;
	}
	if (!d20QueryWithData(d20a->d20ATarget, DK_QUE_AOOIncurs, (uint32_t)(d20a->d20APerformer & 0xFFFFffff), (uint32_t)(d20a->d20APerformer >> 32))){ return 0; }
	if (!d20QueryWithData(d20a->d20APerformer, DK_QUE_AOOPossible, (uint32_t)(d20a->d20ATarget & 0xFFFFffff), (uint32_t)(d20a->d20ATarget >> 32))){ return 0; }
	if (!d20QueryWithData(d20a->d20APerformer, DK_QUE_AOOWillTake, (uint32_t)(d20a->d20ATarget & 0xFFFFffff), (uint32_t)(d20a->d20ATarget >> 32))){ return 0; }
	// not fully implemented yet, but that should cover 90% of the cases anyway ;) TODO: complete this function
	return _tumbleCheck(d20a);
}

void D20System::D20ActnSetSpellData(D20SpellData* d20SpellData, uint32_t spellEnumOrg, uint32_t spellClassCode, uint32_t spellSlotLevel, uint32_t itemSpellData, uint32_t metaMagicData)
{
	*(uint32_t *)&d20SpellData->metaMagicData = metaMagicData;
	d20SpellData->spellEnumOrg = spellEnumOrg;
	d20SpellData->spellClassCode = spellClassCode;
	d20SpellData->itemSpellData = itemSpellData;
	d20SpellData->spontCastType = (SpontCastType)0;
	d20SpellData->spellSlotLevel = spellSlotLevel;
}

void D20System::GlobD20ActnSetSpellData(D20SpellData* d20SpellData)
{
	d20Sys.globD20Action->d20SpellData = *d20SpellData;
}

uint64_t D20System::d20QueryReturnData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1, ::uint32_t arg2)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (!dispatch.dispatcherValid(dispatcher)){ return 0; }
	DispIoD20Query dispIO;
	dispIO.dispIOType = dispIOTypeQuery;
	dispIO.return_val = 0;
	dispIO.data1 = arg1;
	dispIO.data2 = arg2;
	objects.dispatch.DispatcherProcessor(dispatcher, dispTypeD20Query, dispKey, &dispIO);
	return *(uint64_t*)&dispIO.data1;
}
#pragma endregion 





#pragma region D20 Spell Stuff

void D20SpellDataExtractInfo
  (D20SpellData * d20SpellData	, uint32_t * spellEnum		, uint32_t * spellEnumOriginal	, 
   uint32_t * spellClassCode	, uint32_t * spellSlotLevel	, uint32_t * itemSpellData		, 
   uint32_t * metaMagicData)
{
	if ( ! (spellEnumOriginal == nullptr) )
	{
		*spellEnumOriginal = d20SpellData->spellEnumOrg;
	}

	if (!(spellSlotLevel == nullptr ) )
	{
		*spellSlotLevel = d20SpellData->spellSlotLevel;
	}

	if (! (spellEnum == nullptr))
	{
		if ((SpontCastType) d20SpellData->spontCastType == spontCastGoodCleric)
		{
			*spellEnum = spontCastSpellLists.spontCastSpellsGoodCleric[d20SpellData->spellSlotLevel];
		}
		else if ((SpontCastType)d20SpellData->spontCastType == spontCastEvilCleric)
		{
			*spellEnum = spontCastSpellLists.spontCastSpellsEvilCleric[d20SpellData->spellSlotLevel];
		} 
		else if ( (SpontCastType) d20SpellData->spontCastType == spontCastDruid)
		{
			*spellEnum = spontCastSpellLists.spontCastSpellsDruid[d20SpellData->spellSlotLevel];
		}
 else
 {
	 *spellEnum = d20SpellData->spellEnumOrg;
 }
	}

	if (!(spellClassCode == nullptr))
	{
		*spellClassCode = d20SpellData->spellClassCode;
	}

	if (!(itemSpellData == nullptr))
	{
		*itemSpellData = d20SpellData->itemSpellData;
	}

	if (!(metaMagicData == nullptr))
	{
		*metaMagicData = (*((uint32_t*)&(d20SpellData->metaMagicData))) & (0xFFFFFF); // sue me! it WORKS
	}

	return;
}


void __cdecl D20SpellDataSetSpontCast(D20SpellData* d20SpellData, SpontCastType spontCastType)
{
	if (!(d20SpellData == nullptr))
	{
		d20SpellData->spontCastType = spontCastType;
	}
};

#pragma endregion



#pragma region D20Status Init Functions

void _D20StatusInit(objHndl objHnd)
{
	d20Sys.d20Status->D20StatusInit(objHnd);
	}


void _D20StatusInitRace(objHndl objHnd)
{
	d20Sys.d20Status->initRace(objHnd);
};


void _D20StatusInitClass(objHndl objHnd)
{
	d20Sys.d20Status->initClass(objHnd);
};

void _D20StatusInitDomains(objHndl objHnd)
{
	d20Sys.d20Status->initDomains(objHnd);
}

void _D20StatusInitFeats(objHndl objHnd)
{
	d20Sys.d20Status->initFeats(objHnd);
};

void _D20StatusInitItemConditions(objHndl objHnd)
{
	d20Sys.d20Status->initItemConditions(objHnd);
}

#pragma endregion


uint32_t _D20Query(objHndl objHnd, D20DispatcherKey dispKey)
{
	return d20Sys.d20Query(objHnd, dispKey);
}

void _d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, int32_t arg1, int32_t arg2)
{
	d20Sys.d20SendSignal(objHnd, dispKey, arg1, arg2);
}

void __cdecl _d20aInitCdecl(objHndl objHnd, D20Actn* d20a)
{
	d20Sys.d20ActnInit(objHnd, d20a);
}


void __declspec(naked) _d20aInitUsercallWrapper(objHndl objHnd)
{
	__asm{ // esi is D20Actn * d20a
		push ebp; 
		mov ebp, esp; // ebp = &var4  ebp+4 = &retaddr  ebp+8 = &arg1

		push esi; 
		mov eax, [ebp + 12];
		push eax;
		mov eax, [ebp + 8];
		push eax;
		mov eax, _d20aInitCdecl;
		call eax;
		add esp, 8;

		pop esi;
		mov esp, ebp;
		pop ebp;
		retn;
	}
}

void _d20ActnSetSpellData(D20SpellData* d20SpellData, uint32_t spellEnumOrg, uint32_t spellClassCode, uint32_t spellSlotLevel, uint32_t itemSpellData, uint32_t metaMagicData)
{
	d20Sys.D20ActnSetSpellData(d20SpellData, spellEnumOrg, spellClassCode, spellSlotLevel, itemSpellData, metaMagicData);
}

void _globD20aSetTypeAndData1(D20ActionType d20type, uint32_t data1)
{
	d20Sys.GlobD20ActnSetTypeAndData1(d20type, data1);
}

uint32_t _d20QueryWithData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2)
{
	return d20Sys.d20QueryWithData(objHnd, dispKey, arg1, arg2);
}

uint64_t _d20QueryReturnData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2)
{
	return d20Sys.d20QueryReturnData(objHnd, dispKey, arg1, arg2);
}

void _globD20aSetPerformer(objHndl objHnd)
{
	d20Sys.globD20ActnSetPerformer(objHnd);
}

void _globD20ActnInit()
{
	d20Sys.GlobD20ActnInit();
}

void _GlobD20ActnSetSpellData(D20SpellData* d20SpellData)
{
	d20Sys.GlobD20ActnSetSpellData(d20SpellData);
}
