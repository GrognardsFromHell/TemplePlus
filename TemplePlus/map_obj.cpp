#include "stdafx.h"
#include "common.h"
#include "map_obj.h"
#include <temple/dll.h>
#include "obj.h"
#include "python/python_debug.h"
#include "temple_functions.h"
#include "map_obj.h"
#include "util/fixes.h"

/**/
class MapObjReplacements : public TempleFix
{
	macTempleFix(Map Object System)
	{
		logger->info("Replacing Map Object Functions");

		//macReplaceFun(100819C0, _getSecLocFromLoc) //buggy
		//macReplaceFun(100C0DD0, _findSector) // buggy
		macReplaceFun(100C0FE0, _objNodeAlloc)
		//macReplaceFun(100C1130, _objFindNodeAlloc) // incomplete
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
	if (findNodeData->objPages) objPages = (MapFindNodeObjPage **)realloc(findNodeData->objPages, sizeof(void *) * objNodeCount);
	else objPages = (MapFindNodeObjPage **)malloc(sizeof(void *) * objNodeCount);
	findNodeData->objPages = objPages;

	MapFindNodeObjPage * objPageNew = (MapFindNodeObjPage *)malloc(0x1000);
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

void MapObjSystem::objFindNodeAlloc(objHndl objHnd)
{
	auto loc=  objects.getInt64(objHnd, obj_f_location);
	//TODO
}

uint32_t MapObjSystem::findSector(locationSec locSec, int32_t* idxOut)
{
	int32_t idxLo = 0;
	int32_t idxHi = findNodeData->sectorNodeCount - 1;
	if (findNodeData->sectorNodeCount <= 0) { *idxOut = 0; return 0; }
	
	int32_t idxMean=0;
	uint64_t idxMeanLoc = findNodeData->sectorNodes[idxMean].sectorLoc.location;
	while (idxLo < idxHi)
	{
		idxMean = (idxHi + idxLo) / 2;
		idxMeanLoc = findNodeData->sectorNodes[idxMean].sectorLoc.location;
		if (locSec.location == idxMeanLoc)
			{	*idxOut = idxMean;		return 1;	}
		if (locSec.location < idxMeanLoc)
		{
			idxHi = idxMean - 1;
			if (idxLo > idxHi)
			{
				*idxOut = idxLo;
				return 0;
			}
		}
		else
		{
			idxLo = idxMean + 1;
			if (idxLo > idxHi)
			{
				*idxOut = idxLo;
				return 0;
			}
		}
	}
	if (locSec.location != idxMeanLoc){ *idxOut = idxLo; return 0; }
	assert(idxMean >= 0 && idxMean < findNodeData->sectorNodeCount);
	*idxOut = idxMean; return 1;
}

uint64_t MapObjSystem::getSecLocFromLoc(uint64_t loc)
{
	uint32_t locy = loc >> 32;
	uint32_t locx = loc & 0xffffFFFF;
	return (locx + (static_cast<uint64_t>(locy) << 26));
}

#pragma region hooks
void _objNodeAlloc()
{
	mapObjSys.objNodeAlloc();
}

void _objFindNodeAlloc(objHndl objHnd)
{
	mapObjSys.objFindNodeAlloc(objHnd);
}

uint32_t _findSector(locationSec locSec, int32_t* idxOut)
{
	return mapObjSys.findSector(locSec, idxOut);
}

uint64_t _getSecLocFromLoc(uint64_t loc)
{
	return mapObjSys.getSecLocFromLoc(loc);
}
#pragma endregion


