
#pragma once

enum class RngType {
	MERSENNE_TWISTER,
	ARCANUM
};

struct TemplePlusConfig
{
	bool showFps = false; // Previously -fps
	bool noSound = false; // Previously -nosound
	bool doublebuffer = false; // Previously -doublebuffer
	bool animCatchup = false; // Previously -animcatchup
	bool noRandomEncounters = false; // Previously -norandom
	bool noMsMouseZ = false; // Previously -nomsmousez
	bool antialiasing = true; // Previously -noantialiasing
	bool mipmapping = false; // Previously -mipmapping
	uint32_t pathLimit = 0; // Default is 10, previously -pathlimit
	uint32_t pathTimeLimit = 0; // Default is 250, previously -pathtimelimit
	RngType rngType = RngType::MERSENNE_TWISTER; // Previously: -rng
	bool showDialogLineNos = false; // Previously: -dialognumber
	uint32_t scrollDistance = 10; // Not really sure what this is used for. Previously: -scrolldist
	string defaultModule = "ToEE"; // Previousl: -mod: 
	bool disableFogOfWar = false; // Previously: -nofog
	bool disableScreenshots = false; // for debugging mushaden's crash
	bool skipIntro = true;
	bool skipLegal = true;
	bool engineEnhancements = true;
	bool useDirect3d9Ex = true;
	bool windowed = true;
	bool lockCursor = true; // When in fullscreen, lock cursor
	int windowWidth = 1024;
	int windowHeight = 768;
	int renderWidth = 1024;
	int renderHeight = 768;
	wstring toeeDir;
	int sectorCacheSize = 128; // Default is 16

	vector<string> additionalTioPaths; // Add these to the Python sys.path variable
	
	// This is some crazy editor stuff leftover from worlded
	bool editor = false;

	// debug msgs
	bool debugMessageEnable = true; // ToEE debug spam
	bool featPrereqWarnings = false;
	bool spellAlreadyKnownWarnings = false;

	// gameplay
	// double randomEncounterExperienceFactor = 0.7; // an additional factor; e.g. if the normal Experience Multiplier is 0.7 and this is 0.7, overall is 0.49 NOT YET IMPLEMENTED
	bool newFeatureTestMode = true;
	bool NPCsLevelLikePCs = true;
	bool showExactHPforNPCs = false; // draw exact HP for NPCs
	int pointBuyPoints = 25; // number of Point Buy points at chargen
	int maxPCs = 5; // max number of PCs in the party
	bool maxPCsFlexible = true; // makes the party PC/NPC composition fluid
	bool usingCo8 = true;
	void Load();
	void Save();
};

extern TemplePlusConfig config;
