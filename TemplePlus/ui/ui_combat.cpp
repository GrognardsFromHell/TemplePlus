#include "stdafx.h"
#include "ui_combat.h"

UiCombat uiCombat;

void UiCombat::ActionBarUnsetFlag1(ActionBar* bar)
{
	bar->flags &= ~1;
}

void UiCombat::ActionBarSetMovementValues(ActionBar* bar, float startDist, float endDist, float depletionSpeed)
{
	bar->startDist = startDist;
	bar->pulseVal = startDist;
	bar->advTimeFuncIdx = 0;
	bar->endDist = endDist;
	if (startDist <= endDist)
	{
		bar->combatDepletionSpeed = depletionSpeed;
	} else
	{
		bar->combatDepletionSpeed = -depletionSpeed;
	}
	bar->flags |= 1;
}
