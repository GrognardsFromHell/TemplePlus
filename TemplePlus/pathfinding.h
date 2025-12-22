#pragma once

#include <optional>

#include <temple/dll.h>
#include "temple_functions.h"

#define PQR_CACHE_SIZE  0x20 // cache used for storing paths for specific action sequences
#define PATH_RESULT_CACHE_SIZE 0x28 // different from the above, used for storing past results to minimize PF time

#pragma region structs

struct LocationSys;
class PathNodeSys;
struct MapPathNode;
struct MapPathNodeList;
struct ProximityList;

enum PathQueryFlags : uint32_t {
	/*
		The pathfinder seems to ignore offset x and y of the destination if this flag
		is not set.
	*/
	PQF_TO_EXACT = 1,
	/*
		Indicates that the query is on behalf of a critter and the critter is set.
	*/
	PQF_HAS_CRITTER = 2,

	/*
		if not set, then sets maxShortPathFindLength to 200 initially
	*/
	PQF_MAX_PF_LENGTH_STHG = 4,


	PQF_STRAIGHT_LINE = 8, // appears to indicate a straight line path from->to

	PQF_10 = 0x10,
	PQF_20 = 0x20,
	PQF_40 = 0x40,
	PQF_IGNORE_CRITTERS = 0x80, // path (pass) through critters (flag is set when pathing out of combat)
	PQF_100 = 0x100,
	PQF_STRAIGHT_LINE_ONLY_FOR_SANS_NODE = 0x200,
	PQF_DOORS_ARE_BLOCKING = 0x400, // if set, it will consider doors to block the path
	PQF_800 = 0x800,
	PQF_TARGET_OBJ = 0x1000, // Indicates that the query is to move to a target object.

	/*
	Indicates that the destination should be adjusted for the critter and target
	radius.
	makes PathInit add the radii of the targets to fields tolRadius and distanceToTargetMin
	*/
	PQF_ADJUST_RADIUS = 0x2000,

	PQF_DONT_USE_PATHNODES = 0x4000,
	PQF_DONT_USE_STRAIGHT_LINE = 0x8000,
	PQF_FORCED_STRAIGHT_LINE = 0x10000,
	PQF_ADJ_RADIUS_REQUIRE_LOS = 0x20000,
	/*
		if the target destination is not cleared, and PQF_ADJUST_RADIUS is off,
		it will search in a 5x5 tile neighbourgood around the original target tile
		for a clear tile (i.e. one that the critter can fit in without colliding with anything)
	*/
	PQF_ALLOW_ALTERNATIVE_TARGET_TILE = 0x40000,

	// Appears to mean that pathfinding should obey the time limit
	PQF_A_STAR_TIME_CAPPED = 0x80000, // it is set when the D20 action has the flag D20CAF_UNNECESSARY

	PQF_IGNORE_CRITTERS_ON_DESTINATION = 0x800000, // NEW! makes it ignored critters on the PathDestIsClear function
	PQF_AVOID_AOOS = 0x1000000 // NEW! Make the PF attempt avoid Aoos (using the ShouldIgnore function in combat.py to ignore insiginificant threats)
};

#pragma pack(push, 1)
struct PathQuery {
	PathQueryFlags flags; 
	int field_4;
	LocAndOffsets from;
	LocAndOffsets to;
	int maxShortPathFindLength;
	int field2c;
	objHndl critter;  // Set PQF_HAS_CRITTER
	objHndl targetObj; // Set PQF_TARGET_OBJ
	/*
	 When ADJ_RADIUS is set, (usually when there's a TARGET_OBJ)
	 this is the minimum distance required. Should be equal
	 to the sum of radii of critter + target.
	*/
	float distanceToTargetMin; 
	float tolRadius; // Tolerance (How far away from the exact destination you are allowed to be)
	int flags2;
	int field_4c;

	PathQuery() {
		memset(this, 0, sizeof(PathQuery));
	}
};

const uint32_t TestSizeofPathQuery = sizeof(PathQuery); // should be 80 (0x50)

enum PathFlags {
	PF_NONE = 0,
	PF_COMPLETE = 0x1, // Seems to indicate that the path is complete (or valid?)
	PF_2 = 0x2,
	PF_4 = 0x4,
	PF_STRAIGHT_LINE_SUCCEEDED = 0x8, 
	PF_TIMED_OUT = 0x10, // Seems to be set in response to query flag PQF_A_STAR_TIME_CAPPED (maybe timeout flag?)
	PF_20 = 0x20
};

struct Path {
	int flags; // see PathFlags
	int field4;
	LocAndOffsets from;
	LocAndOffsets to;
	objHndl mover;
	ScreenDirections directions[200];
	int nodeCount3;
	int initTo1;
	LocAndOffsets tempNodes[200];
	int nodeCount2;
	int fieldd84;
	LocAndOffsets nodes[200];
	int nodeCount;
	int currentNode;
	int field_1a10;
	int field_1a14;

	float GetPathResultLength();

	bool IsComplete() const {
		return (flags & PF_COMPLETE) == PF_COMPLETE;
	}

	// Returns the next node along this path, or empty in case the path is not completely built
	std::optional<LocAndOffsets> GetNextNode() const;

	Path() {
		memset(this, 0, sizeof(Path));
	}
};

struct PathQueryResult : Path {
	// Sometimes, a pointer to the following two values is passed as "pPauseTime" (see 100131F0)
	int occupiedFlag;
	int someDelay;

	PathQueryResult() {
		memset(this, 0, sizeof(PathQueryResult));
	}
};

struct PathResultCache : PathQuery
{
	Path path;
	uint32_t timeCached =0;
};


