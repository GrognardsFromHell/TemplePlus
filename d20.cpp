#include "stdafx.h"
#include "common.h"
#include "d20.h"
#include "temple_functions.h"
#include "obj.h"
#include "addresses.h"
#include "feat.h"
#include "fixes.h"
#include "spell.h"
#include "dispatcher.h"
#include "condition.h"


static_assert(sizeof(D20SpellData) == (8U), "D20SpellData structure has the wrong size!"); //shut up compiler, this is ok


ConditionStructs conds;
CharacterClasses charClasses;


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
			ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionMonsterUndead);
		}

		uint32_t objRace = objects.GetRace(objHnd);
		CondStruct ** condStructRace = conds.ConditionArrayRace + objRace;
		ConditionAddToAttribs_NumArgs0(dispatcher, *condStructRace);

		if (objects.IsSubtypeFire(objHnd))
		{
			ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionSubtypeFire);
		}

		if (objects.IsOoze(objHnd))
		{
			ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionMonsterOoze);
		}
	}
};


void D20StatusInitClass(objHndl objHnd)
{
	if (objects.IsCritter(objHnd))
	{
		Dispatcher * dispatcher = objects.GetDispatcher(objHnd);

		CondStruct ** condStructClass = conds.ConditionArrayClasses;

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
			ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionTurnUndead );
		}

		if (objects.StatLevelGet(objHnd, stat_level_bard) >= 1)
		{
			ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionBardicMusic);
		}

		if (objects.GetInt32(objHnd, obj_f_critter_school_specialization) & 0xFF)
		{
			ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionSchoolSpecialization);
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
		CondStruct * condStructDomain1 = *(conds.ConditionArrayDomains + 3*domain_1);
		uint32_t arg1 = * (conds.ConditionArrayDomainsArg1 + 3*domain_1);
		uint32_t arg2 = *(conds.ConditionArrayDomainsArg2 + 3 * domain_1);
		if (condStructDomain1 != nullptr)
		{
			ConditionAddToAttribs_NumArgs2(dispatcher, condStructDomain1, arg1, arg2);
		}
	}

	if (domain_2)
	{
		CondStruct * condStructDomain2 = *(conds.ConditionArrayDomains + 3 * domain_2);
		uint32_t arg1 = *(conds.ConditionArrayDomainsArg1 + 3 * domain_2);
		uint32_t arg2 = *(conds.ConditionArrayDomainsArg2 + 3 * domain_2);
		if (condStructDomain2 != nullptr)
		{
			ConditionAddToAttribs_NumArgs2(dispatcher, condStructDomain2, arg1, arg2);
		}
	}

	auto alignmentchoice = objects.GetInt32(objHnd, obj_f_critter_alignment_choice);
	if (alignmentchoice == 2)
	{
		ConditionAddToAttribs_NumArgs2(dispatcher, conds.ConditionTurnUndead, 1, 0);
	} else
	{
		ConditionAddToAttribs_NumArgs2(dispatcher, conds.ConditionTurnUndead, 0, 0);
	}
}

void D20StatusInitFeats(objHndl objHnd)
{
	if( objects.IsCritter(objHnd))
	{
		Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
		feat_enums featList[1000] = {};
		uint32_t numFeats = feats.FeatListElective(objHnd, featList);

		for (uint32_t i = 0; i < numFeats; i++)
		{
			CondStruct * cond; //WIP TODO
		//	if ()
			{
	//			ConditionAddToAttribs_NumArgs2(dispatcher, cond, featList[i], )
			}
		}
	}
};




