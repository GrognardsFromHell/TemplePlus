#pragma once
#include "common.h"


#define MAX_PATH_NODE_CHAIN_LENGTH 30
#include "util/fixes.h"
#include <temple/dll.h>
#define MAX_NEIGHBOURS 64
#define MAX_OBJ_RADIUS_TILES 10 // the maximum object radius supported by clearance mapping, expressed in tiles. CANNOT BE MORE THAN 5.
#define MAX_OBJ_RADIUS_SUBTILES MAX_OBJ_RADIUS_TILES*3 // the maximum object radius supported by clearance mapping, expressed in subtiles

struct TioFile;

struct ClearanceProfile
{
	uint8_t radius;
	uint8_t clearance[MAX_OBJ_RADIUS_SUBTILES * 6 + 1][MAX_OBJ_RADIUS_SUBTILES * 6 + 1];
	ClearanceProfile()
	{
		radius = 1;
		memset(clearance, 255, sizeof(clearance));

	}
	void InitWithRadius(float radiusF)
	{
		radius = static_cast<uint8_t>(radiusF / (INCH_PER_TILE / 3));
		memset(clearance, 255, sizeof(clearance));
		int i0 = MAX_OBJ_RADIUS_SUBTILES * 6 / 2;
		for (int ny = 0; ny < MAX_OBJ_RADIUS_SUBTILES * 6 + 1; ny++)
		{
			for (int nx = 0; nx < MAX_OBJ_RADIUS_SUBTILES * 6 + 1; nx++)
			{
				if (ny == 45 && radius == 29)
				{
					int dummy = 1;
				}
				clearance[ny][nx] = (uint8_t) max(0.0001f, sqrtf(
					min({ powf(nx - 0.0f, 2.0f), powf(nx - 0.0f - 1, 2.0f), powf(nx - i0 + 1.0f, 2.0f) })
					+ min({ powf(ny - 0.0f, 2.0f), powf(ny - 0.0f - 1, 2.0f), powf(ny - i0 + 1.0f, 2.0f) }
				)) - radius);
				//		clearance[ny][nx] = sqrt(
				//			min( pow(nx -i0 ,2), pow(nx-i0+1,2), pow(nx-i0-1,2) )
				//			+ min(pow(ny-i0, 2), pow(ny-i0 + 1, 2), pow(ny-i0 - 1, 2))
				//			);
			}
		}
	};

};

struct ClearanceIndex
{
	unsigned char numSectors;
	uint16_t clrAddr[16][16]; // sectorY, sectorX
	ClearanceIndex()
	{
		Reset();
	}
	void Reset()
	{
		numSectors = 0;
		for (int i = 0; i < 16 * 16; i++)
		{
			for (int j = 0; j < 16; j++)
			{
				clrAddr[i][j] = -1;
			}
		}
	}
};

struct SectorClearanceData
{
	float val[64 * 3][64 * 3]; // in feet (matched to the Distance3d function return value)
};

struct MapClearanceData
{
	ClearanceIndex clrIdx;
	SectorClearanceData * secClr;
};

struct MapPathNode
{
	int id;
	int flags;
	LocAndOffsets nodeLoc;
	int neighboursCount;
	int * neighbours;
	float * neighDistances; // distances to the neighbours; is the negative of the distance if straight line is possible
};

enum PathNodeFlags : int
{
	PNF_NEIGHBOUR_STATUS_CHANGED = 1,
	PNF_NEIGHBOUR_DISTANCES_SET = 0x1000
};

struct MapPathNodeList
{
	PathNodeFlags flags;
	int field4;
	MapPathNode node;
	MapPathNodeList * next;
};

const int TestSizeMapPathNodeList = sizeof(MapPathNodeList); //  should be 48 (0x30)

struct FindPathNodeData
{
	int nodeId;
	int refererId; // for the From node is -1; together with the "distCumul = -1" nodes will form a chain leading From -> To
	float distFrom; // distance to the From node; is set to 0 for the from node naturally :)
	float distTo; // distance to the To node;
	float distCumul; // can be set to -1
	float distActualTotal;
	float heuristic;
	bool usingActualDistance;
};

class PathNodeSys: public TempleFix
{
public:
	const char* name() override {
		return "Path Node System";
	}

	static constexpr int MaxPathNodes = 30000; // hommlet has about 1000, so that should be enough! that would be 0.6MB

	void RecipDebug(); // debugging for non-reciprocating neighbour nodes (i.e. finds situations where A->B but not B->A)
	static char pathNodesLoadDir[260];
	static char pathNodesSaveDir[260];
	static ClearanceProfile clearanceProfiles[MAX_OBJ_RADIUS_SUBTILES];
	static MapClearanceData clearanceData;
	static bool hasClearanceData;
	//const int testSizeofClearanceData = sizeof(clearanceData);

	static MapPathNodeList * pathNodeList;
	MapPathNodeList _pathNodeList[MaxPathNodes]; //  will replace the referenced list once we're done

	int fpbnCount; // Current number of used nodes in fpbnData
	std::array<FindPathNodeData, MaxPathNodes> fpbnData;



	static BOOL LoadNodeFromFile(TioFile* file, MapPathNodeList ** listOut );
	static bool LoadNeighDistFromFile(TioFile* file, MapPathNodeList* node);
	static BOOL LoadNodesCurrent();
	static void FreeNode(MapPathNodeList* node);
	static BOOL FreeAndLoad(char* loadDir, char*saveDir);
	static void Reset();
	static void SetDirs(char *loadDir, char*saveDir);

	static void RecalculateNeighbours(MapPathNodeList* node);
	static void RecalculateAllNeighbours();
	static bool WriteNodeDistToFile(MapPathNodeList* node, TioFile* tioFile);
	static BOOL FlushNodes();

	static void GenerateClearanceFile();
	static int CalcClearanceFromNearbyObjects(objHndl obj, float clearanceReq);

	void FindPathNodeAppend(const FindPathNodeData &);
	int PopMinHeuristicNode(FindPathNodeData* fpndOut, bool useActualDistances); // output is 1 on success 0 on fail (if all nodes are negative distance)
	int PopMinHeuristicNodeLegacy(FindPathNodeData* fpndOut); // output is 1 on success 0 on fail (if all nodes are negative distance)
	BOOL FindClosestPathNode(LocAndOffsets * loc, int * nodeIdOut);
	int FindPathBetweenNodes(int fromNodeId, int toNodeId, int *nodeIds, int maxChainLength);
	static BOOL GetPathNode(int id, MapPathNode * pathNodeOut);
	static BOOL WriteNodeToFile(MapPathNodeList*, TioFile*);
	void apply() override
	{
		//rebase(GetPathNode, 0x100A9660);
		//rebase(FindClosestPathNode, 0x100A96A0);
		//_FindPathBetweenNodes = temple::GetPointer<>(0x100A9E30);
		//pathNodeList = temple::GetPointer<MapPathNodeList*>(0x10BA62B0);
		replaceFunction(0x100A9720, SetDirs);
		replaceFunction(0x100A9C00, LoadNodesCurrent);
		replaceFunction(0x100A9DA0, Reset);
		replaceFunction(0x100A9DD0, FreeAndLoad);
		hasClearanceData = false;
		for (int i = 1; i <= MAX_OBJ_RADIUS_SUBTILES;i++)
		{
			clearanceProfiles[i-1].InitWithRadius(i *INCH_PER_TILE/3);
		}
	}
	
};

extern PathNodeSys pathNodeSys;