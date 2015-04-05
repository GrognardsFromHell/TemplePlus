#include "stdafx.h"
#include "d20.h"
#include "obj.h"

static_assert(sizeof(D20SpellData) == (8U), "D20SpellData structure has the wrong size!"); //shut up compiler, this is ok

extern SpontCastSpellLists spontCastSpellLists_spells;
GlobalPrimitive<CondStruct, 0x102E8088> ConditonGlobal;
GlobalPrimitive<CondNode*, 0x10BCADA0> pCondNodeGlobal;
GlobalPrimitive<CondStruct, 0x102EF9A8> ConditionMonsterUndead;
GlobalPrimitive<CondStruct, 0x102EFBE8> ConditionSubtypeFire;
GlobalPrimitive<CondStruct, 0x102EFAF0> ConditionMonsterOoze;
GlobalPrimitive<CondStruct*, 0x102EFC18> ConditionArrayRace;


void D20SpellDataExtractInfo
  (D20SpellData * d20SpellData	, uint32_t * spellEnum		, uint32_t * spellEnumOriginal	, 
   uint32_t * spellClassCode	, uint32_t * spellSlotLevel	, uint32_t * itemSpellData		, 
   uint32_t * metaMagicData)
{
	if ( ! (spellEnumOriginal == nullptr) )
	{
		*spellEnumOriginal = d20SpellData->spellEnumOriginal;
	}

	if (!(spellSlotLevel == nullptr ) )
	{
		*spellSlotLevel = d20SpellData->spellSlotLevel;
	}

	if (! (spellEnum == nullptr))
	{
		if ((SpontCastType) d20SpellData->spontCastType == spontCastGoodCleric)
		{
			*spellEnum = spontCastSpellLists_spells.spontCastSpellsGoodCleric[d20SpellData->spellSlotLevel];
		}
		else if ((SpontCastType)d20SpellData->spontCastType == spontCastEvilCleric)
		{
			*spellEnum = spontCastSpellLists_spells.spontCastSpellsEvilCleric[d20SpellData->spellSlotLevel];
		} 
		else if ( (SpontCastType) d20SpellData->spontCastType == spontCastDruid)
		{
			*spellEnum = spontCastSpellLists_spells.spontCastSpellsDruid[d20SpellData->spellSlotLevel];
		} else
		{
			*spellEnum = d20SpellData->spellEnumOriginal;
		}
	}

	if (!(spellClassCode == nullptr))
	{
		*spellClassCode = d20SpellData->spellClassCode;
	}

	if (!(itemSpellData== nullptr))
	{
		*itemSpellData = d20SpellData->itemSpellData;
	}

	if (!(metaMagicData== nullptr))
	{
		*metaMagicData =  (* ( (uint32_t*) &(d20SpellData->metaMagicData) ) ) & ( 0xFFFFFF); // sue me! it WORKS
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


uint32_t ConditionPrevent(DispatcherCallbackArgs args)
{
	DispIO14h * dispIO = DispIO14hCheckDispIOType1( (DispIO14h*)args.dispIO);
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

void D20StatusInitRace(objHndl objHnd)
{
	if (objects.IsCritter(objHnd))
	{
		Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
		if (objects.IsUndead(objHnd))
		{
			ConditionAddToAttribs_NumArgs0(dispatcher, ConditionMonsterUndead.ptr());
		}

		uint32_t objRace = objects.GetRace(objHnd);
		CondStruct ** condStructRace = ConditionArrayRace.ptr() + objRace;
		ConditionAddToAttribs_NumArgs0(dispatcher, *condStructRace);

		if (objects.IsSubtypeFire(objHnd))
		{
			ConditionAddToAttribs_NumArgs0(dispatcher, ConditionSubtypeFire.ptr());
		}

		if (objects.IsOoze(objHnd))
		{
			ConditionAddToAttribs_NumArgs0(dispatcher, ConditionMonsterOoze.ptr());
		}
	}
};

class D20Replacements : public TempleFix {
public:
	const char* name() override {
		return "D20 Function Replacements";
	}

	void apply() override {
		replaceFunction(0x10077850, D20SpellDataExtractInfo);
		replaceFunction(0x10077830, D20SpellDataSetSpontCast);
		replaceFunction(0x1004D700, DispIO14hCheckDispIOType1);
	//	replaceFunction(0x100ECF30, ConditionPrevent);
	//	replaceFunction(0x100FD790, D20StatusInitRace);
	}
} d20Replacements;



DispIO14h * DispIO14hCheckDispIOType1(DispIO14h * dispIO)
{
	if (dispIO->dispIOType == 1){ return dispIO; }
	else { return nullptr; }
};