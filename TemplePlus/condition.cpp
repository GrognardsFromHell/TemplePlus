#include "stdafx.h"
#include "common.h"
#include "dispatcher.h"
#include "condition.h"
#include "temple_functions.h"
#include "obj.h"
#include "bonus.h"
#include "radialmenu.h"
#include "combat.h"
#include "critter.h"
#include "location.h"
#include "damage.h"
#include "float_line.h"
#include "ui/ui_dialog.h"
#include "party.h"
#include "weapon.h"
#include "action_sequence.h"
#include "ui/ui_item_creation.h"
#include "util/fixes.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include <infrastructure/elfhash.h>
#include "particles.h"
#include "gamesystems/particlesystems.h"
#include "anim.h"
#include "python/python_integration_spells.h"
#include "history.h"
#include "gamesystems/objects/objevent.h"
#include "ui/ui_party.h"

ConditionSystem conds;
CondStructNew conditionDisableAoO;
CondStructNew conditionGreaterTwoWeaponFighting;
CondStructNew condGreaterTWFRanger;
CondStructNew condDivineMight;
CondStructNew condDivineMightBonus;
CondStructNew condRecklessOffense;
CondStructNew condKnockDown;
CondStructNew condDeadlyPrecision;
CondStructNew condPersistentSpell;

CondStructNew condGreaterWeaponSpecialization;

CondStructNew ConditionSystem::mCondDisarm;
CondStructNew ConditionSystem::mCondDisarmed;


//items
CondStructNew ConditionSystem::mCondNecklaceOfAdaptation;

// monsters
CondStructNew condRend;

CondStructNew ConditionSystem::mCondCaptivatingSong;
CondStructNew ConditionSystem::mCondCaptivated;

CondStructNew ConditionSystem::mCondHezrouStench;
CondStructNew ConditionSystem::mCondHezrouStenchHit;




struct ConditionSystemAddresses : temple::AddressTable
{
	void(__cdecl* SetPermanentModArgsFromDataFields)(Dispatcher* dispatcher, CondStruct* condStruct, int* condArgs);
	int(__cdecl*RemoveSpellCondition)(DispatcherCallbackArgs args); 
	int(__cdecl*RemoveSpellMod)(DispatcherCallbackArgs args);
	ConditionSystemAddresses()
	{
		rebase(SetPermanentModArgsFromDataFields, 0x100E1B90);
		rebase(RemoveSpellCondition, 0x100D7620);
		rebase(RemoveSpellMod, 0x100CBAB0);
	}

	
	
} addresses;

class ConditionFunctionReplacement : public TempleFix {
public:
	const char* name() override {
		return "Condition Function Replacement";
	}

	static int LayOnHandsPerform(DispatcherCallbackArgs arg);
	static int RemoveDiseasePerform(DispatcherCallbackArgs arg); // also used in WholenessOfBodyPerform
	void HookSpellCallbacks();
	void apply() override {
		logger->info("Replacing Condition-related Functions");
		
		//conds.RegisterNewConditions();
		
		

		replaceFunction(0x100E19C0, _CondStructAddToHashtable);
		replaceFunction(0x100E1A80, _GetCondStructFromHashcode);
		replaceFunction(0x100E1AB0, _CondNodeGetArg);
		replaceFunction(0x100E1AD0, _CondNodeSetArg);
		replaceFunction(0x100E1DD0, _CondNodeAddToSubDispNodeArray);

		replaceFunction(0x100E22D0, _ConditionAddDispatch);
		replaceFunction(0x100E24C0, _ConditionAddToAttribs_NumArgs0);
		replaceFunction(0x100E2500, _ConditionAddToAttribs_NumArgs2);
		replaceFunction(0x100E24E0, _ConditionAdd_NumArgs0);
		replaceFunction(0x100E2530, _ConditionAdd_NumArgs2);
		replaceFunction(0x100E2560, _ConditionAdd_NumArgs3);
		replaceFunction(0x100E2590, _ConditionAdd_NumArgs4);
		replaceFunction(0x100E25C0, InitCondFromCondStructAndArgs);
		
		replaceFunction(0x100EABB0, BarbarianRageStatBonus);
		replaceFunction(0x100EABE0, BarbarianRageSaveBonus);
		
		replaceFunction(0x100ECF30, ConditionPrevent);
		replaceFunction(0x100EE050, GlobalGetArmorClass);
		replaceFunction(0x100EE280, GlobalToHitBonus);
		replaceFunction(0x100EE760, GlobalOnDamage);
		
		// fixes for lack of uniqueAnimID registration
		replaceFunction(0x100FA060, LayOnHandsPerform);
		replaceFunction(0x100FB150, RemoveDiseasePerform);
		static int (__cdecl* orgLayOnHandsPerformOnActionFrame)(DispatcherCallbackArgs) = replaceFunction<int(__cdecl)(DispatcherCallbackArgs)>(0x100FA0F0, [](DispatcherCallbackArgs args){
			return orgLayOnHandsPerformOnActionFrame(args);
		});
		


		replaceFunction(0x100F7B60, _FeatConditionsRegister);
		replaceFunction(0x100F7BE0, _GetCondStructFromFeat);

		replaceFunction(0x100F7D60, CombatExpertiseRadialMenu);
		replaceFunction(0x100F7F00, CombatExpertiseSet);
		replaceFunction(0x100F88C0, TwoWeaponFightingBonus);
		replaceFunction(0x100F8940, TwoWeaponFightingBonusRanger);
		replaceFunction(0x100FEBA0, BarbarianDamageResistance);

		
		replaceFunction(0x10101150, ItemSkillBonusCallback);

		// Replace hooks for S_Is_BreakFree_Possible so they also return the spell Id
		int writeVal = (int)QueryRetrun1GetArgs;
		write(0x102E0F1C + 8, &writeVal, sizeof(int*)); // web on
		write(0x102D7958 + 8, &writeVal, sizeof(int*)); // entangle on


		//replaceFunction(0x100C7180, QueryReturn1GetArgs); // caused a crash :(

		HookSpellCallbacks();

		

		
	}
} condFuncReplacement;


class SpellCallbacks {
#define SPELL_CALLBACK(name) static int __cdecl name(DispatcherCallbackArgs args);
public:

