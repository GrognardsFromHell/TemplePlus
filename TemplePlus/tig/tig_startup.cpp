#include "stdafx.h"

#include <graphics/device.h>
#include <graphics/shaperenderer2d.h>
#include <graphics/shaperenderer3d.h>
#include <graphics/mdfmaterials.h>

#include <debugui.h>

#include <temple/dll.h>
#include <temple/vfs.h>
#include <temple/soundsystem.h>
#include <temple/moviesystem.h>

#include <temple/dll.h>

#include "mainwindow.h"
#include "graphics/legacyvideosystem.h"
#include "fonts/fonts.h"
#include "messages/messagequeue.h"

#include "tig_startup.h"
#include "../tio/tio.h"
#include "tig_console.h"

#include "../gameview.h"

#include "../config/config.h"
#include <fstream>
#include <mod_support.h>
#include <winsock.h>

TigInitializer* tig = nullptr;

using TigStartupFunction = int(const TigConfig*);
using TigShutdownFunction = void();

/*
	These function addresses point to empty functions that
	simply do nothing.
*/
static constexpr uint32_t TigStartupNoop = 0x10262530;
static constexpr uint32_t TigShutdownNoop = 0x100027F0;

class LegacyTigSystem {
public:

	LegacyTigSystem(const TigConfig* config,
	                const std::string& name,
	                uint32_t startupAddr,
	                uint32_t shutdownAddr) : mName(name), mShutdownAddr(shutdownAddr) {
		auto startup = reinterpret_cast<TigStartupFunction*>(
			temple::GetPointer(startupAddr)
		);
		if (startup(config) != 0) {
			throw TempleException("Unable to initialize TIG system {}", name);
		}
	}

	~LegacyTigSystem() {
		logger->info("Shutting down {}", mName);
		auto shutdown = reinterpret_cast<TigShutdownFunction*>(
			temple::GetPointer(mShutdownAddr)
		);
		shutdown();
	}

private:
	std::string mName;
	uint32_t mShutdownAddr;
};

static struct TigInternal : temple::AddressTable {
	bool* consoleDisabled; // TODO: move to tig_console.h

	int (__cdecl *FindSound)(int soundId, char* filenameOut);

	TigInternal() {
		rebase(consoleDisabled, 0x10D26DB4);
		rebase(FindSound, 0x1003B9E0);
	}
} tigInternal;

static TigConfig createTigConfig(HINSTANCE hInstance);

