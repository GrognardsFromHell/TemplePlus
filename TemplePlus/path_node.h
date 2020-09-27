#pragma once
#include "common.h"


#define MAX_PATH_NODE_CHAIN_LENGTH 30
#include "util/fixes.h"
#include <temple/dll.h>
#define MAX_NEIGHBOURS 64
#define MAX_OBJ_RADIUS_TILES 10 // the maximum object radius supported by clearance mapping, expressed in tiles. CANNOT BE MORE THAN 5.
#define MAX_OBJ_RADIUS_SUBTILES MAX_OBJ_RADIUS_TILES*3 // the maximum object radius supported by clearance mapping, expressed in subtiles

struct TioFile;
struct RenderWorldInfo;

struct ClearanceIndex // Holds the mapping from sector (Y,X) into SectorClearanceData[] array index; -1 if none exists
{
	unsigned char numSectors;
	uint16_t clrAddr[16][16]; // sectorY, sectorX
	ClearanceIndex();
	
	void Reset();
};

struct SectorClearanceData
{ 
	float val[64 * 3][64 * 3]; // clearance distance, for each subtile, in feet (matched to the Distance3d function return value)
};

struct MapClearanceData
{
	ClearanceIndex clrIdx;
	SectorClearanceData * secClr;

	void Reset();
	MapClearanceData();
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
	MapPathNodeList();
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
	static constexpr int MaxPathNodes = 30000; // hommlet has about 1000, so that should be enough! that would be 0.6MB

	void RecipDebug(); // debugging for non-reciprocating neighbour nodes (i.e. finds situations where A->B but not B->A)
	static char pathNodesLoadDir[260];
	static char pathNodesSaveDir[260];
	static MapClearanceData clearanceData;
	static bool hasClearanceData;

	static MapPathNodeList * pathNodeList;
	MapPathNodeList _pathNodeList[MaxPathNodes]; //  will replace the referenced list once we're done

	int fpbnCount; // Current number of used nodes in fpbnData
	std::array<FindPathNodeData, MaxPathNodes> fpbnData;



	static BOOL LoadNodeFromFile(TioFile* file, MapPathNodeList ** listOut );
	static bool LoadNeighDistFromFile(TioFile* file, MapPathNodeList* node);
	static BOOL LoadNodesCurrent();
	static void FreeNode(MapPathNodeList* node);
	void PopNode(MapPathNodeList* node);
	BOOL FreeAndLoad(char* loadDir, char*saveDir);
	void Reset();
	static void SetDirs(char *loadDir, char*saveDir);

	static void RecalculateNeighbours(MapPathNodeList* node);
	void RecalculateAllNeighbours();
	static bool WriteNodeDistToFile(MapPathNodeList* node, TioFile* tioFile);
	BOOL FlushNodes(const char* saveDir = nullptr);

	static void GenerateClearanceFile(const char* saveDir = nullptr);
	static int CalcClearanceFromNearbyObjects(objHndl obj, float clearanceReq);

	void FindPathNodeAppend(const FindPathNodeData &);
	int PopMinHeuristicNode(FindPathNodeData* fpndOut, bool useActualDistances); // output is 1 on success 0 on fail (if all nodes are negative distance)
	int PopMinHeuristicNodeLegacy(FindPathNodeData* fpndOut); // output is 1 on success 0 on fail (if all nodes are negative distance)
	BOOL FindClosestPathNode(LocAndOffsets * loc, int * nodeIdOut);
	int FindPathBetweenNodes(int fromNodeId, int toNodeId, int *nodeIds, int maxChainLength);
	void AddPathNode(LocAndOffsets& loc, bool recalcNeighbours=false);
	static BOOL GetPathNode(int id, MapPathNode * pathNodeOut);
	
	static bool PathNodeExists(int id);
	static BOOL WriteNodeToFile(MapPathNodeList*, TioFile*);

	void SetPathNodeVisibile(bool setting);
	void RenderPathNodes(int tileX1, int tileX2, int tileY1, int tileY2);
	bool SetActiveNodeFromLoc(LocAndOffsets& loc);
	void DeleteActiveNode();
	bool MoveActiveNode(LocAndOffsets * newLoc = nullptr); // return true if finished moving (second left click)
	void CancelMoveActiveNode();

	int GetNewId();

	void apply() override;

private:
	bool mNeedsRecalcNeighbours = false;
	bool mRenderPathNodesEn = false;
	MapPathNodeList* mActivePathNode = nullptr;
	LocAndOffsets mPathnodeMoveRef = LocAndOffsets::null;
	static MapPathNodeList* GetPathNodeListEntry(int id);
};

extern PathNodeSys pathNodeSys;