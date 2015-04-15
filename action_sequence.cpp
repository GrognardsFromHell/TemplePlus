#include "stdafx.h"
#include "common.h"
#include "d20.h"
#include "action_sequence.h"
#include "obj.h"
#include "temple_functions.h"
#include "tig\tig_mes.h"
#include "float_line.h"
#include "combat.h"
#include "description.h"
#include "location.h"
#include "pathfinding.h"


class ActnSeqReplacements : public TempleFix
{
public:
	const char* name() override {
		return "ActionSequence Replacements";
	}
	void apply() override{
		
		macReplaceFun(1008BFA0, _addSeqSimple)
			macReplaceFun(1008A100, _addD20AToSeq)
			macReplaceFun(10094C60, _seqCheckAction)
			macReplaceFun(1008A050, _isPerforming)
			macReplaceFun(1008A980, _actSeqOkToPerform)
			macReplaceFun(100925E0, _isSimultPerformer)
			//macReplaceFun(10094CA0, _seqCheckFuncs) 
			/*
		macReplaceFun(100996E0, _actionPerform)
		
		
		//macReplaceFun(10094F70, _moveSeqD20SthgUsercallWrapper)
		//macReplaceFun(10095860, _unspecifiedMoveAddToSeq)
		*/
	}
} actSeqReplacements;


#pragma region Action Sequence System Implementation

ActionSequenceSystem actSeqSys;

ActionSequenceSystem::ActionSequenceSystem()
{
	d20 = &d20sys;
	combat = &combatSys;
	pathfinding = &pathfindingSys;
	rebase(actSeqCur, 0x1186A8F0);
	macRebase(actnSthg118CD3C0, 118CD3C0)
	rebase(actnProcState, 0x10B3D5A4);
	macRebase(_sub_1008BB40, 1008BB40)
	macRebase(sub_1008B8A0, 1008B8A0);
	macRebase(sub_1008B9A0, 1008B9A0);
	rebase(_sub_10093950, 0x10093950);
	rebase(_sub_10096450, 0x10096450);
	rebase(actionMesHandle, 0x10B3BF48);
	rebase(seqSthg_10B3D5C0, 0x10B3D5C0);
	rebase(actnProc_10B3D5A0, 0x10B3D5A0);
	rebase(actSeqArray, 0x118A09A0);
	rebase(_actionPerformProjectile, 0x1008AC70);
	rebase(_AOOSthgSub_10097D50, 0x10097D50);
	rebase(_actionPerformRecursion, 0x100961C0);
	rebase(_AOOSthg2_100981C0, 0x100981C0);
	rebase(_curSeqNext, 0x10098ED0);
	rebase(_InterruptSthg_10099320, 0x10099320);
	rebase(_InterruptSthg_10099360, 0x10099360);
	rebase(_actSeqSpellHarmful, 0x1008AD10);
	rebase(_combatTriggerSthg, 0x10093890);
	rebase(numSimultPerformers, 0x10B3D5B8);
	rebase(simultPerformerQueue, 0x118A06C0);
	macRebase(simulsIdx, 10B3D5BC)
	macRebase(_moveSeqD20Sthg, 10094F70)
	macRebase(_sub_100939D0, 100939D0)
	
}


uint32_t ActionSequenceSystem::addD20AToSeq(D20Actn* d20a, ActnSeq* actSeq)
{
	D20ActionType d20aType = d20a->d20ActType;
	if (d20aType == D20A_NONE){ return 0xE; }

	*actnProcState =  d20->d20Defs[d20aType].addToSeqFunc(d20a, actSeq, &actSeq->actnSthgField);

	ActnSeq * curSeq = *actSeqCur;
	int32_t d20aIdx = curSeq->d20aCurIdx + 1;
	if (d20aIdx < curSeq->d20ActArrayNum)
	{
		do
		{
			uint32_t caflags = curSeq->d20ActArray[d20aIdx].d20Caf;
			if (caflags & D20CAF_ATTACK_OF_OPPORTUNITY)
			{
				caflags |= (uint32_t)D20CAF_FULL_ATTACK;
				curSeq->d20ActArray[d20aIdx].d20Caf = (D20CAF)caflags;
			}
			++d20aIdx;
		} while (d20aIdx < curSeq->d20ActArrayNum);
	}
	return *actnProcState;
}

