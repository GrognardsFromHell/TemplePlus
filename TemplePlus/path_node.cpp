#include "stdafx.h"
#include "common.h"
#include "path_node.h"
#include "pathfinding.h"
#include "location.h"
#include "tio/tio.h"

PathNodeSys pathNodeSys;
char PathNodeSys::pathNodesLoadDir[260];
char PathNodeSys::pathNodesSaveDir[260];
MapPathNodeList ** PathNodeSys::pathNodeList;

struct PathNodeSysAddresses : temple::AddressTable
{
	
} addresses;


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
	newNode->node.neighbours = 0;
	if (!tio_fread(&newNode->node.id, 4, 1, file)
		|| !tio_fread(&newNode->node.nodeLoc, sizeof(LocAndOffsets), 1, file)
		|| !tio_fread(&newNode->node.neighboursCount, 4, 1, file))
		return 0;
	
	auto neighCnt = newNode->node.neighboursCount;
	if (!neighCnt)
		return 1;

	newNode->node.neighbours = new int32_t[neighCnt];
	if (tio_fread(newNode->node.neighbours, 4, neighCnt, file) == neighCnt)
		return 1;

	

	return result;
}

BOOL PathNodeSys::LoadNodesCurrent()
{
	char fileName[260];

	_snprintf(fileName, 260, "%s\\%s", pathNodesLoadDir, "pathnode.pnd");
	auto file = tio_fopen(fileName, "rb");
	if (!file)
		return 1;

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
				newNode->next = *pathNodeList;
				*pathNodeList = newNode;
			} else
			{
				logger->warn("path_node.cpp: LoadNodesCurrent(): failed to read from file");
				status = 0;
			}
		}
	}
	tio_fclose(file);
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
			auto node = *pathNodeList;
			while (node)
			{
				if (node->node.id == neighId)
					*(int*)&node->flags |= PNF_REMOVED_FROM_NEIGHBOUR_LIST;
				node = node->next;
			}
		}
		free(pn->node.neighbours);
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
	auto node = *pathNodeList;
	while (node)
	{
		*pathNodeList = node->next;
		FreeNode(node);
		node = *pathNodeList;
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
	auto i = *pathNodeList;
	PathQuery pathQ;
	PathQueryResult path;
	while (i)
	{
		if (node->node.id == i->node.id)
			continue;
		pathQ.from = node->node.nodeLoc;
		pathQ.to = i->node.nodeLoc;
		pathQ.flags = (PathQueryFlags) (PQF_DONT_USE_STRAIGHT_LINE | PQF_DONT_USE_PATHNODES | PQF_TO_EXACT);
		pathQ.critter = 0;
		pathQ.flags2 = 0;
		if (pathfindingSys.FindPath(&pathQ, &path ) > 0)
		{
			int neighSizeNew = 4 * (node->node.neighboursCount + 1);
			auto neighbours = node->node.neighbours;
			++node->node.neighboursCount;
			int * neighboursNew = (int*) realloc(neighbours, neighSizeNew);

			node->node.neighbours = neighboursNew;
			neighboursNew[node->node.neighboursCount - 1] = i->node.id;
		}
		i = i->next;
	}
}

void PathNodeSys::RecalculateAllNeighbours()
{
	auto node = *pathNodeList;
	while(node)
	{
		if (node->node.neighboursCount > 0)
		{
			free(node->node.neighbours);
			*(int*)&node->flags &= ~PNF_REMOVED_FROM_NEIGHBOUR_LIST;
			node->node.neighboursCount = 0;
			node->node.neighbours = nullptr;
		}
		RecalculateNeighbours(node);
		node = node->next;
	}
}

