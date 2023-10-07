#include "stdafx.h"
#include "d20_status.h"
#include "condition.h"
#include "critter.h"
#include "obj.h"
#include "temple_functions.h"
#include "d20_obj_registry.h"
#include "gamesystems/objects/objsystem.h"
#include <gamesystems/gamesystems.h>
#include "d20_race.h"
#include <config\config.h>


D20StatusSystem d20StatusSys;

void D20StatusSystem::initRace(objHndl objHnd)
{
	if (!objects.IsCritter(objHnd))
		return;

	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (critterSys.IsUndead(objHnd)){

		_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionMonsterUndead);
	}

	auto race = critterSys.GetRace(objHnd, false);
	auto raceCond = d20RaceSys.GetRaceCondition(race);
	_ConditionAddToAttribs_NumArgs0(dispatcher, conds.GetByName(raceCond));

		
	auto racialSpells = d20RaceSys.GetSpellLikeAbilities(race);
	auto spellsMemorized = objSystem->GetObject(objHnd)->GetSpellArray(obj_f_critter_spells_memorized_idx);
	for (auto it: racialSpells){
		auto &spell = it.first;
		auto count = 0;
		auto specifiedCount = it.second;

		for (auto i = 0u; i < spellsMemorized.GetSize(); i++) {
			auto &memSpell = spellsMemorized[i];
			if (memSpell.classCode == spell.classCode && memSpell.spellLevel == spell.spellLevel && memSpell.spellEnum == spell.spellEnum && memSpell.padSpellStore == race){
				count++;
				if (count >= specifiedCount){
					break;
				}
			}
		}
		auto spData = spell;
		spData.padSpellStore = race;
		auto spellStoreData = *(int*)(&spData.spellStoreState);
		for (auto i = 0 ; i < specifiedCount - count; i++){
			spellSys.SpellMemorizedAdd(objHnd, spell.spellEnum, spell.classCode, spell.spellLevel, spellStoreData, 0);
		}
	}

	if (critterSys.IsSubtypeFire(objHnd))
	{
		_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionSubtypeFire);
	}

	if (critterSys.IsOoze(objHnd))
	{
		_ConditionAddToAttribs_NumArgs0(dispatcher, conds.ConditionMonsterOoze);
	}
	
}

