#pragma once
#include <common.h>
#include <map>

#define DEITY_COUNT_VANILLA 28 // includes non-selectable ones like Ralishaz and Zuggtmoy

struct LegacyDeitySpec{
	Alignment alignment; // the character's alignment must not oppose this
	Domain domains[7];
	Race races[7]; // races that can pick the deity regardless of anything else; matches obj_f_critter_race; race_human doesn't count
	Stat classes[12]; // // classes that can pick the deity provided the alignment is unopposed; see stat_level_X enums
	int isSelectable;

	bool HasClass(Stat deityClass);
	bool HasRace(Race race);

};


struct DeitySpec:LegacyDeitySpec {
	int id;
	std::string name;
	WeaponTypes favoredWeapon;

	DeitySpec();
	DeitySpec(LegacyDeitySpec&);
};


class LegacyDeitySystem{
	friend class DeitySystem;
public:
	bool CanPickDeity(objHndl handle, int deityId);
	bool DeityHasDomain(int deityId, Domain domain);
	WeaponTypes GetDeityFavoredWeapon(int deityId);

protected:
	DeitySpec &GetDeitySpec(int id);
	LegacyDeitySpec &GetLegacyDeitySpec(int deityId);


	void Init();
	std::map<int, DeitySpec> mDeitySpecs;

	WeaponTypes GetHardcodedFavoredWeapon(Deities deityEnum);
};

extern LegacyDeitySystem deitySys;