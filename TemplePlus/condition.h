#pragma once

#include "common.h"
#include "dispatcher.h"
#include "hashtable.h"

#define GET_DISPIO(ioType, eventObjType ) args.dispIO->AssertType( ioType ); auto dispIo = static_cast< eventObjType *>(args.dispIO);

const uint32_t CondStructHastableAddr = 0x11868F60;
int __cdecl AoODisableRadialMenuInit(DispatcherCallbackArgs args);
int __cdecl AoODisableQueryAoOPossible(DispatcherCallbackArgs args);
extern CondStructNew conditionDisableAoO;
extern CondStructNew conditionGreaterTwoWeaponFighting;

struct CondFeatDictionary;

struct CondHashSystem : ToEEHashtableSystem < CondStruct >
{
	ToEEHashtable<CondStruct>* condHashTable = (ToEEHashtable<CondStruct>*)CondStructHastableAddr;

	CondHashSystem() {
		temple::Dll::RegisterAddressPtr((void**)&condHashTable);
	}

	uint32_t ConditionHashtableInit(ToEEHashtable<CondStruct>* hashtable);

	uint32_t CondStructAddToHashtable(CondStruct* condStruct, bool overriding = false);

	int GetCondStructHashkey(CondStruct* condStruct)
	{
		int N = HashtableNumItems(condHashTable);

		for (int i = 0; i < N;i++)
		{
			if (condStruct == HashtableGetDataPtr(condHashTable, i))
			{
				return HashtableGetKey(condHashTable, i);
			}
		}
		return 0;
	};

	CondStruct * GetCondStruct(uint32_t key)
	{
		CondStruct * condOut = nullptr;
		HashtableSearch(condHashTable, key, &condOut);
		return condOut;
	}

	uint32_t CondStructOverwriteInHashtable(CondStruct * condStruct)
	{
		uint32_t key = StringHash(condStruct->condName);
		CondStruct * condFound;
		uint32_t result = HashtableSearch(condHashTable, key, &condFound);
		if (!result)
		{
			logger->info("Condition Overwrite warning: Condition Struct not found, adding new.");
		}
		result = HashtableOverwriteItem(condHashTable, key, condStruct);
		return result;
	}

};


struct ConditionSystem : temple::AddressTable
{
#pragma region CondStruct definitions
public:
	CondStruct * ConditionGlobal;
	CondNode ** pCondNodeGlobal;
	CondStruct ** ConditionArraySpellEffects;

	CondStruct * ConditionMonsterUndead;
	CondStruct * ConditionSubtypeFire;
	CondStruct * ConditionMonsterOoze;
	CondStruct ** ConditionArrayRace;
	CondStruct ** ConditionArrayClasses;
	CondStruct * ConditionTurnUndead;
	CondStruct * ConditionGreaterTurning;
	CondStruct * ConditionBardicMusic;
	CondStruct * ConditionSchoolSpecialization;
	CondStruct ** ConditionArrayDomains;
	uint32_t * ConditionArrayDomainsArg1;
	uint32_t * ConditionArrayDomainsArg2;
	CondFeatDictionary * FeatConditionDict; //102EEBF0   includes feats
	uint32_t FeatConditionDictSize;
	CondStruct * ConditionAttackOfOpportunity;
	CondStruct * ConditionCastDefensively;
	CondStruct * ConditionCombatCasting;
	CondStruct * ConditionDealSubdualDamage;
	CondStruct * ConditionDealNormalDamage;
	CondStruct * ConditionFightDefensively;
	CondStruct * ConditionDisabled;
	CondStruct * ConditionUnconscious;
	CondStruct * ConditionAnimalCompanionAnimal;
	CondStruct * ConditionAutoendTurn;



	CondHashSystem hashmethods;

	ToEEHashtable<CondStruct> * mCondStructHashtable;

	char mConditionDisableAoOName [100];
	CondStructNew* mConditionDisableAoO;
	char mConditionGreaterTwoWeaponFightingName[100];
	CondStructNew* mCondGreaterTwoWeaponFighting;
	char mCondGreaterTWFRangerName[100];
	CondStructNew* mCondGreaterTWFRanger;
	char mCondDivineMightName[100];
	CondStructNew* mCondDivineMight;
	char mCondDivineMightBonusName[100];
	CondStructNew* mCondDivineMightBonus;

