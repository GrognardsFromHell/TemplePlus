#pragma once
#include "obj_structs.h"

namespace ContainerArt {
	void Load(const std::string &mapDirectory);
	void Clear();
	bool HasPolygon(objHndl container);
	bool IsHovered(objHndl container);
	float HitArea(objHndl container, float worldX, float worldZ);
	void DrawOutline(objHndl container);
}
