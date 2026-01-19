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
#include "gamesystems/objects/gameobject.h"

enum ActionErrorCode : uint32_t;
enum D20ADF : int;
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
struct ActionCostPacket;
enum D20TargetClassification : int {
	Target0 = 0,
	D20TC_Movement = 1,
	D20TC_SingleExcSelf,
	D20TC_CastSpell,
	D20TC_SingleIncSelf,
	D20TC_CallLightning,
	D20TC_ItemInteraction, // includes: portals, container, dead critters

	D20TC_Invalid = -1
};


struct PythonActionSpec {
	D20ADF flags;
	D20TargetClassification tgtClass;
	std::string name;
	ActionCostType costType;
};

struct LegacyD20System : temple::AddressTable
{
	D20Actn * globD20Action;
	D20DispatcherKey globD20ActionKey; // used for the new Python actions
	D20ActionDef * d20Defs;
	Pathfinding * pathfinding;
	ActionSequenceSystem * actSeq;
	//D20ClassSystem * d20Class;
	D20StatusSystem * d20Status;
	TabFileStatus* d20ActionsTabFile;
	uint32_t(* d20actionTabLineParser)(TabFileStatus*, uint32_t, const char**);
	void d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, int32_t arg1, int32_t arg2);
	void d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, D20Actn* arg1, int32_t arg2);
	void d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, objHndl arg);
	void d20SendSignal(objHndl objHnd, D20DispatcherKey dispKey, int64_t arg);
	void D20SignalPython(const objHndl& handle, const std::string& queryKey, int arg1 = 0, int arg2 = 0);
	void D20SignalPython(const objHndl& handle, int queryKey, int arg1 = 0, int arg2 = 0);
	void D20SignalPython(objHndl objHnd, const std::string& queryKey, D20Actn* arg1, int32_t arg2 = 0);

	uint32_t d20Query(objHndl ObjHnd, D20DispatcherKey dispKey);
	uint32_t d20QueryWithData(objHndl ObjHnd, D20DispatcherKey dispKey, uint32_t arg1, uint32_t arg2);
	uint32_t d20QueryWithData(objHndl ObjHnd, D20DispatcherKey dispKey, objHndl argObj);
	uint32_t d20QueryWithData(objHndl ObjHnd, D20DispatcherKey dispKey, CondStruct* cond, uint32_t arg2);
	uint32_t d20QueryWithData(objHndl ObjHnd, D20DispatcherKey dispKey, D20SpellData* spellData, uint32_t arg2);
	uint32_t d20QueryWithData(objHndl ObjHnd, D20DispatcherKey dispKey, D20Actn* d20a);
	uint32_t d20QueryHasSpellCond(objHndl ObjHnd, int spellEnum);
	uint64_t d20QueryReturnData(objHndl objHnd, D20DispatcherKey dispKey, uint32_t arg1 =0 , uint32_t arg2 =0);
	uint64_t d20QueryReturnData(objHndl objHnd, D20DispatcherKey dispKey, CondStruct *arg1, uint32_t arg2);
	int D20QueryPython(const objHndl& handle, const std::string& queryKey, int arg1 = 0, int arg2 = 0);
    int D20QueryPython(const objHndl& handle, const std::string& queryKey, objHndl argObj);


	D20ADF GetActionFlags(D20ActionType d20ActionType);
	

	static bool D20QueryWithDataDefaultTrue(objHndl obj, D20DispatcherKey dispKey, const D20Actn * d20a, int arg2);

	void D20ActnInit(objHndl objHnd, D20Actn * d20a);
	void GlobD20ActnSetTypeAndData1(D20ActionType d20type, uint32_t data1);
	void globD20ActnSetPerformer(objHndl objHnd);
	ActionErrorCode GlobD20ActnSetTarget(objHndl objHnd, LocAndOffsets * loc);
	void GlobD20ActnSetD20CAF(D20CAF d20_caf); // OR's flags
	void GlobD20ActnInit();
	void d20aTriggerCombatCheck(ActnSeq* actSeq, int32_t idx);//1008AE90    ActnSeq * @<eax>
	int D20ActionTriggersAoO(D20Actn* d20a, TurnBasedStatus* tbStat);// 1008A9C0
	uint32_t CheckAooIncurRegardTumble(D20Actn*);
	void D20ActnSetSpellData(D20SpellData* d20SpellData, uint32_t spellEnumOrg, uint32_t spellClassCode, uint32_t spellSlotLevel, uint32_t itemSpellData, uint32_t metaMagicData);
	void ExtractSpellInfo(D20SpellData* d20spellData, uint32_t* spellEnum, uint32_t * spellEnumOrg, uint32_t* spellClassCode, uint32_t* spellSlotLevel, uint32_t* itemSpellData, MetaMagicData* metaMagicData);
	void GlobD20ActnSetSpellData(D20SpellData* d20SpellData);
	bool UsingSecondaryWeapon(D20Actn* d20a);
	bool UsingSecondaryWeapon(objHndl obj, int attackCode);
	void ExtractAttackNumber(objHndl obj, int attackCode, int * attackNumber, int* dualWielding); // e.g. is it a 2nd attack? (-5 penalty)
	objHndl GetAttackWeapon(objHndl obj, int attackCode, D20CAF flags);

	int TargetWithinReachOfLoc(objHndl obj, objHndl target, LocAndOffsets* loc);
	void D20ActnSetSetSpontCast(D20SpellData* d20SpellData, SpontCastType spontCastType);
	D20TargetClassification TargetClassification(D20Actn* d20A);
	int TargetCheck(D20Actn* d20a);
	
	void (__cdecl *D20StatusInitFromInternalFields)(objHndl objHnd, Dispatcher *dispatcher);
	void (__cdecl *D20ObjRegistryAppend)(objHndl ObjHnd);
	void(__cdecl * _d20aTriggerCombatCheck)(int32_t idx);//1008AE90    ActnSeq * @<eax>
	BOOL IsActionOffensive(D20ActionType actionType, objHndl obj) const;
	uint32_t * d20EditorMode;
	void(__cdecl *ToHitProc)(D20Actn *);
	uint32_t (__cdecl*_tumbleCheck)(D20Actn* d20a);
	int32_t (__cdecl *_d20aTriggersAOO)(void * iO); // d20a @<esi> // 1008A9C0
	bool SpellIsInterruptedCheck(D20Actn *d20a, int invIdx, SpellStoreData *spellData);
	int CastSpellProcessTargets(D20Actn *d20a, SpellPacketBody &spellPkt);
	ActionErrorCode CombatActionCostFromSpellCastingTime(uint32_t spellCastingTime, int & actionCostOut); // gets action cost (in the context of combat mode) based on spell casting time

	void (__cdecl *CreateRollHistory)(int idx);

	//char **ToEEd20ActionNames;
	void NewD20ActionsInit();
	void InfinityEngineBullshit();

	void GetPythonActionSpecs();
	std::string &GetPythonActionName(D20DispatcherKey key) const;
	ActionErrorCode GetPyActionCost(D20Actn * d20a, TurnBasedStatus * tbStat, ActionCostPacket* acp);

	LegacyD20System();

