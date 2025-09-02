#include "stdafx.h"
#include "dispatcher.h"
#include "d20.h"
#include "temple_functions.h"
#include "obj.h"
#include "condition.h"
#include "bonus.h"
#include "action_sequence.h"
#include "turn_based.h"
#include "damage.h"
#include "util/fixes.h"
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/gamesystems.h"
#include "ui/ui_party.h"
#include "python/python_dispatcher.h"
#include <critter.h>

// Dispatcher System Function Replacements
class DispatcherReplacements : public TempleFix {
public:

	static int Dispatch54AoE(objHndl, DispIO *, D20DispatcherKey);

	void apply() override {
		logger->info("Replacing basic Dispatcher functions");
		
		replaceFunction(0x1004D700, _DispIoCheckIoType1);
		replaceFunction(0x1004D720, _DispIoCheckIoType2);
		replaceFunction(0x1004D760, _DispIoCheckIoType4);
		replaceFunction(0x1004D780, _DispIoCheckIoType5);
		replaceFunction(0x1004D7A0, _DispIoCheckIoType6);
		replaceFunction(0x1004D7C0, _DispIoCheckIoType7);
		replaceFunction(0x1004D7E0, _DispIoCheckIoType8);
		replaceFunction(0x1004D800, _DispIoCheckIoType9);
		replaceFunction(0x1004D820, _DispIoCheckIoType10);
		replaceFunction(0x1004D840, _DispIoCheckIoType11);
		replaceFunction(0x1004D860, _DispIoCheckIoType12);
		replaceFunction(0x1004D8A0, _DispIoCheckIoType14);
		replaceFunction(0x1004F780, _PackDispatcherIntoObjFields);
		
		replaceFunction(0x100E1E30, _DispatcherRemoveSubDispNodes);
		replaceFunction(0x100E2400, _DispatcherClearField);
		replaceFunction(0x100E2720, _DispatcherClearPermanentMods);
		replaceFunction(0x100E2740, _DispatcherClearItemConds);
		replaceFunction(0x100E2760, _DispatcherClearConds);
		replaceFunction(0x100E2120, _DispatcherProcessor);
		replaceFunction(0x100E1F10, _DispatcherInit);
		replaceFunction(0x1004DBA0, DispIOType21Init);
		replaceFunction(0x1004D3A0, _Dispatch62);
		replaceFunction(0x1004D440, _Dispatch63);

		replaceFunction(0x1004DEC0, _DispatchAttackBonus);
		replaceFunction(0x1004E040, _DispatchDamage);
		replaceFunction(0x1004E790, _dispatchTurnBasedStatusInit); 
		replaceFunction<int(__cdecl)(objHndl , SavingThrowType , DispIoSavingThrow*)>(0x1004E870, [](objHndl handle, SavingThrowType saveType, DispIoSavingThrow * evtObj) ->int{
			return dispatch.Dispatch13SavingThrow(handle, saveType, evtObj);
			});

		replaceFunction(0x1004ED70, _dispatch1ESkillLevel); 

		replaceFunction<int(__cdecl)(D20Actn*, TurnBasedStatus*, enum_disp_type)>(0x1004E0D0, [](D20Actn* d20a, TurnBasedStatus* tbStat, enum_disp_type dispType)
		{
			return dispatch.DispatchD20ActionCheck(d20a, tbStat, dispType);
		});
		
		replaceFunction<void(__cdecl)(objHndl, int)>(0x1004E730, [](objHndl handle, int numRounds)->void {
			return dispatch.Dispatch48BeginRound(handle, numRounds);
		});

		replaceFunction(0x1004D360, Dispatch54AoE);

		replaceFunction<BOOL(__cdecl)(objHndl,enum_disp_type, D20DispatcherKey, DispIO*)>(0x1004CDB0, [](objHndl item, enum_disp_type dispType, D20DispatcherKey dispKey, DispIO* evtObj) {
			dispatch.DispatchForItem(item, dispType, dispKey, evtObj);
			return 0;
		});
		
		replaceFunction<BOOL(__cdecl)(objHndl, D20DispatcherKey)>(0x1004CEB0, [](objHndl item, D20DispatcherKey dispKey) {
			return dispatch.DispatchItemQuery(item, dispKey);
		});
			
	}
} dispatcherReplacements;

int DispatcherReplacements::Dispatch54AoE(objHndl handle, DispIO* evtObj, D20DispatcherKey dispKey){
	if (!handle) return 0;
	auto obj = objSystem->GetObject(handle);
	auto dispatcher = obj->GetDispatcher();
	if (!dispatcher->IsValid())
		return 0;
	dispatcher->Process(dispTypeObjectEvent, dispKey, evtObj);
	return 0;
}


#pragma region Dispatcher System Implementation
DispatcherSystem dispatch;

void DispatcherSystem::DispatcherProcessor(Dispatcher* dispatcher, enum_disp_type dispType, uint32_t dispKey, DispIO* dispIO)
{
	_DispatcherProcessor(dispatcher, dispType, (D20DispatcherKey)dispKey, dispIO);
}

void DispatcherSystem::DispatcherProcessorForItems(CondStruct* condStruct, int condArgs[64], enum_disp_type dispType,
	D20DispatcherKey key, DispIO* dispIo){
	
	auto sdd = condStruct->subDispDefs;
	auto numArgs = condStruct->numArgs;
	CondNode condNode(condStruct);
	if (numArgs > 0){
		memcpy(&condNode.args, condArgs, sizeof(int) * numArgs); // was qmemcpy originally
	}
	
	while (sdd->dispType != enum_disp_type::dispType0){
		if (sdd->dispType != dispType){
			sdd++;
			continue;
		}
		if (sdd->dispKey == key || dispType == dispType0 /* shouldn't happen...*/ 
			|| sdd->dispKey == DK_NONE){
			SubDispNode sdn;
			sdn.subDispDef = sdd;
			sdn.condNode = &condNode;
			sdd->dispCallback(&sdn, objHndl::null, dispType, key, dispIo);
		}
		sdd++;
	}

}

Dispatcher * DispatcherSystem::DispatcherInit(objHndl objHnd)
{
	return _DispatcherInit(objHnd);
}

bool DispatcherSystem::dispatcherValid(Dispatcher* dispatcher)
{
	return (dispatcher != nullptr && dispatcher != (Dispatcher*)-1);
}

void  DispatcherSystem::DispatcherClearField(Dispatcher * dispatcher, CondNode ** dispCondList)
{
	_DispatcherClearField(dispatcher, dispCondList);
}

void  DispatcherSystem::DispatcherClearPermanentMods(Dispatcher * dispatcher)
{
	_DispatcherClearField(dispatcher, &dispatcher->permanentMods);
}

void  DispatcherSystem::DispatcherClearItemConds(Dispatcher * dispatcher)
{
	_DispatcherClearField(dispatcher, &dispatcher->itemConds);
}

void  DispatcherSystem::DispatcherClearConds(Dispatcher *dispatcher)
{
	_DispatcherClearConds(dispatcher);
}

int DispatcherSystem::DispatchForCritter(objHndl handle, DispIoBonusList * eventObj, enum_disp_type dispType, D20DispatcherKey dispKey)
{
	if (!handle)
		return 0;
	auto obj = gameSystems->GetObj().GetObject(handle);
	if (!obj->IsCritter())
		return 0;
	auto dispatcher = obj->GetDispatcher();
	if (!dispatcher->IsValid())
		return 0;
	
	DispIoBonusList * eventObjUsed = eventObj;
	DispIoBonusList eventObjLocal;
	if (!eventObj){
		eventObjUsed = &eventObjLocal;
	}
	
	DispatcherProcessor(dispatcher, dispType, dispKey, eventObjUsed);
	return eventObjUsed->bonlist.GetEffectiveBonusSum();
}

void DispatcherSystem::DispatchForItem(objHndl item, enum_disp_type dispType, D20DispatcherKey key, DispIO * dispIo){
	auto argarrayCount = 0;
	auto itemObj = objSystem->GetObject(item);
	if (!itemObj){
		return;
	}
	auto condArray = itemObj->GetInt32Array(obj_f_item_pad_wielder_condition_array);
	auto argArray = itemObj->GetInt32Array(obj_f_item_pad_wielder_argument_array);
	auto condCount = condArray.GetSize();
	
	for (auto i = 0u; i < condCount; i++){
		auto condId = condArray[i];
		auto cond = conds.GetById(condId);
		if (!cond){
			continue;
		}

		int condArgs[64] = {0,};
		for (auto j=0u; j < cond->numArgs; j++){
			condArgs[j] = argArray[argarrayCount++];
		}
		condArgs[2] = -1;
		DispatcherProcessorForItems(cond, condArgs, dispType, key, dispIo);
	}
}

