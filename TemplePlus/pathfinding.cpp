
#include "stdafx.h"
#include "pathfinding.h"
#include "location.h"
#include "combat.h"
#include "obj_iterator.h"
#include "obj.h"
#include "critter.h"

Pathfinding pathfindingSys;


class PathfindingReplacements : TempleFix
{
public: 
	const char* name() override {
		return "Bathfinding Functions" "Function Replacements";
	} 
	void apply() override 
	{
		 replaceFunction(0x10040520, _ShouldUsePathnodesUsercallWrapper); 
		 replaceFunction(0x10041730, _FindPathShortDistance);
	}
} pathFindingReplacements;

float Pathfinding::pathLength(Path* path)
{
	float distTot;
	if (path->flags & PQF_UNK2)	return loc->distBtwnLocAndOffs(path->to, path->from) / 12.0f;
	distTot = 0;
	auto nodeFrom = path->from;
	for (int i = 0; i < path->nodeCount; i++)
	{
		auto nodeTo = path->nodes[i];
		distTot += loc->distBtwnLocAndOffs(nodeFrom, nodeTo);
		nodeFrom = nodeTo;
	}
	return distTot / 12.0f;
}

bool Pathfinding::pathQueryResultIsValid(PathQueryResult* pqr)
{
	return pqr != nullptr && pqr >= pathQArray && pqr < &pathQArray[pfCacheSize];
}

Pathfinding::Pathfinding() {
	static_assert(sizeof(PathQuery) == 0x50, "Path Query has the wrong size");
	static_assert(sizeof(Path) == 0x1a18, "Path has the wrong size");
	static_assert(sizeof(PathQueryResult) == 0x1a20, "Path Query Result has the wrong size");

	loc = &locSys;

	
	
	//rebase(PathStraightLineIsClear, 0x10040A90); // signed int __usercall PathStraightLineIsClear@<eax>(PathQueryResult *pqr@<ebx>, PathQuery *pathQ, LocAndOffsets from, LocAndOffsets to)
	rebase(PathDestIsClear, 0x10040C30);
	// rebase(FindPathStraightLine, 0x100427F0); //  signed int __usercall FindPathStraightLine@<eax>(PathQueryResult *pqr@<eax>, PathQuery *pq@<edi>)
	rebase(FindPath, 0x10043070);

	rebase(canPathToParty,0x10057F80); 

	rebase(pathQArray, 0x1186AC60);

	rebase(pathFindRefTime, 0x109DD270);
	rebase(pathFindAttemptCount, 0x109DD274);
	rebase(pathTimeCumulative, 0x109DD278);
	rebase(pathSthgFlag_10B3D5C8,0x10B3D5C8); 
	
}

uint32_t Pathfinding::ShouldUsePathnodes(PathQueryResult* pathQueryResult, PathQuery* pathQuery)
{
	bool result = false;
	LocAndOffsets from = pathQuery->from;
	if (!(pathQuery->flags & PQF_UNKNOWN4000h))
	{
		if (combatSys.isCombatActive())
		{
			if (locSys.distBtwnLocAndOffs(pathQueryResult->from, pathQueryResult->to) > (float)1200.0)
			{
				return true;
			}
			
		}
		else if (locSys.distBtwnLocAndOffs(pathQueryResult->from, pathQueryResult->to) > (float)800.0)
		{
			return true;
		}
	}
	return result;
}


uint32_t __cdecl _ShouldUsePathnodesCDecl(PathQueryResult * pqr, PathQuery * pq)
{
	return pathfindingSys.ShouldUsePathnodes(pqr, pq);
}


