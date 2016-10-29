#include "stdafx.h"
#include "legacydeitysystem.h"
#include "temple/dll.h"
#include "util/fixes.h"
#include <gamesystems/objects/objsystem.h>
#include <critter.h>
#include <gamesystems/d20/d20stats.h>

LegacyDeitySystem deitySys;


class DeityHooks : TempleFix
{
	void apply() override {
		replaceFunction<BOOL(__cdecl)(int, Domain)>(0x1004AA90, [](int deityId, Domain domain)->BOOL	{
			return (BOOL)deitySys.DeityHasDomain(deityId, domain);
		});
	}
} deityHooks;

bool LegacyDeitySystem::CanPickDeity(objHndl handle, int deityId){

	if (!handle)
		return false;

	auto &deitySpec = GetDeitySpec(deityId);
	if (!deitySpec.isSelectable)
		return false;

	auto obj = objSystem->GetObject(handle);
	auto race = (Race)obj->GetInt32(obj_f_critter_race);
	auto alignment = (Alignment) obj->GetInt32(obj_f_critter_alignment);
	auto classEnum = (Stat)obj->GetInt32(obj_f_critter_level_idx, 0);
	auto deityClass = d20ClassSys.GetDeityClass(classEnum);


	auto isCleric = false;
	if (deityClass == stat_level_cleric){
		if (!deityId)
			return false;
		isCleric = true;
	}

	auto hasRace = deitySpec.HasRace(race);
	if (hasRace && !isCleric) // can't have Clerics casting spells of opposed alignments 
		return true;

	// doesn't have race automatic, so check the supported calsses
	auto hasClass = !deitySpec.classes[0] || deitySpec.HasClass(deityClass);

	if (!hasClass && !isCleric) {
		return false;
	}

	// special casing - probably buggy but that's how it was in the original
	if (deitySpec.races[0]){

		auto isException = false;
		if (deityId == 2){  // Corellon Larethian for Bards
			isException = deityClass == stat_level_bard;
		} 
		else if (deityId == 3){ // Ehlonna
			isException = deityClass == stat_level_cleric;
		}

		if (!isException && !hasRace)
			return false;
	}

	// check alignment
	if (deityId == 16) // St. Cuthbert special case
		return (alignment == ALIGNMENT_LAWFUL_GOOD || alignment == ALIGNMENT_LAWFUL);

	auto deiAlign = deitySpec.alignment;
	if (deiAlign == ALIGNMENT_NEUTRAL && deityClass != stat_level_cleric)
		return true;

	if (alignment == deiAlign)
		return true;

	return d20Stats.AlignmentsUnopposed(alignment, deiAlign, true);
}

bool LegacyDeitySystem::DeityHasDomain(int deityId, Domain domain){

	auto &deiSpec = GetDeitySpec(deityId);
	auto foundIt = false;
	for (auto it: deiSpec.domains){
		if (it == Domain::Domain_None)
			break;
		if (it == domain)
			return true;
	}

	return false;
}

DeitySpec & LegacyDeitySystem::GetDeitySpec(int deityId){
	return temple::GetRef<DeitySpec[28]>(0x102726F8)[deityId];
}

bool DeitySpec::HasClass(Stat deityClass)
{
	auto foundIt = false;
	for (auto it : classes) {
		if (it == (Stat)0)
			break;
		if (it == deityClass)
			return true;
	}

	return false;
}

bool DeitySpec::HasRace(Race race)
{
	auto foundIt = false;
	for (auto it : races) {
		if (it == Race::race_human)
			break;
		if (it == race)
			return true;
	}

	return false;
}