// A complicated dispatcher for wearable items.
//
// If the item is worn, its item conditions will be among the conditions for its
// parent creature, so we can dispatch against the creature. If not, the item
// conditions can only be checked by doing an item dispatch.
//
// This is common code for attributes like armor check penalty that might want
// to work this way. At the moment, only item conditions (like masterwork or
// nimbleness) influence the penalty, so we could just use an item dispatch.
// This more complicated logic tries to allow for e.g. feats that would
// influence the penalty for specific characters.
void DispatcherSystem::DispatchForWearable(objHndl item, enum_disp_type dispType, D20DispatcherKey key, DispIO *dispIo) {
	auto parent = gameSystems->GetObj().GetObject(inventory.GetParent(item));
	auto critterParent = parent != nullptr && parent->IsCritter();
	auto loc = inventory.GetInventoryLocation(item);

	if (critterParent) {
		auto dispatcher = parent->GetDispatcher();
		if (dispatcher->IsValid()) {
			DispatcherProcessor(dispatcher, dispType, key, dispIo);
		} else {
			// fall back to item dispatch
			critterParent = false;
		}
	}

	// if there is no parent critter, or the item isn't worn, we
	// need to do item dispatch to incorporate item conditions
	if (!critterParent || !inventory.IsInvIdxWorn(loc)) {
		dispatch.DispatchForItem(item, dispType, key, dispIo);
	}
}

int DispatcherSystem::Dispatch10AbilityScoreLevelGet(objHndl handle, Stat stat, DispIoBonusList * dispIo){
	return dispatch.DispatchForCritter(handle, dispIo, dispTypeAbilityScoreLevel, (D20DispatcherKey)(stat+1));
}

int32_t DispatcherSystem::dispatch1ESkillLevel(objHndl objHnd, SkillEnum skill, BonusList* bonOut, objHndl objHnd2, int32_t flag)
{
	DispIoObjBonus dispIO;
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (!dispatcherValid(dispatcher)) return 0;

	dispIO.dispIOType = dispIoTypeObjBonus;
	dispIO.flags = flag;
	dispIO.bonOut = bonOut;
	dispIO.obj = objHnd2;
	if (!bonOut)
	{
		dispIO.bonOut = &dispIO.bonlist;
		bonusSys.initBonusList(&dispIO.bonlist);
	}
	DispatcherProcessor(dispatcher, dispTypeSkillLevel, skill + 20, &dispIO);
	return bonusSys.getOverallBonus(dispIO.bonOut);
	
}

float DispatcherSystem::Dispatch29hGetMoveSpeed(objHndl objHnd, DispIoMoveSpeed *dispIoIn) // including modifiers like armor restirction
{
	float result = 30.0;

	auto dispatcher = gameSystems->GetObj().GetObject(objHnd)->GetDispatcher();
	if (!dispatcher->IsValid())
		return result;


	DispIoMoveSpeed dispIo;
	BonusList bonlist;
	if (dispIoIn) {
		dispIo.bonlist = dispIoIn->bonlist;
	}
	else {
		dispIo.bonlist = &bonlist;
	}

	dispatch.Dispatch40GetBaseMoveSpeed(objHnd, &dispIo);
	dispatch.DispatcherProcessor(dispatcher, dispTypeGetMoveSpeed, DK_NONE, &dispIo);
	auto moveTot = dispIo.bonlist->GetEffectiveBonusSum();

	if (dispIo.factor < 0)
		dispIo.factor = 0;
	if (moveTot < 0)
		moveTot = 0;

	result = moveTot * dispIo.factor;

	return result;
}

float DispatcherSystem::Dispatch40GetBaseMoveSpeed(objHndl objHnd, DispIoMoveSpeed *dispIoIn)
{
	float result = 30.0;

	auto dispatcher = gameSystems->GetObj().GetObject(objHnd)->GetDispatcher();
	if (!dispatcher->IsValid())
		return result;


	DispIoMoveSpeed dispIo;
	BonusList bonlist;
	if (dispIoIn) {
		dispIo.bonlist = dispIoIn->bonlist;
	}
	else {
		dispIo.bonlist = &bonlist;
	}

	dispatch.DispatcherProcessor(dispatcher, dispTypeGetMoveSpeedBase, DK_NONE, &dispIo);
	auto bonResult = dispIo.bonlist->GetEffectiveBonusSum();

	if (dispIoIn) {
		dispIoIn->factor = dispIo.factor;
	}
	
	result = bonResult * dispIo.factor;

	return result;
}



void DispatcherSystem::dispIOTurnBasedStatusInit(DispIOTurnBasedStatus* dispIOtbStat)
{
	dispIOtbStat->dispIOType = dispIOTypeTurnBasedStatus;
	dispIOtbStat->tbStatus = nullptr;
}


void DispatcherSystem::dispatchTurnBasedStatusInit(objHndl objHnd, DispIOTurnBasedStatus* dispIOtB)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (dispatcherValid(dispatcher))
	{
		DispatcherProcessor(dispatcher, dispTypeTurnBasedStatusInit, 0, dispIOtB);
		if (dispIOtB->tbStatus)
		{
			if (dispIOtB->tbStatus->hourglassState > 0)
			{
				d20Sys.d20SendSignal(objHnd, DK_SIG_BeginTurn, 0, 0);
			}
		}
	}
}

int DispatcherSystem::DispatchSavingThrow(objHndl handle, DispIoSavingThrow * evtObj, enum_disp_type dispType, D20DispatcherKey d20DispatcherKey)
{
	auto obj = objSystem->GetObject(handle);
	if (!obj || !obj->IsCritter())
		return 0;
	
	auto dispatcher = obj->GetDispatcher();
	if (!dispatcherValid(dispatcher))
		return 0;

	if (!evtObj){
		DispIoSavingThrow evtObjLocal;
		dispatcher->Process(dispType, d20DispatcherKey, &evtObjLocal);
		return evtObjLocal.bonlist.GetEffectiveBonusSum();
	}

	dispatcher->Process(dispType, d20DispatcherKey, evtObj);
	return evtObj->bonlist.GetEffectiveBonusSum();
}

int DispatcherSystem::Dispatch13SavingThrow(objHndl handle, SavingThrowType saveType, DispIoSavingThrow * evtObj)
{
	return DispatchSavingThrow(handle, evtObj, dispTypeSaveThrowLevel, (D20DispatcherKey)((int)saveType + D20DispatcherKey::DK_SAVE_FORTITUDE) );
}

int DispatcherSystem::Dispatch14SavingThrowMod(objHndl handle, SavingThrowType saveType, DispIoSavingThrow * evtObj)
{
	return DispatchSavingThrow(handle, evtObj, dispTypeSaveThrowSpellResistanceBonus, (D20DispatcherKey)((int)saveType + D20DispatcherKey::DK_SAVE_FORTITUDE));
}

int DispatcherSystem::Dispatch44FinalSaveThrow(objHndl handle, SavingThrowType saveType, DispIoSavingThrow* evtObj){
	return DispatchSavingThrow(handle, evtObj, dispTypeCountersongSaveThrow, (D20DispatcherKey)((int)saveType + D20DispatcherKey::DK_SAVE_FORTITUDE));
}


DispIoCondStruct* DispatcherSystem::DispIoCheckIoType1(DispIoCondStruct* dispIo)
{
	if (dispIo->dispIOType != dispIoTypeCondStruct) return nullptr;
	return dispIo;
}

DispIoCondStruct* DispatcherSystem::DispIoCheckIoType1(DispIO* dispIo)
{
	return DispIoCheckIoType1((DispIoCondStruct*)dispIo);
}

DispIoBonusList* DispatcherSystem::DispIoCheckIoType2(DispIoBonusList* dispIo)
{
	if (dispIo->dispIOType != dispIOTypeBonusList) return nullptr;
	return dispIo;
}

DispIoBonusList* DispatcherSystem::DispIoCheckIoType2(DispIO* dispIoBonusList)
{
	return DispIoCheckIoType2( (DispIoBonusList*) dispIoBonusList);
}

DispIoSavingThrow* DispatcherSystem::DispIoCheckIoType3(DispIoSavingThrow* dispIo)
{
	if (dispIo->dispIOType != dispIOTypeSavingThrow) return nullptr;
	return dispIo;
}

DispIoSavingThrow* DispatcherSystem::DispIoCheckIoType3(DispIO* dispIo)
{
	return DispIoCheckIoType3((DispIoSavingThrow*)dispIo);
}

DispIoDamage* DispatcherSystem::DispIoCheckIoType4(DispIoDamage* dispIo)
{
	if (dispIo->dispIOType != dispIOTypeDamage) return nullptr;
	return dispIo;
}

DispIoDamage* DispatcherSystem::DispIoCheckIoType4(DispIO* dispIo)
{
	return DispIoCheckIoType4((DispIoDamage*)dispIo);
}