BOOL Pathfinding::PathStraightLineIsClear(PathQueryResult* pqr, PathQuery* pq, LocAndOffsets from, LocAndOffsets to)
{
	BOOL result = 1;
	ObjIterator objIt;
	objIt.origin = from;
	objIt.targetLoc = to;
	objIt.flags = (ObjIteratorFlags)0x38;
	if (pqr->mover)
	{
		objIt.flags = (ObjIteratorFlags)((int)objIt.flags | 6);
		objIt.performer = pqr->mover;
		objIt.radius = objects.GetRadius(pqr->mover) * 0.7;
	}
	if (objIt.TargettingSthg_100BACE0())
	{
		int i = 0;
		while( objIt.results[i].obj)
		{
			auto objType = objects.GetType(objIt.results[i].obj);
			ObjectFlag objFlags = (ObjectFlag)objects.GetFlags(objIt.results[i].obj);
			if (! (objFlags & OF_NO_BLOCK))
			{
				auto pathFlags = pq->flags;
				if ( pathFlags & PQF_400 || objType)
				{
					if (objType != obj_t_pc && objType != obj_t_npc)
						break;
					if (pathFlags & PQF_80 == 0 && objects.IsUnconscious(objIt.results[i].obj))
					{
						if (!pqr->mover)
							break;
						if (!critterSys.IsFriendly(pqr->mover, objIt.results[i].obj))
							break;
					}
				}
			}
			++i;
			if (i >= objIt.resultCount)
				goto LABEL_19;
		}
		result = 0;
	}
	LABEL_19:
	return result;
}

int Pathfinding::GetDirection(int idx1, int n, int idx2)
{
	int deltaX; 
	int deltaY; 
	int result; 

	deltaX = idx2 % n - idx1 % n;
	deltaY = idx2 / n - idx1 / n;
	if (deltaY >= 0)
	{
		if (deltaX >= 0)
		{
			if (deltaY <= 0)
			{
				if (deltaY > -1)
					result = (deltaY < 1) + 4;
				else
					result = 6;
			}
			else if (deltaX > -1)
			{
				result = (deltaX >= 1) + 3;
			}
			else
			{
				result = 2;
			}
		}
		else if (deltaY > -1)
		{
			result = (deltaY >= 1) + 1;
		}
		else
		{
			result = 0;
		}
	}
	else
	{
		result = 0;
		if (deltaX > -1)
		{
			result = deltaX < 1;
			result += 6;
		}
	}
	return result;
}