	static int __cdecl SkillBonus(DispatcherCallbackArgs args);
	static int __cdecl BeginHezrouStench(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchObjEvent(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchCountdown(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchTurnbasedStatus(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchAooPossible(DispatcherCallbackArgs args);


	static int __cdecl HezrouStenchAbilityCheckMod(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchSavingThrowLevel(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchDealingDamage(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchToHit2(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchEffectTooltip(DispatcherCallbackArgs args);
	static int __cdecl HezrouStenchCureNausea(DispatcherCallbackArgs args);
	static int __cdecl RemoveSpell(DispatcherCallbackArgs args);
	static int __cdecl HasCondition(DispatcherCallbackArgs args);
} spCallbacks;

class ItemCallbacks
{
public:
	static int __cdecl SkillBonus(DispatcherCallbackArgs args);
} itemCallbacks;

class ClassAbilityCallbacks
{
public:
	// note: conditions obtained from feats always arg0 set to the feat enum (automatically)
	static int __cdecl FeatDamageReduction(DispatcherCallbackArgs args);
	static int __cdecl FeatEmptyBody(DispatcherCallbackArgs args); // radial menu builder
	static int __cdecl FeatEmptyBodyInit(DispatcherCallbackArgs args);
	static int __cdecl FeatEmptyBodyReduceRounds(DispatcherCallbackArgs);

	static int __cdecl FeatQuiveringPalmRadial(DispatcherCallbackArgs args); // radial menu builder
	static int __cdecl FeatQuiveringPalmInit(DispatcherCallbackArgs);
	static int __cdecl FeatQuiveringPalmPerform(DispatcherCallbackArgs args);
	static int __cdecl FeatQuiveringPalmAvailable(DispatcherCallbackArgs args);


	// Timed effect callbacks; assumption: num of rounds remaining is at arg[2]
	static int __cdecl GetNumRoundsRemaining(DispatcherCallbackArgs args);
	static int __cdecl TimedEffectCountdown(DispatcherCallbackArgs args);


	
} classAbilityCallbacks;

class GenericCallbacks
{
public:
	static int QuerySetReturnVal1(DispatcherCallbackArgs args);
	static int QuerySetReturnVal0(DispatcherCallbackArgs);
	static int ActionInvalidQueryTrue(DispatcherCallbackArgs);

	static int EffectTooltip(DispatcherCallbackArgs args); // SubDispDef data1 denotes the effect type idx, data2 denotes combat.mes line
	static int TooltipUnrepeated(DispatcherCallbackArgs); // SubDispDef data1 denotes combat.mes line

	static int AddEtherealDamageImmunity(DispatcherCallbackArgs args);
	static int EtherealOnAdd(DispatcherCallbackArgs args);
	static int EtherealOnD20StatusInit(DispatcherCallbackArgs args);
	static int EtherealDamageDealingNull(DispatcherCallbackArgs);
	static int EtherealOnRemove(DispatcherCallbackArgs);


	
} genericCallbacks;

CondNode::CondNode(CondStruct *cond) {
	memset(this, 0, sizeof(CondNode));
	condStruct = cond;
}


#pragma region Condition Add Functions

CondFeatDictionary::CondFeatDictionary(){
	featEnum = FEAT_NONE;
	featEnumMax = FEAT_NONE;
	condStruct.cs = nullptr;
	condArg = 0;
}

CondFeatDictionary::CondFeatDictionary(CondStructNew* cs, feat_enums Feat, feat_enums FeatMax, uint32_t arg){
	featEnum = Feat;
	featEnumMax = FeatMax;
	condStruct.cs = cs;
	condArg = arg;
}

int32_t _CondNodeGetArg(CondNode* condNode, uint32_t argIdx)
{
	return conds.CondNodeGetArg(condNode, argIdx);
}

void _CondNodeSetArg(CondNode* condNode, uint32_t argIdx, uint32_t argVal)
{
	conds.CondNodeSetArg(condNode, argIdx, argVal);
}

uint32_t _ConditionAddDispatch(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
	assert(condStruct->numArgs >= 0 && condStruct->numArgs <= 8);

	vector<int> args;
	if (condStruct->numArgs > 0) {
		args.push_back(arg1);
	}
	if (condStruct->numArgs > 1) {
		args.push_back(arg2);
	}
	if (condStruct->numArgs > 2) {
		args.push_back(arg3);
	}
	if (condStruct->numArgs > 3) {
		args.push_back(arg4);
	}


	return _ConditionAddDispatchArgs(dispatcher, ppCondNode, condStruct, args);
};

uint32_t _ConditionAddDispatchArgs(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, const vector<int> &args) {
	assert(condStruct->numArgs >= args.size());

	// pre-add section (may abort adding condition, or cause another condition to be deleted first)
	DispIoCondStruct dispIO14h;
	dispIO14h.dispIOType = dispIoTypeCondStruct;
	dispIO14h.condStruct = condStruct;
	dispIO14h.outputFlag = 1;
	dispIO14h.arg1 = 0;
	dispIO14h.arg2 = 0;
	if (args.size() > 0) {
		dispIO14h.arg1 = args[0];
	}
	if (args.size() > 1) {
		dispIO14h.arg2 = args[1];
	}

	_DispatcherProcessor(dispatcher, dispTypeConditionAddPre, 0, (DispIO*)&dispIO14h);

	if (dispIO14h.outputFlag == 0) {
		return 0;
	}

	// adding condition
	auto condNodeNew = new CondNode(condStruct);
	for (unsigned int i = 0; i < condStruct->numArgs; ++i) {
		if (i < args.size()) {
			condNodeNew->args[i] = args[i];
		} else {
			// Fill the rest with zeros
			condNodeNew->args[i] = 0;
		}
	}

	CondNode** ppNextCondeNode = ppCondNode;

	while (*ppNextCondeNode != nullptr) {
		ppNextCondeNode = &(*ppNextCondeNode)->nextCondNode;
	}
	*ppNextCondeNode = condNodeNew;

	_CondNodeAddToSubDispNodeArray(dispatcher, condNodeNew);


	auto dispatcherSubDispNodeType1 = dispatcher->subDispNodes[1];
	while (dispatcherSubDispNodeType1 != nullptr) {
		if (dispatcherSubDispNodeType1->subDispDef->dispKey == 0
			&& (dispatcherSubDispNodeType1->condNode->flags & 1) == 0
			&& condNodeNew == dispatcherSubDispNodeType1->condNode) {
			dispatcherSubDispNodeType1->subDispDef->dispCallback(dispatcherSubDispNodeType1, dispatcher->objHnd, dispTypeConditionAdd, 0, nullptr);
		}

		dispatcherSubDispNodeType1 = dispatcherSubDispNodeType1->next;
	}

	return 1;
};

void _CondNodeAddToSubDispNodeArray(Dispatcher* dispatcher, CondNode* condNode) {
	auto subDispDef = condNode->condStruct->subDispDefs;

	while (subDispDef->dispType != 0) {
		auto subDispNodeNew = (SubDispNode *)malloc(sizeof(SubDispNode));
		subDispNodeNew->subDispDef = subDispDef;
		subDispNodeNew->next = nullptr;
		subDispNodeNew->condNode = condNode;


		auto dispType = subDispDef->dispType;
		assert(dispType >= 0 && dispType < dispTypeCount);

		auto ppDispatcherSubDispNode = &(dispatcher->subDispNodes[dispType]);

		if (*ppDispatcherSubDispNode != nullptr) {
			while ((*ppDispatcherSubDispNode)->next != nullptr) {
				ppDispatcherSubDispNode = &((*ppDispatcherSubDispNode)->next);
			}
			(*ppDispatcherSubDispNode)->next = subDispNodeNew;
		}
		else {
			dispatcher->subDispNodes[subDispDef->dispType] = subDispNodeNew;
		}


		subDispDef += 1;
	}

};


uint32_t _ConditionAddToAttribs_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct) {
	return _ConditionAddDispatch(dispatcher, &dispatcher->permanentMods, condStruct, 0, 0, 0, 0);
};

uint32_t _ConditionAddToAttribs_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2) {
	return _ConditionAddDispatch(dispatcher, &dispatcher->permanentMods, condStruct, arg1, arg2, 0, 0);
};

uint32_t _ConditionAdd_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct) {
	return _ConditionAddDispatch(dispatcher, &dispatcher->conditions, condStruct, 0, 0, 0, 0);
};

uint32_t _ConditionAdd_NumArgs1(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1) {
	return _ConditionAddDispatch(dispatcher, &dispatcher->conditions, condStruct, arg1, 0, 0, 0);
};

uint32_t _ConditionAdd_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2) {
	return _ConditionAddDispatch(dispatcher, &dispatcher->conditions, condStruct, arg1, arg2, 0, 0);
};

uint32_t _ConditionAdd_NumArgs3(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
	return _ConditionAddDispatch(dispatcher, &dispatcher->conditions, condStruct, arg1, arg2, arg3, 0);
};

uint32_t _ConditionAdd_NumArgs4(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
	return _ConditionAddDispatch(dispatcher, &dispatcher->conditions, condStruct, arg1, arg2, arg3, arg4);
}

void InitCondFromCondStructAndArgs(Dispatcher* dispatcher, CondStruct* condStruct, int* condargs)
{
	conds.InitCondFromCondStructAndArgs(dispatcher, condStruct, condargs);
};

#pragma endregion

int ConditionPrevent(DispatcherCallbackArgs args)
{
	DispIoCondStruct * dispIO = _DispIoCheckIoType1((DispIoCondStruct*)args.dispIO);
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

int ConditionPreventWithArg(DispatcherCallbackArgs args)
{
	int arg1 = arg1 = conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	DispIoCondStruct *dispIo = dispatch.DispIoCheckIoType1((DispIoCondStruct *)args.dispIO);;
	
	if (dispIo->condStruct == (CondStruct *)args.subDispNode->subDispDef->data1 && dispIo->arg1 == arg1)
		dispIo->outputFlag = 0;
	return 0;
}

int CondResetArgs(DispatcherCallbackArgs args)
{
	memset(args.subDispNode->condNode->args, 0, args.subDispNode->condNode->condStruct->numArgs);
	return 0;
}
/*
used in BeginRound callback to update the remaining duration and remove condition when it ends
*/
int ConditionDurationTicker(DispatcherCallbackArgs args)
{
	auto argIdx = args.subDispNode->subDispDef->data1;
	auto condArg = _CondNodeGetArg(args.subDispNode->condNode, args.subDispNode->subDispDef->data1);
	auto dispIo = dispatch.DispIoCheckIoType6(args.dispIO);
	int durationRemaining = condArg - static_cast<int>(dispIo->data1);
	if (durationRemaining >= 0)
	{
		_CondNodeSetArg(args.subDispNode->condNode, argIdx, durationRemaining);
	} else
	{
		conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
	}
	return 0;
}

int ConditionRemoveCallback(DispatcherCallbackArgs args)
{
	conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
	return 0;
}

int RemoveSpellConditionAndMod(DispatcherCallbackArgs args)
{
	auto argsCopy = args;
	argsCopy.dispKey = DK_SIG_Action_Recipient;
	addresses.RemoveSpellCondition(argsCopy);
	addresses.RemoveSpellMod(argsCopy);
	return 0;
};

/*
data1 - argIdx (to store partsys ID)
data2 - char * to particle system name
*/
int PlayParticlesSavePartsysId(DispatcherCallbackArgs args)
{
	auto partsysName = reinterpret_cast<char*>(args.subDispNode->subDispDef->data2);
	if (partsysName)
	{
		auto partsysId = gameSystems->GetParticleSys().CreateAtObj(partsysName, args.objHndCaller);
		_CondNodeSetArg(args.subDispNode->condNode,
			args.subDispNode->subDispDef->data1, partsysId);
	}
	return 0;
}

int EndParticlesFromArg(DispatcherCallbackArgs args)
{
	auto partsysId = _CondNodeGetArg(args.subDispNode->condNode, args.subDispNode->subDispDef->data1);
	if (partsysId)
	{
		gameSystems->GetParticleSys().End(partsysId);
		_CondNodeSetArg(args.subDispNode->condNode,
			args.subDispNode->subDispDef->data1, 0);
	}
	return 0;
}

int DivineMightEffectTooltipCallback(DispatcherCallbackArgs args)
{
	void * dispIo = args.dispIO;
	int (__cdecl* callback )(int, int, int)= (int(__cdecl*)(int, int, int))temple::GetPointer(0x100F4760);
	const char shit[] = "Divine Might";
	callback( *((int*)dispIo + 1), args.subDispNode->subDispDef->data1, (int)shit);
	return 0;
};

/*
gets a tooltip string from combat.mes
*/
int GenericCallbacks::TooltipUnrepeated(DispatcherCallbackArgs args)
{
	DispIoTooltip *dispIo = dispatch.DispIoCheckIoType9(args.dispIO);
	auto mesLine = combatSys.GetCombatMesLine(args.subDispNode->subDispDef->data1);
	int numstrings = dispIo->numStrings;
	if (numstrings >= 10)
		return 0;
	int idx = 0;
	for (idx = 0; idx < numstrings && idx < 10 ; idx++)
	{
		if (!strcmp(dispIo->strings[idx], mesLine))
			break;
	}
	if (idx == numstrings) // reached the end and not found
	{
		strncpy(dispIo->strings[numstrings], mesLine, 0x100);
		dispIo->numStrings++;
	}
	return 0;
};



int __cdecl CondNodeSetArg0FromSubDispDef(DispatcherCallbackArgs args)
{
	conds.CondNodeSetArg(args.subDispNode->condNode, 0, args.subDispNode->subDispDef->data1);
	return 0;
}

int QueryCritterHasCondition(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	if (dispIo->data1 == args.subDispNode->subDispDef->data1 && !dispIo->data2)
	{
		dispIo->return_val = 1;
		dispIo->data1 = _CondNodeGetArg(args.subDispNode->condNode, 0); //spellId
		dispIo->data2 = 0;
	}
	return 0;
}


int GenericCallbacks::QuerySetReturnVal1(DispatcherCallbackArgs args)
{
	DispIoD20Query * dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	dispIo->return_val = 1;
	return 0;
}

int GenericCallbacks::QuerySetReturnVal0(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	dispIo->return_val = 0;
	return 0;
}

int GenericCallbacks::ActionInvalidQueryTrue(DispatcherCallbackArgs args){
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	dispIo->return_val = 1;
	return 0;
}

int GenericCallbacks::AddEtherealDamageImmunity(DispatcherCallbackArgs args){
	auto dispIo = dispatch.DispIoCheckIoType4(args.dispIO);
	dispIo->damage.AddEtherealImmunity();
	return 0;
}

int GenericCallbacks::EtherealOnAdd(DispatcherCallbackArgs args)
{
	floatSys.FloatCombatLine(args.objHndCaller, 210); // Ethereal
	objects.FadeTo(args.objHndCaller, 70, 10, 30, 0);
	return 0;
}

int GenericCallbacks::EtherealOnD20StatusInit(DispatcherCallbackArgs args){
	objects.FadeTo(args.objHndCaller, 70, 10, 30, 0);
	return 0;
}

int GenericCallbacks::EtherealDamageDealingNull(DispatcherCallbackArgs args){
	auto dispIo = dispatch.DispIoCheckIoType4(args.dispIO);
	dispIo->damage.AddEtherealImmunity();
	return 0;
}

int GenericCallbacks::EtherealOnRemove(DispatcherCallbackArgs args)
{
	objects.FadeTo(args.objHndCaller, 255, 0, 5, 0);
	return 0;
}


int GenericCallbacks::EffectTooltip(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType24(args.dispIO);
	auto numRounds = args.GetCondArg(2);

	dispIo->Append( args.GetData1(), -1, fmt::format("{}\n{}: {}",combatSys.GetCombatMesLine(args.GetData2()), combatSys.GetCombatMesLine(175), numRounds).c_str() );
	return 0;
}

int QuerySetReturnVal0(DispatcherCallbackArgs args)
{
	DispIoD20Query * dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	dispIo->return_val = 0;
	return 0;
};

int QueryRetrun1GetArgs(DispatcherCallbackArgs args)
{
	DispIoD20Query * dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	dispIo->return_val = 1;
	dispIo->data1 = conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	dispIo->data2 = conds.CondNodeGetArg(args.subDispNode->condNode, 1);
	return 0;
};

int DisarmedRetrieveQuery(DispatcherCallbackArgs args)
{
	DispIoD20Query * dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	dispIo->return_val = 1;
	objHndl weapon;
	ObjectId objId;
	memcpy(&objId, args.subDispNode->condNode->args, sizeof(ObjectId));
	weapon = gameSystems->GetObj().GetHandleById(objId);
	*(uint64_t*)&dispIo->data1 = weapon;
	return 0;
};

int __cdecl CondNodeSetArgFromSubDispDef(DispatcherCallbackArgs args)
{
	// sets arg[data1] from data2  
	// e.g. IF data1 = 0, data2 = 15 
	//    THEN it'll set arg0 = 15
	conds.CondNodeSetArg(args.subDispNode->condNode, args.subDispNode->subDispDef->data1,
		args.subDispNode->subDispDef->data2);
	return 0;
};


int __cdecl CondArgDecrement(DispatcherCallbackArgs args)
{
	conds.CondNodeSetArg(args.subDispNode->condNode, args.subDispNode->subDispDef->data1, 
		conds.CondNodeGetArg(args.subDispNode->condNode, args.subDispNode->subDispDef->data1) - 1);
	return 0;
};

int __cdecl ItemSkillBonusCallback(DispatcherCallbackArgs args)
{
	/*
	used by conditions: Skill Circumstance Bonus, Skill Competence Bonus
	*/
	SkillEnum skillEnum = (SkillEnum)conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	int bonValue = conds.CondNodeGetArg(args.subDispNode->condNode, 1);
	int bonType = args.subDispNode->subDispDef->data1;
	if (args.dispKey - 20 == skillEnum)
	{
		int invIdx = conds.CondNodeGetArg(args.subDispNode->condNode, 2);
		objHndl itemHnd = inventory.GetItemAtInvIdx(args.objHndCaller, invIdx);
		DispIoBonusAndObj * dispIo = dispatch.DispIoCheckIoType10((DispIoBonusAndObj*)args.dispIO);
		const char * name = description.getDisplayName(itemHnd, args.objHndCaller);
		bonusSys.bonusAddToBonusListWithDescr(dispIo->bonOut, bonValue, bonType, 112, (char*)name);
	}
	return 0;
}


int __cdecl GlobalToHitBonus(DispatcherCallbackArgs args)
{
	DispIoAttackBonus * dispIo = dispatch.DispIoCheckIoType5((DispIoAttackBonus*)args.dispIO);
	dispatch.DispatchToHitBonusBase(args.objHndCaller, dispIo);

	// natural attack - get attack bonus from internal defs
	if (dispIo->attackPacket.dispKey >= (ATTACK_CODE_NATURAL_ATTACK+1) 
		 && !d20Sys.d20Query(args.objHndCaller, DK_QUE_Polymorphed) )
	{
		int attackIdx = dispIo->attackPacket.dispKey - (ATTACK_CODE_NATURAL_ATTACK + 1);
		int bonValue = 0; // temporarily used as an index value for obj_f_attack_bonus_idx field
		for (int i = 0,  j=0; i < 4; i++)
		{
			j += objects.getArrayFieldInt32(args.objHndCaller, obj_f_critter_attacks_idx, i); // number of attacks
			if (attackIdx < j){
				bonValue = i;
				break;
			}
		}
		bonValue = objects.getArrayFieldInt32(args.objHndCaller, obj_f_attack_bonus_idx, bonValue);
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, bonValue, 1, 118); // base attack
	}

	if (dispIo->attackPacket.flags & D20CAF_RANGED) // get to hit modifier from DEX
	{
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 
			objects.GetModFromStatLevel(objects.abilityScoreLevelGet(args.objHndCaller, stat_dexterity, 0)) , 3, 104);
	} else // get to hit mod from STR
	{
		bonusSys.bonusAddToBonusList(&dispIo->bonlist,
			objects.GetModFromStatLevel(objects.abilityScoreLevelGet(args.objHndCaller, stat_strength, 0)), 2, 103);
	}

	int attackCode = dispIo->attackPacket.dispKey;
	if (attackCode < ATTACK_CODE_NATURAL_ATTACK) // apply penalties for Nth attack
	{
		int attackNumber = 1;
		int dualWielding = 0;
		d20Sys.ExtractAttackNumber(args.objHndCaller, attackCode, &attackNumber, &dualWielding);
		if (attackNumber <= 0)
		{
			int dummy = 1;
			assert(attackNumber > 0);
		}
		switch (attackNumber)
		{
		case 1: 
			break;
		case 2:
			bonusSys.bonusAddToBonusList(&dispIo->bonlist, -(attackNumber-1) * 5, 24, 119);
			break;
		default:
			bonusSys.bonusAddToBonusList(&dispIo->bonlist, -(attackNumber - 1) * 5, 25, 120);
		}
		if (dualWielding)
		{
			if (d20Sys.UsingSecondaryWeapon(args.objHndCaller, attackCode))
				bonusSys.bonusAddToBonusList(&dispIo->bonlist, -10, 26, 121); // penalty for dualwield on offhand attack
			else
				bonusSys.bonusAddToBonusList(&dispIo->bonlist, -6, 27, 122); // penalty for dualwield on primary attack

			auto offhand = inventory.ItemWornAt(args.objHndCaller, 4);
			if (offhand)
			{
				if (inventory.GetWieldType(dispIo->attackPacket.attacker, offhand) == 0)
					bonusSys.bonusAddToBonusList(&dispIo->bonlist, 2, 0, 167); // Light Off-hand Weapon
			}
		}
	}

	// helplessness bonus
	if (dispIo->attackPacket.victim
		&& d20Sys.d20Query(dispIo->attackPacket.victim, DK_QUE_Helpless)
		&& !d20Sys.d20Query(dispIo->attackPacket.victim, DK_QUE_Critter_Is_Stunned))
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 4, 30, 136);
	
	// flanking bonus
	if (combatSys.IsFlankedBy(dispIo->attackPacket.victim, dispIo->attackPacket.attacker))
	{
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 2, 0, 201);
		*(int*)(&dispIo->attackPacket.flags ) |= (int)D20CAF_FLANKED;
	}

	// size bonus / penalty
	int sizeCategory = dispatch.DispatchGetSizeCategory(args.objHndCaller);
	int sizeCatBonus = critterSys.GetBonusFromSizeCategory(sizeCategory);
	bonusSys.bonusAddToBonusList(&dispIo->bonlist, sizeCatBonus, 0, 115);

	if (dispIo->attackPacket.flags & D20CAF_RANGED)
	{

		if (dispIo->attackPacket.victim)
		{
			// firing into melee penalty
			objHndl canMeleeList[100];

			int numEnemiesCanMelee = combatSys.GetEnemiesCanMelee(dispIo->attackPacket.victim, canMeleeList);
			if (numEnemiesCanMelee > 0
				&& (numEnemiesCanMelee != 1 || canMeleeList[0]!= args.objHndCaller)
				&& !feats.HasFeatCount(args.objHndCaller, FEAT_PRECISE_SHOT))
				bonusSys.bonusAddToBonusList(&dispIo->bonlist, -4, 0, 150); 
		
			// range penalty 
			objHndl weapon = combatSys.GetWeapon(&dispIo->attackPacket);
			float dist = locSys.DistanceToObj(args.objHndCaller, dispIo->attackPacket.victim);
			if (dist < 0.0) dist = 0.0;
			if (weapon)
			{
				long double weaponRange = (long double)objects.getInt32(weapon, obj_f_weapon_range);
				int rangePenaltyFacotr = (int)(dist / weaponRange);
				if ((int)rangePenaltyFacotr > 0)
					bonusSys.bonusAddToBonusList(&dispIo->bonlist, -2 * rangePenaltyFacotr, 0, 303);
			}
		}
	}

	return 0;
}

int GlobalGetArmorClass(DispatcherCallbackArgs args) // the basic AC value (initial value and some misc. global modifiers)
{
	DispIoAttackBonus * dispIo = dispatch.DispIoCheckIoType5((DispIoAttackBonus*)args.dispIO);
	BonusList *bonlist = &dispIo->bonlist;

	// Armor Class initial value 10
	bonusSys.bonusAddToBonusList(bonlist, 10, 1, 102); 
	
	// add npc natural armor AC bonus
	if (!(dispIo->attackPacket.flags & D20CAF_TOUCH_ATTACK))
	{
		objHndl defender = args.objHndCaller;
		int polymorphedTo = d20Sys.d20Query(args.objHndCaller, DK_QUE_Polymorphed);
		if (polymorphedTo)
			defender = objects.GetProtoHandle(polymorphedTo);
		if (objects.GetType(args.objHndCaller) == obj_t_npc || polymorphedTo)
		{
			bonusSys.bonusAddToBonusList(bonlist, objects.getInt32(defender, obj_f_npc_ac_bonus), 9, 123);
		}
	}

	// add size bonus / penalty to AC
	int sizeCat = dispatch.DispatchGetSizeCategory(args.objHndCaller);
	int sizeCatBonus = critterSys.GetBonusFromSizeCategory(sizeCat);
	bonusSys.bonusAddToBonusList(bonlist, sizeCatBonus, 0, 115); 

	// dex bonus
	int dexScore = objects.abilityScoreLevelGet(args.objHndCaller, stat_dexterity, 0);
	int dexMod = objects.GetModFromStatLevel(dexScore);
	bonusSys.bonusAddToBonusList(bonlist, dexMod, 3, 104);

	// dodging trap
	if (dispIo->attackPacket.flags & D20CAF_TRAP)
	{
		if (dispIo->attackPacket.victim && feats.HasFeatCount(dispIo->attackPacket.victim, FEAT_UNCANNY_DODGE))
		{
			bonusSys.zeroBonusSetMeslineNum(bonlist, 165); // dex bonus retained due to Uncanny Dodge
			return 0;
		}
		bonusSys.bonusCapAdd(bonlist, 8, 0, 0x99u); // flatfooted
		bonusSys.bonusCapAdd(bonlist, 3, 0, 0x99u);
	}

	if (dispIo->attackPacket.flags & D20CAF_COVER)
	{
		if (dispIo->attackPacket.attacker && feats.HasFeatCount(dispIo->attackPacket.attacker, FEAT_IMPROVED_PRECISE_SHOT))
		{
			bonusSys.zeroBonusSetMeslineNum(bonlist, 335); // Cover negated by Imp. Precise Shot
			return 0;
		}

		if (dispIo->attackPacket.attacker && feats.HasFeatCountByClass(dispIo->attackPacket.attacker, FEAT_IMPROVED_PRECISE_SHOT_RANGER, (Stat)0, 0))
		{
			if (critterSys.IsWearingLightOrNoArmor(dispIo->attackPacket.attacker))
			{
				bonusSys.zeroBonusSetMeslineNum(bonlist, 335); // Cover negated by Imp. Precise Shot
				return 0;
			}
						
		}

		if (dispIo->attackPacket.attacker && feats.HasFeatCount(dispIo->attackPacket.attacker, FEAT_SHARP_SHOOTING))
		{
			bonusSys.bonusAddToBonusList(bonlist, 2, 0, 336);; // Cover (diminished by Sharp-Shooting)
			return 0;
		}
		bonusSys.bonusAddToBonusList(bonlist, 4, 0, 309);
	}
		
	return 0;
}


int __cdecl GlobalOnDamage(DispatcherCallbackArgs args)
{
	DispIoDamage * dispIo = dispatch.DispIoCheckIoType4(args.dispIO);
	objHndl weapon = combatSys.GetWeapon(&dispIo->attackPacket);
	int polymorphedTo = d20Sys.d20Query(args.objHndCaller, DK_QUE_Polymorphed);
	objHndl v34 = args.objHndCaller;
	DispIoAttackDice dispIoAttackDice;
	int attackDice;
	DamageType attackDamageType = DamageType::Bludgeoning;
	const char * weaponName = feats.emptyString;
	int damageMesLine = 100; // ~Weapon~[TAG_WEAPONS]

	if (polymorphedTo)
	{
		v34 = objects.GetProtoHandle(polymorphedTo);
	}

	if (weapon)
	{
		dispIoAttackDice.flags = dispIo->attackPacket.flags;
		dispIoAttackDice.wielder = args.objHndCaller;
		dispIoAttackDice.weapon = weapon;
		attackDice = dispatch.Dispatch60GetAttackDice(args.objHndCaller, &dispIoAttackDice);
		attackDamageType = dispIoAttackDice.attackDamageType;
		weaponName = description.getDisplayName(weapon, args.objHndCaller);
	} 
	else // unarmed
	{
		int attackCode = dispIo->attackPacket.dispKey;
		if (attackCode > ATTACK_CODE_NATURAL_ATTACK ) // natural attack
		{
			int attackIdx = attackCode - (ATTACK_CODE_NATURAL_ATTACK + 1);
			int attackDiceUnarmed = critterSys.GetCritterDamageDice(v34, attackIdx);
			
			damageMesLine = critterSys.GetCritterAttackType(v34, attackIdx) + 114;
			attackDamageType = critterSys.GetCritterAttackDamageType(v34, attackIdx);
			dispIoAttackDice.flags = dispIo->attackPacket.flags;
			dispIoAttackDice.weapon = 0i64;
			dispIoAttackDice.wielder = args.objHndCaller;
			dispIoAttackDice.dicePacked = attackDiceUnarmed;
			dispIoAttackDice.attackDamageType = attackDamageType;
			attackDice = dispatch.Dispatch60GetAttackDice(args.objHndCaller, &dispIoAttackDice);
		} 
		else
		{
			int monkLvl = objects.StatLevelGet(args.objHndCaller, stat_level_monk);
			attackDamageType = DamageType::Bludgeoning;
			damageMesLine = 113; // Unarmed

			auto beltItem = inventory.ItemWornAt(args.objHndCaller, EquipSlot::Lockpicks);
			if (beltItem){
				auto beltObj = gameSystems->GetObj().GetObject(beltItem);
				if (beltObj->protoId == 12420){
					monkLvl += 5;
				}
			}

			int attackDiceType = 3;
			int attackDiceCount = 1;
			int dudeSize = objects.StatLevelGet(args.objHndCaller, stat_size);
			if (dudeSize< 5) // small monk
			{
				if (monkLvl <= 0)
				{
					attackDiceType = 2;
					attackDamageType = DamageType::Subdual;
				} else if (monkLvl < 4)
				{
					attackDiceType = 4;
				} else 
				if (monkLvl < 8)
				{
					attackDiceType = 6;
				}
				else if (monkLvl < 12)
				{
					attackDiceType = 8;
				}
				else if (monkLvl < 16)
				{
					attackDiceCount = 1;
					attackDiceType = 10;
				} else if (monkLvl < 20)
				{
					attackDiceCount = 2;
					attackDiceType = 6;
				} else // 20 and above
				{
					attackDiceCount = 2;
					attackDiceType = 8;
				}
			}
			else if (dudeSize > 5) // Large Monk
			{
				if (monkLvl <= 0)
				{
					attackDiceType = 4;
					attackDamageType = DamageType::Subdual;
				}
				else if (monkLvl < 4)
				{
					attackDiceType = 8;
				}
				else if (monkLvl < 8)
				{
					attackDiceCount = 2;
					attackDiceType = 6;
				}
				else if (monkLvl < 12)
				{
					attackDiceCount = 2;
					attackDiceType = 8;
				}
				else if (monkLvl < 16)
				{
					attackDiceCount = 3;
					attackDiceType = 6;
				}
				else if (monkLvl < 20)
				{
					attackDiceCount = 3;
					attackDiceType = 8;
				}
				else
				{
					attackDiceCount = 4;
					attackDiceType = 8;
				}
			}
			else // normal monk
			{
				if (monkLvl <= 0)
				{
					attackDiceType = 3;
					attackDamageType = DamageType::Subdual;
				} else if (monkLvl < 4)
				{
					attackDiceType = 6;
				} else 
				if (monkLvl < 8)
				{
					attackDiceType = 8;
				}
				else if (monkLvl < 12)
				{
					attackDiceType = 10;
				}
				else if (monkLvl < 16)
				{
					attackDiceCount = 2;
					attackDiceType = 6;
				} else if (monkLvl < 20)
				{
					attackDiceCount = 2;
					attackDiceType = 8;
				} else
				{
					attackDiceCount = 2;
					attackDiceType = 10;
				}

			}

			Dice diceUnarmed(attackDiceCount, attackDiceType, 0);
			attackDice = diceUnarmed.ToPacked();

		}

	}
	damage.AddDamageDice(&dispIo->damage, attackDice, attackDamageType, damageMesLine);

	int strScore = objects.StatLevelGet(args.objHndCaller, stat_strength);
	int strMod = objects.GetModFromStatLevel(strScore);
	D20CAF flags = dispIo->attackPacket.flags;
	if (!(flags & D20CAF_RANGED) || (flags & D20CAF_THROWN))
	{
		int attackCode = dispIo->attackPacket.dispKey;
		if (d20Sys.UsingSecondaryWeapon(args.objHndCaller, attackCode))
		{
			if (strMod > 0)
			{
				strMod /= 2;
			}
				
		} else if (d20Sys.d20QueryWithData(args.objHndCaller, DK_QUE_WieldedTwoHanded, (int)dispIo, 0) && strMod > 0 && inventory.GetWieldType(args.objHndCaller, weapon))
		{
			strMod += strMod / 2;
		}
		if (attackCode > ATTACK_CODE_NATURAL_ATTACK && strMod > 0 && critterSys.GetDamageIdx(args.objHndCaller, attackCode - (ATTACK_CODE_NATURAL_ATTACK + 1)) > 0)
		{
			strMod /= 2;
		}
		damage.AddDamageBonusWithDescr(&dispIo->damage, strMod, 2, 103, 0);
		return 0;
	}
	if (!weapon)
		return 0;
	auto weaponType = objects.GetWeaponType(weapon);
	if (weaponType == wt_sling || ((weaponType == wt_shortbow ||weaponType == wt_longbow )&& strMod < 0))
	{
		damage.AddDamageBonusWithDescr(&dispIo->damage, strMod, 2, 103, 0);
	}
	return 0;
}


int CaptivatingSongOnConditionAdd(DispatcherCallbackArgs args);

int __cdecl TurnBasedStatusInitNoActions(DispatcherCallbackArgs args){
	auto dispIo = dispatch.DispIoCheckIoType8(args.dispIO);
	if (dispIo){
		auto tbStat = dispIo->tbStatus;
		if (tbStat){
			tbStat->hourglassState = 0;
			dispIo->tbStatus->tbsFlags |= TurnBasedStatusFlags::TBSF_Movement;
			logger->debug("Zeroed actions for {}", description.getDisplayName(args.objHndCaller));
		}
	}
	return 0;
}

int __cdecl CaptivatingSongEffectTooltipDuration(DispatcherCallbackArgs args){
	
	auto dispIo = dispatch.DispIoCheckIoType24(args.dispIO);
	int durationRemaining = _CondNodeGetArg(args.subDispNode->condNode, 0);
	char tooltipString[256];
	sprintf(tooltipString, "\n%d rounds remaining.", durationRemaining);
	auto effectTooltipBase = temple::GetRef<int(__cdecl)(BuffDebuffPacket*, int someIdx, int spellEnum, char*)>(0x100F4680);
	effectTooltipBase(dispIo->bdb, 100, 20054, tooltipString); // will fetch 20054 from spell.mes (Captivated!)
	return 0;
}


class Conditions
{
public:
	static void AddConditionsToTable();
	std::map<feat_enums,CondFeatDictionary> condDict;
} conditions;

void _FeatConditionsRegister()
{

	// In moebiues DLL the condition table was moved and extended
	auto condCount = 84;
	if (temple::Dll::GetInstance().IsVanillaDll()) {
		conds.FeatConditionDict = temple::GetPointer<CondFeatDictionary>(0x102EEC40);
		condCount = 79;
	}

	conds.hashmethods.CondStructAddToHashtable(conds.ConditionAttackOfOpportunity);
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionCastDefensively);
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionCombatCasting);
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionDealSubdualDamage );
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionDealNormalDamage);
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionFightDefensively);
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionAnimalCompanionAnimal);
	conds.hashmethods.CondStructAddToHashtable(conds.ConditionAutoendTurn);


	// Craft Wand
	static CondStructNew craftWand("Craft Wand", 0);
	craftWand.AddHook(dispTypeRadialMenuEntry, DK_NONE, CraftWandOnAdd);

	conds.FeatConditionDict[61].condStruct.cs = &craftWand;

	for (unsigned int i = 0; i < condCount; i++){
		conds.hashmethods.CondStructAddToHashtable(conds.FeatConditionDict[i].condStruct.old);
	}
}