DispIoAttackBonus* DispatcherSystem::DispIoCheckIoType5(DispIoAttackBonus* dispIo)
{
	if (dispIo->dispIOType != dispIOTypeAttackBonus) return nullptr;
	return dispIo;
}

DispIoAttackBonus* DispatcherSystem::DispIoCheckIoType5(DispIO* dispIo)
{
	return DispIoCheckIoType5((DispIoAttackBonus*)dispIo);
}

DispIoD20Signal* DispatcherSystem::DispIoCheckIoType6(DispIoD20Signal* dispIo)
{
	if (dispIo->dispIOType != dispIoTypeSendSignal) return nullptr;
	return dispIo;
}

DispIoD20Signal* DispatcherSystem::DispIoCheckIoType6(DispIO* dispIo)
{
	return DispIoCheckIoType6((DispIoD20Signal*)dispIo);
}

DispIoD20Query* DispatcherSystem::DispIoCheckIoType7(DispIoD20Query* dispIo)
{
	if (dispIo->dispIOType != dispIOTypeQuery) return nullptr;
	return dispIo;
}

DispIoD20Query* DispatcherSystem::DispIoCheckIoType7(DispIO* dispIo)
{
	return DispIoCheckIoType7((DispIoD20Query*)dispIo);
}

DispIOTurnBasedStatus* DispatcherSystem::DispIoCheckIoType8(DispIOTurnBasedStatus* dispIo)
{
	if (dispIo->dispIOType != dispIOTypeTurnBasedStatus) return nullptr;
	return dispIo;
}

DispIOTurnBasedStatus* DispatcherSystem::DispIoCheckIoType8(DispIO* dispIo)
{
	return DispIoCheckIoType8((DispIOTurnBasedStatus*)dispIo);
}

DispIoTooltip* DispatcherSystem::DispIoCheckIoType9(DispIoTooltip* dispIo)
{
	if (dispIo->dispIOType != dispIoTypeTooltip) return nullptr;
	return dispIo;
}

DispIoTooltip* DispatcherSystem::DispIoCheckIoType9(DispIO* dispIo)
{
	if (dispIo->dispIOType != dispIoTypeTooltip) return nullptr;
	return static_cast<DispIoTooltip*>(dispIo);
}

DispIoObjBonus* DispatcherSystem::DispIoCheckIoType10(DispIoObjBonus* dispIo)
{
	if (dispIo->dispIOType != dispIoTypeObjBonus) return nullptr;
	return dispIo;
}

DispIoObjBonus* DispatcherSystem::DispIoCheckIoType10(DispIO* dispIo)
{
	if (dispIo->dispIOType != dispIoTypeObjBonus) return nullptr;
	return static_cast<DispIoObjBonus*>(dispIo);
}

DispIoDispelCheck* DispatcherSystem::DispIOCheckIoType11(DispIoDispelCheck* dispIo)
{
	if (dispIo->dispIOType != dispIOTypeDispelCheck) return nullptr;
	return dispIo;
}

DispIoD20ActionTurnBased* DispatcherSystem::DispIoCheckIoType12(DispIoD20ActionTurnBased* dispIo)
{
	if (dispIo->dispIOType != dispIOTypeD20ActionTurnBased) return nullptr;
	return dispIo;
}

DispIoD20ActionTurnBased* DispatcherSystem::DispIoCheckIoType12(DispIO* dispIo)
{
	return DispIoCheckIoType12((DispIoD20ActionTurnBased*)dispIo);
}

DispIoMoveSpeed * DispatcherSystem::DispIoCheckIoType13(DispIoMoveSpeed* dispIo)
{
	if (dispIo->dispIOType != dispIOTypeMoveSpeed) return nullptr;
	return dispIo;
}

DispIoMoveSpeed* DispatcherSystem::DispIoCheckIoType13(DispIO* dispIo)
{
	return DispIoCheckIoType13((DispIoMoveSpeed*)dispIo);
}

void DispatcherSystem::Dispatch48BeginRound(objHndl obj, int numRounds) const
{
	auto dispatcher = gameSystems->GetObj().GetObject(obj)->GetDispatcher();
	if (dispatcher->IsValid()) {
		DispIoD20Signal dispIo;
		(int64_t&)(dispIo.data1) = numRounds;
		dispatch.DispatcherProcessor(dispatcher, dispTypeBeginRound, DK_NONE, &dispIo);
		spellSys.SpellBeginRound(obj);
	}
}

bool DispatcherSystem::Dispatch64ImmunityCheck(objHndl handle, DispIoImmunity* dispIo)
{
	auto dispatcher = gameSystems->GetObj().GetObject(handle)->GetDispatcher();
	if (dispatcher->IsValid())
	{
		DispatcherProcessor(dispatcher, dispTypeSpellImmunityCheck, 0, dispIo);
		return dispIo->returnVal != 0;
	}
	
	return 0;
}

void DispatcherSystem::Dispatch68ItemRemove(objHndl handle){
	if (!handle)
		return;
	auto dispatcher = objSystem->GetObject(handle)->GetDispatcher();
	if (dispatcher->IsValid()){
		DispatcherProcessor(dispatcher, dispTypeItemForceRemove, 0, nullptr);
	}
}

DispIoObjEvent* DispatcherSystem::DispIoCheckIoType17(DispIO* dispIo)
{
	if (dispIo->dispIOType != dispIoTypeObjEvent)
		return nullptr;
	return static_cast<DispIoObjEvent*>(dispIo);
}

DispIoAttackDice* DispatcherSystem::DispIoCheckIoType20(DispIO* dispIo)
{
	if (dispIo->dispIOType != dispIOType20)
		return nullptr;
	return static_cast<DispIoAttackDice*>(dispIo);
}

DispIoImmunity* DispatcherSystem::DispIoCheckIoType23(DispIoImmunity* dispIo)
{
	if (dispIo->dispIOType != dispIOTypeImmunityHandler) return nullptr;
	return dispIo;
}

DispIoImmunity* DispatcherSystem::DispIoCheckIoType23(DispIO* dispIo)
{
	return DispIoCheckIoType23((DispIoImmunity*)dispIo);
}

DispIoEffectTooltip* DispatcherSystem::DispIoCheckIoType24(DispIoEffectTooltip* dispIo)
{
	if (dispIo->dispIOType != dispIOTypeEffectTooltip) return nullptr;
	return dispIo;
}

DispIoEffectTooltip* DispatcherSystem::DispIoCheckIoType24(DispIO* dispIo)
{
	return DispIoCheckIoType24((DispIoEffectTooltip*)dispIo);
}

DispIOBonusListAndSpellEntry* DispatcherSystem::DispIOCheckIoType14(DispIOBonusListAndSpellEntry* dispIo)
{
	if (dispIo->dispIOType != dispIoTypeBonusListAndSpellEntry) return nullptr;
	return dispIo;
}

void DispatcherSystem::PackDispatcherIntoObjFields(objHndl objHnd, Dispatcher* dispatcher)
{
	int numArgs;
	int k;
	int hashkey;
	int numConds;
	int condArgs[64] = {0,};
	const char* name = description.getDisplayName(objHnd);

	auto obj = objSystem->GetObject(objHnd);

	k = 0;
	d20Sys.d20SendSignal(objHnd, DK_SIG_Pack, 0, 0);
	obj->ResetField(obj_f_conditions);
	obj->ResetField(obj_f_condition_arg0);
	obj->ClearArray(obj_f_permanent_mods);
	obj->ClearArray(obj_f_permanent_mod_data);
	numConds = conds.GetActiveCondsNum(dispatcher);
	for (int i = 0; i < numConds; i++)
	{
		numArgs = conds.ConditionsExtractInfo(dispatcher, i, &hashkey, condArgs);
		obj->SetInt32(obj_f_conditions, i, hashkey);
		for (int j = 0; j < numArgs; ++k) {
			obj->SetInt32(obj_f_condition_arg0, k, condArgs[j++]);
		}
			
	}
	k = 0;
	numConds = conds.GetPermanentModsAndItemCondCount(dispatcher);
	for (int i = 0; i < numConds; i++)
	{
		numArgs = conds.PermanentAndItemModsExtractInfo(dispatcher, i, &hashkey, condArgs);
		if (hashkey)
		{
			obj->SetInt32(obj_f_permanent_mods, i, hashkey);
			for (int j = 0; j < numArgs; ++k) {
				obj->SetInt32(obj_f_permanent_mod_data, k, condArgs[j++]);
			}
				
		}
	}
}

