#pragma once
#include "stdafx.h"
#include "common.h"
#include "tig/tig_mes.h"


#define actSeqArraySize 0x20

struct CombatSystem;
struct D20System;
struct ActnSeq;
struct DescriptionSystem;
struct ActnSthg;

struct ActionSequenceSystem : AddressTable
{
	
	D20System * d20;
	CombatSystem * combat;
	ActnSeq ** actSeqCur;
	ActnSeq * actSeqArray; // size 32
	uint32_t * actnProcState;
	MesHandle  * actionMesHandle; 
	uint32_t * seqSthg_10B3D5C0;
	uint32_t * actnProc_10B3D5A0;
	ActnSthg * actnSthg118CD3C0;
	uint32_t * numSimultPerformers;
	uint32_t * simulsIdx;  //10B3D5BC
	objHndl * simultPerformerQueue; 
	uint32_t addD20AToSeq(D20Actn * d20a, ActnSeq * actSeq);
	uint32_t isPerforming(objHndl objHnd);
	uint32_t addSeqSimple(D20Actn * d20a, ActnSeq * actSeq);
	uint32_t actSeqOkToPerform();

	uint32_t sub_10093950(D20Actn* d20a, void* iO);
	uint32_t sub_10096450(ActnSeq * actSeq, uint32_t idx, void* iO);
	void AOOSthgSub_10097D50(objHndl, objHndl);
	int32_t AOOSthg2_100981C0(objHndl);
	int32_t InterruptSthg_10099320(D20Actn *d20a);
	int32_t InterruptSthg_10099360(D20Actn *d20a);
	uint32_t combatTriggerSthg(ActnSeq* actSeq);

	uint32_t seqCheckAction(D20Actn * d20a, void * iO);
	uint32_t curSeqNext();
	void actionPerform();
	void actionPerformRecursion();
	bool projectileCheckBeforeNextAction();
	uint32_t actSeqSpellHarmful(ActnSeq* actSeq);
	uint32_t isSimultPerformer(objHndl);
	uint32_t simulsOk(ActnSeq* actSeq);
	uint32_t simulsAbort(objHndl);
	uint32_t isSomeoneAlreadyActingSimult(objHndl objHnd);


	ActionSequenceSystem();
private:
	bool (__cdecl *_actionPerformProjectile)();
	uint32_t (__cdecl *_sub_10093950)(D20Actn* d20a);
	uint32_t (__cdecl* _sub_10096450)(ActnSeq * actSeq, uint32_t); // void * iO @<ebx>
	void(__cdecl* _AOOSthgSub_10097D50)(objHndl, objHndl);
	void(__cdecl *_actionPerformRecursion)();
	int32_t(__cdecl *_AOOSthg2_100981C0)(objHndl);
	uint32_t (__cdecl *_curSeqNext)();
	int32_t(__cdecl * _InterruptSthg_10099360)();
	int32_t(__cdecl * _InterruptSthg_10099320)();
	uint32_t (__cdecl *_actSeqSpellHarmful)(); // ActnSeq* @<ebx> 
	uint32_t(__cdecl *_combatTriggerSthg)(); // ActnSeq* @<ebx> 
	
};

extern ActionSequenceSystem actSeqSys;



struct ActnSthg
{
	uint32_t field_B14;
	D20CAF callActionFrameFlags;
	uint32_t idxSthg;
	uint32_t field_B20;
	uint32_t field_B24;
	uint32_t field_B28;
	uint32_t field_B2C;
	uint32_t field_B30;
	uint32_t field_B34__errCodeApparently;
};

const uint32_t TestSizeOfActnSthg = sizeof(ActnSthg); // should be 36 (0x24)

struct ActnSeq
{
	D20Actn d20ActArray[32];
	int32_t d20ActArrayNum;
	int32_t d20aCurIdx;
	ActnSeq * prevSeq;
	uint32_t field_B0C;
	uint32_t seqOccupied;
	ActnSthg actnSthgField;
	objHndl performer;
	LocAndOffsets locAndOff;
	objHndl unknown_maybeInteruptee;
	SpellPacketBody spellPktBody;
	D20Actn * d20Action;
	uint32_t field_1644_maybe_spellAssignedFlag;
};

const uint32_t TestSizeOfD20Action= sizeof(D20Actn); // should be 0x58 (88)

const uint32_t TestSizeOfActionSequence = sizeof(ActnSeq); // should be 0x1648 (5704)

uint32_t _addD20AToSeq(D20Actn* d20a, ActnSeq* actSeq);
uint32_t _addSeqSimple(D20Actn* d20a, ActnSeq * actSeq);
uint32_t _seqCheckAction(D20Actn * d20a, void * iO);
uint32_t _isPerforming(objHndl objHnd);
uint32_t _actSeqOkToPerform();
void _actionPerform();
uint32_t _isSimultPerformer(objHndl objHnd);