#include "stdafx.h"
#include "d20.h"
#include "obj.h"
#include "addresses.h"

static_assert(sizeof(D20SpellData) == (8U), "D20SpellData structure has the wrong size!"); //shut up compiler, this is ok

extern SpontCastSpellLists spontCastSpellLists_spells;
GlobalPrimitive<CondStruct, 0x102E8088> ConditonGlobal;
GlobalPrimitive<CondNode*, 0x10BCADA0> pCondNodeGlobal;
GlobalPrimitive<CondStruct, 0x102EF9A8> ConditionMonsterUndead;
GlobalPrimitive<CondStruct, 0x102EFBE8> ConditionSubtypeFire;
GlobalPrimitive<CondStruct, 0x102EFAF0> ConditionMonsterOoze;
GlobalPrimitive<CondStruct*, 0x102EFC18> ConditionArrayRace;
GlobalPrimitive<CondStruct *, 0x102F0634> ConditionArrayClasses;
GlobalPrimitive<CondStruct, 0x102B0D48> ConditionTurnUndead;
GlobalPrimitive<CondStruct, 0x102F0520> ConditionBardicMusic;
GlobalPrimitive<CondStruct, 0x102F0604> ConditionSchoolSpecialization;
GlobalPrimitive<CondStruct *, 0x102B1690> ConditionArrayDomains;
GlobalPrimitive<uint32_t, 0x102B1694> ConditionArrayDomainsArg1;
GlobalPrimitive<uint32_t, 0x102B1698> ConditionArrayDomainsArg2;

auto pConditionTurnUndead = ConditionTurnUndead.ptr();
auto pConditionBardicMusic = ConditionBardicMusic.ptr();
auto pConditionSchoolSpec = ConditionSchoolSpecialization.ptr();


const uint32_t NUM_CLASSES = stat_level_wizard - stat_level_barbarian + 1;




class D20Replacements : public TempleFix {
public:
	const char* name() override {
		return "D20 Function Replacements";
	}

