#pragma once

#include "dispatcher.h"
#include "common.h"
#include "d20_defs.h"
#include "d20_class.h"
#include "spell.h"
#include "d20_status.h"

#define ATTACK_CODE_PRIMARY 0
#define ATTACK_CODE_OFFHAND 99 // originally 4
#define ATTACK_CODE_NATURAL_ATTACK 999 //originally 9
#include "tab_file.h"

struct GameSystemConf;
struct ActionSequenceSystem;
// Forward decls
struct D20Actn;
struct ActnSeq;
struct TurnBasedStatus;
enum SpontCastType;
struct D20SpellData;
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
	D20Actn * globD20Action;
	D20ActionDef * d20Defs;
	Pathfinding * pathfinding;
	ActionSequenceSystem * actSeq;
	D20ClassSystem * d20Class;
	D20StatusSystem * d20Status;
	TabFileStatus* d20ActionsTabFile;
	uint32_t(* d20actionTabLineParser)(TabFileStatus*, uint32_t, const char**);
	void d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, int32_t arg1, int32_t arg2);
	void d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, D20Actn* arg1, int32_t arg2);
	void d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, objHndl arg);

	uint32_t d20Query(objHndl ObjHnd, D20DispatcherKey dispKey);
	uint32_t d20QueryWithData(objHndl ObjHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2);
	uint64_t d20QueryReturnData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2);

	void D20ActnInit(objHndl objHnd, D20Actn * d20a);
	void GlobD20ActnSetTypeAndData1(D20ActionType d20type, uint32_t data1);
	void globD20ActnSetPerformer(objHndl objHnd);
	void GlobD20ActnSetTarget(objHndl objHnd, LocAndOffsets * loc);
	void GlobD20ActnInit();
	void d20aTriggerCombatCheck(ActnSeq* actSeq, int32_t idx);//1008AE90    ActnSeq * @<eax>
	int32_t d20aTriggersAOOCheck(D20Actn * d20a, void * iO);// 1008A9C0
	uint32_t tumbleCheck(D20Actn*);
	void D20ActnSetSpellData(D20SpellData* d20SpellData, uint32_t spellEnumOrg, uint32_t spellClassCode, uint32_t spellSlotLevel, uint32_t itemSpellData, uint32_t metaMagicData);
	void GlobD20ActnSetSpellData(D20SpellData* d20SpellData);
	bool UsingSecondaryWeapon(D20Actn* d20a);
	bool UsingSecondaryWeapon(objHndl obj, int attackCode);
	void ExtractAttackNumber(objHndl obj, int attackCode, int * attackNumber, int* dualWielding); // e.g. is it a 2nd attack? (-5 penalty)
	objHndl GetAttackWeapon(objHndl obj, int attackCode, D20CAF flags);
	int PerformStandardAttack(D20Actn* d20a);
	void (__cdecl *D20StatusInitFromInternalFields)(objHndl objHnd, Dispatcher *dispatcher);
	void (__cdecl *D20ObjRegistryAppend)(objHndl ObjHnd);
	void(__cdecl * _d20aTriggerCombatCheck)(int32_t idx);//1008AE90    ActnSeq * @<eax>
	uint32_t * d20EditorMode;
	void(__cdecl *ToHitProc)(D20Actn *);
	uint32_t (__cdecl*_tumbleCheck)(D20Actn* d20a);
	int32_t (__cdecl *_d20aTriggersAOO)(void * iO); // d20a @<esi> // 1008A9C0

	void (__cdecl *CreateRollHistory)(int idx);

	//char **ToEEd20ActionNames;
	void NewD20ActionsInit();

	D20System();
};


extern D20System d20Sys;

int _D20Init(GameSystemConf* conf);


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
	int d20Caf; // Based on D20_CAF
	uint32_t field_C;
	objHndl d20APerformer;
	objHndl d20ATarget;
	LocAndOffsets destLoc;
	float distTraversed;
	uint32_t radialMenuActualArg;
	uint32_t rollHist3;
	uint32_t rollHist1;
	uint32_t rollHist2;
	D20SpellData d20SpellData;
	uint32_t spellId;
	uint32_t animID;
	PathQueryResult * path;

	D20Actn() {}

	D20Actn(D20ActionType type) {
		rollHist1 = -1;
		rollHist2 = -1;
		rollHist3 = -1;
		d20ActType = type;
		d20APerformer = 0;
		d20ATarget = 0;
		d20Caf = 0;
		distTraversed = 0;
		path = 0;
		spellId = 0;
		data1 = 0;
	}
};

