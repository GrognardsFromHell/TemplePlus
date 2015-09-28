#pragma once
#include "common.h"


#define MAX_PATH_NODE_CHAIN_LENGTH 30
#define PATH_NODE_CAP 30000 // hommlet has about 1000, so that should be enough! that would be 0.6MB

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

struct FindPathNodeData
{
	int nodeId;
	int refererId; // for the From node is -1; together with the "distCumul = -1" nodes will form a chain leading From -> To
	float distFrom; // distance to the From node; is set to 0 for the from node naturally :)
	float distTo; // distance to the To node;
	float distCumul; // can be set to -1
};

struct PathNodeSys: temple::AddressTable
{
	MapPathNodeList ** pathNodeList;
	int fpbnCap;
	int fpbnCount;
	FindPathNodeData fpbnData[PATH_NODE_CAP];

	void FindPathNodeAppend(FindPathNodeData *);
	int PopMinCumulNode(FindPathNodeData* fpndOut); // output is 1 on success 0 on fail (if all nodes are negative distance)
	BOOL (__cdecl* FindClosestPathNode)(LocAndOffsets * loc, int * nodeIdOut);
	int(__cdecl *_FindPathBetweenNodes)(int fromNodeId, int toNodeId, int *nodeIds, int maxChainLength);
	int FindPathBetweenNodes(int fromNodeId, int toNodeId, int *nodeIds, int maxChainLength);
	BOOL(__cdecl* GetPathNode)(int id, MapPathNode * pathNodeOut);
	PathNodeSys()
	{
		rebase(GetPathNode, 0x100A9660);
		rebase(FindClosestPathNode, 0x100A96A0);
		rebase(_FindPathBetweenNodes, 0x100A9E30);
		rebase(pathNodeList, 0x10BA62B0);
	}
};

extern PathNodeSys pathNodeSys;