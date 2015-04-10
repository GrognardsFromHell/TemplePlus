#pragma once

#include "common.h"
#include "dispatcher.h"
#include "hashtable.h"

const uint32_t CondStructHastableAddr = 0x11868F60;


struct CondFeatDictionary;

struct CondHashSystem : ToEEHashtableSystem < CondStruct >
{
	uint32_t ConditionHashtableInit(ToEEHashtable<CondStruct> * hashtable)
	{
		return HashtableInit(hashtable, 1000);
	}

	uint32_t CondStructAddToHashtable(CondStruct * condStruct)
	{
		uint32_t key = StringHash(condStruct->condName);
		CondStruct * condFound;
		uint32_t result = HashtableSearch((ToEEHashtable < CondStruct >*)temple_address(CondStructHastableAddr), key, &condFound);
		if (result)
		{
			result = HashtableAddItem((ToEEHashtable < CondStruct >*)temple_address(CondStructHastableAddr), key, condStruct);
		}
		return result;
	}

	CondStruct * GetCondStruct(uint32_t key)
	{
		CondStruct * condOut = nullptr;
		HashtableSearch((ToEEHashtable < CondStruct >*)temple_address(CondStructHastableAddr), key, &condOut);
		return condOut;
	}

};


struct ConditionSystem : AddressTable
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
	CondStruct * ConditionBardicMusic;
	CondStruct * ConditionSchoolSpecialization;
	CondStruct ** ConditionArrayDomains;
	uint32_t * ConditionArrayDomainsArg1;
	uint32_t * ConditionArrayDomainsArg2;
	CondFeatDictionary * FeatConditionDict; //102EEBF0   includes feats
	CondStruct * ConditionAttackOfOpportunity;
	CondStruct * ConditionCastDefensively;
	CondStruct * ConditionDealSubdualDamage;
	CondStruct * ConditionDealNormalDamage;
	CondStruct * ConditionFightDefensively;
	CondStruct * ConditionDisabled;
	CondStruct * ConditionUnconscious;



	CondHashSystem hashmethods;

	ToEEHashtable<CondStruct> * mCondStructHashtable;


#pragma endregion

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
		rebase(ConditionBardicMusic, 0x102F0520);
		rebase(ConditionSchoolSpecialization, 0x102F0604);
		rebase(ConditionArrayDomains, 0x102B1690);
		rebase(ConditionArrayDomainsArg1, 0x102B1694);
		rebase(ConditionArrayDomainsArg2, 0x102B1698);
		rebase(FeatConditionDict, 0x102EEBF0);
		rebase(ConditionAttackOfOpportunity, 0x102ED5B0);
		rebase(ConditionCastDefensively, 0x102ED630);
		rebase(ConditionDealSubdualDamage, 0x102ED790);
		rebase(ConditionDealNormalDamage, 0x102ED828);
		rebase(ConditionFightDefensively, 0x102ECC38);
		rebase(ConditionDisabled, 0x102E4A70);
		rebase(ConditionUnconscious, 0x102E4CB0);
		
		rebase(mCondStructHashtable, 0x11868F60);
	}

};

extern ConditionSystem conds;




struct CondFeatDictionary  // maps feat enums to CondStructs
{
	CondStruct * condStruct;
	feat_enums featEnum;
	feat_enums featEnumMax;
	uint32_t condArg2Offset; // the GetCondStruct
};

uint32_t _ConditionAddDispatch(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
void _CondNodeAddToSubDispNodeArray(Dispatcher* dispatcher, CondNode* condNode);
uint32_t _ConditionAddToAttribs_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct);
uint32_t _ConditionAddToAttribs_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2);
uint32_t _ConditionAdd_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct);
uint32_t _ConditionAdd_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2);
uint32_t _ConditionAdd_NumArgs3(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3);
uint32_t _ConditionAdd_NumArgs4(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);


uint32_t ConditionPrevent(DispatcherCallbackArgs args);

uint32_t  _GetCondStructFromFeat(feat_enums featEnum, CondStruct ** ppCondStruct, uint32_t * arg2);
uint32_t _CondStructAddToHashtable(CondStruct * condStruct);
CondStruct * _GetCondStructFromHashcode(uint32_t key);