uint32_t ActionSequenceSystem::isPerforming(objHndl objHnd)
{
	for (auto i = 0; i < actSeqArraySize; i++)
	{
		if (actSeqArray[i].performer == objHnd && (actSeqArray[i].seqOccupied & 1))
		{
			return 1;
		}
	}
	return 0;
}

uint32_t ActionSequenceSystem::addSeqSimple(D20Actn* d20a, ActnSeq * actSeq)
{
	memcpy(actSeq + sizeof(D20Actn) * actSeq->d20ActArrayNum++, d20a, sizeof(D20Actn));
	return 0;
}

void ActionSequenceSystem::IntrrptSthgsub_100939D0(D20Actn* d20a, CmbtIntrpts* str84)
{
	macAsmProl;
	__asm{
		mov ecx, this;
		mov esi, [ecx]._sub_100939D0;
		mov eax, str84;
		push eax;
		mov eax, d20a;
		call esi;
		add esp, 4;
	}
	macAsmEpil;
}

uint32_t ActionSequenceSystem::moveSeqD20Sthg(D20Actn* d20aIn, ActnSeq* actSeq, ActnSthg* actnSthg, float distToTgt, float reach, int flagSthg)
{
	D20Actn d20aCopy;
	ActnSthg actnSthgCopy;
	PathQuery pathQ;
	PathQueryResult * pqResult = nullptr;
	LocAndOffsets locAndOffCopy = d20aIn->locAndOff;

	memcpy(&d20aCopy, d20aIn, sizeof(D20Actn));
	memcpy(&actnSthgCopy, actnSthg, sizeof(ActnSthg));

	seqCheckFuncs(&actnSthgCopy);
	if (d20->d20Query(d20aCopy.d20APerformer, DK_QUE_Prone))
	{
		auto nextd20a = &actSeq->d20ActArray[actSeq->d20ActArrayNum];
		d20aCopy.d20ActType = D20A_STAND_UP;
		memcpy(nextd20a, &d20aCopy, sizeof(D20Actn));
		++actSeq->d20ActArrayNum; 
	}

	auto d20a = d20aIn;

	pathQ.critter = d20aCopy.d20APerformer;
	pathQ.from = actSeq->performerLoc;


	if (d20a->d20ATarget)
	{
		const float twelve = 12.0;
		const float fourPointSevenPlusEight = (float)4.714045 + 8.0;
		pathQ.targetObj = d20a->d20ATarget;
		pathQ.flags = (PathQueryFlags)0x23803;
		if (reach < 0.1){ reach = 3.0; }
		actSeq->targetObj = d20a->d20ATarget;
		pathQ.distanceToTarget = distToTgt * twelve;
		pathQ.radius = reach * twelve - fourPointSevenPlusEight;
	} else
	{
		pathQ.to = d20aIn->locAndOff;
		pathQ.flags = static_cast<PathQueryFlags>(0x40803);
	}

	if (d20a->d20Caf & D20CAF_TRUNCATED)
	{
		auto asdf = &pathQ.flags;
		*((int*)asdf) |= 0x80000;
	}

	if (d20aCopy.path && d20aCopy.path >= pathfindingSys.pathQArray && d20aCopy.path < &pathfindingSys.pathQArray[pfCacheSize]) d20aCopy.path->occupiedFlag = 0; // frees the last path used in the d20a

	for (int i = 0; i < pfCacheSize; i++)
	{
		if (pathfindingSys.pathQArray[i].occupiedFlag == 0)
		{
			pathfindingSys.pathQArray[i].occupiedFlag = 1;
			pqResult = &pathfindingSys.pathQArray[i];
			break;
		}
	}
	if (!pqResult) return 0x9;
	d20aCopy.path = pqResult;
	*pathfindingSys.pathSthgFlag_10B3D5C8 = 0;
	if (! pathfinding->FindPath(&pathQ, pqResult))
	{
		if (pqResult->flags & 0x10) *pathfindingSys.pathSthgFlag_10B3D5C8 = 1;
		hooked_print_debug_message("\nFAILED PATH...");
		if (pqResult >= pathfindingSys.pathQArray && pqResult < &pathfindingSys.pathQArray[pfCacheSize]) pqResult->occupiedFlag = 0;
		return 0x9;
	}
	
	auto distTraversed = pathfinding->pathDistBtwnToAndFrom(pqResult);
	d20aCopy.locAndOff = pqResult->to;
	d20aCopy.distTraversed = distTraversed;

	if (distTraversed < 0.1) return 0;
	if (!combat->isCombatActive()){	d20aCopy.distTraversed = 0;		distTraversed = 0.0;	}

	float dist2 = 0;
	if (sub_1008B8A0(d20a, &actnSthgCopy, &dist2))
	{
		if (dist2 < 0.1)	{releasePath(d20aCopy.path);	return 0x8;	}
		if (static_cast<long double>(dist2) < distTraversed)
		{
			auto temp = 1;;
			if (sub_1008B9A0(&d20aCopy, dist2, &pathQ)){ releasePath(d20aCopy.path); return temp; }
			pqResult = d20aCopy.path;
			distTraversed = dist2;
		}
	}
	if (flagSthg)
	{
		dist2 = dispatch.Dispatch29hMovementSthg(d20aCopy.d20APerformer, nullptr);
		if (!(d20aCopy.d20Caf & D20CAF_RANGED))
		{
			if (distTraversed > (long double)actnSthgCopy.floaSthg)
			{
				if (dist2 + dist2 + actnSthgCopy.floaSthg < (long double)distTraversed){ releasePath(pqResult); return 8; }
				if (actnSthgCopy.floaSthg + dist2 < (long double)distTraversed){ d20aCopy.d20ActType = D20A_DOUBLE_MOVE; goto LABEL_53; }
				else if (distTraversed <= 5.0)
				{
					if (d20a->d20ActType)
					{
						d20->d20Defs[d20a->d20ActType].moveFunc(d20a, &actnSthgCopy, &locAndOffCopy);
						if (locAndOffCopy.location.locx == 4 || !actnSthgCopy.field_B14) //BUG? TODO
						{
							d20aCopy.d20ActType = D20A_5FOOTSTEP; goto LABEL_53;
						}
					} else if (!actnSthgCopy.field_B14)
					{
						d20aCopy.d20ActType = D20A_5FOOTSTEP;
						if (!(actnSthgCopy.callActionFrameFlags & 6)){	goto LABEL_53; }
					}
				}

			}
			d20aCopy.d20ActType = D20A_MOVE;
			goto LABEL_53;
		} 
		else if (dist2 + dist2 >= (long double)distTraversed){ d20aCopy.d20ActType = D20A_RUN;	goto LABEL_53; }
		else	{	releasePath(pqResult); return 8;	}
	}

LABEL_53: actSeq->performerLoc = pqResult->to;
	CmbtIntrpts str84;
	str84.numItems = 0;
	IntrrptSthgsub_100939D0(&d20aCopy, &str84);
	sub_1008BB40(actSeq, &d20aCopy);
	updateDistTraversed(actSeq);
	return 0;
}

