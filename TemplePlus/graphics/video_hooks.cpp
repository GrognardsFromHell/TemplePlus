#include "video_hooks.h"

#include <MinHook.h>

#include <platform/windows.h>

#include <infrastructure/renderstates.h>
#include <infrastructure/exception.h>
#include <infrastructure/stringutil.h>
#include <infrastructure/logging.h>
#include <util/folderutils.h>
#include <shlwapi.h>

#include <d3d8to9_vertexbuffer.h>
#include <d3d8to9_texture.h>
#include <d3d8to9_device.h>

#include "graphics.h"
#include "movies.h"
#include "util/config.h"
#include "util/fixes.h"
#include "mainwindow.h"
#include <tig/tig_startup.h>

#include "legacyvideosystem.h"

VideoFuncs videoFuncs;
temple::GlobalStruct<VideoData, 0x11E74580> video;

static class VideoFixes : TempleFix {
public:

	const char* name() override {
		return "Graphics Engine Fixes";
	}

	void apply() override;

private:
	static int BeginFrame();
	static int PresentFrame();
	static bool AllocTextureMemory(Direct3DDevice8Adapter* adapter,
	                               int w,
	                               int h,
	                               int flags,
	                               Direct3DTexture8Adapter** textureOut,
	                               int* textureTypePtr);
	static int CleanUpBuffers();
	static int SetVideoMode(int adapter,
	                        int nWidth,
	                        int nHeight,
	                        int bpp,
	                        int refresh,
	                        int flags);

	static void GetSystemMemory(int* totalMem, int* availableMem);
	static void TakeScreenshot(int);
	static void UpdateProjMatrices(const TigMatrices& matrices);
	static void TakeSaveScreenshots();
} fix;

/*
This is a temporary hack that forces alpha testing to be enabled when textured shaders are drawn.
Usually they rely on it being true, but never actually make sure to set it to true in the render
states. In reality this is actually an issue with the *other* shader that sets it to false but
doesn't reset it, but that is harder to fix.
See GitHub issue #88
*/
typedef int (*FngHookedShaderTexturedRender)(int vertexCount, void* pos, void* normals, void* diffuse, void* uv, int primCount, void* indices, void* shaderData);
FngHookedShaderTexturedRender OrgHookedShaderTexturedRender;

static int HookedShaderTexturedRender(int vertexCount, void* pos, void* normals, void* diffuse, void* uv, int primCount, void* indices, void* shaderData) {
	renderStates->SetAlphaTestEnable(true);
	return OrgHookedShaderTexturedRender(vertexCount, pos, normals, diffuse, uv, primCount, indices, shaderData);
}

void VideoFixes::apply() {

	/*
	These assertions are based on mallocs or memsets in the code that allow us to deduce the original struct
	size.
	*/
	static_assert(sizeof(VideoData) == 4796, "Video Data struct has the wrong size.");

	// We only differ between borderless and normal window mode.
	videoFuncs.startupFlags = SF_WINDOW;

	MH_CreateHook(videoFuncs.updateProjMatrices, UpdateProjMatrices, reinterpret_cast<LPVOID*>(&videoFuncs.updateProjMatrices));

	// Fixes alpha testing inconsistencies with textured shaders
	MH_CreateHook(temple::GetPointer(0x101E22B0), HookedShaderTexturedRender, (void**)&OrgHookedShaderTexturedRender);

	// Hook into present frame to do after-frame stuff
	MH_CreateHook(temple::GetPointer(0x101DCB80), PresentFrame, nullptr);
	MH_CreateHook(temple::GetPointer(0x101D8870), BeginFrame, nullptr);

	MH_CreateHook(videoFuncs.SetVideoMode, SetVideoMode, reinterpret_cast<LPVOID*>(&videoFuncs.SetVideoMode));
	MH_CreateHook(videoFuncs.CleanUpBuffers, CleanUpBuffers, reinterpret_cast<LPVOID*>(&videoFuncs.CleanUpBuffers));

	// We hook the entire video subsystem initialization function
	MH_CreateHook(temple::GetPointer<0x101DBC80>(), AllocTextureMemory, nullptr);
	MH_CreateHook(temple::GetPointer<0x101E0750>(), GetSystemMemory, nullptr);
	MH_CreateHook(temple::GetPointer<0x101DBD80>(), TakeScreenshot, nullptr);
	MH_CreateHook(temple::GetPointer<0x10002830>(), TakeSaveScreenshots, nullptr);

	hook_movies();

}

