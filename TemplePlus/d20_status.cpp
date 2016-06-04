#include "stdafx.h"
#include "d20_status.h"
#include "condition.h"
#include "critter.h"
#include "obj.h"
#include "temple_functions.h"
#include "d20_obj_registry.h"
#include "gamesystems/objects/objsystem.h"


D20StatusSystem d20StatusSys;

void D20StatusSystem::initRace(objHndl objHnd)
{
	if (objects.IsCritter(objHnd))
	{
		Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
		if (critterSys.IsUndead(objHnd))
		{
			_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionMonsterUndead);
		}

		uint32_t objRace = critterSys.GetRace(objHnd);
		CondStruct ** condStructRace = conds.ConditionArrayRace + objRace;
		_ConditionAddToAttribs_NumArgs0(dispatcher, *condStructRace);

		if (critterSys.IsSubtypeFire(objHnd))
		{
			_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionSubtypeFire);
		}

		if (critterSys.IsOoze(objHnd))
		{
			_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionMonsterOoze);
		}
	}
}

void D20StatusSystem::initClass(objHndl objHnd)
{
	if (objects.IsCritter(objHnd))
	{
		Dispatcher * dispatcher = objects.GetDispatcher(objHnd);

		CondStruct ** condStructClass = conds.ConditionArrayClasses;

		uint32_t stat = stat_level_barbarian;
		for (uint32_t i = 0; i < VANILLA_NUM_CLASSES; i++)
		{
			if (objects.StatLevelGet(objHnd, (Stat)stat) > 0
				&& *condStructClass != nullptr)
			{
				_ConditionAddToAttribs_NumArgs0(dispatcher, *condStructClass);
			};

			condStructClass += 1;
			stat += 1;
		}

		if (objects.StatLevelGet(objHnd, stat_level_cleric) >= 1)
		{
			_D20StatusInitDomains(objHnd);
		}

		if (objects.StatLevelGet(objHnd, stat_level_paladin) >= 3)
		{
			_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionTurnUndead);
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
}

void D20StatusSystem::D20StatusInit(objHndl objHnd)
{
	static int debugLol = 0;
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (dispatcher != nullptr && (uint32_t)dispatcher != 0xFFFFFFFF)
	{
		return;
	}

	dispatcher = objects.dispatch.DispatcherInit(objHnd);

	objects.SetDispatcher(objHnd, (uint32_t)dispatcher);

	objects.dispatch.DispatcherClearPermanentMods(dispatcher);

	if (objects.IsCritter(objHnd))
	{
	//	hooked_print_debug_message("D20Status Init for %s", description.getDisplayName(objHnd));
		initClass(objHnd);

		initRace(objHnd);

		initFeats(objHnd);

	}
	else
	{
		logger->info("Attempted D20Status Init for non-critter {}", description.getDisplayName(objHnd));
		debugLol++;
		if (debugLol % 1000 == 1)
		{
			auto lololol = 0;
		}
	}

	initItemConditions(objHnd);

	d20StatusSys.D20StatusInitFromInternalFields(objHnd, dispatcher);

	d20ObjRegistrySys.Append(objHnd);

	if (*d20Sys.d20EditorMode != 0){ return; }

	if (objects.IsCritter(objHnd))
	{
		if (!critterSys.IsDeadNullDestroyed(objHnd))
		{
			int hpCur = static_cast<int>(objects.StatLevelGet(objHnd, stat_hp_current));
			uint32_t subdualDam = objects.getInt32(objHnd, obj_f_critter_subdual_damage);

			if (hpCur != -65535)
			{
				if (hpCur < 0)
				{
					if (feats.HasFeatCount(objHnd, FEAT_DIEHARD))
					{
						_ConditionAdd_NumArgs0(dispatcher, conds.ConditionDisabled);
					}
					else
					{
						_ConditionAdd_NumArgs0(dispatcher, conds.ConditionUnconscious);
					}
				}

				else
				{
					if (hpCur == 0)
					{
						_ConditionAdd_NumArgs0(dispatcher, conds.ConditionDisabled);
					}
					else if ((int) subdualDam > hpCur)
					{
						_ConditionAdd_NumArgs0(dispatcher, conds.ConditionUnconscious);
					}
				}

			}
		}
	}
	return;
}

void D20StatusSystem::D20StatusRefresh(objHndl objHnd)
{
	Dispatcher *dispatcher; 
	logger->info("Refreshing D20 Status for {}", description.getDisplayName(objHnd));
	dispatcher = objects.GetDispatcher(objHnd);
	if (dispatch.dispatcherValid(dispatcher)){
		dispatch.PackDispatcherIntoObjFields(objHnd, dispatcher);
		dispatch.DispatcherClearPermanentMods(dispatcher);
		initClass(objHnd);
		initRace(objHnd);
		initFeats(objHnd);
		D20StatusInitFromInternalFields(objHnd, dispatcher);	
	}
}

void D20StatusSystem::initDomains(objHndl objHnd)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	uint32_t domain_1 = objects.getInt32(objHnd, obj_f_critter_domain_1);
	uint32_t domain_2 = objects.getInt32(objHnd, obj_f_critter_domain_2);

	if (domain_1)
	{
		CondStruct * condStructDomain1 = *(conds.ConditionArrayDomains + 3 * domain_1);
		uint32_t arg1 = *(conds.ConditionArrayDomainsArg1 + 3 * domain_1);
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
	}
	else
	{
		_ConditionAddToAttribs_NumArgs2(dispatcher, conds.ConditionTurnUndead, 0, 0);
	}
}