void ActionSequenceSystem::releasePath(PathQueryResult* pqr)
{
	if (pqr)
	{
		if (pqr >= pathfinding->pathQArray && pqr < &pathfinding->pathQArray[pfCacheSize])
		{
			pqr->occupiedFlag = 0;
		}
	}
}

void ActionSequenceSystem::addReadiedInterrupts(ActnSeq* actSeq, CmbtIntrpts* intrpts)
{
	D20Actn d20aNew;
	int32_t intrptNum = intrpts->numItems;
	if (intrptNum <= 0) return;
	
	d20aNew.d20ATarget = actSeq->performer;
	d20aNew.d20Caf = (D20CAF)0;
	d20aNew.d20ActType = D20A_READIED_INTERRUPT;
	d20aNew.data1 = 1;
	d20aNew.path = nullptr;
	IntrptSthg * intrptSthg ;
	for (int i = 0; i < intrptNum; i++)
	{
		intrptSthg = intrpts->field0[i];
		d20aNew.d20APerformer = intrptSthg->interrupter;
		memcpy(&(actSeq->d20ActArray[actSeq->d20ActArrayNum]), &d20aNew, sizeof(d20aNew));
		++actSeq->d20ActArrayNum;
	}
}

void ActionSequenceSystem::updateDistTraversed(ActnSeq* actSeq)
{
	PathQueryResult * path;
	int numd20s = actSeq->d20ActArrayNum;
	for (int i = 0; i < numd20s; i++)
	{
		path = actSeq->d20ActArray[i].path;
		if (path){	actSeq->d20ActArray[i].distTraversed = pathfinding->pathDistBtwnToAndFrom(path);	}
	}
}