uint32_t  _GetCondStructFromFeat(feat_enums featEnum, CondStruct ** condStructOut, uint32_t * argout)
{
	switch (featEnum)
	{
	case FEAT_GREATER_TWO_WEAPON_FIGHTING:
		*condStructOut = (CondStruct*)conds.mCondGreaterTwoWeaponFighting;
		*argout = 0;
		return 1;
	case FEAT_GREATER_TWO_WEAPON_FIGHTING_RANGER:
		*condStructOut = (CondStruct*)conds.mCondGreaterTWFRanger;
		*argout = 0;
		return 1;
	case FEAT_DIVINE_MIGHT:
		*condStructOut = (CondStruct*)conds.mCondDivineMight;
		*argout = 0;
		return 1;
	case FEAT_RECKLESS_OFFENSE:
		*condStructOut = (CondStruct*)conds.mCondRecklessOffense;
		*argout = 0;
		return 1;
	case FEAT_KNOCK_DOWN:
		*condStructOut = (CondStruct*)conds.mCondKnockDown;
		*argout = 0;
		return 1;
	case FEAT_SUPERIOR_EXPERTISE:
		return 0; // willl just be patched inside Combat Expertise
	case FEAT_DEADLY_PRECISION:
		*condStructOut = (CondStruct*)conds.mCondDeadlyPrecision;
		*argout = 0;
		return 1;
	case FEAT_PERSISTENT_SPELL:
		*condStructOut = (CondStruct*)conds.mCondPersistentSpell;
		*argout = 0;
		return 1;
	default:
		break;
	}

	if (featEnum >= FEAT_GREATER_WEAPON_SPECIALIZATION_GAUNTLET && featEnum <= FEAT_GREATER_WEAPON_SPECIALIZATION_GRAPPLE)
	{
		*condStructOut = (CondStruct*)conds.mCondGreaterWeaponSpecialization;
		*argout = featEnum - FEAT_GREATER_WEAPON_SPECIALIZATION_GAUNTLET;
		return 1;
	}
	
	// Search the new Feat-CondStruct dictionary
	auto it = conditions.condDict.find(featEnum);
	if (it != conditions.condDict.end()){
		if (it->second.featEnum == featEnum && it->second.condStruct.cs != nullptr){
			*condStructOut = static_cast<CondStruct*>(it->second.condStruct.old);
			*argout = it->second.condArg;
			return 1;
		}
	};

	feat_enums * featFromDict = & ( conds.FeatConditionDict->featEnum );
	uint32_t iter = 0;
	while (
		( (int32_t)featEnum != featFromDict[0] || featFromDict[1] != -1)
		&&  ( (int32_t)featEnum < (int32_t)featFromDict[0] 
				|| (int32_t)featEnum >= (int32_t)featFromDict[1]  )
		)
	{
		iter += 16;
		featFromDict += 4;
		if (iter >= 0x540){ return 0; }
	}

	*condStructOut = (CondStruct *)*(featFromDict - 1);
	*argout = featEnum + featFromDict[2] - featFromDict[0];
	return 1;
}

