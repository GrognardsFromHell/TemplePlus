#include "stdafx.h"
#include "d20_race.h"
#include "temple_enums.h"
#include "python/python_integration_race.h"

#include <pybind11/embed.h>
#include <pybind11/cast.h>
#include <pybind11/stl.h>
#include "python/python_object.h"
#include "python/python_dice.h"
#include "ui/ui_char_editor.h"
#include "config/config.h"
#include "python/python_spell.h"

namespace py = pybind11;

class RaceSpec
{
public:
	std::vector<int> statModifiers;
	int effectiveLevel = 0; // modifier for Effective Character Level (determines XP requirement)
	std::string helpTopic;  // helpsystem id ("TAG_xxx")
	int flags;              // D20RaceSys::RaceDefinitionFlags 
	Dice hitDice;
	int naturalArmor = 0;
	std::vector<int> feats; // feat enums; for entering new-style feats in python, use tpdp.hash
	std::map<SpellStoreData, int> spellLikeAbilities;

	int protoId;            // protos.tab entry (male; female is +1)
	int materialOffset = 0; // offset for rules/materials.mes (or materials_ext.mes for non-vanilla races)

	std::vector<int> weightMale = { 80,100 };
	std::vector<int> weightFemale = { 80,100 };
	std::vector<int> heightMale = { 70,80 };
	std::vector<int> heightFemale = { 70,80 };

	// Main Race
	RaceBase baseRace = race_base_human;
	bool isBaseRace = true;
	std::vector<Race> subraces;

	// Subrace
	Subrace subrace = subrace_none;

	std::string conditionName;
	

	RaceSpec(){
		statModifiers = { 0,0,0,0,0,0 };
		hitDice = Dice(0, 0, 0);
		
	}

	bool IsEnabled();
};

D20RaceSys d20RaceSys;


template <> class py::detail::type_caster<Dice> {
public:
	bool load(handle src, bool) {
		ConvertDice(src.ptr(), &value);
		success = true;
		return true;
	}

	static handle cast(const Dice &src, return_value_policy /* policy */, handle /* parent */) {
		return PyDice_FromDice(src);
	}

	PYBIND11_TYPE_CASTER(Dice, _("Dice"));
protected:
	bool success = false;
};

template <> class py::detail::type_caster<SpellStoreData> {
public:
	bool load(handle src, bool) {
		SpellStoreData spData;
		ConvertSpellStore(src.ptr(), &spData);
		value = spData;
		success = true;
		return true;
	}

	static handle cast(const SpellStoreData& src, return_value_policy
		/* policy */, handle
	/* parent */) {
		return PySpellStore_Create(src);
	}

	PYBIND11_TYPE_CASTER(SpellStoreData, _("PySpellStore"));
protected:
	bool success = false;
};

PYBIND11_EMBEDDED_MODULE(race_defs, m) {

	m.doc() = "Temple+ Char Editor, used for extending the ToEE character editor.";

	m.def("GetRaceFileList", []() {
		auto result = std::vector<std::string>();
		TioFileList flist;
		tio_filelist_create(&flist, "rules\\races\\race*.py");

		for (auto i = 0; i < flist.count; i++) {
			auto &f = flist.files[i];
			if (!_strcmpi(f.name, "__init__.py"))
				continue;
			for (auto ch = f.name; *ch; ch++)
			{
				if (*ch == '.')
				{
					*ch = 0;
					break;
				}

			}
			result.push_back(f.name);
		}

		tio_filelist_destroy(&flist);
		return result;
	});

	m.def("GetStatModifiers", [](int raceEnum)-> std::vector<int>{
		auto result = d20RaceSys.GetStatModifiers((Race)raceEnum);
		return result;
	});

	py::class_<RaceSpec>(m, "RaceSpec")
		.def(py::init<>())
		.def_readwrite("hit_dice", &RaceSpec::hitDice)
		.def_readwrite("modifier_name", &RaceSpec::conditionName)
		.def_readwrite("flags", &RaceSpec::flags)
		.def_readwrite("level_modifier", &RaceSpec::effectiveLevel)
		.def_readwrite("height_male", &RaceSpec::heightMale)
		.def_readwrite("height_female", &RaceSpec::heightFemale)
		.def_readwrite("weight_male", &RaceSpec::weightMale)
		.def_readwrite("weight_female", &RaceSpec::weightFemale)
		.def_readwrite("stat_modifiers", &RaceSpec::statModifiers)
		.def_readwrite("natural_armor", &RaceSpec::naturalArmor)
		.def_readwrite("help_topic", &RaceSpec::helpTopic)
		.def_readwrite("proto_id", &RaceSpec::protoId)
		.def_readwrite("material_offset", &RaceSpec::materialOffset)
		.def_readwrite("feats", &RaceSpec::feats)
		.def_readwrite("spell_like_abilities", &RaceSpec::spellLikeAbilities)
		//.def_readwrite("", &RaceSpec::)
		.def("register", [](RaceSpec& spec, int raceEnum){
		d20RaceSys.RegisterRace(spec, raceEnum);
		});
		;
}

