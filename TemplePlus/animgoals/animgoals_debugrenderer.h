
#pragma once

#include "obj.h"

class AnimGoalsDebugRenderer {
public:

	static void RenderAllAnimGoals(int tileX1, int tileX2, int tileY1, int tileY2);
	static void RenderAnimGoals(objHndl handle);

	static void Enable(bool enable) {
		enabled_ = enable;
	}

private:

	static bool enabled_;

};
