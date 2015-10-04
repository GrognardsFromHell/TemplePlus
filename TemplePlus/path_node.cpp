#include "stdafx.h"
#include "common.h"
#include "path_node.h"
#include "pathfinding.h"
#include "location.h"
#include "tio/tio.h"

PathNodeSys pathNodeSys;
char PathNodeSys::pathNodesLoadDir[260];
char PathNodeSys::pathNodesSaveDir[260];
MapPathNodeList * PathNodeSys::pathNodeList;

struct PathNodeSysAddresses : temple::AddressTable
{
	
} addresses;


void PathNodeSys::RecipDebug()
{

	// check reciprocity
	struct RecipCheck
	{
		int nodeId;
		MapPathNodeList * node;
		int neighIds[MAX_NEIGHBOURS];
		MapPathNode neighbours[MAX_NEIGHBOURS];
		int nonrecipratingNodes[MAX_NEIGHBOURS];
		float distances[MAX_NEIGHBOURS];
		float distances2[MAX_NEIGHBOURS]; // for the opposite direction
		int numNonRecip;
		RecipCheck()
		{
			node = nullptr;
			nodeId = -1;
			numNonRecip = 0;
		}
	};

	auto oneSided = new RecipCheck[PATH_NODE_CAP];
	int n = 0;

	auto node = pathNodeList;
	while (node)
	{
		bool hasNonRecip = false;
		for (int i = 0; i < node->node.neighboursCount; i++)
		{
			int naughtyNeighbour = 0;
			MapPathNode neighbour;
			GetPathNode(node->node.neighbours[i], &neighbour);
			bool isReciprocal = false;
			for (int j = 0; j < neighbour.neighboursCount && !isReciprocal; j++)
			{
				if (neighbour.neighbours[j] == node->node.id)
					isReciprocal = true;
			}
			if (!isReciprocal)
			{
				hasNonRecip = true;
				oneSided[n].nodeId = node->node.id;
				oneSided[n].node = node;
				oneSided[n].nonrecipratingNodes[oneSided[n].numNonRecip] = neighbour.id;
				oneSided[n].neighbours[oneSided[n].numNonRecip++] = neighbour;
			}
		}
		if (hasNonRecip)
		{
			n++;
		}
		node = node->next;
	}
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < oneSided[i].numNonRecip; j++)
		{
			// first verify the direct path - this should be ok
			PathQuery pathQ;
			PathQueryResult path;
			pathQ.from = oneSided[i].node->node.nodeLoc;
			pathQ.to = oneSided[i].neighbours[j].nodeLoc;
			pathQ.flags = (PathQueryFlags)(PQF_DONT_USE_STRAIGHT_LINE | PQF_DONT_USE_PATHNODES | PQF_TO_EXACT);
			pathQ.critter = 0;
			pathQ.flags2 = 0;
			if (pathfindingSys.FindPath(&pathQ, &path) > 0)
			{
				auto pathLen = pathfindingSys.pathLength(&path);
				auto fromToDist = locSys.distBtwnLocAndOffs(pathQ.from, pathQ.to) / 12.0;
				oneSided[i].distances[j] = pathLen;
			} else
			{
				oneSided[i].distances[j] = -1.0;
			}

			// now try the opposite path
			pathQ.from = oneSided[i].neighbours[j].nodeLoc;
			pathQ.to = oneSided[i].node->node.nodeLoc;
			pathQ.flags = (PathQueryFlags)(PQF_DONT_USE_STRAIGHT_LINE | PQF_DONT_USE_PATHNODES | PQF_TO_EXACT);
			pathQ.critter = 0;
			pathQ.flags2 = 0;
			if (pathfindingSys.FindPath(&pathQ, &path) > 0)
			{
				auto pathLen2 = pathfindingSys.pathLength(&path);
				auto fromToDist = locSys.distBtwnLocAndOffs(pathQ.from, pathQ.to) / 12.0;
				oneSided[i].distances2[j] = pathLen2;
			}
			else // failed to path back; let's see if it's critters blocking
			{
				pathQ.from = oneSided[i].neighbours[j].nodeLoc;
				pathQ.to = oneSided[i].node->node.nodeLoc;
				pathQ.flags = (PathQueryFlags)(PQF_DONT_USE_STRAIGHT_LINE | PQF_DONT_USE_PATHNODES | PQF_TO_EXACT | PQF_IGNORE_CRITTERS | PQF_IGNORE_CRITTERS_ON_DESTINATION);
				pathQ.critter = 0;
				pathQ.flags2 = 0;
				if (pathfindingSys.FindPath(&pathQ, &path) > 0)
				{
					auto pathLen2 = pathfindingSys.pathLength(&path);
					oneSided[i].distances2[j] = pathLen2;
				} 
				else
					oneSided[i].distances2[j] = -1.0;
			}

		}
	}

	// hommlet results:
	// 90 cases of nodes with non-reciprocatig neighbours
	// all of them are actually pathable so long as you ignore critters on the destination tile
}