uint32_t _CondStructAddToHashtable(CondStruct * condStruct)
{
	return conds.hashmethods.CondStructAddToHashtable(condStruct);
}

CondStruct * _GetCondStructFromHashcode(uint32_t key)
{
	return conds.hashmethods.GetCondStruct(key);
}

CondStruct* ConditionSystem::GetByName(const string& name) {
	auto key = templeFuncs.StringHash(name.c_str());
	return hashmethods.GetCondStruct(key);
}

void ConditionSystem::AddToItem(objHndl item, const CondStruct* cond, const vector<int>& args) {
	assert(args.size() == cond->numArgs);

	auto obj = objSystem->GetObject(item);
	auto curCondCount = obj->GetInt32Array(obj_f_item_pad_wielder_condition_array).GetSize();
	auto curCondArgCount = obj->GetInt32Array(obj_f_item_pad_wielder_argument_array).GetSize();

	// Add the condition name hash to the list
	auto key = templeFuncs.StringHash(cond->condName);
	obj->SetInt32(obj_f_item_pad_wielder_condition_array, curCondCount, key);

	auto idx = curCondArgCount;
	for (auto arg : args) {
		obj->SetInt32(obj_f_item_pad_wielder_argument_array, idx, arg);
		idx++;
	}
}

bool ConditionSystem::AddTo(objHndl handle, const CondStruct* cond, const vector<int>& args) {
	assert(args.size() == cond->numArgs);

	auto dispatcher = objects.GetDispatcher(handle);

	if (!dispatch.dispatcherValid(dispatcher)) {
		logger->info("Dispatcher invalid for {}", objects.description.getDisplayName(handle));
		return false;
	}

	return _ConditionAddDispatchArgs(dispatcher, &dispatcher->conditions, const_cast<CondStruct*>(cond), args) != 0;
}

bool ConditionSystem::AddTo(objHndl handle, const string& name, const vector<int>& args) {
	auto cond = GetByName(name);
	if (!cond) {
		logger->warn("Unable to find condition {}", name);
		return false;
	}

	return AddTo(handle, cond, args);
}

bool ConditionSystem::ConditionAddDispatchArgs(Dispatcher* dispatcher, CondNode** nodes, CondStruct* condStruct, const vector<int>& args)
{
	return _ConditionAddDispatchArgs(dispatcher, nodes, condStruct, args);

}

int32_t ConditionSystem::CondNodeGetArg(CondNode* condNode, uint32_t argIdx)
{
	if (argIdx < condNode->condStruct->numArgs)
	{
		return condNode->args[argIdx];
	}
	return 0;
}

void ConditionSystem::CondNodeSetArg(CondNode* condNode, uint32_t argIdx, uint32_t argVal)
{
	if (argIdx < condNode->condStruct->numArgs)
	{
		condNode->args[argIdx] = argVal;
	}
}

void ConditionSystem::CondNodeAddToSubDispNodeArray(Dispatcher* dispatcher, CondNode* condNodeNew)
{
	_CondNodeAddToSubDispNodeArray(dispatcher, condNodeNew);
}

void ConditionSystem::InitCondFromCondStructAndArgs(Dispatcher* dispatcher, CondStruct* condStruct, int* condargs)
{
	CondNode **v4; 
	SubDispNode *subDispNode; 
	CondNode *condNode; 

	auto *condNodeNew = new CondNode(condStruct);
	v4 = &dispatcher->conditions;
	while (*v4)
	{
		v4 = & (*v4)->nextCondNode;
	}
	*v4 = condNodeNew;

	for (auto i = 0; i < condStruct->numArgs; i++)
	{
		condNodeNew->args[i] = condargs[i];
	}
	conds.CondNodeAddToSubDispNodeArray(dispatcher, condNodeNew);
	for (subDispNode = dispatcher->subDispNodes[dispTypeConditionAddFromD20StatusInit]; subDispNode; subDispNode = subDispNode->next)
	{
		if (subDispNode->subDispDef->dispKey == 0)
		{
			condNode = subDispNode->condNode;
			if (!(condNode->flags & 1) && condNode == condNodeNew)
				subDispNode->subDispDef->dispCallback(subDispNode,dispatcher->objHnd, dispTypeConditionAddFromD20StatusInit, 0, nullptr);
		}
	}
}

void ConditionSystem::RegisterNewConditions()
{

	CondStructNew * cond;
	char * condName;

#pragma region Feats

	// Disable AoO
	mConditionDisableAoO = &conditionDisableAoO;
	cond = mConditionDisableAoO; 	condName = mConditionDisableAoOName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Disable AoO", sizeof("Disable AoO"));

	cond->condName = mConditionDisableAoOName;
	cond->numArgs = 1;

	DispatcherHookInit(cond, 0, dispTypeD20Query, DK_QUE_AOOPossible, AoODisableQueryAoOPossible,	0, 0);
	DispatcherHookInit(cond, 1, dispTypeRadialMenuEntry, 0, AoODisableRadialMenuInit, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)mConditionDisableAoO, 0);
	DispatcherHookInit(cond, 3, dispType0, 0, 0, 0, 0);

	// Greater Two Weapon Fighting
	mCondGreaterTwoWeaponFighting = &conditionGreaterTwoWeaponFighting;
	cond = mCondGreaterTwoWeaponFighting; 	condName = mConditionGreaterTwoWeaponFightingName;
	memset(condName, 0, sizeof(condName));
	memcpy(condName, "Greater Two Weapon Fighting", sizeof("Greater Two Weapon Fighting"));

	cond->condName = mConditionGreaterTwoWeaponFightingName;
	cond->numArgs = 2;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)mCondGreaterTwoWeaponFighting, 0);
	DispatcherHookInit(cond, 1, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)mCondGreaterTWFRanger, 0);
	DispatcherHookInit(cond, 2, dispTypeGetNumAttacksBase, 0, GreaterTwoWeaponFighting, 0, 0); // same callback as Improved TWF (it just adds an extra attack... logic is inside the action sequence / d20 / GlobalToHit functions
	DispatcherHookInit(cond, 3, dispType0, 0, nullptr, 0, 0);

	// Greater TWF Ranger
	mCondGreaterTWFRanger = &condGreaterTWFRanger;
	cond = mCondGreaterTWFRanger; 	condName = mCondGreaterTWFRangerName;
	memset(condName, 0, sizeof(condName));
	memcpy(condName, "Greater Two Weapon Fighting Ranger", sizeof("Greater Two Weapon Fighting Ranger"));

	cond->condName = mCondGreaterTWFRangerName;
	cond->numArgs = 2;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)mCondGreaterTwoWeaponFighting, 0);
	DispatcherHookInit(cond, 1, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)mCondGreaterTWFRanger, 0); // TODO: add TWF_RANGER
	DispatcherHookInit(cond, 2, dispTypeGetNumAttacksBase, 0, GreaterTWFRanger, 0, 0); // same callback as Improved TWF (it just adds an extra attack... logic is inside the action sequence / d20 / GlobalToHit functions
	DispatcherHookInit(cond, 3, dispType0, 0, nullptr, 0, 0);

	// Divine Might Ability
	mCondDivineMight = &condDivineMight;
	cond = mCondDivineMight; 	condName = mCondDivineMightName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Divine Might", sizeof("Divine Might"));

	cond->condName = condName;
	cond->numArgs = 2;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0 ,ConditionPrevent, (uint32_t) cond , 0 );
	DispatcherHookInit(cond, 1, dispTypeConditionAdd, 0, CondNodeSetArg0FromSubDispDef, 1, 0);
	DispatcherHookInit(cond, 2, dispTypeRadialMenuEntry, 0, DivineMightRadial, 0, 0);

	DispatcherHookInit((CondStructNew*)ConditionTurnUndead, 6, dispTypeD20ActionPerform, DK_D20A_DIVINE_MIGHT, CondArgDecrement, 1, 0); // decrement the number of turn charges remaining; 
	DispatcherHookInit((CondStructNew*)ConditionGreaterTurning, 6, dispTypeD20ActionPerform, DK_D20A_DIVINE_MIGHT, CondArgDecrement, 1, 0); // decrement the number of turn charges remaining
	
	// Divine Might Bonus (gets activated when you choose the action from the Radial Menu)
	mCondDivineMightBonus = &condDivineMightBonus;
	cond = mCondDivineMightBonus; 	condName = mCondDivineMightBonusName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Divine Might Bonus", sizeof("Divine Might Bonus"));

	cond->condName = condName;
	cond->numArgs = 2;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeDealingDamage, 0, DivineMightDamageBonus, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeBeginRound, 0, ConditionRemoveCallback, 0, 0);
	DispatcherHookInit(cond, 3, dispTypeD20Signal, DK_SIG_Killed, ConditionRemoveCallback, 0, 0);
	DispatcherHookInit(cond, 4, dispTypeEffectTooltip, 0, DivineMightEffectTooltipCallback, 81, 0);

	// Reckless Offense
	mCondRecklessOffense = &condRecklessOffense;
	cond = mCondRecklessOffense; 	condName = mCondRecklessOffenseName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Reckless Offense", sizeof("Reckless Offense"));

	cond->condName = condName;
	cond->numArgs = 2;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeRadialMenuEntry, 0, RecklessOffenseRadialMenuInit, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeGetAC, 0, RecklessOffenseAcPenalty, 0, 0);
	DispatcherHookInit(cond, 3, dispTypeToHitBonus2, 0, RecklessOffenseToHitBonus, 0, 0);
	DispatcherHookInit(cond, 4, dispTypeD20Signal, DK_SIG_Attack_Made, TacticalOptionAbusePrevention, 0, 0);
	DispatcherHookInit(cond, 5, dispTypeBeginRound, 0, CondNodeSetArgFromSubDispDef, 1, 0);
	DispatcherHookInit(cond, 6, dispTypeConditionAdd, 0, CondNodeSetArgFromSubDispDef, 0, 0);

	// Knock Down
	mCondKnockDown = &condKnockDown;
	cond = mCondKnockDown; 	condName = mCondKnockDownName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Knock-Down", sizeof("Knock-Down"));

	cond->condName = condName;
	cond->numArgs = 2;

	// Greater Weapon Specialization
	mCondGreaterWeaponSpecialization = &condGreaterWeaponSpecialization;
	cond = mCondGreaterWeaponSpecialization; 	condName = mCondGreaterWeaponSpecializationName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Greater Weapon Specialization", sizeof("Greater Weapon Specialization"));

	cond->condName = condName;
	cond->numArgs = 2;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPreventWithArg, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeDealingDamage, 0, GreaterWeaponSpecializationDamage, 0, 0);

	// Deadly Precision
	mCondDeadlyPrecision = &condDeadlyPrecision;
	cond = mCondDeadlyPrecision; 	condName = mCondDeadlyPrecisionName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Deadly Precision", sizeof("Deadly Precision"));

	cond->condName = condName;
	cond->numArgs = 2;

	// Persistent Spell
	mCondPersistentSpell = &condPersistentSpell;
	cond = mCondPersistentSpell; 	condName = mCondPersistentSpellName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Persistent Spell", sizeof("Persistent Spell"));

	cond->condName = condName;
	cond->numArgs = 2;

	// Disarm
	cond = &mCondDisarm; 	condName = (char*)mCondDisarmName;

	cond->condName = condName;
	cond->numArgs = 2;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeRadialMenuEntry, 0, DisarmRadialMenu, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeD20Signal, DK_SIG_HP_Changed, DisarmHpChanged, 0, 0);
	DispatcherHookInit(cond, 3, dispTypeD20Query, DK_QUE_ActionTriggersAOO, DisarmQueryAoOResetArg, 1, 1);
	DispatcherHookInit(cond, 4, dispTypeD20Query, DK_QUE_Can_Perform_Disarm, DisarmCanPerform, 0, 0);
	//DispatcherHookInit(cond, 5, dispTypeD20Query, DK_QUE_ActionTriggersAOO, QuerySetReturnVal1, 0, 0);

#pragma endregion
	// Disarmed
	cond = &mCondDisarmed; 	condName = (char*)mCondDisarmedName;

	cond->condName = condName;
	cond->numArgs = 8;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeD20Query, DK_QUE_Disarmed, DisarmedRetrieveQuery, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeD20Signal, DK_SIG_Combat_End, DisarmedReminder, 0, 0);
	DispatcherHookInit(cond, 3, dispTypeD20Signal, DK_SIG_Disarmed_Weapon_Retrieve, DisarmedWeaponRetrieve, 0, 0);
	DispatcherHookInit(cond, 4, dispTypeRadialMenuEntry, 0, DisarmedRetrieveWeaponRadialMenu, 0, 0);
	DispatcherHookInit(cond, 5, dispTypeConditionAdd, 0, DisarmedOnAdd, 0, 0);

