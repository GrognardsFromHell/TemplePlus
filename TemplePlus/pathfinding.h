#pragma once

#include <temple/dll.h>
#include "temple_functions.h"

#define pfCacheSize  0x20
#define PATH_RESULT_CACHE_SIZE 0x28 // different from the above

#pragma region structs
#include "game_config.h"
//



struct LocationSys;
class PathNodeSys;
struct MapPathNode;
struct MapPathNodeList;




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


	PQF_UNK2 = 8, // appears to indicate a straight line path from->to

	PQF_10 = 0x10,
	PQF_20 = 0x20,
	PQF_40 = 0x40,
	PQF_IGNORE_CRITTERS = 0x80, // path (pass) through critters (flag is set when pathing out of combat)
	PQF_100 = 0x100,
	PQF_200 = 0x200,
	PQF_DOORS_ARE_BLOCKING = 0x400, // if set, it will consider doors to block the path
	PQF_800 = 0x800,


	/*
	Indicates that the query is to move to a target object.
	WAS ERRONEOUSLY LISTED AS 0x10  (look out for those BYTE1() operators DS!)
	*/
	PQF_TARGET_OBJ = 0x1000,

	/*
	Indicates that the destination should be adjusted for the critter and target
	radius.
	makes PathInit add the radii of the targets to fields tolRadius and distanceToTarget
	*/
	PQF_ADJUST_RADIUS = 0x2000,

	PQF_DONT_USE_PATHNODES = 0x4000,
	PQF_DONT_USE_STRAIGHT_LINE = 0x8000,
	PQF_FORCED_STRAIGHT_LINE =  0x10000,
	PQF_UNKNOWN20000h = 0x20000,
	/*
		if the target destination is not cleared, and PQF_ADJUST_RADIUS is off, 
		it will search in a 5x5 tile neighbourgood around the original target tile 
		for a clear tile (i.e. one that the critter can fit in without colliding with anything)
	*/
	PQF_ALLOW_ALTERNATIVE_TARGET_TILE = 0x40000,

	// Appears to mean that pathfinding should obey the time limit
	PQF_A_STAR_TIME_CAPPED =			  0x80000, // it is set when the D20 action has the flag D20CAF_TRUNCATED
	PQF_IGNORE_CRITTERS_ON_DESTINATION = 0x800000 // NEW! makes it ignored critters on the PathDestIsClear function

	// used in practice for unspecified move:
	// with target critter
	// pathQ.flags = (PathQueryFlags)0x23803;
	//	PQF_TO_EXACT = 1
	//  PQF_HAS_CRITTER = 1
	//  PQF_MAX_PF_LENGTH_STHG = 0 // (0x4) if not set, then sets maxShortPathFindLength to 200 initially
	//	PQF_UNK2 = 0 // appears to indicate a straight line path from->to
	/*
	PQF_10 = 0
	PQF_20 = 0
	PQF_40 = 0,
	PQF_IGNORE_CRITTERS = 0 
	PQF_100 = 0,
	PQF_200 = 0,
	PQF_DOORS_ARE_BLOCKING = 0, // something to do with the type of object
	PQF_800 = 1,
	*/

	/*
	Indicates that the query is to move to a target object.
	WAS ERRONEOUSLY LISTED AS 0x10  (look out for those BYTE1() operators DS!)
	*/
	// PQF_TARGET_OBJ = 1,

	/*
	Indicates that the destination should be adjusted for the critter and target
	radius.
	WAS ERRONEOUSLY LISTED AS 0x20  (look out for those BYTE1() operators DS!)
	*/
	// PQF_ADJUST_RADIUS = 1

	// PQF_DONT_USE_PATHNODES = 0,

	// Appears to mean that pathfinding should obey the time limit
	// PQF_A_STAR_TIME_CAPPED = ? // it is set when the D20 action has the flag D20CAF_TRUNCATED (or D20CAF_UNNECESSARY???)


	// WITHOUT TARGET CRITTER:
	// pathQ.flags = static_cast<PathQueryFlags>(0x40803);
	// PQF_TARGET_OBJ = 0
	// PQF_ADJUST_RADIUS = 0
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
	float distanceToTarget; // Related to the targetObj's radius
	float tolRadius; // Tolerance (How far away from the exact destination you are allowed to be)
	int flags2;
	int field_4c;

	PathQuery() {
		memset(this, 0, sizeof(PathQuery));
	}
};