uint32_t ActionSequenceSystem::actSeqOkToPerform()
{
	ActnSeq * curSeq = *actSeqCur;
	if (curSeq->seqOccupied & 1 && (curSeq->d20aCurIdx >= 0) && curSeq->d20aCurIdx < (int32_t)curSeq->d20ActArrayNum )
	{
		auto caflags = curSeq->d20ActArray[curSeq->d20aCurIdx].d20Caf;
		if (caflags & D20CAF_NEED_PROJECTILE_HIT){ return 0; }
		return (caflags & D20CAF_NEED_ANIM_COMPLETED) == 0;
	}
	return 1;
}

void ActionSequenceSystem::sub_1008BB40(ActnSeq* actSeq, D20Actn* d20a)
{
	macAsmProl;
	__asm{
		mov ecx, this;
		mov esi, [ecx]._sub_1008BB40;
		mov eax, d20a;
		push eax;
		mov ebx, actSeq;
		call esi;
		add esp, 4;
	}
	macAsmEpil;
}

unsigned ActionSequenceSystem::sub_10093950(D20Actn* d20a, ActnSthg* actnSthg)
{
	uint32_t result = 0;
	__asm{
		push esi;
		push ecx;
		push ebx;
		mov ecx, this;
		mov esi, actnSthg;
		push esi;
		mov esi, [ecx]._sub_10093950;
		mov ebx, d20a;
		call esi;
		add esp, 4;

		mov result, eax;
		pop ebx;
		pop ecx;
		pop esi;
	}
	return result;
}

uint32_t ActionSequenceSystem::sub_10096450(ActnSeq* actSeq, uint32_t idx ,void * iO)
{
	uint32_t result = 0;
	__asm{
		push esi;
		push ecx;
		push ebx;
		mov ecx, this;


		mov esi, idx;
		push esi;
		mov esi, [ecx]._sub_10096450;
		mov ebx, iO;
		mov eax, actSeq;
		push eax;
		call esi;
		add esp, 8;

		mov result, eax;
		pop ebx;
		pop ecx;
		pop esi;
	}
	return result;
}