BOOL PathNodeSys::LoadNodeFromFile(TioFile* file, MapPathNodeList** nodeOut)
{
	int result = 0;

	if (!file)
		return 0;

	if (!nodeOut)
		return 0;

	MapPathNodeList * newNode = new MapPathNodeList;
	*nodeOut = newNode;

	if (!newNode)
		return 0;

	newNode->flags = (PathNodeFlags)0;
	newNode->node.flags = (PathNodeFlags)0;
	newNode->node.neighbours = 0;
	if (!tio_fread(&newNode->node.id, 4, 1, file)
		|| !tio_fread(&newNode->node.nodeLoc, sizeof(LocAndOffsets), 1, file)
		|| !tio_fread(&newNode->node.neighboursCount, 4, 1, file))
		return 0;
	
	auto neighCnt = newNode->node.neighboursCount;
	if (!neighCnt)
	{ // why are we holding nodes without neighbours? they should probably be culled...
		return 1;
	}
	if (neighCnt > MAX_NEIGHBOURS)
	{
		logger->info("Too many neighbours for node {}", newNode->node.id);
		assert(neighCnt <= MAX_NEIGHBOURS);
	}
		

	// newNode->node.neighbours = new int32_t[neighCnt];
	newNode->node.neighbours = new int32_t[MAX_NEIGHBOURS];
	if (tio_fread(newNode->node.neighbours, sizeof(int), neighCnt, file) != neighCnt)
		return 0;


	// newNode->node.neighDistances = new float[newNode->node.neighboursCount];
	newNode->node.neighDistances = new float[MAX_NEIGHBOURS];
	memset(newNode->node.neighDistances, 0, sizeof(float)* newNode->node.neighboursCount);
	
	return 1;
}

bool PathNodeSys::LoadNeighDistFromFile(TioFile* file, MapPathNodeList* node)
{

	int result = 0;

	if (!file)
		return false;

	if (!node)
		return false;



	auto neighCnt = node->node.neighboursCount;

	int nodeId;
	if (!tio_fread(&nodeId, 4, 1, file))
		return false;

	if (nodeId != node->node.id)
		return false;

	if (!tio_fread(&neighCnt, 4, 1, file))
		return false;

	if (neighCnt != node->node.neighboursCount)
		return false;

	if (!neighCnt)
	{
		return true;
	}

	if (tio_fread(node->node.neighDistances, sizeof(float), neighCnt, file) == neighCnt)
	{
		*(int*)&node->node.flags |= PNF_NEIGHBOUR_DISTANCES_SET;
		return 1;
	}
	return false;

}