const auto TestSizeOfD20Action = sizeof(D20Actn); // should be 88 (0x58)

struct ActionCostPacket
{
	int hourglassCost;
	int chargeAfterPicker; // flag I think; is only set at stuff that requires using the picker it seems
	float moveDistCost;
};
const auto TestSizeOfActionCostPacket = sizeof(ActionCostPacket); // should be 12 (0xC)

struct D20ActionDef
{
	uint32_t (__cdecl *addToSeqFunc)(D20Actn *, ActnSeq *, TurnBasedStatus*iO);
	uint32_t (__cdecl* aiCheckMaybe)(D20Actn* d20a, TurnBasedStatus* iO);
	uint32_t (__cdecl * actionCheckFunc)(D20Actn* d20a, TurnBasedStatus* iO);
	uint32_t (__cdecl * tgtCheckFunc)(D20Actn* d20a, TurnBasedStatus* iO);
	uint32_t (__cdecl * locCheckFunc)(D20Actn* d20a, TurnBasedStatus* iO, LocAndOffsets * locAndOff); // also seems to double as a generic check function (e.g. for move silently it checks if combat is active and nothing to do with location)
	uint32_t (__cdecl * performFunc)(D20Actn* d20a);
	uint32_t (__cdecl * actionFrameFunc)(D20Actn* d20a);
	void * projectilePerformFunc;
	uint32_t pad_apparently;
	uint32_t(__cdecl * actionCost)(D20Actn* d20a, TurnBasedStatus* iO, ActionCostPacket * actionCostPacket);
	void (__cdecl * unknownFunc3)(D20Actn* d20a, int flags);
	uint32_t flags; // not D20CAF I think; maybe the STD flags? path query flags?
};




enum SpontCastType : unsigned char{
	spontCastGoodCleric = 2,
	spontCastEvilCleric = 4,
	spontCastDruid = 8
};

#pragma endregion 

#pragma region D20 Action Function Replacements
void __cdecl _D20StatusInitFromInternalFields(objHndl objHnd, Dispatcher* dispatcher);
void _D20StatusInitRace(objHndl objHnd);
void _D20StatusInitClass(objHndl objHnd);
void _D20StatusInit(objHndl objHnd);
void _D20StatusRefresh(objHndl objHnd);
void _D20StatusInitDomains(objHndl objHnd);
void _D20StatusInitFeats(objHndl objHnd);
void _D20StatusInitItemConditions(objHndl objHnd);
uint32_t _D20Query(objHndl objHnd, D20DispatcherKey dispKey);
void _d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, int32_t arg1, int32_t arg2);
void __cdecl D20SpellDataSetSpontCast(D20SpellData*, SpontCastType spontCastType);
void D20SpellDataExtractInfo
(D20SpellData * d20SpellData, uint32_t * spellEnum, uint32_t * spellEnumOriginal, uint32_t * spellClassCode, uint32_t * spellSlotLevel, uint32_t * itemSpellData, uint32_t * metaMagicData);
void _D20ActnInitUsercallWrapper(objHndl objHnd);
void _d20ActnSetSpellData(D20SpellData * d20SpellData, uint32_t spellEnumOrg, uint32_t spellClassCode, uint32_t spellSlotLevel, uint32_t itemSpellData, uint32_t metaMagicData);
void _globD20aSetTypeAndData1(D20ActionType d20type, uint32_t data1);
uint32_t _d20QueryWithData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2);
uint64_t _d20QueryReturnData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2);
void _globD20aSetPerformer(objHndl objHnd);
void _GlobD20ActnInit();
void _GlobD20ActnSetSpellData(D20SpellData * d20SpellData);
uint32_t _CanLevelup(objHndl objHnd);
int __cdecl _PerformStandardAttack(D20Actn * d20a);
objHndl _GetAttackWeapon(objHndl obj, int attackCode, D20CAF flags);
#pragma endregion 

inline int GetAttributeMod(int stat) {
	return (stat - 10) / 2;
}

uint32_t _d20actionTabLineParser(TabFileStatus*, uint32_t n, const char** strings);

uint32_t _DivineMightCheck(D20Actn* d20a, TurnBasedStatus* tbStat);
uint32_t _DivineMightPerform(D20Actn* d20a);

uint32_t _ActionCheckDisarm(D20Actn* d20a, TurnBasedStatus* tbStat);
uint32_t _PerformDisarm(D20Actn* d20a);
uint32_t _ActionFrameDisarm(D20Actn* d20a);