// Delegate to the new begin frame
int VideoFixes::BeginFrame() {
	if (!graphics->BeginFrame()) {
		return 1;
	}
	return 0;
}

// Delegate to the new present frame
int VideoFixes::PresentFrame() {
	if (!graphics->Present()) {
		return 1;
	}
	return 0;
}

struct TempleTextureType {
	D3DFORMAT d3dFormat;
	int fallbackIndex;
};

struct TempleTextureTypeTable {
	TempleTextureType formats[8];
};

static temple::GlobalStruct<TempleTextureTypeTable, 0x102A05A8> textureFormatTable;

bool VideoFixes::AllocTextureMemory(Direct3DDevice8Adapter* adapter, int w, int h, int flags, Direct3DTexture8Adapter** textureOut, int* textureTypePtr) {
	auto device = adapter->delegate;

	int levels = 1;
	D3DFORMAT format;
	IDirect3DTexture9* texture = nullptr;

	auto textureType = *textureTypePtr;
	auto desiredType = textureFormatTable->formats[textureType];
	format = desiredType.d3dFormat;

	DWORD usage = config.useDirect3d9Ex ? D3DUSAGE_DYNAMIC : 0;

	// d3d9ex does not support managed anymore, but default has better guarantees now anyway
	D3DPOOL pool = config.useDirect3d9Ex ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
	if (flags & 0x40) {
		// This previously allocated a render target texture, but this is not supported and i do not think used by ToEE
		throw TempleException("Render Targets are not supported by this method");
	}

	if (flags & 0x20 && video->enableMipMaps) {
		levels = 2;
	}

	if (D3DLOG(device->CreateTexture(
		w,
		h,
		levels,
		usage,
		format,
		pool,
		&texture,
		nullptr
	)) != D3D_OK) {
		return false;
	}

	*textureOut = new Direct3DTexture8Adapter(texture);

	return true;
}

int VideoFixes::CleanUpBuffers() {
	logger->error("Buffer cleanup called");
	return 0;
}

/*
Video mode switching no longer needs to happen.
*/
int VideoFixes::SetVideoMode(int adapter, int nWidth, int nHeight, int bpp, int refresh, int flags) {
	return 0;
}

/*
The original function could not handle PCs with >4GB ram
*/
void VideoFixes::GetSystemMemory(int* totalMem, int* availableMem) {
	MEMORYSTATUS status;

	GlobalMemoryStatus(&status);

	// Max 1GB because of process space limitations
	*totalMem = std::min<SIZE_T>(1024 * 1024 * 1024, status.dwTotalPhys);
	*availableMem = std::min<SIZE_T>(1024 * 1024 * 1024, status.dwAvailPhys);
}

// They keycode parameter can be ignored
void VideoFixes::TakeScreenshot(int) {

	// Calculate output filename
	auto folder = GetScreenshotFolder();
	std::wstring path;
	if (!folder.empty()) {
		folder.append(L"\\");
	}
	for (auto i = 1; i <= 9999; ++i) {
		path = fmt::format(L"{}ToEE{:04d}.jpg", folder, i);
		if (!PathFileExistsW(path.c_str())) {
			break;
		}
	}

	logger->info("Saving screenshot as {}", ucs2_to_local(path));

	// Take screenshot
	graphics->TakeScaledScreenshot(ucs2_to_local(path.c_str()), 0, 0);

}


void VideoFixes::UpdateProjMatrices(const TigMatrices& matrices) {
	TigMatrices modifiedParams;
	// - 0.5f might be better here
	modifiedParams.xOffset = matrices.xOffset + 0.5f;
	modifiedParams.yOffset = matrices.yOffset + 0.5f;
	modifiedParams.scale = matrices.scale;

	videoFuncs.updateProjMatrices(&modifiedParams);

}

// Take screenshots for the savegame
void VideoFixes::TakeSaveScreenshots() {
	graphics->TakeScaledScreenshot("save\\temps.jpg", 64, 48);
	graphics->TakeScaledScreenshot("save\\templ.jpg", 256, 192);
}

class LegacyResourceManager : public ResourceListener {
public:

	explicit LegacyResourceManager(Graphics& graphics) : mRegistration(graphics, this) {
	}

	void CreateResources(Graphics&) override;
	void FreeResources(Graphics&) override;

private:
	CComPtr<IDirect3DVertexBuffer9> mRenderQuadBuffer;
	CComPtr<IDirect3DVertexBuffer9> mSharedVBuffer2;
	CComPtr<IDirect3DVertexBuffer9> mSharedVBuffer3;
	CComPtr<IDirect3DVertexBuffer9> mSharedVBuffer4;

	ResourceListenerRegistration mRegistration;
};

void LegacyResourceManager::CreateResources(Graphics& g) {

	logger->info("Creating legacy graphics resources...");

	videoFuncs.buffersFreed = false;

	// Store back buffer size
	const auto& backBufferDesc = g.backBufferDesc();
	videoFuncs.backbufferWidth = backBufferDesc.Width;
	videoFuncs.backbufferHeight = backBufferDesc.Height;

	auto device = g.device();
	
	if (D3DLOG(device->CreateVertexBuffer(
		140, // Space for 5 vertices
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW, // 28 bytes per vertex
		D3DPOOL_SYSTEMMEM,
		&mRenderQuadBuffer,
		nullptr)) != D3D_OK) {
		throw TempleException("Couldn't create shared vertex buffer");
	}
	videoFuncs.renderQuadBuffer = new Direct3DVertexBuffer8Adapter(mRenderQuadBuffer);

	if (D3DLOG(device->CreateVertexBuffer(
		56, // 2 vertices
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW, // 28 bytes per vertex
		D3DPOOL_SYSTEMMEM,
		&mSharedVBuffer2,
		nullptr)) != D3D_OK) {
		throw TempleException("Couldn't create shared vertex buffer");
	}
	videoFuncs.sharedVBuffer2 = new Direct3DVertexBuffer8Adapter(mSharedVBuffer2);

	if (D3DLOG(device->CreateVertexBuffer(
		7168, // 256 vertices
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW, // 28 bytes per vertex
		D3DPOOL_SYSTEMMEM,
		&mSharedVBuffer3,
		nullptr)) != D3D_OK) {
		throw TempleException("Couldn't create shared vertex buffer");
	}
	videoFuncs.sharedVBuffer3 = new Direct3DVertexBuffer8Adapter(mSharedVBuffer3);

	if (D3DLOG(device->CreateVertexBuffer(
		4644, // 129 device
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_XYZ, // 36 byte per vertex
		D3DPOOL_SYSTEMMEM,
		&mSharedVBuffer4,
		nullptr)) != D3D_OK) {
		throw TempleException("Couldn't create shared vertex buffer");
	}
	videoFuncs.sharedVBuffer4 = new Direct3DVertexBuffer8Adapter(mSharedVBuffer4);

	// This is always the same pointer although it's callback 2 of the GameStartConfig	
	videoFuncs.PartSysCreateBuffers();


}

void LegacyResourceManager::FreeResources(Graphics&) {

	logger->info("Freeing legacy graphics resources...");

	videoFuncs.buffersFreed = true;
	
	videoFuncs.tigMatrices2 = videoFuncs.screenTransform;

	videoFuncs.PartSysFreeBuffers();
	videoFuncs.TigShaderFreeBuffers();
	
	mRenderQuadBuffer.Release();
	mSharedVBuffer2.Release();
	mSharedVBuffer3.Release();
	mSharedVBuffer4.Release();
}