TigInitializer::TigInitializer(HINSTANCE hInstance)
	: mConfig(createTigConfig(hInstance)) {

	Expects(tig == nullptr);
	tig = this;

	StopwatchReporter reporter("TIG initialized in {}");
	logger->info("Initializing TIG");

	vfs = std::make_unique<temple::TioVfs>();

	tio_path_add("tig.dat");
	tio_path_add(".");
	LoadDataFiles();

	// No longer used: mStartedSystems.emplace_back(StartSystem("memory.c", 0x101E04F0, 0x101E0510));
	// No longer used: mStartedSystems.emplace_back(StartSystem("debug.c", 0x101E4DE0, TigShutdownNoop));
	mMainWindow = std::make_unique<MainWindow>(hInstance);
	mRenderingDevice = std::make_unique<gfx::RenderingDevice>(mMainWindow->GetHwnd(),
		config.displayAdapter,
		config.d3dDebug);
	mRenderingDevice->SetAntiAliasing(config.antialiasing,
		config.msaaSamples,
		config.msaaQuality);
	mRenderingDevice->SetVSync(config.vsync);

	mDebugUI = std::make_unique<DebugUI>(*mRenderingDevice);
	// Install a message filter for the debug UI
	mMainWindow->SetWindowMsgFilter([&](UINT msg, WPARAM wParam, LPARAM lParam) {
		return mDebugUI->HandleMessage(msg, wParam, lParam);
	});

	mMdfFactory = std::make_unique<gfx::MdfMaterialFactory>(*mRenderingDevice);
	mMdfFactory->LoadReplacementSets("rules\\materials.mes");
	mMdfFactory->LoadReplacementSets("rules\\materials_ext.mes"); // material extension file
	mShapeRenderer2d = std::make_unique<gfx::ShapeRenderer2d>(*mRenderingDevice);
	mShapeRenderer3d = std::make_unique<gfx::ShapeRenderer3d>(*mRenderingDevice);
	mTextLayouter = std::make_unique<TextLayouter>(*mRenderingDevice, *mShapeRenderer2d);
	mStartedSystems.emplace_back(StartSystem("idxtable.c", 0x101EC400, 0x101ECAD0));
	mStartedSystems.emplace_back(StartSystem("trect.c", TigStartupNoop, 0x101E4E40));
	mStartedSystems.emplace_back(StartSystem("color.c", 0x101ECB20, 0x101ED070));

	mLegacyVideoSystem = std::make_unique<LegacyVideoSystem>(*mMainWindow, *mRenderingDevice);

	// mStartedSystems.emplace_back(StartSystem("video.c", 0x101DC6E0, 0x101D8540));
	
	mStartedSystems.emplace_back(StartSystem("shader", 0x101E3350, 0x101E2090));
	mStartedSystems.emplace_back(StartSystem("palette.c", 0x101EBE30, 0x101EBEB0));
	mStartedSystems.emplace_back(StartSystem("window.c", 0x101DED20, 0x101DF320));
	mStartedSystems.emplace_back(StartSystem("timer.c", 0x101E34E0, 0x101E34F0));
	// mStartedSystems.emplace_back(StartSystem("dxinput.c", 0x101FF910, 0x101FF950));
	// mStartedSystems.emplace_back(StartSystem("keyboard.c", 0x101DE430, 0x101DE2D0));

	// mStartedSystems.emplace_back(StartSystem("texture.c", 0x101EDF60, 0x101EE0A0));
	mStartedSystems.emplace_back(StartSystem("mouse.c", 0x101DDF50, 0x101DDE30));
	mStartedSystems.emplace_back(StartSystem("message.c", 0x101DE460, 0x101DE4E0));
	mMessageQueue = std::make_unique<MessageQueue>();
	// startedSystems.emplace_back(StartSystem("gfx.c", TigStartupNoop, TigShutdownNoop));
	mStartedSystems.emplace_back(StartSystem("strparse.c", 0x101EBF00, TigShutdownNoop));
	mStartedSystems.emplace_back(StartSystem("filecache.c", TigStartupNoop, TigShutdownNoop));
	if (!config.noSound) {
		mStartedSystems.emplace_back(StartSystem("sound.c", 0x101E3FA0, 0x101E48A0));
	}
	mSoundSystem = std::make_unique<temple::SoundSystem>();
	mMovieSystem = std::make_unique<temple::MovieSystem>(*mSoundSystem);
	// mStartedSystems.emplace_back(StartSystem("movie.c", 0x101F1090, TigShutdownNoop));
	mStartedSystems.emplace_back(StartSystem("wft.c", 0x101F98A0, 0x101F9770));
	mStartedSystems.emplace_back(StartSystem("font.c", 0x101EAEC0, 0x101EA140));
	mConsole = std::make_unique<Console>();
	// mStartedSystems.emplace_back(StartSystem("console.c", 0x101E0290, 0x101DFBC0));
	mStartedSystems.emplace_back(StartSystem("loadscreen.c", 0x101E8260, TigShutdownNoop));

	mGameView = std::make_unique<GameView>(*mMainWindow, *mRenderingDevice, config.renderWidth, config.renderHeight);

	*tigInternal.consoleDisabled = false; // tig init disables console by default
}

static std::string FindTpData() {
	char ownFilename[MAX_PATH];
	GetModuleFileNameA(nullptr, ownFilename, MAX_PATH);
	PathRemoveFileSpecA(ownFilename);
	PathAppendA(ownFilename, "tpdata");
	if (PathIsDirectoryA(ownFilename)) {
		return ownFilename;
	}

#ifndef TP_RELEASE_BUILD
	GetModuleFileNameA(nullptr, ownFilename, MAX_PATH);
	PathRemoveFileSpecA(ownFilename);
	PathAppendA(ownFilename, "..\\tpdata");
	if (PathIsDirectoryA(ownFilename)) {
		return ownFilename;
	}
#endif

	return "tpdata"; // Just fall back to this then...

}

#ifndef TP_RELEASE_BUILD
static std::string FindPythonLibDir() {
	char ownFilename[MAX_PATH];
	GetModuleFileNameA(nullptr, ownFilename, MAX_PATH);
	PathRemoveFileSpecA(ownFilename);
	PathAppendA(ownFilename, "..\\dependencies\\python-lib");
	if (PathIsDirectoryA(ownFilename)) {
		GetModuleFileNameA(nullptr, ownFilename, MAX_PATH);
		PathRemoveFileSpecA(ownFilename);
		PathAppendA(ownFilename, "..\\dependencies");
		return ownFilename;
	}
	return "";
}
#endif

