
#pragma once
#include "obj.h"

class AnimationGoals {
public:

	/*
		Pushes a goal for the obj that will rotate it to the given rotation using its rotation speed.
		Please note that the target rotation is absolute, not relative.
	*/
	bool PushRotate(objHndl obj, float rotation);
};

extern AnimationGoals animationGoals;
