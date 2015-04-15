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


static_assert(sizeof(D20SpellData) == (8U), "D20SpellData structure has the wrong size!"); //shut up compiler, this is ok
static_assert(sizeof(D20Actn) == 0x58, "D20Action struct has the wrong size!");
static_assert(sizeof(D20ActionDef) == 0x30, "D20ActionDef struct has the wrong size!");


#pragma region D20System Implementation
D20System d20sys;

D20System::D20System()
{
	pathfinding = &pathfindingSys;
	actSeq = &actSeqSys;
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
}


void D20System::D20StatusInitRace(objHndl objHnd)
{
	_D20StatusInitRace(objHnd);
}
void D20System::D20StatusInitClass(objHndl objHnd)
{
	_D20StatusInitClass(objHnd);
}
void D20System::D20StatusInit(objHndl objHnd)
{
	_D20StatusInit(objHnd);
}
void D20System::D20StatusInitDomains(objHndl objHnd)
{
	_D20StatusInitDomains(objHnd);
}

void D20System::D20StatusInitFeats(objHndl objHnd)
{
	_D20StatusInitFeats(objHnd);
}

void D20System::D20StatusInitItemConditions(objHndl objHnd)
{
	_D20StatusInitItemConditions(objHnd);
}

uint32_t D20System::d20Query(objHndl objHnd, D20DispatcherKey dispKey)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (dispatcher == nullptr || (int32_t)dispatcher == -1){ return 0; }
	DispIO10h dispIO;
	dispIO.dispIOType = dispIOType7;
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
	DispIO10h dispIO;
	dispIO.dispIOType = dispIOType7;
	dispIO.return_val = 0;
	dispIO.data1 = arg1;
	dispIO.data2 = arg2;
	objects.dispatch.DispatcherProcessor(dispatcher, dispTypeD20Query, dispKey, &dispIO);
	return dispIO.return_val;
}

void D20System::d20ActnInit(objHndl objHnd, D20Actn* d20a)
{
	d20a->d20APerformer = objHnd;
	d20a->d20ActType = D20A_NONE;
	d20a->data1=0 ;
	d20a->d20ATarget=0;
	d20a->distTraversed = 0;
	d20a->field_34 = 0;
	d20a->spellEnum = 0;
	objects.loc->getLocAndOff(objHnd, &d20a->locAndOff);
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

void D20System::globD20aSetTypeAndData1(D20ActionType d20type, uint32_t data1)
{
	globD20Action->d20ActType = d20type;
	globD20Action->data1 = data1;
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
#pragma endregion 

CharacterClasses charClasses;


class D20Replacements : public TempleFix {
public:
	const char* name() override {
		return "D20 Function Replacements";
	}

	void apply() override {
		replaceFunction(0x10077850, D20SpellDataExtractInfo);
		replaceFunction(0x10077830, D20SpellDataSetSpontCast);
		

		replaceFunction(0x100FD790, _D20StatusInitRace);
		replaceFunction(0x100FEE60, _D20StatusInitClass);
		replaceFunction(0x1004FDB0, _D20StatusInit);
		replaceFunction(0x100FD2D0, _D20StatusInitFeats);
		replaceFunction(0x1004CA00, _D20StatusInitItemConditions);
		replaceFunction(0x1004CC00, _D20Query);
		replaceFunction(0x10093810, _d20aInitUsercallWrapper); // function takes esi as argument
		replaceFunction(0x10089F80, _globD20aSetTypeAndData1);
		replaceFunction(0x1004CC60, _d20QueryWithData);
	}
} d20Replacements;


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
	static int debugLol = 0;
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (dispatcher != nullptr && (uint32_t)dispatcher != 0xFFFFFFFF)
	{
		return ;
	}

	dispatcher = objects.dispatch.DispatcherInit(objHnd);

	objects.SetDispatcher(objHnd, (uint32_t)dispatcher);

	objects.dispatch.DispatcherClearAttribs(dispatcher);
	
	if (objects.IsCritter(objHnd))
	{
		objects.d20.D20StatusInitClass(objHnd);

		objects.d20.D20StatusInitRace(objHnd);

		objects.d20.D20StatusInitFeats(objHnd);

	} else
	{
		debugLol ++;
		if (debugLol % 1000 == 1)
		{
			auto lololol = 0;
		}
	}

	objects.d20.D20StatusInitItemConditions(objHnd);

	objects.d20.D20StatusInitFromInternalFields(objHnd, dispatcher);

	objects.d20.AppendObjHndToArray10BCAD94(objHnd);

	if (*objects.d20.D20GlobalSthg10AA3284 != 0){ return; }

	if (objects.IsCritter(objHnd))
	{
		if (! objects.IsDeadNullDestroyed(objHnd))
		{
			uint32_t hpCur = objects.StatLevelGet(objHnd, stat_hp_current);
			uint32_t subdualDam = objects.getInt32(objHnd, obj_f_critter_subdual_damage);

			if ( (uint32_t)hpCur != 0xFFFF0001)
			{
				if (hpCur < 0)
				{
					if (feats.HasFeatCount(objHnd, FEAT_DIEHARD))
					{
						_ConditionAdd_NumArgs0(dispatcher, conds.ConditionDisabled);
					} else
					{
						_ConditionAdd_NumArgs0(dispatcher, conds.ConditionUnconscious);
					}
				} 
				
				else
				{
					if (hpCur == 0)
					{
						_ConditionAdd_NumArgs0(dispatcher, conds.ConditionDisabled);
					} else if ( subdualDam > hpCur)
					{
						_ConditionAdd_NumArgs0(dispatcher, conds.ConditionUnconscious);
					}
				}

			}
		}
	}
	return;


}


void _D20StatusInitRace(objHndl objHnd)
{
	if (objects.IsCritter(objHnd))
	{
		Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
		if (objects.IsUndead(objHnd))
		{
			_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionMonsterUndead);
		}

		uint32_t objRace = objects.GetRace(objHnd);
		CondStruct ** condStructRace = conds.ConditionArrayRace + objRace;
		_ConditionAddToAttribs_NumArgs0(dispatcher, *condStructRace);

		if (objects.IsSubtypeFire(objHnd))
		{
			_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionSubtypeFire);
		}

		if (objects.IsOoze(objHnd))
		{
			_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionMonsterOoze);
		}
	}
};