	void apply() override {
		replaceFunction(0x10077850, D20SpellDataExtractInfo);
		replaceFunction(0x10077830, D20SpellDataSetSpontCast);
		replaceFunction(0x1004D700, DispIO14hCheckDispIOType1);
		replaceFunction(0x100ECF30, ConditionPrevent);
		replaceFunction(0x100FD790, D20StatusInitRace);
		replaceFunction(0x100FD790, D20StatusInitClass);
		replaceFunction(0x100E1E30, DispatcherRemoveSubDispNodes);
		replaceFunction(0x100E2400, DispatcherClearField);
		replaceFunction(0x100E2720, DispatcherClearAttribs);
		replaceFunction(0x100E2740, DispatcherClearItemConds);
		replaceFunction(0x100E2760, DispatcherClearConds);
		
	}
} d20Replacements;












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
		}
 else
 {
	 *spellEnum = d20SpellData->spellEnumOriginal;
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


uint32_t ConditionPrevent(DispatcherCallbackArgs args)
{
	DispIO14h * dispIO = DispIO14hCheckDispIOType1((DispIO14h*)args.dispIO);
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


void D20StatusInit(objHndl objHnd)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (dispatcher != nullptr && (uint32_t)dispatcher != 0xFFFFFFFF)
	{
		return ;
	}

	dispatcher = DispatcherInit(objHnd);

	objects.SetDispatcher(objHnd, (uint32_t)dispatcher);

	DispatcherClearAttribs(dispatcher);
	
	D20StatusInitClass(objHnd);


	D20StatusInitRace(objHnd);

}


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


void D20StatusInitClass(objHndl objHnd)
{
	if (objects.IsCritter(objHnd))
	{
		Dispatcher * dispatcher = objects.GetDispatcher(objHnd);

		CondStruct ** condStructClass = ConditionArrayClasses.ptr();

		uint32_t stat = stat_caster_level_barbarian;
		for (uint32_t i = 0; i < NUM_CLASSES; i++)
		{
			if ( objects.StatLevelGet(objHnd, (Stat)stat) > 0 
				&& *condStructClass != nullptr)
			{
				ConditionAddToAttribs_NumArgs0(dispatcher, *condStructClass);
			};

			condStructClass += 1;
			stat+=1;
		}
		
		if (objects.StatLevelGet(objHnd, stat_level_cleric) >= 1)
		{
			D20StatusInitDomains(objHnd);
		}

		if (objects.StatLevelGet(objHnd, stat_level_paladin) >= 3)
		{
			ConditionAddToAttribs_NumArgs0(dispatcher, pConditionTurnUndead );
		}

		if (objects.StatLevelGet(objHnd, stat_level_bard) >= 1)
		{
			ConditionAddToAttribs_NumArgs0(dispatcher, pConditionBardicMusic);
		}

		if (objects.GetInt32(objHnd, obj_f_critter_school_specialization) & 0xFF)
		{
			ConditionAddToAttribs_NumArgs0(dispatcher, pConditionSchoolSpec);
		}
	}
};

void D20StatusInitDomains(objHndl objHnd)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	uint32_t domain_1 = objects.GetInt32(objHnd, obj_f_critter_domain_1);
	uint32_t domain_2 = objects.GetInt32(objHnd, obj_f_critter_domain_2);
	
	if (domain_1)
	{
		CondStruct * condStructDomain1 = *(ConditionArrayDomains.ptr() + 3*domain_1);
		uint32_t arg1 = * (ConditionArrayDomainsArg1.ptr() + 3*domain_1);
		uint32_t arg2 = *(ConditionArrayDomainsArg2.ptr() + 3 * domain_1);
		if (condStructDomain1 != nullptr)
		{
			ConditionAddToAttribs_NumArgs2(dispatcher, condStructDomain1, arg1, arg2);
		}
	}

	if (domain_2)
	{
		CondStruct * condStructDomain2 = *(ConditionArrayDomains.ptr() + 3 * domain_2);
		uint32_t arg1 = *(ConditionArrayDomainsArg1.ptr() + 3 * domain_2);
		uint32_t arg2 = *(ConditionArrayDomainsArg2.ptr() + 3 * domain_2);
		if (condStructDomain2 != nullptr)
		{
			ConditionAddToAttribs_NumArgs2(dispatcher, condStructDomain2, arg1, arg2);
		}
	}

	auto alignmentchoice = objects.GetInt32(objHnd, obj_f_critter_alignment_choice);
	if (alignmentchoice == 2)
	{
		ConditionAddToAttribs_NumArgs2(dispatcher, pConditionTurnUndead, 1, 0);
	} else
	{
		ConditionAddToAttribs_NumArgs2(dispatcher, pConditionTurnUndead, 0, 0);
	}
}


void __cdecl DispatcherClearField(Dispatcher *dispatcher, CondNode ** dispCondList)
{
	CondNode * cond = *dispCondList;
	objHndl obj = dispatcher->objHnd;
	while ( cond != nullptr)
	{
		SubDispNode * subDispNode_TypeRemoveCond = dispatcher->subDispNodes[2];
		CondNode * nextCond = cond->nextCondNode;

		while (subDispNode_TypeRemoveCond != nullptr)
		{
			
			SubDispDef * sdd = subDispNode_TypeRemoveCond->subDispDef;
			if (sdd->dispKey == 0 && (subDispNode_TypeRemoveCond->condNode->flags & 1 ) == 0 
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

void __cdecl DispatcherRemoveSubDispNodes(Dispatcher * dispatcher, CondNode * cond)
{
	for (uint32_t i = 0; i < dispTypeCount; i++)
	{
		SubDispNode ** ppSubDispNode = &dispatcher->subDispNodes[i];
		while (*ppSubDispNode != nullptr)
		{
			if ( (*ppSubDispNode)->condNode == cond)
			{
				SubDispNode * savedNext = (*ppSubDispNode)->next;
				allocFuncs.free(*ppSubDispNode);
				*ppSubDispNode = savedNext;
				
			} else
			{
				ppSubDispNode = & ((*ppSubDispNode)->next);
			}
			
		}
	}

};


DispIO14h * DispIO14hCheckDispIOType1(DispIO14h * dispIO)
{
	if (dispIO->dispIOType == 1){ return dispIO; }
	else { return nullptr; }
};