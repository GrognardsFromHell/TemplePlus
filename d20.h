#pragma once

#include "dispatcher.h"
#include "common.h"
#include "d20_defs.h"


enum SpontCastType : unsigned char {
	spontCastGoodCleric = 2,
	spontCastEvilCleric = 4,
	spontCastDruid = 8
};

struct D20SpellData
{
	uint16_t spellEnumOriginal;
	MetaMagicData metaMagicData;
	uint8_t spellClassCode ;
	uint8_t itemSpellData;
	SpontCastType spontCastType: 4;
	unsigned char spellSlotLevel : 4;
};

const uint32_t TestSizeOfD20SpellData = sizeof(D20SpellData);


void __cdecl D20SpellDataSetSpontCast(D20SpellData*, SpontCastType spontCastType);
void D20SpellDataExtractInfo
(D20SpellData * d20SpellData, uint32_t * spellEnum, uint32_t * spellEnumOriginal,
uint32_t * spellClassCode, uint32_t * spellSlotLevel, uint32_t * itemSpellData,
uint32_t * metaMagicData);


void DispIO_Size32_Type21_Init(DispIO20h* dispIO);
uint32_t Dispatch62(objHndl, DispIO*, uint32_t dispKey);
uint32_t Dispatch63(objHndl objHnd, DispIO* dispIO);
uint32_t ConditionAddDispatch(Dispatcher* dispatcher, CondNode** ppCondNode, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
void CondNodeAddToSubDispNodeArray(Dispatcher* dispatcher, CondNode* condNode);
uint32_t ConditionAddToAttribs_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct);
uint32_t ConditionAddToAttribs_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2);
uint32_t ConditionAdd_NumArgs0(Dispatcher* dispatcher, CondStruct* condStruct);
uint32_t ConditionAdd_NumArgs2(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2);
uint32_t ConditionAdd_NumArgs3(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3);
uint32_t ConditionAdd_NumArgs4(Dispatcher* dispatcher, CondStruct* condStruct, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);


DispIO14h * DispIO14hCheckDispIOType1(DispIO14h * dispIO);
uint32_t ConditionPrevent(DispatcherCallbackArgs args);

void D20StatusInitRace(objHndl objHnd);
void D20StatusInitClass(objHndl objHnd);
void D20StatusInit(objHndl objHnd);
void D20StatusInitDomains(objHndl objHnd);



struct CharacterClasses : AddressTable
{
public:
	Stat charClassEnums[NUM_CLASSES];
	CharacterClasses()
	{
		Stat _charClassEnums[NUM_CLASSES] = { stat_level_barbarian, stat_level_bard, stat_level_cleric, stat_level_druid, stat_level_fighter, stat_level_monk, stat_level_paladin, stat_level_ranger, stat_level_rogue, stat_level_sorcerer, stat_level_wizard };
		memcpy(charClassEnums, _charClassEnums, NUM_CLASSES * sizeof(uint32_t));
	};
};

extern CharacterClasses charClasses;

struct ConditionStructs : AddressTable
{
	CondStruct * ConditionGlobal;
	CondNode ** pCondNodeGlobal;
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


	ConditionStructs()
	{
		rebase(ConditionGlobal, 0x102E8088);
		rebase(pCondNodeGlobal, 0x10BCADA0);
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
	}

};

extern ConditionStructs conds;