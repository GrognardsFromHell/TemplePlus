
#include "stdafx.h"
#include "pathfinding.h"
#include "location.h"
#include "combat.h"
#include "raycast.h"
#include "obj.h"
#include "critter.h"
#include "path_node.h"
#include "game_config.h"
#include "gamesystems/map/sector.h"
#include "objlist.h"
#include "util/config.h"

Pathfinding pathfindingSys;

struct ProximityList
{
	int count = 0;
	struct ProxListObj
	{
		objHndl obj;
		float radius;
		LocAndOffsets loc;

	} proxListObjs[256];

	void Append(objHndl obj)
	{
		if (count >= 256)
		{
			logger->error("Error: Proximity list cap reached");
			assert(count < 256);
		}
		proxListObjs[count].obj = obj;
		proxListObjs[count].radius = objects.GetRadius(obj);
		proxListObjs[count].loc = objects.GetLocationFull(obj);
		count++;
	}

	bool FindNear(LocAndOffsets loc, float radius)
	{
		for (int i = 0; i < count; i++)
		{
			if (abs((int)loc.location.locx - (int)proxListObjs[i].loc.location.locx) < radius + proxListObjs[i].radius)
			{
				if (abs((int)loc.location.locy - (int)proxListObjs[i].loc.location.locy) < radius)
				{
					if (locSys.Distance3d(loc, proxListObjs[i].loc) < radius + proxListObjs[i].radius)
					{
						if (config.pathfindingDebugMode)
							logger->info("Pathfinding bump into critter: {} at location {}", description.getDisplayName(proxListObjs[i].obj), proxListObjs[i].loc);
						return true;
					}
						
				}
			}
		}
		return false;
	}

	void Populate(PathQuery * pq, Path * pqr, float radius)
	{
		ObjList objlist;
		objlist.ListRadius(pqr->from, INCH_PER_TILE * 40, OLC_PATH_BLOCKER);
		int objlistSize = objlist.size();
		if (pqr->mover)
		{
			for (int i = 0; i < objlistSize; i++)
			{
				objHndl obj = objlist.get(i);
				auto objFlags = objects.GetFlags(obj);
				auto objType = objects.GetType(obj);
				if (!(objFlags & OF_NO_BLOCK))
				{
					if ((pq->flags & PQF_DOORS_ARE_BLOCKING) || (objType != obj_t_portal))
					{
						if (objType == obj_t_pc || objType == obj_t_npc)
						{
							if (critterSys.IsFriendly(obj, pqr->mover) || critterSys.IsDeadOrUnconscious(obj))
								continue;
						}
						Append(obj);
					}

				}
			}
		}
	}
};


struct PathFindAddresses : temple::AddressTable
{
	PathResultCache * pathCache; // 40 entries, used as a ring buffer
	int * pathCacheIdx;
	int * pathCacheCleared;

	int * aStarMaxTimeMs;
	int * aStarMaxWindowMs;
	int * aStarTimeIdx;
	int * aStarTimeElapsed; // array 20
	int * aStarTimeEnded; //  array 20


	int * pathTimeCumulative;
	int * pathFindAttemptCount;
	int * pathFindRefTime;

	int * npcPathStraightLineTimeRef;
	int * npcPathStraightLineAttemps;
	int * npcPathStraightLineCumulativeTime;

	PathQueryResult * pathQArray;
	uint32_t * pathSthgFlag_10B3D5C8;

	int(__cdecl*PathDestIsClear)(PathQuery* pq, objHndl mover, LocAndOffsets destLoc); // checks if there's anything blocking the destination location (taking into account the mover's radius)
	objHndl(__cdecl * canPathToParty)(objHndl objHnd);

	bool(__cdecl *_FindPath)(PathQuery *query, PathQueryResult *result); // now replaced
	int (__cdecl*FindPathUsingNodes) (PathQuery*pq, Path*pqr);
	int(__cdecl *FindPathShortDistanceAdjRadius)(PathQuery* pq, Path* pqr);
	PathFindAddresses()
	{
		rebase(PathDestIsClear, 0x10040C30);

		rebase(FindPathShortDistanceAdjRadius, 0x10041E30);
		rebase(FindPathUsingNodes, 0x10042B50);



		rebase(_FindPath, 0x10043070);

		rebase(canPathToParty, 0x10057F80);

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
		rebase(pathSthgFlag_10B3D5C8, 0x10B3D5C8);
		
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
		 replaceFunction(0x10042A00, _PathAstarInit);
	}
} pathFindingReplacements;

float Pathfinding::pathLength(Path* path)
{
	float distTot;
	if (path->flags & PF_STRAIGHT_LINE_SUCCEEDED)	return loc->distBtwnLocAndOffs(path->to, path->from) / 12.0f;
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
	
	aStarMaxTimeMs = 4000;
	aStarMaxWindowMs = 5000;
	aStarTimeIdx = -1;

	memset(pathCache, 0, sizeof(pathCache));
	pathCacheIdx = 0;
	pathCacheCleared = 1;


	// these two are still used a lot in other places outside the pathfinding system so I'm keeping them here for the time being
	rebase(pathSthgFlag_10B3D5C8,0x10B3D5C8); 
	rebase(pathQArray, 0x1186AC60);	
}

int Pathfinding::PathCacheGet(PathQuery* pq, Path* pathOut)
{
	if (pathCacheCleared)
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
			|| abs(pq->distanceToTargetMin - pathCacheQ->distanceToTargetMin) > 0.0001
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
	memcpy(&pathCache[pathCacheIdx].path, pqr, sizeof(Path));
	memcpy(&pathCache[pathCacheIdx++], pq, sizeof(PathQuery));

	pathCacheCleared = 0;
	if (pathCacheIdx >= PATH_RESULT_CACHE_SIZE)
		pathCacheIdx = 0;
}

int Pathfinding::PathSumTime()
{
	int totalTime = 0;
	int now = timeGetTime();
	for (int i = 0; i < 20; i++)
	{
		int astarIdx = aStarTimeIdx - i;
		if (astarIdx < 0)
			astarIdx += 20;
		if (!aStarTimeEnded[astarIdx])
			break;
		if (now - aStarTimeEnded[astarIdx] > aStarMaxWindowMs)
			break;
		totalTime += aStarTimeElapsed[i];
	}
	
	return totalTime;
}

