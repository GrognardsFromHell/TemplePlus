#pragma once
#include "common.h"
#include "tig/tig_mes.h"


#define ACT_SEQ_ARRAY_SIZE 0x20
#define READIED_ACTION_CACHE_SIZE 0x20
#define MAX_AOO_SHADER_LOCATIONS 64

struct LocationSys;
struct PathQuery;
struct LegacyCombatSystem;
struct LegacyD20System;
struct ActnSeq;
struct TurnBasedStatus;
struct Pathfinding;
struct ReadiedActionPacket;
struct CmbtIntrpts;
struct TurnBasedSys;
struct Objects;
struct AoOPacket;

enum ActionErrorCode : uint32_t
{
	AEC_OK = 0,
	AEC_NOT_ENOUGH_TIME1,
	AEC_NOT_ENOUGH_TIME2,
	AEC_NOT_ENOUGH_TIME3,
	AEC_ALREADY_MOVED,
	AEC_TARGET_OUT_OF_RANGE,
	AEC_TARGET_TOO_CLOSE,
	AEC_TARGET_BLOCKED,
	AEC_TARGET_TOO_FAR,
	AEC_TARGET_INVALID,
	AEC_NO_LOS,
	AEC_OUT_OF_AMMO,
	AEC_NEED_MELEE_WEAPON,
	AEC_CANT_WHILE_PRONE,
	AEC_INVALID_ACTION,
	AEC_CANNOT_CAST_SPELLS,
	AEC_OUT_OF_CHARGES,
	AEC_WRONG_WEAPON_TYPE,
	AEC_CANNOT_CAST_OUT_OF_AVAILABLE_SPELLS,
	AEC_CANNOT_CAST_NOT_ENOUGH_XP,
	AEC_CANNOT_CAST_NOT_ENOUGH_GP,
	AEC_OUT_OF_COMBAT_ONLY,
	AEC_CANNOT_USE_MUST_USE_BEFORE_ATTACKING,
	AEC_NEED_A_STRAIGHT_LINE,
	AEC_NO_ACTIONS,
	AEC_NOT_IN_COMBAT,
	AEC_AREA_NOT_SAFE
};

// Allows for direct use of ActionErrorCode in format() strings
ostream &operator<<(ostream &str, ActionErrorCode id);

enum TurnBasedStatusFlags : uint32_t
{
	TBSF_NONE = 0,
	TBSF_1 = 1, // looks like no one actually sets this flag?? is checked in the context of movement sometimes
	TBSF_Movement = 2,
	TBSF_Movement2 = 4, // 5ft step movement
	TBSF_TouchAttack = 8, // denotes that you're doing a touch attack
	TBSF_AvoidAoO = 0x10, // the action doesn't provoke AoO. Used for Swift Actions and Creature Spells (spell enums 600-699)
	TBSF_HasActedThisRound = 0x20, // prevents you from dragging the portrait in the initiative row
	TBSF_FullAttack = 0x40,
	TBSF_80 = 0x80,
	TBSF_100 = 0x100,
	TBSF_SwiftActionPerformed = 0x200, // already performed swift action spell this round (e.g. Quickened spell), cannot do another
	TBSF_400 = 0x400,
	TBSF_ChangedWornItem = 0x800 // denotes that you've changed items in the inventory during combat (to prevent double-charging you); unflags this when hiding the inventory
};

enum SequenceFlags : int {
	SEQF_NONE = 0,
	SEQF_PERFORMING = 1,
	SEQF_2 = 2
};

struct ActionSequenceSystem : temple::AddressTable
{
	
	LegacyD20System * d20;
	LegacyCombatSystem * combat;
	Pathfinding * pathfinding;
	LocationSys * location;
	TurnBasedSys * turnbased;
	Objects * object;