void D20StatusSystem::initClass(objHndl objHnd){
	if (objects.IsCritter(objHnd))
	{
		Dispatcher * dispatcher = objects.GetDispatcher(objHnd);

		for (auto classCode: d20ClassSys.classEnums){
			if (objects.StatLevelGet(objHnd, (Stat)classCode) <= 0)
				continue;
			auto condStructClass = conds.GetByName(d20StatusSys.classCondMap[(Stat)classCode]);
			if (!condStructClass)
				continue;
			_ConditionAddToAttribs_NumArgs0(dispatcher, condStructClass);
		}
		

		if (objects.StatLevelGet(objHnd, stat_level_cleric) >= 1){
			_D20StatusInitDomains(objHnd);
		}

		if (feats.HasFeatCountByClass(objHnd, FEAT_REBUKE_UNDEAD)) {
			_ConditionAddToAttribs_NumArgs2(dispatcher, conds.GetByName("Turn Undead"), 1, 0);
		} else if (feats.HasFeatCountByClass(objHnd, FEAT_TURN_UNDEAD)) {
			_ConditionAddToAttribs_NumArgs2(dispatcher, conds.GetByName("Turn Undead"), 0, 0);
		}

		if (objects.StatLevelGet(objHnd, stat_level_bard) >= 1){
			_ConditionAddToAttribs_NumArgs0(dispatcher, conds.GetByName("Bardic Music"));
		}
		
		if (objects.getInt32(objHnd, obj_f_critter_school_specialization) & 0xFF){
			_ConditionAddToAttribs_NumArgs0(dispatcher, conds.GetByName("School Specialization"));
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

	if (objects.IsCritter(objHnd)){

		auto psiptsCondStruct = conds.GetByName("Psi Points");
		if (psiptsCondStruct){
			_ConditionAddToAttribs_NumArgs0(dispatcher, psiptsCondStruct); // args will be set from D20StatusInitFromInternalFields if this condition has already been previously applied
		}
		

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

	d20StatusSys.D20StatusInitFromInternalFields(objHnd, dispatcher); // triggers Dispatch for dispTypeConditionAddFromD20StatusInit

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

	initDomain(dispatcher, domain_1);
	initDomain(dispatcher, domain_2);
}

void D20StatusSystem::initDomain(Dispatcher * dispatcher, uint32_t domain)
{
	if (domain) {
		CondStruct * condStructDomain = *(conds.ConditionArrayDomains + 3 * domain);
		uint32_t arg1 = *(conds.ConditionArrayDomainsArg1 + 3 * domain);
		uint32_t arg2 = *(conds.ConditionArrayDomainsArg2 + 3 * domain);
		if (condStructDomain != nullptr)
		{
			//Check if the domain should be retrieved from the condition system 
			if (domain != Domain_Destruction && domain != Domain_Sun) {
				_ConditionAddToAttribs_NumArgs2(dispatcher, condStructDomain, arg1, arg2);
			}
			else {
				_ConditionAddToAttribs_NumArgs2(dispatcher, conds.GetByName(condStructDomain->condName), arg1, arg2);
			}
		}
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
	_ConditionAddToAttribs_NumArgs0(dispatcher, conds.GetByName("Fighting Defensively"));// ConditionFightDefensively);
	_ConditionAddToAttribs_NumArgs0(dispatcher, (CondStruct*)conds.mConditionDisableAoO);
	_ConditionAddToAttribs_NumArgs0(dispatcher, (CondStruct*)&conds.mCondDisarm);
	_ConditionAddToAttribs_NumArgs0(dispatcher, (CondStruct*)conds.mCondAidAnother);
	_ConditionAddToAttribs_NumArgs0(dispatcher, conds.GetByName("Prefer One Handed Wield"));
	//addToDispatcher("Trip Attack Of Opportunity"); // decided to incorporate this in Improved Trip to prevent AoOs on AoOs
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
		auto polyProto = d20Sys.d20Query(objHnd, DK_QUE_Polymorphed);
		auto itemsAreUsable = config.wildShapeUsableItems; // there are also feats that preserve your armor/shield bonuses, todo...
		
		if (!polyProto || itemsAreUsable)
		{
			uint32_t invenCount = obj->GetInt32(obj_f_critter_inventory_num);
			for (uint32_t i = 0; i < invenCount; i++)
			{
				objHndl objHndItem = obj->GetObjHndl(obj_f_critter_inventory_list_idx, i);
				auto item = objSystem->GetObject(objHndItem);
				if (!item) {
					logger->error("InitItemConditions: Critter {} has invalid inventory mismatch between obj_f_critter_inventory_num and obj_f_critter_inventory_list_idx, null item handle at index {} / {}", objHnd, i, invenCount);
					continue;
				}
				uint32_t itemInvLocation = item->GetInt32(obj_f_item_inv_location);
				auto isInEffect = inventory.IsItemEffectingConditions(objHndItem, itemInvLocation);
				if (isInEffect && polyProto && itemsAreUsable) {
					isInEffect = false;
					if (inventory.ItemAccessibleDuringPolymorph(objHndItem))
						isInEffect = true;
					// Todo Wild Armor/Shield
				}
				if (isInEffect) {
					InitFromItemConditionFields(dispatcher, objHndItem, itemInvLocation); // sets args[2] equal to the itemInvLocation
				}
			}
		}
		
		if (polyProto){
			// New! Adds monster conditions (as parsed from protos.tab and stored in the protos objects)
			auto protoHandle = objects.GetProtoHandle(polyProto);
			if (protoHandle) {
				auto protoObj = objSystem->GetObject(protoHandle);
				if (!protoObj) return;

				
				auto condArray = protoObj->GetInt32Array(obj_f_conditions);
				auto condArgArray = protoObj->GetInt32Array(obj_f_condition_arg0);
				auto argIdx = 0u;

				for (auto i = 0u; i < condArray.GetSize(); ++i) {
					int condArgs[64] = { 0, };
					auto monsterCondId = condArray[i]; //conds.GetByName("Tripping Bite");
					auto monsterCond = conds.GetById(monsterCondId); // this should be assured due to check in proto parser for valid conds (protos.cpp)
					if (!monsterCond) continue;
					for (auto j = 0; j < monsterCond->numArgs; ++j) {
						condArgs[j] = condArgArray[argIdx++];
					}

					conds.InitItemCondFromCondStructAndArgs(dispatcher, monsterCond, condArgs);
				}
				
			}
		}

	}
}

/* 0x100FF500*/
void D20StatusSystem::InitFromItemConditionFields(Dispatcher * dispatcher, objHndl item, int invIdx){

	auto itemObj = gameSystems->GetObj().GetObject(item);
	auto &itemConds = itemObj->GetInt32Array(obj_f_item_pad_wielder_condition_array);
	auto itemArgs = itemObj->GetInt32Array(obj_f_item_pad_wielder_argument_array);
	int condArgs[64];

	auto argIdx = 0u;
	for (auto i = 0u; i < itemConds.GetSize(); i++){
		auto condId = itemConds[i];
		auto condStruct = conds.GetById(condId);
		if (!condStruct){
			logger->warn("Item condition not found!");
			continue;
		}

		for (auto j=0u; j<condStruct->numArgs; j++)	{
			condArgs[j] = itemArgs[argIdx++];
		}
		condArgs[2] = invIdx;

		conds.InitItemCondFromCondStructAndArgs(dispatcher, condStruct, condArgs);

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

	// attempt recovery if necessary
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