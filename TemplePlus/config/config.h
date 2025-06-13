
#pragma once

#include <unordered_map>
#include <unordered_set>

enum class RngType {
	MERSENNE_TWISTER,
	ARCANUM
};

using ConfigChangedCallback = std::function<void()>;

struct VanillaSetting {
	std::string value;
	ConfigChangedCallback callback;
};

// Recognized sources for spells etc.
//
// Not a flag type, but making the codes sparse makes it easier to
// classify them when filtering.
//
// Convention:
//   0-255 core
//   256-511 offical non-core
//   512+ unofficial
enum class PnPSource : uint32_t
{
	PHB = 0,
	ToEE = 1,
	SpellCompendium = 0x100,
	PHB2 = 0x101,
	Homebrew = 0x200,
	Co8 = 0x201
};


// Calculates a source for a spell based on its numbering, as a default.
PnPSource DefaultSpellSource(int spellEnum);

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
	bool upscaleLinearFiltering = true;
	bool enlargeDialogFonts = false;
	std::wstring toeeDir;
	int sectorCacheSize = 128; // Default is now 128 (ToEE was 16)
	int screenshotQuality = 80; // 1-100, Default is 80
	bool debugPartSys = false;
	bool debugClipping = false;
	bool drawObjCylinders = false;
	bool newAnimSystem = false;
	float dmGuiScale = 1.0f;

	bool autoUpdate = true;
	std::string autoUpdateFeed = "https://templeplus.org/update-feeds/stable/";

	std::vector<std::string> additionalTioPaths; // Add these to the Python sys.path variable
	
	// This is some crazy editor stuff leftover from worlded
	bool editor = false;

	// debug settings
	int logLevel = 1; // debug; set to trace (0) for more
	bool debugMessageEnable = true; // ToEE debug spam
	bool featPrereqWarnings = false;
	bool spellAlreadyKnownWarnings = false;
	bool dumpFullMemory = false;
	bool debugObjects = false; // enables debugging game objects
	bool newFeatureTestMode = false; // not really used right now

	// gameplay 
	// double randomEncounterExperienceFactor = 0.7; // an additional factor; e.g. if the normal Experience Multiplier is 0.7 and this is 0.7, overall is 0.49 NOT YET IMPLEMENTED
	bool dungeonMaster = false;
	bool NPCsLevelLikePCs = true;
	bool showNpcStats = false;
	bool showExactHPforNPCs = false; // draw exact HP for NPCs
	bool showHitChances = false; // show hit chance tooltips for all attacks

	int pointBuyPoints = 25; // number of Point Buy points at chargen
	uint32_t maxPCs = 5; // max number of PCs in the party
	bool maxPCsFlexible = false; // makes the party PC/NPC composition fluid
	uint32_t maxLevel = 10; // maximum character level
	std::string hpOnLevelup = "Normal";
	std::string HpForNPCHd = "Normal";
	bool disableTargetSurrounded = false;
	bool maxHpForNpcHitdice = false;
	bool allowXpOverflow = false;
	bool slowerLevelling = false;
	bool laxRules = false; // Relaxed restrictions for various things; this also acts as a master switch
	bool stricterRulesEnforcement = false; //Stricter rules enforcement for things such as the size of the grease spell
	bool preferPoisonSpecFile = false; // don't load vanilla poisons
	bool disableMulticlassXpPenalty = false;
	bool disableCraftingSpellReqs = false;
	bool showTargetingCirclesInFogOfWar = false;
	bool disableAlignmentRestrictions = false;
	bool disableDoorRelocking = false;
	bool dialogueUseBestSkillLevel = false; // uses best skill level from the (PC) group in dialogue checks
	bool disableReachWeaponDonut = false; // set to true to restore vanilla ToEE reach weapon behavior

	bool newClasses = false; // Prestige classes and such
	bool newRaces = false; // Drow etc.
	bool metamagicStacking = false; // Allows stacking of Meta Magic feats
	bool monstrousRaces = false; // monstrous races. unbalanced as hell ><
	bool forgottenRealmsRaces = false;  //Races from the forgotten realms campaign setting (Gold Dwarf, Genasi, ...)
	bool nonCoreMaterials = false; // splatbooks, fan suggestions etc
	std::unordered_set<PnPSource> nonCoreSources;
	bool tolerantNpcs = false; // NPCs tolerate monster party members
	std::string fogOfWar = "Normal";
	bool disableFogOfWar = false; // Previously: -nofog
	double speedupFactor = 1.0;
	bool equalizeMoveSpeed = true;
	bool fastSneakAnim = false;
	bool disableScreenShake = false; // disables screen shakes from animation
	bool alertAiThroughDoors = false;
	bool preferUse5FootStep = false;
	bool extendedSpellDescriptions = false;
	int walkDistanceFt = 0;
	bool disableChooseRandomSpell_RegardInvulnerableStatus = false;
	bool wildShapeUsableItems = false; // allows some worn items to be used
	int npcStatBoost = 0;

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

	TemplePlusConfig() {
		nonCoreSources.insert(PnPSource::SpellCompendium);
		nonCoreSources.insert(PnPSource::PHB2);
		nonCoreSources.insert(PnPSource::Homebrew);
		nonCoreSources.insert(PnPSource::Co8);
	}
};

extern TemplePlusConfig config;