BOOL PathNodeSys::LoadNodesCurrent()
{
	//int orphanNodeCount = 0;
	//int orphanNodes[PATH_NODE_CAP] ={};

	char fileName[260];
	char supplem[260];

	_snprintf(fileName, 260, "%s\\%s", pathNodesLoadDir, "pathnode.pnd");
	_snprintf(supplem, 260, "%s\\%s", pathNodesLoadDir, "pathnodedist.pnd");

	auto file = tio_fopen(fileName, "rb");
	if (!file)
		return 1;
	auto fileSupplem = tio_fopen(supplem, "rb");
	int nodeCountSupplem = 0;
	if (fileSupplem)
	{
		if (!tio_fread(&nodeCountSupplem, 4, 1 ,fileSupplem))
			nodeCountSupplem = 0;
	}

	int nodeCount = 0;
	int status = 0;
	MapPathNodeList * newNode;
	if (tio_fread(&nodeCount, 4, 1, file) == 1)
	{
		status = 1;
		for (int i = 0; i < nodeCount; i++)
		{
			if (LoadNodeFromFile(file, &newNode ))
			{
				if (nodeCountSupplem == nodeCount)
					if (!LoadNeighDistFromFile(fileSupplem, newNode))
					{
						int dummy = 1;
					};
				newNode->next = pathNodeList;
				pathNodeList = newNode;
				/*
				if (newNode->node.neighboursCount == 0)
				{
					orphanNodes[orphanNodeCount++] = newNode->node.id;
				}
				*/
				if (pathNodeList->node.neighboursCount >0 &&  pathNodeList->node.neighDistances[0] < 0.1)
				{
					int dummy = 1;
				}
				else
				{
					int dum = 1;
				}
			} else
			{
				logger->warn("path_node.cpp: LoadNodesCurrent(): failed to read from file");
				status = 0;
			}
		}
	}
	tio_fclose(file);
	if (fileSupplem) tio_fclose(fileSupplem);
	return status;
}

void PathNodeSys::FreeNode(MapPathNodeList* pn)
{
	int neighCnt = pn->node.neighboursCount;

	if (neighCnt)
	{
		for (int i = 0; i < neighCnt; i++)
		{
			auto neighId = pn->node.neighbours[i];
			auto node = pathNodeList;
			while (node)
			{
				if (node->node.id == neighId)
					*(int*)&node->flags |= PNF_NEIGHBOUR_STATUS_CHANGED;
				node = node->next;
			}
		}
		free(pn->node.neighbours);
		//if (pn->node.flags & PNF_NEIGHBOUR_DISTANCES_SET)
			free(pn->node.neighDistances);
	}

	
	free( pn);
}

BOOL PathNodeSys::FreeAndLoad(char* loadDir, char* saveDir)
{
	Reset();
	SetDirs(loadDir, saveDir);
	return LoadNodesCurrent();
}

void PathNodeSys::Reset()
{
	auto node = pathNodeList;
	while (node)
	{
		pathNodeList = node->next;
		FreeNode(node);
		node = pathNodeList;
	}
}

void PathNodeSys::SetDirs(char* loadDir, char* saveDir)
{
	strncpy(pathNodesLoadDir, loadDir, 260);
	strncpy(pathNodesSaveDir, saveDir, 260);
}

void PathNodeSys::RecalculateNeighbours(MapPathNodeList* node)
{
	if (!node)
		return;
	auto i = pathNodeList;
	PathQuery pathQ;
	PathQueryResult path;
	while (i)
	{
		if (node->node.id == i->node.id)
		{
			i = i->next;
			continue;
		}
			
		pathQ.from = node->node.nodeLoc;
		pathQ.to = i->node.nodeLoc;
		pathQ.flags = (PathQueryFlags) (PQF_DONT_USE_STRAIGHT_LINE | PQF_DONT_USE_PATHNODES | PQF_TO_EXACT | PQF_IGNORE_CRITTERS | PQF_IGNORE_CRITTERS_ON_DESTINATION);
		pathQ.critter = 0;
		pathQ.flags2 = 0;
		if (pathfindingSys.FindPath(&pathQ, &path ) > 0)
		{
			auto pathLen = pathfindingSys.pathLength(&path);
			auto fromToDist = locSys.distBtwnLocAndOffs(pathQ.from, pathQ.to) / 12.0;

			if ( pathLen < 6 * fromToDist && node->node.neighboursCount < MAX_NEIGHBOURS) // prevent extremely roundabout links
			{
				int neighSizeNew = 4 * (node->node.neighboursCount + 1);
				auto neighCntNew = ++node->node.neighboursCount;
				// int * neighboursNew = (int*)realloc(neighbours, neighSizeNew);
				

				//node->node.neighbours = neighboursNew;
				//neighboursNew[neighCntNew - 1] = i->node.id;
				node->node.neighbours[neighCntNew - 1] = i->node.id;

				// node->node.neighDistances = (float*)realloc(node->node.neighDistances, neighSizeNew);
				if  (path.flags & PF_STRAIGHT_LINE_SUCCEEDED)
					node->node.neighDistances[neighCntNew - 1] = -pathLen;
				else
					node->node.neighDistances[neighCntNew - 1] = pathLen;
				
				
			}
			else
			{
				int aha = 0;
			}
		}
		i = i->next;
	}
	*(int*)&node->node.flags |= PNF_NEIGHBOUR_DISTANCES_SET;
}

