
#pragma once

#include "obj.h"

class AnimGoalsDebugRenderer {
public:

	static void RenderAllAnimGoals(int tileX1, int tileX2, int tileY1, int tileY2);
	static void RenderAnimGoals(objHndl handle);

	static void Enable(bool enable) {
		enabled_ = enable;
	}

	static void EnablePaths(bool enable) {
		pathsEnabled_ = enable;
	}

	static void EnableObjectNames(bool enable) {
		showObjectNames_ = enable;
	}

private:

	static void RenderCurrentGoalPath(objHndl handle);

	static bool enabled_;

	static bool showObjectNames_;

	static bool pathsEnabled_;

};
