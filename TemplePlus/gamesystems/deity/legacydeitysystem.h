#pragma once
#include <common.h>

struct DeitySpec{
	Alignment alignment; // the character's alignment must not oppose this
	Domain domains[7];
	Race races[7]; // races that can pick the deity regardless of anything else; matches obj_f_critter_race; race_human doesn't count
	Stat classes[12]; // // classes that can pick the deity provided the alignment is unopposed; see stat_level_X enums
	int isSelectable;

	bool HasClass(Stat deityClass);
	bool HasRace(Race race);
};

class LegacyDeitySystem
{
public:
	bool CanPickDeity(objHndl handle, int deityId);
	bool DeityHasDomain(int deityId, Domain domain);

protected:
	DeitySpec &GetDeitySpec(int deityId);

};

extern LegacyDeitySystem deitySys;