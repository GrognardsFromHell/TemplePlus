#include "stdafx.h"
#include "common.h"
#include "path_node.h"
#include "pathfinding.h"
#include "location.h"

PathNodeSys pathNodeSys;


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