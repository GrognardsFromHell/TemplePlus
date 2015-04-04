#include "stdafx.h"
#include "aas.h"
#include "gamesystems.h"
#include "config.h"
#include "fixes.h"
#include "temple_functions.h"
#include "game_config.h"
#include "tig_mouse.h"
#include "tig_mes.h"
#include "tig_msg.h"
#include "movies.h"
#include "tig_loadingscreen.h"
#include "tio.h"
#include "tig_font.h"
#include "tig_texture.h"
#include "tig.h"
#include "ui.h"

GameSystemFuncs gameSystemFuncs;

/*
FieldDefs *fieldDefTable = (FieldDefs*)0x10B3D7D8;
for (auto field : fieldDefTable->fields) {
logger->info("FIELD!");
}
*/

typedef bool (__cdecl *GameSystemInit)(const GameSystemConf* conf);
typedef void (__cdecl *GameSystemReset)();
typedef bool (__cdecl *GameSystemModuleLoad)();
typedef void (__cdecl *GameSystemModuleUnload)();
typedef void (__cdecl *GameSystemExit)();
typedef void (__cdecl *GameSystemAdvanceTime)(uint32_t time);
typedef bool (__cdecl *GameSystemSave)(TioFile file);
typedef bool (__cdecl *GameSystemLoad)(GameSystemSaveFile* saveFile);
typedef void (__cdecl *GameSystemResetBuffers)(RebuildBufferInfo* rebuildInfo);

struct GameSystem {
	const char* name;
	GameSystemInit init;
	GameSystemReset reset;
	GameSystemModuleLoad moduleLoad;
	GameSystemModuleUnload moduleUnload;
	GameSystemExit exit;
	GameSystemAdvanceTime advanceTime;
	uint32_t field1c;
	GameSystemSave save;
	GameSystemLoad load;
	GameSystemResetBuffers resetBuffers;
	uint32_t loadscreenMesIdx;
};

const uint32_t gameSystemCount = 61;

struct GameSystems {
	GameSystem systems[gameSystemCount];
};

static GlobalStruct<GameSystems, 0x102AB368> gameSystems;
static GameSystem orgFirstSystem;
static GameSystem orgLastSystem;

static vector<std::function<void(const GameSystemConf*)>> beforeInitCallbacks;
static vector<std::function<void(const GameSystemConf*)>> afterInitCallbacks;
static vector<std::function<void()>> beforeResetCallbacks;
static vector<std::function<void()>> afterResetCallbacks;
static vector<std::function<void()>> beforeModuleLoadCallbacks;
static vector<std::function<void()>> afterModuleLoadCallbacks;
static vector<std::function<void()>> beforeModuleUnloadCallbacks;
static vector<std::function<void()>> afterModuleUnloadCallbacks;
static vector<std::function<void()>> beforeExitCallbacks;
static vector<std::function<void()>> afterExitCallbacks;
static vector<std::function<void(uint32_t)>> beforeAdvanceTimeCallbacks;
static vector<std::function<void(uint32_t)>> afterAdvanceTimeCallbacks;
static vector<std::function<void(TioFile)>> beforeSaveCallbacks;
static vector<std::function<void(TioFile)>> afterSaveCallbacks;
static vector<std::function<void(GameSystemSaveFile*)>> beforeLoadCallbacks;
static vector<std::function<void(GameSystemSaveFile*)>> afterLoadCallbacks;
static vector<std::function<void(RebuildBufferInfo*)>> beforeResizeBuffersCallbacks;
static vector<std::function<void(RebuildBufferInfo*)>> afterResizeBuffersCallbacks;

struct BeforeCallbacks {
	static bool __cdecl Init(const GameSystemConf* conf) {
		logger->info("EVENT: Before Init");

		for (auto& callback : beforeInitCallbacks) {
			callback(conf);
		}

		if (orgFirstSystem.init) {
			return orgFirstSystem.init(conf);
		}
		return true;
	}

	static void __cdecl Reset() {
		logger->info("EVENT: Before Reset");

		for (auto& callback : beforeResetCallbacks) {
			callback();
		}

		if (orgFirstSystem.reset) {
			orgFirstSystem.reset();
		}
	}

