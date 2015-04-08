#include "stdafx.h"
#include "common.h"
#include "map_obj.h"
#include "addresses.h"
#include "obj.h"
#include "python_debug.h"
#include "temple_functions.h"

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

static struct MapObjInternal : AddressTable {
	
	MapFindNodeData *findNodeData;

	MapObjInternal() {
		rebase(findNodeData, 0x10BCAC08);
	}

} mapObjInternal;

static void DumpMapObjects() {

	auto findNodeData = mapObjInternal.findNodeData;
	auto nodes = findNodeData->sectorNodes;
	
	for (auto i = 0; i < findNodeData->sectorNodeCount; ++i) {
		auto &sectorNode = nodes[i];
		logger->info("{}, {}", sectorNode.sectorLoc.x(), sectorNode.sectorLoc.y());

		auto objNode = sectorNode.firstObj;
		while (objNode) {
			logger->info("{}", objNode->objHandle);
			objNode = objNode->next;
		}
	}

}

static PythonDebugFunc pyMapObjDebugFunc("dump_map_obj", &DumpMapObjects);
