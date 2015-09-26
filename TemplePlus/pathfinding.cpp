
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
		objIt.radius = objects.GetRadius(pqr->mover) * (float)0.7;
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
					if ( !(pathFlags & PQF_IGNORE_CRITTERS) && !objects.IsUnconscious(objIt.results[i].obj))
					{
						if (!pqr->mover)
							break;
						if (!critterSys.IsFriendly(pqr->mover, objIt.results[i].obj))
							break;
					}
				}
			}
			++i;
			if (i > 1)
			{
				int dummy = 1;
			}
				
			if (i >= objIt.resultCount)
				goto LABEL_19;
		}
		result = 0;
	}
	LABEL_19:
	return result;
}

BOOL Pathfinding::PathStraightLineIsClearOfStaticObstacles(PathQueryResult* pqr, PathQuery* pq, LocAndOffsets from, LocAndOffsets to)
{
	BOOL result = 1;
	ObjIterator objIt;
	objIt.origin = from;
	objIt.targetLoc = to;
	objIt.flags = (ObjIteratorFlags)0x38;
	objIt.radius = (float) 0.01;

	if (objIt.TargettingSthg_100BACE0())
	{
		auto resultObj = objIt.results;
		for (auto i = 0; i < objIt.resultCount; i++)
		{
			if (!objIt.results[i].obj
				&& objIt.results[i].flags & 2)
				return 0;
		}
	}

	return result;
}

int Pathfinding::GetDirection(int idxFrom, int n, int idxTo)
{
	int deltaX; 
	int deltaY; 
	int result; 

	deltaX = idxTo % n - idxFrom % n;
	deltaY = idxTo / n - idxFrom / n;
	if (deltaY >= 0)
	{
		if (deltaX >= 0)
		{
			if (deltaY <= 0)
			{
				if (deltaY >=0 )
					result = (deltaY < 1) + 4;
				else
					result = 6;
			}
			else if (deltaX >= 0 )
			{
				result = (deltaX >= 1) + 3;
			}
			else
			{
				result = 2;
			}
		}
		else if (deltaY >= 0)
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
		if (deltaX >= 0)
		{
			result = deltaX < 1;
			result += 6;
		}
	}
	return result;
}