	char mCondRecklessOffenseName[100];
	CondStructNew *mCondRecklessOffense;
	char mCondKnockDownName[100];
	CondStructNew *mCondKnockDown;
	char mCondSuperiorExpertiseName[100];
	CondStructNew *mCondSuperiorExpertise;
	char mCondDeadlyPrecisionName[100];
	CondStructNew *mCondDeadlyPrecision;
	char mCondPersistentSpellName[100];
	CondStructNew *mCondPersistentSpell;

	char mCondGreaterWeaponSpecializationName[100];
	CondStructNew * mCondGreaterWeaponSpecialization;


	char mCondGreaterRageName[100];
	CondStructNew *mCondGreaterRage;
	char mCondIndomitableWillName[100];
	CondStructNew *mCondIndomitableWill;
	char mCondTirelessRageName[100];
	CondStructNew *mCondTirelessRage;
	char mCondMightyRageName[100];
	CondStructNew *mCondMightyRage;
	const char mCondDisarmName[100] = "Disarm";
	static CondStructNew mCondDisarm;
	const char mCondDisarmedName[100] = "Disarmed";
	static CondStructNew mCondDisarmed;
	char mCondImprovedDisarmName[100];
	CondStructNew *mCondImprovedDisarm;



	char mCondAidAnotherName[100];
	CondStructNew * mCondAidAnother;

	const char mCondNecklaceOfAdaptationName[100] = { "Neckalce of Adaptation" };
	static CondStructNew mCondNecklaceOfAdaptation;

	// monsters
	char mCondRendName[100];
	CondStructNew *mCondRend;

	const char mCondCaptivatingSongName[100] = { "Captivating Song" };
	static CondStructNew mCondCaptivatingSong;
	const char mCondCaptivatedName[100] = { "Captivated" };
	static CondStructNew mCondCaptivated;

	static CondStructNew mCondHezrouStench;
	static CondStructNew mCondHezrouStenchHit;

	/*
		Returns the condition definition with the given name,
		null if none exists.
	*/
	CondStruct *GetByName(const string &name);
	CondStruct* GetById(const int condId);

	void DoForAllCondStruct( void(__cdecl*cb)(CondStruct & condStruct));

	/*
		Adds a condition to an item's obj_f_item_pad_wielder_condition_array and 
		obj_f_item_pad_wielder_argument_array.
	*/
	void AddToItem(objHndl item, const CondStruct *cond, const vector<int> &args);

	/*
		Adds a condition to an object. There is no type restriction for the target
		object, but usually it should be a critter.
	*/
	bool AddTo(objHndl handle, const CondStruct* cond, const vector<int> &args);

	/*
		Adds a condition to an object by name. There is no type restriction for the target
		object, but usually it should be a critter.
	*/
	bool AddTo(objHndl handle, const string &name, const vector<int> &args);

	bool ConditionAddDispatchArgs(Dispatcher * dispatcher, CondNode **, CondStruct* condStruct, const vector<int> &args);

	/*
		Get/Set a Condition Node's arg. Often used in the init callbacks of various conditions.
	*/
	int32_t CondNodeGetArg(CondNode* condNode, uint32_t argIdx);
	int * CondNodeGetArgPtr(CondNode* condNode, uint32_t argIdx);
	void CondNodeSetArg(CondNode* condNode, uint32_t argIdx, uint32_t argVal);

	void CondNodeAddToSubDispNodeArray(Dispatcher* dispatcher, CondNode* condNodeNew);
	void InitCondFromCondStructAndArgs(Dispatcher* dispatcher, CondStruct* condStruct, int* condargs);
	void InitItemCondFromCondStructAndArgs(Dispatcher* dispatcher, CondStruct* condStruct, int* condargs);
	

#pragma endregion
	void RegisterNewConditions();
	