class ModOverride {
public:
	static bool IsModFolder(const std::string&);
};

void TigInitializer::LoadDataFiles() {

	{
		// TODO: Migrate to use of vfs
		TioFileList list;
		tio_filelist_create(&list, "*.dat");

		for (int i = 0; i < list.count; ++i) {
			auto file = list.files[i];
			logger->info("Registering archive {}", file.name);
			int result = tio_path_add(file.name);
			if (result != 0) {
				logger->error("Unable to add archive {}: {}", file.name, result);
			}
		}

		tio_filelist_destroy(&list);
	}

	tio_mkdir("data");
	tio_path_add("data");

	std::string tpDataPath = FindTpData();

	logger->info("Registering tpdata\\tpgamefiles.dat");
	if (tio_path_add(fmt::format("{}\\tpgamefiles.dat", tpDataPath).c_str())) {
		logger->error("Unable to add archive tpdata\\tpgamefiles.dat");
		throw TempleException("Could not load tpgamefiles.dat!");
	}

	if (tio_path_add(tpDataPath.c_str())) {
		throw TempleException("Unable to add TemplePlus data to ToEE from {}", tpDataPath);
	}

	// Check if python library is now accessible
#ifndef TP_RELEASE_BUILD
	if (!tio_fileexists("python-lib\\site.py")) {
		auto pythonLibDir = FindPythonLibDir();
		if (!pythonLibDir.empty()) {
			if (tio_path_add(pythonLibDir.c_str())) {
				throw TempleException("Unable to add TemplePlus data (python-lib) to ToEE from {}", pythonLibDir);
			}
		}
	}
#endif

	logger->info("Registering new pathfinding data tpdata\\clearances.dat");
	auto result=  tio_path_add(fmt::format("{}\\clearances.dat", tpDataPath).c_str());
	if (result != 0) {
		logger->error("Unable to add archive tpdata\\clearances.dat");
	}

	modSupport.mInited = true;
	if (temple::Dll::GetInstance().HasCo8Hooks()){
		modSupport.mIsCo8 = true;
		modSupport.DetectCo8ActiveModule();
		if (modSupport.IsKotB()){
			logger->info("KotB module detected; registering  kotbfixes.dat.");
			result = tio_path_add(fmt::format("{}\\kotbfixes.dat", tpDataPath).c_str());
			if (result != 0) {
				logger->error("Unable to add archive tpdata\\kotbfixes.dat");
			}
		}
		else if (modSupport.IsPalCove()) {
			// placeholder
			logger->info("Paladin's Cove module Detected.");
			result = tio_path_add(fmt::format("{}\\palcov.dat", tpDataPath).c_str());
			if (result != 0) {
				logger->error("Unable to add archive tpdata\\palcov.dat");
			}
		}
		else {
			logger->info("Registering Co8 infrastructure fixes tpdata\\co8infra.dat");
			result = tio_path_add(fmt::format("{}\\co8infra.dat", tpDataPath).c_str());
			if (result != 0) {
				logger->error("Unable to add archive tpdata\\co8infra.dat");
			}

			if (modSupport.IsIWD()) {
				// placeholder
				logger->info("Icewind Dale module detected.");
			}
			else {
				logger->info("Registering Co8 file fixes tpdata\\co8fixes.dat");
				result = tio_path_add(fmt::format("{}\\co8fixes.dat", tpDataPath).c_str());
				if (result != 0) {
					logger->error("Unable to add archive tpdata\\co8fixes.dat");
				}
			}
		}
		
	}

	// Adding support for module data folders ("Module Core"), distinct from module map data (mainly MOBs and ground art) which is loaded later.
	// Note: cannot load module here. Crash issue arises due to party pool loader (0x10165790), and who knows, there might be more...
	{
		std::string moduleName = config.defaultModule;
		std::string moduleBase;
		if (!VfsPath::IsFileSystem(moduleName)) {
			moduleBase = VfsPath::Concat(".\\Modules\\", moduleName);
		}
		else {
			moduleBase = moduleName;
		}
		modSupport.DetectZMOD();

		auto moduleCoreDatName = moduleBase + "_core.dat";
		auto moduleDir = moduleBase + "_core";

		auto tioVfs = static_cast<temple::TioVfs*>(vfs.get());

		logger->info("Module core archive: {}", moduleCoreDatName);
		if (vfs->FileExists(moduleCoreDatName)) {
			if (!tioVfs->AddPath(moduleCoreDatName)) {
				throw TempleException("Unable to load module archive {}", moduleCoreDatName);
			}
		}

		logger->info("Module core directory: {}", moduleDir);
		if (vfs->DirExists(moduleDir)) {
			if (!tioVfs->AddPath(moduleDir)) {
				throw TempleException("Unable to add module directory: {}", moduleDir);
			}	
		}

	}

	// overrides for testing (mainly for co8fixes so there's no need to repack the archive). Also for user mods.
	tio_mkdir(fmt::format("overrides").c_str());
	{
		TioFileList list;
		tio_filelist_create(&list, "overrides\\*");

		for (int i = 0; i < list.count; ++i) {
			auto file = list.files[i];
			
			auto fullPath = fmt::format("overrides\\{}", file.name);
			if (!(file.attribs & TioFileAttribs::TFA_SUBDIR))
				continue;
				
			if (ModOverride::IsModFolder(fullPath) ) {

				logger->info("Registering override folder {}", file.name);
				int result = tio_path_add(fullPath.c_str());
				if (result != 0) {
					logger->error("Unable to add archive {}: {}", file.name, result);
				}
				else {
					modSupport.AddOverride(file.name);
				}
			}
			else if (file.name[0] != '.') {
				logger->info("Ignoring override folder {}", file.name);
			}
		}

		tio_filelist_destroy(&list);
	}

	for (auto& entry : config.additionalTioPaths) {
		logger->info("Adding additional TIO path {}", entry);
		tio_path_add(entry.c_str());
	}

}