	// global variables and structs
	ActnSeq ** actSeqCur;
	ActnSeq * actSeqArray; // size 32
	uint32_t * actnProcState;
	MesHandle  * actionMesHandle; 
	uint32_t * performingDefaultAction; // inited to 0
	uint32_t * performedDefaultAction; // inited to 0
	uint32_t * actSeqPickerActive;
	TurnBasedStatus * simulsTbStatus;

	D20TargetClassification * seqPickerTargetingType; // init to -1
	D20ActionType * seqPickerD20ActnType; // init to 1
	int32_t * seqPickerD20ActnData1; // init to 0
	uint32_t * numSimultPerformers;
	uint32_t * simulsIdx;  //10B3D5BC
	objHndl * simultPerformerQueue;
	int turnBasedStatusTransitionMatrix[7][5]; // describes the new hourglass state when current state is i after doing an action that costs j

	void curSeqReset(objHndl objHnd);
	void ResetAll(objHndl handle);
	void ActSeqSpellReset() const;
	
	// Pickers
	void ActSeqGetPicker();
	BOOL SeqPickerHasTargetingType();
	void SeqPickerTargetingReset();
	void SpellPickerCallback(const PickerResult &result, SpellPacketBody *pkt);

	void ActionTypeAutomatedSelection(objHndl handle);
	void TurnStart(objHndl obj);
	int ActionAddToSeq(); // ActionErrorCode
	
		uint32_t addD20AToSeq(D20Actn * d20a, ActnSeq * actSeq);
		ActionErrorCode AddToSeqSimple(D20Actn* d20a, ActnSeq* actSeq, TurnBasedStatus* tbStat);
		int AddToSeqWithTarget(D20Actn* d20a, ActnSeq* actSeq, TurnBasedStatus* tbStat);
		int TouchAttackAddToSeq(D20Actn* d20Actn, ActnSeq* actnSeq, TurnBasedStatus* turnBasedStatus);
		int UnspecifiedAttackAddToSeqRangedMulti(ActnSeq* actnSeq, D20Actn* d20Actn, TurnBasedStatus* tbStat);
		int UnspecifiedAttackAddToSeqMeleeMulti(ActnSeq* actSeq, TurnBasedStatus* tbStat, D20Actn* d20a);
		int  UnspecifiedAttackAddToSeq(D20Actn *d20a, ActnSeq *actSeq, TurnBasedStatus *tbStat);
			void AttackAppend(ActnSeq * actSeq, D20Actn * d20a, TurnBasedStatus* tbStat, int attackCode);

	int StdAttackTurnBasedStatusCheck(D20Actn *d20a, TurnBasedStatus *tbStat);
	uint32_t isPerforming(objHndl objHnd, ActnSeq** = nullptr);
	uint32_t MoveSequenceParse(D20Actn * d20aIn, ActnSeq* actSeq, TurnBasedStatus* tbStat, float distToTgtMin, float reach, int nonspecificMoveType);
		void releasePath(PathQueryResult*);
		void addReadiedInterrupts(ActnSeq* actSeq, CmbtIntrpts * intrpts);
		void updateDistTraversed(ActnSeq* actSeq);
	uint32_t actSeqOkToPerform();
	TurnBasedStatus* curSeqGetTurnBasedStatus();
	const char * ActionErrorString(uint32_t actnErrorCode);

	/*
	// finds available sequence slot, allocates it to obj and makes it the Current Sequence
	*/
	uint32_t AllocSeq(objHndl obj); 

	/*
	 uses the above does some extra handling
	*/
	uint32_t AssignSeq(objHndl objHnd);
	uint32_t TurnBasedStatusInit(objHndl objHnd);
	void ActSeqCurSetSpellPacket(SpellPacketBody* spellPacketBody, int flag);
	int GetNewHourglassState(objHndl performer, D20ActionType d20ActionType, int d20Data1, int radMenuActualArg, D20SpellData* d20SpellData); // used in radial menu rendering
	int GetHourglassTransition(int hourglassCurrent, int hourglassCost);