void PathNodeSys::RecalculateAllNeighbours()
{
	auto node = pathNodeList;
	int maxNeighbours = 0;
	while(node)
	{
		int prevNeighCnt = node->node.neighboursCount;
		int prevNeighs[MAX_NEIGHBOURS] = {};
		memcpy(prevNeighs, node->node.neighbours, sizeof(int)*prevNeighCnt);
		if (node->node.neighboursCount > 0)
		{
			free(node->node.neighbours);
			if (node->node.flags & PNF_NEIGHBOUR_DISTANCES_SET)
				free(node->node.neighDistances);
			*(int*)&node->flags &= ~(PNF_NEIGHBOUR_STATUS_CHANGED );
			node->node.neighboursCount = 0;
			node->node.neighbours = nullptr;
			node->node.neighDistances = nullptr;
		}
		node->node.neighbours = new int[MAX_NEIGHBOURS];
		node->node.neighDistances = new float[MAX_NEIGHBOURS];
		RecalculateNeighbours(node);
		int newNeighCnt = node->node.neighboursCount;
		if (newNeighCnt > maxNeighbours)
			maxNeighbours = newNeighCnt;
		if (newNeighCnt != prevNeighCnt)
		{
			int newNeighs[MAX_NEIGHBOURS] = {};
			memcpy(newNeighs, node->node.neighbours, sizeof(int)*newNeighCnt);
			logger->info("new node connectivity for node {}: old was {} neighbours, new is {}", node->node.id ,prevNeighCnt, newNeighCnt);
		}
		node = node->next;
	}

	// verify reciprocity
	node = pathNodeList;
	while( node)
	{
		for (int i = 0; i < node->node.neighboursCount;i++)
		{
			MapPathNode neighbour;
			GetPathNode(node->node.neighbours[i], &neighbour);
			bool isReciprocal = false;
			for (int j = 0; j < neighbour.neighboursCount && !isReciprocal;j++)
			{
				if (neighbour.neighbours[j] == node->node.id)
					isReciprocal = true;
			}
			if (!isReciprocal)
			{
				int dummy = 1;
			}
		}
		node = node->next;
	}
}



BOOL PathNodeSys::FlushNodes()
{
	int status = 0;
	int foundReleased = 0;
	int numNodes = 0;
	auto node = pathNodeList;

	while(node)
	{
		numNodes++;
		if (node->flags & PNF_NEIGHBOUR_STATUS_CHANGED)
			foundReleased = 1;
		node = node->next;
	}
	if (foundReleased)
		RecalculateAllNeighbours();

	char fileName[260];
	char fileNameSupplem[260];
	_snprintf(fileName, 260, "%s\\%s", pathNodesSaveDir, "pathnode.pnd");
	_snprintf(fileNameSupplem, 260, "%s\\%s", pathNodesSaveDir, "pathnodedist.pnd");
	auto file = tio_fopen(fileName, "wb");
	if (!file)
		return 0;

	auto fileSupplem = tio_fopen(fileNameSupplem, "wb");
	if (!fileSupplem)
		return 0;

	if (tio_fwrite(&numNodes,4,1,file) == 1 && tio_fwrite(&numNodes, 4,1,fileSupplem) == 1)
	{
		node = pathNodeList;
		status = 1;
		while (node)
		{
			if (!WriteNodeToFile(node, file))
			{
				logger->warn("path_node.cpp: FlushNodes() error writing to file");
				status = 0;
			}
			if (!WriteNodeDistToFile(node, fileSupplem))
			{
				logger->warn("path_node.cpp: FlushNodes() error writing neighbour distances to file");
				status = 0;
			}
			node = node->next;
		}
	}

	tio_fclose(file);
	tio_fclose(fileSupplem);
	return status;
}


