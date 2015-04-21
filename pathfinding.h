#pragma once

#include "util/addresses.h"
#include "temple_functions.h"

#define pfCacheSize  0x20


struct LocationSys;

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

	PQF_UNK1 = 4,
	PQF_UNK2 = 8, // appears to indicate a straight line path from->to
	/*
		Indicates that the query is to move to a target object.
	*/
	PQF_TARGET_OBJ = 0x10,
	/*
		Indicates that the destination should be adjusted for the critter and target
		radius.
	*/
	PQF_ADJUST_RADIUS = 0x20,

	// Could mean "USE TIME LIMIT" ? or it could be "continue searching"?
	PQF_UNK3 = 0x80000 // it is set when the D20 action has the flag D20CAF_TRUNCATED
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

struct Pathfinding : AddressTable {

	LocationSys * loc;

	float pathLength(Path *path); // note: unlike the ToEE function, returns a float (and NOT to the FPU!)
	bool pathQueryResultIsValid(PathQueryResult *pqr);

	Pathfinding();

	bool (__cdecl *FindPath)(PathQuery *query, PathQueryResult *result);
	void (__cdecl *ToEEpathDistBtwnToAndFrom)(Path *path); // outputs to FPU (st0);  apparently distance in feet (since it divides by 12)
	objHndl(__cdecl * canPathToParty)(objHndl objHnd);
	PathQueryResult * pathQArray;
	uint32_t * pathSthgFlag_10B3D5C8;

} ;

extern Pathfinding pathfindingSys;