void D20StatusSystem::initFeats(objHndl objHnd)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	auto addToDispatcher = [dispatcher](const std::string& condName)
	{
		auto cond = conds.GetByName(condName);
		if (cond){
			_ConditionAddToAttribs_NumArgs0(dispatcher, cond);
		}
	};

	
	feat_enums featList[1000] = {};
	uint32_t numFeats = feats.FeatListElective(objHnd, featList);

	for (uint32_t i = 0; i < numFeats; i++)
	{
		CondStruct * cond;
		uint32_t arg = 0;
		if (_GetCondStructFromFeat(featList[i], &cond, &arg))
		{
			_ConditionAddToAttribs_NumArgs2(dispatcher, cond, featList[i], arg);
		}
	}
	_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionAttackOfOpportunity);
	_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionCastDefensively);
	_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionDealSubdualDamage);
	_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionDealNormalDamage);
	_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionFightDefensively);
	_ConditionAddToAttribs_NumArgs0(dispatcher, (CondStruct*)conds.mConditionDisableAoO);
	_ConditionAddToAttribs_NumArgs0(dispatcher, (CondStruct*)&conds.mCondDisarm);
	_ConditionAddToAttribs_NumArgs0(dispatcher, (CondStruct*)conds.mCondAidAnother);
	//addToDispatcher("Trip Attack Of Opportunity"); // decided to incorporate this in Improved Will to prevent AoOs on AoOs
}

void D20StatusSystem::initItemConditions(objHndl objHnd)
{
	auto obj = objSystem->GetObject(objHnd);

	auto dispatcher = obj->GetDispatcher();
	if (!dispatcher) {
		return;
	}

	if (obj->IsCritter()) {
		objects.dispatch.DispatcherClearItemConds(dispatcher);
		if (!d20Sys.d20Query(objHnd, DK_QUE_Polymorphed))
		{
			uint32_t invenCount = obj->GetInt32(obj_f_critter_inventory_num);
			for (uint32_t i = 0; i < invenCount; i++)
			{
				objHndl objHndItem = obj->GetObjHndl(obj_f_critter_inventory_list_idx, i);
				auto item = objSystem->GetObject(objHndItem);
				uint32_t itemInvLocation = item->GetInt32(obj_f_item_inv_location);
				if (inventory.IsItemEffectingConditions(objHndItem, itemInvLocation)) {
					inventory.sub_100FF500(dispatcher, objHndItem, itemInvLocation);
				}
			}
		}

	}
}