uint32_t ActionSequenceSystem::seqCheckFuncs(ActnSthg* actnSthg)
{
	LocAndOffsets seqPerfLoc;
	ActnSeq * curSeq = *actSeqCur;
	uint32_t result = 0;
	
	if (!curSeq){	memset(actnSthg, 0, sizeof(ActnSthg));		return 0;	}

	memcpy(actnSthg, &curSeq->actnSthgField, sizeof(ActnSthg));
	objects.loc->getLocAndOff(curSeq->performer, &seqPerfLoc);
	for (int32_t i = 0; i < curSeq->d20ActArrayNum; i++)
	{
		auto d20type = curSeq->d20ActArray[i].d20ActType;

		auto tgtCheckFunc = d20->d20Defs[d20type].tgtCheckFunc;
		if (tgtCheckFunc){	result = tgtCheckFunc(&curSeq->d20ActArray[i], actnSthg);	if (result) break;		}
		
		auto d20a = &curSeq->d20ActArray[i];
		result = sub_10093950(d20a, actnSthg);
		if (result)	{ actnSthg->field_B34__errCodeApparently = result;	break;	} 

		auto actCheckFunc = d20->d20Defs[d20a->d20ActType].actionCheckFunc;
		if (actCheckFunc)	{	result = actCheckFunc(d20a, actnSthg);		if (result) break;		}

		auto locCheckFunc = d20->d20Defs[d20type].locCheckFunc;
		if (locCheckFunc){	result = locCheckFunc(d20a, actnSthg, &seqPerfLoc); if (result) break;	}

		auto path = curSeq->d20ActArray[i].path;
		if (path)
		{
			seqPerfLoc.location.locx = path->to.location.locx;
			seqPerfLoc.location.locy = path->to.location.locy;
			seqPerfLoc.off_x = path->to.off_x;
			seqPerfLoc.off_y = path->to.off_y;
		}
	}
	if (!*actSeqCur){ memset(actnSthg, 0, sizeof(ActnSthg)); }
	memcpy(actnSthg, &(*actSeqCur)->actnSthgField, sizeof(ActnSthg));
	return result;

}

void ActionSequenceSystem::AOOSthgSub_10097D50(objHndl objHnd1, objHndl objHnd2)
{
	_AOOSthgSub_10097D50(objHnd1, objHnd2);
}

int32_t ActionSequenceSystem::AOOSthg2_100981C0(objHndl objHnd)
{
	return _AOOSthg2_100981C0(objHnd);
}

int32_t ActionSequenceSystem::InterruptSthg_10099320(D20Actn* d20a)
{
	uint32_t result = 0;
	__asm{
		push esi;
		push ecx;
		mov ecx, this;
		mov esi, [ecx]._InterruptSthg_10099320;
		mov eax, d20a;
		call esi;
		mov result, eax;
		pop ecx;
		pop esi;
	}
	return result;
}

int32_t ActionSequenceSystem::InterruptSthg_10099360(D20Actn* d20a)
{
	uint32_t result = 0;
	__asm{
		push esi;
		push ecx;
		push ebx;
		mov ecx, this;
		mov esi, [ecx]._InterruptSthg_10099360;
		mov ebx, d20a;
		call esi;
		mov result, eax;
		pop ebx;
		pop ecx;
		pop esi;
	}
	return result;

}

uint32_t ActionSequenceSystem::combatTriggerSthg(ActnSeq* actSeq)
{
	uint32_t result;
	macAsmProl;
	__asm{
		mov ecx, this;
		mov esi, [ecx]._combatTriggerSthg;
		mov ebx, actSeq;
		call esi;
		mov result, eax;
	}
	macAsmEpil;
	
	return result;
}

uint32_t ActionSequenceSystem::seqCheckAction(D20Actn* d20a, ActnSthg* iO)
{
	uint32_t a = sub_10093950(d20a, iO);
	if (a)
	{
		*((uint32_t*)iO + 8) = a;
		return a;
	}
	
	auto checkFunc = d20->d20Defs[ d20a->d20ActType].actionCheckFunc;
	if (checkFunc)
	{
		return checkFunc(d20a, iO);
	}
	return 0;
}

uint32_t ActionSequenceSystem::curSeqNext()
{
	ActnSeq* curSeq = *actSeqCur;
	curSeq->seqOccupied &= 0xffffFFFE; //unset "occupied" flag
	hooked_print_debug_message("\nSequence Completed for %s (%I64x) (sequence %x)",description.GetDisplayName(curSeq->performer, curSeq->performer), curSeq->performer, curSeq);

	return _curSeqNext();
}

