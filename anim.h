
#pragma once
#include "obj.h"
#include "skill.h"

class AnimationGoals {
public:

	/*
		Pushes a goal for the obj that will rotate it to the given rotation using its rotation speed.
		Please note that the target rotation is absolute, not relative.
	*/
	bool PushRotate(objHndl obj, float rotation);

	/*
		Pushes a goal for the actor to use a certain skill on the given target.
	*/
	bool PushUseSkillOn(objHndl actor, objHndl target, SkillEnum skill, objHndl scratchObj = 0, int goalFlags = 0);

};

extern AnimationGoals animationGoals;