int Pathfinding::FindPathShortDistance(PathQuery* pq, PathQueryResult* pqr)
{
	int referenceTime = 0;
	Subtile fromSubtile;
	Subtile toSubtile;
	Subtile _fromSubtile;
	Subtile _toSubtile;
	Subtile shiftedSubtile;
	PathQuery pqLocal; // for testing
	if (pqr)
	{
		fromSubtile = fromSubtile.fromField( locSys.subtileFromLoc(&pqr->from));
		toSubtile = toSubtile.fromField(locSys.subtileFromLoc(&pqr->to));
	} else //  TESTING
	{
		fromSubtile.x = 20;
		fromSubtile.y = 20;
		toSubtile.x = 25;
		toSubtile.y= 20;
		pqLocal.field28 = 200;
		pq = &pqLocal;
	}

	if (pq->critter)
	{
		int attemptCount;
		if (objects.GetType(pq->critter) == obj_t_npc)
		{
			if (*pathFindRefTime && (timeGetTime() - *pathFindRefTime) < 1000)
			{
				attemptCount = *pathFindAttemptCount;
				if (*pathFindAttemptCount > 10 || *pathTimeCumulative > 250)
					return 0;
			} else
			{
				*pathFindRefTime = timeGetTime();
				attemptCount = 0;
				*pathTimeCumulative = 0;
			}
			*pathFindAttemptCount = attemptCount + 1;
			referenceTime = timeGetTime();
		}
	}

	int fromSubtileX = fromSubtile.x;
	int fromSubtileY = fromSubtile.y;
	int toSubtileX = toSubtile.x;
	int toSubtileY = toSubtile.y;

	int deltaSubtileX;
	int deltaSubtileY;
	int lowerSubtileX;
	int lowerSubtileY;
	int halfDeltaShiftedX;
	int halfDeltaShiftedY;

	if (fromSubtileX <= toSubtileX)
	{
		deltaSubtileX = toSubtileX - fromSubtileX;
		lowerSubtileX = fromSubtileX;
	} else
	{
		deltaSubtileX = fromSubtileX - toSubtileX;
		lowerSubtileX = toSubtileX;
	}

	if (fromSubtileY <= toSubtileY)
	{
		deltaSubtileY = toSubtileY - fromSubtileY;
		lowerSubtileY = fromSubtileY;
	}
	else
	{
		deltaSubtileY = fromSubtileY - toSubtileY;
		lowerSubtileY = toSubtileY;
	}

	if (deltaSubtileX > 64 || deltaSubtileY > 64)
		return 0;
	
	halfDeltaShiftedX = lowerSubtileX + deltaSubtileX / 2 - 64;
	halfDeltaShiftedY = lowerSubtileY + deltaSubtileY / 2 - 64;

	int idxMinus0 = fromSubtileX - halfDeltaShiftedX + ((fromSubtileY - halfDeltaShiftedY) << 7);
	int idxPlus0  = toSubtileX - halfDeltaShiftedX + ((toSubtileY - halfDeltaShiftedY) << 7);
	
	int idxPlusX = idxPlus0 % 128;
	int idxPlusY = idxPlus0 / 128;

	struct PathSubtile
	{
		int idx;
		int refererIdx;
		int field8;
		int fieldC;
	};
	PathSubtile pathSubtileMat[128 * 128];
	memset(pathSubtileMat, 0, sizeof(pathSubtileMat));

	pathSubtileMat[idxMinus0].idx = 1;
	pathSubtileMat[idxMinus0].refererIdx = -1;


	int idx1 = idxMinus0;
	int idx2 = idxMinus0;
	int deltaIdxX;
	int deltaIdxY;
	
	int v17;
	int curIdx;
	int v21;
	int v22;
	int v23;
	int v24;
	int v25;
	int v26;
	Subtile v27;
	int v30;
	int v31;
	int v32;
	int v33;
	int v34;
	int v35;
	int v39;
	int directionsCount;
	int direction;
	int _direction;
	int v54;
	int fieldC;
	if (idxMinus0 == -1)
		goto LABEL_74;

	while(1)
	{
		v17 = -1;
		curIdx = idxMinus0;
		do
		{
			deltaIdxX = abs(curIdx % 128 - idxPlusX);
			deltaIdxY = abs(curIdx / 128 - idxPlusY);
			if (deltaIdxX > deltaIdxY)
				deltaIdxY = deltaIdxX;
			v21 = curIdx;
			v22 = pathSubtileMat[curIdx].idx + 10 * deltaIdxY;
			if ( (v22/10) <= pq->field28)
			{
				if (v17 == -1 || v22 < deltaSubtileY)
				{
					deltaSubtileY = pathSubtileMat[curIdx].idx + 10 * deltaIdxY;
					v17 = curIdx;
				}
				fieldC = pathSubtileMat[v21].fieldC;
			} else
			{
				v23 = pathSubtileMat[v21].field8;
				pathSubtileMat[v21].idx = 0x80000000;
				if (v23)
					pathSubtileMat[v23 - 1].fieldC = pathSubtileMat[v21].fieldC;
				else
					idx2 = pathSubtileMat[v21].fieldC - 1;
				v24 = pathSubtileMat[v21].fieldC;
				if (v24)
					pathSubtileMat[v24 - 1].field8 = pathSubtileMat[v21].field8;
				else
					idx1 = pathSubtileMat[v21].field8 - 1;
				fieldC = pathSubtileMat[v21].fieldC;
				pathSubtileMat[v21].field8 = 0;
				pathSubtileMat[v21].fieldC = 0;
			}
			curIdx = fieldC - 1;
		} while (curIdx >= 0);
		v54 = v17;
		if (v17 == -1)
			goto LABEL_74;
		if (v17 == idxPlus0) break;
		v27.x = halfDeltaShiftedX + v17 % 128;
		v27.y = halfDeltaShiftedY + v17 / 128;
		direction = 0;
		_fromSubtile = v27;
		_direction = 0;
		while (1)
		{
			if (!locSys.ShiftSubtileOnceByDirection(v27, direction, &shiftedSubtile))
				goto LABEL_66;
			_toSubtile = shiftedSubtile;
			v30 = shiftedSubtile.x - halfDeltaShiftedX;
			v31 = shiftedSubtile.y - halfDeltaShiftedY;
			if (v30 < 0 || v30 >= 128
				|| v31 < 0 || v31 >= 128
				|| (v32 = v30 + (v31 << 7), v33 = v32,
					pathSubtileMat[v32].idx == 0x80000000))
			{
				v17 = v54;
			}
			else
			{
				LocAndOffsets subPathFrom;
				LocAndOffsets subPathTo;
				locSys.SubtileToLocAndOff(_fromSubtile, &subPathFrom);
				locSys.SubtileToLocAndOff(_toSubtile, &subPathTo);
				v17 = v54;
				if (!PathStraightLineIsClear(pqr, pq, subPathFrom, subPathTo))
					goto LABEL_66;
				int v36 = pathSubtileMat[v33].idx;
				int v37 = pathSubtileMat[v54].idx - 4 * (_direction % 2) + 14;
				int v38 = v36 == 0;
				if (v36 < 0)
				{
					if (-v36 <= v37)
						goto LABEL_66;
				}
				if (v36 <= 0 || v36 > v37)
				{
					pathSubtileMat[v33].idx = v37;
					pathSubtileMat[v33].refererIdx = v54;
					if (!pathSubtileMat[v33].field8 && !pathSubtileMat[v33].fieldC)
					{
						pathSubtileMat[idx1].fieldC = v32 + 1;
						pathSubtileMat[v33].field8 = idx1 + 1;
						pathSubtileMat[v33].fieldC = 0;
						idx1 = v30 + (v31 << 7);
					}

				}
			}
		LABEL_66: direction = _direction + 1;
			if (!(++_direction <= 7))
				break;
			v27 = _fromSubtile;
		}
		pathSubtileMat[v17].idx = -pathSubtileMat[v17].idx;
		if (pathSubtileMat[v17].field8)
		{
			pathSubtileMat[v17 - 1].fieldC = pathSubtileMat[v17].fieldC;
			idxMinus0 = idx2;
		} else
		{
			idxMinus0 = pathSubtileMat[v17].fieldC - 1;
			idx2 = idxMinus0;
		}
		if (pathSubtileMat[v17].fieldC)
			pathSubtileMat[pathSubtileMat[v17].fieldC - 1].field8 = pathSubtileMat[v17].field8;
		else
			idx1 = pathSubtileMat[v17].field8 - 1;
		pathSubtileMat[v17].field8 = 0;
		pathSubtileMat[v17].fieldC = 0;
		if (idxMinus0 == -1)
			goto LABEL_74;
		idxPlusX = idxPlus0 % 128;
	}

	auto v47 = &pathSubtileMat[v17].refererIdx;
	directionsCount = 0;
	if (*v47 != -1)
	{
		int v50;
		auto v49 = *v47;
		do
		{
			v49 = *v47;
			v50 = pathSubtileMat[v49].refererIdx;
			v47 = &pathSubtileMat[v49].refererIdx;
			directionsCount++;
				
		} 
		while (pathSubtileMat[*v47].refererIdx != -1);
	}

	if (directionsCount > pq->field28)
	{
	LABEL_74: if (referenceTime)
		*pathTimeCumulative += timeGetTime() - referenceTime;
		return 0;
	}
	int v51 = idxPlus0;
	auto v53 = &pathSubtileMat[v51].refererIdx;
	for (int i = directionsCount - 1; i >= 0; --i)
	{
		v53 = &pathSubtileMat[v51].refererIdx;
		pqr->field30[i] = GetDirection(*v53, 128, v51);
		v51 = *v53;
	}
	if (pq->flags & PQF_10)
		--directionsCount;
	if (referenceTime)
		*pathTimeCumulative += timeGetTime() - referenceTime;

	return directionsCount;
	
}

uint32_t __declspec(naked) _ShouldUsePathnodesUsercallWrapper()
{
	macAsmProl
		__asm{
		push ecx;
		push eax;
		call _ShouldUsePathnodesCDecl;
		add esp, 8;
	}
	macAsmEpil
	__asm  retn; 
}

int _FindPathShortDistance(PathQuery* pq, PathQueryResult* pqr)
{
	return pathfindingSys.FindPathShortDistance(pq, pqr);
}