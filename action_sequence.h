#pragma once
#include "stdafx.h"
#include "common.h"
#include "tig/tig_mes.h"


#define actSeqArraySize 0x20

struct LocationSys;
struct PathQuery;
struct CombatSystem;
struct D20System;
struct ActnSeq;
struct DescriptionSystem;
struct TurnBasedStatus;
struct Pathfinding;
struct CmbtIntrpts;
struct TurnBasedSys;
struct Objects;

struct ActionSequenceSystem : AddressTable
{
	
	D20System * d20;
	CombatSystem * combat;
	Pathfinding * pathfinding;
	LocationSys * location;
	TurnBasedSys * turnbased;
	Objects * object;

	// global variables and structs
	ActnSeq ** actSeqCur;
	ActnSeq * actSeqArray; // size 32
	uint32_t * actnProcState;
	MesHandle  * actionMesHandle; 
	uint32_t * seqFlag_10B3D5C0; // init to 0
	uint32_t * actnProc_10B3D5A0;
	TurnBasedStatus * tbStatus118CD3C0;

	int32_t * seqSthg_118CD3B8; // init to -1
	int32_t * seqSthg_118A0980; // init to 1
	int32_t * seqSthg_118CD570; // init to 0
	uint32_t * numSimultPerformers;
	uint32_t * simulsIdx;  //10B3D5BC
	objHndl * simultPerformerQueue;
	int turnBasedStatusTransitionMatrix[7][5]; // describes the new hourglass state when current state is i after doing an action that costs j
	void curSeqReset(objHndl objHnd);
	void ActionAddToSeq();
		uint32_t addD20AToSeq(D20Actn * d20a, ActnSeq * actSeq);
		uint32_t AddToSeqSimple(D20Actn* d20a, ActnSeq* actSeq, TurnBasedStatus* tbStat);
		int TouchAttackAddToSeq(D20Actn* d20Actn, ActnSeq* actnSeq, TurnBasedStatus* turnBasedStatus);
		int UnspecifiedAttackAddToSeqRangedMulti(ActnSeq* actnSeq, D20Actn* d20Actn, TurnBasedStatus* tbStat);
		int UnspecifiedAttackAddToSeqMeleeMulti(ActnSeq* actSeq, TurnBasedStatus* tbStat, D20Actn* d20a);
		int  UnspecifiedAttackAddToSeq(D20Actn *d20a, ActnSeq *actSeq, TurnBasedStatus *tbStat);
			void AttackAppend(ActnSeq * actSeq, D20Actn * d20a, TurnBasedStatus* tbStat, int attackCode);
	uint32_t isPerforming(objHndl objHnd);
	void IntrrptSthgsub_100939D0(D20Actn * d20a, CmbtIntrpts * str84);
	uint32_t moveSequenceParse(D20Actn * d20aIn, ActnSeq* actSeq, TurnBasedStatus *actnSthg, float distSthg, float reach, int a5);
		void releasePath(PathQueryResult*);
		void addReadiedInterrupts(ActnSeq* actSeq, CmbtIntrpts * intrpts);
		void updateDistTraversed(ActnSeq* actSeq);
	uint32_t actSeqOkToPerform();
	TurnBasedStatus* curSeqGetTurnBasedStatus();
	const char * ActionErrorString(uint32_t actnErrorCode);

	uint32_t AllocSeq(objHndl objHnd);
	uint32_t AssignSeq(objHndl objHnd);
	uint32_t TurnBasedStatusInit(objHndl objHnd);
	void ActSeqCurSetSpellPacket(SpellPacketBody* spellPacketBody, int flag);
	int GetNewHourglassState(objHndl performer, D20ActionType d20ActionType, int d20Data1, int radMenuActualArg, D20SpellData* d20SpellData);
	int GetHourglassTransition(int hourglassCurrent, int hourglassCost);
	int (__cdecl *sub_1008B9A0)(D20Actn *d20a, float float1, PathQuery *pathQ);
	void sub_1008BB40(ActnSeq*actSeq, D20Actn * d20a); // actSeq@<ebx>
	int(CrossBowSthgReload_1008E8A0)(D20Actn *d20a, ActnSeq*actSeq); //, ActnSeq *actSeq@<ebx>
	uint32_t SequencePathSthgSub_10096450(ActnSeq * actSeq, uint32_t idx, TurnBasedStatus* tbStat);
	//10097C20
	
	uint32_t seqCheckFuncs(TurnBasedStatus *tbStatus);
	void AOOSthgSub_10097D50(objHndl, objHndl);
	int32_t AOOSthg2_100981C0(objHndl);
	int32_t InterruptSthg_10099320(D20Actn *d20a);
	int32_t InterruptSthg_10099360(D20Actn *d20a);
	uint32_t combatTriggerSthg(ActnSeq* actSeq);