D20RaceSys::D20RaceSys()
{
	int _charRaceEnums[VANILLA_NUM_RACES] =
	{ Race::race_human, Race::race_dwarf, Race::race_elf,
		Race::race_gnome, Race::race_halfelf, Race::race_half_orc,
		Race::race_halfling	 };
	memcpy(vanillaRaceEnums, _charRaceEnums, VANILLA_NUM_RACES * sizeof(uint32_t));

	for (auto it:vanillaRaceEnums){
		mRaceSpecs[(Race)it] = RaceSpec();
	}

	mRaceSpecs[Race::race_dwarf].statModifiers[Stat::stat_constitution] = 2;
	mRaceSpecs[Race::race_dwarf].statModifiers[Stat::stat_charisma] = -2;

	mRaceSpecs[Race::race_elf].statModifiers[Stat::stat_constitution] = -2;
	mRaceSpecs[Race::race_elf].statModifiers[Stat::stat_dexterity] = 2;

	mRaceSpecs[Race::race_gnome].statModifiers[Stat::stat_constitution] = 2;
	mRaceSpecs[Race::race_gnome].statModifiers[Stat::stat_strength] = -2;

	mRaceSpecs[Race::race_half_orc].statModifiers[Stat::stat_charisma] = -2;
	mRaceSpecs[Race::race_half_orc].statModifiers[Stat::stat_strength] = 2;

	mRaceSpecs[Race::race_halfling].statModifiers[Stat::stat_dexterity] = 2;
	mRaceSpecs[Race::race_halfling].statModifiers[Stat::stat_strength] = -2;

}

void D20RaceSys::GetRaceSpecsFromPython(){
	auto racesModule = PyImport_ImportModule("races");

	
	std::sort(raceEnums.begin(), raceEnums.end());

	// Compile subrace lists
	for (auto it : mRaceSpecs){
		auto &entry = it.second;
		if (entry.isBaseRace) continue;

		mRaceSpecs[(Race)entry.baseRace].subraces.push_back(it.first);
	}

	for (auto it : baseRaceEnums){
		auto &entry = mRaceSpecs[(Race)it];
		auto isOk = false;
		if (entry.flags & RDF_Vanilla)
			isOk = true;
		else {
			if ( (entry.flags & RDF_Monstrous) && !config.monstrousRaces){
				continue;
			}
			if (!config.newRaces){
				continue;
			}
			isOk = true;
		}
			
		if (isOk) selectableBaseRaces.push_back(it);
	}

}

int D20RaceSys::GetStatModifier(Race race, int stat) {
	auto &spec = GetRaceSpec(race);
	if (stat <= stat_charisma)
		return spec.statModifiers[stat];
	return 0;
}

HairStyleRace D20RaceSys::GetHairStyle(Race race){
	if (race >= VANILLA_NUM_RACES){
		race = (Race)GetBaseRace(race);
	}
	switch (race){
	case race_human:
		return HairStyleRace::Human;
	case race_dwarf:
		return HairStyleRace::Dwarf;
	case race_elf:
		return HairStyleRace::Elf;
	case race_gnome:
		return HairStyleRace::Gnome;
	case race_half_elf:
		return HairStyleRace::HalfElf;
	case race_half_orc:
		return HairStyleRace::HalfOrc;
	case race_halfling:
		return HairStyleRace::Halfling;
	default:
		return HairStyleRace::Human;
	}
}