const uint32_t TestSizeofPathQueryResult = sizeof(PathQueryResult); // should be 6688 (0x1A20)

#pragma pack(pop)

#pragma endregion

struct Pathfinding : temple::AddressTable {

	LocationSys * loc;
	PathNodeSys * pathNode;

	PathResultCache pathCache[PATH_RESULT_CACHE_SIZE]; // used as a ring buffer
	int pathCacheIdx;
	int pathCacheCleared;
	
	int aStarMaxTimeMs;
	int aStarMaxWindowMs;
	int aStarTimeIdx;
	int aStarTimeElapsed[20]; // array 20
	int aStarTimeEnded[20]; //  array 20

#pragma region Debug stuff for diagnostic render
	LocAndOffsets pdbgFrom, pdbgTo;
	objHndl pdbgMover, pdbgTargetObj;
	int pdbgGotPath, pdbgShortRangeError;
	bool pdbgUsingNodes, pdbgAbortedSansNodes;
	int pdbgNodeNum;
	int pdbgDirectionsCount;
#pragma endregion

	PathQueryResult * pathQArray;
	bool pathQueryResultIsValid(PathQueryResult *pqr);
	PathQueryResult* FetchAvailablePQRCacheSlot();
	uint32_t * rollbackSequenceFlag;

	float GetPathLength(Path *path); // path length in feet; note: unlike the ToEE function, returns a float (and NOT to the FPU!)


	Pathfinding();

	int PathCacheGet(PathQuery * pq, Path* path);
	void PathCachePush(PathQuery* pq, Path* pqr);
	int PathSumTime();
	void PathRecordTimeElapsed(int time);

	void PathAstarInit();
	void AStarSettingChanged();
	void PathCacheInit();

	int PathDestIsClear(PathQuery* pq, objHndl mover, LocAndOffsets destLoc); // checks if there's anything blocking the destination location (taking into account the mover's radius)
	int PathDestIsClear(objHndl mover, LocAndOffsets* destLoc); // simpler version without the path query flags
	
	int(__cdecl*FindPathBetweenNodes)(int fromNodeId, int toNodeId, void*, int maxChainLength); // finds the node IDs for the To -> .. -> From course (locally optimal I think? Is this A*?); return value is chain length

	/*
		if target tile is not clear (blocked, or not enough space for the obj), find another one
		irrelevant for ADJ_RADIUS
	*/
	bool GetAlternativeTargetLocation(Path* pqr, PathQuery* pq);
	/*
		For ADJ_RADIUS: test if the target is surrounded and thus unreachable.
		Use in combat only.
	*/
	bool TargetSurrounded(Path* pqr, PathQuery* pq);
	bool TargetSurroundedCheck(Path* pqr, PathQuery* pq);

	int FindPath(PathQuery* pq, PathQueryResult* result);
	

	bool CanPathTo(objHndl handle, objHndl target, PathQueryFlags flags = static_cast<PathQueryFlags>(PQF_HAS_CRITTER | PQF_TO_EXACT | PQF_800 | PQF_ADJ_RADIUS_REQUIRE_LOS | PQF_ADJUST_RADIUS | PQF_TARGET_OBJ), float maxDistance = -1);
	objHndl CanPathToParty(objHndl objHnd, bool excludeUnconscious = true);
	BOOL PathStraightLineIsClear(Path* pqr, PathQuery* pq, LocAndOffsets subPathFrom, LocAndOffsets subPathTo); // including static obstacles it seems
	BOOL PathAdjRadiusLosClear(Path* pqr, PathQuery* pq, LocAndOffsets subPathFrom, LocAndOffsets subPathTo);
	ScreenDirections GetDirection(int a1, int a2, int a3);
	int FindPathShortDistanceSansTargetLegacy(PathQuery * pq, Path* pqr);
	int FindPathShortDistanceSansTarget(PathQuery * pq, Path* pqr);

	/*
		gets a partial path (based on path) starting from startDistFeet to endDistFeet and writes it to pathTrunc
	*/
	int GetPartialPath(Path* path, Path* pathTrunc, float startDistFeet, float endDistFeet);
	int TruncatePathToDistance(Path* path, LocAndOffsets* truncatedLoc, float truncateLengthFeet);

	void PathInit(Path* pqr, PathQuery* pq);
protected:
	

	uint32_t ShouldUsePathnodes(Path* pathQueryResult, PathQuery* pathQuery);
	int FindPathUsingNodes(PathQuery* pq, Path* pqr);
	int FindPathStraightLine(Path* pqr, PathQuery* pq);
	
	/*
		if idx is empty, this function will fill the slot with the location defined by the cumulative result of the "direction" steps.
		whether it appends or not, it returns the tempnode in that idx.
	*/
	LocAndOffsets * PathTempNodeAddByDirections(int idx, Path* pqr, LocAndOffsets* newNode);
	void PathNodesAddByDirections(Path* pqr, PathQuery* pq);
	int FindPathShortDistanceAdjRadius(PathQuery* pq, Path* pqr);
	int FindPathForcecdStraightLine(Path* pqr, PathQuery* pq);
	int FindPathSansNodes(PathQuery* pq, Path* pqr);

	void(__cdecl *ToEEpathDistBtwnToAndFrom)(Path *path); // outputs to FPU (st0);  apparently distance in feet (since it divides by 12)
} ;

extern Pathfinding pathfindingSys;


// hooks
//uint32_t _ShouldUsePathnodesUsercallWrapper();
int _FindPathShortDistanceSansTarget(PathQuery * pq, PathQueryResult * pqr);
int _FindPath(PathQuery* pq, PathQueryResult* pqr);
void _PathAstarInit();
void _aStarSettingChanged();



