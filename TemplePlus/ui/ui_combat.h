#pragma once
#include <temple/dll.h>


struct ActionBar
{
	int advTimeFuncIdx;
	int flags; // 1 - active
	float pulseVal;
	void(__cdecl *resetCallback)(int);
	int resetArg;
	float startDist;
	float endDist;
	float combatDepletionSpeed;
	float pulsePhaseRadians;
	float pulseMean;
	float pulseAmplitude;
	float pulseTime;
};

// const int testsieofActionbar = sizeof(ActionBar); // should be 48

class UiCombat
{
public:
	void ActionBarUnsetFlag1(ActionBar* bar);
	void ActionBarSetMovementValues(ActionBar* bar, float startDist, float endDist, float depletionSpeed);
};

extern UiCombat uiCombat;