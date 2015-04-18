
#pragma once
#include "common.h"
#include "temple_functions.h"


struct MapFindNodeData;

struct MapObjSystem : AddressTable {

	MapFindNodeData *findNodeData;

	MapObjSystem();
	void objNodeAlloc();
};

extern MapObjSystem mapObjSys;

struct MapFindNodeObj {
	int flags; // 2 if in use
	int field4; // badfood
	objHndl objHandle;
	MapFindNodeObj *prev;
	MapFindNodeObj *next;
	locationSec sectorLoc;
};

struct MapFindNodeObjPage {
	MapFindNodeObj nodes[128];
};

struct MapFindNodeSector {
	locationSec sectorLoc;
	MapFindNodeObj *firstObj;
	int unk2; // badfood
};

struct MapFindNodeData {
	MapFindNodeObj *nextFreeObjNode;
	int sectorNodeCount;
	int sectorNodeCapacity;
	int objNodeCount;
	MapFindNodeSector *sectorNodes;
	MapFindNodeObjPage **objPages;
	bool initialized;
};


void _objNodeAlloc();