	static bool __cdecl ModuleLoad() {
		logger->info("EVENT: Before Module Load");

		for (auto& callback : beforeModuleLoadCallbacks) {
			callback();
		}

		if (orgFirstSystem.moduleLoad) {
			return orgFirstSystem.moduleLoad();
		}
		return true;
	}

	static void __cdecl ModuleUnload() {
		logger->info("EVENT: Before Module Unload");

		for (auto& callback : beforeModuleUnloadCallbacks) {
			callback();
		}

		if (orgFirstSystem.moduleUnload) {
			orgFirstSystem.moduleUnload();
		}
	}

	static void __cdecl Exit() {
		logger->info("EVENT: Before Exit");

		for (auto& callback : beforeExitCallbacks) {
			callback();
		}

		if (orgFirstSystem.exit) {
			orgFirstSystem.exit();
		}
	}

	static void __cdecl AdvanceTime(uint32_t time) {
		// This spams the log a lot
		// logger->info("EVENT: Before Advance Time");
		for (auto& callback : beforeAdvanceTimeCallbacks) {
			callback(time);
		}

		if (orgFirstSystem.advanceTime) {
			orgFirstSystem.advanceTime(time);
		}
	}

	static bool __cdecl Save(TioFile file) {
		logger->info("EVENT: Before Save");

		for (auto& callback : beforeSaveCallbacks) {
			callback(file);
		}

		if (orgFirstSystem.save) {
			return orgFirstSystem.save(file);
		}
		return true;
	}

	static bool __cdecl Load(GameSystemSaveFile* file) {
		logger->info("EVENT: Before Load");

		for (auto& callback : beforeLoadCallbacks) {
			callback(file);
		}

		if (orgFirstSystem.load) {
			return orgFirstSystem.load(file);
		}
		return true;
	}

	static void __cdecl ResetBuffers(RebuildBufferInfo* info) {
		logger->info("EVENT: Before Reset Buffers");

		for (auto& callback : beforeResizeBuffersCallbacks) {
			callback(info);
		}

		if (orgFirstSystem.resetBuffers) {
			orgFirstSystem.resetBuffers(info);
		}
	}
};

struct AfterCallbacks {
	static bool __cdecl Init(const GameSystemConf* conf) {
		bool result = true;
		if (orgFirstSystem.init) {
			result = orgFirstSystem.init(conf);
		}

		logger->info("EVENT: After Init");

		for (auto& callback : afterInitCallbacks) {
			callback(conf);
		}

		/*
		
int NumberOfSetBits(int i)
{
	i = i - ((i >> 1) & 0x55555555);
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
	return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}
		struct FieldDef {
			int protoPropIndex;
			int field4;
			int PropBitmap_idx1;
			uint32_t PropBitmask;
			int PropBitmap_idx2;
			uint32_t IsStoredInPropCollection;
			uint32_t FieldTypeCode;
		};

		struct FieldDefs {
			FieldDef fields[430];
		};

		const char **typeNames = (const char**)0x102CD7F8;
		const char **fieldNames = (const char**)0x102CD840;
		FieldDefs *fieldDefTable = *(FieldDefs**)0x10B3D7D8;
		string typeNamesHeader;
		for (int j = 0; j <= 16; ++j) {
			typeNamesHeader.append(";").append(typeNames[j]);
		}

		logger->info("fieldname;proto idx;field4;bitmapidx1;bitmask;bitmapidx2;stored_in_prop_collection;field_type" << typeNamesHeader;
		int i = 0);
		for (auto field : fieldDefTable->fields) {
			string typeSupport;
			for (int j = 0; j <= 16; ++j) {
				if (templeFuncs.DoesTypeSupportField(j, i)) {
					typeSupport.append(";X");
				} else {
					typeSupport.append(";");
				}
			}

			LOG(info)
				<< fieldNames[i++] << ";"
				<< field.protoPropIndex << ";" << field.field4 << ";" << field.PropBitmap_idx1
				<< ";" << format("0x%x") % field.PropBitmask << ";" << field.PropBitmap_idx2
				<< ";" << field.IsStoredInPropCollection << ";" << field.FieldTypeCode << typeSupport;
		}*/

		return result;
	}

