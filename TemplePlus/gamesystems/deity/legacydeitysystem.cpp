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

	auto &deitySpec = GetLegacyDeitySpec(deityId);
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

	auto &deiSpec = GetLegacyDeitySpec(deityId);
	auto foundIt = false;
	for (auto it: deiSpec.domains){
		if (it == Domain::Domain_None)
			break;
		if (it == domain)
			return true;
	}

	return false;
}

WeaponTypes LegacyDeitySystem::GetDeityFavoredWeapon(int deityId){
	auto &ds = GetDeitySpec(deityId);
	return ds.favoredWeapon;
}

bool LegacyDeitySystem::IsDomainSkill(objHndl handle, SkillEnum skill){
	auto obj = objSystem->GetObject(handle);

	auto dom1 = (Domain)obj->GetInt32(obj_f_critter_domain_1);
	auto dom2 = (Domain)obj->GetInt32(obj_f_critter_domain_2);

	return (IsDomainSkill(skill, dom1) || IsDomainSkill(skill, dom2));
}

bool LegacyDeitySystem::IsDomainSkill(SkillEnum skill, Domain dom){

	if (dom == Domain_Trickery){
		return (skill == skill_bluff || skill == skill_disguise || skill == skill_hide);
	}

	if (dom == Domain_Knowledge){
		return (skill == skill_knowledge_all || skill == skill_knowledge_arcana || skill == skill_knowledge_religion);
	}

	if (dom == Domain_Travel){
		return skill == skill_wilderness_lore;
	}

	if (dom == Domain_Animal || dom == Domain_Plant)
		return skill == skill_knowledge_nature;

	return false;
}

const char* LegacyDeitySystem::GetName(int deityId){
	MesLine line(deityId);
	auto mesHandle = temple::GetRef<MesHandle>(0x10AA3228);
	mesFuncs.GetLine_Safe(mesHandle, &line);
	return line.value;
}

DeitySpec & LegacyDeitySystem::GetDeitySpec(int id){
	return mDeitySpecs[id];
}

LegacyDeitySpec & LegacyDeitySystem::GetLegacyDeitySpec(int deityId){
	return temple::GetRef<LegacyDeitySpec[28]>(0x102726F8)[deityId];
}

void LegacyDeitySystem::Init(){
	for (auto i=0; i < DEITY_COUNT_VANILLA; i++)	{
		auto deityEnum = (Deities)i;

		DeitySpec s(GetLegacyDeitySpec(i));
		s.id = i;
		s.favoredWeapon = GetHardcodedFavoredWeapon(deityEnum);
		
		mDeitySpecs[i] = s;
	}


}

WeaponTypes LegacyDeitySystem::GetHardcodedFavoredWeapon(Deities deityEnum){
	// based on the help wiki and http://www.imarvintpa.com/dndlive/Index_Deities.php
	switch (deityEnum) {
	case DEITY_NONE:
		return wt_bastard_sword; // easter egg :)

	case DEITY_BOCCOB: 
		return wt_quarterstaff;

	case DEITY_CORELLON_LARETHIAN: 
		return wt_longsword;

	case DEITY_EHLONNA: 
		return wt_longbow;

	case DEITY_ERYTHNUL: 
		return wt_morningstar;
	
	case DEITY_FHARLANGHN: 
		return wt_quarterstaff;

	case DEITY_GARL_GLITTERGOLD: 
		return wt_battleaxe;
	
	case DEITY_GRUUMSH:
		return wt_longspear;
	
	case DEITY_HEIRONEOUS:
		return wt_longsword;
	
	case DEITY_HEXTOR:
		return wt_heavy_flail;
	
	case DEITY_KORD:return wt_greatsword;
	
	case DEITY_MORADIN:return wt_warhammer;
	
	case DEITY_NERULL:return wt_scythe;
	
	case DEITY_OBAD_HAI:return wt_quarterstaff;
	
	case DEITY_OLIDAMMARA:return wt_rapier;
	
	case DEITY_PELOR:return wt_heavy_mace;
	
	case DEITY_ST_CUTHBERT:return wt_heavy_mace;
	
	case DEITY_VECNA:return wt_dagger;
	
	case DEITY_WEE_JAS:return wt_dagger;
	
	case DEITY_YONDALLA:return wt_short_sword;
	
	case DEITY_OLD_FAITH:return wt_quarterstaff; // taken to match Shillelagh, also seems appropriate
	
	case DEITY_ZUGGTMOY:return wt_scythe;
	
	case DEITY_IUZ:return wt_greatsword;
	
	case DEITY_LOLTH:return wt_whip;
	
	case DEITY_PROCAN:return wt_trident;
	
	case DEITY_NOREBO:return wt_dagger; // https://en.wikipedia.org/wiki/Norebo
	
	case DEITY_PYREMIUS:return wt_longsword;
	
	case DEITY_RALISHAZ:return wt_heavy_mace;
	
	default: return wt_quarterstaff;
	}
}

bool LegacyDeitySpec::HasClass(Stat deityClass)
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

bool LegacyDeitySpec::HasRace(Race race)
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

DeitySpec::DeitySpec(){
	alignment = ALIGNMENT_NEUTRAL;
	domains[0] = Domain_None;
	races[0] = race_human;
	classes[0] = (Stat)0;
	isSelectable = 0;
}

DeitySpec::DeitySpec(LegacyDeitySpec & legacySpec){
	this->alignment = legacySpec.alignment;
	memcpy(this->domains, legacySpec.domains, sizeof(this->domains));
	memcpy(this->classes, legacySpec.classes, sizeof(this->classes));
	memcpy(this->races, legacySpec.races, sizeof(this->races));
	this->isSelectable = legacySpec.isSelectable;
}