void D20StatusSystem::D20StatusInitFromInternalFields(objHndl objHnd, Dispatcher* dispatcher)
{
	CondStruct *condStruct;
	int condArgs[100];

	auto obj = objSystem->GetObject(objHnd);

	dispatch.DispatcherClearConds(dispatcher);
	int numConds = obj->GetInt32Array(obj_f_conditions).GetSize();
	int numPermMods = obj->GetInt32Array(obj_f_permanent_mods).GetSize();
	for (int i=0,j= 0; i < numConds; i++)
	{
		auto condId = obj->GetInt32(obj_f_conditions, i);
		condStruct = conds.hashmethods.GetCondStruct(condId);
		if (condStruct)
		{
			for (unsigned int k = 0; k < condStruct->numArgs; k++)
			{
				condArgs[k] = obj->GetInt32(obj_f_condition_arg0, j++);
			}
			if (strncmp(condStruct->condName, "Unconscious", 12))
				conds.InitCondFromCondStructAndArgs(dispatcher, condStruct, condArgs);
		}
	}

	
	int troubledIdx = -1; int actualArgCount = 0;
	for ( int i=0, j = 0 ; i < numPermMods; ++i)	{
		auto condId = obj->GetInt32(obj_f_permanent_mods, i);
		condStruct  = conds.hashmethods.GetCondStruct(condId);
		if (condId){
			if (!condStruct || reinterpret_cast<uint32_t>(condStruct->condName) == 0xccCCccCC)	{
			troubledIdx = i;
			logger->debug("Missing condStruct for {}, permanent mod idx: {}/{}, arg idx {}; attempting to recover", description.getDisplayName(objHnd), i, numPermMods, j);
			break;
			}
		} 
		else	{
			logger->debug("Found a null condition on the permanent mods??? Critter: {}", description.getDisplayName(objHnd));
			continue;
		}

		if (condStruct->numArgs > 10)	{
			troubledIdx = true;
			logger->warn("Found condition with unusual number of args!");
			break;
		}
		actualArgCount += condStruct->numArgs;
		for (unsigned int k = 0; k < condStruct->numArgs; k++)
		{
			condArgs[k] = obj->GetInt32(obj_f_permanent_mod_data, j++);
		}
		conds.SetPermanentModArgsFromDataFields(dispatcher, condStruct, condArgs);
	}

	// atempt recovery if necessary
	if (troubledIdx != -1){
		actualArgCount = 0;
		auto numOfArgs = obj->GetInt32Array(obj_f_permanent_mod_data).GetSize();
		std::vector<CondStruct*> condRecover;
		for (int i = 0; i < numPermMods; i++) {

			condStruct = conds.hashmethods.GetCondStruct(obj->GetInt32(obj_f_permanent_mods, i));
			condRecover.push_back(reinterpret_cast<CondStruct*>(condStruct));

			if (i == troubledIdx)
				continue;


			if (!condStruct || reinterpret_cast<uint32_t>(condStruct->condName) == 0xccCCccCC)
			{
				logger->debug("Missing ANOTHER condStruct for {}, permanent mod idx: {}/{}; shit", description.getDisplayName(objHnd), i, numPermMods);
				break;
			}

			if (condStruct->numArgs > 10)
			{
				logger->warn("Found condition with unusual number of args!");
				break;
			}
			
			actualArgCount += condStruct->numArgs;
		}

		int diff = obj->GetInt32Array(obj_f_permanent_mod_data).GetSize() - actualArgCount;

		for (int i = 0, j = 0; i < numPermMods; i++) {
			if (i == troubledIdx)
			{
				j += diff;
				continue;
			}
			condStruct = conds.hashmethods.GetCondStruct(obj->GetInt32(obj_f_permanent_mods, i));
			if (!condStruct || reinterpret_cast<uint32_t>(condStruct->condName) == 0xccCCccCC)
			{
				troubledIdx = i;
				logger->debug("Missing ANOTHER condStruct for {}, permanent mod idx: {}/{}, arg idx {}; Too bad", description.getDisplayName(objHnd), i, numPermMods, j);
				break;
			}

			if (condStruct->numArgs > 10)
			{
				troubledIdx = true;
				logger->warn("Found condition with unusual number of args!");
				break;
			}


			if (i < troubledIdx)
			{
				j += condStruct->numArgs;
				continue;
			}
			for (unsigned int k = 0; k < condStruct->numArgs; k++)
			{
				condArgs[k] = obj->GetInt32(obj_f_permanent_mod_data, j++);
			}
			conds.SetPermanentModArgsFromDataFields(dispatcher, condStruct, condArgs);

		}
	}
	

	conds.DispatcherCondsResetFlag2(dispatcher);


	DispIoD20Signal dispIo;
	dispIo.dispIOType = dispIoTypeSendSignal;
	dispIo.data1 = 0;
	dispIo.data2 = 0;
	dispatch.DispatcherProcessor(dispatcher, dispTypeD20Signal, DK_SIG_Unpack, &dispIo);
}