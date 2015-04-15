
#include "stdafx.h"
#include "pathfinding.h"
#include "location.h"

Pathfinding pathfindingSys;

float Pathfinding::pathLength(Path* path)
{
	float distTot;
	if (path->flags & PQF_UNK2)	return loc->distBtwnLocAndOffs(path->to, path->from) / 12.0;
	distTot = 0;
	auto nodeFrom = path->from;
	for (int i = 0; i < path->nodeCount; i++)
	{
		auto nodeTo = path->nodes[i];
		distTot += loc->distBtwnLocAndOffs(nodeFrom, nodeTo);
		nodeFrom = nodeTo;
	}
	return distTot / 12.0;
}

Pathfinding::Pathfinding() {
	static_assert(sizeof(PathQuery) == 0x50, "Path Query has the wrong size");
	static_assert(sizeof(Path) == 0x1a18, "Path has the wrong size");
	static_assert(sizeof(PathQueryResult) == 0x1a20, "Path Query Result has the wrong size");

	loc = &locSys;

	rebase(FindPath, 0x10043070);
	rebase(pathQArray, 0x1186AC60);
	rebase(pathSthgFlag_10B3D5C8,0x10B3D5C8); 
}
