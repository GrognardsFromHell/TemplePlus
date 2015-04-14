
#include "stdafx.h"
#include "pathfinding.h"

Pathfinding pathfindingSys;

Pathfinding::Pathfinding() {
	static_assert(sizeof(PathQuery) == 0x50, "Path Query has the wrong size");
	static_assert(sizeof(Path) == 0x1a18, "Path has the wrong size");
	static_assert(sizeof(PathQueryResult) == 0x1a20, "Path Query Result has the wrong size");

	rebase(FindPath, 0x10043070);
	rebase(pathQArray, 0x1186AC60);
}