LegacyVideoSystem::LegacyVideoSystem(MainWindow& mainWindow, Graphics& graphics) {
	memset(video, 0, 4796);

	video->adapter = 0;

	//video->d3d = new Direct3D8Adapter;
	//video->d3d->delegate = mDirect3d9;

	video->fullscreen = false;
	video->unk2 = false;
	video->gammaSupported = false;

	video->d3dDevice = new Direct3DDevice8Adapter;
	video->d3dDevice->delegate = graphics.device();

	// Make some caps available for other legacy systems
	const auto& caps = graphics.GetCaps();
	video->maxActiveLights = std::min<DWORD>(8, caps.MaxActiveLights);

	/* Probably without effect */
	if (video->makesSthLarger) {
		video->neverReadFlag1 = 4096;
		video->neverReadFlag2 = 16;
	} else {
		video->neverReadFlag1 = 2048;
		video->neverReadFlag2 = 0;
	}
	if ((caps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0) {
		video->capPowerOfTwoTextures = true;
	}
	if ((caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) != 0) {
		video->capSquareTextures = true;
	}

	// TODO: Validate necessity of all these	
	video->hinstance = mainWindow.GetHinstance();
	video->hwnd = mainWindow.GetHwnd(); // TODO: Check necessity
	video->width = config.renderWidth;
	video->height = config.renderHeight;
	video->halfWidth = video->width * 0.5f;
	video->halfHeight = video->height * 0.5f;
	video->current_bpp = 32;
	temple::WriteMem<0x10D24E0C>(0);
	temple::WriteMem<0x10D24E10>(0);
	temple::WriteMem<0x10D24E14>(config.renderWidth);

	video->unusedCap = 0;
	video->makesSthLarger = 0;
	video->capPowerOfTwoTextures = 0;
	video->capSquareTextures = 0;
	video->neverReadFlag1 = 0;
	video->neverReadFlag2 = 0;
	video->dword_11E7572C = 0;
	video->enableMipMaps = config.mipmapping;

	// This is only really used by alloc_texture_mem and the init func
	video->adapterformat = D3DFMT_X8R8G8B8;
	video->current_bpp = 32;

	// memcpy(&video->screenSizeRect, &windowRect, sizeof(RECT));

	// Seems always enabled in default config and never read
	// temple::GetRef<0x11E7570C, int>() = (settings->flags & 4) != 0;

	// These may actually no longer be needed since we replaced the referencing subsystems directly
	// temple::GetRef<void*>(0x10D25134) = settings->createBuffers;
	// temple::GetRef<void*>(0x10D25138) = settings->freeBuffers;

	// Scratchbuffer size sometimes doesn't seem to be set by ToEE itself
	video->current_width = config.renderWidth;
	video->current_height = config.renderHeight;
	temple::WriteMem<0x10307284>(video->current_width);
	temple::WriteMem<0x10307288>(video->current_height);

	video->maxActiveTextures = 4; // We do not accept less than 4

	// temple::GetRef<0x10D250E0, int>() = 0; // Seems unused
	temple::WriteMem(0x10D250E4, 1); // video_initialized
	// temple::GetRef<0x10300914, int>() = -1; Related to mk screenshot

	// Unused mkscreenshot related pointer
	// temple_set<0x10D2511C, int>(0);

	/* This stuff doesn't really seem to be used. */
	/*uint32_t v3 = 0x10D24CAC;
									 do {
									 temple::GetRef<int>(v3) = 0;
									 v3 += 12;
									 } while (v3 < 0x10D24D6C);

									 v3 = 0x10D24C8C;
									 do {
									 temple::GetRef<int>(v3) = 0;
									 v3 += 8;
									 } while (v3 < 0x10D24CAC);*/

	D3DXMatrixIdentity(&video->stru_11E75788);
	D3DXMatrixIdentity(&video->matrix_identity);

	// Seems to be 4 VECTOR3's for the screen corners
	auto fadeScreenRect = videoFuncs.fadeScreenRect.ptr();
	fadeScreenRect[0] = 0;
	fadeScreenRect[1] = 0;
	fadeScreenRect[2] = 0;
	fadeScreenRect[3] = (float)video->current_width;
	fadeScreenRect[4] = 0;
	fadeScreenRect[5] = 0;
	fadeScreenRect[6] = (float)video->current_width;
	fadeScreenRect[7] = (float)video->current_height;
	fadeScreenRect[8] = 0;
	fadeScreenRect[9] = 0;
	fadeScreenRect[10] = (float)video->current_height;
	fadeScreenRect[11] = 0;

	video->unusedCap = 1; // Seems to be ref'd from light_init

	__asm mov eax, D3DFMT_X8R8G8B8
	if (!videoFuncs.tig_d3d_init_handleformat()) {
		logger->error("Format init failed.");
	}

	videoFuncs.tig_font_related_init();
	videoFuncs.updateProjMatrices(videoFuncs.tigMatrices2);

	mResources = std::make_unique<LegacyResourceManager>(graphics);
}

LegacyVideoSystem::~LegacyVideoSystem() {
}
