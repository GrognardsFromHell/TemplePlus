#pragma once

#include "dispatcher.h"
#include "common.h"
#include "d20_defs.h"
#include "spell.h"


struct D20Action;
struct ActionSequence;
enum SpontCastType;
struct D20SpellData;
enum D20ActionType : uint32_t;
enum D20CAF : uint32_t;




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


#pragma region D20 Action and Action Sequence Structs
struct D20Action
{
	D20ActionType d20ActType;
	uint32_t data1;
	D20CAF d20Caf;
	uint32_t field_C;
	objHndl d20APerformer;
	objHndl d20ATarget;
	LocAndOffsets locAndOff;
	uint32_t field_30;
	uint32_t field_34;
	uint32_t rollHist3;
	uint32_t rollHist1;
	uint32_t rollHist2;
	D20SpellData d20SpellData;
	uint32_t spellEnum;
	uint32_t animID;
	void * path;
};


struct ActionSequence
{
	D20Action d20ActArray[32];
	uint32_t d20ActArrayNum;
	ActionSequence * prevSeq;
	uint32_t field_B0C;
	uint32_t seqOccupied;
	uint32_t field_B14;
	D20CAF callActionFrameFlags;
	uint32_t idxSthg;
	uint32_t field_B20;
	uint32_t field_B24;
	uint32_t field_B28;
	uint32_t field_B2C;
	uint32_t field_B30;
	uint32_t field_B34;
	objHndl performer;
	LocAndOffsets locAndOff;
	objHndl unknown_maybeInteruptee;
	SpellPacketBody spellPktBody;
	D20Action * d20Action;
	uint32_t field_1644_maybe_spellAssignedFlag;
};

const uint32_t TestSizeOfActionSequence = sizeof(ActionSequence); // should be 0x1648 (5704)

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
	D20Action * globD20Action;

	void (__cdecl *D20StatusInitFromInternalFields)(objHndl objHnd, Dispatcher *dispatcher);
	void (__cdecl *AppendObjHndToArray10BCAD94)(objHndl ObjHnd);
	uint32_t * D20GlobalSthg10AA3284;



	D20System()
	{
		rebase(D20StatusInitFromInternalFields, 0x1004F910);
		rebase(AppendObjHndToArray10BCAD94, 0x100DFAD0);
		rebase(D20GlobalSthg10AA3284, 0x10AA3284);
		rebase(globD20Action, 0x1186AC00);
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
