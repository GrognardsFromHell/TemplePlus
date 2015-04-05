
#include "stdafx.h"
#include "map_obj.h"
#include "addresses.h"
#include "obj.h"
#include "python_debug.h"

struct MapFindNodeObj {
	int flags;
	int field4;
	objHndl objHandle;
	int field10;
	MapFindNodeObj *next;
	int field18;
	int field1c;
};

struct MapFindNodeObjPage {
	MapFindNodeObj nodes[128];
};

struct MapFindNodeSector {
	locXY sectorLoc;
	int unk1;
	int unk2;
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

static struct MapObjInternal : AddressTable {
	
	MapFindNodeData *findNodeData;

	MapObjInternal() {
		rebase(findNodeData, 0x10BCAC08);
	}

} mapObjInternal;

static void DumpMapObjects() {
	logger->info("HELLO WORLD!");
}

static PythonDebugFunc pyMapObjDebugFunc("dump_map_obj", &DumpMapObjects);
