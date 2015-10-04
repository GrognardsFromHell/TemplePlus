#pragma once
#include "common.h"


#define MAX_PATH_NODE_CHAIN_LENGTH 30
#define PATH_NODE_CAP 30000 // hommlet has about 1000, so that should be enough! that would be 0.6MB
#include "util/fixes.h"
#include <temple/dll.h>

struct TioFile;

struct MapPathNode
{
	int id;
	int field4;
	LocAndOffsets nodeLoc;
	int neighboursCount;
	int * neighbours;
};

enum PathNodeFlags : int
{
	PNF_REMOVED_FROM_NEIGHBOUR_LIST = 1
};

struct MapPathNodeList
{
	PathNodeFlags flags;
	int field4;
	MapPathNode node;
	MapPathNodeList * next;
	int pad;
};

const int TestSizeMapPathNodeList = sizeof(MapPathNodeList); //  should be 48 (0x30)

struct FindPathNodeData
{
	int nodeId;
	int refererId; // for the From node is -1; together with the "distCumul = -1" nodes will form a chain leading From -> To
	float distFrom; // distance to the From node; is set to 0 for the from node naturally :)
	float distTo; // distance to the To node;
	float distCumul; // can be set to -1
};

class PathNodeSys: public TempleFix
{
public:
	const char* name() override {
		return "Path Node System";
	}

	static char pathNodesLoadDir[260];
	static char pathNodesSaveDir[260];

	static MapPathNodeList ** pathNodeList;
	MapPathNodeList _pathNodeList[PATH_NODE_CAP]; //  will replace the referenced list once we're done
	int fpbnCap;
	int fpbnCount;
	FindPathNodeData fpbnData[PATH_NODE_CAP];

	static BOOL LoadNodeFromFile(TioFile* file, MapPathNodeList ** listOut );
	static BOOL LoadNodesCurrent();
	static void FreeNode(MapPathNodeList* node);
	static BOOL FreeAndLoad(char* loadDir, char*saveDir);
	static void Reset();
	static void SetDirs(char *loadDir, char*saveDir);

	static void RecalculateNeighbours(MapPathNodeList* node);
	static void RecalculateAllNeighbours();
	static BOOL FlushNodes();

	void FindPathNodeAppend(FindPathNodeData *);
	int PopMinCumulNode(FindPathNodeData* fpndOut); // output is 1 on success 0 on fail (if all nodes are negative distance)
	BOOL FindClosestPathNode(LocAndOffsets * loc, int * nodeIdOut);
	int FindPathBetweenNodes(int fromNodeId, int toNodeId, int *nodeIds, int maxChainLength);
	BOOL GetPathNode(int id, MapPathNode * pathNodeOut);
	static BOOL WriteNodeToFile(MapPathNodeList*, TioFile*);
	void apply() override
	{
		//rebase(GetPathNode, 0x100A9660);
		//rebase(FindClosestPathNode, 0x100A96A0);
		//_FindPathBetweenNodes = temple::GetPointer<>(0x100A9E30);
		pathNodeList = temple::GetPointer<MapPathNodeList*>(0x10BA62B0);
		replaceFunction(0x100A9720, SetDirs);
		replaceFunction(0x100A9C00, LoadNodesCurrent);
		replaceFunction(0x100A9DA0, Reset);
		replaceFunction(0x100A9DD0, FreeAndLoad);
	}
	
};

extern PathNodeSys pathNodeSys;