void Pathfinding::PathRecordTimeElapsed(int refTime)
{
	if (++aStarTimeIdx >= 20)
		aStarTimeIdx = 0;
	aStarTimeEnded[aStarTimeIdx] = timeGetTime();
	aStarTimeElapsed[aStarTimeIdx] = aStarTimeEnded[aStarTimeIdx] - refTime;
}

void Pathfinding::PathAstarInit()
{
	gameConfigFuncs.AddSetting("astar_max_window_ms", "5000", _aStarSettingChanged);
	gameConfigFuncs.AddSetting("astar_max_time_ms", "4000", _aStarSettingChanged);
	aStarMaxWindowMs = gameConfigFuncs.GetInt("astar_max_window_ms");
	aStarMaxTimeMs = gameConfigFuncs.GetInt("astar_max_time_ms");
	memset(aStarTimeElapsed, 0, sizeof(aStarTimeElapsed));
	memset(aStarTimeEnded, 0, sizeof(aStarTimeEnded));
	aStarTimeIdx = -1;
}

void Pathfinding::AStarSettingChanged()
{
	aStarMaxWindowMs = gameConfigFuncs.GetInt("astar_max_window_ms");
	aStarMaxTimeMs = gameConfigFuncs.GetInt("a_star_max_time_ms");
}

void Pathfinding::PathCacheInit()
{
	if (!pathCacheCleared)
		memset(pathCache, 0, sizeof(pathCache));
	pathCacheCleared = 1;
}

int Pathfinding::PathDestIsClear(PathQuery* pq, objHndl mover, LocAndOffsets destLoc)
{
	RaycastPacket objIt;
	objIt.origin = destLoc;
	objIt.targetLoc = destLoc;

	*(int*)&objIt.flags |= (ExcludeItemObjects | StopAfterFirstBlockerFound | StopAfterFirstFlyoverFound);

	if (mover)
	{
		*(int*)&objIt.flags |= (HasSourceObj | HasRadius);
		objIt.sourceObj = mover;
		objIt.radius = objects.GetRadius(mover);
	}

	ObjectType objType;
	ObjectFlag objFlags;
	auto pqFlags = pq->flags;
	if (objIt.RaycastShortRange())
	{

		for (int i = 0; i < objIt.resultCount; i++)
		{
			if (!objIt.results[i].obj) // means it's a sector blocker
				return 0;

			objType = objects.GetType(objIt.results[i].obj);

			if ((pqFlags & PQF_DOORS_ARE_BLOCKING) || objType != obj_t_portal)
			{
				objFlags = (ObjectFlag)objects.GetFlags(objIt.results[i].obj);
				if (!(objFlags & OF_NO_BLOCK))
				{
					if ( (objType == obj_t_pc || objType == obj_t_npc ) 
						&& !objects.IsUnconscious(objIt.results[i].obj) )
					{
						if ( (pqFlags & PQF_IGNORE_CRITTERS_ON_DESTINATION) == 0)
						{
							return 0;
						}
					}


				}

			}
		}
	}
	//int result0 = addresses.PathDestIsClear(pq, mover, destLoc);
	//if (!result0)
	//{
	//	int dummy = 1;
	//}
	return 1;

}

