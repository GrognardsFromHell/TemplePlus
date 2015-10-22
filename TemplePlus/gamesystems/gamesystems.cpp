
#include "stdafx.h"

#include <temple/dll.h>

#include "gamesystems.h"
#include "util/config.h"
#include "graphics/graphics.h"
#include "legacy.h"
#include <tig/tig_texture.h>
#include "../tig/tig_startup.h"
#include <tig/tig_mouse.h>
#include <tig/tig_loadingscreen.h>
#include <tig/tig_font.h>
#include <tig/tig_msg.h>
#include "loadingscreen.h"
#include <aas.h>
#include <movies.h>
#include <python/python_integration.h>

#include "mapsystems.h"

/*
Manages graphics engine resources used by
legacy game systems.
*/
class LegacyGameSystemResources : public ResourceListener {
public:

	explicit LegacyGameSystemResources(Graphics &graphics)
		: mRegistration(graphics, this) {
	}

	void CreateResources(Graphics&) override {

		gameSystemInitTable.CreateTownmapFogBuffer();
		gameSystemInitTable.CreateShadowMapBuffer();
	}

	void FreeResources(Graphics&) override {

		gameSystemInitTable.GameDisableDrawingForce();
		gameSystemInitTable.ReleaseTownmapFogBuffer();
		gameSystemInitTable.ReleaseShadowMapBuffer();

	}

private:
	ResourceListenerRegistration mRegistration;
};

GameSystems::GameSystems(TigInitializer &tig) : mTig(tig) {
	StopwatchReporter reporter("Game systems initialized in {}");
	logger->info("Loading game systems");

	memset(&mConfig, 0, sizeof(mConfig));
	mConfig.editor = ::config.editor ? 1 : 0;
	mConfig.width = tig.GetConfig().width;
	mConfig.height = tig.GetConfig().height;
	mConfig.field_10 = temple::GetPointer(0x10002530); // Callback 1
	mConfig.renderfunc = temple::GetPointer(0x10002650); // Callback 1
	mConfig.bufferstuffIdx = mTigBuffer.bufferIdx();

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

	mouseFuncs.SetBounds(mConfig.width, mConfig.height);

	// This may be redundant since it should have been set during tig initialization
	loadingScreenFuncs.SetBounds(mConfig.width, mConfig.height);

	// I don't even think partial install is an option for ToEE, but it may have been for Arcanum
	// It seems to be left-over, really since it also references Sierra in the code, while ToEE
	// was published by Atari
	// TODO: Once we reimplement the gamelib_mod_load function, this can be removed since it's only read from there
	*gameSystemInitTable.fullInstall = true;

	RegisterDataFiles();
	VerifyTemplePlusData();

	auto lang = GetLanguage();
	if (lang == "en") {
		logger->info("Assuming english fonts");
		*gameSystemInitTable.fontIsEnglish = true;
	}

	if (!config.skipLegal) {
		PlayLegalMovies();
	}

	// For whatever reason these are set here
	memcpy(gameSystemInitTable.gameSystemConf, &mConfig, sizeof(GameSystemConf));
	gameSystemInitTable.gameSystemConf->field_10 = temple::GetPointer<0x10002530>();
	gameSystemInitTable.gameSystemConf->renderfunc = temple::GetPointer<0x10002650>();

	InitBufferStuff(mConfig);


	GameSystemLoadingScreen loadingScreen;

	InitAnimationSystem();

	tigFont.LoadAll("art\\interface\\fonts\\*.*");
	tigFont.PushFont("priory-12", 12, true);

	// Now we finally load all game systems
	// TODO: RAII this
	// TODO:before / after init callbacks should be placed directly in here
	for (auto &system : legacyGameSystems->systems) {
		logger->info("Loading game system {}", system.name);
		msgFuncs.ProcessSystemEvents();
		if (system.loadscreenMesIdx) {
			loadingScreen.NextMessage();
		}
		if (system.init) {
			if (!system.init(&mConfig)) {
				throw TempleException(format("Game system {} failed to initialize", system.name));
			}
		}
	}

	mMapSystems = std::make_unique<MapSystems>(tig);

	mLegacyResources
		= std::make_unique<LegacyGameSystemResources>(tig.GetGraphics());

	gameSystemInitTable.InitPfxLightning();

	*gameSystemInitTable.ironmanFlag = false;
	*gameSystemInitTable.ironmanSaveGame = 0;

	if (!gameSystems) {
		gameSystems = this;
	}
}

GameSystems::~GameSystems() {

	logger->info("Unloading game systems");

	mLegacyResources.reset();

	gameSystemFuncs.Shutdown();

	if (gameSystems == this) {
		gameSystems = nullptr;
	}

}

TigBufferstuffInitializer::TigBufferstuffInitializer() {
	StopwatchReporter reporter("Game scratch buffer initialized in {}");
	logger->info("Creating game scratch buffer");
	if (!gameSystemInitTable.TigWindowBufferstuffCreate(&mBufferIdx)) {
		throw TempleException("Unable to initialize TIG buffer");
	}
}

TigBufferstuffInitializer::~TigBufferstuffInitializer() {
	logger->info("Freeing game scratch buffer");
	gameSystemInitTable.TigWindowBufferstuffFree(mBufferIdx);
}


void GameSystems::RegisterDataFiles() {
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
void GameSystems::VerifyTemplePlusData() {

	TioFileListFile info;
	if (!tio_fileexists("templeplus\\data_present", &info)) {
		throw TempleException("The TemplePlus data files are not installed.");
	}

}

// Gets the language of the current toee installation (i.e. "en")
std::string GameSystems::GetLanguage() {
	MesFile mesFile("mes\\language.mes");
	if (!mesFile.valid()) {
		return "en";
	}
	auto result = mesFile[1];
	return result ? result : "en";
}

void GameSystems::PlayLegalMovies() {
	movieFuncs.PlayMovie("movies\\AtariLogo.bik", 0, 0, 0);
	movieFuncs.PlayMovie("movies\\TroikaLogo.bik", 0, 0, 0);
	movieFuncs.PlayMovie("movies\\WotCLogo.bik", 0, 0, 0);
}

void GameSystems::InitBufferStuff(const GameSystemConf &conf) {

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

void GameSystems::InitAnimationSystem() {
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


void GameSystems::AdvanceTime() {

	auto now = timeGetTime();

	// This is used from somewhere in the object system
	*gameSystemInitTable.lastAdvanceTime = now;

	// TODO: Insert pre advance hook here

	for (auto &system : legacyGameSystems->systems) {
		if (system.advanceTime) {
			system.advanceTime(now);
		}
	}

	// TODO: Insert post advance hook here

}

void GameSystems::ResizeScreen(int w, int h) {

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
	*gameSystemInitTable.scratchBuffer = nullptr;

	ui.ResizeScreen(windowBufferStuffId, w, h);

}

void GameSystems::EndGame() {
	return gameSystemInitTable.EndGame();
}

void GameSystems::DestroyPlayerObject() {
	gameSystemInitTable.DestroyPlayerObject();
}

GameSystems *gameSystems = nullptr;