int DispatcherSystem::DispatchDispelCheck(objHndl handle, int spellId, int flags, int returnValue)
{
	DispIoDispelCheck dispelIOCheck;
	dispelIOCheck.dispIOType = dispIOTypeDispelCheck;
	dispelIOCheck.flags = flags;
	dispelIOCheck.returnVal = returnValue;
	dispelIOCheck.spellId = spellId;
	auto dispatcher = objects.GetDispatcher(handle);
	if (dispatch.dispatcherValid(dispatcher))
	{
		DispatcherProcessor(dispatcher, dispTypeDispelCheck, 0, &dispelIOCheck);
	}

	return 0;
}

int DispatcherSystem::DispatchAttackBonus(objHndl objHnd, objHndl victim, DispIoAttackBonus* dispIo, enum_disp_type dispType, int key)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (!dispatcherValid(dispatcher))
		return 0;
	DispIoAttackBonus dispIoLocal;
	DispIoAttackBonus * dispIoToUse = dispIo;
	if (!dispIo)
	{
		dispIoLocal.attackPacket.victim = victim;
		dispIoLocal.attackPacket.attacker = objHnd;
		dispIoLocal.attackPacket.d20ActnType = (D20ActionType)dispType; //  that's the original code, jones...
		dispIoLocal.attackPacket.ammoItem = 0i64;
		dispIoLocal.attackPacket.weaponUsed = 0i64;
		dispIoToUse = &dispIoLocal;
	}
	DispatcherProcessor(dispatcher, dispType, key, dispIoToUse);
	return bonusSys.getOverallBonus(&dispIoToUse->bonlist);

}

int DispatcherSystem::DispatchToHitBonusBase(objHndl objHndCaller, DispIoAttackBonus* dispIo){
	int key = 0;
	if (dispIo)
	{
		key = dispIo->attackPacket.dispKey;
	}
	return DispatchAttackBonus(objHndCaller, objHndl::null, dispIo, dispTypeToHitBonusBase, key);
}

int DispatcherSystem::DispatchGetSizeCategory(objHndl handle, bool base)
{
	auto obj = objSystem->GetObject(handle);
	if (!obj) return 0;

	Dispatcher * dispatcher = obj->GetDispatcher();
	if (!dispatcherValid(dispatcher)) return 0;

	DispIoD20Query dispIo;
	dispIo.return_val = obj->GetInt32(obj_f_size);

	// Most size modifying spells (e.g. Enlarge Person) set these to 1 to avoid
	// stacking, so initializing to 1 will get a 'base' value. data1 tracks
	// enlargement, data2 tracks reduction.
	if (base) {
		dispIo.data1 = 1;
		dispIo.data2 = 1;
	} else {
		dispIo.data1 = 0;
		dispIo.data2 = 0;
	}
	
	if (objects.IsCritter(handle)) {
		auto polymorphHandle = critterSys.GetPolymorphedHandle(handle);
		if (polymorphHandle) {
			auto polyObj = objSystem->GetObject(polymorphHandle);
			if (polyObj) {
				dispIo.return_val = polyObj->GetInt32(obj_f_size);
			}
		}
	}

	DispatcherProcessor( dispatcher, dispTypeGetSizeCategory,	0, &dispIo);
	return dispIo.return_val;
	
}

void DispatcherSystem::DispatchConditionRemove(Dispatcher* dispatcher, CondNode* cond)
{
	for (auto subDispNode = dispatcher->subDispNodes[dispTypeConditionRemove]; subDispNode; subDispNode = subDispNode->next)
	{
		if (subDispNode->subDispDef->dispKey == 0){
			if (!(subDispNode->condNode->flags & 1) && (subDispNode->condNode == cond)){
				DispatcherCallbackArgs dca;
				dca.dispIO = nullptr;
				dca.dispType = dispTypeConditionRemove;
				dca.dispKey = 0;
				dca.objHndCaller = dispatcher->objHnd;
				dca.subDispNode = subDispNode;
				subDispNode->subDispDef->dispCallback(dca.subDispNode, dca.objHndCaller, dca.dispType, dca.dispKey, dca.dispIO);
			}	
		}
	}

	for (auto subDispNode = dispatcher->subDispNodes[dispTypeConditionRemove2]; subDispNode; subDispNode = subDispNode->next)
	{
		if (subDispNode->subDispDef->dispKey == 0){
			if (!(subDispNode->condNode->flags & 1) && (subDispNode->condNode == cond)) {
				DispatcherCallbackArgs dca;
				dca.dispIO = nullptr;
				dca.dispType = dispTypeConditionRemove2;
				dca.dispKey = 0;
				dca.objHndCaller = dispatcher->objHnd;
				dca.subDispNode = subDispNode;
				subDispNode->subDispDef->dispCallback(dca.subDispNode, dca.objHndCaller, dca.dispType, dca.dispKey, dca.dispIO);
			}
		}
	}
	cond->flags |= 1;
}

void DispatcherSystem::DispatchMetaMagicModify(objHndl obj, MetaMagicData& mmData, unsigned char spellLevel, uint16_t spellEnum, uint32_t spellClass)
{
	auto _dispatcher = objects.GetDispatcher(obj);
	if (!dispatch.dispatcherValid(_dispatcher)) return;
	EvtObjMetaMagic dispIo;
	dispIo.mmData = mmData;
	dispIo.spellEnum = spellEnum;
	dispIo.spellLevel = spellLevel;
	dispIo.spellClass = spellClass;
	DispatcherProcessor(_dispatcher, dispTypeMetaMagicMod, 0, &dispIo);
	mmData = dispIo.mmData;
}

double DispatcherSystem::DispatchRangeBonus(objHndl obj, objHndl weaponUsed)
{
	auto _dispatcher = objects.GetDispatcher(obj);
	if (!dispatch.dispatcherValid(_dispatcher)) return 0;
	EvtObjRangeIncrementBonus dispIo;
	dispIo.weaponUsed = weaponUsed;
	DispatcherProcessor(_dispatcher, dispRangeIncrementBonus, 0, &dispIo);
	return dispIo.rangeBonus;
}

void DispatcherSystem::DispatchSpecialAttack(objHndl obj, int attack, objHndl target)
{
	auto _dispatcher = objects.GetDispatcher(obj);
	if (!dispatch.dispatcherValid(_dispatcher)) return;
	EvtObjSpecialAttack dispIo;
	dispIo.target = target;
	dispIo.attack = static_cast<EvtObjSpecialAttack::AttackType>(attack);
	DispatcherProcessor(_dispatcher, dispTypeSpecialAttack, 0, &dispIo);
}

void DispatcherSystem::DispatchSpellResistanceCasterLevelCheck(objHndl caster, objHndl target, BonusList *bonusList, SpellPacketBody *spellPkt)
{
	auto _dispatcher = objects.GetDispatcher(caster);
	if (!dispatch.dispatcherValid(_dispatcher)) return;
	EvtObjSpellTargetBonus dispIo;
	dispIo.spellPkt = spellPkt;
	dispIo.target = target;
	dispIo.bonusList = bonusList;
	DispatcherProcessor(_dispatcher, dispTypeSpellResistanceCasterLevelCheck, 0, &dispIo);
}

void DispatcherSystem::DispatchTargetSpellDCBonus(objHndl caster, objHndl target, BonusList *bonusList, SpellPacketBody*spellPkt)
{
	auto _dispatcher = objects.GetDispatcher(caster);
	if (!dispatch.dispatcherValid(_dispatcher)) return;
	EvtObjSpellTargetBonus dispIo;
	dispIo.spellPkt = spellPkt;
	dispIo.target = target;
	dispIo.bonusList = bonusList;
	DispatcherProcessor(_dispatcher, dispTypeTargetSpellDCBonus, 0, &dispIo);
}

bool DispatcherSystem::DispatchIgnoreDruidOathCheck(objHndl character, objHndl item)
{
	auto _dispatcher = objects.GetDispatcher(character);
	if (!dispatch.dispatcherValid(_dispatcher)) return false;
	EvtIgnoreDruidOathCheck dispIo;
	dispIo.item = item;
	DispatcherProcessor(_dispatcher, dispTypeIgnoreDruidOathCheck, 0, &dispIo);
	return dispIo.ignoreDruidOath;
}

unsigned DispatcherSystem::Dispatch35CasterLevelModify(objHndl obj, SpellPacketBody* spellPkt)
{
	auto _dispatcher = objects.GetDispatcher(obj);
	if (!dispatch.dispatcherValid(_dispatcher))
		return 0;
	DispIoD20Query dispIo;
	dispIo.return_val = spellPkt->casterLevel;
	dispIo.data1 = reinterpret_cast<uint32_t>(spellPkt);
	dispIo.data2 = 0;
	DispatcherProcessor(_dispatcher, dispTypeBaseCasterLevelMod, 0, &dispIo);
	//spellPkt->casterLevel = dispIo.return_val; // this caused bugs!
	return dispIo.return_val;
}