#pragma region Monster Abilities
	// Rend

	mCondRend = &condRend;
	cond = mCondRend; 	condName = mCondRendName;
	memset(condName, 0, sizeof(condName)); 	memcpy(condName, "Rend", sizeof("Rend"));

	cond->condName = condName;
	cond->numArgs = 8;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeDealingDamage, 0, RendOnDamage, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeBeginRound, 0, CondResetArgs, 0, 0);
	
	// Captivating Song

	cond = &mCondCaptivatingSong; 	condName = (char*)mCondCaptivatingSongName;

	cond->condName = condName;
	cond->numArgs = 8; // 2-7 is the caster objId

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeConditionAdd, 0, CaptivatingSongOnConditionAdd, 1, 0x1028C7C8);

	// Captivated

	cond = &mCondCaptivated; 	condName = (char*)mCondCaptivatedName;

	cond->condName = condName;
	cond->numArgs = 8;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeD20Query, DK_QUE_SneakAttack, genericCallbacks.QuerySetReturnVal1, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeD20Query, DK_QUE_CannotCast, genericCallbacks.QuerySetReturnVal1, 0, 0);
	DispatcherHookInit(cond, 3, dispTypeD20Query, DK_QUE_AOOPossible, QuerySetReturnVal0, 0, 0);
	DispatcherHookInit(cond, 4, dispTypeD20Signal, DK_SIG_Killed, ConditionRemoveCallback, 0, 0);
	DispatcherHookInit(cond, 5, dispTypeTooltip, 0, genericCallbacks.TooltipUnrepeated, 205, 0); // Captivated
	DispatcherHookInit(cond, 6, dispTypeConditionAdd, 0, PlayParticlesSavePartsysId, 1, 0x1028C7C8); // 'Bardic-Fascinate-hit'
	DispatcherHookInit(cond, 7, dispTypeConditionAddFromD20StatusInit, 0, PlayParticlesSavePartsysId, 1, 0x1028C7C8); // 'Bardic-Fascinate-hit'
	DispatcherHookInit(cond, 8, dispTypeConditionRemove, 0, EndParticlesFromArg, 1, 0);
	DispatcherHookInit(cond, 9, dispTypeBeginRound, 0, ConditionDurationTicker, 0, 0);
	DispatcherHookInit(cond, 10, dispTypeTurnBasedStatusInit, 0, TurnBasedStatusInitNoActions, 0, 0);
	DispatcherHookInit(cond, 11, dispTypeEffectTooltip, 0, CaptivatingSongEffectTooltipDuration, 0, 0);


	// Hezrou Stench

	cond = &mCondHezrouStench; 

	cond->condName = "Hezrou Stench";
	cond->numArgs = 4; // 0 - spellId; 1 - duration; 2 - eventId; 3 - partsysId;

	auto aoeSpellRemover = temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100D3430);

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeConditionAdd, 0, spCallbacks.BeginHezrouStench, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeD20Signal, DK_SIG_Spell_End, aoeSpellRemover, 0, 0);
	DispatcherHookInit(cond, 3, dispTypeObjectEvent, 0, spCallbacks.HezrouStenchObjEvent, 0, 0);
	DispatcherHookInit(cond, 4, dispTypeD20Signal, DK_SIG_Combat_End, aoeSpellRemover, 0, 0);
	DispatcherHookInit(cond, 5, dispTypeD20Signal, DK_SIG_Critter_Killed, aoeSpellRemover, 0, 0);

	// Hezrou Stench Nausea / Sickness

	cond = &mCondHezrouStenchHit;

	cond->condName = "Hezrou Stench Hit";
	cond->numArgs = 5; // 0 - spellId; 1 - duration; 2 - eventId; 3 - partsysId; 4 - nausea/sickness (0 = nausea, 1 = sickness)

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeBeginRound, 0, spCallbacks.HezrouStenchCountdown, 0, 0);
	DispatcherHookInit(cond, 2, dispTypeNewDay, DK_NEWDAY_REST, spCallbacks.RemoveSpell, 0, 0);
	DispatcherHookInit(cond, 3, dispTypeObjectEvent, 0, spCallbacks.HezrouStenchObjEvent, 1, 0);
	DispatcherHookInit(cond, 4, dispTypeTurnBasedStatusInit, 0, spCallbacks.HezrouStenchTurnbasedStatus, 0, 0);
	DispatcherHookInit(cond, 5, dispTypeD20Query, DK_QUE_AOOPossible, spCallbacks.HezrouStenchAooPossible, 0, 0);
	DispatcherHookInit(cond, 6, dispTypeSkillLevel, 0, spCallbacks.SkillBonus, -1, -2);
	DispatcherHookInit(cond, 7, dispTypeAbilityCheckModifier, 0, spCallbacks.HezrouStenchAbilityCheckMod, -2, 345);
	DispatcherHookInit(cond, 8, dispTypeSaveThrowLevel, 0, spCallbacks.HezrouStenchSavingThrowLevel, -2, 345);
	DispatcherHookInit(cond, 9, dispTypeDealingDamage2, 0, spCallbacks.HezrouStenchDealingDamage, -2, 345);
	DispatcherHookInit(cond, 10, dispTypeToHitBonus2, 0, spCallbacks.HezrouStenchToHit2, -2, 345);
	DispatcherHookInit(cond, 11, dispTypeEffectTooltip, 0, spCallbacks.HezrouStenchEffectTooltip, 141, 0);
	DispatcherHookInit(cond, 12, dispTypeD20Signal, DK_SIG_Combat_End, spCallbacks.HezrouStenchCureNausea,0,0 );
	DispatcherHookInit(cond, 13, dispTypeD20Query, DK_QUE_Critter_Has_Condition, spCallbacks.HasCondition, (uint32_t)cond, 0);

	// Necklace of Adaptation

	auto itemForceRemoveCallback = temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x10104410);
	auto immunityCheckHandler = temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100ED650);
	auto immunityTriggerCallback = temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100ed5a0);
	cond = &mCondNecklaceOfAdaptation; 

	cond->condName = "Necklace of Adaptation";
	cond->numArgs = 4;
	DispatcherHookInit(cond, 0, dispTypeItemForceRemove, 0, itemForceRemoveCallback, 0, 0);
	DispatcherHookInit(cond, 1, dispTypeSpellImmunityCheck,0, immunityCheckHandler, 4,0);
	DispatcherHookInit(cond, 2, dispTypeImmunityTrigger, DK_IMMUNITY_SPECIAL, immunityTriggerCallback, 0x10, 0);

#pragma endregion


	mCondCraftWandLevelSet = CondStructNew("Craft Wand Level Set", 2);
	mCondCraftWandLevelSet.AddHook(dispTypeD20Query, DK_QUE_Craft_Wand_Spell_Level, QueryRetrun1GetArgs, (uint32_t)&mCondCraftWandLevelSet, 0);
	mCondCraftWandLevelSet.AddHook(dispTypeRadialMenuEntry, DK_NONE, CraftWandRadialMenu);

	// Aid Another
	mCondAidAnother = new CondStructNew();
	memset(mCondAidAnother, 0, sizeof(CondStructNew));
	cond = mCondAidAnother;	condName = mCondAidAnotherName;
	sprintf(condName, "Aid Another");

	cond->condName = condName;
	cond->numArgs = 8;

	DispatcherHookInit(cond, 0, dispTypeConditionAddPre, 0, ConditionPrevent, (uint32_t)cond, 0);
	DispatcherHookInit(cond, 1, dispTypeRadialMenuEntry, 0, AidAnotherRadialMenu, 0, 0);
	// 
	
	/*
	char mCondIndomitableWillName[100];
	CondStructNew *mCondIndomitableWill;
	char mCondTirelessRageName[100];
	CondStructNew *mCondTirelessRage;
	char mCondMightyRageName[100];
	CondStructNew *mCondMightyRage;
	char mCondDisarmName[100];
	CondStructNew *mCondDisarm;
	char mCondImprovedDisarmName[100];
	CondStructNew *mCondImprovedDisarm;

	// monsters
	
	
	*/

	conditions.AddConditionsToTable();

}

void ConditionSystem::AddToFeatDictionary(CondStructNew* condStruct, feat_enums feat, feat_enums featEnumMax, uint32_t condArg2Offset){
	conditions.condDict[feat] = { condStruct, feat, featEnumMax, condArg2Offset };
}

void ConditionSystem::DispatcherHookInit(SubDispDefNew* sdd, enum_disp_type dispType, int key, void* callback, int data1, int data2){
	sdd->dispType = dispType;
	sdd->dispKey = key;
	sdd->dispCallback = ( int(__cdecl*)(DispatcherCallbackArgs))callback;
	sdd->data1.sVal = data1;
	sdd->data2.sVal = data2;
}

void ConditionSystem::DispatcherHookInit(CondStructNew* cond, int hookIdx, enum_disp_type dispType, int key, void* callback, int data1, int data2)
{
	if (cond->subDispDefs[hookIdx].dispType != 0)
	{
		int dummy = 1;
	}
	assert(cond->subDispDefs[hookIdx].dispType == 0);
	DispatcherHookInit(&cond->subDispDefs[hookIdx], dispType, key, callback, data1, data2 );
}

void ConditionSystem::SetPermanentModArgsFromDataFields(Dispatcher* dispatcher, CondStruct* condStruct, int* condArgs)
{
	addresses.SetPermanentModArgsFromDataFields(dispatcher, condStruct, condArgs);
}

void ConditionSystem::DispatcherCondsResetFlag2(Dispatcher* dispatcher)
{
	CondNode *condNode; 
	for (condNode = dispatcher->permanentMods; condNode; condNode = condNode->nextCondNode)
		condNode->flags &= 0xFFFFFFFD;
	for (condNode = dispatcher->itemConds; condNode; condNode = condNode->nextCondNode)
		condNode->flags &= 0xFFFFFFFD;
}

int ConditionSystem::GetActiveCondsNum(Dispatcher* dispatcher)
{ 
	int numConds=0; 

	CondNode *cond = dispatcher->conditions;
	while (cond)
	{
		if (!(cond->flags & 1))
			++numConds;
		cond = cond->nextCondNode;
	}
	return numConds;
}

int ConditionSystem::GetPermanentModsAndItemCondCount(Dispatcher* dispatcher)
{
	int numConds = 0;
	CondNode *cond = dispatcher->permanentMods;

	while (cond)
	{
		if (!(cond->flags & 1))
			++numConds;
		cond = cond->nextCondNode;
	}

	cond = dispatcher->itemConds;
	while(cond)
	{
		if (!(cond->flags & 1))
			++numConds;
		cond = cond->nextCondNode;
	}
	return numConds;
}

int ConditionSystem::ConditionsExtractInfo(Dispatcher* dispatcher, int activeCondIdx, int* hashkeyOut, int* condArgsOut)
{
	CondNode *cond;
	int n; 
	int numArgs; 


	cond = dispatcher->conditions;              
	n = 0;
	while (cond)
	{
		if (!(cond->flags & 1))
		{
			if (n == activeCondIdx)
			{
				*hashkeyOut = conds.hashmethods.GetCondStructHashkey(cond->condStruct);
				int numArgs = cond->condStruct->numArgs;
				for (int i = 0; i < numArgs; i++)
				{
					condArgsOut[i] = cond->args[i];
				}
				return cond->condStruct->numArgs;
			}
			++n;
		}
		cond = cond->nextCondNode;
	}
	if (!cond) return 0;

	*hashkeyOut = conds.hashmethods.GetCondStructHashkey(cond->condStruct);
	numArgs = cond->condStruct->numArgs;
	for (int i = 0; i < numArgs; i++)
	{
		condArgsOut[i] = cond->args[i];
	}
	return cond->condStruct->numArgs;
}

int ConditionSystem::PermanentAndItemModsExtractInfo(Dispatcher* dispatcher, int permModIdx, int* hashkeyOut, int* condArgsOut)
{
	
	int i=0; 
	CondNode *cond;

	cond = dispatcher->permanentMods;
	while (cond )
	{
		if (!(cond->flags & 1))
		{
			if (i == permModIdx)
			{
				*hashkeyOut = conds.hashmethods.GetCondStructHashkey(cond->condStruct);
				int numArgs = cond->condStruct->numArgs;
				for (int i = 0; i < numArgs; i++)
				{
					condArgsOut[i] = cond->args[i];
				}
				return cond->condStruct->numArgs;
			}
			++i;
		}
		cond = cond->nextCondNode;
	}


	cond = dispatcher->itemConds;
	while (cond)
	{
		if (! (cond->flags & 1))
		{
			if (i == permModIdx)
			{
				*hashkeyOut = conds.hashmethods.GetCondStructHashkey(cond->condStruct);
				int numArgs = cond->condStruct->numArgs;
				for (i = 0; i < numArgs; i++)
				{
					condArgsOut[i] = cond->args[i];
				}
				return cond->condStruct->numArgs;
			}
			i++;
		}
		cond = cond->nextCondNode;
	}
	if (!cond) return 0;
}

void ConditionSystem::ConditionRemove(objHndl objHnd, CondNode* cond)
{
	Dispatcher * dispatcher = objects.GetDispatcher(objHnd);
	if (dispatch.dispatcherValid(dispatcher))
	{
		dispatch.DispatchConditionRemove(dispatcher, cond);
	}
}

int* ConditionSystem::CondNodeGetArgPtr(CondNode* condNode, int argIdx)
{
	if (argIdx < condNode->condStruct->numArgs)
		return (int*)&condNode->args[argIdx];
	return 0;
}

#pragma region NewConditionCallbacks

 int __cdecl AoODisableRadialMenuInit(DispatcherCallbackArgs args)
{
	RadialMenuEntry radEntry;
	radEntry.maxArg = 1;
	radEntry.minArg = 0;
	radEntry.type = RadialMenuEntryType::Toggle;
	radEntry.actualArg = (int)conds.CondNodeGetArgPtr(args.subDispNode->condNode, 0);
	radEntry.callback = (void (__cdecl*)(objHndl, RadialMenuEntry*))temple::GetPointer(0x100F0200);
	MesLine mesLine;
	mesLine.key = 5105; //disable AoOs
	if (!mesFuncs.GetLine(*combatSys.combatMesfileIdx, &mesLine) )
	{
		//sprintf((char*)temple::GetPointer(0x10EEE228), "Disable Attacks of Opportunity");
		mesLine.value = conds.mConditionDisableAoOName;
	}
	//mesFuncs.GetLine_Safe(*combatSys.combatMesfileIdx, &mesLine);
	radEntry.text = (char*)mesLine.value;
	radEntry.helpId = conds.hashmethods.StringHash("TAG_RADIAL_MENU_DISABLE_AOOS");
	int parentNode = radialMenus.GetStandardNode(RadialMenuStandardNode::Options);
	radialMenus.AddChildNode(args.objHndCaller, &radEntry, parentNode);
	return 0;
}

int __cdecl AoODisableQueryAoOPossible(DispatcherCallbackArgs args)
{
	DispIoD20Query * dispIo = dispatch.DispIoCheckIoType7((DispIoD20Query*)args.dispIO);
	if (dispIo->return_val && conds.CondNodeGetArg(args.subDispNode->condNode, 0))
	{
		dispIo->return_val = 0;
	}
	return 0;
}

int __cdecl GreaterTwoWeaponFighting(DispatcherCallbackArgs args)
{
	DispIoD20ActionTurnBased *dispIo = dispatch.DispIoCheckIoType12((DispIoD20ActionTurnBased*)args.dispIO);
	objHndl mainWeapon = inventory.ItemWornAt(args.objHndCaller, 3);
	objHndl offhand = inventory.ItemWornAt(args.objHndCaller, 4);
	
	if (mainWeapon != offhand)
	{
		if (mainWeapon)
		{
			if (offhand)
			{
				int weapFlags = objects.getInt32(mainWeapon, obj_f_weapon_flags);
				if (!(weapFlags & (4<<8)) && objects.getInt32(offhand, obj_f_type) != obj_t_armor)
					++dispIo->returnVal;
			}
		}
	}
	return 0;

}

int __cdecl GreaterTWFRanger(DispatcherCallbackArgs args)
{
	if (!critterSys.IsWearingLightOrNoArmor(args.objHndCaller))
	{
		return 0;
	}
	return GreaterTwoWeaponFighting(args);
};

