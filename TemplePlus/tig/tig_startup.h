#pragma once

#include <vector>
#include <memory>

// Functions used to init subsystems

// Observed in window mode: 0x11024
// 0x4 seems to be the default (seems to be VSYNC)
// 0x20 is windowed
// 0x1000 is unknown
// 0x10000 means anti aliasing is turned on
enum StartupFlag {
	SF_FPS = 0x1, // -fps
	SF_VSYNC = 0x4, // TODO: This is probably wrong. 0x4 gets tested against d3d flags, not these flags
	SF_NOSOUND = 0x2000, // -nosound
	SF_DOUBLEBUFFER = 0x2, // -doublebuffer
	SF_ANIMCATCHUP, // -animcatchup (stored elsewhere)
	SF_ANIMDEBUG, // -animdebug (not supported)
	SF_NORANDOM,// -norandom stored in 10BDDD9C
	SF_NONMSMOUSEZ, // -nonmsmousez (stores 0 in 10300974)
	SF_MOUSEZ, // -msmousez (stores 1 in 10300974)
	SF_2680, // -2680 (stores 1 in 103072E0)
	SF_0897, // -0897 (stores 2 in 103072E0)
	SF_4637, // -4637 (stores 3 in 103072E0)
	SF_PATHLIMIT, // -pathlimitNN (stored in 102AF7C0, max seems to be 35, default 10)
	SF_SHADOW_POLY, // -shadow_poly, shadow_debug_flag = 1
	SF_SHADOW_MAP, // -shadow_map, shadow_debug_flag = 2
	SF_SHADOW_BLOBBY, // -shadow_blobby, shadow_debug_flag = 0
	SF_WINDOW = 0x20, // -window
	SF_GEOMETRY, // -geometry<X>x<Y> sets window width + height
	SF_MAXREFRESH, // -maxrefreshNNN apparently sets a framelimit, default=100
	SF_ANTIALIASING = 0x10000, // -noantialiasing inverts this
	SF_MIPMAPPING = 0x20000, // -mipmapping

	SF_FLAG_uint32_t = 0x7FFFFFFF
};

// 19 values total (guessed from memset 0 at start of main method)
struct TigConfig {
	uint32_t flags;
	int32_t x;
	int32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t bpp;
	HINSTANCE hinstance;
	uint32_t unk7; // Set to 0
	uint32_t unk8; // Set to 0
	WNDPROC wndproc;
	bool (__cdecl *windowMessageFilter)(MSG* msg);
	int (__cdecl *findSound)(int soundId, char* filenameOut); // @1003B9E0
	const char* soundSystem; // "miles"
	uint32_t minTexWidth;
	uint32_t minTexHeight;
	uint32_t framelimit;
	const char* windowTitle;
	void (__cdecl *createBuffers)();
	void (__cdecl *freeBuffers)();
};

class MainWindow;
namespace gfx {
	class RenderingDevice;
	class ShapeRenderer2d;
	class MdfMaterialFactory;
}
namespace temple {
	class SoundSystem;
	class MovieSystem;
}
class TextLayouter;

// RAII for TIG initialization
class TigInitializer {
public:
	explicit TigInitializer(HINSTANCE hInstance);
	~TigInitializer();

	const TigConfig& GetConfig() const {
		return mConfig;
	}

	MainWindow &GetMainWindow() {
		return *mMainWindow;
	}

	//Graphics& GetGraphics() {
	//	return *mGraphics;
	//}

	gfx::RenderingDevice &GetRenderingDevice() {
		return *mRenderingDevice;
	}
	
	TextLayouter& GetTextLayouter() {
		return *mTextLayouter;
	}

	gfx::MdfMaterialFactory& GetMdfFactory() {
		return *mMdfFactory;
	}

	gfx::ShapeRenderer2d& GetShapeRenderer2d() {
		return *mShapeRenderer2d;
	}

	temple::SoundSystem& GetSoundSystem() {
		return *mSoundSystem;
	}

	temple::MovieSystem& GetMovieSystem() {
		return *mMovieSystem;
	}

private:

	using TigSystemPtr = std::unique_ptr<class LegacyTigSystem>;

	void LoadDataFiles();

	/*
		Starts a legacy system still contained in temple.dll
		and returns a TigSystem that will shutdown the system
		when destroyed.
	*/
	TigSystemPtr StartSystem(const std::string& name,
	                         uint32_t startAddress,
	                         uint32_t shutdownAddr);

	TigConfig mConfig;

	std::unique_ptr<MainWindow> mMainWindow;
	std::unique_ptr<gfx::RenderingDevice> mRenderingDevice;
	std::unique_ptr<TextLayouter> mTextLayouter;
	std::unique_ptr<gfx::MdfMaterialFactory> mMdfFactory;
	std::unique_ptr<gfx::ShapeRenderer2d> mShapeRenderer2d;
	std::unique_ptr<temple::SoundSystem> mSoundSystem;
	std::unique_ptr<temple::MovieSystem> mMovieSystem;
	std::unique_ptr<class LegacyVideoSystem> mLegacyVideoSystem;
	std::unique_ptr<class MessageQueue> mMessageQueue;

	// Contains all systems that have already been started
	std::vector<TigSystemPtr> mStartedSystems;
};

extern TigInitializer* tig;