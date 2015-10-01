
#include "stdafx.h"
#include "pathfinding.h"
#include "location.h"
#include "combat.h"
#include "obj_iterator.h"
#include "obj.h"
#include "critter.h"
#include "path_node.h"

Pathfinding pathfindingSys;


struct PathFindAddresses : temple::AddressTable
{
	int (__cdecl*FindPathUsingNodes) (PathQuery*pq, Path*pqr);
	int(__cdecl *FindPathShortDistanceAdjRadius)(PathQuery* pq, Path* pqr);
	PathFindAddresses()
	{
		rebase(FindPathShortDistanceAdjRadius, 0x10041E30);
		rebase(FindPathUsingNodes, 0x10042B50);
		
	}
} addresses;


class PathfindingReplacements : TempleFix
{
public: 
	const char* name() override {
		return "Bathfinding Functions" "Function Replacements";
	} 
	void apply() override 
	{
	//	 replaceFunction(0x10040520, _ShouldUsePathnodesUsercallWrapper); 
		 replaceFunction(0x10041730, _FindPathShortDistanceSansTarget);
		 replaceFunction(0x10043070, _FindPath);
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
	rebase(_FindPath, 0x10043070);

	rebase(canPathToParty,0x10057F80); 

	rebase(pathQArray, 0x1186AC60);
	
	rebase(aStarMaxTimeMs, 0x102AF7FC);
	rebase(aStarMaxWindowMs, 0x102AF7F8);
	rebase(aStarTimeIdx, 0x1094B0F8);
	rebase(aStarTimeElapsed, 0x1095B180);
	rebase(aStarTimeEnded, 0x1095B1D0);

	rebase(pathCache, 0x1099B220);
	rebase(pathCacheIdx, 0x109DD260);
	rebase(pathCacheCleared, 0x109DD264);
	
	
	rebase(pathFindRefTime, 0x109DD270);
	rebase(pathFindAttemptCount, 0x109DD274);
	rebase(pathTimeCumulative, 0x109DD278);
	rebase(npcPathStraightLineTimeRef, 0x109DD288);
	rebase(npcPathStraightLineAttemps, 0x109DD28C);
	rebase(npcPathStraightLineCumulativeTime, 0x109DD290);
	rebase(pathSthgFlag_10B3D5C8,0x10B3D5C8); 
	
	
}

int Pathfinding::PathCacheGet(PathQuery* pq, Path* pathOut)
{
	if (*pathCacheCleared)
		return 0;

	for (int i = 0; i < PATH_RESULT_CACHE_SIZE; i++ )
	{
		auto pathCacheQ = &pathCache[i];
		if (pq->flags != pathCacheQ->flags || pq->critter != pathCacheQ->critter)
			continue;
		auto fromSubtileCache = locSys.subtileFromLoc(&pathCacheQ->from);
		auto fromSubtile = locSys.subtileFromLoc(&pq->from);
		if (fromSubtileCache != fromSubtile)
			continue;
		
		if (pq->flags & PQF_TARGET_OBJ)
		{
			if ( pq->targetObj != pathCacheQ->targetObj)
				continue;
		}
		else
		{
			auto toSubtileCache = locSys.subtileFromLoc(&pathCacheQ->to);
			auto toSubtile = locSys.subtileFromLoc(&pq->to);
			if (toSubtileCache != toSubtile)
				continue;
		}

		if (abs(pq->tolRadius - pathCacheQ->tolRadius) > 0.0001
			|| abs(pq->distanceToTarget - pathCacheQ->distanceToTarget) > 0.0001
			|| pq->maxShortPathFindLength != pathCacheQ->maxShortPathFindLength
			)
			continue;
		
		memcpy(pathOut, &pathCache[i].path, sizeof(Path));
		return 1;
		
	}
	return 0;
}

void Pathfinding::PathCachePush(PathQuery* pq, Path* pqr)
{
	memcpy(&pathCache[*pathCacheIdx].path, pqr, sizeof(Path));
	memcpy(&pathCache[(*pathCacheIdx)++], pq, sizeof(PathQuery));

	*pathCacheCleared = 0;
	if (*pathCacheIdx >= 40)
		*pathCacheIdx = 0;
}

int Pathfinding::PathSumTime()
{
	int totalTime = 0;
	int now = timeGetTime();
	for (int i = 0; i < 20; i++)
	{
		int astarIdx = *aStarTimeIdx - i;
		if (astarIdx < 0)
			astarIdx += 20;
		if (!aStarTimeEnded[astarIdx])
			break;
		if (now - aStarTimeEnded[astarIdx] > *aStarMaxWindowMs)
			break;
		totalTime += aStarTimeElapsed[i];
	}
	
	return totalTime;
}

void Pathfinding::PathRecordTimeElapsed(int refTime)
{
	if ((*aStarTimeIdx)++ >= 20)
		*aStarTimeIdx = 0;
	aStarTimeEnded[*aStarTimeIdx] = timeGetTime();
	aStarTimeElapsed[*aStarTimeIdx] = aStarTimeEnded[*aStarTimeIdx] - refTime;
}

uint32_t Pathfinding::ShouldUsePathnodes(Path* pathQueryResult, PathQuery* pathQuery)
{
	bool result = false;
	LocAndOffsets from = pathQuery->from;
	if (!(pathQuery->flags & PQF_DONT_USE_PATHNODES))
	{
		if (combatSys.isCombatActive())
		{
			if (locSys.distBtwnLocAndOffs(pathQueryResult->from, pathQueryResult->to) > (float)1200.0)
			{
				return true;
			}
			
		}
		else if (locSys.distBtwnLocAndOffs(pathQueryResult->from, pathQueryResult->to) > (float)600.0)
		{
			return true;
		}
	}
	return result;
}



void Pathfinding::PathInit(Path* pqr, PathQuery* pq)
{
	PathQueryFlags pqFlags = pq->flags;

	pqr->flags = 0;
	pqr->field_1a10 = 0;
	if (pq->flags & PQF_HAS_CRITTER)
		pqr->mover = pq->critter;
	else
		pqr->mover = 0;

	pqr->from = pq->from;

	if (pqFlags & PQF_TARGET_OBJ)
	{
		pqr->to = objects.GetLocationFull(pq->targetObj);
		if (pqFlags & PQF_ADJUST_RADIUS)
		{
			float tgtRadius = objects.GetRadius(pq->targetObj);
			pq->distanceToTarget = tgtRadius + pq->distanceToTarget;
			pq->tolRadius = tgtRadius + pq->tolRadius;
			if (pqFlags & PQF_HAS_CRITTER)
			{
				float critterRadius = objects.GetRadius(pq->critter);
				pq->distanceToTarget = critterRadius + pq->distanceToTarget;
				pq->tolRadius = critterRadius + pq->tolRadius;
			}
		}
	} else
	{
		pqr->to = pq->to;
		if (!(pqFlags & PQF_TO_EXACT))
		{
			pqr->to.off_x = 0;
			pqr->to.off_y = 0;
		}
			
	}

	if (!(pqFlags & PQF_MAX_PF_LENGTH_STHG))
		pq->maxShortPathFindLength = 200;

	memset(pqr->directions, 0, sizeof(pqr->directions));
	memset(pqr->tempNodes, 0, sizeof(pqr->tempNodes));
	pqr->initTo1 = 1;
	pqr->currentNode = 0;
}

bool Pathfinding::GetAlternativeTargetLocation(PathQueryResult* pqr, PathQuery* pq)
{
	auto pqFlags = pq->flags;
	if (!(pqFlags & PQF_ADJUST_RADIUS) 
		&& !(pqFlags & (PQF_10 | PQF_20))) 
	{
		auto toLoc = pqr->to;
		if (!PathDestIsClear(pq, pqr->mover, toLoc))
		{
			if (!(pqFlags & PQF_ALLOW_ALTERNATIVE_TARGET_TILE))
				return 0;
			
			for (int i = 1; i++; i<=18)
			{
				auto iOff = i * 9.4280901;
				for (int j = -i; j < i; j++)
				{
					auto jOff = j * 9.4280901;
					auto toLocTweaked = toLoc;
					toLocTweaked.off_x += jOff;
					toLocTweaked.off_y -= iOff;
					locSys.RegularizeLoc(&toLocTweaked);
					if (PathDestIsClear(pq, pqr->mover, toLocTweaked))
					{
						pqr->to = toLocTweaked;
						return 1;
					}

					 toLocTweaked = toLoc;
					toLocTweaked.off_x -= jOff;
					toLocTweaked.off_y += iOff;
					locSys.RegularizeLoc(&toLocTweaked);
					if (PathDestIsClear(pq, pqr->mover, toLocTweaked))
					{
						pqr->to = toLocTweaked;
						return 1;
					}

					 toLocTweaked = toLoc;
					toLocTweaked.off_x -= jOff;
					toLocTweaked.off_y -= iOff;
					locSys.RegularizeLoc(&toLocTweaked);
					if (PathDestIsClear(pq, pqr->mover, toLocTweaked))
					{
						pqr->to = toLocTweaked;
						return 1;
					}

					 toLocTweaked = toLoc;
					toLocTweaked.off_x += jOff;
					toLocTweaked.off_y += iOff;
					locSys.RegularizeLoc(&toLocTweaked);
					if (PathDestIsClear(pq, pqr->mover, toLocTweaked))
					{
						pqr->to = toLocTweaked;
						return 1;
					}
				}
			}
			return 0;
		}
	}
	return 1;
}

int Pathfinding::FindPathUsingNodes(PathQuery* pq, Path* path)
{
	auto from = path->from;
	int nodeTotal = 0;
	int fromClosestId, toClosestId;
	int chainLength;
	int nodeIds[MAX_PATH_NODE_CHAIN_LENGTH];

	if (!pathNodeSys.FindClosestPathNode(&path->from, &fromClosestId))
		return 0;

	if (!pathNodeSys.FindClosestPathNode(&path->to, &toClosestId))
		return 0;
	
	chainLength = pathNodeSys.FindPathBetweenNodes(fromClosestId, toClosestId, nodeIds, MAX_PATH_NODE_CHAIN_LENGTH);
	if (!chainLength)
	{
		return 0;
	}
		



	MapPathNode nodeTemp1, nodeTemp0;
	int i0 = 0;


	if (chainLength>1)
	{
		pathNodeSys.GetPathNode(nodeIds[1], &nodeTemp1);
		pathNodeSys.GetPathNode(nodeIds[0], &nodeTemp0);
		float distFromSecond = locSys.distBtwnLocAndOffs(from, nodeTemp1.nodeLoc);
		if (distFromSecond < 614.0)
		{
			if (locSys.distBtwnLocAndOffs(nodeTemp0.nodeLoc, nodeTemp1.nodeLoc) > distFromSecond)
				i0 = 1;
		}
		pathNodeSys.GetPathNode(nodeIds[chainLength - 2], &nodeTemp1);
		pathNodeSys.GetPathNode(nodeIds[chainLength - 1], &nodeTemp0);
		float distSecondLastToDestination = locSys.distBtwnLocAndOffs(path->to, nodeTemp1.nodeLoc);
		if (distSecondLastToDestination < 614.0)
		{
			if (locSys.distBtwnLocAndOffs(nodeTemp0.nodeLoc, nodeTemp1.nodeLoc) > distSecondLastToDestination)
				chainLength--;
		}
	}

	// add paths from node to node
	PathQuery pathQueryLocal;
	Path pathLocal;
	for (int i = i0; i < chainLength; i++)
	{
		pathNodeSys.GetPathNode(nodeIds[i], &nodeTemp1);
		memcpy(&pathQueryLocal, pq, sizeof(PathQuery));
		pathQueryLocal.flags = (PathQueryFlags)(
			(uint32_t)pathQueryLocal.flags & (~PQF_ADJUST_RADIUS) | PQF_ALLOW_ALTERNATIVE_TARGET_TILE
			);
		pathQueryLocal.from = from;
		pathQueryLocal.to = nodeTemp1.nodeLoc;
		
		PathInit(&pathLocal, &pathQueryLocal);

		memcpy(&pathQueryLocal, pq, sizeof(PathQuery));
		pathQueryLocal.flags = (PathQueryFlags)(
			(uint32_t)pathQueryLocal.flags & (~PQF_ADJUST_RADIUS) | PQF_ALLOW_ALTERNATIVE_TARGET_TILE
			);
		pathQueryLocal.from = from;
		pathQueryLocal.to = nodeTemp1.nodeLoc;
		pathLocal.from = from;
		pathLocal.nodeCount = 0;
		pathLocal.nodeCount2 = 0;
		pathLocal.nodeCount3 = 0;

		int nodeCountAdded = FindPathSansNodes(&pathQueryLocal, &pathLocal);
		if (!nodeCountAdded || (nodeCountAdded + nodeTotal > pq->maxShortPathFindLength))
			return 0;
		memcpy(&path->nodes[nodeTotal], pathLocal.nodes, sizeof(LocAndOffsets) * nodeCountAdded);
		nodeTotal += nodeCountAdded;
		from = nodeTemp1.nodeLoc;
	}


	memcpy(&pathQueryLocal, pq, sizeof(PathQuery));
	pathQueryLocal.from = from;
	pathQueryLocal.to = path->to;

	PathInit(&pathLocal, &pathQueryLocal);

	memcpy(&pathQueryLocal, pq, sizeof(PathQuery));
	pathQueryLocal.from = from;
	pathQueryLocal.to = path->to;
	pathLocal.from = from;

	int nodeCountAdded = FindPathSansNodes(&pathQueryLocal, &pathLocal);
	if (!nodeCountAdded || (nodeCountAdded + nodeTotal > pq->maxShortPathFindLength))
		return 0;

	memcpy(&path->nodes[nodeTotal], pathLocal.nodes, sizeof(LocAndOffsets) * nodeCountAdded);
	int nodeCount = nodeTotal + nodeCountAdded;
	path->nodeCount = nodeCount;
	path->to = path->nodes[nodeCount - 1];
	return nodeCount;
	//return addresses.FindPathUsingNodes(pq, path);
}

int Pathfinding::FindPathStraightLine(Path* pqr, PathQuery* pq)
{
	auto pqFlags = pq->flags;
	if (!(pqFlags & PQF_ADJUST_RADIUS))
	{
		if (!PathStraightLineIsClear(pqr, pq, pqr->from, pqr->to))
			return 0;
		
		pqr->flags |= PF_STRAIGHT_LINE_SUCCEEDED;
		return 1;
	} 

	// get an adjust To location (in a very hackneyed way :P)
	float fromAbsX, fromAbsY, toAbsX, toAbsY;
		
	locSys.GetOverallOffset(pqr->from, &fromAbsX, &fromAbsY);
	locSys.GetOverallOffset(pqr->to, &toAbsX, &toAbsY);
		
	long double deltaX = toAbsX - fromAbsX;
	long double deltaY = toAbsY - fromAbsY;
	long double distFromTo = sqrt(deltaX*deltaX + deltaY*deltaY);
	if (distFromTo <= pq->tolRadius - 2.0)
		return 0;
		
	long double adjustFactor = (distFromTo - (pq->tolRadius - 2.0)) / distFromTo;
	auto adjToLoc = pqr->from;
		adjToLoc.off_x += deltaX * adjustFactor;
		adjToLoc.off_y += deltaY * adjustFactor;
	locSys.RegularizeLoc(&adjToLoc);
		
	if (!PathDestIsClear(pq, pqr->mover, adjToLoc))
		return 0;

	if (!PathStraightLineIsClear(pqr, pq, pqr->from, adjToLoc))
		return 0;

	pqr->to = adjToLoc;
	pqr->flags |= PF_STRAIGHT_LINE_SUCCEEDED;

	return 1;
}

LocAndOffsets* Pathfinding::PathTempNodeAddByDirections(int idx, Path* pqr, LocAndOffsets* newNode)
{
	if (idx > pqr->nodeCount3)
		idx = pqr->nodeCount3;
	auto tempNode = &pqr->tempNodes[idx];
	if (!tempNode->location)
	{
		auto loc = pqr->from;
		for (int i = 0; i < idx; i++)
			locSys.ShiftLocationByOneSubtile(&loc, pqr->directions[i], &loc);
		
		*tempNode = loc;
	}
	*newNode = *tempNode;
	return newNode;
}

void Pathfinding::PathNodesAddByDirections(Path* pqr, PathQuery* pq)
{
	auto pqFlags = pqr->flags;
	if (pqFlags & PF_STRAIGHT_LINE_SUCCEEDED)
	{
		pqr->flags = pqr->flags - PF_STRAIGHT_LINE_SUCCEEDED;
		pqr->nodes[0] = pqr->to;
		pqr->nodeCount = 1;

	} else
	{
		LocAndOffsets newNode;
		LocAndOffsets fromLoc = pqr->from;
		PathTempNodeAddByDirections(pqr->nodeCount3, pqr, &newNode);
		pqr->nodeCount = 0;
		for (int i = 2; i < pqr->nodeCount2; i++)
		{
			PathTempNodeAddByDirections(i, pqr, &newNode);
			if (!PathStraightLineIsClear(pqr, pq, fromLoc, newNode))
			{
				PathTempNodeAddByDirections(i - 1, pqr, &newNode);
				fromLoc = newNode;
				pqr->nodes[pqr->nodeCount++] = newNode;
			}
		}
		pqr->nodes[pqr->nodeCount++] = pqr->to;
	}
	pqr->currentNode = 0;
}

int Pathfinding::FindPathShortDistanceAdjRadius(PathQuery* pq, Path* pqr)
{
	return addresses.FindPathShortDistanceAdjRadius(pq, pqr);
}

int Pathfinding::FindPathForcecdStraightLine(Path* pqr, PathQuery* pq)
{
	auto critter = pq->critter;
	if (critter)
	{
		// auto critterRadius = objects.GetRadius(pq->critter); //wtf? looks like they commented something out here
		if (objects.GetType(critter) == obj_t_npc)
		{
			if (*npcPathStraightLineTimeRef 
				&& (timeGetTime() - *npcPathStraightLineTimeRef) < 1000)
			{
				
				if (*npcPathStraightLineAttemps > 10
					|| *npcPathStraightLineCumulativeTime > 250)
					return 0;
				(*npcPathStraightLineAttemps)++;
			} else
			{
				*npcPathStraightLineAttemps = 1;
				*npcPathStraightLineCumulativeTime = 0;
				*npcPathStraightLineTimeRef = timeGetTime();
			}
		}
	}
	return 0;
}

int Pathfinding::FindPathSansNodes(PathQuery* pq, Path* pqr)
{
	auto pqFlags = pq->flags;
	int result = 0;
	if (!(pqFlags & PQF_DONT_USE_STRAIGHT_LINE))
	{
		result = FindPathStraightLine(pqr, pq);
		pqr->nodeCount = 0;
		pqr->nodeCount2 = 0;
		pqr->nodeCount3 = 0;
		if (result)
		{
			PathNodesAddByDirections(pqr, pq);
			return pqr->nodeCount;
		}
	}
	pqFlags = pq->flags;

	if (!(pqFlags&PQF_200)) {
		if (pqFlags &PQF_100)
			result = FindPathShortDistanceSansTarget(pq, pqr);
		else if (pqFlags & PQF_ADJUST_RADIUS)
			result = FindPathShortDistanceAdjRadius(pq, pqr);
		else if (!(pqFlags & PQF_FORCED_STRAIGHT_LINE))
		{
			//for (int i = 0; i < 100; i++)
				result = FindPathShortDistanceSansTarget(pq, pqr);
		}
			
		else
			result = FindPathForcecdStraightLine(pqr, pq);

		pqr->nodeCount = result;
		pqr->nodeCount2 = result;
		pqr->nodeCount3 = result;
		if (result)
			PathNodesAddByDirections(pqr, pq);
		
	}
	return pqr->nodeCount;
}

int Pathfinding::FindPath(PathQuery* pq, PathQueryResult* pqr)
{
	int gotPath = 0;
	int triedPathNodes = 0;
	int refTime;

	PathInit(pqr, pq);
	auto toSubtile = locSys.subtileFromLoc(&pqr->to);
	if (locSys.subtileFromLoc(&pqr->from) == toSubtile || !GetAlternativeTargetLocation(pqr, pq))
		return 0;

	if (PathCacheGet(pq, pqr)) // has this query been done before? if so copies it and returns the result
		return pqr->nodeCount;

	if (pq->flags & PQF_A_STAR_TIME_CAPPED && PathSumTime() >= *aStarMaxTimeMs)
	{
		pqr->flags |= 16;
		return 0;
	}


	refTime = timeGetTime();
	if (ShouldUsePathnodes(pqr, pq))
	{
		triedPathNodes = 1;
		gotPath = FindPathUsingNodes(pq, pqr);
		if (!gotPath)
		{
			int dummy = 1;
		}
	} 
	else
	{
		gotPath = FindPathSansNodes(pq, pqr);
	}
		
	if (!gotPath)
	{
		if (!(pq->flags & PQF_DONT_USE_PATHNODES)  && !triedPathNodes)
		{
			gotPath = FindPathUsingNodes(pq, pqr);
			if (!gotPath)
			{
				int dummy = 1;
			}
		}
	}

	if (gotPath)
	{
		pqr->flags |= PF_COMPLETE;
	}
	PathCachePush(pq, pqr);
	PathRecordTimeElapsed(refTime);
	return gotPath;

}

BOOL Pathfinding::PathStraightLineIsClear(Path* pqr, PathQuery* pq, LocAndOffsets from, LocAndOffsets to)
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
				
			if (i >= objIt.resultCount)
				goto LABEL_19;
		}
		result = 0;
	}
	LABEL_19:
	return result;
}

BOOL Pathfinding::PathStraightLineIsClearOfStaticObstacles(Path* pqr, PathQuery* pq, LocAndOffsets from, LocAndOffsets to)
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

int Pathfinding::FindPathShortDistanceSansTarget(PathQuery* pq, Path* pqr)
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
			{
				continue;
			}
				
			//else
			//{
				//for (int k = 0; k < 99; k++)
				//	PathStraightLineIsClear(pqr, pq, subPathFrom, subPathTo);

			//}
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


int _FindPathShortDistanceSansTarget(PathQuery* pq, PathQueryResult* pqr)
{
	return pathfindingSys.FindPathShortDistanceSansTarget(pq, pqr);
}

int _FindPath(PathQuery* pq, PathQueryResult* pqr)
{
	return pathfindingSys.FindPath(pq, pqr);
}