uint32_t Pathfinding::ShouldUsePathnodes(Path* pathQueryResult, PathQuery* pathQuery)
{
	bool result = false;
	LocAndOffsets from = pathQuery->from;
	if (!(pathQuery->flags & PQF_DONT_USE_PATHNODES))
	{
		if (combatSys.isCombatActive())
		{
			if (locSys.distBtwnLocAndOffs(pathQueryResult->from, pathQueryResult->to) > (float)600.0)
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
			pq->distanceToTargetMin = tgtRadius + pq->distanceToTargetMin;
			pq->tolRadius = tgtRadius + pq->tolRadius;
			if (pqFlags & PQF_HAS_CRITTER)
			{
				float critterRadius = objects.GetRadius(pq->critter);
				pq->distanceToTargetMin = critterRadius + pq->distanceToTargetMin;
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

bool Pathfinding::GetAlternativeTargetLocation(Path* pqr, PathQuery* pq)
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
	PathQuery pathQueryLocal;
	Path pathLocal;
	memcpy(&pathQueryLocal, pq, sizeof(PathQuery));
	pathQueryLocal.to = path->to;
	pathQueryLocal.from = path->from;
	pathQueryLocal.flags = (PathQueryFlags)(
		(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS | PQF_TARGET_OBJ)) | PQF_ALLOW_ALTERNATIVE_TARGET_TILE | PQF_STRAIGHT_LINE_ONLY_FOR_SANS_NODE);
	PathInit(&pathLocal, &pathQueryLocal);
	pathQueryLocal.to = path->to;
	pathQueryLocal.from = path->from;
	pathQueryLocal.flags = (PathQueryFlags)(
		(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS | PQF_TARGET_OBJ)) | PQF_ALLOW_ALTERNATIVE_TARGET_TILE | PQF_STRAIGHT_LINE_ONLY_FOR_SANS_NODE);
	pathLocal.from = path->from;
	pathLocal.to = pathQueryLocal.to;

	int result = FindPathStraightLine(&pathLocal, &pathQueryLocal);
	if (result)
	{
		pathLocal.nodeCount = 0;
		pathLocal.nodeCount2 = 0;
		pathLocal.nodeCount3 = 0;
		PathNodesAddByDirections(&pathLocal, pq);
		memcpy(path, &pathLocal, sizeof(Path));
		return path->nodeCount;
	}

		
	

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

		// attempt straight line from 2nd last pathnode to destination
		// if this is possible, it will shorten the path and avoid a zigzag going from the last node to destination
		pathNodeSys.GetPathNode(nodeIds[chainLength - 2], &nodeTemp1);
		pathNodeSys.GetPathNode(nodeIds[chainLength - 1], &nodeTemp0);
		if (! (pathQueryLocal.flags & PQF_TARGET_OBJ))
		{
			memcpy(&pathQueryLocal, pq, sizeof(PathQuery));
			pathQueryLocal.to = path->to;
			pathQueryLocal.from = nodeTemp1.nodeLoc;
			pathQueryLocal.flags = (PathQueryFlags)(
				(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS | PQF_TARGET_OBJ)) 
				| PQF_ALLOW_ALTERNATIVE_TARGET_TILE | ((!pathNodeSys.hasClearanceData)* PQF_STRAIGHT_LINE_ONLY_FOR_SANS_NODE));
			PathInit(&pathLocal, &pathQueryLocal);
			pathQueryLocal.to = path->to;
			pathQueryLocal.from = nodeTemp1.nodeLoc;
			pathQueryLocal.flags = (PathQueryFlags)(
				(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS | PQF_TARGET_OBJ))
				| PQF_ALLOW_ALTERNATIVE_TARGET_TILE | ( (!pathNodeSys.hasClearanceData)* PQF_STRAIGHT_LINE_ONLY_FOR_SANS_NODE));
			pathLocal.from = nodeTemp1.nodeLoc;
			pathLocal.to = pathQueryLocal.to;
			int nodeCountAdded = FindPathSansNodes(&pathQueryLocal, &pathLocal);
			if (nodeCountAdded)
			{
				chainLength--;
			}
		} else
		{
			float distSecondLastToDestination = locSys.distBtwnLocAndOffs(path->to, nodeTemp1.nodeLoc);
			if (distSecondLastToDestination < 400.0)
			{
				if (locSys.distBtwnLocAndOffs(nodeTemp0.nodeLoc, nodeTemp1.nodeLoc) > distSecondLastToDestination)
					chainLength--;
			}
		}

	}

	// add paths from node to node

	for (int i = i0; i < chainLength; i++)
	{
		pathNodeSys.GetPathNode(nodeIds[i], &nodeTemp1);
		memcpy(&pathQueryLocal, pq, sizeof(PathQuery));
		pathQueryLocal.flags = (PathQueryFlags)(
			(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS | PQF_TARGET_OBJ)) | PQF_ALLOW_ALTERNATIVE_TARGET_TILE
			//(uint32_t)pathQueryLocal.flags | PQF_ALLOW_ALTERNATIVE_TARGET_TILE
			);
		pathQueryLocal.from = from;
		pathQueryLocal.to = nodeTemp1.nodeLoc;
		
		PathInit(&pathLocal, &pathQueryLocal);

		memcpy(&pathQueryLocal, pq, sizeof(PathQuery));
		pathQueryLocal.flags = (PathQueryFlags)(
			(uint32_t)pathQueryLocal.flags & (~ (PQF_ADJUST_RADIUS | PQF_TARGET_OBJ)) | PQF_ALLOW_ALTERNATIVE_TARGET_TILE
			//(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS | PQF_TARGET_OBJ)) | PQF_ALLOW_ALTERNATIVE_TARGET_TILE
			);

		pathLocal.from = pathQueryLocal.from = from;
		pathLocal.to = pathQueryLocal.to = nodeTemp1.nodeLoc;
		pathLocal.nodeCount = pathLocal.nodeCount2 = pathLocal.nodeCount3 = 0;
		
		if (!GetAlternativeTargetLocation(&pathLocal, &pathQueryLocal)) // verifies that the destination is clear, and if not, tries to get an available tile
		{
			logger->warn("Warning: pathnode not clear");
		}
		if (pathLocal.to != nodeTemp1.nodeLoc) //  path "To" location has been adjusted
		{
			nodeTemp1.nodeLoc = pathLocal.to;
		}


		int nodeCountAdded = FindPathSansNodes(&pathQueryLocal, &pathLocal);
		if (!nodeCountAdded)
		{
			memcpy(&pathQueryLocal, pq, sizeof(PathQuery));
			pathQueryLocal.flags = (PathQueryFlags)(
				(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS )) | PQF_ALLOW_ALTERNATIVE_TARGET_TILE
				);
			pathQueryLocal.from = from;
			pathQueryLocal.to = nodeTemp1.nodeLoc;
			pathLocal.from = from;
			pathLocal.nodeCount = 0;
			pathLocal.nodeCount2 = 0;
			pathLocal.nodeCount3 = 0;
			pathLocal.to = pathQueryLocal.to;

			nodeCountAdded = FindPathSansNodes(&pathQueryLocal, &pathLocal);
		}
		if (!nodeCountAdded || (nodeCountAdded + nodeTotal > pq->maxShortPathFindLength))
		{
			return 0;
		}
			
		memcpy(&path->nodes[nodeTotal], pathLocal.nodes, sizeof(LocAndOffsets) * nodeCountAdded);
		nodeTotal += nodeCountAdded;
		from = nodeTemp1.nodeLoc;
	}

	// now path from the last location (can be an adjusted path node) to the final destination
	memcpy(&pathQueryLocal, pq, sizeof(PathQuery));
	pathQueryLocal.from = from;
	pathQueryLocal.to = path->to;

	PathInit(&pathLocal, &pathQueryLocal);

	memcpy(&pathQueryLocal, pq, sizeof(PathQuery));
	pathLocal.from = pathQueryLocal.from = from;
	pathLocal.to = pathQueryLocal.to = path->to;

	int nodeCountAdded = FindPathSansNodes(&pathQueryLocal, &pathLocal);
	if (   (!nodeCountAdded  && (pathLocal.to != pathLocal.from) )  // there's a possibility that the "from" is within reach, in which case the search will set the To same as From
		|| (nodeCountAdded + nodeTotal > pq->maxShortPathFindLength))
	{
		if (config.pathfindingDebugMode)
		{
			nodeCountAdded = FindPathSansNodes(&pathQueryLocal, &pathLocal);
		}
		return 0;
	}
	
	int nodeCount = nodeTotal + nodeCountAdded;
	if (nodeCountAdded)
	{
		memcpy(&path->nodes[nodeTotal], pathLocal.nodes, sizeof(LocAndOffsets) * nodeCountAdded);
		path->nodeCount = nodeCount;
		path->to = path->nodes[nodeCount - 1];
	} else
	{
		path->nodes[path->nodeCount++] = pathLocal.to;
		path->to = pathLocal.to;
	}
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