	unsigned seqCheckAction(D20Actn* d20a, TurnBasedStatus* iO);
	uint32_t curSeqNext();
	void actionPerform();
	void sequencePerform();
	bool projectileCheckBeforeNextAction();
	uint32_t actSeqSpellHarmful(ActnSeq* actSeq);
	uint32_t isSimultPerformer(objHndl);
	uint32_t simulsOk(ActnSeq* actSeq);
	uint32_t simulsAbort(objHndl);
	uint32_t isSomeoneAlreadyActingSimult(objHndl objHnd);

	uint32_t ActionCostNull(D20Actn* d20Actn, TurnBasedStatus* turnBasedStatus, ActionCostPacket* actionCostPacket);
	int (__cdecl *ActionCostReload)(D20Actn *d20, TurnBasedStatus *tbStat, ActionCostPacket *acp); 
	int ActionCostFullAttack(D20Actn *d20, TurnBasedStatus *tbStat, ActionCostPacket *acp);
	void FullAttackCostCalculate(D20Actn *d20a, TurnBasedStatus *tbStatus, int *baseAttackNumCode, int *bonusAttacks, int *numAttacks, int *attackModeCode);
	int ActionCostProcess(TurnBasedStatus* tbStat, D20Actn* d20a);
	uint32_t TurnBasedStatusUpdate(D20Actn* d20a, TurnBasedStatus* tbStat);
		int TurnBasedStatusUpdate(TurnBasedStatus* tbStat, D20Actn* d20a);


	ActionSequenceSystem();
private:
	bool (__cdecl *_actionPerformProjectile)();
	void (__cdecl *_sub_1008BB40)(D20Actn * d20a); // ActnSeq*actSeq@<ebx>, 
	int(__cdecl* _CrossBowSthgReload_1008E8A0)(D20Actn *d20a); //, ActnSeq *actSeq@<ebx>
	uint32_t (__cdecl *getRemainingMaxMoveLength)(D20Actn *d20a, TurnBasedStatus *actnSthg, float *floatOut); // doesn't take things like having made 5 foot step into account, just a raw calculation
	int(__cdecl*_TurnBasedStatusUpdate)(D20Actn* d20Actn, TurnBasedStatus* turnBasedStatus);
	void (__cdecl *_sub_100939D0)(CmbtIntrpts* d20a); // D20Actn*@<eax>
	uint32_t (__cdecl* _sub_10096450)(ActnSeq * actSeq, uint32_t); // void * iO @<ebx>
	void(__cdecl* _AOOSthgSub_10097D50)(objHndl, objHndl);
	void(__cdecl *_actionPerformRecursion)();
	int32_t(__cdecl *_AOOSthg2_100981C0)(objHndl);
	uint32_t (__cdecl *_curSeqNext)();
	int32_t(__cdecl * _InterruptSthg_10099360)();
	int32_t(__cdecl * _InterruptSthg_10099320)();
	uint32_t (__cdecl *_actSeqSpellHarmful)(); // ActnSeq* @<ebx> 
	uint32_t(__cdecl *_combatTriggerSthg)(); // ActnSeq* @<ebx> 
	uint32_t(__cdecl * _moveSeqD20Sthg)(ActnSeq* actSeq, TurnBasedStatus *actnSthg, float a3, float reach, int a5); //, D20Actn * d20aIn @<eax>
	
};

extern ActionSequenceSystem actSeqSys;



struct TurnBasedStatus
{
	uint32_t hourglassState; // 4 - full action remaining; 2 - single action remaining; 1 - move action remaining
	int tbsFlags; // 0x40 full attack
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
	int32_t d20aCurIdx;
	ActnSeq * prevSeq;
	uint32_t field_B0C;
	uint32_t seqOccupied;
	TurnBasedStatus tbStatus;
	objHndl performer;
	LocAndOffsets performerLoc;
	objHndl targetObj;
	SpellPacketBody spellPktBody;
	D20Actn * d20Action;
	uint32_t aiSpellFlagSthg_maybe;
};
#pragma pack(pop)

struct IntrptSthg
{
	uint32_t field0;
	uint32_t field4;
	objHndl interrupter;
};

struct CmbtIntrpts
{
	IntrptSthg* intrptSthgs[32];
	int32_t numItems;
};



const uint32_t TestSizeOfActionSequence = sizeof(ActnSeq); // should be 0x1648 (5704)

uint32_t _addD20AToSeq(D20Actn* d20a, ActnSeq* actSeq);
uint32_t _AddToSeqSimple(D20Actn* d20a, ActnSeq * actSeq, TurnBasedStatus * tbStat);
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
uint32_t _ActionCostNull(D20Actn* d20a, TurnBasedStatus* tbStat, ActionCostPacket *acp);
int _GetNewHourglassState(objHndl performer, D20ActionType d20aType, int d20Data1, int radMenuActualArg, D20SpellData *d20SpellData);