int __cdecl TwoWeaponFightingBonus(DispatcherCallbackArgs args)
{
	DispIoAttackBonus *dispIo = dispatch.DispIoCheckIoType5((DispIoAttackBonus*)args.dispIO);
	char *featName; // eax@3 MAPDST

	feat_enums feat = (feat_enums)conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	int attackCode = dispIo->attackPacket.dispKey;
	int dualWielding = 0;
	int attackNumber = 1;
	if (d20Sys.UsingSecondaryWeapon(args.objHndCaller, attackCode))
	{
		featName = feats.GetFeatName(feat);
		bonusSys.bonusAddToBonusListWithDescr(&dispIo->bonlist, 6, 0, 114, featName);
	}
		else if ( d20Sys.ExtractAttackNumber(args.objHndCaller, attackCode, &attackNumber, &dualWielding), dualWielding != 0)
	{
		featName = feats.GetFeatName(feat);
		bonusSys.bonusAddToBonusListWithDescr(&dispIo->bonlist, 2, 0, 114, featName);
	}
	return 0;
}

int TwoWeaponFightingBonusRanger(DispatcherCallbackArgs args)
{
	DispIoAttackBonus * dispIo = dispatch.DispIoCheckIoType5((DispIoAttackBonus*)args.dispIO);
	if ( !critterSys.IsWearingLightOrNoArmor(args.objHndCaller))
	{
		bonusSys.zeroBonusSetMeslineNum(&dispIo->bonlist, 166);
		return 0;
	}
	
	
	feat_enums feat = FEAT_TWO_WEAPON_FIGHTING;
	char * featName;
	int attackCode = dispIo->attackPacket.dispKey;
	int dualWielding = 0;
	int attackNumber = 1;
	if (d20Sys.UsingSecondaryWeapon(args.objHndCaller, attackCode))
	{
		featName = feats.GetFeatName(feat);
		bonusSys.bonusAddToBonusListWithDescr(&dispIo->bonlist, 6, 0, 114, featName);
	}
	else if (d20Sys.ExtractAttackNumber(args.objHndCaller, attackCode, &attackNumber, &dualWielding), dualWielding != 0)
	{
		featName = feats.GetFeatName(feat);
		bonusSys.bonusAddToBonusListWithDescr(&dispIo->bonlist, 2, 0, 114, featName);
	}
	return 0;

}



int __cdecl DivineMightRadial(DispatcherCallbackArgs args)
{
	if (d20Sys.d20Query(args.objHndCaller, DK_QUE_IsFallenPaladin))
		return 0;

	RadialMenuEntryAction radEntry(5106, D20A_DIVINE_MIGHT, 0, ElfHash::Hash("TAG_DIVINE_MIGHT"));
	radEntry.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::Feats);
	return 0;
}


int __cdecl DivineMightDamageBonus(DispatcherCallbackArgs args)
{
	DispIoDamage * dispIo = dispatch.DispIoCheckIoType4((DispIoDamage*)args.dispIO);
	char * desc = feats.GetFeatName(FEAT_DIVINE_MIGHT);
	int damBonus = conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	damage.AddDamageBonusWithDescr(&dispIo->damage, damBonus, 0, 114, desc);
	return 0;
}


int GreaterWeaponSpecializationDamage(DispatcherCallbackArgs args)
{
	int weaponType; 
	char *featName; 

	feat_enums feat = (feat_enums)conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	WeaponTypes wpnTypeFromCond = (WeaponTypes)conds.CondNodeGetArg(args.subDispNode->condNode, 1);
	DispIoDamage * dispIo = dispatch.DispIoCheckIoType4(args.dispIO);
	objHndl weapon = combatSys.GetWeapon(&dispIo->attackPacket);
	if (weapon)
		weaponType = objects.GetWeaponType(weapon);
	else
		weaponType = 1;
	if (weaponType == wpnTypeFromCond)
	{
		featName = feats.GetFeatName(feat);
		damage.AddDamageBonusWithDescr(&dispIo->damage, 2, 0, 114, featName);
	}
	return 0;
}

int __cdecl RecklessOffenseRadialMenuInit(DispatcherCallbackArgs args)
{
	RadialMenuEntry radEntry;
	radEntry.maxArg = 1;
	radEntry.minArg = 0;
	radEntry.type = RadialMenuEntryType::Toggle;
	radEntry.actualArg = (int)conds.CondNodeGetArgPtr(args.subDispNode->condNode, 0);
	radEntry.callback = (void(__cdecl*)(objHndl, RadialMenuEntry*))temple::GetPointer(0x100F0200);
	MesLine mesLine;
	mesLine.key = 5107; // reckless offense
	if (!mesFuncs.GetLine(*combatSys.combatMesfileIdx, &mesLine))
	{
		//sprintf((char*)temple::GetPointer(0x10EEE228), "Reckless Offense");
		mesLine.value = conds.mCondRecklessOffenseName;
	};
	radEntry.text = (char*)mesLine.value;
	radEntry.helpId = conds.hashmethods.StringHash("TAG_FEAT_RECKLESS_OFFENSE");
	int parentNode = radialMenus.GetStandardNode(RadialMenuStandardNode::Feats);
	radialMenus.AddChildNode(args.objHndCaller, &radEntry, parentNode);
	return 0;
}

int RecklessOffenseAcPenalty(DispatcherCallbackArgs args)
{
	if (conds.CondNodeGetArg(args.subDispNode->condNode, 0))
	{
		if (conds.CondNodeGetArg(args.subDispNode->condNode, 1))
		{
			DispIoAttackBonus* dispIo = dispatch.DispIoCheckIoType5(args.dispIO);
			BonusList* bonlist = &dispIo->bonlist;
			bonusSys.bonusAddToBonusList(bonlist, -4, 8, 337);
		}
	}
	return 0;
}

int RecklessOffenseToHitBonus(DispatcherCallbackArgs args)
{
	if (conds.CondNodeGetArg(args.subDispNode->condNode, 0))
	{
		DispIoAttackBonus* dispIo = dispatch.DispIoCheckIoType5(args.dispIO);
		if (!(dispIo->attackPacket.flags & D20CAF_RANGED))
			bonusSys.bonusAddToBonusList(&dispIo->bonlist, 2, 0, 337);
	}
	return 0;
}

int TacticalOptionAbusePrevention(DispatcherCallbackArgs args)
{ // signifies that an attack has been made using that tactical option (so user doesn't toggle it off and shrug off the penalties)
	DispIoD20Signal * dispIo = dispatch.DispIoCheckIoType6(args.dispIO);
	if (!(*(char*)(dispIo->data1 + 32) & 4))
		_CondNodeSetArg(args.subDispNode->condNode, 1, 1);
	return 0;
}

#pragma region Barbarian Stuff

int __cdecl CombatExpertiseRadialMenu(DispatcherCallbackArgs args)
{
	int bab = dispatch.DispatchToHitBonusBase(args.objHndCaller, 0);
	if (bab > 0)
	{
		auto maxArg = feats.HasFeatCount(args.objHndCaller, FEAT_SUPERIOR_EXPERTISE)? bab:min(5, bab);
		RadialMenuEntrySlider radEntry(5007,0, maxArg, args.GetCondArgPtr(0), -1, ElfHash::Hash("TAG_COMBAT_EXPERTISE") );
		int parentNode = radialMenus.GetStandardNode(RadialMenuStandardNode::Feats);
		radialMenus.AddChildNode(args.objHndCaller, &radEntry, parentNode);
	}
	return 0;
}

int CombatExpertiseSet(DispatcherCallbackArgs args)
{
	int bab = dispatch.DispatchToHitBonusBase(args.objHndCaller, 0);
	if (bab > 5 && !feats.HasFeatCount(args.objHndCaller, FEAT_SUPERIOR_EXPERTISE))
		bab = 5;
	DispIoD20Signal * dispIo = dispatch.DispIoCheckIoType6(args.dispIO);
	int bonus = dispIo->data1;
	if (bonus > bab)
		bonus = bab;
	if (bonus < 0)
		bonus = 0;
	conds.CondNodeSetArg(args.subDispNode->condNode, 0, bonus);
	return 0;
}

int BarbarianRageStatBonus(DispatcherCallbackArgs args)
{
	DispIoBonusList * dispIo = dispatch.DispIoCheckIoType2(args.dispIO);
	if (feats.HasFeatCountByClass(args.objHndCaller, FEAT_MIGHTY_RAGE, (Stat)0, 0))
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 8, 0, 339); // Greater Rage
	else if (feats.HasFeatCountByClass(args.objHndCaller, FEAT_GREATER_RAGE, (Stat)0, 0))
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 6, 0, 338); // Greater Rage
	else
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 4, 0, 195); // normal rage
	return 0;
}

int BarbarianRageSaveBonus(DispatcherCallbackArgs args)
{
	DispIoSavingThrow * dispIo = dispatch.DispIoCheckIoType3(args.dispIO);
	if (feats.HasFeatCountByClass(args.objHndCaller, FEAT_MIGHTY_RAGE, (Stat)0, 0))
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 4, 0, 339); // Mighty Rage
	else if (feats.HasFeatCountByClass(args.objHndCaller, FEAT_GREATER_RAGE, (Stat)0, 0))
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 3, 0, 338); // Greater Rage
	else
		bonusSys.bonusAddToBonusList(&dispIo->bonlist, 2, 0, 195); // normal rage

	if (dispIo->flags & 0x100) // enchantment
	{
		if (feats.HasFeatCountByClass(args.objHndCaller, FEAT_INDOMITABLE_WILL, (Stat)0, 0))
			bonusSys.bonusAddToBonusList(&dispIo->bonlist, 4, 0, 344); // Indomitable Will
	}
	return 0;
}

int BarbarianDamageResistance(DispatcherCallbackArgs args)
{
	DamagePacket *damagePacket; 
	int barbLvl; 
	int damRes = 0;

	damagePacket = &dispatch.DispIoCheckIoType4(args.dispIO)->damage;
	barbLvl = objects.StatLevelGet(args.objHndCaller, stat_level_barbarian);
	if (barbLvl >= 7)
	{
		damRes = 1 + (barbLvl - 7) / 3;
		damage.AddPhysicalDR(damagePacket, damRes, 1, 126u);
	}
	
	return 0;
}


void __cdecl BarbarianTirelessRageCheck(objHndl obj)
{
	if (objects.StatLevelGet(obj, stat_level_barbarian) < 17)
		conds.AddTo(obj, "Barbarian_Fatigued", {0,0});
};

class BarbarianTirelessRagePatch : public TempleFix
{
public:
	const char* name() override {
		return "Barbarian Tireless Rage patch";
	}

	void apply() override {
		redirectCall(0x100EADBF, BarbarianTirelessRageCheck);
	}

} barbarianTirelessRagePatch;

#pragma endregion

int __cdecl DisarmRadialMenu(DispatcherCallbackArgs args){

	RadialMenuEntryAction radEntry(5109, D20A_DISARM, 0, ElfHash::Hash("TAG_DISARM"));
	radEntry.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::Offense);
	return 0;
}

int __cdecl DisarmHpChanged(DispatcherCallbackArgs args)
{
	DispIoD20Signal * dispIo = dispatch.DispIoCheckIoType6(args.dispIO);
	int hpChange = dispIo->data1;
	if (hpChange < 0)
	{
		if (conds.CondNodeGetArg(args.subDispNode->condNode, 1) )
			conds.CondNodeSetArg(args.subDispNode->condNode, 0, 1);
	}
	return 0;
};


int __cdecl DisarmQueryAoOResetArg(DispatcherCallbackArgs args)
{
	DispIoD20Query * dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	D20Actn * d20a = (D20Actn*)dispIo->data1;
	if (d20a->d20ActType == D20A_DISARM)
	{
		if (!feats.HasFeatCountByClass(args.objHndCaller, FEAT_IMPROVED_DISARM))
			dispIo->return_val = 1;
		conds.CondNodeSetArg(args.subDispNode->condNode, args.subDispNode->subDispDef->data1,
			args.subDispNode->subDispDef->data2);
	}
		
	// sets arg[data1] from data2  
	// e.g. IF data1 = 0, data2 = 15 
	//    THEN it'll set arg0 = 15
	return 0;
};

int __cdecl DisarmCanPerform(DispatcherCallbackArgs args)
{
	DispIoD20Query * dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	conds.CondNodeSetArg(args.subDispNode->condNode, 1, 0);
	if (conds.CondNodeGetArg(args.subDispNode->condNode, 0) == 0)
	{
		dispIo->return_val = 1;
	}
	conds.CondNodeSetArg(args.subDispNode->condNode, 0, 0);
	return 0;
};

int DisarmedReminder(DispatcherCallbackArgs args)
{
	if (args.subDispNode->condNode->args[7] < 2 && party.IsInParty(args.objHndCaller) 
		&& !critterSys.IsDeadOrUnconscious(args.objHndCaller))
	{
		char blargh[1000];
		memcpy(blargh, "I was disarmed.", sizeof("I was disarmed."));
		uiDialog.ShowTextBubble(args.objHndCaller, args.objHndCaller, { blargh }, -1);
		args.subDispNode->condNode->args[7]++;
	}
	return 0;
}

int DisarmedOnAdd(DispatcherCallbackArgs args)
{
	objHndl weapon;
	((int*)&weapon)[0] = conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	((int*)&weapon)[1] = conds.CondNodeGetArg(args.subDispNode->condNode, 1);
	ObjectId objId = objects.GetId(weapon);
	memcpy(args.subDispNode->condNode->args, &objId, sizeof(ObjectId));
	return 0;
}

int DisarmedWeaponRetrieve(DispatcherCallbackArgs args)
{
	objHndl weapon = 0i64;
	DispIoD20Signal * dispIo = dispatch.DispIoCheckIoType6(args.dispIO);
	D20Actn* d20a = (D20Actn*)dispIo->data1;
	if (d20a->d20ATarget && objects.GetType(d20a->d20ATarget) == obj_t_weapon)
	{
		weapon = d20a->d20ATarget;
	}
	else
	{
		ObjectId objId;
		memcpy(&objId, args.subDispNode->condNode->args, sizeof(ObjectId));
		weapon = gameSystems->GetObj().GetHandleById(objId);
	}
	if (!weapon || (inventory.GetParent(weapon) && combatSys.isCombatActive())|| objects.GetType(weapon) != obj_t_weapon)
	{
		objects.floats->FloatCombatLine(args.objHndCaller, 195); //fail!
		if (args.subDispNode->condNode->args[6] < 2)
			args.subDispNode->condNode->args[6]++;
		else
			conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		return 0; 
	}

	//((int*)&weapon)[0]= conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	//((int*)&weapon)[1] = conds.CondNodeGetArg(args.subDispNode->condNode, 1);
	if (!inventory.ItemWornAt(args.objHndCaller, 203))
		inventory.ItemGetAdvanced(weapon, args.objHndCaller, 203, 0);
	else
		inventory.ItemGetAdvanced(weapon, args.objHndCaller, -1, 0);
	objects.floats->FloatCombatLine(args.objHndCaller, 201);
	conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
	return 0;
};