	static void AddToFeatDictionary(CondStructNew* condStruct,feat_enums feat, feat_enums featEnumMax, uint32_t condArg2Offset);
	/*
		used for initializing new SubDispDef's with the specified values
	*/
	void DispatcherHookInit( SubDispDefNew * sdd, enum_disp_type dispType,  int key, void * callback, int data1, int data2);
	void DispatcherHookInit( CondStructNew * cond , int hookIdx, enum_disp_type dispType, int key, void * callback, int data1, int data2);
	ConditionSystem()
	{

		rebase(ConditionGlobal, 0x102E8088);
		rebase(pCondNodeGlobal, 0x10BCADA0);
		rebase(ConditionArraySpellEffects, 0x102E2600);
		rebase(ConditionMonsterUndead, 0x102EF9A8);
		rebase(ConditionSubtypeFire, 0x102EFBE8);
		rebase(ConditionMonsterOoze, 0x102EFAF0);
		rebase(ConditionArrayRace, 0x102EFC18);
		rebase(ConditionArrayClasses, 0x102F0634);
		rebase(ConditionTurnUndead, 0x102B0D48);
		rebase(ConditionGreaterTurning, 0x102B0DE0);
		rebase(ConditionBardicMusic, 0x102F0520);
		rebase(ConditionSchoolSpecialization, 0x102F0604);
		rebase(ConditionArrayDomains, 0x102B1690);
		rebase(ConditionArrayDomainsArg1, 0x102B1694);
		rebase(ConditionArrayDomainsArg2, 0x102B1698);
		rebase(FeatConditionDict, 0x102EEBF0);
		rebase(ConditionAttackOfOpportunity, 0x102ED5B0);
		rebase(ConditionCastDefensively, 0x102ED630);
		rebase(ConditionCombatCasting ,0x102ED738);
		rebase(ConditionDealSubdualDamage, 0x102ED790);
		rebase(ConditionDealNormalDamage, 0x102ED828);
		rebase(ConditionFightDefensively, 0x102ECC38);
		rebase(ConditionDisabled, 0x102E4A70);
		rebase(ConditionUnconscious, 0x102E4CB0);
		rebase(ConditionAnimalCompanionAnimal, 0x102EE590);
		rebase(ConditionAutoendTurn, 0x102ED6C8);
		
		rebase(mCondStructHashtable, 0x11868F60);
			
	}

	void SetPermanentModArgsFromDataFields(Dispatcher* dispatcher, CondStruct* condStruct, int *condArgs);
	void DispatcherCondsResetFlag2(Dispatcher* dispatcher);
	int GetActiveCondsNum(Dispatcher* dispatcher);
	int GetPermanentModsAndItemCondCount(Dispatcher* dispatcher);
	int ConditionsExtractInfo(Dispatcher* dispatcher, int condIdx, int* hashkeyOut, int *condsArgsOut);
	int PermanentAndItemModsExtractInfo(Dispatcher* dispatcher, int permModIdx, int* hashkeyOut, int * condsArgs);
	void ConditionRemove(objHndl objHnd, CondNode* cond);
	
};

extern ConditionSystem conds;




struct CondFeatDictionary  // maps feat enums to CondStructs
{
	union
	{
		CondStruct * old;
		CondStructNew* cs;
	} condStruct;
	feat_enums featEnum;
	feat_enums featEnumMax;
	uint32_t condArg; 
	CondFeatDictionary();
	CondFeatDictionary(CondStructNew*, feat_enums Feat, feat_enums FeatMax, uint32_t arg2Off);
};


int32_t _CondNodeGetArg(CondNode* condNode, uint32_t argIdx);
void _CondNodeSetArg(CondNode* condNode, uint32_t argIdx, uint32_t argVal);
uint32_t _ConditionAddDispatch(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
uint32_t _ConditionAddDispatchArgs(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, const vector<int> &args);
void _CondNodeAddToSubDispNodeArray(Dispatcher* dispatcher, CondNode* condNode);
uint32_t _ConditionAddToAttribs_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct, bool isInternalUse = true);
uint32_t _ConditionAddToAttribs_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, bool isInternalUse = true);
uint32_t _ConditionAdd_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct, bool isInternalUse = true);
uint32_t _ConditionAdd_NumArgs1(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, bool isInternalUse = true);
uint32_t _ConditionAdd_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, bool isInternalUse = true);
uint32_t _ConditionAdd_NumArgs3(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, bool isInternalUse = true);
uint32_t _ConditionAdd_NumArgs4(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, bool isInternalUse = true);
void InitCondFromCondStructAndArgs(Dispatcher *dispatcher, CondStruct *condStruct, int *condargs);



