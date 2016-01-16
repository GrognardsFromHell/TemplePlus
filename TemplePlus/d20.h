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
struct D20SpellData;
enum D20CAF : uint32_t;
//enum SpontCastType : unsigned char;
struct D20SpellData;
struct D20Actn;
struct D20ActionDef;
struct ActnSeq;
struct PathQueryResult;
struct Pathfinding;
enum D20TargetClassification : int {
	Target0 = 0,
	D20TC_Movement = 1,
	D20TC_SingleExcSelf,
	D20TC_CastSpell,
	D20TC_SingleIncSelf,
	D20TC_CallLightning,
	D20TC_ItemInteraction // includes: portals, container, dead critters
};


struct D20System : temple::AddressTable
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
	uint32_t d20QueryWithData(objHndl ObjHnd, D20DispatcherKey dispKey, objHndl argObj);
	uint32_t d20QueryHasSpellCond(objHndl ObjHnd, int spellEnum);
	uint64_t d20QueryReturnData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2);

	void D20ActnInit(objHndl objHnd, D20Actn * d20a);
	void GlobD20ActnSetTypeAndData1(D20ActionType d20type, uint32_t data1);
	void globD20ActnSetPerformer(objHndl objHnd);
	int GlobD20ActnSetTarget(objHndl objHnd, LocAndOffsets * loc);
	void GlobD20ActnInit();
	void d20aTriggerCombatCheck(ActnSeq* actSeq, int32_t idx);//1008AE90    ActnSeq * @<eax>
	int D20ActionTriggersAoO(D20Actn* d20a, TurnBasedStatus* tbStat);// 1008A9C0
	uint32_t tumbleCheck(D20Actn*);
	void D20ActnSetSpellData(D20SpellData* d20SpellData, uint32_t spellEnumOrg, uint32_t spellClassCode, uint32_t spellSlotLevel, uint32_t itemSpellData, uint32_t metaMagicData);
	void GlobD20ActnSetSpellData(D20SpellData* d20SpellData);
	bool UsingSecondaryWeapon(D20Actn* d20a);
	bool UsingSecondaryWeapon(objHndl obj, int attackCode);
	void ExtractAttackNumber(objHndl obj, int attackCode, int * attackNumber, int* dualWielding); // e.g. is it a 2nd attack? (-5 penalty)
	objHndl GetAttackWeapon(objHndl obj, int attackCode, D20CAF flags);
	int PerformStandardAttack(D20Actn* d20a);
	int TargetWithinReachOfLoc(objHndl obj, objHndl target, LocAndOffsets* loc);
	void D20ActnSetSetSpontCast(D20SpellData* d20SpellData, SpontCastType spontCastType);
	D20TargetClassification TargetClassification(D20Actn* d20A);
	int TargetCheck(D20Actn* d20a);
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
	int rollHist3;
	int rollHist1;
	int rollHist2;
	D20SpellData d20SpellData;
	uint32_t spellId;
	uint32_t animID;
	PathQueryResult * path;

	D20Actn()
	{
		rollHist1 = -1;
		rollHist2 = -1;
		rollHist3 = -1;
		//d20ActType = 0;
		d20APerformer = 0;
		d20ATarget = 0;
		d20Caf = 0;
		distTraversed = 0;
		path = 0;
		spellId = 0;
		data1 = 0;
		//animID = -1;
	}

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
		//animID = -1;
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


enum D20ADF : int{
	D20ADF_None = 0,
	D20ADF_Unk1 = 1,
	D20ADF_Unk2 = 2,
	D20ADF_Movement = 4,
	D20ADF_TargetSingleExcSelf = 8,
	D20ADF_MagicEffectTargeting = 0x10,
	D20ADF_Unk20 = 0x20,
	D20ADF_Unk40 = 0x40,
	D20ADF_QueryForAoO = 0x80, // will trigger an AoO depending on a D20 Query for Action_Triggers_AOO
	D20ADF_TriggersAoO = 0x100,
	D20ADF_TargetSingleIncSelf = 0x200,
	D20ADF_TargetingBasedOnD20Data = 0x400,
	D20ADF_TriggersCombat = 0x800, // might be somewhat more general actually
	D20ADF_CallLightningTargeting = 0x1000,
	D20ADF_Unk2000 = 0x2000,
	D20ADF_Unk4000 = 0x4000,
	D20ADF_Unk8000 = 0x8000, // perhaps indicates a followup action, or to record what the last action was
	D20ADF_TargetContainer = 0x10000,
	D20ADF_SimulsCompatible = 0x20000,
	D20ADF_DrawPathByDefault = 0x40000, // will draw path even without holding ALT
	D20ADF_PathSthg =   0x80000,
	D20ADF_Unk100000 = 0x100000
};

struct D20ActionDef
{
	uint32_t (__cdecl *addToSeqFunc)(D20Actn *, ActnSeq *, TurnBasedStatus*iO);
	uint32_t (__cdecl* turnBasedStatusCheck)(D20Actn* d20a, TurnBasedStatus* iO);
	uint32_t (__cdecl * actionCheckFunc)(D20Actn* d20a, TurnBasedStatus* iO);
	uint32_t (__cdecl * tgtCheckFunc)(D20Actn* d20a, TurnBasedStatus* iO);
	uint32_t (__cdecl * locCheckFunc)(D20Actn* d20a, TurnBasedStatus* iO, LocAndOffsets * locAndOff); // also seems to double as a generic check function (e.g. for move silently it checks if combat is active and nothing to do with location)
	uint32_t (__cdecl * performFunc)(D20Actn* d20a);
	uint32_t (__cdecl * actionFrameFunc)(D20Actn* d20a);
	void * projectilePerformFunc;
	uint32_t pad_apparently;
	uint32_t(__cdecl * actionCost)(D20Actn* d20a, TurnBasedStatus* iO, ActionCostPacket * actionCostPacket);
	uint32_t (__cdecl * pickerFuncMaybe)(D20Actn* d20a, int flags);
	D20ADF flags; // not D20CAF I think; maybe the STD flags? path query flags?
};




struct AiTacticDef;

struct AiTactic 
{
	AiTacticDef * aiTac;
	uint32_t field4;
	objHndl performer;
	objHndl target;
	int32_t tacIdx;
	D20SpellData d20SpellData;
	uint32_t field24;
	SpellPacketBody spellPktBody;
};

struct AiTacticDef
{
	char * name;
	uint32_t(__cdecl * aiFunc)(AiTactic *);
	int (__cdecl* onInitiativeAdd)(AiTactic*); // seem to be zero for all tactics; i.e. unused, maybe Arcanum leftover
};


struct AiStrategy
{
	char * name;
	AiTacticDef * aiTacDefs[20];
	uint32_t field54[20];
	SpellStoreData spellsKnown[20];
	uint32_t numTactics;
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

uint32_t _ActionCheckDisarmedWeaponRetrieve(D20Actn* d20a, TurnBasedStatus* tbStat);
uint32_t LocationCheckDisarmedWeaponRetrieve(D20Actn* d20a, TurnBasedStatus* tbStat, LocAndOffsets* loc);
uint32_t _PerformDisarmedWeaponRetrieve(D20Actn* d20a);

uint32_t _ActionCheckSunder(D20Actn* d20a, TurnBasedStatus* tbStat);
uint32_t _ActionFrameSunder(D20Actn* d20a);

uint32_t _PerformAidAnotherWakeUp(D20Actn* d20a);
uint32_t _ActionFrameAidAnotherWakeUp(D20Actn* d20a);
uint32_t _ActionCheckAidAnotherWakeUp(D20Actn* d20a, TurnBasedStatus* tbStat);