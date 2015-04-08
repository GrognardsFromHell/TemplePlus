#pragma once

#include "dispatcher.h"
#include "common.h"
#include "d20_defs.h"


#pragma region D20 Spell Related Structs

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


#pragma endregion

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

struct D20System : AddressTable
{
	void D20StatusInitRace(objHndl objHnd);
	void D20StatusInitClass(objHndl objHnd);
	void D20StatusInit(objHndl objHnd);
	void D20StatusInitDomains(objHndl objHnd);
	void D20StatusInitFeats(objHndl objHnd);
	void D20StatusInitItemConditions(objHndl objHnd);
	uint32_t D20Query(objHndl ObjHnd, D20DispatcherKey dispKey);

	void (__cdecl *D20StatusInitFromInternalFields)(objHndl objHnd, Dispatcher *dispatcher);
	void (__cdecl *AppendObjHndToArray10BCAD94)(objHndl ObjHnd);
	uint32_t * D20Global10AA3284;


	D20System()
	{
		rebase(D20StatusInitFromInternalFields, 0x1004F910);
		rebase(AppendObjHndToArray10BCAD94, 0x100DFAD0);
		rebase(D20Global10AA3284, 0x10AA3284);
	};
};


extern D20System d20;




void _D20StatusInitRace(objHndl objHnd);
void _D20StatusInitClass(objHndl objHnd);
void _D20StatusInit(objHndl objHnd);
void _D20StatusInitDomains(objHndl objHnd);
void _D20StatusInitFeats(objHndl objHnd);
void _D20StatusInitItemConditions(objHndl objHnd);
uint32_t _D20Query(objHndl objHnd, D20DispatcherKey dispKey);
