
#include "stdafx.h"
#include "util/fixes.h"
#include "common.h"
#include "temple_functions.h"
#include "obj.h"
#include "gamesystems/objects/objsystem.h"
#include <critter.h>






class BonusSpellFix : public TempleFix {
public:
	static uint32_t _abilityScoreLevelGet(objHndl obj, Stat abScore, DispIO * dispIO)
	{
		return objects.abilityScoreLevelGet(obj, abScore, dispIO);
	}
	
	void apply() override {
		redirectCall(0x100F4C65, _abilityScoreLevelGet);
		//redirectCall(0x1010EFE8, hookedSpellsPendingToMemorized);
	}
} bonusSpellFix;