#pragma region Preamble
	const unsigned int gridSize = 160; // number of subtiles spanned in the grid (vanilla: 128)
	int referenceTime = 0;
	Subtile fromSubtile, toSubtile, _fromSubtile, shiftedSubtile;

	fromSubtile = fromSubtile.fromField(locSys.subtileFromLoc(&pqr->from));	toSubtile = toSubtile.fromField(locSys.subtileFromLoc(&pqr->to));

	static int npcPathFindRefTime = 0;	static int npcPathFindAttemptCount = 0; 	static int npcPathTimeCumulative = 0;

	if (pq->critter)
	{
		int attemptCount;
		if (objects.GetType(pq->critter) == obj_t_npc)
		{
			if (npcPathFindRefTime && (timeGetTime() - npcPathFindRefTime) < 1000)
			{
				attemptCount = npcPathFindAttemptCount;
				if (npcPathFindAttemptCount > 10 || npcPathTimeCumulative > 250)
				{
					return 0;
				}
			}
			else
			{
				npcPathFindRefTime = timeGetTime();
				attemptCount = 0;
				npcPathTimeCumulative = 0;
			}
			npcPathFindAttemptCount = attemptCount + 1;
			referenceTime = timeGetTime();
		}
	}

	int fromSubtileX = fromSubtile.x;	int fromSubtileY = fromSubtile.y;
	int toSubtileX = toSubtile.x;	int toSubtileY = toSubtile.y;

	int deltaSubtileX = abs(toSubtileX - fromSubtileX);	int deltaSubtileY = abs(toSubtileY - fromSubtileY);
	if (deltaSubtileX > gridSize / 2 || deltaSubtileY > gridSize / 2)
		return 0;

	int lowerSubtileX = min(fromSubtileX, toSubtileX);	int lowerSubtileY = min(fromSubtileY, toSubtileY);

	int cornerX = lowerSubtileX + deltaSubtileX / 2 - gridSize / 2;	int cornerY = lowerSubtileY + deltaSubtileY / 2 - gridSize / 2;

	int curIdx = fromSubtileX - cornerX + ((fromSubtileY - cornerY) * gridSize);
	int idxTarget = toSubtileX - cornerX + ((toSubtileY - cornerY) * gridSize);

	int idxTgtX = idxTarget % gridSize;
	int idxTgtY = idxTarget / gridSize;

	struct PathSubtile
	{
		int length;
		int refererIdx;
		int idxPreviousChain; // is actually +1 (0 means none); i.e. subtract 1 to get the actual idx
		int idxNextChain; // same as above
	};
	PathSubtile pathFindAstar[gridSize * gridSize];
	memset(pathFindAstar, 0, sizeof(pathFindAstar));

	pathFindAstar[curIdx].length = 1;
	pathFindAstar[curIdx].refererIdx = -1;


	int lastChainIdx = curIdx, firstChainIdx = curIdx;
	int newIdx, shiftedXidx, shiftedYidx, deltaIdxX, distanceMetric, refererIdx, heuristic, idxPrevChain, idxNextChain;
	int minHeuristic = 0x7FFFffff;

	float requisiteClearance = objects.GetRadius(pq->critter);
	float diagonalClearance = requisiteClearance * 0.7; // diagonals need to be more restrictive to avoid jaggy paths
	float requisiteClearanceCritters = requisiteClearance * 0.7;
	if (requisiteClearance > 12)
		requisiteClearance *= 0.85;

	if (curIdx == -1)
	{
		if (referenceTime)
			npcPathTimeCumulative += timeGetTime() - referenceTime;
		return 0;
	}

	ProximityList proxList;
	proxList.Populate(pq, pqr, INCH_PER_TILE * 40);

	LocAndOffsets subPathFrom;
	LocAndOffsets subPathTo;