void _D20StatusInitClass(objHndl objHnd)
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
				_ConditionAddToAttribs_NumArgs0(dispatcher, *condStructClass);
			};

			condStructClass += 1;
			stat+=1;
		}
		
		if (objects.StatLevelGet(objHnd, stat_level_cleric) >= 1)
		{
			_D20StatusInitDomains(objHnd);
		}

		if (objects.StatLevelGet(objHnd, stat_level_paladin) >= 3)
		{
			_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionTurnUndead );
		}

		if (objects.StatLevelGet(objHnd, stat_level_bard) >= 1)
		{
			_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionBardicMusic);
		}

		if (objects.getInt32(objHnd, obj_f_critter_school_specialization) & 0xFF)
		{
			_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionSchoolSpecialization);
		}
	}
};

void _D20StatusInitDomains(objHndl objHnd)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	uint32_t domain_1 = objects.getInt32(objHnd, obj_f_critter_domain_1);
	uint32_t domain_2 = objects.getInt32(objHnd, obj_f_critter_domain_2);
	
	if (domain_1)
	{
		CondStruct * condStructDomain1 = *(conds.ConditionArrayDomains + 3*domain_1);
		uint32_t arg1 = * (conds.ConditionArrayDomainsArg1 + 3*domain_1);
		uint32_t arg2 = *(conds.ConditionArrayDomainsArg2 + 3 * domain_1);
		if (condStructDomain1 != nullptr)
		{
			_ConditionAddToAttribs_NumArgs2(dispatcher, condStructDomain1, arg1, arg2);
		}
	}

	if (domain_2)
	{
		CondStruct * condStructDomain2 = *(conds.ConditionArrayDomains + 3 * domain_2);
		uint32_t arg1 = *(conds.ConditionArrayDomainsArg1 + 3 * domain_2);
		uint32_t arg2 = *(conds.ConditionArrayDomainsArg2 + 3 * domain_2);
		if (condStructDomain2 != nullptr)
		{
			_ConditionAddToAttribs_NumArgs2(dispatcher, condStructDomain2, arg1, arg2);
		}
	}

	auto alignmentchoice = objects.getInt32(objHnd, obj_f_critter_alignment_choice);
	if (alignmentchoice == 2)
	{
		_ConditionAddToAttribs_NumArgs2(dispatcher, conds.ConditionTurnUndead, 1, 0);
	} else
	{
		_ConditionAddToAttribs_NumArgs2(dispatcher, conds.ConditionTurnUndead, 0, 0);
	}
}

void _D20StatusInitFeats(objHndl objHnd)
{
	
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	feat_enums featList[1000] = {};
	uint32_t numFeats = feats.FeatListElective(objHnd, featList);

	for (uint32_t i = 0; i < numFeats; i++)
	{
		CondStruct * cond; 
		uint32_t arg2 = 0;
		if (_GetCondStructFromFeat(featList[i], &cond, &arg2))
		{
			_ConditionAddToAttribs_NumArgs2(dispatcher, cond, featList[i], arg2);
		}
	}
	_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionAttackOfOpportunity);
	_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionCastDefensively);
	_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionDealSubdualDamage);
	_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionDealNormalDamage);
	_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionFightDefensively);
	
};

void _D20StatusInitItemConditions(objHndl objHnd)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (dispatcher == 0 || (int32_t)dispatcher == -1){ return; }
	if (objects.IsCritter(objHnd))
	{
		objects.dispatch.DispatcherClearItemConds(dispatcher);
		if (!objects.d20.d20Query(objHnd, DK_QUE_Polymorphed))
		{
			uint32_t invenCount = objects.getInt32(objHnd, obj_f_critter_inventory_num);
			for (uint32_t i = 0; i < invenCount; i++)
			{
				objHndl objHndItem = templeFuncs.Obj_Get_IdxField_ObjHnd(objHnd, obj_f_critter_inventory_list_idx, i);
				uint32_t itemInvocation = objects.getInt32(objHndItem, obj_f_item_inv_location);
				if (inventory.IsItemEffectingConditions(objHndItem, itemInvocation))
				{
					inventory.sub_100FF500(dispatcher, objHndItem, itemInvocation);
				}
			}
		}

	}
	return;
}

#pragma endregion


uint32_t _D20Query(objHndl objHnd, D20DispatcherKey dispKey)
{
	return d20sys.d20Query(objHnd, dispKey);
}


void __cdecl _d20aInitCdecl(objHndl objHnd, D20Actn* d20a)
{
	d20sys.d20ActnInit(objHnd, d20a);
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

void _globD20aSetTypeAndData1(D20ActionType d20type, uint32_t data1)
{
	d20sys.globD20aSetTypeAndData1(d20type, data1);
}

uint32_t _d20QueryWithData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2)
{
	return d20sys.d20QueryWithData(objHnd, dispKey, arg1, arg2);
}

static void D20Dumper() {

	

/*	for (auto i = 0; i < 68; i++) {
		logger->info("{}", d20sys.ToEEd20ActionNames[i]);
	}
	*/ // dump completed, imported into TemplePlus :)
}

static PythonDebugFunc pyMapObjDebugFunc("dump_d20_actions", &D20Dumper);