protected:
	std::map<int, PythonActionSpec> pyactions;
};


extern LegacyD20System d20Sys;

int _D20Init(GameSystemConf* conf);




#pragma region D20 Action and Action Sequence Structs

enum D20ADF : int {
	D20ADF_None = 0,
	D20ADF_Unk1 = 1,
	D20ADF_Unk2 = 2,
	D20ADF_Movement = 4,
	D20ADF_TargetSingleExcSelf = 8,
	D20ADF_MagicEffectTargeting = 0x10,
	D20ADF_Unk20 = 0x20,
	D20ADF_Unk40 = 0x40,
	D20ADF_QueryForAoO = 0x80, // will trigger an AoO depending on a D20 Query for Action_Triggers_AOO (returns 1 by default from the Global condition, Cast Defensively sets this to 0 for D20A_CAST_SPELL)
	D20ADF_TriggersAoO = 0x100,
	D20ADF_TargetSingleIncSelf = 0x200,
	D20ADF_TargetingBasedOnD20Data = 0x400,
	D20ADF_TriggersCombat = 0x800, // might be somewhat more general actually
	D20ADF_CallLightningTargeting = 0x1000,
	D20ADF_Unk2000 = 0x2000,
	D20ADF_Unk4000 = 0x4000,
	D20ADF_UseCursorForPicking = 0x8000, // indicates that the target should be selected with a "normal" cursor (as opposed to a picker)
	D20ADF_TargetContainer = 0x10000,
	D20ADF_SimulsCompatible = 0x20000,
	D20ADF_DrawPathByDefault = 0x40000, // will draw path even without holding ALT
	D20ADF_DoLocationCheckAtDestination = 0x80000,
	D20ADF_Breaks_Concentration = 0x100000,

	D20ADF_Python = 0x1000000
};