	static void __cdecl Reset() {
		if (orgFirstSystem.reset) {
			orgFirstSystem.reset();
		}

		logger->info("EVENT: After Reset");
		for (auto& callback : afterResetCallbacks) {
			callback();
		}
	}

	static bool __cdecl ModuleLoad() {
		bool result = true;
		if (orgFirstSystem.moduleLoad) {
			result = orgFirstSystem.moduleLoad();
		}
		logger->info("EVENT: After Module Load");
		for (auto& callback : afterModuleLoadCallbacks) {
			callback();
		}
		return result;
	}

	static void __cdecl ModuleUnload() {
		if (orgFirstSystem.moduleUnload) {
			orgFirstSystem.moduleUnload();
		}
		logger->info("EVENT: After Module Unload");
		for (auto& callback : afterModuleUnloadCallbacks) {
			callback();
		}
	}

	static void __cdecl Exit() {
		if (orgFirstSystem.exit) {
			orgFirstSystem.exit();
		}
		logger->info("EVENT: After Exit");
		for (auto& callback : afterExitCallbacks) {
			callback();
		}
	}

	static void __cdecl AdvanceTime(uint32_t time) {
		if (orgFirstSystem.advanceTime) {
			orgFirstSystem.advanceTime(time);
		}
		// Logspam
		// logger->info("EVENT: After Advance Time");
		for (auto& callback : afterAdvanceTimeCallbacks) {
			callback(time);
		}
	}

	static bool __cdecl Save(TioFile file) {
		bool result = true;
		if (orgFirstSystem.save) {
			result = orgFirstSystem.save(file);
		}
		logger->info("EVENT: After Save");
		for (auto& callback : afterSaveCallbacks) {
			callback(file);
		}
		return result;
	}

	static bool __cdecl Load(GameSystemSaveFile* file) {
		bool result = true;
		if (orgFirstSystem.load) {
			result = orgFirstSystem.load(file);
		}
		logger->info("EVENT: After Load");
		for (auto& callback : afterLoadCallbacks) {
			callback(file);
		}
		return result;
	}

	static void __cdecl ResetBuffers(RebuildBufferInfo* info) {
		if (orgFirstSystem.resetBuffers) {
			orgFirstSystem.resetBuffers(info);
		}
		logger->info("EVENT: After Reset Buffers");
		for (auto& callback : afterResizeBuffersCallbacks) {
			callback(info);
		}
	}
};

/*
	Overrides the first and last game system so we can provide before/after hooks for all events.
*/
class GameSystemHookInitializer : TempleFix {
public:
	const char* name() override {
		return "Gamesystem Hooks";
	}

	void apply() override {
		// Override all functions for "before" callbacks
		auto& firstSystem = gameSystems->systems[0];
		orgFirstSystem = firstSystem;
		firstSystem.init = &BeforeCallbacks::Init;
		firstSystem.reset = &BeforeCallbacks::Reset;
		firstSystem.moduleLoad = &BeforeCallbacks::ModuleLoad;
		firstSystem.moduleUnload = &BeforeCallbacks::ModuleUnload;
		firstSystem.exit = &BeforeCallbacks::Exit;
		firstSystem.advanceTime = &BeforeCallbacks::AdvanceTime;
		firstSystem.save = &BeforeCallbacks::Save;
		firstSystem.load = &BeforeCallbacks::Load;
		firstSystem.resetBuffers = &BeforeCallbacks::ResetBuffers;

		// Override all functions for "before" callbacks
		auto& lastSystem = gameSystems->systems[60];
		orgLastSystem = lastSystem;
		lastSystem.init = &AfterCallbacks::Init;
		lastSystem.reset = &AfterCallbacks::Reset;
		lastSystem.moduleLoad = &AfterCallbacks::ModuleLoad;
		lastSystem.moduleUnload = &AfterCallbacks::ModuleUnload;
		lastSystem.exit = &AfterCallbacks::Exit;
		lastSystem.advanceTime = &AfterCallbacks::AdvanceTime;
		lastSystem.save = &AfterCallbacks::Save;
		lastSystem.load = &AfterCallbacks::Load;
		lastSystem.resetBuffers = &AfterCallbacks::ResetBuffers;
	}

} gameSystemHookInitializer;