void ActionSequenceSystem::actionPerform()
{
	MesLine mesLine;
	ActnSthg var_24;
	int32_t * curIdx ;
	D20Actn * d20a = nullptr;
	while (1)
	{
		ActnSeq * curSeq = *actSeqCur;
		curIdx = &curSeq->d20aCurIdx;
		++*curIdx;
		mesLine.key = (uint32_t)curIdx;

		objHndl performer = curSeq->performer;
		if (objects.IsUnconscious(performer))
		{
			curSeq->d20ActArrayNum = curSeq->d20aCurIdx;
			hooked_print_debug_message("\nUnconscious actor %s - cutting sequence", objects.description.GetDisplayName(performer, performer));
		}
		if (curSeq->d20aCurIdx >= (int32_t)curSeq->d20ActArrayNum) break;	
		
		memcpy(&var_24, &curSeq->actnSthgField, sizeof(var_24));
		d20a = &curSeq->d20ActArray[*curIdx];
		
		auto errCode = sub_10096450( curSeq, *curIdx,&var_24);
		if (errCode)
		{
			
			mesLine.key = errCode + 1000;
			mesFuncs.GetLine_Safe(*actionMesHandle, &mesLine);
			hooked_print_debug_message("Action unavailable for %s (%I64x): %s\n", objects.description.GetDisplayName(d20a->d20APerformer, d20a->d20APerformer), d20a->d20APerformer, mesLine.value );
			*actnProcState = errCode;
			curSeq->actnSthgField.field_B34__errCodeApparently = errCode;
			objects.floats->floatMesLine(performer, 1, 1, mesLine.value);
			curSeq->d20ActArrayNum = curSeq->d20aCurIdx;
			break;
		}

		if (seqSthg_10B3D5C0[0]){	seqSthg_10B3D5C0[1] = 1;	}

		d20->d20aTriggerCombatCheck(curSeq, *curIdx);

		if (d20a->d20ActType != D20A_AOO_MOVEMENT)
		{
			if ( d20->d20aTriggersAOOCheck(d20a, &var_24) && AOOSthg2_100981C0(d20a->d20APerformer))
			{
				hooked_print_debug_message("\nSequence Preempted %s (%I64x)", description.GetDisplayName(d20a->d20APerformer, d20a->d20APerformer), d20a->d20APerformer);
				--*(curIdx);
				actionPerformRecursion();
			} else
			{
				memcpy(&curSeq->actnSthgField, &var_24, sizeof(var_24));
				*(uint32_t*)(&curSeq->actnSthgField.callActionFrameFlags) |= (uint32_t)D20CAF_NEED_ANIM_COMPLETED;
				InterruptSthg_10099360(d20a);
				hooked_print_debug_message("\nPerforming action for %s (%I64x)", description.GetDisplayName(d20a->d20APerformer, d20a->d20APerformer), d20a->d20APerformer);
				d20->d20Defs[d20a->d20ActType].performFunc(d20a);
				InterruptSthg_10099320(d20a);
			}
			return;
		}
		if (d20->tumbleCheck(d20a))
		{
			AOOSthgSub_10097D50(d20a->d20APerformer, d20a->d20ATarget);
			D20CAF * caflags = & (curSeq->d20ActArray[curSeq->d20ActArrayNum - 1].d20Caf);
			*((uint32_t*)caflags) |= (uint32_t)D20CAF_AOO_MOVEMENT;
			actionPerformRecursion();
			return;
		}
	}
	if ( projectileCheckBeforeNextAction())
	{
		curSeqNext();
		return;
	}
	return;

}