inline D20ADF & operator |=(D20ADF & lhs, D20ADF rhs) {
	lhs = static_cast<D20ADF>(lhs | rhs);
	return lhs;
}

struct D20Actn{
	D20ActionType d20ActType;
	int data1; // generic piece of data
	D20CAF d20Caf; // Based on D20_CAF flags
	uint32_t field_C; // unknown use
	objHndl d20APerformer;
	objHndl d20ATarget;
	LocAndOffsets destLoc; // action located (usually movement destination)
	float distTraversed; // distanced traversed by a move action
	uint32_t radialMenuActualArg; // the value chosen by radial menu toggle/slider
	int rollHistId0;
	int rollHistId1;
	int rollHistId2;
	D20SpellData d20SpellData;
	uint32_t spellId;
	uint32_t animID;
	PathQueryResult * path;

	D20Actn(){
		rollHistId1 = -1;
		rollHistId2 = -1;
		rollHistId0 = -1;
		//d20ActType = 0;
		d20APerformer = 0;
		d20ATarget = 0;
		d20Caf = D20CAF_NONE;
		distTraversed = 0;
		path = 0;
		spellId = 0;
		data1 = 0;
		//animID = -1;
	}

	D20Actn(D20ActionType type) {
		rollHistId1 = -1;
		rollHistId2 = -1;
		rollHistId0 = -1;
		d20ActType = type;
		d20APerformer = 0;
		d20ATarget = 0;
		d20Caf = D20CAF_NONE;
		distTraversed = 0;
		path = 0;
		spellId = 0;
		data1 = 0;
		//animID = -1;
	}
	BOOL ProjectileAppend(objHndl projHndl, objHndl thrownItem);
	int FilterSpellTargets(SpellPacketBody& spellPkt); // returns number of remaining targets
	D20ADF GetActionDefinitionFlags();
	bool IsMeleeHit();
	bool IsHarmful();

	// Python action section
	D20DispatcherKey GetPythonActionEnum();
	void SetPythonActionEnum(D20DispatcherKey pyActionEnum); // piggyback on distTraversed (used to be data1); todo revisit this in the future
};

const auto TestSizeOfD20Action = sizeof(D20Actn); // should be 88 (0x58)

//struct ActionCostPacket
//{
//	int hourglassCost;
//	int chargeAfterPicker; // flag I think; is only set at stuff that requires using the picker it seems
//	float moveDistCost;
//	
////	ActionCostPacket() { hourglassCost = 0; chargeAfterPicker = 0; moveDistCost = 0.0f; }
//};
////const auto TestSizeOfActionCostPacket = sizeof(ActionCostPacket); // should be 12 (0xC)

struct D20ActionDef{
	ActionErrorCode (__cdecl *addToSeqFunc)(D20Actn *d20a, ActnSeq *actSeq, TurnBasedStatus *tbStat);
	ActionErrorCode (__cdecl *turnBasedStatusCheck)(D20Actn *d20a, TurnBasedStatus *tbStat);
	ActionErrorCode (__cdecl *actionCheckFunc)(D20Actn *d20a, TurnBasedStatus *tbStat);
	uint32_t (__cdecl *tgtCheckFunc)(D20Actn *d20a, TurnBasedStatus *tbStat);
	ActionErrorCode (__cdecl *locCheckFunc)(D20Actn *d20a, TurnBasedStatus *tbStat, LocAndOffsets * locAndOff); // also seems to double as a generic check function (e.g. for move silently it checks if combat is active and nothing to do with location)
	ActionErrorCode (__cdecl *performFunc)(D20Actn *d20a);
	BOOL(__cdecl *actionFrameFunc)(D20Actn *d20a);
	BOOL (__cdecl *projectileHitFunc)(D20Actn *d20a, objHndl projectile, objHndl ammoItem);
	uint32_t pad_apparently; // only spell related actions have this as non-zero, and the callback is just return0()...
	ActionErrorCode (__cdecl *actionCost)(D20Actn *d20a, TurnBasedStatus *tbStat, ActionCostPacket *actionCostPacket);
	uint32_t (__cdecl *seqRenderFunc)(D20Actn *d20a, int flags);
	D20ADF flags;
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
	
	AiTactic();
	bool ChooseFireballLocation(LocAndOffsets& locOut);
};

struct AiTacticDef
{
	char * name;
	uint32_t(__cdecl * aiFunc)(AiTactic *);
	int (__cdecl* onInitiativeAdd)(AiTactic*); // seem to be zero for all tactics; i.e. unused, maybe Arcanum leftover
};


struct AiStrategy
{
	std::string name;
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