TigInitializer::TigSystemPtr TigInitializer::StartSystem(const std::string& name,
                                                         uint32_t startAddr,
                                                         uint32_t shutdownAddr) {

	logger->info("Starting TIG subsystem {}", name);

	return std::make_unique<LegacyTigSystem>(&mConfig, name, startAddr, shutdownAddr);

}

TigInitializer::~TigInitializer() {
	
	tig = nullptr;

	StopwatchReporter reporter("TIG shut down in {}");
	logger->info("Shutting down TIG");

	for (auto it = mStartedSystems.rbegin();
	     it != mStartedSystems.rend();
	     ++it) {
		it->reset();
	}

	vfs.reset();

}

static TigConfig createTigConfig(HINSTANCE hInstance) {
	TigConfig tigConfig;
	memset(&tigConfig, 0, sizeof(tigConfig));
	tigConfig.minTexWidth = 1024;
	tigConfig.minTexHeight = 1024;
	// From ToEE's pov we now always run windowed
	tigConfig.flags = SF_VSYNC | SF_WINDOW;
	// This should no longer be used since we completely replaced this part of tig
	// tigConfig.wndproc = (int)windowproc;
	tigConfig.framelimit = 100;
	// NOTE: These are the actual render sizes, we use a different size for the window / presentation
	tigConfig.width = config.renderWidth;
	tigConfig.height = config.renderHeight;
	tigConfig.bpp = 32;
	tigConfig.hinstance = hInstance;
	tigConfig.findSound = tigInternal.FindSound;

	if (config.antialiasing) {
		tigConfig.flags |= SF_ANTIALIASING;
	}
	if (config.mipmapping) {
		tigConfig.flags |= SF_MIPMAPPING;
	}
	tigConfig.flags |= 0x1000; // Usage unknown
	tigConfig.soundSystem = "miles";

	// No longer used
	// tigConfig.createBuffers = tigInternal.GameCreateVideoBuffers;
	// tigConfig.freeBuffers = tigInternal.GameFreeVideoBuffers;

	if (config.showFps) {
		tigConfig.flags |= SF_FPS;
	}
	if (config.noSound) {
		tigConfig.flags |= SF_NOSOUND;
	}
	if (config.doublebuffer) {
		tigConfig.flags |= SF_DOUBLEBUFFER;
	}
	if (config.animCatchup) {
		tigConfig.flags |= SF_ANIMCATCHUP;
	}
	return tigConfig;
}


bool ModOverride::IsModFolder(const std::string& path)
{
	auto specFilePath = fmt::format("{}\\mod_specs.json", path);
	if (!vfs->FileExists(specFilePath))
		return false;
	return true;
}