#pragma endregion

	while(1)
	{
		curIdx = firstChainIdx;
		refererIdx = -1;

		do // loop over the Open Set chain to find the node with minimal valid heuristic; initially the chain is just the "from" node
		{
			deltaIdxX = abs((int)(curIdx % gridSize - idxTgtX));
			distanceMetric = abs((int)(curIdx / gridSize - idxTgtY)); // deltaIdxY
			if (deltaIdxX > distanceMetric)
				distanceMetric = deltaIdxX;

			heuristic = pathFindAstar[curIdx].length + 10 * distanceMetric;
			if ((heuristic / 10) <= pq->maxShortPathFindLength)
			{
				if (refererIdx == -1 || heuristic < minHeuristic)
				{
					minHeuristic = pathFindAstar[curIdx].length + 10 * distanceMetric;
					refererIdx = curIdx;
				}
				idxNextChain = pathFindAstar[curIdx].idxNextChain;
			}
			else
			{	// node has exceeded max length, dump it
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
		{ // none found, return 0
			if (referenceTime)
				npcPathTimeCumulative += timeGetTime() - referenceTime;
			return 0;
		}
			
		// Halt condition - within reach of the target
		_fromSubtile.x = cornerX + (refererIdx % gridSize);
		_fromSubtile.y = cornerY + (refererIdx / gridSize);

		locSys.SubtileToLocAndOff(_fromSubtile, &subPathFrom);
		locSys.SubtileToLocAndOff(toSubtile, &subPathTo);
		float distToTgt = locSys.distBtwnLocAndOffs(subPathFrom, pqr->to);

		if (distToTgt >= pq->distanceToTargetMin && distToTgt <= pq->tolRadius)
		{
			if (!(pq->flags & PQF_ADJ_RADIUS_REQUIRE_LOS) || PathAdjRadiusLosClear(pqr, pq, subPathFrom, subPathTo))
			{
				if ( pq->flags & (PQF_20 | PQF_10) || PathDestIsClear(pq, pqr->mover, subPathFrom ))
				{
					break;
				}
			}
		}


		/* 
			find the best adjacent node
		*/
		_fromSubtile.x = cornerX + (refererIdx % gridSize);
		_fromSubtile.y = cornerY + (refererIdx / gridSize);

		// loop over all possible directions for better path
		for (auto direction = 0; direction < 8; direction++)
		{
			if (!locSys.ShiftSubtileOnceByDirection(_fromSubtile, direction, &shiftedSubtile))
				continue;
			shiftedXidx = shiftedSubtile.x - cornerX;
			shiftedYidx = shiftedSubtile.y - cornerY;
			if (shiftedXidx >= 0 && shiftedXidx < gridSize && shiftedYidx >= 0 && shiftedYidx < gridSize)
			{
				newIdx = shiftedXidx + (shiftedYidx * gridSize);
			}
			else
				continue;

			if (pathFindAstar[newIdx].length == 0x80000000)
				continue;

			locSys.SubtileToLocAndOff(_fromSubtile, &subPathFrom);
			locSys.SubtileToLocAndOff(shiftedSubtile, &subPathTo);

			if (PathNodeSys::hasClearanceData)
			{
				SectorLoc secLoc(subPathTo.location);
				//secLoc.GetFromLoc(subPathTo.location);
				int secX = secLoc.x(), secY = secLoc.y();
				int secClrIdx = PathNodeSys::clearanceData.clrIdx.clrAddr[secLoc.y()][secLoc.x()];
				auto secBaseTile = secLoc.GetBaseTile();
				int ssty = shiftedSubtile.y % 192;
				int sstx = shiftedSubtile.x % 192;
				if (direction % 2) // 
				{
					if (PathNodeSys::clearanceData.secClr[secClrIdx].val[shiftedSubtile.y % 192][shiftedSubtile.x % 192] < requisiteClearance)
						continue;
				} else
				{
					if (PathNodeSys::clearanceData.secClr[secClrIdx].val[shiftedSubtile.y % 192][shiftedSubtile.x % 192] < diagonalClearance)
						continue;
				}
				
				bool foundBlockers = false;

				if (proxList.FindNear(subPathTo, requisiteClearanceCritters))
					foundBlockers = true;

				if (foundBlockers)
					continue;

			}
			else if (!PathStraightLineIsClear(pqr, pq, subPathFrom, subPathTo))
			{
				continue;
			}

			int oldLength = pathFindAstar[newIdx].length;
			int newLength = pathFindAstar[refererIdx].length + 14 - 4 * (direction % 2); // +14 for diagonal, +10 for straight

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


		/*
		remove the referer from the Open Set chain
		and prepare the chain for looping it over again
		*/
		pathFindAstar[refererIdx].length = -pathFindAstar[refererIdx].length; 
		// connect referer's previous to its next
		idxPrevChain = pathFindAstar[refererIdx].idxPreviousChain - 1;
		if (idxPrevChain != -1)
		{
			pathFindAstar[idxPrevChain].idxNextChain = pathFindAstar[refererIdx].idxNextChain;
			curIdx = firstChainIdx;
		}
		else // if no "previous" exists, set the First Chain as the referer's next
		{
			firstChainIdx = curIdx = pathFindAstar[refererIdx].idxNextChain - 1;
		}
		//connect its next to its previous
		if (pathFindAstar[refererIdx].idxNextChain)
			pathFindAstar[pathFindAstar[refererIdx].idxNextChain - 1].idxPreviousChain
			= pathFindAstar[refererIdx].idxPreviousChain;
		else // if no "next" exists, set the last chain as its previous
			lastChainIdx = pathFindAstar[refererIdx].idxPreviousChain - 1;
		pathFindAstar[refererIdx].idxPreviousChain = 0;
		pathFindAstar[refererIdx].idxNextChain = 0;
		if (curIdx == -1)
		{
			if (referenceTime)
				npcPathTimeCumulative += timeGetTime() - referenceTime;
			return 0;
		}

	} //  major loop

	// count the directions
	auto refIdx = &pathFindAstar[refererIdx].refererIdx;
	int directionsCount = 0;
	while (*refIdx != -1)
	{
		directionsCount++;
		refIdx = &pathFindAstar[*refIdx].refererIdx;
	}


	if (directionsCount > pq->maxShortPathFindLength)
	{
		if (referenceTime)
			npcPathTimeCumulative += timeGetTime() - referenceTime;
		return 0;
	}

	int lastIdx = refererIdx;
	for (int i = directionsCount - 1; i >= 0; --i)
	{
		refIdx = &pathFindAstar[lastIdx].refererIdx;
		pqr->directions[i] = GetDirection(*refIdx, gridSize, lastIdx);
		lastIdx = *refIdx;
	}
	if (pq->flags & PQF_10)
		--directionsCount;
	if (referenceTime)
		npcPathTimeCumulative += timeGetTime() - referenceTime;

	// modify the destination to the found location
	pqr->to = subPathFrom;
	if (directionsCount == 0) // in case the destination is already within reach
		pqr->to = pqr->from;

	return directionsCount;

	// return addresses.FindPathShortDistanceAdjRadius(pq, pqr);
}

int Pathfinding::FindPathForcecdStraightLine(Path* pqr, PathQuery* pq)
{
	static int npcPathTimeRef = 0;
	static int npcPathAttemps = 0;
	static int npcPathCumulativeTime = 0;

	auto critter = pq->critter;
	if (critter)
	{
		// auto critterRadius = objects.GetRadius(pq->critter); //wtf? looks like they commented something out here
		if (objects.GetType(critter) == obj_t_npc)
		{
			if (npcPathTimeRef 
				&& (timeGetTime() - npcPathTimeRef) < 1000)
			{
				
				if (npcPathAttemps > 10	|| npcPathCumulativeTime > 250)
					return 0;
				(npcPathAttemps)++;
			} else
			{
				npcPathAttemps = 1;
				npcPathCumulativeTime = 0;
				npcPathTimeRef = timeGetTime();
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

	if (!(pqFlags&PQF_STRAIGHT_LINE_ONLY_FOR_SANS_NODE)) {
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
			result = FindPathForcecdStraightLine(pqr, pq); // does nothing - looks like some WIP

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

	if (!config.pathfindingDebugMode )
		if (PathCacheGet(pq, pqr)) // has this query been done before? if so copies it and returns the result
			return pqr->nodeCount;

	if (pq->flags & PQF_A_STAR_TIME_CAPPED && PathSumTime() >= aStarMaxTimeMs)
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
		if (config.pathfindingDebugMode)
		try
		{
			if (pq->critter)
				logger->info("{} pathed successfully to {}", description.getDisplayName(pq->critter), pqr->to);
		} catch (...)
		{
			logger->info("Corrupt handle: {}", pq->critter);
		}
		
		pqr->flags |= PF_COMPLETE;
	}
	PathCachePush(pq, pqr);
	PathRecordTimeElapsed(refTime);
	return gotPath;

}

objHndl Pathfinding::canPathToParty(objHndl objHnd)
{
	return addresses.canPathToParty(objHnd);
}

BOOL Pathfinding::PathStraightLineIsClear(Path* pqr, PathQuery* pq, LocAndOffsets from, LocAndOffsets to)
{
	BOOL result = 1;
	RaycastPacket objIt;
	objIt.origin = from;
	objIt.targetLoc = to;
	auto dx = abs(static_cast<int>(to.location.locx - from.location.locx));
	auto dy = abs(static_cast<int>(to.location.locy - from.location.locy));
	if ( max( dx, dy) >= SECTOR_SIDE_SIZE * 3) // RayCast supports up to a span of 4 sectors
		return 0;
	objIt.flags = static_cast<RaycastFlags>(RaycastFlags::StopAfterFirstFlyoverFound| RaycastFlags::StopAfterFirstBlockerFound | RaycastFlags::ExcludeItemObjects);
	if (pqr->mover)
	{
		objIt.flags = (RaycastFlags)((int)objIt.flags | RaycastFlags::HasRadius | RaycastFlags::HasSourceObj);
		objIt.sourceObj = pqr->mover;
		objIt.radius = objects.GetRadius(pqr->mover) * (float)0.7;
	}
	
	if (objIt.Raycast())
	{
		int i = 0;
		while( objIt.results[i].obj)
		{
			auto objType = objects.GetType(objIt.results[i].obj);
			ObjectFlag objFlags = (ObjectFlag)objects.GetFlags(objIt.results[i].obj);
			if (! (objFlags & OF_NO_BLOCK))
			{
				auto pathFlags = pq->flags;
				if ( (pathFlags & PQF_DOORS_ARE_BLOCKING) || objType)
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

BOOL Pathfinding::PathAdjRadiusLosClear(Path* pqr, PathQuery* pq, LocAndOffsets from, LocAndOffsets to)
{
	BOOL result = 1;
	RaycastPacket objIt;
	objIt.origin = from;
	objIt.targetLoc = to;
	objIt.flags = static_cast<RaycastFlags>(RaycastFlags::StopAfterFirstBlockerFound | RaycastFlags::StopAfterFirstFlyoverFound | RaycastFlags::ExcludeItemObjects);
	objIt.radius = static_cast<float>(0.01);

	if (objIt.Raycast())
	{
		auto resultObj = objIt.results;
		for (auto i = 0; i < objIt.resultCount; i++)
		{
			if (!objIt.results[i].obj
				&& objIt.results[i].flags & RaycastResultFlags::BlockerSubtile)
				return 0;
		}
	}

	return result;
}

ScreenDirections Pathfinding::GetDirection(int idxFrom, int gridSize, int idxTo)
{
	int deltaX; 
	int deltaY; 
	ScreenDirections result;

	deltaX = (idxTo % gridSize) - (idxFrom % gridSize);
	deltaY = (idxTo / gridSize) - (idxFrom / gridSize);
	assert(abs(deltaX) + abs(deltaY) > 0);
	if (deltaY < 0)
	{
		if ( deltaX < 0)
			result = ScreenDirections::Top;
		else if (deltaX > 0)
			result = ScreenDirections::Left;
		else // deltaX == 0
			result = ScreenDirections::TopLeft;
	} 
	else if (deltaY > 0)
	{
		if (deltaX > 0)
			result = ScreenDirections::Bottom;
		else if ( deltaX < 0)
			result = ScreenDirections::Right;
		else // deltaX == 0
			result = ScreenDirections::BottomRight;
	} 
	else // deltaY == 0
	{
		if (deltaX < 0)
			result = ScreenDirections::TopRight;
		else // deltaX > 0
			result = ScreenDirections::BottomLeft;
	}

	
	return result;
}

int Pathfinding::FindPathShortDistanceSansTargetLegacy(PathQuery* pq, Path* pqr)
{ // uses a form of A*
	// pathfinding heuristic:
	// taxicab metric h(dx,dy)=max(dx, dy), wwhere  dx,dy is the subtile difference
	int referenceTime = 0;
	Subtile fromSubtile, toSubtile, _fromSubtile, shiftedSubtile;
	
	fromSubtile = fromSubtile.fromField( locSys.subtileFromLoc(&pqr->from));
	toSubtile = toSubtile.fromField(locSys.subtileFromLoc(&pqr->to));

	static int npcPathFindRefTime = 0;	static int npcPathFindAttemptCount = 0;	static int npcPathTimeCumulative = 0;

	if (pq->critter)
	{
		int attemptCount;
		if (objects.GetType(pq->critter) == obj_t_npc)
		{
			if (npcPathFindRefTime && (timeGetTime() - npcPathFindRefTime) < 1000)
			{
				attemptCount = npcPathFindAttemptCount;
				if (npcPathFindAttemptCount > 10 || npcPathTimeCumulative > 250)
				{
					return 0;
				}
			} else
			{
				npcPathFindRefTime = timeGetTime();
				attemptCount = 0;
				npcPathTimeCumulative = 0;
			}
			npcPathFindAttemptCount = attemptCount + 1;
			referenceTime = timeGetTime();
		}
	}

	int fromSubtileX = fromSubtile.x;	int fromSubtileY = fromSubtile.y;
	int toSubtileX = toSubtile.x;		int toSubtileY = toSubtile.y;


	int deltaSubtileX = abs(toSubtileX - fromSubtileX);	int deltaSubtileY = abs(toSubtileY - fromSubtileY);
	if (deltaSubtileX > 64 || deltaSubtileY > 64)
		return 0;
	int lowerSubtileX = min(fromSubtileX, toSubtileX);	int lowerSubtileY = min(fromSubtileY, toSubtileY);

	
	int cornerX = lowerSubtileX + deltaSubtileX / 2 - 64;
	int cornerY = lowerSubtileY + deltaSubtileY / 2 - 64;

	int idxMinus0 = fromSubtileX - cornerX + ((fromSubtileY - cornerY) << 7);
	int idxTarget  = toSubtileX - cornerX + ((toSubtileY - cornerY) << 7);
	
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




	if (idxMinus0 == -1){}
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
		_fromSubtile.x = cornerX + (refererIdx % 128);
		_fromSubtile.y = cornerY + (refererIdx / 128);

		// loop over all possible directions for better path
		for (auto direction = 0; direction < 8 ; direction++)
		{
			if (!locSys.ShiftSubtileOnceByDirection(_fromSubtile, direction, &shiftedSubtile))
				continue;
			shiftedXidx = shiftedSubtile.x - cornerX;
			shiftedYidx = shiftedSubtile.y - cornerY;
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
				
			int oldLength = pathFindAstar[newIdx].length;
			int newLength = pathFindAstar[refererIdx].length + 14 - 4 * (direction % 2) ; // +14 for diagonal, +10 for straight

			if (oldLength == 0 || abs(oldLength) > newLength)
			{
				pathFindAstar[newIdx].length = newLength;
				pathFindAstar[newIdx].refererIdx = refererIdx;
				if (!pathFindAstar[newIdx].idxPreviousChain && !pathFindAstar[newIdx].idxNextChain) //  if node is not part of Open Set chain
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
		npcPathTimeCumulative += timeGetTime() - referenceTime;
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
		npcPathTimeCumulative += timeGetTime() - referenceTime;

	return directionsCount;
	
}

int Pathfinding::FindPathShortDistanceSansTarget(PathQuery* pq, Path* pqr)
{
	// uses a form of A*
	// pathfinding heuristic:
	// taxicab metric h(dx,dy)=max(dx, dy), wwhere  dx,dy is the subtile difference
	#pragma region Preamble
	const unsigned int gridSize = 160; // number of subtiles spanned in the grid (vanilla: 128)
	int referenceTime = 0;
	Subtile fromSubtile, toSubtile, _fromSubtile, shiftedSubtile;

	fromSubtile = fromSubtile.fromField(locSys.subtileFromLoc(&pqr->from));	toSubtile = toSubtile.fromField(locSys.subtileFromLoc(&pqr->to));

	static int npcPathFindRefTime = 0;	static int npcPathFindAttemptCount = 0; 	static int npcPathTimeCumulative = 0;

	if (pq->critter)
	{
		int attemptCount;
		if (objects.GetType(pq->critter) == obj_t_npc)
		{
			if (npcPathFindRefTime && (timeGetTime() - npcPathFindRefTime) < 1000)
			{
				attemptCount = npcPathFindAttemptCount;
				if (npcPathFindAttemptCount > 10 || npcPathTimeCumulative > 250)
				{
					return 0;
				}
			}
			else
			{
				npcPathFindRefTime = timeGetTime();
				attemptCount = 0;
				npcPathTimeCumulative = 0;
			}
			npcPathFindAttemptCount = attemptCount + 1;
			referenceTime = timeGetTime();
		}
	}

	int fromSubtileX = fromSubtile.x;	int fromSubtileY = fromSubtile.y;
	int toSubtileX = toSubtile.x;	int toSubtileY = toSubtile.y;

	int deltaSubtileX = abs(toSubtileX - fromSubtileX);	int deltaSubtileY = abs(toSubtileY - fromSubtileY);
	if (deltaSubtileX > gridSize/2 || deltaSubtileY > gridSize/2)
		return 0;
	
	int lowerSubtileX = min(fromSubtileX, toSubtileX);	int lowerSubtileY = min(fromSubtileY, toSubtileY);

	int cornerX = lowerSubtileX + deltaSubtileX / 2 - gridSize/2;	int cornerY = lowerSubtileY + deltaSubtileY / 2 - gridSize/2;


	//TileRect tileR;
	//tileR.x1 = lowerSubtileX/3 - 64; tileR.y1 = lowerSubtileY / 3 - 64;
	//tileR.x2 = tileR.x1 + 128; tileR.y2 = tileR.y1 + 128;
	//Subtile blockedSubtiles[129*129];
	//int count;
	//sectorSys.GetTileFlagsArea(&tileR, blockedSubtiles, &count);

	int curIdx = fromSubtileX - cornerX + ((fromSubtileY - cornerY) * gridSize);
	int idxTarget = toSubtileX - cornerX + ((toSubtileY - cornerY) * gridSize);

	int idxTgtX = idxTarget % gridSize;
	int idxTgtY = idxTarget / gridSize;

	struct PathSubtile
	{
		int length;
		int refererIdx;
		int idxPreviousChain; // is actually +1 (0 means none); i.e. subtract 1 to get the actual idx
		int idxNextChain; // same as above
	};
	PathSubtile pathFindAstar[gridSize * gridSize];
	memset(pathFindAstar, 0, sizeof(pathFindAstar));

	pathFindAstar[curIdx].length = 1;
	pathFindAstar[curIdx].refererIdx = -1;


	int lastChainIdx = curIdx;
	int firstChainIdx = curIdx;
	int deltaIdxX;
	int distanceMetric;

	int refererIdx, heuristic;
	int minHeuristic = 0x7FFFffff;
	int idxPrevChain, idxNextChain;

	int shiftedXidx, shiftedYidx, newIdx;

	float requisiteClearance = objects.GetRadius(pq->critter);
	float diagonalClearance = requisiteClearance * 0.7;
	float requisiteClearanceCritters = requisiteClearance * 0.7;
	if (requisiteClearance > 12)
		requisiteClearance *= 0.85;




	if (curIdx == -1)
	{
		if (referenceTime)
			npcPathTimeCumulative += timeGetTime() - referenceTime;
		return 0;
	}



	struct ProximityList proxList;
	proxList.Populate(pq, pqr, INCH_PER_TILE * 40);

#pragma endregion
	if (config.pathfindingDebugMode)
	{
		logger->info("*** START OF PF ATTEMPT SANS TARGET - DESTINATION {} ***", pqr->to);
	}
	while (1)
	{
		refererIdx = -1;
		do // loop over the chain to find the node with minimal heuristic; initially the chain is just the "from" node
		{
			deltaIdxX = abs((int) (curIdx % gridSize - idxTgtX));
			distanceMetric = abs( (int) (curIdx / gridSize - idxTgtY));
			if (deltaIdxX > distanceMetric)
				distanceMetric = deltaIdxX;

			heuristic = pathFindAstar[curIdx].length + 10 * distanceMetric;
			if ((heuristic / 10) <= pq->maxShortPathFindLength)
			{
				if (refererIdx == -1 || heuristic < minHeuristic)
				{
					minHeuristic = pathFindAstar[curIdx].length + 10 * distanceMetric;
					refererIdx = curIdx;
				}
				idxNextChain = pathFindAstar[curIdx].idxNextChain;
			}
			else
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
		{
			if (referenceTime)
				npcPathTimeCumulative += timeGetTime() - referenceTime;
			if (config.pathfindingDebugMode) {
				logger->info("*** END OF PF ATTEMPT SANS TARGET - OPEN SET EMPTY ***");
			}
			return 0;
		}
		
		// halt condition
		if (refererIdx == idxTarget) break;
		
		_fromSubtile.x = cornerX + (refererIdx % gridSize);
		_fromSubtile.y = cornerY + (refererIdx / gridSize);

		// loop over all possible directions for better path
		for (auto direction = 0; direction < 8; direction++)
		{
			if (!locSys.ShiftSubtileOnceByDirection(_fromSubtile, direction, &shiftedSubtile))
				continue;
			shiftedXidx = shiftedSubtile.x - cornerX;
			shiftedYidx = shiftedSubtile.y - cornerY;
			if (shiftedXidx >= 0 && shiftedXidx < gridSize && shiftedYidx >= 0 && shiftedYidx < gridSize)
			{
				newIdx = shiftedXidx + (shiftedYidx * gridSize);
			}
			else
				continue;

			if (pathFindAstar[newIdx].length == 0x80000000)
				continue;

			LocAndOffsets subPathFrom;
			LocAndOffsets subPathTo;
			locSys.SubtileToLocAndOff(_fromSubtile, &subPathFrom);
			locSys.SubtileToLocAndOff(shiftedSubtile, &subPathTo);
			
			if (PathNodeSys::hasClearanceData)
			{
				SectorLoc secLoc(subPathTo.location);
				//secLoc.GetFromLoc(subPathTo.location);
				int secX = secLoc.x(), secY = secLoc.y();
				int secClrIdx = PathNodeSys::clearanceData.clrIdx.clrAddr[secLoc.y()][secLoc.x()];
				auto secBaseTile = secLoc.GetBaseTile();
				int ssty = shiftedSubtile.y % 192;
				int sstx = shiftedSubtile.x % 192;
				if (direction%2 ) // xy straight
				{
					if (PathNodeSys::clearanceData.secClr[secClrIdx].val[shiftedSubtile.y % 192][shiftedSubtile.x % 192] < requisiteClearance)
					{
						if (config.pathfindingDebugMode)
						{
							logger->info("Pathfinding clearance too small:  {},  clearance value {}", subPathTo, PathNodeSys::clearanceData.secClr[secClrIdx].val[shiftedSubtile.y % 192][shiftedSubtile.x % 192]);
						}
						continue;
					}
				} else // xy diagonal
				{
					if (PathNodeSys::clearanceData.secClr[secClrIdx].val[shiftedSubtile.y % 192][shiftedSubtile.x % 192] < diagonalClearance)
					{
						if (config.pathfindingDebugMode)
						{
							logger->info("Pathfinding clearance too small:  {},  clearance value {}", subPathTo, PathNodeSys::clearanceData.secClr[secClrIdx].val[shiftedSubtile.y % 192][shiftedSubtile.x % 192]);
						}
						continue;
					}
				}
				
					
		
				bool foundBlockers = false;

				if (proxList.FindNear(subPathTo, requisiteClearanceCritters))
					foundBlockers = true;

				if (foundBlockers)
					continue;
				
			} 
			else if (!PathStraightLineIsClear(pqr, pq, subPathFrom, subPathTo))
			{
				continue;
			}

			int oldLength = pathFindAstar[newIdx].length;
			int newLength = pathFindAstar[refererIdx].length + 14 - 4 * (direction % 2); // +14 for diagonal, +10 for straight

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

		pathFindAstar[refererIdx].length = -pathFindAstar[refererIdx].length; // mark the referer as used
		// remove the referer from the chain
		// connect its previous to its next
		idxPrevChain = pathFindAstar[refererIdx].idxPreviousChain - 1;
		if (idxPrevChain != -1)
		{
			pathFindAstar[idxPrevChain].idxNextChain = pathFindAstar[refererIdx].idxNextChain;
			curIdx = firstChainIdx;
		}
		else // if no "previous" exists, set the First Chain as the referer's next
		{
			curIdx = pathFindAstar[refererIdx].idxNextChain - 1;
			firstChainIdx = curIdx;
		}
		//connect its next to its previous
		if (pathFindAstar[refererIdx].idxNextChain)
			pathFindAstar[pathFindAstar[refererIdx].idxNextChain - 1].idxPreviousChain
			= pathFindAstar[refererIdx].idxPreviousChain;
		else // if no "next" exists, set the last chain as its previous
			lastChainIdx = pathFindAstar[refererIdx].idxPreviousChain - 1;
		pathFindAstar[refererIdx].idxPreviousChain = 0;
		pathFindAstar[refererIdx].idxNextChain = 0;
		if (curIdx == -1)
		{
			if (referenceTime)
				npcPathTimeCumulative += timeGetTime() - referenceTime;
			if (config.pathfindingDebugMode){
				logger->info("*** END OF PF ATTEMPT SANS TARGET - A* OPTIONS EXHAUSTED ***");
			}
			return 0;
		}
		idxTgtX = idxTarget % gridSize;
	}


	// count the directions
	auto refIdx = &pathFindAstar[refererIdx].refererIdx;
	int directionsCount = 0;
	while (*refIdx != -1)
	{
		directionsCount++;
		refIdx = &pathFindAstar[*refIdx].refererIdx;
	}


	if (directionsCount > pq->maxShortPathFindLength)
	{
		if (referenceTime)
			npcPathTimeCumulative += timeGetTime() - referenceTime;
		return 0;
	}
	int lastIdx = idxTarget;
	refIdx = &pathFindAstar[lastIdx].refererIdx;
	for (int i = directionsCount - 1; i >= 0; --i)
	{
		refIdx = &pathFindAstar[lastIdx].refererIdx;
		pqr->directions[i] = GetDirection(*refIdx, gridSize, lastIdx);
		lastIdx = *refIdx;
	}
	if (pq->flags & PQF_10)
		--directionsCount;
	if (referenceTime)
		npcPathTimeCumulative += timeGetTime() - referenceTime;

	if (config.pathfindingDebugMode)
	{
		logger->info("*** END OF PF ATTEMPT SANS TARGET - {} DIRECTIONS USED ***", directionsCount);
	}


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

void _PathAstarInit()
{
	pathfindingSys.PathAstarInit();
}

void _aStarSettingChanged()
{
	pathfindingSys.AStarSettingChanged();
}

