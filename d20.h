#pragma once

#include "dispatcher.h"
#include "common.h"
#include "d20_defs.h"
#include "spell.h"

struct ActionSequenceSystem;
// Forward decls
struct D20Actn;
struct ActnSeq;
struct TurnBasedStatus;
enum SpontCastType;
struct D20SpellData;
enum D20ActionType : int32_t;
enum D20CAF : uint32_t;
enum SpontCastType : unsigned char;
struct D20SpellData;
struct D20Actn;
struct D20ActionDef;
struct ActnSeq;
struct PathQueryResult;
struct Pathfinding;



struct D20System : AddressTable
{
	void D20StatusInitRace(objHndl objHnd);
	void D20StatusInitClass(objHndl objHnd);
	void D20StatusInit(objHndl objHnd);
	void D20StatusInitDomains(objHndl objHnd);
	void D20StatusInitFeats(objHndl objHnd);
	void D20StatusInitItemConditions(objHndl objHnd);
	uint32_t d20Query(objHndl ObjHnd, D20DispatcherKey dispKey);
	uint32_t d20QueryWithData(objHndl ObjHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2);

	void d20ActnInit(objHndl objHnd, D20Actn * d20a);
	void globD20aSetTypeAndData1(D20ActionType d20type, uint32_t data1);
	void d20aTriggerCombatCheck(ActnSeq* actSeq, int32_t idx);//1008AE90    ActnSeq * @<eax>
	int32_t d20aTriggersAOOCheck(D20Actn * d20a, void * iO);// 1008A9C0
	uint32_t tumbleCheck(D20Actn*); 

	ActionSequenceSystem * actSeq;

	D20Actn * globD20Action;
	D20ActionDef * d20Defs;
	void (__cdecl *D20StatusInitFromInternalFields)(objHndl objHnd, Dispatcher *dispatcher);
	void (__cdecl *AppendObjHndToArray10BCAD94)(objHndl ObjHnd);
	void(__cdecl * _d20aTriggerCombatCheck)(int32_t idx);//1008AE90    ActnSeq * @<eax>
	uint32_t * D20GlobalSthg10AA3284;
	void(__cdecl *ToHitProc)(D20Actn *);
	uint32_t (__cdecl*_tumbleCheck)(D20Actn* d20a);
	int32_t (__cdecl *_d20aTriggersAOO)(void * iO); // d20a @<esi> // 1008A9C0

	Pathfinding * pathfinding;
	//char **ToEEd20ActionNames;

	D20System();
};


extern D20System d20sys;


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

void _D20StatusInitRace(objHndl objHnd);
void _D20StatusInitClass(objHndl objHnd);
void _D20StatusInit(objHndl objHnd);
void _D20StatusInitDomains(objHndl objHnd);
void _D20StatusInitFeats(objHndl objHnd);
void _D20StatusInitItemConditions(objHndl objHnd);
uint32_t _D20Query(objHndl objHnd, D20DispatcherKey dispKey);
void __cdecl D20SpellDataSetSpontCast(D20SpellData*, SpontCastType spontCastType);
void D20SpellDataExtractInfo
(D20SpellData * d20SpellData, uint32_t * spellEnum, uint32_t * spellEnumOriginal, uint32_t * spellClassCode, uint32_t * spellSlotLevel, uint32_t * itemSpellData, uint32_t * metaMagicData);
void _d20aInitUsercallWrapper(objHndl objHnd);
void _globD20aSetTypeAndData1(D20ActionType d20type, uint32_t data1);
uint32_t _d20QueryWithData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2);

struct D20SpellData
{
	uint16_t spellEnumOrg;
	MetaMagicData metaMagicData;
	uint8_t spellClassCode;
	uint8_t itemSpellData;
	SpontCastType spontCastType : 4;
	unsigned char spellSlotLevel : 4;
};

const uint32_t TestSizeOfD20SpellData = sizeof(D20SpellData);

#pragma region D20 Action and Action Sequence Structs

struct D20Actn
{
	D20ActionType d20ActType;
	uint32_t data1;
	D20CAF d20Caf;
	uint32_t field_C;
	objHndl d20APerformer;
	objHndl d20ATarget;
	LocAndOffsets destLoc;
	float distTraversed;
	uint32_t field_34;
	uint32_t rollHist3;
	uint32_t rollHist1;
	uint32_t rollHist2;
	D20SpellData d20SpellData;
	uint32_t spellEnum;
	uint32_t animID;
	PathQueryResult * path;
};


struct D20ActionDef
{
	uint32_t (__cdecl *addToSeqFunc)(D20Actn *, ActnSeq *, TurnBasedStatus*iO);
	uint32_t (__cdecl* unknownFunc1)(D20Actn* d20a, TurnBasedStatus* iO);
	uint32_t (__cdecl * actionCheckFunc)(D20Actn* d20a, TurnBasedStatus* iO);
	uint32_t (__cdecl * tgtCheckFunc)(D20Actn* d20a, TurnBasedStatus* iO);
	uint32_t (__cdecl * locCheckFunc)(D20Actn* d20a, TurnBasedStatus* iO, LocAndOffsets * locAndOff); // also seems to double as a generic check function (e.g. for move silently it checks if combat is active and nothing to do with location)
	uint32_t (__cdecl* performFunc)(D20Actn* d20a);
	void * actionFrameFunc;
	void * projectilePerformFunc;
	uint32_t pad_apparently;
	uint32_t (__cdecl * moveFunc)(D20Actn* d20a, TurnBasedStatus* iO, LocAndOffsets * locAndOff); 
	void * unknownFunc3;
	uint32_t flags; // not D20CAF I think; maybe the STD flags? path query flags?
};

#pragma endregion



enum SpontCastType : unsigned char{
	spontCastGoodCleric = 2,
	spontCastEvilCleric = 4,
	spontCastDruid = 8
};

#pragma endregion 