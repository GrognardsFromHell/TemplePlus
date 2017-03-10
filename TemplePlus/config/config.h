
#pragma once

enum class RngType {
	MERSENNE_TWISTER,
	ARCANUM
};

using ConfigChangedCallback = std::function<void()>;

struct VanillaSetting {
	std::string value;
	ConfigChangedCallback callback;
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
	uint32_t displayAdapter = 0; // Which adapter to use. 0 = default
	uint8_t msaaSamples = 4; // If antialiasing is true
	uint8_t msaaQuality = 0; // For vendor specific AA
	bool mipmapping = false; // Previously -mipmapping
	uint32_t pathLimit = 0; // Default is 10, previously -pathlimit
	uint32_t pathTimeLimit = 0; // Default is 250, previously -pathtimelimit
	RngType rngType = RngType::MERSENNE_TWISTER; // Previously: -rng
	bool showDialogLineNos = false; // Previously: -dialognumber
	uint32_t scrollDistance = 10; // Not really sure what this is used for. Previously: -scrolldist
	std::string defaultModule = "ToEE"; // Previousl: -mod: 
	bool skipIntro = true;
	bool skipLegal = true;
	bool engineEnhancements = true;
	bool pathfindingDebugMode = false;
	bool softShadows = false;
	bool windowed = false;
	bool d3dDebug = false; // Enables D3D11 debugging
	bool lockCursor = true; // When in fullscreen, lock cursor
	bool windowedLockCursor = false; // lock cursor anyway
	int windowWidth = 1024;
	int windowHeight = 768;
	int renderWidth = 800; // will set to window size on first run
	int renderHeight = 600;
	std::wstring toeeDir;
	int sectorCacheSize = 128; // Default is now 128 (ToEE was 16)
	int screenshotQuality = 80; // 1-100, Default is 80
	bool debugPartSys = false;
	bool debugClipping = true;
	bool drawObjCylinders = false;

	bool autoUpdate = true;
	std::string autoUpdateFeed = "https://templeplus.org/update-feeds/stable/";

	std::vector<std::string> additionalTioPaths; // Add these to the Python sys.path variable
	
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
	bool showNpcStats = false;
	bool showExactHPforNPCs = false; // draw exact HP for NPCs
	int pointBuyPoints = 25; // number of Point Buy points at chargen
	uint32_t maxPCs = 5; // max number of PCs in the party
	bool maxPCsFlexible = true; // makes the party PC/NPC composition fluid
	uint32_t maxLevel = 10; // maximum character level
	std::string hpOnLevelup = "Normal" ;
	bool allowXpOverflow = false;
	bool slowerLevelling = false;
	bool laxRules = false; // Relaxed restrictions for various things
	bool newClasses = false; // Prestige classes and such
	bool nonCoreMaterials = false; // splatbooks, fan suggestions etc
	bool tolerantNpcs = false; // NPCs tolerate monster party members
	std::string fogOfWar = "Normal";
	bool disableFogOfWar = false; // Previously: -nofog
	double speedupFactor = 1.0;
	bool equalizeMoveSpeed = true;

	std::unordered_map<std::string, VanillaSetting> vanillaSettings;
	void AddVanillaSetting(const std::string &name, 
		const std::string &defaultValue, 
		ConfigChangedCallback changeCallback = ConfigChangedCallback());
	void RemoveVanillaCallback(const std::string &name);
	int GetVanillaInt(const std::string &name) const;
	std::string GetVanillaString(const std::string &name) const;
	void SetVanillaString(const std::string &name, const std::string &value);
	void SetVanillaInt(const std::string &name, int value);

	void Load();
	void Save();

	std::string GetPath();
	void SetPath(const std::string &path);
	
};

extern TemplePlusConfig config;