int DisarmedRetrieveWeaponRadialMenu(DispatcherCallbackArgs args){

	RadialMenuEntryAction radEntry(5111, D20A_DISARMED_WEAPON_RETRIEVE, 0, "TAG_DISARM");
	radEntry.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::Items);
	return 0;
}

int __cdecl SunderRadialMenu(DispatcherCallbackArgs args){

	RadialMenuEntryAction radEntry(5110, D20A_SUNDER, 0, "TAG_SUNDER");
	radEntry.AddChildToStandard(args.objHndCaller, RadialMenuStandardNode::Offense);
	return 0;
}



int RendOnDamage(DispatcherCallbackArgs args)
{
	DispIoDamage * dispIo = dispatch.DispIoCheckIoType4(args.dispIO);
	objHndl weapon = combatSys.GetWeapon(&dispIo->attackPacket);
	if (weapon)
		return 0;
	
	

	DamagePacket * dmgPacket = &dispIo->damage;
	auto attackDescr = dmgPacket->dice[0].typeDescription; // e.g. Claw etc.
	if (conds.CondNodeGetArg(args.subDispNode->condNode, 0) && attackDescr == (char*)conds.CondNodeGetArg(args.subDispNode->condNode, 1))
	{
		Dice dice(2, 6, 9);
		damage.AddDamageDice(&dispIo->damage, dice.ToPacked(), DamageType::PiercingAndSlashing, 133);
		floatSys.FloatCombatLine(args.objHndCaller, 203);
		conds.CondNodeSetArg(args.subDispNode->condNode, 0, 0);
	}
	else
	{
		conds.CondNodeSetArg(args.subDispNode->condNode, 0, 1);
		conds.CondNodeSetArg(args.subDispNode->condNode, 1, (int)attackDescr);
	}
		
	return 0;
}


int ConditionFunctionReplacement::LayOnHandsPerform(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType12(args.dispIO);
	auto d20a = dispIo->d20a;
	bool animResult = false;
	if (critterSys.IsUndead(d20a->d20ATarget)){
		d20a->d20Caf |= D20CAF_TOUCH_ATTACK;
		if (d20a->d20Caf & D20CAF_RANGED)
			return 0;
		d20Sys.ToHitProc(d20a);
		animResult = animationGoals.PushAttemptAttack(d20a->d20APerformer, d20a->d20ATarget);
	} else	{
		animResult = animationGoals.PushAnimate(d20a->d20APerformer, 86);
	}

	
	if (animResult)
	{
		// fixes lack of animation ID
		d20a->animID = animationGoals.GetAnimIdSthgSub_1001ABB0(d20a->d20APerformer);
		d20a->d20Caf |= D20CAF_NEED_ANIM_COMPLETED;
	}
		

	return 0;
}

int ConditionFunctionReplacement::RemoveDiseasePerform(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType12(args.dispIO);
	auto d20a = dispIo->d20a;
	bool animResult = animationGoals.PushAnimate(d20a->d20APerformer, 86);
	
	if (animResult){
		// fixes lack of animation ID
		d20a->animID = animationGoals.GetAnimIdSthgSub_1001ABB0(d20a->d20APerformer);
		d20a->d20Caf |= D20CAF_NEED_ANIM_COMPLETED;
	}
	return 0;
}

void ConditionFunctionReplacement::HookSpellCallbacks()
{
	// QueryCritterHasCondition for sp-Spiritual Weapon
	int writeVal = dispTypeD20Query;
	SubDispDefNew sdd;
	sdd.dispType = dispTypeD20Query;
	sdd.data1.sVal = 0x102DFC00;
	sdd.data2.usVal = 0;
	sdd.dispCallback = QueryCritterHasCondition;
	sdd.dispKey = DK_QUE_Critter_Has_Condition;
	write(0x102DFD48, &sdd, sizeof(SubDispDefNew));

	// QueryCritterHasCondition for sp-Sleep
	sdd.dispType = dispTypeD20Query;
	sdd.data1.sVal = 0x102DEB08;
	sdd.data2.usVal = 0;
	sdd.dispCallback = QueryCritterHasCondition;
	sdd.dispKey = DK_QUE_Critter_Has_Condition;
	write(0x102DEC00, &sdd, sizeof(SubDispDefNew));

	//  DK_SIG_AID_ANOTHER_WAKE_UP for sp-Sleep
	sdd.dispType = dispTypeD20Signal;
	sdd.data1.usVal = 0;
	sdd.data2.usVal = 0;
	sdd.dispCallback = RemoveSpellConditionAndMod;
	sdd.dispKey = DK_SIG_AID_ANOTHER_WAKE_UP;
	write(0x102DEB9C, &sdd, sizeof(SubDispDefNew)); // overwriting S_Teleport_Reconnect since it does nothing (return_0 callback)


	// EffectTooltip for Stinking Cloud
	sdd.dispType = dispTypeEffectTooltip;
	sdd.dispKey = 0;
	sdd.data1.usVal = 141;
	sdd.data2.usVal = 0;
	sdd.dispCallback = [](DispatcherCallbackArgs args)->int
	{
		auto dispIo = dispatch.DispIoCheckIoType24(args.dispIO);
		auto remainingDuration = args.GetCondArg(1);
		auto spellId = args.GetCondArg(0);
		SpellPacketBody spellPkt(spellId);

		auto text = fmt::format("\n {}: {}/{}", combatSys.GetCombatMesLine(175), remainingDuration, spellPkt.duration);
		dispIo->Append(args.GetData1(), spellPkt.spellEnum, text.c_str());
		return 0;
	};
	write(0x102DFF50, &sdd, sizeof(SubDispDefNew));

}

#pragma region Spell Callbacks

int SpellCallbacks::SkillBonus(DispatcherCallbackArgs args){

	SkillEnum skillEnum = (SkillEnum)args.GetData1();
	int bonValue = args.GetData2();

	auto spellId = args.GetCondArg(0);
	SpellPacketBody spellPkt(spellId);

	int bonType = 0; // will stack if 0

	if (args.dispKey == skillEnum + 20 || skillEnum == -1) {
		auto dispIo = dispatch.DispIoCheckIoType10((DispIoBonusAndObj*)args.dispIO);
		auto spellName = spellSys.GetSpellName(spellPkt.spellEnum);
		dispIo->bonOut->AddBonusWithDesc(bonValue, bonType, 113, (char*)spellName); // 113 is ~Spell~[TAG_SPELLS] in bonus.mes
	}
	return 0;
}

int SpellCallbacks::BeginHezrouStench(DispatcherCallbackArgs args){

	auto spellId = args.GetCondArg(0);
	SpellPacketBody spellPkt(spellId);
	if (!spellId)
		return 0;

	SpellEntry spellEntry(spellPkt.spellEnum);

	auto evtId = objEvents.EventAppend(args.objHndCaller, 0, 1, OLC_CRITTERS, 12.0 * spellEntry.radiusTarget, 0.0, M_PI * 2);

	args.SetCondArg(2, evtId);
	if (!spellPkt.UpdateSpellsCastRegistry()){
		logger->warn("BeginHezrouStench: Unable to update spell cast registry!");
		return 0;
	}

	spellPkt.spellObjs[0].obj = spellPkt.aoeObj;
	spellPkt.spellObjs[0].partySysId = args.GetCondArg(3);
	spellPkt.UpdateSpellsCastRegistry();
	pySpellIntegration.UpdateSpell(spellPkt.spellId);


	return 0;
}

int SpellCallbacks::HezrouStenchObjEvent(DispatcherCallbackArgs args){

	DispIoObjEvent* dispIo = dispatch.DispIoCheckIoType17(args.dispIO);
	auto condEvtId = args.GetCondArg(2);
	if (dispIo->evtId == condEvtId) {

		auto spellId = args.GetCondArg(0);
		SpellPacketBody spellPkt(spellId);
		if (!spellPkt.spellId) {
			logger->warn("HezrouStenchObjEvent: Unable to fetch spell! ID {}", spellId);
			return 0;
		}

		/*
		AoE Entered;
		- add the target to the Spell's Target List
		- Do a saving throw
		*/
		if (args.dispKey == DK_OnEnterAoE)
		{
			/*
				Hezrou Stench condition (the one applied to the SpellObject)
			*/
			if (args.GetData1() == 0)
			{
				// if already has the condition - skip
				if (d20Sys.d20QueryWithData(dispIo->tgt, DK_QUE_Critter_Has_Condition, 	(CondStruct*)&conds.mCondHezrouStenchHit,0))
					return 0;
				if (d20Sys.d20Query(dispIo->tgt, DK_QUE_Critter_Is_Immune_Poison))
					return 0;
				if (critterSys.IsCategorySubtype(dispIo->tgt, MonsterCategory::mc_subtype_demon))
					return 0;
				if (critterSys.IsCategoryType(dispIo->tgt, MonsterCategory::mc_type_elemental))
					return 0;


				pySpellIntegration.SpellSoundPlay(&spellPkt, SpellEvent::SpellStruck);
				pySpellIntegration.SpellTrigger(spellId, SpellEvent::AreaOfEffectHit);


				// Hezrou Stench does not provoke Spell Resistance
				/*if (spellSys.CheckSpellResistance(&spellPkt, dispIo->tgt) == 1)
					return 0;*/


				auto partsysId = gameSystems->GetParticleSys().CreateAtObj("sp-Stinking Cloud Hit", dispIo->tgt);
				spellPkt.AddTarget(dispIo->tgt, partsysId, 1);
				// save succeeds - apply Sickened
				if (damage.SavingThrowSpell(dispIo->tgt, spellPkt.caster, 24, SavingThrowType::Fortitude, 0, spellPkt.spellId)) {
					conds.AddTo(dispIo->tgt, "Hezrou Stench Hit", { static_cast<int>(spellPkt.spellId), spellPkt.durationRemaining, static_cast<int>(dispIo->evtId), partsysId,1 });
					floatSys.FloatSpellLine(dispIo->tgt, 20026, FloatLineColor::Red);
				}
				// save failed - apply nauseated
				else {
					conds.AddTo(dispIo->tgt, "Hezrou Stench Hit", { static_cast<int>(spellPkt.spellId), spellPkt.durationRemaining, static_cast<int>(dispIo->evtId), partsysId, 0 });
					combatSys.FloatCombatLine(dispIo->tgt, 150, FloatLineColor::Red);
				}
			} 
			
			/*
				"Hezrou Stench Hit" condition (the one applied to the critter from the above)
			*/
			else if (args.GetData1() == 1) 
			{
				if (args.GetCondArg(4) == 2) // "cured"
				{
					args.SetCondArg(4, 1); // re-establish sickness when stepping into Hezrou AoE
				}
			}
		
		}

		/*
		AoE exited;
		- If Nauseated (identified by arg3= 0), apply a 1d4 duration
		- If Sickened, changed to "cured" (arg3 = 2)
		*/
		else if (args.dispKey == DK_OnLeaveAoE)
		{
			if (args.GetData1() == 1){

				if (args.GetCondArg(4) == 1) // sickened
				{
					args.SetCondArg(4, 2);
					// histSys.CreateFromFreeText(fmt::format("{} exited Stinking Cloud; Nauseated for {} more rounds.\n", description.getDisplayName(dispIo->tgt), rollResult).c_str());
				}
				else if (args.GetCondArg(4) == 0) // nauseated
				{
					auto rollResult = Dice::Roll(1, 4, 0); // new duration
					args.SetCondArg(1, rollResult);
					histSys.CreateFromFreeText(fmt::format("{} exited Hezrou Stench; Nauseated for {} more rounds.\n", description.getDisplayName(dispIo->tgt), rollResult).c_str());
				}
			}

		}

		if (!spellPkt.UpdateSpellsCastRegistry()) {
			logger->warn("HezrouStenchObjEvent: Unable to save update SpellPacket!");
			return 0;
		}
		pySpellIntegration.UpdateSpell(spellId);
	}
	return 0;
}

int SpellCallbacks::HezrouStenchCountdown(DispatcherCallbackArgs args)
{

	auto effectType = args.GetCondArg(4);
	/*
	count down for nauseated only
	*/
	if (effectType != 0)
	{
		return 0;
	}

	auto dispIo = dispatch.DispIoCheckIoType6(args.dispIO);
	
	// new duration
	int durationRem = (int)args.GetCondArg(1) - (int)dispIo->data1;
	args.SetCondArg(1, durationRem);

	// if duration drops below 0, change to "Cured" status
	// (the assumption is that this will only happen for the 1d4 countdown, i.e. after you leave the hezrou area)
	if (durationRem < 0){
		args.SetCondArg(4, 2);
	}
	return 0;
}

int SpellCallbacks::HezrouStenchTurnbasedStatus(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType8(args.dispIO);
	if (dispIo && args.GetCondArg(4) == 0) {
		auto tbStat = dispIo->tbStatus;
		if (tbStat) {
			if (tbStat->hourglassState > 1)
				tbStat->hourglassState = 1;
		}
	}
	return 0;
}

int SpellCallbacks::HezrouStenchAooPossible(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	// if nauseated
	if (dispIo->return_val && args.GetCondArg(4) == 0){
		dispIo->return_val = 0;
	}
	return 0;
}

int SpellCallbacks::HezrouStenchAbilityCheckMod(DispatcherCallbackArgs args)
{
	if (args.GetCondArg(4) > 1)
		return 0;

	auto dispIo = dispatch.DispIoCheckIoType10(args.dispIO);

	dispIo->bonOut->AddBonus(args.GetData1(), 0, args.GetData2());

	return 0;

}

int SpellCallbacks::HezrouStenchSavingThrowLevel(DispatcherCallbackArgs args)
{
	if (args.GetCondArg(4) > 1)
		return 0;

	auto dispIo = dispatch.DispIoCheckIoType3(args.dispIO);
	dispIo->bonlist.AddBonus(args.GetData1(), 0, args.GetData2());
	return 0;
}

int SpellCallbacks::HezrouStenchDealingDamage(DispatcherCallbackArgs args)
{
	if (args.GetCondArg(4) > 1)
		return 0;

	auto dispIo = dispatch.DispIoCheckIoType4(args.dispIO);
	dispIo->damage.bonuses.AddBonus(args.GetData1(), 0, args.GetData2());
	return 0;
}

int SpellCallbacks::HezrouStenchToHit2(DispatcherCallbackArgs args)
{
	if (args.GetCondArg(4) > 1)
		return 0;

	auto dispIo = dispatch.DispIoCheckIoType5(args.dispIO);
	dispIo->bonlist.AddBonus(args.GetData1(), 0, args.GetData2());
	return 0;
}

int SpellCallbacks::HezrouStenchEffectTooltip(DispatcherCallbackArgs args)
{

	if (args.GetCondArg(4) > 1)
		return 0;

	auto dispIo = dispatch.DispIoCheckIoType24(args.dispIO);
	auto spellId = args.GetCondArg(0);
	SpellPacketBody spellPkt(spellId);

	/*
		nauseated
	*/
	if (args.GetCondArg(4) == 0){

		auto remainingDuration = args.GetCondArg(1);
		if (remainingDuration < 5){
			dispIo->Append(args.GetData1(), spellPkt.spellEnum, fmt::format("\n {}: {}", combatSys.GetCombatMesLine(175), remainingDuration).c_str());
			return 0;
		}
	}
	dispIo->Append(args.GetData1(), spellPkt.spellEnum, nullptr);
	
	return 0;
}

