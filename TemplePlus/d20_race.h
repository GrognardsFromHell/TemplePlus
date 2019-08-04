#pragma once
#include "d20_defs.h"
#include "ui/ui_chargen.h"
#include "critter.h"
#include "ui/ui_pc_creation.h"


class RaceSpec;


class D20RaceSys
{
public:

	enum RaceDefinitionFlags
	{
		RDF_Vanilla = 1,
		RDF_Monstrous = 2,
		RDF_ForgottenRealms = 4
	};

	int vanillaRaceEnums[VANILLA_NUM_RACES];
	std::vector<int> raceEnums;
	std::vector<int> baseRaceEnums;
	std::vector<int> selectableBaseRaces;

	// initialization
	D20RaceSys();
	void GetRaceSpecsFromPython();
	void RegisterRace(const RaceSpec& spec, int raceEnum); // used by the Python layer to add a RaceSpec object to the registry



	int GetStatModifier(Race race, int stat); // e.g. +2 CON for Dwarves
	HairStyleRace GetHairStyle(Race race); // get racial hair style from among those available in ToEE
	RaceBase GetBaseRace(Race race); // Gets base race enum from compound race ID
	Subrace GetSubrace(Race race); // Gets subrace enum from compound race ID
	bool UseBaseRaceForDeity(Race race);  //Flag to treat diety selections the same as the base rase for the subrace
	int GetProtoId(Race race);
	std::string& GetHelpTopic(Race race);
	Race GetRaceEnum(const std::string& raceName);
	bool BonusFirstLevelFeat(Race race);
	int GetMinHeight(Race race, Gender genderId);
	int GetMaxHeight(Race race, Gender genderId);
	int GetMinWeight(Race race, Gender genderId);
	int GetMaxWeight(Race race, Gender genderId);
	bool HasSubrace(Race race);
	const std::vector<Race> GetSubraces(RaceBase raceBase);
	float GetModelScale(Race race, int genderId);
	int GetRaceMaterialOffset(Race race); // index into rules/material.mes  (or rules/material_ext.mes for non-vanilla races)
	std::string GetRaceCondition(Race race);
	int GetLevelAdjustment( objHndl& objHnd);
	Dice GetHitDice(Race race);
	bool IsVanillaRace(Race race);
	std::vector<int> GetStatModifiers(Race race);
	bool HasFeat(Race race, feat_enums featEnum);
	int GetNaturalArmor(Race race);
	std::map<SpellStoreData, int> GetSpellLikeAbilities(Race race);
	

protected:
	RaceSpec & GetRaceSpec(Race race);
	std::map<Race, RaceSpec> mRaceSpecs;

};


extern D20RaceSys d20RaceSys;