	BOOL SequenceSwitch(objHndl obj);
	void GreybarReset();


	ActionErrorCode ActionSequenceChecksRegardLoc(LocAndOffsets* loc, TurnBasedStatus * tbStatus, int d20aIdx, ActnSeq* actSeq);
	ActionErrorCode ActionSequenceChecksWithPerformerLocation();
	
	void ActionSequenceRevertPath(int d20ANum);
	bool GetPathTargetLocFromCurD20Action(LocAndOffsets* loc);
	int TrimPathToRemainingMoveLength(D20Actn *d20a, float remainingMoveLength, PathQuery *pathQ);
	/*
		cuts the path short and readies an interrupt action
	*/
	void ProcessPathForReadiedActions(D20Actn * d20a, CmbtIntrpts * interrupts);
	BOOL HasReadiedAction(objHndl d20APerformer);
	
	void ProcessPathForAoOs(objHndl obj, PathQueryResult* pqr, AoOPacket* aooPacket, float distFeet);
	/*
		splits up the movement to move -> aoo movement -> move as necessary
	*/
	void ProcessSequenceForAoOs(ActnSeq*actSeq, D20Actn * d20a); // actSeq@<ebx>
	ActionErrorCode AppendReloadAttack(ActnSeq *, D20Actn *, TurnBasedStatus *);
	uint32_t SequencePathSthgSub_10096450(ActnSeq * actSeq, int idx, TurnBasedStatus* tbStat);
	//10097C20
	
	uint32_t seqCheckFuncs(TurnBasedStatus *tbStatus);
	void DoAoo(objHndl, objHndl);
	int32_t DoAoosByAdjcentEnemies(objHndl);
	int32_t ProvokeAooFrom(objHndl provoker, objHndl enemy);
	
	bool SpellTargetsFilterInvalid(D20Actn &d20a);
	void HandleInterruptSequence();
	int32_t InterruptNonCounterspell(D20Actn *d20a);
	int32_t InterruptCounterspell(D20Actn *d20a);
	int32_t GetCurSeqD20ActionCount();
	objHndl getNextSimulsPerformer(); // returns next simultaneous action performer
	

	static int ReadyVsApproachOrWithdrawalCount();
	static void ReadyVsRemoveForObj(objHndl obj);
	static ReadiedActionPacket * ReadiedActionGetNext(ReadiedActionPacket * prevReadiedAction, D20Actn* d20a);
	void InterruptSwitchActionSequence(ReadiedActionPacket* readiedAction);

	uint32_t combatTriggerSthg(ActnSeq* actSeq);

	ActionErrorCode seqCheckAction(D20Actn* d20a, TurnBasedStatus* iO);


	
	uint32_t curSeqNext();
		int SequencePop(); // 10098010
	static bool ShouldAutoendTurn(TurnBasedStatus* tbStat);
	void ActionPerform();
	void sequencePerform();
	void ActionBroadcastAndSignalMoved();
	int ActionFrameProcess(objHndl obj);
	void PerformOnProjectileComplete(objHndl projectile, objHndl thrower);
	void PerformOnAnimComplete(objHndl obj, int animId); // runs any actions that need to be run when the animation finishes

	unsigned int ChargeAttackAddToSeq(D20Actn* d20a, ActnSeq* actSeq, TurnBasedStatus* tbStat);

	bool projectileCheckBeforeNextAction();
	uint32_t ShouldTriggerCombat(ActnSeq* actSeq);
	uint32_t isSimultPerformer(objHndl);
	uint32_t simulsOk(ActnSeq* actSeq);
	uint32_t simulsAbort(objHndl);
	uint32_t isSomeoneAlreadyActingSimult(objHndl objHnd);
	BOOL IsObjCurrentActorRegardSimuls(objHndl objHnd);
	BOOL IsSimulsCompleted();
	BOOL IsLastSimultPopped(objHndl obj); // last one that was popped, that is
	BOOL IsLastSimulsPerformer(objHndl obj);
	BOOL SimulsRestoreSeqTo(objHndl handle);
	BOOL SimulsAdvance();