int SpellCallbacks::HezrouStenchCureNausea(DispatcherCallbackArgs args)
{
	if (args.GetCondArg(4) == 1)
		combatSys.FloatCombatLine(args.objHndCaller, 206, FloatLineColor::White);
	else if (args.GetCondArg(4) == 0)
		combatSys.FloatCombatLine(args.objHndCaller, 207, FloatLineColor::White);
	args.SetCondArg(4, 2);
	return 0;
}

int SpellCallbacks::RemoveSpell(DispatcherCallbackArgs args)
{
	conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
	return 0;
}

int SpellCallbacks::HasCondition(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	if (dispIo->data1 == args.GetData1())
		dispIo->return_val = 1;
	return 0;
}


#pragma endregion

#pragma region Item Callbacks
int ItemCallbacks::SkillBonus(DispatcherCallbackArgs args)
{
	auto skillEnum = args.GetCondArg(0);
	auto bonValue = args.GetCondArg(1);
	auto bonType = args.GetData1();
	if (args.dispKey - 20 == skillEnum)
	{
		auto invIdx = args.GetCondArg( 2);
		auto item = inventory.GetItemAtInvIdx(args.objHndCaller, invIdx);
		auto dispIo =dispatch.DispIoCheckIoType10(args.dispIO);
		auto itemName = description.getDisplayName(item, args.objHndCaller);
		dispIo->bonOut->AddBonusWithDesc(bonValue, bonType, 112, const_cast<char*>(itemName));
	}
	return 0;
}

#pragma endregion 


#pragma region Class Ability Callbacks
int ClassAbilityCallbacks::FeatDamageReduction(DispatcherCallbackArgs args){
	auto drAmt = args.GetCondArg(1);
	auto bypasserBitmask = args.GetData1();
	auto dispIo = dispatch.DispIoCheckIoType4(args.dispIO);
	damage.AddPhysicalDR(&dispIo->damage, drAmt, bypasserBitmask, 126); // 126 is ~Damage Reduction~[TAG_SPECIAL_ABILITIES_DAMAGE_REDUCTION]
	return 0;
}

int ClassAbilityCallbacks::FeatEmptyBody(DispatcherCallbackArgs args){
	
	RadialMenuEntry mainRadEntry;

	mainRadEntry.text = combatSys.GetCombatMesLine(6020); // Empty Body
	mainRadEntry.helpId = ElfHash::Hash("TAG_CLASS_FEATURES_MONK_EMPTY_BODY");
	int parentNode = radialMenus.GetStandardNode(RadialMenuStandardNode::Class);
	int newParent = radialMenus.AddParentChildNode(args.objHndCaller, &mainRadEntry, parentNode);
	mainRadEntry.d20ActionType = D20A_EMPTY_BODY;

	auto duration = args.GetCondArg(2);
	auto actualArg = args.GetCondArgPtr(1);
	RadialMenuEntrySlider setterChild( 6014, 0, duration, actualArg, 6021, ElfHash::Hash("TAG_CLASS_FEATURES_MONK_EMPTY_BODY"));
	radialMenus.AddChildNode(args.objHndCaller, &setterChild, newParent);


	RadialMenuEntryAction activaterChild(6013, D20A_EMPTY_BODY, 0, ElfHash::Hash("TAG_CLASS_FEATURES_MONK_EMPTY_BODY")); // use
	radialMenus.AddChildNode(args.objHndCaller, &activaterChild, newParent);


	return 0;
}

int ClassAbilityCallbacks::FeatEmptyBodyInit(DispatcherCallbackArgs args){
	// init the remaining number of rounds to the Monk's level
	args.SetCondArg(2, objects.StatLevelGet(args.objHndCaller, stat_level_monk));
	return 0;
}

int ClassAbilityCallbacks::FeatEmptyBodyReduceRounds(DispatcherCallbackArgs args)
{
	int numRoundsRem = args.GetCondArg(2);
	auto dispIo = dispatch.DispIoCheckIoType12(args.dispIO);
	if (!dispIo)
	{
		throw TempleException("FeatEmptyBodyReduceRounds: wrong dispatcher type!");
	}

	D20Actn*d20a = dispIo->d20a;
	args.SetCondArg(2, max(0,numRoundsRem - (int)d20a->data1));

	return 0;
}

int ClassAbilityCallbacks::FeatQuiveringPalmRadial(DispatcherCallbackArgs args){

	RadialMenuEntryAction radEntry(5116, D20A_QUIVERING_PALM, 0,ElfHash::Hash("TAG_CLASS_FEATURES_MONK_QUIVERING_PALM"));
	radEntry.AddChildToStandard(args.objHndCaller ,RadialMenuStandardNode::Class);

	return 0;
}

int ClassAbilityCallbacks::FeatQuiveringPalmInit(DispatcherCallbackArgs args){
	args.SetCondArg(2, 1);
	return 0;
}

int ClassAbilityCallbacks::FeatQuiveringPalmPerform(DispatcherCallbackArgs args){
	args.SetCondArg(2, 0);
	return 0;
}

int ClassAbilityCallbacks::FeatQuiveringPalmAvailable(DispatcherCallbackArgs args)
{
	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	if (!dispIo) {
		throw TempleException("GetNumRoundsRemaining: Wrong dispatcher type!");
	}

	if (args.GetCondArg(2))
		dispIo->return_val = 1;

	return 0;
}

int ClassAbilityCallbacks::GetNumRoundsRemaining(DispatcherCallbackArgs args){

	auto dispIo = dispatch.DispIoCheckIoType7(args.dispIO);
	if (!dispIo){
		throw TempleException("GetNumRoundsRemaining: Wrong dispatcher type!");
	}

	if (dispIo->data1 == 1) // getting the number of rounds set by slider
	{
		int numRounds = args.GetCondArg(1);
		if (numRounds < 0) {
			numRounds = 1;
		}
		if (numRounds > args.GetCondArg(2))
			numRounds = max(0, (int)args.GetCondArg(2));
		
		dispIo->data2 = numRounds;
	} 
	
	else if (dispIo->data1 == 2) // getting the max possible number of rounds
	{
		auto numRounds = args.GetCondArg(2);
		dispIo->data2 = numRounds;
	}
	return 0;
}

int ClassAbilityCallbacks::TimedEffectCountdown(DispatcherCallbackArgs args){

	if (!args.GetData1()) // if data1 is set to 0, make it a permanent effect
		return 0;

	int numRoundsRem = args.GetCondArg(2);

	if ( numRoundsRem <= 1 ){
		ConditionRemoveCallback(args);
		return 0;
	}
	args.SetCondArg(2, numRoundsRem - 1);
	return 0;
}
#pragma endregion

int CaptivatingSongOnConditionAdd(DispatcherCallbackArgs args)
{
	
	int spellId = conds.CondNodeGetArg(args.subDispNode->condNode, 0);
	// int duration = conds.CondNodeGetArg(args.subDispNode->condNode, 1);
	int duration = 2;
	if (!spellId)
		return 0;
	SpellPacketBody spellPktBody;
	spellSys.GetSpellPacketBody(spellId, &spellPktBody);
	objHndl singer = spellPktBody.caster;
	ObjectId singerId = objects.GetId(singer);
	memcpy(&args.subDispNode->condNode->args[2], &singerId, sizeof(ObjectId));
	std::vector<int> argg = { duration,0, 0,0,0,0,0,0 };
	//conds.AddTo(args.objHndCaller, "Captivated", { duration,0, 0,0,0,0,0,0 });
	memcpy(&argg[2], &singerId, sizeof(ObjectId));
	conds.AddTo(args.objHndCaller, "Captivated", argg);
	return 0;
}


void Conditions::AddConditionsToTable(){

	static CondStructNew itemSkillBonus("Special Equipment Skill Bonus", 3, false);
	itemSkillBonus.AddHook(dispTypeSkillLevel, DK_SKILL_APPRAISE, itemCallbacks.SkillBonus, 99, 0);

	static CondStructNew perfectSelf("Perfect Self", 3);
	perfectSelf.AddHook(dispTypeTakingDamage2, DK_NONE, classAbilityCallbacks.FeatDamageReduction, 0x4, 0); // 0x4 denotes Magical attacks
	perfectSelf.AddToFeatDictionary(FEAT_MONK_PERFECT_SELF, FEAT_INVALID, 10);


	static CondStructNew emptyBody("Empty Body", 3);
	emptyBody.AddHook(dispTypeRadialMenuEntry, DK_NONE, classAbilityCallbacks.FeatEmptyBody);
	emptyBody.AddHook(dispTypeD20Query, DK_QUE_Empty_Body_Num_Rounds, classAbilityCallbacks.GetNumRoundsRemaining);
	emptyBody.AddHook(dispTypeConditionAdd, DK_NONE, classAbilityCallbacks.FeatEmptyBodyInit);
	emptyBody.AddHook(dispTypeNewDay, DK_NEWDAY_REST, classAbilityCallbacks.FeatEmptyBodyInit);
	emptyBody.AddHook(dispTypeD20ActionPerform, DK_D20A_EMPTY_BODY, classAbilityCallbacks.FeatEmptyBodyReduceRounds);
	emptyBody.AddToFeatDictionary(FEAT_MONK_EMPTY_BODY, FEAT_INVALID, -1);

	static CondStructNew ethereal("Ethereal", 3);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_Is_Ethereal, genericCallbacks.QuerySetReturnVal1);
	ethereal.AddHook(dispTypeBeginRound, DK_NONE, classAbilityCallbacks.TimedEffectCountdown, 1, 0);
	ethereal.AddHook(dispTypeTakingDamage2, DK_NONE, genericCallbacks.AddEtherealDamageImmunity);
	ethereal.AddHook(dispTypeDealingDamage2, DK_NONE, genericCallbacks.EtherealDamageDealingNull);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_AOOPossible, genericCallbacks.QuerySetReturnVal0);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_AOOWillTake, genericCallbacks.QuerySetReturnVal0);
	ethereal.AddHook(dispTypeEffectTooltip, DK_NONE, genericCallbacks.EffectTooltip, 82, 210);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_Critter_Is_Invisible, genericCallbacks.QuerySetReturnVal1);
	ethereal.AddHook(dispTypeConditionAdd, DK_NONE, genericCallbacks.EtherealOnAdd);
	ethereal.AddHook(dispTypeConditionAddFromD20StatusInit, DK_NONE, genericCallbacks.EtherealOnD20StatusInit);
	ethereal.AddHook(dispTypeConditionRemove, DK_NONE, genericCallbacks.EtherealOnRemove);
	ethereal.AddHook(dispTypeTooltip, DK_NONE, genericCallbacks.TooltipUnrepeated, 210, 0);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_IsActionInvalid_CheckAction, genericCallbacks.ActionInvalidQueryTrue);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_Critter_Is_Immune_Poison, genericCallbacks.QuerySetReturnVal1);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_AOOWillTake, genericCallbacks.QuerySetReturnVal0);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_AOOIncurs, genericCallbacks.QuerySetReturnVal0);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_ActionTriggersAOO,genericCallbacks.QuerySetReturnVal0);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_Critter_Has_Freedom_of_Movement, genericCallbacks.QuerySetReturnVal1);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_CanBeAffected_PerformAction, genericCallbacks.QuerySetReturnVal0);
	ethereal.AddHook(dispTypeD20Query, DK_QUE_CanBeAffected_ActionFrame, genericCallbacks.QuerySetReturnVal0);


	static CondStructNew quivPalm("Quivering Palm", 3 );
	quivPalm.AddHook(dispTypeRadialMenuEntry, DK_NONE, classAbilityCallbacks.FeatQuiveringPalmRadial);
	quivPalm.AddHook(dispTypeD20Query, DK_QUE_Quivering_Palm_Can_Perform, classAbilityCallbacks.FeatQuiveringPalmAvailable);
	quivPalm.AddHook(dispTypeConditionAdd, DK_NONE, classAbilityCallbacks.FeatQuiveringPalmInit);
	quivPalm.AddHook(dispTypeNewDay, DK_NEWDAY_REST, classAbilityCallbacks.FeatQuiveringPalmInit);
	quivPalm.AddHook(dispTypeD20ActionPerform, DK_D20A_QUIVERING_PALM, classAbilityCallbacks.FeatQuiveringPalmPerform);
	quivPalm.AddToFeatDictionary(FEAT_MONK_QUIVERING_PALM);


	// New Conditions!
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mConditionDisableAoO);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondGreaterTwoWeaponFighting);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondGreaterTWFRanger);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondDivineMight);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondDivineMightBonus);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondRecklessOffense);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondGreaterWeaponSpecialization);

	conds.hashmethods.CondStructAddToHashtable((CondStruct*)&conds.mCondDisarm);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)&conds.mCondDisarmed);
	// conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondSuperiorExpertise); // will just be patched inside Combat Expertise callbacks
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondRend);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)&conds.mCondCaptivatingSong);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)&conds.mCondCaptivated);
	// conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondCraftWandLevelSet);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondAidAnother);

	conds.hashmethods.CondStructAddToHashtable((CondStruct*)&conds.mCondHezrouStench);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)&conds.mCondHezrouStenchHit);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)&conds.mCondNecklaceOfAdaptation);
	/*
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondDeadlyPrecision);

	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondGreaterRage);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondImprovedDisarm);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondIndomitableWill);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondKnockDown);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondMightyRage);
	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondPersistentSpell);


	conds.hashmethods.CondStructAddToHashtable((CondStruct*)conds.mCondTirelessRage);
	*/
}

int AidAnotherRadialMenu(DispatcherCallbackArgs args)
{
	//MesLine mesLine;
	RadialMenuEntry radMenuAidAnotherMain;
	
	radMenuAidAnotherMain.text = combatSys.GetCombatMesLine(5112);
	radMenuAidAnotherMain.d20ActionType = D20A_NONE;
	radMenuAidAnotherMain.d20ActionData1 = 0;
	radMenuAidAnotherMain.helpId = templeFuncs.StringHash("TAG_AID_ANOTHER");

	int newParent = radialMenus.AddParentChildNode(args.objHndCaller, &radMenuAidAnotherMain, radialMenus.GetStandardNode(RadialMenuStandardNode::Tactical));

	//RadialMenuEntryAction defensiveAssist(6018, D20A_ITEM_CREATION, 0, "TAG_AID_ANOTHER");
	// radialMenus.AddChildNode(args.objHndCaller, &defensiveAssist, newParent);

	RadialMenuEntryAction wakeUp(5113, D20A_AID_ANOTHER_WAKE_UP, 0, "TAG_AID_ANOTHER");
	radialMenus.AddChildNode(args.objHndCaller, &wakeUp, newParent);
	


	return 0;
}
#pragma endregion