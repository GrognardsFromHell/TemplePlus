#pragma once

#include <temple/dll.h>
#include "temple_functions.h"

#define pfCacheSize  0x20


struct LocationSys;

struct FindPathNodeData
{
	int nodeId;
	int refererId; // for the From node is -1; together with the "distCumul = -1" nodes will form a chain leading From -> To
	int distFrom; // distance to the From node; is set to 0 for the from node naturally :)
	float distTo; // distance to the To node;
	float distCumul; // can be set to -1
};


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
		if not set, then sets field28 to 200 initially
	*/
	PQF_UNK1 = 4,


	PQF_UNK2 = 8, // appears to indicate a straight line path from->to
	
	PQF_10 = 0x10,
	PQF_20 = 0x20,
	PQF_40 = 0x40,
	PQF_80 = 0x80, // path (pass) through critters apparently (maybe out of combat?)
	PQF_100 = 0x100,
	PQF_200 = 0x200,
	PQF_400 = 0x400, // something to do with the type of object
	PQF_800 = 0x800,


	/*
	Indicates that the query is to move to a target object.
	WAS ERRONEOUSLY LISTED AS 0x10  (look out for those BYTE1() operators DS!)
	*/
	PQF_TARGET_OBJ = 0x1000,

	/*
	Indicates that the destination should be adjusted for the critter and target
	radius.
	WAS ERRONEOUSLY LISTED AS 0x20  (look out for those BYTE1() operators DS!)
	*/
	PQF_ADJUST_RADIUS = 0x2000,

	PQF_UNKNOWN4000h = 0x4000,

	// Appears to mean that pathfinding should obey the time limit
	PQF_A_STAR_TIME_CAPPED = 0x80000 // it is set when the D20 action has the flag D20CAF_TRUNCATED
};

#pragma pack(push, 1)
struct PathQuery {
	PathQueryFlags flags; 
	int field_4;
	LocAndOffsets from;
	LocAndOffsets to;
	int field28;
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
	PF_UNK1 = 0x10, // Seems to be set in response to query flag 0x80000
};

struct Path {
	int flags;
	int field4;
	LocAndOffsets from;
	LocAndOffsets to;
	objHndl mover;
	int field30[50];
	int nodeCount3;
	int initTo1;
	int field100[800];
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

const uint32_t TestSizeofPathQueryResult = sizeof(PathQueryResult); // should be 6688 (0x1A20)

#pragma pack(pop)

struct Pathfinding : temple::AddressTable {

	LocationSys * loc;

	float pathLength(Path *path); // note: unlike the ToEE function, returns a float (and NOT to the FPU!)
	bool pathQueryResultIsValid(PathQueryResult *pqr);

	Pathfinding();
	uint32_t ShouldUsePathnodes(PathQueryResult* pathQueryResult, PathQuery* pathQuery);
	int(__cdecl* PopMinDist2Node)(FindPathNodeData* fpndOut); // output is 1 on success 0 on fail (if all nodes are negative distance)
	int(__cdecl*PathDestIsClear)(PathQuery* pq, objHndl mover, LocAndOffsets destLoc); // checks if there's anything blocking the destination location (taking into account the mover's radius)
	int(__cdecl*FindPathBetweenNodes)(int fromNodeId, int toNodeId, void*, int maxChainLength); // finds the node IDs for the To -> .. -> From course (locally optimal I think? Is this A*?); return value is chain length
	bool (__cdecl *FindPath)(PathQuery *query, PathQueryResult *result);
	void (__cdecl *ToEEpathDistBtwnToAndFrom)(Path *path); // outputs to FPU (st0);  apparently distance in feet (since it divides by 12)
	objHndl(__cdecl * canPathToParty)(objHndl objHnd);
	PathQueryResult * pathQArray;
	uint32_t * pathSthgFlag_10B3D5C8;

} ;

extern Pathfinding pathfindingSys;



uint32_t _ShouldUsePathnodesUsercallWrapper();