BOOL PathNodeSys::FlushNodes()
{
	int status = 0;
	int foundReleased = 0;
	int numNodes = 0;
	auto node = *pathNodeList;

	while(node)
	{
		numNodes++;
		if (node->flags & PNF_REMOVED_FROM_NEIGHBOUR_LIST)
			foundReleased = 1;
		node = node->next;
	}
	if (foundReleased)
		RecalculateAllNeighbours();

	char fileName[260];
	_snprintf(fileName, 260, "s\\s", pathNodesSaveDir, "pathnode.pnd");
	auto file = tio_fopen(fileName, "wb");
	if (!file)
		return 0;

	if (tio_fwrite(&numNodes,4,1,file) == 1)
	{
		node = *pathNodeList;
		status = 1;
		while (node)
		{
			if (!WriteNodeToFile(node, file))
			{
				logger->warn("path_node.cpp: FlushNodes() error writing to file");
				status = 0;
			}
			node = node->next;
		}
	}

	tio_fclose(file);
	return status;
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

int PathNodeSys::PopMinCumulNode(FindPathNodeData* fpndOut)
{
	int idxMinCumul ;

	// find a node with a positive cumulative distance
	for (idxMinCumul = 0; idxMinCumul < fpbnCount; idxMinCumul++)
		if (fpbnData[idxMinCumul].distCumul >= 0.0)
			break;
	if (idxMinCumul == fpbnCount)
		return 0;

	// search for minimum cumulative distance
	float minDistCumul = fpbnData[idxMinCumul].distCumul;
	for (int i = idxMinCumul +1 ; i < fpbnCount; i++)
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
	memcpy(&fpbnData[idxMinCumul], &fpbnData[fpbnCount-1], sizeof(FindPathNodeData));
	fpbnCount--;
	return 1;
}

BOOL PathNodeSys::FindClosestPathNode(LocAndOffsets* loc, int* nodeIdOut)
{
	float closestDist = 100000.0;
	float dist;
	auto node = *pathNodeList;
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
	auto pnIterator = *pathNodeList;
	for (numNodes = 0; pnIterator; ++numNodes)
		pnIterator = pnIterator->next;

	//fpbnCap = numNodes;
	//fpbnData = new FindPathNodeData[numNodes];
	fpbnCount = 0;


	// find the from/to nodes
	pnIterator = *pathNodeList;
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

	// testing neighbour IDs
	int neighboursTest[100];
	memcpy(neighboursTest, toNode.neighbours, toNode.neighboursCount * sizeof(int));
	


	// begin the A* algorithm
	float distFromTo = locSys.distBtwnLocAndOffs(fromNode.nodeLoc, toNode.nodeLoc);
	
	FindPathNodeData fpMinCumul;
	fpMinCumul.nodeId = fromNodeId;
	fpMinCumul.refererId = -1;
	fpMinCumul.distFrom = 0;
	fpMinCumul.distTo = distFromTo;
	fpMinCumul.distCumul = distFromTo;

	FindPathNodeAppend(&fpMinCumul);

	if (!PopMinCumulNode(&fpMinCumul))
	{
	//	delete fpbnData;
		return 0;
	}

	MapPathNodeList * neighbourListEntry;
	MapPathNode minCumulNode, neighNode;
	FindPathNodeData fpTemp, refererNode;

	while(fpMinCumul.nodeId != toNodeId)
	{
		neighbourListEntry = *pathNodeList;

		// find the minCumul node
		pnIterator = *pathNodeList;
		while  (pnIterator && pnIterator->node.id != fpMinCumul.nodeId)
		{
			pnIterator = pnIterator->next;
		}
		memcpy(&minCumulNode, &pnIterator->node, sizeof(MapPathNode));

		// loop thru its neighbours, searching 
		for (int i = 0; i < minCumulNode.neighboursCount; i++)
		{
			
			fpTemp.refererId = fpMinCumul.nodeId;
			fpTemp.nodeId = minCumulNode.neighbours[i];
			
			// find the neighbour node
			int neighbourId = minCumulNode.neighbours[i];
			pnIterator = *pathNodeList;
			while (pnIterator && pnIterator->node.id != neighbourId)
				pnIterator = pnIterator->next;
			if (pnIterator)
				memcpy(&neighNode, &pnIterator->node, sizeof(MapPathNode));

			// calculate its heuristic
			fpTemp.distTo = locSys.distBtwnLocAndOffs(neighNode.nodeLoc, toNode.nodeLoc);
			fpTemp.distFrom = locSys.distBtwnLocAndOffs(fromNode.nodeLoc, neighNode.nodeLoc);
			fpTemp.distCumul = fpMinCumul.distCumul + fpTemp.distTo + fpTemp.distFrom;

			// append node if it's optimal
			if (fpbnCount <= 0)
				FindPathNodeAppend(&fpTemp);
			else
			{
				int j = 0;;
				while (j < fpbnCount && fpbnData[j].nodeId != fpTemp.nodeId)
					j++;
				if (j == fpbnCount || fpTemp.distCumul < fpbnData[j].distCumul)
					FindPathNodeAppend(&fpTemp);
			}
			
		}
		fpMinCumul.distCumul = -1.0;
		FindPathNodeAppend(&fpMinCumul);

		if (!PopMinCumulNode(&fpMinCumul))
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
	auto node = *pathNodeList;
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

BOOL PathNodeSys::WriteNodeToFile(MapPathNodeList* node, TioFile* file)
{
	BOOL result = 0;
	if (!file)
		return 0;
	if (!node)
		return 0;
	if (!tio_fwrite(&node->node, 4, 1, file) == 1)
		return 0;
	if (!tio_fwrite(&node->node.nodeLoc, sizeof(LocAndOffsets), 1, file))
		return 0;
	int * count = &node->node.neighboursCount;
	if (tio_fwrite(&node->node.neighboursCount, 4,1, file) == 1 
		&& (!*count || tio_fwrite(node->node.neighbours, 4, *count , file) == *count) )
	{
		return 1;
	}
	return result;
}