BOOL PathNodeSys::WriteNodeToFile(MapPathNodeList* node, TioFile* file)
{
	BOOL result = 0;
	if (!file)
		return 0;
	if (!node)
		return 0;
	if (!tio_fwrite(&node->node.id, 4, 1, file) == 1)
		return 0;
	if (!tio_fwrite(&node->node.nodeLoc, sizeof(LocAndOffsets), 1, file))
		return 0;
	int  count = node->node.neighboursCount;
	if (tio_fwrite(&node->node.neighboursCount, 4, 1, file) == 1
		&& (!count || tio_fwrite(node->node.neighbours, sizeof(int), count, file) == count))
	{
		return 1;
	}
	return result;
}

bool PathNodeSys::WriteNodeDistToFile(MapPathNodeList* node, TioFile* file)
{
	BOOL result = 0;
	if (!file)
		return 0;
	if (!node)
		return 0;
	if (!tio_fwrite(&node->node.id, 4, 1, file) == 1)
		return 0;

	int *count = &node->node.neighboursCount;
	if (tio_fwrite(count, 4, 1, file) == 1
		&& (!*count || tio_fwrite(node->node.neighDistances, sizeof(float), *count, file) == *count)
		)
	{
		return 1;
	}
	return result;
}

void PathNodeSys::FindPathNodeAppend(FindPathNodeData* node)
{ /*
	if (fpbnCount >= fpbnCap)
	{
		fpbnCap *= 2;
		fpbnData = (FindPathNodeData*)realloc(fpbnData, sizeof(FindPathNodeData)*fpbnCap);
	}
	*/
	memcpy(&fpbnData[fpbnCount], node, sizeof(FindPathNodeData));
	fpbnCount++;
}

int PathNodeSys::PopMinHeuristicNode(FindPathNodeData* fpndOut, bool useActualDist)
{

	if (!useActualDist)
		return PopMinHeuristicNodeLegacy(fpndOut);

	int idxMin;
	// find a node with a positive actual distance
	for (idxMin = 0; idxMin < fpbnCount; idxMin++)
		if (fpbnData[idxMin].distActualTotal >= 0.0)
			break;
	if (idxMin == fpbnCount)
		return 0;

	// search for minimum cumulative distance
	float minDistCumul = fpbnData[idxMin].distActualTotal + fpbnData[idxMin].distTo;
	for (int i = idxMin + 1; i < fpbnCount; i++)
	{
		if (fpbnData[i].distActualTotal >= 0.0
			&& ( fpbnData[i].distActualTotal + fpbnData[i].distTo ) < minDistCumul)
		{
			idxMin = i;
			minDistCumul = fpbnData[idxMin].distActualTotal + fpbnData[idxMin].distTo;
		}
	}

	// copy it out
	memcpy(fpndOut, &fpbnData[idxMin], sizeof(FindPathNodeData));

	// pop the found entry
	memcpy(&fpbnData[idxMin], &fpbnData[fpbnCount - 1], sizeof(FindPathNodeData));
	fpbnCount--;
	return 1;
}

int PathNodeSys::PopMinHeuristicNodeLegacy(FindPathNodeData* fpndOut)
{
	int idxMinCumul;
	// find a node with a positive cumulative distance
	for (idxMinCumul = 0; idxMinCumul < fpbnCount; idxMinCumul++)
		if (fpbnData[idxMinCumul].distCumul >= 0.0)
			break;
	if (idxMinCumul == fpbnCount)
		return 0;

	// search for minimum cumulative distance
	float minDistCumul = fpbnData[idxMinCumul].distCumul;
	for (int i = idxMinCumul + 1; i < fpbnCount; i++)
	{
		if (fpbnData[i].distCumul >= 0.0
			&& fpbnData[i].distCumul < minDistCumul)
		{
			idxMinCumul = i;
			minDistCumul = fpbnData[idxMinCumul].distCumul;
		}
	}

	// copy it out
	memcpy(fpndOut, &fpbnData[idxMinCumul], sizeof(FindPathNodeData));

	// pop the found entry
	memcpy(&fpbnData[idxMinCumul], &fpbnData[fpbnCount - 1], sizeof(FindPathNodeData));
	fpbnCount--;
	return 1;
}