void ActionSequenceSystem::actionPerformRecursion()
{
	if (*actnProc_10B3D5A0){ return; }
	if (!actSeqOkToPerform())
	{
		hooked_print_debug_message("Sequence given while performing previous action - aborted. \n");
		d20->d20ActnInit(d20->globD20Action->d20APerformer, d20->globD20Action);
		return;
	}
	ActnSeq * curSeq = *actSeqCur;
	if (combat->isCombatActive() || !actSeqSpellHarmful(curSeq) || !combatTriggerSthg(curSeq) ) // POSSIBLE BUG: I think this can cause spells to be overridden (e.g. when the temple priests prebuff simulataneously with you, and you get the spell effect instead) TODO
	{
		hooked_print_debug_message("\n%s performing sequence...", description.GetDisplayName(curSeq->performer, curSeq->performer));
		if (isSimultPerformer(curSeq->performer))
		{ 
			hooked_print_debug_message("simultaneously...");
			if (!simulsOk(curSeq))
			{
				if (simulsAbort(curSeq->performer)) hooked_print_debug_message("sequence not allowed... aborting simuls (pending).\n");
				else hooked_print_debug_message("sequence not allowed... aborting subsequent simuls.\n");
				return;
			}
			hooked_print_debug_message("succeeded...\n");
			
		} else
		{
			hooked_print_debug_message("independently.\n");
		}
		*actnProcState = 0;
		curSeq->seqOccupied |= 1;
		actionPerform();
		for (auto i = *actSeqCur; isPerforming(i->performer); i = *actSeqCur) // I think actionPerform can modify the sequence, so better be safe
		{
			if (i->seqOccupied & 1)
			{
				auto curIdx = i->d20aCurIdx;
				if (curIdx >= 0 && curIdx < i->d20ActArrayNum)
				{
					auto caflags = i->d20ActArray[curIdx].d20Caf;
					if ( (caflags & D20CAF_NEED_PROJECTILE_HIT) || (caflags & D20CAF_NEED_ANIM_COMPLETED ) )
					break;
				}
			}
			actionPerform();
		}
	}

	//_actionPerformRecursion();
}

bool ActionSequenceSystem::projectileCheckBeforeNextAction()
{
	// 
	ActnSeq * curSeq = *actSeqCur;
	if (curSeq->d20aCurIdx < curSeq->d20ActArrayNum){ return 0; }
	if (curSeq->d20ActArrayNum <= 0){ return 1; }
	for (auto i = 0; i < curSeq->d20ActArrayNum; i++)
	{
		if (d20->d20Defs[curSeq->d20ActArray[i].d20ActType].performFunc 
			&& (curSeq->d20ActArray[i].d20Caf & D20CAF_NEED_PROJECTILE_HIT)){
			return 0;
		}
	}
	return 1;
}

uint32_t ActionSequenceSystem::actSeqSpellHarmful(ActnSeq* actSeq)
{
	uint32_t result;
	macAsmProl;
	__asm{
		mov ecx, this;
		mov esi, [ecx]._actSeqSpellHarmful;
		mov ebx, actSeq;
		call esi;
		mov result, eax;
	}
	macAsmEpil;
	return result;
}

uint32_t ActionSequenceSystem::isSimultPerformer(objHndl objHnd)
{
	uint32_t numPerfs = *numSimultPerformers;
	objHndl * perfs = simultPerformerQueue;
	assert(numPerfs < 10000);
	if (numPerfs <= 0){ return 0; }
	for (uint32_t i = 0; i < numPerfs; i++) 
	{
		if (perfs[i] == objHnd){ return 1; }
	}
	return 0;
}

uint32_t ActionSequenceSystem::simulsOk(ActnSeq* actSeq)
{
	auto numd20as = actSeq->d20ActArrayNum;
	if (numd20as <= 0){ return 1; }
	auto d20a = &actSeq->d20ActArray[0];
	auto numStdAttkActns = 0;
	while (d20a->d20ActType & D20A_STANDARD_ATTACK) // maybe we should add use potion in here, because the co8 critters usually start with that so they can breakfree; then again it might cause intereferences TODO
	{
		++d20a;
		++numStdAttkActns;
		if (numStdAttkActns >= numd20as) return 1;
	}
	if (isSomeoneAlreadyActingSimult(actSeq->performer))
	{
		return 0;
	} else
	{
		*numSimultPerformers = 0;
		*simultPerformerQueue = 0i64;
		hooked_print_debug_message("first simul actor, proceeding");

	}
	return 1;

}

