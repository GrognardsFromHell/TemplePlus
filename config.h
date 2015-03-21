
#pragma once

struct TemplePlusConfig
{
	bool skipIntro = true;
	bool skipLegal = true;
	bool useDirect3d9Ex = true;
	bool windowed = true;

	// debug msgs
	bool featPrereqWarnings = false;
	bool spellAlreadyKnownWarnings = false;

	// gameplay
	double randomEncounterExperienceFactor = 0.7; // an additional factor; e.g. if the normal Experience Multiplier is 0.7 and this is 0.7, overall is 0.49
	bool NPCsLevelLikePCs = true;
};

extern TemplePlusConfig config;