const uint32_t TestSizeofPathQuery = sizeof(PathQuery); // should be 80 (0x50)

enum PathFlags {
	PF_COMPLETE = 0x1, // Seems to indicate that the path is complete (or valid?)
	PF_2 = 0x2,
	PF_4 = 0x4,
	PF_STRAIGHT_LINE_SUCCEEDED = 0x8, // straight line succeeded perhaps?
	PF_UNK1 = 0x10, // Seems to be set in response to query flag 0x80000
	PF_20 = 0x20
};

struct Path {
	int flags;
	int field4;
	LocAndOffsets from;
	LocAndOffsets to;
	objHndl mover;
	char directions[200];
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
};

struct PathQueryResult : Path {
	int occupiedFlag;
	int someDelay;
};

struct PathResultCache : PathQuery
{
	Path path;
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


	PathQueryResult * pathQArray;
	uint32_t * pathSthgFlag_10B3D5C8;

	float pathLength(Path *path); // path length in feet; note: unlike the ToEE function, returns a float (and NOT to the FPU!)
	bool pathQueryResultIsValid(PathQueryResult *pqr);

	Pathfinding();

	int PathCacheGet(PathQuery * pq, Path* path);
	void PathCachePush(PathQuery* pq, Path* pqr);
	int PathSumTime();
	void PathRecordTimeElapsed(int time);

	void PathAstarInit();
	void AStarSettingChanged();
	void PathCacheInit();

	int PathDestIsClear(PathQuery* pq, objHndl mover, LocAndOffsets destLoc); // checks if there's anything blocking the destination location (taking into account the mover's radius)
	int(__cdecl*FindPathBetweenNodes)(int fromNodeId, int toNodeId, void*, int maxChainLength); // finds the node IDs for the To -> .. -> From course (locally optimal I think? Is this A*?); return value is chain length


	bool GetAlternativeTargetLocation(PathQueryResult* pqr, PathQuery* pq);



	int FindPath(PathQuery* pq, PathQueryResult* result);
	void (__cdecl *ToEEpathDistBtwnToAndFrom)(Path *path); // outputs to FPU (st0);  apparently distance in feet (since it divides by 12)

	objHndl canPathToParty(objHndl objHnd);
	BOOL PathStraightLineIsClear(Path* pqr, PathQuery* pq, LocAndOffsets subPathFrom, LocAndOffsets subPathTo); // including static obstacles it seems
	BOOL PathStraightLineIsClearOfStaticObstacles(Path* pqr, PathQuery* pq, LocAndOffsets subPathFrom, LocAndOffsets subPathTo);
	int GetDirection(int a1, int a2, int a3);
	int FindPathShortDistanceSansTarget(PathQuery * pq, Path* pqr);


protected:
	void PathInit(Path* pqr, PathQuery* pq);

	uint32_t ShouldUsePathnodes(Path* pathQueryResult, PathQuery* pathQuery);
	int FindPathUsingNodes(PathQuery* pq, Path* pqr);
	int FindPathStraightLine(Path* pqr, PathQuery* pq);
	LocAndOffsets * PathTempNodeAddByDirections(int idx, Path* pqr, LocAndOffsets* newNode);
	void PathNodesAddByDirections(Path* pqr, PathQuery* pq);
	int FindPathShortDistanceAdjRadius(PathQuery* pq, Path* pqr);
	int FindPathForcecdStraightLine(Path* pqr, PathQuery* pq);
	int FindPathSansNodes(PathQuery* pq, Path* pqr);
} ;

extern Pathfinding pathfindingSys;


// hooks
//uint32_t _ShouldUsePathnodesUsercallWrapper();
int _FindPathShortDistanceSansTarget(PathQuery * pq, PathQueryResult * pqr);
int _FindPath(PathQuery* pq, PathQueryResult* pqr);
void _PathAstarInit();
void _aStarSettingChanged();