uint32_t ActionSequenceSystem::simulsAbort(objHndl objHnd)
{
	// aborts sequence; returns 1 if objHnd is not the first in queue
	if (!combat->isCombatActive()) return 0;
	uint32_t isFirstInQueue = 1;
	auto numSimuls = *numSimultPerformers;
	if (numSimuls <= 0) return 0;

	for (uint32_t i = 0; i < numSimuls; i++)
	{
		if (objHnd == simultPerformerQueue[i])
		{
			if (isFirstInQueue)
			{
				*numSimultPerformers = 0;
				*simultPerformerQueue = 0i64;
				return 0;
			}
			else{
				*numSimultPerformers = *simulsIdx;
				memcpy(actnSthg118CD3C0, &(*actSeqCur)->actnSthgField, sizeof(ActnSthg));
				hooked_print_debug_message("Simul aborted %s (%d)", description.GetDisplayName(objHnd, objHnd), *simulsIdx);
				return 1;
			}
		}

		if (isPerforming(simultPerformerQueue[i]))	isFirstInQueue = 0;
	}
	
	return 0;
}

uint32_t ActionSequenceSystem::isSomeoneAlreadyActingSimult(objHndl objHnd)
{
	if (*numSimultPerformers == 0) return 0;
	assert(*numSimultPerformers < 10000);
	for (uint32_t i = 0; i < *numSimultPerformers; i++)
	{
		if (objHnd == simultPerformerQueue[i]) return 0;

		auto perf = simultPerformerQueue[i];
		for (auto j = 0; j < actSeqArraySize; j++)
		{
			if (actSeqArray[j].seqOccupied &&actSeqArray[j].performer == perf) return 1;
		}
	}
	return 0;
}
#pragma endregion


#pragma region hooks

uint32_t _addD20AToSeq(D20Actn* d20a, ActnSeq* actSeq)
{
	return actSeqSys.addD20AToSeq(d20a, actSeq);
}


uint32_t _addSeqSimple(D20Actn* d20a, ActnSeq * actSeq)
{
	return actSeqSys.addSeqSimple(d20a, actSeq);
}

uint32_t _seqCheckAction(D20Actn* d20a, ActnSthg* iO)
{
	return actSeqSys.seqCheckAction(d20a, iO);
}

uint32_t _isPerforming(objHndl objHnd)
{
	return actSeqSys.isPerforming(objHnd);
}

uint32_t _actSeqOkToPerform()
{
	return actSeqSys.actSeqOkToPerform();
}

void _actionPerform()
{
	actSeqSys.actionPerform();
};


uint32_t _isSimultPerformer(objHndl objHnd)
{
	return actSeqSys.isSimultPerformer(objHnd);
}

uint32_t _seqCheckFuncs(ActnSthg* actnSthg)
{
	return actSeqSys.seqCheckFuncs(actnSthg);
}


uint32_t __cdecl _moveSeqD20SthgCdecl(D20Actn* d20a, ActnSeq* actSeq, ActnSthg* actnSthg, float distSthg, float reach, int flagSthg)
{
	return actSeqSys.moveSeqD20Sthg(d20a, actSeq, actnSthg, distSthg, reach, flagSthg);
};

uint32_t __declspec(naked) _moveSeqD20SthgUsercallWrapper(ActnSeq* actSeq, ActnSthg* actnSthg, float distSthg, float reach, int flagSthg)
{ //, D20_Action *d20aIn@<eax>
	macAsmProl;
	__asm{
		mov ebx, flagSthg;
		push ebx;
		mov edi, reach;
		push edi;
		mov ebx, distSthg;
		push ebx;
		mov edi, actnSthg;
		push edi;
		mov ebx, actSeq;
		push ebx;
		push eax;
		mov eax, _moveSeqD20SthgCdecl;
		call eax;
		add esp, 24; // result should be in eax already
	}
	macAsmEpil
		__asm retn;
}

uint32_t _unspecifiedMoveAddToSeq(D20Actn* d20a, ActnSeq* actSeq, ActnSthg* actnSthg)
{
	return actSeqSys.moveSeqD20Sthg(d20a, actSeq, actnSthg, 0.0, 0.0, 1);
}
#pragma endregion