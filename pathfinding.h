#pragma once

#include "util/addresses.h"
#include "temple_functions.h"

#define pfCacheSize  0x20

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
		Indicates that the query is to move to a target object.
	*/
	PQF_TARGET_OBJ = 0x10,
	/*
		Indicates that the destination should be adjusted for the critter and target
		radius.
	*/
	PQF_ADJUST_RADIUS = 0x20,

	// Could mean "USE TIME LIMIT" ? or it could be "continue searching"?
	PQF_UNK1 = 0x80000
};

#pragma pack(push, 1)
struct PathQuery {
	int flags; // See PathQueryFlags
	int field_4;
	LocAndOffsets from;
	LocAndOffsets to;
	int field28;
	int field2c;
	objHndl critter;  // Set PQF_HAS_CRITTER
	objHndl targetObj; // Set PQF_TARGET_OBJ
	int distanceToTarget; // Related to the targetObj's radius
	int radius; // I think this is the radius of critter
	int flags2;
	int field_4c;

	PathQuery() {
		memset(this, 0, sizeof(PathQuery));
	}
};

enum PathFlags {
	PF_COMPLETE = 0x1, // Seems to indicate that the path is complete (or valid?)
	PF_UNK1 = 0x10, // Seems to be setin response to query flag 0x80000
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

struct PathQueryResult {
	Path path;
	int d20sthg;
	int someDelay;
};

#pragma pack(pop)

struct Pathfinding : AddressTable {
	Pathfinding();

	bool (__cdecl *FindPath)(PathQuery &query, PathQueryResult &result);
	PathQueryResult * pathQArray;

} ;

extern Pathfinding pathfindingSys;