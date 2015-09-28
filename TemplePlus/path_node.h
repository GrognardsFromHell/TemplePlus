#pragma once
#include "common.h"


#define MAX_PATH_NODE_CHAIN_LENGTH 30

struct MapPathNode
{
	int id;
	int field4;
	LocAndOffsets nodeLoc;
	int neighboursCount;
	int * neighbours;
};

struct MapPathNodeList
{
	int flags;
	int field4;
	MapPathNode node;
	MapPathNodeList * next;
};

struct PathNodeSys: temple::AddressTable
{
	MapPathNodeList ** pathNodeList;
	BOOL (__cdecl* FindClosestPathNode)(LocAndOffsets * loc, int * nodeIdOut);
	int(__cdecl *FindPathBetweenNodes)(int fromNodeId, int toNodeId, int *nodeIds, int maxChainLength);
	BOOL(__cdecl* GetPathNode)(int id, MapPathNode * pathNodeOut);
	PathNodeSys()
	{
		rebase(GetPathNode, 0x100A9660);
		rebase(FindClosestPathNode, 0x100A96A0);
		rebase(FindPathBetweenNodes, 0x100A9E30);
		rebase(pathNodeList, 0x10BA62B0);
	}
};

extern PathNodeSys pathNodeSys;