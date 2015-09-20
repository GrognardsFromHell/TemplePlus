#include "stdafx.h"
#include "aas.h"
#include "gamesystems.h"
#include "game_config.h"
#include "tig/tig_mouse.h"
#include "tig/tig_mes.h"
#include "tig/tig_msg.h"
#include "movies.h"
#include "tig/tig_loadingscreen.h"
#include "tio/tio.h"
#include "tig/tig_font.h"
#include "tig/tig_texture.h"
#include "tig/tig.h"
#include "python/python_integration.h"
#include "gamelib_private.h"
#include "util/config.h"

GameSystemFuncs gameSystemFuncs;

temple::GlobalStruct<GameSystems, 0x102AB368> gameSystems;

struct GameSystemInitTable : temple::AddressTable {

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

	void (__cdecl *DestroyPlayerObject)();
	void (__cdecl *EndGame)();

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
		rebase(DestroyPlayerObject, 0x1006EEF0);
		rebase(EndGame, 0x1014E160);

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
static void verifyTemplePlusData();
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
	verifyTemplePlusData();

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
	gameSystemInitTable.gameSystemConf->field_10 = temple::GetPointer<0x10002530>();
	gameSystemInitTable.gameSystemConf->renderfunc = temple::GetPointer<0x10002650>();
	
	initBufferStuff(conf);
	
	// This seems to be the primary rendering function, it's called from the renderfunc above (0x10002650)
	if (!conf.editor) {
		temple::GetRef<0x103072BC, void*>() = temple::GetPointer(0x100039E0);
	} else {
		temple::GetRef<0x103072BC, void*>() = temple::GetPointer(0x10003A50);
	}

	temple::GetRef<0x10306C10, int>() = FALSE; // always set to false, used by the renderfunc

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

	// TODO: Can be removed since we write it on the stack in GameSystemsRender
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

void GameSystemFuncs::EndGame() {
	return gameSystemInitTable.EndGame();
}

void GameSystemFuncs::DestroyPlayerObject() {
	gameSystemInitTable.DestroyPlayerObject();
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

	tio_mkdir("tpdata");
	tio_path_add("tpdata");

	tio_mkdir("data");
	tio_path_add("data");

	for (auto &entry : config.additionalTioPaths) {
		logger->info("Adding additional TIO path {}", entry);
		tio_path_add(entry.c_str());
	}
}

/*
	Checks that the TemplePlus data file has been loaded in some way.
*/
static void verifyTemplePlusData() {

	TioFileListFile info;
	if (!tio_fileexists("templeplus\\data_present", &info)) {
		throw TempleException("The TemplePlus data files are not installed.");
	}

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
	conf.getSkmFile = (uint32_t)temple::GetPointer<0x100041E0>();
	conf.getSkaFile = (uint32_t)temple::GetPointer<0x10004230>();
	conf.runScript = RunAnimFramePythonScript;

	if (aasFuncs.Init(&conf)) {
		throw TempleException("Failed to initialize animation system.");
	}
}