RaceBase D20RaceSys::GetBaseRace(Race race){
	return (RaceBase)((int)race & 0x1F);
}

Subrace D20RaceSys::GetSubrace(Race race) {
	return (Subrace)((int)race >> 5);
}

int D20RaceSys::GetProtoId(Race race){

	if (race < VANILLA_NUM_RACES){
		return 13000 + race * 2;
	}
	
	auto &raceSpec = GetRaceSpec(race);
	return raceSpec.protoId;
}

Race D20RaceSys::GetRaceEnum(const std::string & raceName)
{
	for (auto it : mRaceSpecs){
		if (it.second.conditionName == raceName){
			return it.first;
		}
	}
	auto result= temple::GetRef<Race(__cdecl)(const char*)>(0x10073B70)(raceName.c_str());
	return result;
}

int D20RaceSys::GetMinHeight(Race race, Gender genderId)
{
	if (race <= VANILLA_NUM_RACES)
		return temple::GetRef<int[]>(0x102EFCE0)[2*(int)race + (int)genderId];
	auto &raceSpecs = GetRaceSpec(race);
	if (genderId == Gender::Male)
		return raceSpecs.heightMale[0];
	else
		return raceSpecs.heightFemale[0];
}

int D20RaceSys::GetMaxHeight(Race race, Gender genderId)
{
	if (race <= VANILLA_NUM_RACES)
		return temple::GetRef<int[]>(0x102EFD18)[2 * (int)race + (int)genderId];
	auto &raceSpecs = GetRaceSpec(race);
	if (genderId == Gender::Male)
		return raceSpecs.heightMale[1];
	else
		return raceSpecs.heightFemale[1];
}

int D20RaceSys::GetMinWeight(Race race, Gender genderId)
{
	if (race <= VANILLA_NUM_RACES)
		return temple::GetRef<int[]>(0x102EFD50)[2 * (int)race + (int)genderId];
	auto &raceSpecs = GetRaceSpec(race);
	if (genderId == Gender::Male)
		return raceSpecs.weightMale[0];
	else
		return raceSpecs.weightFemale[0];
}

int D20RaceSys::GetMaxWeight(Race race, Gender genderId)
{
	if (race <= VANILLA_NUM_RACES)
		return temple::GetRef<int[]>(0x102EFD88)[2 * (int)race + (int)genderId];
	auto &raceSpecs = GetRaceSpec(race);
	if (genderId == Gender::Male)
		return raceSpecs.weightMale[1];
	else
		return raceSpecs.weightFemale[1];
}

bool D20RaceSys::HasSubrace(Race race){
	auto &raceSpec = GetRaceSpec(race);
	if (!raceSpec.isBaseRace){
		return false;
	}
	for (auto it : raceSpec.subraces) {
		if (mRaceSpecs[it].IsEnabled())
			return true;
	}
	return false;
}

const std::vector<Race> D20RaceSys::GetSubraces(RaceBase raceBase)
{
	auto race = (Race)raceBase;
	auto &raceSpec = GetRaceSpec(race);
	auto result = std::vector<Race>();
	for (auto it: raceSpec.subraces){
		if (mRaceSpecs[it].IsEnabled())
			result.push_back(it);
	}

	return result;
}

float D20RaceSys::GetModelScale(Race race, int genderId){
	if (race < VANILLA_NUM_RACES){
		return temple::GetRef<float[14]>(0x102FE188)[race * 2 + genderId];
	}

	auto raceBase = GetBaseRace(race);
	if (raceBase < VANILLA_NUM_RACES && raceBase >= 0)
		return GetModelScale((Race)raceBase, genderId);

	return 1.0f;
}