	uint32_t ActionCostNull(D20Actn* d20Actn, TurnBasedStatus* turnBasedStatus, ActionCostPacket* actionCostPacket);
	ActionErrorCode ActionCostReload(D20Actn *d20, TurnBasedStatus *tbStat, ActionCostPacket *acp); 
	int ActionCostFullAttack(D20Actn *d20, TurnBasedStatus *tbStat, ActionCostPacket *acp);
	void FullAttackCostCalculate(D20Actn *d20a, TurnBasedStatus *tbStatus, int *baseAttackNumCode, int *bonusAttacks, int *numAttacks, int *attackModeCode);
	int ActionCostProcess(TurnBasedStatus* tbStat, D20Actn* d20a);
	uint32_t TurnBasedStatusUpdate(D20Actn* d20a, TurnBasedStatus* tbStat);
	int TurnBasedStatusUpdate(TurnBasedStatus* tbStat, D20Actn* d20a);

	void RegisterBardSongStoppingPythonAction(int action);
	bool IsBardSongStoppingPythonAction(int action);

	ActionSequenceSystem();
private:
	bool (__cdecl *_actionPerformProjectile)();
	void (__cdecl *_sub_1008BB40)(D20Actn * d20a); // ActnSeq*actSeq@<ebx>, 
	uint32_t GetRemainingMaxMoveLength(D20Actn *d20a, TurnBasedStatus *actnSthg, float *floatOut); // doesn't take things like having made 5 foot step into account, just a raw calculation
	int(__cdecl*_TurnBasedStatusUpdate)(D20Actn* d20Actn, TurnBasedStatus* turnBasedStatus);
	void (__cdecl *_ProcessPathForReadiedActions)(CmbtIntrpts* d20a); // D20Actn*@<eax>
	uint32_t (__cdecl* _sub_10096450)(ActnSeq * actSeq, uint32_t); // void * iO @<ebx>
	void(__cdecl* _AOOSthgSub_10097D50)(objHndl, objHndl);
	void(__cdecl *_actionPerformRecursion)();
	int32_t(__cdecl *_AOOSthg2_100981C0)(objHndl);
	uint32_t (__cdecl *_curSeqNext)();
	int32_t(__cdecl * _InterruptCounterspell)();
	int32_t(__cdecl * _InterruptNonCounterspell)();
	uint32_t (__cdecl *_actSeqSpellHarmful)(); // ActnSeq* @<ebx> 
	uint32_t(__cdecl *_combatTriggerSthg)(); // ActnSeq* @<ebx> 
	uint32_t(__cdecl * _moveSeqD20Sthg)(ActnSeq* actSeq, TurnBasedStatus *actnSthg, float a3, float reach, int a5); //, D20Actn * d20aIn @<eax>

	std::unordered_set<int> bardSongStoppingPythonActions;
	D20DispatcherKey pythonDispatcherKey = DK_NONE;
};

extern ActionSequenceSystem actSeqSys;



struct TurnBasedStatus
{
	uint32_t hourglassState; // 4 - full action remaining; 3 - partial?? used in interrupts, checked by partial charge; 2 - single action remaining; 1 - move action remaining
	int tbsFlags; // see TurnBasedStatusFlags 
	uint32_t idxSthg;
	float surplusMoveDistance; // is nonzero when you have started a move action already and haven't used it all up
	uint32_t baseAttackNumCode; // is composed of the base number of attacks (dispatch 51 or 53) + a code number: 99 for dual wielding (+1 for extra offhand attack), 999 for natural attacks
	uint32_t attackModeCode; // 0 for normal main hand, 99 for dual wielding, 999 for natural attacks
	uint32_t numBonusAttacks; // number of bonus attacks (dispatch 52)
	uint32_t numAttacks;
	uint32_t errCode;
	TurnBasedStatus()
	{
		hourglassState = 4;
		tbsFlags = 0;
		idxSthg = - 1;
		surplusMoveDistance = 0.0;
		baseAttackNumCode = 0;
		attackModeCode = 0;
		numBonusAttacks = 0;
		numAttacks = 0;
		errCode = 0;
	}
};

