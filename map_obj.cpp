#include "stdafx.h"
#include "common.h"
#include "map_obj.h"
#include "util/addresses.h"
#include "obj.h"
#include "python/python_debug.h"
#include "temple_functions.h"
#include "map_obj.h"

/**/
class MapObjReplacements : public TempleFix
{
	macTempleFix(Map Object System)
	{
		logger->info("Replacing Map Object Functions");

		macReplaceFun(100C0FE0, _objNodeAlloc)
	}
} mapObjReplacements;


struct MapObjSystem mapObjSys;


MapObjSystem::MapObjSystem() {

	rebase(findNodeData, 0x10BCAC08);
}

void MapObjSystem::objNodeAlloc()
{
	MapFindNodeObjPage ** objPages;
	int objNodeCount = (*findNodeData).objNodeCount++ + 1;
	if (findNodeData->objPages) objPages = (MapFindNodeObjPage **)allocFuncs._realloc(findNodeData->objPages, sizeof(void *) * objNodeCount);
	else objPages = (MapFindNodeObjPage **)allocFuncs._malloc_0(sizeof(void *) * objNodeCount);
	findNodeData->objPages = objPages;

	MapFindNodeObjPage * objPageNew = (MapFindNodeObjPage *)allocFuncs._malloc_0(0x1000);
	findNodeData->objPages[findNodeData->objNodeCount - 1] = objPageNew;

	auto objNode = &objPageNew[0];
	auto nextObjNode = findNodeData->nextFreeObjNode;
	for (auto i = 0; i < 128; i++)
	{
		objPageNew->nodes[i].next = nextObjNode;
		objPageNew->nodes[i].flags = 0;
		nextObjNode = &objPageNew->nodes[i];
	}
	findNodeData->nextFreeObjNode = nextObjNode;
}

void _objNodeAlloc()
{
	mapObjSys.objNodeAlloc();
}

static void DumpMapObjects() {

	auto findNodeData = mapObjSys.findNodeData;
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