BOOL PathNodeSys::FindClosestPathNode(LocAndOffsets* loc, int* nodeIdOut)
{
	float closestDist = 100000.0;
	float dist;
	auto node = pathNodeList;
	MapPathNodeList * closestNode = nullptr;

	while (node)
	{
		dist = locSys.distBtwnLocAndOffs(node->node.nodeLoc, *loc);
		if (!closestNode || dist < closestDist)
		{
			closestNode = node;
			closestDist = dist;
		}
		node = node->next;
	}

	if (closestNode )
	{
		if (nodeIdOut)
			*nodeIdOut = closestNode->node.id;
		return 1;
	}

	return 0;
}

int PathNodeSys::FindPathBetweenNodes(int fromNodeId, int toNodeId, int* nodeIds, int maxChainLength)
{
	MapPathNode fromNode, toNode;
	int chainLength = 0;
	int numNodes ;
	auto pnIterator = pathNodeList;
	for (numNodes = 0; pnIterator; ++numNodes)
		pnIterator = pnIterator->next;

	//fpbnCap = numNodes;
	//fpbnData = new FindPathNodeData[numNodes];
	fpbnCount = 0;


	// find the from/to nodes
	pnIterator = pathNodeList;
	int foundStatus = 0;
	while (foundStatus != 3 && pnIterator )
	{
		if (pnIterator->node.id == fromNodeId)
		{
			memcpy(&fromNode, &pnIterator->node, sizeof(MapPathNode));
			foundStatus |= 1;
		}
			
		else if (pnIterator->node.id == toNodeId)
		{
			memcpy(&toNode, &pnIterator->node, sizeof(MapPathNode));
			foundStatus |= 2;
		}
			
		pnIterator = pnIterator->next;
	}
	if (foundStatus != 3)
		return 0;


	// determine if the pathnodes are using the supplemental information of Actual Travel Distance (NEW! TemplePlus only)
	// if so, the node distance evaluation algorithm will be quite different, taking into acconut the actual travel distance rather than the strange method employed by troika
	bool useActualDistances = false;
	if ((toNode.flags & PNF_NEIGHBOUR_DISTANCES_SET) && (fromNode.flags & PNF_NEIGHBOUR_DISTANCES_SET))
		useActualDistances = true;
	

	// testing neighbour IDs
	//int neighboursTest[MAX_NEIGHBOURS];
	//memcpy(neighboursTest, toNode.neighbours, toNode.neighboursCount * sizeof(int));
	


	// begin the A* algorithm
	float distFromTo = locSys.distBtwnLocAndOffs(fromNode.nodeLoc, toNode.nodeLoc);
	
	FindPathNodeData fpMinCumul;
	fpMinCumul.nodeId = fromNodeId;
	fpMinCumul.refererId = -1;
	fpMinCumul.distFrom = 0;
	fpMinCumul.distTo = distFromTo;
	fpMinCumul.distCumul = distFromTo;
	fpMinCumul.distActualTotal = 0;
	fpMinCumul.heuristic = 0;
	fpMinCumul.usingActualDistance = useActualDistances;

	FindPathNodeAppend(&fpMinCumul);

	if (!PopMinHeuristicNode(&fpMinCumul, useActualDistances))
	{
	//	delete fpbnData;
		return 0;
	}

	MapPathNodeList * neighbourListEntry;
	MapPathNode minCumulNode, neighNode;
	FindPathNodeData fpTemp, refererNode;

	while(fpMinCumul.nodeId != toNodeId)
	{
		neighbourListEntry = pathNodeList;

		// find the matching Path Node
		pathNodeSys.GetPathNode(fpMinCumul.nodeId, &minCumulNode);

		// loop thru its neighbours, searching 
		for (int i = 0; i < minCumulNode.neighboursCount; i++)
		{
			
			fpTemp.refererId = fpMinCumul.nodeId;
			fpTemp.nodeId = minCumulNode.neighbours[i];
			
			// find the neighbour node
			int neighbourId = minCumulNode.neighbours[i];
			pathNodeSys.GetPathNode(neighbourId, &neighNode);
			

			// calculate its heuristic
			fpTemp.distTo = locSys.distBtwnLocAndOffs(neighNode.nodeLoc, toNode.nodeLoc);
			fpTemp.distFrom = locSys.distBtwnLocAndOffs(fromNode.nodeLoc, neighNode.nodeLoc);
			fpTemp.distCumul = fpMinCumul.distCumul + fpTemp.distTo + fpTemp.distFrom;
			if (useActualDistances)
			{
				fpTemp.distActualTotal = fpMinCumul.distActualTotal + minCumulNode.neighDistances[i];
			}
				

			// append node if it's optimal
			if (fpbnCount <= 0)
				FindPathNodeAppend(&fpTemp);
			else
			{
				int j = 0;;
				while (j < fpbnCount && fpbnData[j].nodeId != fpTemp.nodeId)
					j++;
				if (useActualDistances)
				{
					if (j == fpbnCount || (fpTemp.distActualTotal + fpTemp.distTo )< (fpbnData[j].distActualTotal + fpbnData[j].distTo))
						FindPathNodeAppend(&fpTemp);
				} 
				else
				{
					if (j == fpbnCount || fpTemp.distCumul < fpbnData[j].distCumul)
						FindPathNodeAppend(&fpTemp);
				}
				
			}
			
		}
		fpMinCumul.distCumul = -1.0;
		fpMinCumul.distActualTotal = -1.0;
		FindPathNodeAppend(&fpMinCumul);

		if (!PopMinHeuristicNode(&fpMinCumul, useActualDistances))
		{
		//	delete fpbnData;
			return 0;
		}
	}

	memcpy(&fpTemp, &fpMinCumul, sizeof(FindPathNodeData));

	int refId0 = fpMinCumul.refererId; // the node leading up to the last node
	chainLength = 1;

	// get the chain length
	int refererId = fpTemp.refererId;
	while (refererId != -1)
	{
		int refererFound = 0;
		for (int i = 0; i < fpbnCount; i++)
		{
			if (fpbnData[i].nodeId == refererId)
			{
				memcpy(&fpTemp, &fpbnData[i], sizeof(FindPathNodeData));
				refererFound = 1;
				break;
			}
				
		}
		if (!refererFound)
		{
			break;
			//shit
		}
		chainLength++;
		refererId = fpTemp.refererId;
	}

	if (chainLength >= maxChainLength)
	{
	//	delete fpbnData;
		return 0;
		
	}

	// dump node chain into nodeIds
	memcpy(&fpTemp, &fpMinCumul, sizeof(FindPathNodeData));
	refererId = fpMinCumul.refererId;
	for (int i = chainLength -1 ; i >= 0 ; i--)
	{
		nodeIds[i] = fpTemp.nodeId;
		int refererFound = 0;
		for (int refIdx = 0; refIdx < fpbnCount; refIdx++)
		{
			if (fpbnData[refIdx].nodeId == refererId)
			{
				memcpy(&fpTemp, &fpbnData[refIdx], sizeof(FindPathNodeData));
				refererId = fpTemp.refererId;
				refererFound = 1;
				break;
			}

		}
		if (i == 0 && refererId != -1)
		{
			int breakPointDummy = 1;
		}

	}

//	delete fpbnData;
	return chainLength;
	


	// return _FindPathBetweenNodes(fromNodeId, toNodeId, nodeIds, maxChainLength);
}

BOOL PathNodeSys::GetPathNode(int id, MapPathNode* pathNodeOut)
{
	auto node = pathNodeList;
	if (node)
	{
		while( node && node->node.id != id)
		{
			node = node->next;
		}
		if (!node)
			return 0;
		memcpy(pathNodeOut, &node->node, sizeof(MapPathNode));
		return 1;
	}
	return 0;
}