int ConditionPrevent(DispatcherCallbackArgs args);
int ConditionRemoveCallback(DispatcherCallbackArgs args);
int RemoveSpellConditionAndMod(DispatcherCallbackArgs args);
int CondNodeSetArg0FromSubDispDef(DispatcherCallbackArgs args);
int QueryCritterHasCondition(DispatcherCallbackArgs args);
int QueryRetrun1GetArgs(DispatcherCallbackArgs args);
int ItemSkillBonusCallback(DispatcherCallbackArgs args); 

int GlobalToHitBonus(DispatcherCallbackArgs args);
int GlobalGetArmorClass(DispatcherCallbackArgs args);
int GlobalOnDamage(DispatcherCallbackArgs args);

int DispelCheck(DispatcherCallbackArgs args);
int DispelAlignmentTouchAttackSignalHandler(DispatcherCallbackArgs args);

int GreaterTwoWeaponFighting(DispatcherCallbackArgs args);
int GreaterTWFRanger(DispatcherCallbackArgs args);
int TwoWeaponFightingBonus(DispatcherCallbackArgs args);
int TwoWeaponFightingBonusRanger(DispatcherCallbackArgs args);
int DivineMightRadial(DispatcherCallbackArgs args);
int DivineMightDamageBonus(DispatcherCallbackArgs args);
int GreaterWeaponSpecializationDamage(DispatcherCallbackArgs args);
int RecklessOffenseRadialMenuInit(DispatcherCallbackArgs args);
int RecklessOffenseAcPenalty(DispatcherCallbackArgs args);
int RecklessOffenseToHitBonus(DispatcherCallbackArgs args);
int TacticalOptionAbusePrevention(DispatcherCallbackArgs args);

int HeldCapStatBonus(DispatcherCallbackArgs args);
int HelplessCapStatBonus(DispatcherCallbackArgs args);
int ParalyzeCheckRemove(DispatcherCallbackArgs args);
int ParalyzeEffectTooltip(DispatcherCallbackArgs args);
int HelplessConditionRemoved(DispatcherCallbackArgs args);

int MonsterMeleeParalysisApply(DispatcherCallbackArgs args);
int MonsterMeleeParalysisNoElfApply(DispatcherCallbackArgs args);

int CombatExpertiseRadialMenu(DispatcherCallbackArgs args);
int CombatExpertiseSet(DispatcherCallbackArgs args);


int BarbarianRageStatBonus(DispatcherCallbackArgs args);
int BarbarianRageSaveBonus(DispatcherCallbackArgs args);
int BarbarianRageACPenalty(DispatcherCallbackArgs args);
int BarbarianDamageResistance(DispatcherCallbackArgs args);
uint32_t BarbarianAddFatigue(objHndl critter, CondStruct* cond);

int NonlethalDamageRadial(DispatcherCallbackArgs args);
int DealNormalDamageCallback(DispatcherCallbackArgs args);
int NonlethalDamageSetSubdual(DispatcherCallbackArgs args);
int DealNormalDamageAttackPenalty(DispatcherCallbackArgs args);

int DisarmedOnAdd(DispatcherCallbackArgs args);
int DisarmedRetrieveQuery(DispatcherCallbackArgs args);
int DisarmRadialMenu(DispatcherCallbackArgs args);
int DisarmHpChanged(DispatcherCallbackArgs args);
int DisarmQueryAoOResetArg(DispatcherCallbackArgs args);
int DisarmCanPerform(DispatcherCallbackArgs args);
int DisarmedReminder(DispatcherCallbackArgs args);
int DisarmedWeaponRetrieve(DispatcherCallbackArgs args);
int DisarmedRetrieveWeaponRadialMenu(DispatcherCallbackArgs args);

int GetMaxDexBonus(objHndl armor);
int GetArmorCheckPenalty(objHndl armor);

int RendOnDamage(DispatcherCallbackArgs args);

int AidAnotherRadialMenu(DispatcherCallbackArgs args);

void _FeatConditionsRegister();
uint32_t  _GetCondStructFromFeat(feat_enums featEnum, CondStruct ** ppCondStruct, uint32_t * arg2);
uint32_t _CondStructAddToHashtable(CondStruct * condStruct);
CondStruct * _GetCondStructFromHashcode(uint32_t key);