int DispatcherSystem::DispatchSpellListLevelExtension(objHndl handle, Stat casterClass)
{
	auto objDispatcher = gameSystems->GetObj().GetObject(handle)->GetDispatcher();
	if (!objDispatcher->IsValid())
		return 0;

	EvtObjSpellCaster evtObj;
	evtObj.handle = handle;
	evtObj.arg0 = casterClass;
	DispatcherProcessor(objDispatcher, dispTypeSpellListExtension, DK_NONE, &evtObj);
	return evtObj.bonlist.GetEffectiveBonusSum();
}

int DispatcherSystem::DispatchSpellsPerDay(objHndl handle, Stat casterClass, int spellLevel, int effectiveLvl)
{
	auto objDispatcher = gameSystems->GetObj().GetObject(handle)->GetDispatcher();
	if (!objDispatcher->IsValid())
		return 0;

	DispIoSpellsPerDay evtObj;
	BonusList bonlist;
	evtObj.bonList = &bonlist;
	evtObj.classCode = casterClass;
	evtObj.spellLvl = spellLevel;
	evtObj.casterEffLvl = effectiveLvl;
	DispatcherProcessor(objDispatcher, dispType58SpellsPerDayMod, DK_NONE, &evtObj);
	return evtObj.bonList->GetEffectiveBonusSum();
}

int DispatcherSystem::DispatchGetBaseCasterLevel(objHndl handle, Stat casterClass){
	auto objDispatcher = gameSystems->GetObj().GetObject(handle)->GetDispatcher();
	if (!objDispatcher->IsValid())
		return 0;

	EvtObjSpellCaster evtObj;
	evtObj.handle = handle;
	evtObj.arg0 = casterClass;
	DispatcherProcessor(objDispatcher, dispTypeGetBaseCasterLevel, DK_NONE, &evtObj);
	return evtObj.bonlist.GetEffectiveBonusSum();
}

int DispatcherSystem::DispatchGetCasterLevelStage2(objHndl handle, Stat casterClass, int initialVal) {
	auto objDispatcher = gameSystems->GetObj().GetObject(handle)->GetDispatcher();
	if (!objDispatcher->IsValid())
		return 0;

	EvtObjSpellCaster evtObj;
	evtObj.handle = handle;
	evtObj.arg0 = casterClass;
	evtObj.bonlist.AddBonus(initialVal, 1, 102);

	DispatcherProcessor(objDispatcher, dispTypeSpellCasterGeneral, DK_SPELL_Base_Caster_Level_2, &evtObj);
	return evtObj.bonlist.GetEffectiveBonusSum();
}

int DispatcherSystem::DispatchLevelupSystemEvent(objHndl handle, Stat casterClass, D20DispatcherKey evtType)
{
	auto objDispatcher = gameSystems->GetObj().GetObject(handle)->GetDispatcher();
	if (!objDispatcher->IsValid())
		return 0;

	EvtObjSpellCaster evtObj;
	evtObj.handle = handle;
	evtObj.arg0 = casterClass;
	DispatcherProcessor(objDispatcher, dispTypeLevelupSystemEvent, evtType, &evtObj);
	return evtObj.bonlist.GetEffectiveBonusSum();
}

void DispatcherSystem::DispatchLevelupSpellsFinalize(objHndl handle, Stat casterClass){
	auto objDispatcher = gameSystems->GetObj().GetObject(handle)->GetDispatcher();
	if (!objDispatcher->IsValid())
		return;

	EvtObjSpellCaster evtObj;
	evtObj.handle = handle;
	evtObj.arg0 = casterClass;
	DispatcherProcessor(objDispatcher, dispTypeLevelupSystemEvent, DK_NONE, &evtObj);
}

int DispatcherSystem::Dispatch45SpellResistanceMod(objHndl handle, DispIOBonusListAndSpellEntry* dispIo)
{
	auto dispatcher = gameSystems->GetObj().GetObject(handle)->GetDispatcher();
	if (dispatcher->IsValid())
	{
		BonusList bonlist;
		dispIo->bonList = &bonlist;
		DispatcherProcessor(dispatcher, dispTypeSpellResistanceMod, 0, dispIo);
		auto bonus = bonlist.GetEffectiveBonusSum();
		return bonus;
	}
	else
		return 0;
}

void DispatcherSystem::DispIoDamageInit(DispIoDamage* dispIoDamage)
{
	dispIoDamage->dispIOType = dispIOTypeDamage;
	damage.DamagePacketInit(&dispIoDamage->damage);
	dispIoDamage->attackPacket.attacker=0i64;
	dispIoDamage->attackPacket.victim = 0i64;
	dispIoDamage->attackPacket.dispKey = 0;
	*(int*)&dispIoDamage->attackPacket.flags = 0;
	dispIoDamage->attackPacket.weaponUsed = 0i64;
	dispIoDamage->attackPacket.ammoItem = 0i64;
	dispIoDamage->attackPacket.d20ActnType= D20A_NONE;

}

void DispatcherSystem::DispatchSpellDamage(objHndl obj, DamagePacket* damage, objHndl target, SpellPacketBody *spellPkt)
{
	auto _dispatcher = objects.GetDispatcher(obj);
	if (!dispatch.dispatcherValid(_dispatcher)) return;
	EvtObjDealingSpellDamage dispIo;
	dispIo.damage = damage;
	dispIo.spellPkt = spellPkt;
	dispIo.target = target;
	DispatcherProcessor(_dispatcher, dispTypeDealingDamageSpell, 0, &dispIo);
}

int32_t DispatcherSystem::DispatchDamage(objHndl objHnd, DispIoDamage* dispIoDamage, enum_disp_type dispType, D20DispatcherKey key)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (!dispatch.dispatcherValid(dispatcher)) return 0;
	DispIoDamage * dispIo = dispIoDamage;
	DispIoDamage dispIoLocal;
	if (!dispIoDamage)
	{
		dispatch.DispIoDamageInit(&dispIoLocal);
		dispIo = &dispIoLocal;
	}
	logger->info("Dispatching damage event for {} - type {}, key {}, victim {}", 
		objHnd, dispType, key, dispIoDamage->attackPacket.victim );
	dispatch.DispatcherProcessor(dispatcher, dispType, key, dispIo);
	return 1;
}

int DispatcherSystem::DispatchD20ActionCheck(D20Actn* d20a, TurnBasedStatus* turnBasedStatus, enum_disp_type dispType)
{
	auto dispatcher = objects.GetDispatcher(d20a->d20APerformer);
	if (dispatch.dispatcherValid(dispatcher))
	{
		DispIoD20ActionTurnBased dispIo(d20a);
		dispIo.tbStatus = turnBasedStatus;
		if (dispType == dispTypeGetBonusAttacks) {
			BonusList bonlist;
			dispIo.bonlist = &bonlist;
			dispatch.DispatcherProcessor(dispatcher, dispType, d20a->d20ActType + 75, &dispIo);
			auto bonval = bonlist.GetEffectiveBonusSum();
			return bonval + dispIo.returnVal;
		}
		else {
			dispatch.DispatcherProcessor(dispatcher, dispType, d20a->d20ActType + 75, &dispIo);
			return dispIo.returnVal;
		}
	}
	return 0;
}

int DispatcherSystem::Dispatch60GetAttackDice(objHndl obj, DispIoAttackDice* dispIo)
{
	BonusList localBonlist;
	if (!dispIo->bonlist)
		dispIo->bonlist = &localBonlist;

	Dispatcher * dispatcher = objects.GetDispatcher(obj);
	if (!dispatcherValid(dispatcher))
		return 0;
	if (dispIo->weapon)
	{
		int weaponDice = objects.getInt32(dispIo->weapon, obj_f_weapon_damage_dice);
		dispIo->dicePacked = weaponDice;
		auto dmgTy = static_cast<DamageType>(objects.getInt32(dispIo->weapon, obj_f_weapon_attacktype));
		dispIo->attackDamageType = dmgTy;
	}
	DispatcherProcessor(dispatcher, dispTypeGetAttackDice, 0, dispIo);
	int sizeMod = bonusSys.getOverallBonus(dispIo->bonlist);
	Dice base = Dice::FromPacked(dispIo->dicePacked);
	Dice mod = damage.ModifyDamageDiceForSize(base, sizeMod);
	return mod.ToPacked();
}

int DispatcherSystem::Dispatch61GetLevel(objHndl handle, Stat stat, BonusList* bonlist, objHndl someObj, LevelDrainType omit)
{
	auto obj = objSystem->GetObject(handle);
	if (!obj)
		return 0;
	auto dispatcher = obj->GetDispatcher();
	if (!dispatcher->IsValid())
		return 0;
	DispIoObjBonus evtObj;
	evtObj.obj = someObj;
	evtObj.flags = static_cast<uint32_t>(omit);
	if (bonlist) {
		evtObj.bonOut = bonlist;
	}
	dispatcher->Process(enum_disp_type::dispTypeGetLevel, (D20DispatcherKey)( stat + D20DispatcherKey::DK_CL_Level), &evtObj);
	auto result = evtObj.bonOut->GetEffectiveBonusSum();
	return result;
}