int Pathfinding::FindPathShortDistance(PathQuery* pq, PathQueryResult* pqr)
{ // uses a form of A*
	// pathfinding heuristic:
	// taxicab metric h(dx,dy)=max(dx, dy), wwhere  dx,dy is the subtile difference
	int referenceTime = 0;
	Subtile fromSubtile;
	Subtile toSubtile;
	Subtile _fromSubtile;
	Subtile shiftedSubtile;
	
	fromSubtile = fromSubtile.fromField( locSys.subtileFromLoc(&pqr->from));
	toSubtile = toSubtile.fromField(locSys.subtileFromLoc(&pqr->to));


	if (pq->critter)
	{
		int attemptCount;
		if (objects.GetType(pq->critter) == obj_t_npc)
		{
			if (*pathFindRefTime && (timeGetTime() - *pathFindRefTime) < 1000)
			{
				attemptCount = *pathFindAttemptCount;
				if (*pathFindAttemptCount > 10 || *pathTimeCumulative > 250)
				{
					return 0;
				}
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
	int idxTarget  = toSubtileX - halfDeltaShiftedX + ((toSubtileY - halfDeltaShiftedY) << 7);
	
	int idxTgtX = idxTarget % 128;
	int idxTgtY = idxTarget / 128;

	struct PathSubtile
	{
		int length;
		int refererIdx;
		int idxPreviousChain; // is actually +1 (0 means none); i.e. subtract 1 to get the actual idx
		int idxNextChain; // same as above
	};
	PathSubtile pathFindAstar[128 * 128];
	memset(pathFindAstar, 0, sizeof(pathFindAstar));

	pathFindAstar[idxMinus0].length = 1;
	pathFindAstar[idxMinus0].refererIdx = -1;


	int lastChainIdx = idxMinus0;
	int firstChainIdx = idxMinus0;
	int deltaIdxX;
	int distanceMetric;
	
	int refererIdx;
	int curIdx;
	int heuristic;
	int minHeuristic = 0x7FFFffff;
	int idxPrevChain;
	int idxNextChain;

	int shiftedXidx;
	int shiftedYidx;
	int newIdx;




	if (idxMinus0 == -1)
		goto LABEL_74;

	while(1)
	{
		refererIdx = -1;
		curIdx = idxMinus0;
		do // loop over the chain to find the node with minimal heuristic; initially the chain is just the "from" node
		{
			deltaIdxX = abs(curIdx % 128 - idxTgtX);
			distanceMetric = abs(curIdx / 128 - idxTgtY);
			if (deltaIdxX > distanceMetric)
				distanceMetric = deltaIdxX;

			heuristic = pathFindAstar[curIdx].length + 10 * distanceMetric;
			if ( (heuristic/10) <= pq->maxShortPathFindLength)
			{
				if (refererIdx == -1 || heuristic < minHeuristic)
				{
					minHeuristic = pathFindAstar[curIdx].length + 10 * distanceMetric;
					refererIdx = curIdx;
				}
				idxNextChain = pathFindAstar[curIdx].idxNextChain;
			} else
			{
				idxPrevChain = pathFindAstar[curIdx].idxPreviousChain;
				pathFindAstar[curIdx].length = 0x80000000;
				if (idxPrevChain)
					pathFindAstar[idxPrevChain - 1].idxNextChain = pathFindAstar[curIdx].idxNextChain;
				else
					firstChainIdx = pathFindAstar[curIdx].idxNextChain - 1;
				idxNextChain = pathFindAstar[curIdx].idxNextChain;
				if (idxNextChain)
					pathFindAstar[idxNextChain - 1].idxPreviousChain = pathFindAstar[curIdx].idxPreviousChain;
				else
					lastChainIdx = pathFindAstar[curIdx].idxPreviousChain - 1;
				pathFindAstar[curIdx].idxPreviousChain = 0;
				pathFindAstar[curIdx].idxNextChain = 0;
			}
			curIdx = idxNextChain - 1;
		} while (curIdx >= 0);

		if (refererIdx == -1)
			goto LABEL_74;
		if (refererIdx == idxTarget) break;
		_fromSubtile.x = halfDeltaShiftedX + (refererIdx % 128);
		_fromSubtile.y = halfDeltaShiftedY + (refererIdx / 128);

		// loop over all possible directions for better path
		for (auto direction = 0; direction < 8 ; direction++)
		{
			if (!locSys.ShiftSubtileOnceByDirection(_fromSubtile, direction, &shiftedSubtile))
				continue;
			shiftedXidx = shiftedSubtile.x - halfDeltaShiftedX;
			shiftedYidx = shiftedSubtile.y - halfDeltaShiftedY;
			if (shiftedXidx >= 0 && shiftedXidx < 128 && shiftedYidx >= 0 && shiftedYidx < 128)
			{
				newIdx = shiftedXidx + (shiftedYidx << 7);
			}
			else
				continue;

			if (pathFindAstar[newIdx].length == 0x80000000)
				continue;
			
			LocAndOffsets subPathFrom;
			LocAndOffsets subPathTo;
			locSys.SubtileToLocAndOff(_fromSubtile, &subPathFrom);
			locSys.SubtileToLocAndOff(shiftedSubtile, &subPathTo);

			if (!PathStraightLineIsClear(pqr, pq, subPathFrom, subPathTo))
				continue;
			int oldLength = pathFindAstar[newIdx].length;
			int newLength = pathFindAstar[refererIdx].length + 14 - 4 * (direction % 2) ; // +14 for diagonal, +10 for straight

			if (oldLength == 0 || abs(oldLength) > newLength)
			{
				pathFindAstar[newIdx].length = newLength;
				pathFindAstar[newIdx].refererIdx = refererIdx;
				if (!pathFindAstar[newIdx].idxPreviousChain && !pathFindAstar[newIdx].idxNextChain) //  if node is not part of chain
				{
					pathFindAstar[lastChainIdx].idxNextChain = newIdx + 1;
					pathFindAstar[newIdx].idxPreviousChain = lastChainIdx + 1;
					pathFindAstar[newIdx].idxNextChain = 0;
					lastChainIdx = newIdx;
				}

			}
			
		}
		pathFindAstar[refererIdx].length = -pathFindAstar[refererIdx].length;
		idxPrevChain = pathFindAstar[refererIdx].idxPreviousChain - 1;
		if (idxPrevChain != -1)
		{
			pathFindAstar[idxPrevChain].idxNextChain = pathFindAstar[refererIdx].idxNextChain;
			idxMinus0 = firstChainIdx;
		} else
		{
			idxMinus0 = pathFindAstar[refererIdx].idxNextChain - 1;
			firstChainIdx = idxMinus0;
		}
		if (pathFindAstar[refererIdx].idxNextChain)
			pathFindAstar[pathFindAstar[refererIdx].idxNextChain - 1].idxPreviousChain 
			 = pathFindAstar[refererIdx].idxPreviousChain;
		else
			lastChainIdx = pathFindAstar[refererIdx].idxPreviousChain - 1;
		pathFindAstar[refererIdx].idxPreviousChain = 0;
		pathFindAstar[refererIdx].idxNextChain = 0;
		if (idxMinus0 == -1)
			goto LABEL_74;
		idxTgtX = idxTarget % 128;
	}


	// count the directions
	auto refIdx = &pathFindAstar[refererIdx].refererIdx;
	int directionsCount = 0;
	while( *refIdx != -1)
	{
		directionsCount++;
		refIdx = &pathFindAstar[*refIdx].refererIdx;
	}
	

	if (directionsCount > pq->maxShortPathFindLength)
	{
	LABEL_74: if (referenceTime)
		*pathTimeCumulative += timeGetTime() - referenceTime;
		return 0;
	}
	int lastIdx = idxTarget;
	refIdx = &pathFindAstar[lastIdx].refererIdx;
	for (int i = directionsCount - 1; i >= 0; --i)
	{
		refIdx = &pathFindAstar[lastIdx].refererIdx;
		pqr->directions[i] = GetDirection(*refIdx, 128, lastIdx);
		lastIdx = *refIdx;
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