void GameSystemHooks::AddInitHook(std::function<void(const GameSystemConf*)> callback, bool before) {
	if (before) {
		beforeInitCallbacks.push_back(callback);
	} else {
		afterInitCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddResetHook(std::function<void()> callback, bool before) {
	if (before) {
		beforeResetCallbacks.push_back(callback);
	} else {
		afterResetCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddModuleLoadHook(std::function<void()> callback, bool before) {
	if (before) {
		beforeModuleLoadCallbacks.push_back(callback);
	} else {
		afterModuleLoadCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddModuleUnloadHook(std::function<void()> callback, bool before) {
	if (before) {
		beforeModuleUnloadCallbacks.push_back(callback);
	} else {
		afterModuleUnloadCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddExitHook(std::function<void()> callback, bool before) {
	if (before) {
		beforeExitCallbacks.push_back(callback);
	} else {
		afterExitCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddAdvanceTimeHook(std::function<void(uint32_t)> callback, bool before) {
	if (before) {
		beforeAdvanceTimeCallbacks.push_back(callback);
	} else {
		afterAdvanceTimeCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddSaveHook(std::function<void(TioFile)> callback, bool before) {
	if (before) {
		beforeSaveCallbacks.push_back(callback);
	} else {
		afterSaveCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddLoadHook(std::function<void(GameSystemSaveFile*)> callback, bool before) {
	if (before) {
		beforeLoadCallbacks.push_back(callback);
	} else {
		afterLoadCallbacks.push_back(callback);
	}
}

void GameSystemHooks::AddResizeBuffersHook(std::function<void(RebuildBufferInfo*)> callback, bool before) {
	if (before) {
		beforeResizeBuffersCallbacks.push_back(callback);
	} else {
		afterResizeBuffersCallbacks.push_back(callback);
	}
}

struct GameSystemInitTable : AddressTable {

	// Called when config setting for game difficulty changes
	GameConfigChangedCallback DifficultyChanged;

	// Used by module load to determine whether the module has already been loaded
	bool *moduleLoaded;
	
	// These functions are used to propagate initial config values to the corresponding subsystems
	void (__cdecl *SetDrawHp)(bool drawHp);
	void (__cdecl *SetEndTurnDefault)(int endTurnDefault);
	void (__cdecl *SetEndTurnTime)(int endTurnTime);
	void (__cdecl *SetConcurrentTurnbased)(int value);	
	void (__cdecl *SetClothFrameSkip)(int value);
	void (__cdecl *SetEnvMapping)(int value);
	void (__cdecl *SetParticleFidelity)(float value); // Value is clamped to [0,1]

	// Initializes several random tables, vectors and shuffled int lists as well as the lightning material.
	void (__cdecl *InitPfxLightning)();

	// Indicates that the game was not installed partially and the CD is not needed for dat files
	bool *fullInstall;

	// This is set to true if the language is "en". Purpose is yet unknown other than it being read by tig_font
	bool *fontIsEnglish;

	// Global copy of the game system config, mostly read by game systems themselves
	GameSystemConf *gameSystemConf;

	// Handle to the id->filename table for meshes
	MesHandle *meshesMes;
	
	// State relating to the ironman mode
	bool *ironmanFlag;
	char **ironmanSaveGame;

	// The exact purpose of the scratchbuffer is unknown at this point
	TigBuffer **scratchBuffer;

	// It's not even safe to assume these rects belong to the scratchbuffer
	TigRect *scratchBufferRect;
	TigRect *scratchBufferRectExtended;

	/*
		Stores the last timestamp when the game systems were processed.
		Sadly, this is used from somewhere deep in the obj system.
	*/
	uint32_t *lastAdvanceTime;
	
	GameSystemInitTable() {
		rebase(DifficultyChanged, 0x10003770);
		rebase(SetDrawHp, 0x1002B910);
		rebase(SetEndTurnDefault, 0x10092280);
		rebase(SetEndTurnTime, 0x100922A0);
		rebase(SetConcurrentTurnbased, 0x10092260);
		rebase(SetParticleFidelity, 0x101E7810);
		rebase(SetClothFrameSkip, 0x102629C0);
		rebase(SetEnvMapping, 0x101E0A20);
		rebase(InitPfxLightning, 0x10087220);

		rebase(moduleLoaded, 0x10307054);
		rebase(fullInstall, 0x10306F48);
		rebase(fontIsEnglish, 0x10EF2E5C);
		rebase(gameSystemConf, 0x103072A0);
		rebase(meshesMes, 0x10307168);
		rebase(ironmanFlag, 0x103072B8);
		rebase(ironmanSaveGame, 0x103072C0);
		rebase(scratchBuffer, 0x103072DC);
		rebase(scratchBufferRect, 0x1030727C);
		rebase(scratchBufferRectExtended, 0x10307290);
		rebase(lastAdvanceTime, 0x11E726AC);
	}
} gameSystemInitTable;

static void registerDataFiles();
static string getLanguage();
static void playLegalMovies();
static void initBufferStuff(const GameSystemConf &conf);
static void initAas();

class GameSystemLoadingScreen {
public:
	GameSystemLoadingScreen() : mes("mes\\loadscreen.mes") {						
		if (!mes.valid()) {
			throw TempleException("loadscreen.mes not found");
		}

		sequence.messages = new const char*[gameSystemCount]; // 61 is the number of game systems
		sequence.image = ui.LoadImg("art\\splash\\legal0322.img");
		sequence.textColor = &graycolor;
		sequence.barBorderColor = &graycolor;
		sequence.barBackground = &darkblue;
		sequence.barForeground = &lightblue;
		sequence.offsetLeft = 0;
		sequence.offsetUp = 5;
		sequence.barWidth = 256;
		sequence.barHeight = 24;

		for (int i = 0; i < gameSystemCount; ++i) {
			const auto &system = gameSystems->systems[i];

			if (system.loadscreenMesIdx) {
				auto idx = sequence.messagesCount++;
				sequence.messages[idx] = mes[system.loadscreenMesIdx];
			}
		}

		loadingScreenFuncs.Push(&sequence);
	}

	~GameSystemLoadingScreen() {
		loadingScreenFuncs.Pop();
	}

	void NextMessage() {
		loadingScreenFuncs.NextStep();
	}
private:
	MesFile mes;
	LoadingSequence sequence;
	RectColor darkblue = RectColor(0xFF1C324E);
	RectColor graycolor = RectColor(0xFF808080);
	RectColor lightblue = RectColor(0xFF1AC3FF);
};

void GameSystemFuncs::NewInit(const GameSystemConf& conf) {

	gameConfigFuncs.Init("ToEE.cfg");
	gameConfigFuncs.Load();

	gameConfigFuncs.AddSetting("difficulty", "1", gameSystemInitTable.DifficultyChanged);
	gameSystemInitTable.DifficultyChanged(); // Read initial setting
	gameConfigFuncs.AddSetting("autosave_between_maps", "1");
	gameConfigFuncs.AddSetting("movies_seen", "(304,-1)");
	gameConfigFuncs.AddSetting("startup_tip", "0");
	gameConfigFuncs.AddSetting("video_adapter", "0");
	gameConfigFuncs.AddSetting("video_width", "800");
	gameConfigFuncs.AddSetting("video_height", "600");
	gameConfigFuncs.AddSetting("video_bpp", "32");
	gameConfigFuncs.AddSetting("video_refresh_rate", "60");
	gameConfigFuncs.AddSetting("video_antialiasing", "0");
	gameConfigFuncs.AddSetting("video_quad_blending", "1");
	gameConfigFuncs.AddSetting("particle_fidelity", "100");
	gameConfigFuncs.AddSetting("env_mapping", "1");
	gameConfigFuncs.AddSetting("cloth_frame_skip", "0");
	gameConfigFuncs.AddSetting("concurrent_turnbased", "1");
	gameConfigFuncs.AddSetting("end_turn_time", "1");
	gameConfigFuncs.AddSetting("end_turn_default", "1");
	gameConfigFuncs.AddSetting("draw_hp", "0");

	// Some of these are also registered as value change callbacks and could be replaced by simply calling all 
	// value change callbacks here, which makes sense anyway.
	auto particleFidelity = gameConfigFuncs.GetInt("particle_fidelity") / 100.0f;
	gameSystemInitTable.SetParticleFidelity(particleFidelity);

	auto envMappingEnabled = gameConfigFuncs.GetInt("env_mapping");
	gameSystemInitTable.SetEnvMapping(envMappingEnabled);

	auto clothFrameSkip = gameConfigFuncs.GetInt("cloth_frame_skip");
	gameSystemInitTable.SetClothFrameSkip(clothFrameSkip);
	
	auto concurrentTurnbased = gameConfigFuncs.GetInt("concurrent_turnbased");
	gameSystemInitTable.SetConcurrentTurnbased(concurrentTurnbased);
	
	auto endTurnTime = gameConfigFuncs.GetInt("end_turn_time");
	gameSystemInitTable.SetEndTurnTime(endTurnTime);
	
	auto endTurnDefault = gameConfigFuncs.GetInt("end_turn_default");
	gameSystemInitTable.SetEndTurnDefault(endTurnDefault);

	auto drawHp = gameConfigFuncs.GetInt("draw_hp");
	gameSystemInitTable.SetDrawHp(drawHp != 0);

	*gameSystemInitTable.moduleLoaded = 0;

	mouseFuncs.SetBounds(conf.width, conf.height);

	// This may be redundant since it should have been set during tig initialization
	loadingScreenFuncs.SetBounds(conf.width, conf.height);

	// I don't even think partial install is an option for ToEE, but it may have been for Arcanum
	// It seems to be left-over, really since it also references Sierra in the code, while ToEE
	// was published by Atari
	// TODO: Once we reimplement the gamelib_mod_load function, this can be removed since it's only read from there
	*gameSystemInitTable.fullInstall = true;
	
	registerDataFiles();

	auto lang = getLanguage();
	if (lang == "en") {
		logger->info("Assuming english fonts");
		*gameSystemInitTable.fontIsEnglish = true;
	}

	if (!config.skipLegal) {
		playLegalMovies();
	}

	// For whatever reason these are set here
	memcpy(gameSystemInitTable.gameSystemConf, &conf, sizeof(GameSystemConf));
	gameSystemInitTable.gameSystemConf->field_10 = temple_address<0x10002530>();
	gameSystemInitTable.gameSystemConf->renderfunc = temple_address<0x10002650>();
	
	initBufferStuff(conf);
	
	// This seems to be the primary rendering function, it's called from the renderfunc above (0x10002650)
	if (!conf.editor) {
		temple_set<0x103072BC>(temple_address(0x100039E0));
	} else {
		temple_set<0x103072BC>(temple_address(0x10003A50));
	}

	temple_set<0x10306C10>(false); // always set to false, used by the renderfunc

	GameSystemLoadingScreen loadingScreen;

	initAas();

	tigFont.LoadAll("art\\interface\\fonts\\*.*");
	tigFont.PushFont("priory-12", 12, true);

	// Now we finally load all game systems
	// TODO: RAII this
	// TODO:before / after init callbacks should be placed directly in here
	for (auto &system : gameSystems->systems) {
		logger->info("Loading game system {}", system.name);
		msgFuncs.ProcessSystemEvents();
		if (system.loadscreenMesIdx) {
			loadingScreen.NextMessage();
		}
		if (system.init) {
			if (!system.init(&conf)) {
				throw TempleException(format("Game system {} failed to initialize", system.name));
			}
		}
	}
	
	gameSystemInitTable.InitPfxLightning();

	*gameSystemInitTable.ironmanFlag = false;
	*gameSystemInitTable.ironmanSaveGame = 0;
}

void GameSystemFuncs::AdvanceTime() {

	auto now = timeGetTime();

	// This is used from somewhere in the object system
	*gameSystemInitTable.lastAdvanceTime = now;

	// TODO: Insert pre advance hook here

	for (auto &system : gameSystems->systems) {
		if (system.advanceTime) {
			system.advanceTime(now);
		}
	}

	// TODO: Insert post advance hook here

}

void GameSystemFuncs::ResizeScreen(int w, int h) {

	gameSystemInitTable.gameSystemConf->width = w;
	gameSystemInitTable.gameSystemConf->height = h;

	// These do not seem to be read anywhere
	// dword_1030705C = video_width / 4;
	// dword_10307170 = video_height / 4;

	auto scratchBufferRectExtended = gameSystemInitTable.scratchBufferRectExtended;
	scratchBufferRectExtended->y = -256;
	scratchBufferRectExtended->width = w + 512;
	scratchBufferRectExtended->x = -256;
	scratchBufferRectExtended->height = h + 512;

	auto scratchBufferRect = gameSystemInitTable.scratchBufferRect;
	scratchBufferRect->x = 0;
	scratchBufferRect->y = 0;
	scratchBufferRect->width = w;
	scratchBufferRect->height = h;

	auto windowBufferStuffId = gameSystemInitTable.gameSystemConf->bufferstuffIdx;
	
	// I do not think that the scratchbuffer is used in any way
	// rebuild_scratchbuffer(&resizeargs);

	ui.ResizeScreen(windowBufferStuffId, w, h);
	
}

static void registerDataFiles() {
	TioFileList list;
	tio_filelist_create(&list, "*.dat");

	for (int i = 0; i < list.count; ++i) {
		auto file = list.files[i];
		logger->info("Registering archive {}", file.name);
		int result = tio_path_add(file.name);
		if (result != 0) {
			logger->trace("Unable to add archive {}: {}", file.name, result);
		}
	}

	tio_filelist_destroy(&list);
	
	tio_mkdir("data");
	tio_path_add("data");
}

// Gets the language of the current toee installation (i.e. "en")
static string getLanguage() {
	MesFile mesFile("mes\\language.mes");
	if (!mesFile.valid()) {
		return "en";
	}
	auto result = mesFile[1];
	return result ? result : "en";
}

static void playLegalMovies() {
	movieFuncs.PlayMovie("movies\\AtariLogo.bik", 0, 0, 0);
	movieFuncs.PlayMovie("movies\\TroikaLogo.bik", 0, 0, 0);
	movieFuncs.PlayMovie("movies\\WotCLogo.bik", 0, 0, 0);
}

static void initBufferStuff(const GameSystemConf &conf) {

	// TODO: It is VERY dubious that this is actually used anywhere!!!
	// scratchbuffer is never accessed
	if (*gameSystemInitTable.scratchBuffer == nullptr)
	{
		TigBufferCreateArgs createArgs;
		createArgs.flagsOrSth = 5;
		createArgs.width = conf.width;
		createArgs.height = conf.height;
		createArgs.field_10 = 0;
		createArgs.zero4 = 0;
		createArgs.texturetype = 2;
		if (textureFuncs.CreateBuffer(&createArgs, gameSystemInitTable.scratchBuffer)) {
			throw TempleException("Unable to create the scratchbuffer!");
		}
	}

	/*auto rect = gameSystemInitTable.scratchBufferRect;
	rect->y = 0;
	rect->x = 0;
	rect->width = conf.width;
	rect->height = conf.height;*/
	
	// Without this rectangle in particular, the ingame view is not drawn
	auto rectExtended = gameSystemInitTable.scratchBufferRectExtended;
	rectExtended->x = -256;
	rectExtended->y = -256;	
	rectExtended->width = conf.width + 512;
	rectExtended->height = conf.height + 512;

	/*
	tig_bufferstuff_get(conf->tigwindowbufferstuffidx, &tigwindowbuffer);
	bufferstuff_left = tigwindowbuffer.rect.x;
	bufferstuff_top = tigwindowbuffer.rect.y;
	These two seem definetly unused
	dword_1030705C = video_width / 4;
	dword_10307170 = video_height / 4;	
	*/
}

static void initAas() {
	// This is probably loaded here because gamelib also provides the skm/ska filename lookup functions
	if (!mesFuncs.Open("art\\meshes\\meshes.mes", gameSystemInitTable.meshesMes)) {
		throw TempleException("Unable to open meshes.mes");
	}

	DWORD pixelPerWorldTile = 0x41E24630;
	AasConfig conf;
	memset(&conf, 0, sizeof(conf));
	conf.pixelPerWorldTile1 = *reinterpret_cast<float*>(&pixelPerWorldTile);
	conf.pixelPerWorldTile2 = conf.pixelPerWorldTile1;
	conf.getSkmFile = (uint32_t)temple_address<0x100041E0>();
	conf.getSkaFile = (uint32_t)temple_address<0x10004230>();
	conf.runScript = (uint32_t)temple_address<0x100AD990>();

	if (aasFuncs.Init(&conf)) {
		throw TempleException("Failed to initialize animation system.");
	}
}