int DispatcherSystem::DispatchGetBonus(objHndl critter, DispIoBonusList* eventObj, enum_disp_type dispType, D20DispatcherKey key) {

	DispIoBonusList dispIo;
	if (!eventObj) {
		dispIo.dispIOType = dispIOTypeBonusList;
		dispIo.flags = 0;
		eventObj = &dispIo;
	}

	auto obj = objSystem->GetObject(critter);
	if (!obj->IsCritter()) {
		return 0;
	}

	auto dispatcher = obj->GetDispatcher();
	if (!dispatcher) {
		return 0;
	}

	DispatcherProcessor(dispatcher, dispType, key, eventObj);

	return eventObj->bonlist.GetEffectiveBonusSum();

}

/*  0x1004CEB0 */
int DispatcherSystem::DispatchItemQuery(objHndl item, D20DispatcherKey key)
{
	DispIoD20Query evtObj;
	*(objHndl*)(&evtObj.data1) = item;
	DispatchForItem(item, dispTypeD20Query, key, &evtObj);
	return evtObj.return_val;
}

void DispIO::AssertType(enum_dispIO_type eventObjType) const
{
	assert(dispIOType == eventObjType);
}
#pragma endregion





#pragma region Dispatcher Functions

void __cdecl _DispatcherRemoveSubDispNodes(Dispatcher * dispatcher, CondNode * cond)
{
	for (uint32_t i = 0; i < dispTypeCount; i++)
	{
		SubDispNode ** ppSubDispNode = &dispatcher->subDispNodes[i];
		while (*ppSubDispNode != nullptr)
		{
			if ((*ppSubDispNode)->condNode == cond)
			{
				SubDispNode * savedNext = (*ppSubDispNode)->next;
				free(*ppSubDispNode);
				*ppSubDispNode = savedNext;

			}
			else
			{
				ppSubDispNode = &((*ppSubDispNode)->next);
			}

		}
	}

	};


void __cdecl _DispatcherClearField(Dispatcher *dispatcher, CondNode ** dispCondList)
{
	CondNode * cond = *dispCondList;
	objHndl obj = dispatcher->objHnd;
	while (cond != nullptr)
	{
		SubDispNode * subDispNode_TypeRemoveCond = dispatcher->subDispNodes[2];
		CondNode * nextCond = cond->nextCondNode;

		while (subDispNode_TypeRemoveCond != nullptr)
		{

			SubDispDef * sdd = subDispNode_TypeRemoveCond->subDispDef;
			if (sdd->dispKey == 0 && (subDispNode_TypeRemoveCond->condNode->flags & 1) == 0
				&& subDispNode_TypeRemoveCond->condNode == cond)
			{
				sdd->dispCallback(subDispNode_TypeRemoveCond, obj, dispTypeConditionRemove, 0, nullptr);
			}
			subDispNode_TypeRemoveCond = subDispNode_TypeRemoveCond->next;
		}
		_DispatcherRemoveSubDispNodes(dispatcher, cond);
		free(cond);
		cond = nextCond;

	}
	*dispCondList = nullptr;
};

void __cdecl _DispatcherClearPermanentMods(Dispatcher *dispatcher)
{
	_DispatcherClearField(dispatcher, &dispatcher->permanentMods);
};

void __cdecl _DispatcherClearItemConds(Dispatcher *dispatcher)
{
	_DispatcherClearField(dispatcher, &dispatcher->itemConds);
};

void __cdecl _DispatcherClearConds(Dispatcher *dispatcher)
{
	_DispatcherClearField(dispatcher, &dispatcher->conditions);
};

DispIoCondStruct * _DispIoCheckIoType1(DispIoCondStruct * dispIo)
{
	return dispatch.DispIoCheckIoType1(dispIo);
}

DispIoBonusList* _DispIoCheckIoType2(DispIoBonusList* dispIo)
{
	return dispatch.DispIoCheckIoType2(dispIo);
}

DispIoSavingThrow* _DispIOCheckIoType3(DispIoSavingThrow* dispIo)
{
	return dispatch.DispIoCheckIoType3(dispIo);
}

DispIoDamage* _DispIoCheckIoType4(DispIoDamage* dispIo)
{
	return dispatch.DispIoCheckIoType4(dispIo);
}

DispIoAttackBonus* _DispIoCheckIoType5(DispIoAttackBonus* dispIo)
{
	return dispatch.DispIoCheckIoType5(dispIo);
}

DispIoD20Signal* _DispIoCheckIoType6(DispIoD20Signal* dispIo)
{
	return dispatch.DispIoCheckIoType6(dispIo);
}

DispIoD20Query* _DispIoCheckIoType7(DispIoD20Query* dispIo)
{
	return dispatch.DispIoCheckIoType7(dispIo);
}

DispIOTurnBasedStatus* _DispIoCheckIoType8(DispIOTurnBasedStatus* dispIo)
{
	return dispatch.DispIoCheckIoType8(dispIo);
}

DispIoTooltip* _DispIoCheckIoType9(DispIoTooltip* dispIo)
{
	return dispatch.DispIoCheckIoType9(dispIo);
}

DispIoObjBonus* _DispIoCheckIoType10(DispIoObjBonus* dispIo)
{
	return dispatch.DispIoCheckIoType10(dispIo);
}

DispIoDispelCheck* _DispIoCheckIoType11(DispIoDispelCheck* dispIo)
{
	return dispatch.DispIOCheckIoType11(dispIo);
}

DispIoD20ActionTurnBased* _DispIoCheckIoType12(DispIoD20ActionTurnBased* dispIo)
{
	return dispatch.DispIoCheckIoType12(dispIo);
};

DispIOBonusListAndSpellEntry* _DispIoCheckIoType14(DispIOBonusListAndSpellEntry* dispIO)
{
	return dispatch.DispIOCheckIoType14(dispIO);
}

void _PackDispatcherIntoObjFields(objHndl objHnd, Dispatcher* dispatcher)
{
	dispatch.PackDispatcherIntoObjFields(objHnd, dispatcher);
};

void _DispatcherProcessor(Dispatcher* dispatcher, enum_disp_type dispType, D20DispatcherKey dispKey, DispIO* dispIO) {
	static uint32_t dispCounter = 0;
	if (dispCounter > DISPATCHER_MAX) {
		logger->info("Dispatcher maximum recursion reached!");
		return;
	}
	dispCounter++;
	
	SubDispNode* subDispNode = dispatcher->subDispNodes[dispType];

	while (subDispNode != nullptr) {

		if ((subDispNode->subDispDef->dispKey == dispKey || subDispNode->subDispDef->dispKey == 0) && ((subDispNode->condNode->flags & 1) == 0)) {

			DispIoTypeImmunityTrigger dispIoImmunity;
			DispIOType21Init((DispIoTypeImmunityTrigger*)&dispIoImmunity);
			dispIoImmunity.condNode = (CondNode *)subDispNode->condNode;

			// prevent recursion
			if (dispKey != DK_IMMUNITY_SPELL || dispType != dispTypeImmunityTrigger) {
				_Dispatch62(dispatcher->objHnd, (DispIO*)&dispIoImmunity, DK_IMMUNITY_SPELL);
			}
			
			if (dispIoImmunity.interrupt == 1 && dispType != dispType63) { // dispType63 is essentially <-> Minor globe of invulnerability
				dispIoImmunity.interrupt = 0;
				dispIoImmunity.val2 = 10;
				dispIoImmunity.dispKey = dispKey;
				dispIoImmunity.dispType = dispType;
				_Dispatch63(dispatcher->objHnd, (DispIO*)&dispIoImmunity);
				if (dispIoImmunity.interrupt == 0) {
					subDispNode->subDispDef->dispCallback(subDispNode, dispatcher->objHnd, dispType, dispKey, (DispIO*)dispIO);
				}
			}
			else {
				subDispNode->subDispDef->dispCallback(subDispNode, dispatcher->objHnd, dispType, dispKey, (DispIO*)dispIO);
			}

		}

		subDispNode = subDispNode->next;
	}

	dispCounter--;
	
	return;
}

int32_t _DispatchDamage(objHndl objHnd, DispIoDamage* dispIo, enum_disp_type dispType, D20DispatcherKey key)
{
	return dispatch.DispatchDamage(objHnd, dispIo, dispType, key);
}

int32_t _dispatch1ESkillLevel(objHndl objHnd, SkillEnum skill, BonusList* bonOut, objHndl objHnd2, int32_t flag)
{
	return dispatch.dispatch1ESkillLevel(objHnd, skill, bonOut, objHnd2, flag);
}


