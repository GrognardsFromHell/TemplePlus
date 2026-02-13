
#include "stdafx.h"
#include "pathfinding.h"
#include "location.h"
#include "combat.h"
#include "raycast.h"
#include "obj.h"
#include "critter.h"
#include "path_node.h"
#include "gamesystems/map/sector.h"
#include "objlist.h"
#include "config/config.h"
#include "party.h"
#include "python/python_integration_obj.h"
#include "python/python_object.h"

static constexpr int DontUseLength = std::numeric_limits<int>::min();
#define PATH_CACHE_EXPIRATION_TIME 5000 // miliseconds

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

	void Append(objHndl obj, float radius)
	{
		if (count >= 256)
		{
			logger->error("Error: Proximity list cap reached");
			assert(count < 256);
		}
		proxListObjs[count].obj = obj;
		proxListObjs[count].radius = radius;
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
						/*
						if (config.pathfindingDebugMode)
							logger->info("Pathfinding bump into critter: {} at location {}", proxListObjs[i].obj, proxListObjs[i].loc);
							*/
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
				if (!(objFlags & (OF_NO_BLOCK | OF_DONTDRAW | OF_OFF)  ))
				{
					if ((pq->flags & PQF_DOORS_ARE_BLOCKING) || (objType != obj_t_portal))
					{
						if (objType == obj_t_pc || objType == obj_t_npc)
						{
							if ( (pq->flags & PQF_IGNORE_CRITTERS) || critterSys.IsFriendly(obj, pqr->mover) || critterSys.IsDeadOrUnconscious(obj))
								continue;
						}

						if (!(pq->flags & PQF_AVOID_AOOS))
						Append(obj);
						else
						{
							auto args = PyTuple_New(2);
							PyTuple_SET_ITEM(args, 0, PyObjHndl_Create(pq->critter));
							PyTuple_SET_ITEM(args, 1, PyObjHndl_Create(obj));
							auto result = pythonObjIntegration.ExecuteScript("combat", "ShouldIgnoreTarget", args);
							//auto result2 = pythonObjIntegration.ExecuteScript("combat", "TargetClosest", args);
							int ignoreTarget = PyInt_AsLong(result);
							Py_DECREF(result);
							if (!ignoreTarget){
								if (critterSys.IsWieldingRangedWeapon(obj))
								{
									Append(obj);
								} else
								{
									float objReach = critterSys.GetReach(obj, D20A_UNSPECIFIED_ATTACK);
									float objRadius = objects.GetRadius(obj);
									Append(obj, objReach + objRadius);
								}
							} else
							{
								Append(obj);
							}
						}
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
	int(__cdecl*TruncatePathToDistance)(Path* path, LocAndOffsets* truncatedLoc, float truncateLengthFeet);
	int(__cdecl*GetPartialPath)(Path* path, Path* pathTrunc, float startDistFeet, float endDistFeet);
	PathFindAddresses()
	{
		rebase(TruncatePathToDistance,  0x10040200);
		rebase(PathDestIsClear,			0x10040C30);
		rebase(GetPartialPath, 0x10041630);
		rebase(FindPathShortDistanceAdjRadius, 0x10041E30);
		rebase(FindPathUsingNodes, 0x10042B50);
		rebase(_FindPath,              0x10043070);

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
	void apply() override 
	{
	//	 replaceFunction(0x10040520, _ShouldUsePathnodesUsercallWrapper); 
		 replaceFunction(0x10041730, _FindPathShortDistanceSansTarget);
		 replaceFunction(0x10043070, _FindPath);
		 replaceFunction(0x10042A00, _PathAstarInit);
	}
} pathFindingReplacements;

float Pathfinding::GetPathLength(Path* path)
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
	return pqr != nullptr && pqr >= pathQArray && pqr < &pathQArray[PQR_CACHE_SIZE];
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
	rebase(rollbackSequenceFlag,0x10B3D5C8); 
	rebase(pathQArray, 0x1186AC60);	

	pdbgMover = 0i64;
	pdbgGotPath = pdbgShortRangeError =  0;
	pdbgTargetObj = 0i64;
	pdbgNodeNum = 0;
	pdbgUsingNodes = false;
	pdbgAbortedSansNodes = false;
	pdbgDirectionsCount = 0;
}

int Pathfinding::PathCacheGet(PathQuery* pq, Path* pathOut)
{
	if (pathCacheCleared)
		return 0;

	for (int i = 0; i < PATH_RESULT_CACHE_SIZE; i++ )
	{
		

		auto pathCacheQ = &pathCache[i];

		if (!pathCacheQ->timeCached || pathCacheQ->timeCached < timeGetTime() - PATH_CACHE_EXPIRATION_TIME){
			continue;
		}

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
	pathCache[pathCacheIdx].path = *pqr;
	pathCache[pathCacheIdx].timeCached = timeGetTime();
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
	config.AddVanillaSetting("astar_max_window_ms", "5000", _aStarSettingChanged);
	config.AddVanillaSetting("astar_max_time_ms", "4000", _aStarSettingChanged);
	aStarMaxWindowMs = config.GetVanillaInt("astar_max_window_ms");
	aStarMaxTimeMs = config.GetVanillaInt("astar_max_time_ms");
	memset(aStarTimeElapsed, 0, sizeof(aStarTimeElapsed));
	memset(aStarTimeEnded, 0, sizeof(aStarTimeEnded));
	aStarTimeIdx = -1;
}

void Pathfinding::AStarSettingChanged()
{
	aStarMaxWindowMs = config.GetVanillaInt("astar_max_window_ms");
	aStarMaxTimeMs = config.GetVanillaInt("a_star_max_time_ms");
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
						&& !critterSys.IsDeadOrUnconscious(objIt.results[i].obj) )
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

int Pathfinding::PathDestIsClear(objHndl mover, LocAndOffsets* destLoc)
{

	RaycastPacket objIt;
	objIt.origin = *destLoc;
	objIt.targetLoc = *destLoc;

	*(int*)&objIt.flags |= (ExcludeItemObjects | StopAfterFirstBlockerFound | StopAfterFirstFlyoverFound);

	if (mover)
	{
		*(int*)&objIt.flags |= (HasSourceObj | HasRadius);
		objIt.sourceObj = mover;
		objIt.radius = objects.GetRadius(mover);
	}

	ObjectType objType;
	ObjectFlag objFlags;
	if (objIt.RaycastShortRange())
	{

		for (int i = 0; i < objIt.resultCount; i++)
		{
			if (!objIt.results[i].obj) // means it's a sector blocker
				return 0;

			objType = objects.GetType(objIt.results[i].obj);

			if (objType != obj_t_portal)
			{
				objFlags = (ObjectFlag)objects.GetFlags(objIt.results[i].obj);
				if (!(objFlags & OF_NO_BLOCK))
				{
					if ((objType == obj_t_pc || objType == obj_t_npc)
						&& !critterSys.IsDeadOrUnconscious(objIt.results[i].obj))
					{
						return 0;
					}
				}
			}
		}
	}
	return 1;
}

PathQueryResult* Pathfinding::FetchAvailablePQRCacheSlot()
{
	for (int i = 0; i < PQR_CACHE_SIZE; i++)
	{
		if (pathQArray[i].occupiedFlag == 0)
		{
			pathQArray[i].occupiedFlag = 1;
			return &pathQArray[i];
		}
	}
	return nullptr;
}

uint32_t Pathfinding::ShouldUsePathnodes(Path* pathQueryResult, PathQuery* pathQuery)
{
	bool result = false;
	LocAndOffsets from = pathQuery->from;
	if (!(pathQuery->flags & PQF_DONT_USE_PATHNODES))
	{
		if (false /*combatSys.isCombatActive()*/ )
		{
			if (locSys.distBtwnLocAndOffs(pathQueryResult->from, pathQueryResult->to) > (float)600.0)
			{
				return true;
			}
			
		}
		else {
			// check if the closest nodes (from/to) are neighbours; if so, return false so that it tries a direct path search first
			MapPathNode fromNode, toNode;
			int fromId, toId;
			pathNodeSys.FindClosestPathNode(&from, &fromId);
			pathNodeSys.FindClosestPathNode(&pathQueryResult->to, &toId);
			if (!pathNodeSys.GetPathNode(fromId, &fromNode) || !pathNodeSys.GetPathNode(toId, &toNode))
				return false;
			for (int i = 0; i < fromNode.neighboursCount; ++i) {
				auto neighbour = fromNode.neighbours[i];
				if (neighbour == toId)
					return false;
			}

			if (locSys.distBtwnLocAndOffs(pathQueryResult->from, pathQueryResult->to) > (float)600.0)
			{
				return true;
			}
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
			
			for (int i = 1; i <= 18; i++ )
			{
				auto iOff = i * 9.4280901f;
				for (int j = -i; j < i; j++)
				{
					auto jOff = j * 9.4280901f;
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

bool Pathfinding::TargetSurrounded(Path* pqr, PathQuery* pq)
{
	if (config.disableTargetSurrounded) return false;
	return TargetSurroundedCheck(pqr, pq);
}

bool Pathfinding::TargetSurroundedCheck(Path* pqr, PathQuery* pq)
{
	if (    (pq->flags & PQF_IGNORE_CRITTERS) 
		|| !(pq->flags & PQF_TARGET_OBJ ) 
		|| !(pq->flags & PQF_HAS_CRITTER)
		|| objects.IsPlayerControlled(pq->critter))
	{ // do this only in combat and only for paths with target critter
		return 0;
	}

	auto tgtObj = pq->targetObj;
	

	float overallOffX, overallOffY, maxDist = pq->tolRadius, minDist = pq->distanceToTargetMin;
	int maxSubtileDist = static_cast<int>(maxDist / (INCH_PER_TILE / 3)),
		minSubtileDist = static_cast<int>(minDist / (INCH_PER_TILE / 3)),
		minSubtileDistSqr = static_cast<int>(minDist / (INCH_PER_TILE / 3) * minDist / (INCH_PER_TILE / 3)),
		maxSubtileDistSqr = static_cast<int>(maxDist / (INCH_PER_TILE / 3) * maxDist / (INCH_PER_TILE / 3));
		
	LocAndOffsets tgtLoc, tweakedLoc;
	locSys.getLocAndOff(tgtObj, &tgtLoc);
	locSys.GetOverallOffset(tgtLoc, &overallOffX, &overallOffY);
	const float INCH_PER_SUBTILE = INCH_PER_TILE / 3;
	for (int i = 1; i <= maxSubtileDist; i++)
	{
		float iOff = i * INCH_PER_SUBTILE;
		for (int j = -i; j <= i; j++)
		{
			float jOff = j * INCH_PER_SUBTILE;

			int digitalDistSqr = (i * i + j * j);
			if (digitalDistSqr < minSubtileDistSqr
				|| digitalDistSqr > maxSubtileDistSqr)
				continue;
			tweakedLoc = tgtLoc;
			tweakedLoc.off_x += jOff;
			tweakedLoc.off_y -= iOff;
			locSys.RegularizeLoc(&tweakedLoc);
			if (PathDestIsClear(pq, pqr->mover, tweakedLoc))
			{
				return 0;
			}

			tweakedLoc = tgtLoc;
			tweakedLoc.off_x -= jOff;
			tweakedLoc.off_y += iOff;
			locSys.RegularizeLoc(&tweakedLoc);
			if (PathDestIsClear(pq, pqr->mover, tweakedLoc))
			{
				return 0;
			}

			tweakedLoc = tgtLoc;
			tweakedLoc.off_x += jOff;
			tweakedLoc.off_y += iOff;
			locSys.RegularizeLoc(&tweakedLoc);
			if (PathDestIsClear(pq, pqr->mover, tweakedLoc))
			{
				return 0;
			}

			tweakedLoc = tgtLoc;
			tweakedLoc.off_x -= jOff;
			tweakedLoc.off_y -= iOff;
			locSys.RegularizeLoc(&tweakedLoc);
			if (PathDestIsClear(pq, pqr->mover, tweakedLoc))
			{
				return 0;
			}
		}
	}
	return 1;

}


int Pathfinding::FindPathUsingNodes(PathQuery* pq, Path* path)
{
	if (config.pathfindingDebugMode)
	{
		logger->debug("Attempting PF using nodes");
	}
	PathQuery pathQueryLocal;
	Path pathLocal;

	pathQueryLocal = *pq;
	pathQueryLocal.to   = path->to;
	pathQueryLocal.from = path->from;
	pathQueryLocal.flags = (PathQueryFlags)(
		(uint32_t)pathQueryLocal.flags | PQF_ALLOW_ALTERNATIVE_TARGET_TILE | PQF_STRAIGHT_LINE_ONLY_FOR_SANS_NODE);
	PathInit(&pathLocal, &pathQueryLocal);
	pathQueryLocal.to   = path->to;
	pathQueryLocal.from = path->from;
	pathQueryLocal.flags = (PathQueryFlags)(
		(uint32_t)pathQueryLocal.flags  | PQF_ALLOW_ALTERNATIVE_TARGET_TILE | PQF_STRAIGHT_LINE_ONLY_FOR_SANS_NODE);
	pathLocal.from = path->from;
	pathLocal.to   = pathQueryLocal.to;


	int result = FindPathStraightLine(&pathLocal, &pathQueryLocal);
	if (result)	{
		pathLocal.nodeCount = 0;
		pathLocal.nodeCount2 = 0;
		pathLocal.nodeCount3 = 0;
		PathNodesAddByDirections(&pathLocal, pq);
		memcpy(path, &pathLocal, sizeof(Path));
		return path->nodeCount;
	}



	//auto from = path->from;
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

		
	if (config.pathfindingDebugMode)
	{
		pdbgUsingNodes = true;
		pdbgNodeNum = chainLength;
	}


	
	int i0 = 0;

	
	// Attempt straight line from 2nd last pathnode to destination (or general short PF if has clearance data)
	// If this is possible, shorten the path to avoid a zigzag going from the last node to destination
	if (chainLength>1)
	{
		MapPathNode node2ndLast, nodeLast;
		pathNodeSys.GetPathNode(nodeIds[chainLength - 2], &node2ndLast);
		pathNodeSys.GetPathNode(nodeIds[chainLength - 1], &nodeLast);
		if (! (pathQueryLocal.flags & PQF_TARGET_OBJ))
		{
			pathQueryLocal = * pq;
			pathQueryLocal.to = path->to;
			pathQueryLocal.from = node2ndLast.nodeLoc;
			pathQueryLocal.flags = (PathQueryFlags)(
				(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS | PQF_TARGET_OBJ)) 
				| PQF_ALLOW_ALTERNATIVE_TARGET_TILE | ((!pathNodeSys.hasClearanceData)* PQF_STRAIGHT_LINE_ONLY_FOR_SANS_NODE));
			PathInit(&pathLocal, &pathQueryLocal);
			pathQueryLocal.to = path->to;
			pathQueryLocal.from = node2ndLast.nodeLoc;
			pathQueryLocal.flags = (PathQueryFlags)(
				(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS | PQF_TARGET_OBJ))
				| PQF_ALLOW_ALTERNATIVE_TARGET_TILE | ( (!pathNodeSys.hasClearanceData)* PQF_STRAIGHT_LINE_ONLY_FOR_SANS_NODE));
			pathLocal.from = node2ndLast.nodeLoc;
			pathLocal.to = pathQueryLocal.to;
			int nodeCountAdded = FindPathSansNodes(&pathQueryLocal, &pathLocal);
			if (nodeCountAdded)
			{
				chainLength--;
			}
		} else
		{
			float distSecondLastToDestination = locSys.distBtwnLocAndOffs(path->to, node2ndLast.nodeLoc);
			if (distSecondLastToDestination < 400.0)
			{
				if (locSys.distBtwnLocAndOffs(nodeLast.nodeLoc, node2ndLast.nodeLoc) > distSecondLastToDestination)
					chainLength--;
			}
		}

	}

	// Attempt straight line from origin to 2nd node (or general short PF if has clearance data)
	// If this is possible, shorten the path
	// Note: The case where chainLength == 2 (initial) should have been already tested in principle... but we'll leave the check just in case
	if (chainLength > 1) {

		MapPathNode nodeFirst, nodeSecond;
		pathNodeSys.GetPathNode(nodeIds[0], &nodeFirst);
		pathNodeSys.GetPathNode(nodeIds[1], &nodeSecond);
		
		if (!(pathQueryLocal.flags & PQF_TARGET_OBJ))
		{
			pathQueryLocal = *pq;
			pathQueryLocal.from  = path->from;
			pathQueryLocal.to    = nodeSecond.nodeLoc;
			pathQueryLocal.flags = (PathQueryFlags)(
				(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS | PQF_TARGET_OBJ))
				| PQF_ALLOW_ALTERNATIVE_TARGET_TILE | ((!pathNodeSys.hasClearanceData) * PQF_STRAIGHT_LINE_ONLY_FOR_SANS_NODE));
			PathInit(&pathLocal, &pathQueryLocal);
			pathLocal.from = pathQueryLocal.from  = path->from;
			pathLocal.to   = pathQueryLocal.to    = nodeSecond.nodeLoc;
			pathQueryLocal.flags = (PathQueryFlags)(
				(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS | PQF_TARGET_OBJ))
				| PQF_ALLOW_ALTERNATIVE_TARGET_TILE | ((!pathNodeSys.hasClearanceData) * PQF_STRAIGHT_LINE_ONLY_FOR_SANS_NODE));
			
			int nodeCountAdded = FindPathSansNodes(&pathQueryLocal, &pathLocal);
			if (nodeCountAdded){
				/*memcpy(&nodeIds[0], &nodeIds[1], chainLength-1);
				chainLength--;*/
				i0 = 1;
			}
		}
		else {
			float distFromSecond = locSys.distBtwnLocAndOffs(path->from, nodeSecond.nodeLoc);
			if (distFromSecond < 614.0)
			{
				if (locSys.distBtwnLocAndOffs(nodeFirst.nodeLoc, nodeSecond.nodeLoc) > distFromSecond)
					i0 = 1;
			}
		}
		
	}

	// add paths from node to node
	bool destinationReached = false;
	auto curLoc = path->from;
	MapPathNode nodeTemp1;
	for (int i = i0; i < chainLength; i++)
	{
		// define the queries, init etc.
		{
			pathNodeSys.GetPathNode(nodeIds[i], &nodeTemp1);
			pathQueryLocal = *pq;
			pathQueryLocal.flags = (PathQueryFlags)(
				(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS | PQF_TARGET_OBJ)) | PQF_ALLOW_ALTERNATIVE_TARGET_TILE
				//(uint32_t)pathQueryLocal.flags | PQF_ALLOW_ALTERNATIVE_TARGET_TILE
				);
			pathQueryLocal.from = curLoc;
			pathQueryLocal.to   = nodeTemp1.nodeLoc;
		
			PathInit(&pathLocal, &pathQueryLocal);

			pathQueryLocal = *pq;
			pathQueryLocal.flags = (PathQueryFlags)(
					(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS | PQF_TARGET_OBJ)) | PQF_ALLOW_ALTERNATIVE_TARGET_TILE
				//(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS | PQF_TARGET_OBJ)) | PQF_ALLOW_ALTERNATIVE_TARGET_TILE
				);

			pathLocal.from = pathQueryLocal.from = curLoc;
			pathLocal.to   = pathQueryLocal.to   = nodeTemp1.nodeLoc;
			pathLocal.nodeCount = pathLocal.nodeCount2 = pathLocal.nodeCount3 = 0;
		
			if (!GetAlternativeTargetLocation(&pathLocal, &pathQueryLocal)) // verifies that the destination is clear, and if not, tries to get an available tile
			{
				logger->warn("Warning: pathnode not clear");
			}
			if (pathLocal.to != nodeTemp1.nodeLoc) //  path "To" location has been adjusted
			{
				nodeTemp1.nodeLoc = pathLocal.to;
			}
		}

		// attempt PF
		int nodeCountAdded = FindPathSansNodes(&pathQueryLocal, &pathLocal);
		if (!nodeCountAdded)
		{
			memcpy(&pathQueryLocal, pq, sizeof(PathQuery));
			pathQueryLocal.flags = (PathQueryFlags)(
				(uint32_t)pathQueryLocal.flags & (~(PQF_ADJUST_RADIUS )) | PQF_ALLOW_ALTERNATIVE_TARGET_TILE
				);
			pathLocal.from = pathQueryLocal.from = curLoc;
			pathLocal.to   = pathQueryLocal.to   = nodeTemp1.nodeLoc;
			pathLocal.nodeCount = 0;
			pathLocal.nodeCount2 = 0;
			pathLocal.nodeCount3 = 0;

			nodeCountAdded = FindPathSansNodes(&pathQueryLocal, &pathLocal);
		}
		if (!nodeCountAdded || (nodeCountAdded + nodeTotal > pq->maxShortPathFindLength))
		{
			return 0;
		}
			
		memcpy(&path->nodes[nodeTotal], pathLocal.nodes, sizeof(LocAndOffsets) * nodeCountAdded);
		nodeTotal += nodeCountAdded;
		curLoc = nodeTemp1.nodeLoc;

		if (i == chainLength - 2 && (pq->flags & PQF_TARGET_OBJ)) // the before last node - try bypassing and going directly to critter
		{
			pathQueryLocal = *pq;
			pathQueryLocal.from = curLoc;
			pathQueryLocal.to   = path->to;

			PathInit(&pathLocal, &pathQueryLocal);

			pathQueryLocal = *pq;
			pathLocal.from = pathQueryLocal.from = curLoc;
			pathLocal.to   = pathQueryLocal.to   = path->to;
			
			nodeCountAdded = FindPathSansNodes(&pathQueryLocal, &pathLocal);
			if (nodeCountAdded )
			{
				memcpy(&path->nodes[nodeTotal], pathLocal.nodes, sizeof(LocAndOffsets) * nodeCountAdded);
				nodeTotal += nodeCountAdded;
				path->nodeCount = nodeTotal;
				path->to = pathLocal.to;
				return nodeTotal;
	}
			if ( pathLocal.to == pathLocal.from)
			{
				path->nodes[path->nodeCount++] = pathLocal.to;
				path->to = pathLocal.to;
				return nodeTotal;
			}
		}
	}

	if (destinationReached)
	{
		return nodeTotal;
	}

	// now path from the last location (can be an adjusted path node) to the final destination
	pathQueryLocal = *pq;
	pathQueryLocal.from = curLoc;
	pathQueryLocal.to   = path->to;

	PathInit(&pathLocal, &pathQueryLocal);

	pathQueryLocal= * pq;
	pathLocal.from = pathQueryLocal.from = curLoc;
	pathLocal.to   = pathQueryLocal.to   = path->to;

	int nodeCountAdded = FindPathSansNodes(&pathQueryLocal, &pathLocal);
	if (   (!nodeCountAdded  && (pathLocal.to != pathLocal.from) )  // there's a possibility that the "from" is within reach, in which case the search will set the To same as From
		|| (nodeCountAdded + nodeTotal > pq->maxShortPathFindLength))
	{
		if (config.pathfindingDebugMode){
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
		
	auto deltaX = toAbsX - fromAbsX;
	auto deltaY = toAbsY - fromAbsY;
	auto distFromTo = sqrtf(deltaX * deltaX + deltaY * deltaY);
	if (distFromTo <= pq->tolRadius - 2.0f)
		return 0;
		
	auto adjustFactor = (distFromTo - (pq->tolRadius - 2.0f)) / distFromTo;
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
	// check if straight line succeeded
	if (pqFlags & PF_STRAIGHT_LINE_SUCCEEDED)
	{
		pqr->flags = pqr->flags - PF_STRAIGHT_LINE_SUCCEEDED;
		pqr->nodes[0] = pqr->to;
		pqr->nodeCount = 1;
		pqr->currentNode = 0;
		return;
	} 

	// else use the directions to build the nodes
	LocAndOffsets newNode, curNode;
		LocAndOffsets fromLoc = pqr->from;
	PathTempNodeAddByDirections(pqr->nodeCount3, pqr, &curNode);
	newNode = curNode;
		pqr->nodeCount = 0;
	int lastIdx = 0;
	ScreenDirections fromLocDirection = pqr->directions[0];
	bool directionChanged = false;

		for (int i = 2; i < pqr->nodeCount2; i++)
		{
		PathTempNodeAddByDirections(i, pqr, &curNode);
		if (fromLocDirection != pqr->directions[i-1])
		{
			directionChanged = true;
		}
		if (!PathStraightLineIsClear(pqr, pq, fromLoc, curNode))
		{
			// unable to go from the intermediate fromLoc to curNode
			// first, check if the direction has change. If it hasn't, it's not logical and may be due to the discrepancy in the PathStraightLineIsClear and the new clearance based method.
			if (directionChanged)
			{
				// get the previous node, which was the last "successful" one
				newNode = *PathTempNodeAddByDirections(i - 1, pqr, &newNode);
				fromLoc = newNode;
				// append it to nodes
				pqr->nodes[pqr->nodeCount++] = newNode;
				lastIdx = i - 1;
				fromLocDirection = pqr->directions[i - 1];
				directionChanged = false;
			} else
			{
				int dummy = 1;
			}
		}
		else
		{
			if (lastIdx == i-2)
			{
				int dummy = 1;
			}
			}
			
		}
		pqr->nodes[pqr->nodeCount++] = pqr->to;
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

	float requisiteClearance = objects.GetRadius(pq->critter, true);
	float diagonalClearance = requisiteClearance * 0.7f; // diagonals need to be more restrictive to avoid jaggy paths
	float requisiteClearanceCritters = requisiteClearance * 0.7f;
	if (requisiteClearance > 12)
		requisiteClearance *= 0.85f;

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
	if (config.pathfindingDebugMode)
	{
		logger->info("*** START OF PF ATTEMPT ADJ RADIUS - DESTINATION {} ***", pqr->to);
	}

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
				pathFindAstar[curIdx].length = DontUseLength;
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

			if (pathFindAstar[newIdx].length == DontUseLength)
				continue;

			locSys.SubtileToLocAndOff(_fromSubtile, &subPathFrom);
			locSys.SubtileToLocAndOff(shiftedSubtile, &subPathTo);
			
			int oldLength = pathFindAstar[newIdx].length;
			int newLength = pathFindAstar[refererIdx].length + 14 - 4 * (direction % 2); // +14 for diagonal, +10 for straight

			if (oldLength == 0 || abs(oldLength) > newLength)
			{
				// Check whether the shorter path would be valid
				if (PathNodeSys::hasClearanceData)
				{
					SectorLoc secLoc(subPathTo.location);
					//secLoc.GetFromLoc(subPathTo.location);
					int secX = (int)secLoc.x(), secY = (int)secLoc.y();
					int secClrIdx = PathNodeSys::clearanceData.clrIdx.clrAddr[secLoc.y()][secLoc.x()];
					auto secBaseTile = secLoc.GetBaseTile();
					int ssty = shiftedSubtile.y % 192;
					int sstx = shiftedSubtile.x % 192;
					if (direction % 2) // 
					{
						if (PathNodeSys::clearanceData.secClr[secClrIdx].val[shiftedSubtile.y % 192][shiftedSubtile.x % 192] < requisiteClearance)
						{
							/*if (config.pathfindingDebugMode)
							{
								logger->info("Pathfinding clearance too small:  {},  clearance value {}", subPathTo, PathNodeSys::clearanceData.secClr[secClrIdx].val[shiftedSubtile.y % 192][shiftedSubtile.x % 192]);
							}*/
							continue;
						}

					}
					else
					{
						if (PathNodeSys::clearanceData.secClr[secClrIdx].val[shiftedSubtile.y % 192][shiftedSubtile.x % 192] < diagonalClearance)
						{
							/*if (config.pathfindingDebugMode)
							{
								logger->info("Pathfinding clearance too small:  {},  clearance value {}", subPathTo, PathNodeSys::clearanceData.secClr[secClrIdx].val[shiftedSubtile.y % 192][shiftedSubtile.x % 192]);
							}*/
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
			if (config.pathfindingDebugMode) {
				logger->info("*** END OF PF ATTEMPT ADJ RADIUS - A* OPTIONS EXHAUSTED ***");
			}
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
			if (config.pathfindingDebugMode)
			{
				logger->info("Straight line succeeded.");
			}
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
	if (config.pathfindingDebugMode || !combatSys.isCombatActive())
	{
		
		pdbgUsingNodes = false;
		pdbgAbortedSansNodes = false;
		pdbgNodeNum = 0;
		pdbgGotPath = 0;
		if (pq->critter)
		{
			pdbgMover = pq->critter;
			logger->info("Starting path attempt for {}", pdbgMover);
		}
		pdbgFrom = pq->from;
		if ((pq->flags & PQF_TARGET_OBJ) && pq->targetObj )
		{
			pdbgTargetObj = pq->targetObj;
			pdbgTo = objects.GetLocationFull(pdbgTargetObj);
		}
			
		else
		{
			pdbgTargetObj = 0i64;
			pdbgTo = pqr->to;
		}
	}
	auto toSubtile = locSys.subtileFromLoc(&pqr->to);
	if (locSys.subtileFromLoc(&pqr->from) == toSubtile || !GetAlternativeTargetLocation(pqr, pq))
	{
		if (config.pathfindingDebugMode || !combatSys.isCombatActive())
		{
			if (locSys.subtileFromLoc(&pqr->from) == toSubtile)
				logger->info("Pathfinding: Aborting because from = to.");
			else
				logger->info("Pathfinding: Aborting because target tile is occupied and cannot find alternative tile.");
		}
		pdbgGotPath = 0;
		return 0;
	}
		

	if (TargetSurrounded(pqr, pq))
	{
		logger->info("Pathfinding: Aborting because target is surrounded.");
		return 0;
	}

	//if (!config.pathfindingDebugModeFlushCache )
	if (PathCacheGet(pq, pqr)){
		// has this query been done before? if so copies it and returns the result
		if (config.pathfindingDebugMode || !combatSys.isCombatActive())
			logger->info("Query found in cache, fetching result.");
			return pqr->nodeCount;
	}
		

	if (pq->flags & PQF_A_STAR_TIME_CAPPED && PathSumTime() >= aStarMaxTimeMs)
	{
		logger->info("Astar timed out, aborting.");
		pqr->flags |= PF_TIMED_OUT;
		return 0;
	}


	refTime = timeGetTime();
	if (ShouldUsePathnodes(pqr, pq))
	{
		if (config.pathfindingDebugMode || !combatSys.isCombatActive())
		{
			logger->info("Attempting using nodes...");
		}
		triedPathNodes = 1;
		gotPath = FindPathUsingNodes(pq, pqr);
		if ( config.pathfindingDebugMode || !combatSys.isCombatActive()){
			logger->info("Nodes attempt result: {}..." , gotPath);
		}
	} 
	else
	{
		if (config.pathfindingDebugMode || !combatSys.isCombatActive())
		{
			logger->info("Attempting sans nodes...");
		}
		gotPath = FindPathSansNodes(pq, pqr);
	}
		
	if (!gotPath)
	{
		if (!(pq->flags & PQF_DONT_USE_PATHNODES)  && !triedPathNodes)
		{
			if (config.pathfindingDebugMode || !combatSys.isCombatActive())
			{
				pdbgAbortedSansNodes = true;
				logger->info("Failed Sans Nodes attempt... trying nodes.");
			}
			gotPath = FindPathUsingNodes(pq, pqr);
			if (!gotPath)
			{
				int dummy = 1;
			}
		}
	}

	if (gotPath)
	{
		if (config.pathfindingDebugMode || !combatSys.isCombatActive())
		{
			if (pq->critter)
				logger->info("{} pathed successfully to {}", pq->critter, pqr->to);
		}
		
		pqr->flags |= PF_COMPLETE;
	} else
	{
		if (config.pathfindingDebugMode || !combatSys.isCombatActive())
		{
			logger->info("PF to {} failed!", pqr->to);
		}
	}
	PathCachePush(pq, pqr);
	PathRecordTimeElapsed(refTime);
	if (config.pathfindingDebugMode || !combatSys.isCombatActive())
	{
		pdbgGotPath = gotPath;
		pdbgTo = pqr->to;
	}
	return gotPath;

}

bool Pathfinding::CanPathTo(objHndl obj, objHndl target, PathQueryFlags flags, float maxDistanceFeet){
	auto from = objects.GetLocationFull(obj);
	
	auto partyMember = target;
	PathQueryResult path;
	memset(&path, 0, sizeof(PathQueryResult));
	PathQuery pathQ;
	pathQ.from = from;
	pathQ.flags = flags;
	auto reach = critterSys.GetReach(obj, D20A_UNSPECIFIED_ATTACK);
	pathQ.tolRadius = reach * 12.0f - ( INCH_PER_SUBTILE / 2 + 8.0f);
	pathQ.targetObj = target;
	if (config.pathfindingDebugMode)
		logger->info("PF attempt to party member: {}", target);
	pathQ.critter = obj;
	pathQ.distanceToTargetMin = reach;
	

	if (!FindPath(&pathQ, &path)){
		return false;
	}
	
	if (maxDistanceFeet > 0){
		auto pathDist = path.GetPathResultLength();
		if (pathDist > maxDistanceFeet)
			return false;

	}

	return true;
}

objHndl Pathfinding::CanPathToParty(objHndl obj, bool excludeUnconscious)
{
	if (party.IsInParty(obj))
		return objHndl::null;
	auto from = objects.GetLocationFull(obj);
	int partySize = party.GroupListGetLen();
	for (int i = 0; i < partySize; i++){

		auto partyMember = party.GroupListGetMemberN(i);
		if (excludeUnconscious && critterSys.IsDeadOrUnconscious(partyMember))
			continue;
		if (CanPathTo(obj, partyMember)){
			return partyMember;
		}		
	}
	return objHndl::null;
	//return addresses.canPathToParty(objHnd);
}

BOOL Pathfinding::PathStraightLineIsClear(Path* pqr, PathQuery* pq, LocAndOffsets from, LocAndOffsets to)
{
	BOOL result = 1;
	RaycastPacket objIt;
	objIt.origin = from;
	objIt.targetLoc = to;
	auto dx = abs(static_cast<int>(to.location.locx - from.location.locx));
	auto dy = abs(static_cast<int>(to.location.locy - from.location.locy));

	if ( max( dx, dy) >= SECTOR_SIDE_SIZE * 3 - 1 || (dx + dy  >= 5 * SECTOR_SIDE_SIZE-1)) // RayCast supports up to a span of 4 sectors
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
				if ( (pathFlags & PQF_DOORS_ARE_BLOCKING) || objType != obj_t_portal)
				{
					if (objType != obj_t_pc && objType != obj_t_npc)
						break;
					if ( !(pathFlags & PQF_IGNORE_CRITTERS) && !critterSys.IsDeadOrUnconscious(objIt.results[i].obj))
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
				pathFindAstar[curIdx].length = DontUseLength;
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

			if (pathFindAstar[newIdx].length == DontUseLength)
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
			if (npcPathFindRefTime && (timeGetTime() - npcPathFindRefTime) < 1000  ) // && !pathNodeSys.hasClearanceData limits the number of attempt to 10 per second and cumulative time to 250 sec
			{
				attemptCount = npcPathFindAttemptCount;
				if ( (npcPathFindAttemptCount > 10 + (40 * (pathNodeSys.hasClearanceData == true))  ) || npcPathTimeCumulative > 250)
				{
					if (config.pathfindingDebugMode)
					{
						pdbgDirectionsCount = 0;
						if (npcPathFindAttemptCount > 10 + (40 * (pathNodeSys.hasClearanceData == true)))
						{
							logger->info("NPC pathing attempt count exceeded, aborting.");
							pdbgShortRangeError = -(10 + (40 * (pathNodeSys.hasClearanceData == true)));
						}
						else
				{
							pdbgShortRangeError = -250;
							logger->info("NPC pathing cumulative time exceeded, aborting.");
						}
							
					}
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
	{
		pdbgDirectionsCount = 0;
		pdbgShortRangeError = gridSize;
		logger->info("Desitnation too far for short distance PF grid! Aborting.");
		return 0;
	}
		
	
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

	float requisiteClearance = 1.0f;
	if (pq->critter)
		requisiteClearance = objects.GetRadius(pq->critter, true);
	float diagonalClearance = requisiteClearance * 0.7f;
	float requisiteClearanceCritters = requisiteClearance * 0.7f;
	if (requisiteClearance > 12)
		requisiteClearance *= 0.85f;




	if (curIdx == -1)
	{
		if (referenceTime)
			npcPathTimeCumulative += timeGetTime() - referenceTime;
		if (config.pathfindingDebugMode)
		{
			pdbgDirectionsCount = 0;
			pdbgShortRangeError = -1;
			logger->info("curIdx is -1, aborting.");
		}
		return 0;
	}



	struct ProximityList proxList;
	proxList.Populate(pq, pqr, INCH_PER_TILE * 40);

#pragma endregion
	if (config.pathfindingDebugMode)
	{
		logger->info("*** START OF PF ATTEMPT SANS TARGET - DESTINATION {} ***", pqr->to);
		pdbgDirectionsCount = 0;
		pdbgShortRangeError = 0;
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
				pathFindAstar[curIdx].length = DontUseLength;
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
				pdbgShortRangeError = -999;
				logger->info("*** END OF PF ATTEMPT SANS TARGET - OPEN SET EMPTY; from {} to {} ***", pqr->from, pqr->to);
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

			if (pathFindAstar[newIdx].length == DontUseLength)
				continue;

			LocAndOffsets subPathFrom;
			LocAndOffsets subPathTo;
			locSys.SubtileToLocAndOff(_fromSubtile, &subPathFrom);
			locSys.SubtileToLocAndOff(shiftedSubtile, &subPathTo);
			
			int oldLength = pathFindAstar[newIdx].length;
			int newLength = pathFindAstar[refererIdx].length + 14 - 4 * (direction % 2); // +14 for diagonal, +10 for straight

			if (oldLength == 0 || abs(oldLength) > newLength)
			{
				// Check whether the shorter path would be valid.
				if (PathNodeSys::hasClearanceData)
				{
					SectorLoc secLoc(subPathTo.location);
					//secLoc.GetFromLoc(subPathTo.location);
					int secX = (int)secLoc.x(), secY = (int)secLoc.y();
					int secClrIdx = PathNodeSys::clearanceData.clrIdx.clrAddr[secLoc.y()][secLoc.x()];
					auto secBaseTile = secLoc.GetBaseTile();
					int ssty = shiftedSubtile.y % 192;
					int sstx = shiftedSubtile.x % 192;
					if (direction % 2) // xy straight
					{
						if (PathNodeSys::clearanceData.secClr[secClrIdx].val[ssty][sstx] < requisiteClearance){
							//if (config.pathfindingDebugMode)
							//{
							//	//logger->info("Pathfinding clearance too small:  {},  clearance value {}", subPathTo, PathNodeSys::clearanceData.secClr[secClrIdx].val[shiftedSubtile.y % 192][shiftedSubtile.x % 192]);
							//}
							continue;
						}
					}
					else // xy diagonal
					{
						if (PathNodeSys::clearanceData.secClr[secClrIdx].val[ssty][sstx] < diagonalClearance)
						{
							/*if (config.pathfindingDebugMode)
							{
								logger->info("Pathfinding clearance too small:  {},  clearance value {}", subPathTo, PathNodeSys::clearanceData.secClr[secClrIdx].val[shiftedSubtile.y % 192][shiftedSubtile.x % 192]);
							}*/
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
				logger->info("*** END OF PF ATTEMPT SANS TARGET - A* OPTIONS EXHAUSTED; from {} to {} ***", pqr->from, pqr->to);
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
		pdbgDirectionsCount = directionsCount;
		pdbgShortRangeError = 0;
	}


	return directionsCount;
}

int Pathfinding::GetPartialPath(Path* path, Path* pathTrunc, float startDistFeet, float endDistFeet)
{
	return addresses.GetPartialPath(path, pathTrunc, startDistFeet, endDistFeet);
}

int Pathfinding::TruncatePathToDistance(Path* path, LocAndOffsets* truncatedLoc, float truncateLengthFeet)
{
	return addresses.TruncatePathToDistance(path, truncatedLoc, truncateLengthFeet);
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

float Path::GetPathResultLength(){
	return pathfindingSys.GetPathLength(this);
}

// Originally @ 0x1003ff30
std::optional<LocAndOffsets> Path::GetNextNode() const
{
	if (!IsComplete()) {
		return {};
	}
	
	if (flags & PF_STRAIGHT_LINE_SUCCEEDED) {
		return to;
	} else {
		if (currentNode + 1 < nodeCount) {
			return nodes[currentNode];
		}

		return to;
	}
}