int D20RaceSys::GetRaceMaterialOffset(Race race){
	if (race < VANILLA_NUM_RACES){
		switch (race){
		case race_human: 
			return 0;
		case race_dwarf: 
			return 6;
		case race_elf: 
			return 2;
		case race_gnome: 
			return 8;
		case race_half_elf: 
			return 10;
		case race_half_orc: 
			return 4;
		case race_halfling: 
			return 12;
		default: 
			return 0; // human
		}
	}
	
	auto &raceSpec = GetRaceSpec(race);
	return raceSpec.materialOffset;
}

std::string D20RaceSys::GetRaceCondition(Race race){
	static std::vector<std::string> raceCondNames = { "Human", "Dwarf", "Elf", "Gnome", "Halfelf", "Halforc", "Halfling" };
	if (race < VANILLA_NUM_RACES){
		return raceCondNames[race];
	}

	auto &raceSpec = GetRaceSpec(race);
	return raceSpec.conditionName;
}

int D20RaceSys::GetLevelAdjustment(objHndl & objHnd)
{
	auto race = critterSys.GetRace(objHnd, false);
	auto &raceSpec = GetRaceSpec(race);
	auto lvlAdj = raceSpec.effectiveLevel;
	return lvlAdj;
}


Dice D20RaceSys::GetHitDice(Race race){
	auto &raceSpec = GetRaceSpec(race);
	return raceSpec.hitDice;
}

bool D20RaceSys::IsVanillaRace(Race race){
	auto &raceSpec = GetRaceSpec(race);
	return (raceSpec.flags & RDF_Vanilla) != 0;
}

std::vector<int> D20RaceSys::GetStatModifiers(Race race)
{
	auto &raceSpec = GetRaceSpec(race);
	return raceSpec.statModifiers;
}

bool D20RaceSys::HasFeat(Race race, feat_enums featEnum){
	auto &raceSpec = GetRaceSpec(race);
	for (auto &it:raceSpec.feats){
		if (it == featEnum)
			return true;
	}
	return false;
}

int D20RaceSys::GetNaturalArmor(Race race){
	auto &raceSpec = GetRaceSpec(race);
	return raceSpec.naturalArmor;
}

std::map<SpellStoreData, int> D20RaceSys::GetSpellLikeAbilities(Race race){
	auto &raceSpec = GetRaceSpec(race);
	return raceSpec.spellLikeAbilities;
}

void D20RaceSys::RegisterRace(const RaceSpec & spec, int raceEnum){
	
	for (auto it: raceEnums){
		if (it == raceEnum){
			logger->error("Duplicate race enum {} found! Race spec was for {}.", raceEnum, spec.conditionName);	
			throw TempleException("Duplicate races definition found");
		}
	}
	raceEnums.push_back(raceEnum);

	auto race = (Race)raceEnum;
	RaceSpec &raceSpec = mRaceSpecs[race];

	raceSpec = spec;

	raceSpec.baseRace = GetBaseRace(race);
	raceSpec.subrace = GetSubrace(race);
	raceSpec.isBaseRace = raceSpec.subrace == subrace_none;

	if (d20StatusSys.raceCondMap.find(race) == d20StatusSys.raceCondMap.end()) { // if condition name was not found in the d20Status mapping , update it
		d20StatusSys.raceCondMap[race] = raceSpec.conditionName;
	}

	if (raceSpec.isBaseRace) {
		baseRaceEnums.push_back(raceEnum);
	}


	logger->info(fmt::format("Registered race {}", raceEnum).c_str());
}

RaceSpec& D20RaceSys::GetRaceSpec(Race race){
	if (mRaceSpecs.find(race) != mRaceSpecs.end()) {
		return mRaceSpecs[race];
	}
	logger->error(fmt::format("Bad race input {}, returning human instead", (int)race).c_str());
	return mRaceSpecs[Race::race_human];
}

bool RaceSpec::IsEnabled()
{
	if (flags && D20RaceSys::RaceDefinitionFlags::RDF_Vanilla)
		return true;
	if (flags && D20RaceSys::RaceDefinitionFlags::RDF_Monstrous && !config.monstrousRaces)
		return false;
	return config.newRaces;
}