void DispIoEffectTooltip::Append(int effectTypeId, int spellEnum, const char* text) const
{
	BuffDebuffSub * bdbSub = nullptr;
	auto findSpec = uiParty.IndicatorSpecGet(effectTypeId);
	switch (findSpec.type)
	{
	case IT_BUFF:
		if (this->bdb->buffCount >= 8)
			return;
		bdbSub = &bdb->buffs[bdb->buffCount++];
		break;
	case IT_AILMENT:
		if (bdb->debuffCount >= 8)
			return;
		bdbSub = &bdb->debuffs[bdb->debuffCount++];
		break;
	case IT_CONDITION:
		if (bdb->innerCount >= 6)
			return;
		bdbSub = &bdb->innerStatuses[bdb->innerCount++];
		break;
	}

	//copy the text
	if (bdbSub != nullptr) {
		bdbSub->effectTypeId = effectTypeId;
		bdbSub->spellEnum = spellEnum;
		if (text) {
			bdbSub->text = new char[strlen(text) + 1];
			strcpy(const_cast<char*>(bdbSub->text), text);
		}
		else
		{
			bdbSub->text = nullptr;
		}
	}
	else {
		logger->error("Unknown tooltip effect {}", effectTypeId);
	}
}

void EvtObjActionCost::DispatchCost(D20DispatcherKey key){
	if (!d20a || !d20a->d20APerformer) {
		return;
	}

	auto dispatcher = objects.GetDispatcher(d20a->d20APerformer);

	if (dispatcher->IsValid()) {
		dispatch.DispatcherProcessor(dispatcher, dispTypeActionCostMod, key, this);
	}
}

bool Dispatcher::IsValid()
{
	return dispatch.dispatcherValid(this);
}

void Dispatcher::Process(enum_disp_type dispType, D20DispatcherKey key, DispIO* dispIo){
	dispatch.DispatcherProcessor(this, dispType, key, dispIo);
}

Dispatcher* _DispatcherInit(objHndl objHnd) {
	Dispatcher* dispatcherNew = (Dispatcher *)malloc(sizeof(Dispatcher));
	memset(&dispatcherNew->subDispNodes, 0, dispTypeCount * sizeof(SubDispNode*));
	CondNode* condNode = *(conds.pCondNodeGlobal);
	while (condNode != nullptr) {
		_CondNodeAddToSubDispNodeArray(dispatcherNew, condNode);
		condNode = condNode->nextCondNode;
	}
	dispatcherNew->objHnd = objHnd;
	dispatcherNew->permanentMods = nullptr;
	dispatcherNew->itemConds = nullptr;
	dispatcherNew->conditions = nullptr;
	return dispatcherNew;
};

void DispIOType21Init(DispIoTypeImmunityTrigger* dispIO) {
	dispIO->dispIOType = dispIOType21ImmunityTrigger;
	dispIO->interrupt = 0;
	dispIO->field_8 = 0;
	dispIO->field_C = 0;
	dispIO->SDDKey1 = 0;
	dispIO->val2 = 0;
	dispIO->okToAdd = 0;
	dispIO->condNode = nullptr;
}

void _dispatchTurnBasedStatusInit(objHndl objHnd, DispIOTurnBasedStatus* dispIOtB)
{
	dispatch.dispatchTurnBasedStatusInit(objHnd, dispIOtB);
}

int _DispatchAttackBonus(objHndl objHnd, objHndl victim, DispIoAttackBonus* dispIo, enum_disp_type dispType, int key)
{
	return dispatch.DispatchAttackBonus(objHnd, victim, dispIo, dispType, key);
};


uint32_t _Dispatch62(objHndl objHnd, DispIO* dispIO, uint32_t dispKey) {
	Dispatcher* dispatcher = (Dispatcher *)objects.GetDispatcher(objHnd);
	if (dispatcher != nullptr && (int32_t)dispatcher != -1) {
		_DispatcherProcessor(dispatcher, dispTypeImmunityTrigger, (D20DispatcherKey)dispKey, dispIO);
	}
	return 0;
}


uint32_t _Dispatch63(objHndl objHnd, DispIO* dispIO) {
	Dispatcher* dispatcher = (Dispatcher *)objects.GetDispatcher(objHnd);
	if (dispatcher != nullptr && (int32_t)dispatcher != -1) {
		_DispatcherProcessor(dispatcher, dispType63, (D20DispatcherKey)0, dispIO);
	}
	return 0;
}


#pragma endregion 
SubDispDefNew::SubDispDefNew(){
	dispType = dispType0;
	dispKey = DK_NONE;
	dispCallback = nullptr;
	data1.sVal = 0;
	data2.sVal = 0;
}

SubDispDefNew::SubDispDefNew(enum_disp_type type, uint32_t key, int(* callback)(DispatcherCallbackArgs), CondStructNew* data1In, uint32_t data2In){
	dispType = type;
	dispKey = key;
	dispCallback = callback;
	data1.condStruct = data1In;
	data2.usVal = data2In;
}

SubDispDefNew::SubDispDefNew(enum_disp_type type, uint32_t key, int(* callback)(DispatcherCallbackArgs), uint32_t data1In, uint32_t data2In){
	dispType = type;
	dispKey = key;
	dispCallback = callback;
	data1.usVal = data1In;
	data2.usVal = data2In;
}

//SubDispDefNew::SubDispDefNew(enum_disp_type type, uint32_t key, int(* callback)(DispatcherCallbackArgs), uint32_t data1In, const char* cs)
//{
//	dispType = type;
//	dispKey = key;
//	dispCallback = callback;
//	data1.usVal = data1In;
//	data2.cs = cs;
//}

CondStructNew::CondStructNew(){
	numArgs = 0;
	condName = nullptr;
	memset(subDispDefs, 0, sizeof(subDispDefs));
}

CondStructNew::CondStructNew(std::string Name, int NumArgs, bool preventDuplicate) : CondStructNew() {
	condName = _strdup(Name.c_str());
	numArgs = NumArgs;
	if (preventDuplicate) {
		AddHook(dispTypeConditionAddPre, DK_NONE, ConditionPrevent, this, 0);
	}
	Register();
	
}

CondStructNew::CondStructNew(CondStruct& existingCondStruct){
	ExtendExisting(existingCondStruct.condName);
}

void CondStructNew::AddHook(enum_disp_type dispType, D20DispatcherKey dispKey, int(* callback)(DispatcherCallbackArgs)){
	Expects(numHooks < 99);
	subDispDefs[numHooks++] = { dispType, dispKey, callback, static_cast<uint32_t>(0), static_cast<uint32_t>(0) };
}

void CondStructNew::AddHook(enum_disp_type dispType, D20DispatcherKey dispKey, int(* callback)(DispatcherCallbackArgs), uint32_t data1, uint32_t data2){
	Expects(numHooks < 99);
	subDispDefs[numHooks++] = {dispType, dispKey, callback, data1, data2};
}

void CondStructNew::AddHook(enum_disp_type dispType, D20DispatcherKey dispKey, int(* callback)(DispatcherCallbackArgs), CondStructNew* data1, uint32_t data2){
	Expects(numHooks < 99);
	subDispDefs[numHooks++] = { dispType, dispKey, callback, data1, data2 };
}

void CondStructNew::AddHook(enum_disp_type dispType, D20DispatcherKey dispKey, int(*callback)(DispatcherCallbackArgs), CondStruct * data1, uint32_t data2)
{
	Expects(numHooks < 99);
	subDispDefs[numHooks++] = { dispType, dispKey, callback, (CondStructNew*)data1, data2 };
}

//void CondStructNew::AddHook(enum_disp_type dispType, D20DispatcherKey dispKey, int(* callback)(DispatcherCallbackArgs), uint32_t data1, const char* data2) 
//{
//	Expects(numHooks < 99);
//	subDispDefs[numHooks++] = { dispType, dispKey, callback, data1, data2 };
//}

//void CondStructNew::AddPyHook(enum_disp_type dispType, D20DispatcherKey dispKey, PyObject* pycallback, PyObject* pydataTuple){
//	Expects(numHooks < 99);
//	subDispDefs[numHooks++] = { dispType, dispKey, PyModHookWrapper, (uint32_t) pycallback, (uint32_t) pydataTuple};
//}
//void CondStructNew::AddPyHook(enum_disp_type dispType, D20DispatcherKey dispKey, pybind11::function pycallback,pybind11::tuple pydataTuple) {
//		Expects(numHooks < 99);
//		subDispDefs[numHooks++] = { dispType, dispKey, PyModHookWrapper, (uint32_t) pycallback.ptr(), (uint32_t) pydataTuple.ptr()};
//	}