const uint32_t TestSizeOfActnSthg = sizeof(TurnBasedStatus); // should be 36 (0x24)

#pragma pack(push, 1)
struct ActnSeq
{
	D20Actn d20ActArray[32];
	int32_t d20ActArrayNum;
	int32_t d20aCurIdx; // inited to -1
	ActnSeq * prevSeq;
	ActnSeq * interruptSeq;
	uint32_t seqOccupied; // is actually flags; 1 - performing; 2 - aoo maybe?
	TurnBasedStatus tbStatus;
	objHndl performer;
	LocAndOffsets performerLoc;
	objHndl targetObj;
	SpellPacketBody spellPktBody;
	D20Actn * d20Action;
	uint32_t ignoreLos;
};
#pragma pack(pop)

enum ReadyVsTypeEnum : uint32_t
{
	RV_Spell =0,
	RV_Counterspell,
	RV_Approach,
	RV_Withdrawal
};

struct ReadiedActionPacket
{
	uint32_t flags;
	uint32_t field4;
	objHndl interrupter;
	ReadyVsTypeEnum readyType;
	int field14;
};

struct CmbtIntrpts
{
	ReadiedActionPacket* readyPackets[32];
	int32_t numItems;
};

struct AoOPacket
{
	objHndl obj;
	PathQueryResult * path;
	unsigned int numAoOs;
	objHndl interrupters[32];
	float aooDistFeet[32];
	LocAndOffsets aooLocs[32];
};

const uint32_t TestSizeOfActionSequence = sizeof(ActnSeq); // should be 0x1648 (5704)

uint32_t _addD20AToSeq(D20Actn* d20a, ActnSeq* actSeq);

uint32_t _StdAttackTurnBasedStatusCheck(D20Actn *d20a, TurnBasedStatus *tbStat);

unsigned _seqCheckAction(D20Actn* d20a, TurnBasedStatus* iO);
uint32_t _isPerforming(objHndl objHnd);
uint32_t _actSeqOkToPerform();
void _actionPerform();
uint32_t _isSimultPerformer(objHndl objHnd);
uint32_t _seqCheckFuncsCdecl(TurnBasedStatus *actnSthg);
uint32_t _moveSequenceParseUsercallWrapper(ActnSeq *actSeq, TurnBasedStatus *actnSthg, float distSthg, float reach, int flagSthg); //, D20_Action *d20aIn@<eax>
uint32_t _unspecifiedMoveAddToSeq(D20Actn *d20a, ActnSeq *actSeq, TurnBasedStatus *actnSthg);
int __cdecl _UnspecifiedAttackAddToSeq(D20Actn *d20a, ActnSeq *actSeq, TurnBasedStatus *tbStat);
int __cdecl _ActionCostFullAttack(D20Actn* d20, TurnBasedStatus* tbStat, ActionCostPacket* acp);
void _sequencePerform();
void _curSeqReset(objHndl objHnd);
uint32_t _allocSeq(objHndl objHnd);
uint32_t _assignSeq(objHndl objHnd);
TurnBasedStatus * _curSeqGetTurnBasedStatus();
uint32_t _turnBasedStatusInit(objHndl objHnd);
const char * _ActionErrorString(uint32_t actnErrorCode);
void __cdecl _ActSeqCurSetSpellPacket(SpellPacketBody *, int flag );
int _GetNewHourglassState(objHndl performer, D20ActionType d20aType, int d20Data1, int radMenuActualArg, D20SpellData *d20SpellData);