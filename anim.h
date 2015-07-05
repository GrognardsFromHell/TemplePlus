
#pragma once
#include "obj.h"
#include "skill.h"

enum AnimGoalPriority {
	AGP_NONE = 0,
	AGP_1 = 1,
	AGP_2 = 2,
	AGP_3 = 3,
	AGP_4 = 4,
	AGP_5 = 5,
	AGP_HIGHEST = 6,
	AGP_7 = 7,	
	AGP_MAX = 8
};

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

	/*
		Pushes a goal for the actor to run near a certain tile.
	*/
	bool PushRunNearTile(objHndl actor, LocAndOffsets target, int radiusFeet);

	/*
		Pushes a goal to play the unconceal animation and unconceal the critter.
	*/
	bool PushUnconceal(objHndl actor);

	/*
		Interrupts currently running animation goals up to the given priority.
	*/
	bool Interrupt(objHndl actor, AnimGoalPriority priority, bool all = false);

	void PushFallDown(objHndl actor, int unk);

	int PushAttackAnim(objHndl actor, objHndl target, int unk1, int hitAnimIdx, int playCrit, int useSecondaryAnim);
	int GetAnimIdSthgSub_1001ABB0(objHndl objHndl);
	int PushAttemptAttack(objHndl attacker, objHndl defender);
};

extern AnimationGoals animationGoals;