void CondStructNew::Register(){
	conds.hashmethods.CondStructAddToHashtable(reinterpret_cast<CondStruct*>(this), true);
}

void CondStructNew::ExtendExisting(const std::string & condName)
{
	auto cond = (CondStructNew*)conds.GetByName(condName);
	if (cond) {
		for (auto i = 0; i < 100 && cond->subDispDefs[i].dispType != 0; i++) {
			this->subDispDefs[this->numHooks++] = cond->subDispDefs[i];
		}
		this->numArgs = cond->numArgs;
		this->condName = cond->condName;
		this->Register();
	}
	else {
		logger->info("Extend Existing Error: Condition {} does not exist!", condName);
	}
}

void CondStructNew::AddToFeatDictionary(feat_enums feat, feat_enums featEnumMax, uint32_t condArg2Offset){
	conds.AddToFeatDictionary(this, feat, featEnumMax, condArg2Offset);
}

int32_t DispatcherCallbackArgs::GetCondArg(uint32_t argIdx)
{
	return conds.CondNodeGetArg(subDispNode->condNode, argIdx);
}

objHndl DispatcherCallbackArgs::GetCondArgObjHndl(uint32_t argIdx)
{
	objHndl handle { ((((uint64_t)GetCondArg(argIdx)) << 32) | GetCondArg(argIdx + 1)) };
	if (!gameSystems->GetObj().IsValidHandle(handle))
		handle = objHndl::null;
	return handle;
}

void* DispatcherCallbackArgs::GetCondArgPtr(uint32_t argIdx){
	return conds.CondNodeGetArgPtr(subDispNode->condNode, argIdx);
}

int DispatcherCallbackArgs::GetData1() const
{
	return subDispNode->subDispDef->data1;
}

int DispatcherCallbackArgs::GetData2() const
{
	return subDispNode->subDispDef->data2;
}

void DispatcherCallbackArgs::SetCondArg(uint32_t argIdx, int value)
{
	conds.CondNodeSetArg(subDispNode->condNode, argIdx, value);
}

void DispatcherCallbackArgs::SetCondArgObjHndl(uint32_t argIdx, const objHndl& handle){
	if (argIdx +1 < this->subDispNode->condNode->condStruct->numArgs){
		SetCondArg(argIdx, handle.GetHandleUpper());
		SetCondArg(argIdx + 1, handle.GetHandleLower());
	}
}

void DispatcherCallbackArgs::SetExpired()
{
	subDispNode->condNode->flags |= 1;
}

void DispatcherCallbackArgs::RemoveCondition(){
	conds.ConditionRemove(this->objHndCaller, this->subDispNode->condNode);
}

void DispatcherCallbackArgs::RemoveSpell(){
	auto removeSpell = temple::GetRef<void(__cdecl)(DispatcherCallbackArgs)>(0x100D7620);
	removeSpell(*this);
}

DispIoSavingThrow::DispIoSavingThrow(){
	this->dispIOType = dispIOTypeSavingThrow;
	this->obj = objHndl::null;
	this->flags = 0;
	this->rollResult = 0;
}

DispIoAttackBonus::DispIoAttackBonus(){
	dispIOType = dispIOTypeAttackBonus;
	this->attackPacket.dispKey = 1;
}

int DispIoAttackBonus::Dispatch(objHndl obj, objHndl obj2, enum_disp_type dispType, D20DispatcherKey key){
	return dispatch.DispatchAttackBonus(obj, obj2, this, dispType, key);
}

void DispIoTooltip::Append(string& cs)
{
	if (numStrings < 10)
	{
		strncpy(strings[numStrings++], &cs[0], 0x100);
	}
}

DispIoObjBonus::DispIoObjBonus()
{
	dispIOType = dispIoTypeObjBonus;
	obj = 0i64;
	pad = 0;
	flags = 0;
	bonOut = &bonlist;
}

DispIoD20ActionTurnBased::DispIoD20ActionTurnBased(){
	dispIOType = dispIOTypeD20ActionTurnBased;
	returnVal = 0;
	d20a = nullptr;
	tbStatus = nullptr;
	bonlist = nullptr;
}

DispIoD20ActionTurnBased::DispIoD20ActionTurnBased(D20Actn* D20a):DispIoD20ActionTurnBased(){
	d20a = D20a;
}

void DispIoD20ActionTurnBased::DispatchPerform(D20DispatcherKey key){
	if (!d20a || !d20a->d20APerformer){
		returnVal = AEC_INVALID_ACTION;
		return;
	}
		
	auto dispatcher = objects.GetDispatcher(d20a->d20APerformer);

	if (dispatcher->IsValid() ){
		dispatch.DispatcherProcessor(dispatcher, dispTypeD20ActionPerform, key, this);
	}
}

void DispIoD20ActionTurnBased::DispatchPythonAdf(D20DispatcherKey key){
	if (!d20a || !d20a->d20APerformer) {
		returnVal = AEC_INVALID_ACTION;
		return;
	}

	auto dispatcher = objects.GetDispatcher(d20a->d20APerformer);

	if (dispatcher->IsValid()) {
		dispatch.DispatcherProcessor(dispatcher, dispTypePythonAdf, key, this);
	}
}

void DispIoD20ActionTurnBased::DispatchPythonActionCheck(D20DispatcherKey key){
	if (!d20a || !d20a->d20APerformer) {
		this->returnVal = AEC_INVALID_ACTION;
		return;
	}

	auto dispatcher = objects.GetDispatcher(d20a->d20APerformer);

	if (dispatcher->IsValid()) {
		dispatch.DispatcherProcessor(dispatcher, dispTypePythonActionCheck, key, this);
	}
}

void DispIoD20ActionTurnBased::DispatchPythonActionAddToSeq(D20DispatcherKey key){
	if (!d20a || !d20a->d20APerformer) {
		this->returnVal = AEC_INVALID_ACTION;
		return;
	}

	auto dispatcher = objects.GetDispatcher(d20a->d20APerformer);

	if (dispatcher->IsValid()) {
		dispatch.DispatcherProcessor(dispatcher, dispTypePythonActionAdd, key, this);
	}
}

void DispIoD20ActionTurnBased::DispatchPythonActionPerform(D20DispatcherKey key){
	if (!d20a || !d20a->d20APerformer) {
		this->returnVal = AEC_INVALID_ACTION;
		return;
	}

	auto dispatcher = objects.GetDispatcher(d20a->d20APerformer);

	if (dispatcher->IsValid()) {
		dispatch.DispatcherProcessor(dispatcher, dispTypePythonActionPerform, key, this);
	}
}

void DispIoD20ActionTurnBased::DispatchPythonActionFrame(D20DispatcherKey key){
	if (!d20a || !d20a->d20APerformer) {
		this->returnVal = AEC_INVALID_ACTION;
		return;
	}

	auto dispatcher = objects.GetDispatcher(d20a->d20APerformer);

	if (dispatcher->IsValid()) {
		dispatch.DispatcherProcessor(dispatcher, dispTypePythonActionFrame, key, this);
	}
}

int DispIOBonusListAndSpellEntry::Dispatch(objHndl handle, enum_disp_type evtType){
	auto dispatcher = objSystem->GetObject(handle)->GetDispatcher();
	if (!dispatcher->IsValid())
		return 0;

	BonusList bonlistLocal;
	if (!this->bonList){
		this->bonList = &bonlistLocal;
	}

	dispatcher->Process(evtType, DK_NONE, this);

	return this->bonList->GetEffectiveBonusSum();
}

DispIoTypeImmunityTrigger::DispIoTypeImmunityTrigger()
{
	{
		dispIOType = dispIOType21ImmunityTrigger;
		interrupt = 0;
		field_8 = 0;
		condNode = nullptr;
		field_C = 0;
		SDDKey1 = 0;
		val2 = 0;
		okToAdd = 0;
		dispType = (enum_disp_type)0;
		dispKey = (D20DispatcherKey)0;
	}
}

std::vector<int> EvtObjAddMesh::DispatchGetAddMeshes()
{
	auto &result = addmeshes;
	if (!handle || !objSystem->IsValidHandle(handle))
		return addmeshes;

	auto _dispatcher = objects.GetDispatcher(handle);
	if (!dispatch.dispatcherValid(_dispatcher)) return result;

	dispatch.DispatcherProcessor(_dispatcher, dispTypeAddMesh, 0, this);
	
	return result;
}

void EvtObjAddMesh::Append(int addmeshId)
{
	addmeshes.push_back(addmeshId);
}

EvtObjAddMesh::EvtObjAddMesh(objHndl handleIn)
{
	this->dispIOType = evtObjTypeAddMesh;
	handle = handleIn;
}

DispIoSpellsPerDay::DispIoSpellsPerDay()
{
	dispIOType = dispIOType18;
	unk = 3001;
	unk2 = 0;
}
