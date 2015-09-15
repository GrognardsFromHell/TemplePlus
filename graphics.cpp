#include "stdafx.h"

#include "graphics.h"
#include "temple_functions.h"
#include "util/addresses.h"
#include "idxtables.h"
#include "util/fixes.h"
#include "movies.h"
#include "tig/tig_msg.h"
#include "tig/tig_shader.h"
#include "tig/tig_mouse.h"
#include "mainwindow.h"
#include "ui/ui.h"
#include "util/folderutils.h"
#include "gamesystems.h"
#include "renderstates.h"
#include "legacyrenderstates.h"
#include "ui/ui_text.h"
#include <atlbase.h>

#include "d3d.h"
#include "d3d8adapter.h"
#include "d3d8to9_device.h"
#include "d3d8to9_vertexbuffer.h"
#include "d3d8to9_texture.h"
#include "d3d8to9_rootobj.h"

#include <map>
#include <set>
#include "util/config.h"

Graphics graphics;
VideoFuncs videoFuncs;
GlobalStruct<VideoData, 0x11E74580> video;

// Our precompiled header swallows this somehow...
static const DWORD D3D_SDK_VERSION = 32;

template<typename T>
static void FreeD3dResource(T *&unk) {
	if (unk) {
		unk->Release();
		unk = nullptr;
	}
}

static D3DPRESENT_PARAMETERS CreatePresentParams() {
	D3DPRESENT_PARAMETERS presentParams;
	memset(&presentParams, 0, sizeof(presentParams));

	presentParams.BackBufferFormat = D3DFMT_X8R8G8B8;
	// Using discard here allows us to do multisampling.
	presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParams.hDeviceWindow = video->hwnd;
	presentParams.Windowed = true;
	presentParams.EnableAutoDepthStencil = true;
	presentParams.AutoDepthStencilFormat = D3DFMT_D16;
	presentParams.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	presentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	return presentParams;
}

static void SetDefaultRenderStates(IDirect3DDevice9* d3d9Device) {
	/*
	SET DEFAULT RENDER STATES
	*/
	d3d9Device->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3d9Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	d3d9Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	d3d9Device->SetRenderState(D3DRS_LIGHTING, TRUE);

	D3DLIGHT9 light;
	memset(&light, 0, sizeof(light));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 1.5f;
	light.Diffuse.g = 1.5f;
	light.Diffuse.b = 1.5f;
	light.Specular.r = 1.0f;
	light.Specular.g = 1.0f;
	light.Specular.b = 1.0f;
	light.Direction.x = -0.70700002f;
	light.Direction.y = -0.866f;
	light.Attenuation0 = 1;
	light.Range = 800;
	d3d9Device->SetLight(0, &light);
	d3d9Device->SetRenderState(D3DRS_AMBIENT, 0);
	d3d9Device->SetRenderState(D3DRS_SPECULARENABLE, 0);
	d3d9Device->SetRenderState(D3DRS_LOCALVIEWER, 0);

	D3DMATERIAL9 material;
	memset(&material, 0, sizeof(material));
	material.Diffuse.r = 1.0f;
	material.Diffuse.g = 1.0f;
	material.Diffuse.b = 1.0f;
	material.Diffuse.a = 1.0f;
	material.Ambient.r = 1.0f;
	material.Ambient.g = 1.0f;
	material.Ambient.b = 1.0f;
	material.Ambient.a = 1.0f;
	material.Power = 50.0f;
	d3d9Device->SetMaterial(&material);

	handleD3dError("SetRenderState", d3d9Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	handleD3dError("SetRenderState", d3d9Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	handleD3dError("SetRenderState", d3d9Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

	handleD3dError("SetTextureStageState", d3d9Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1));
	handleD3dError("SetTextureStageState", d3d9Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTOP_SELECTARG1));
	handleD3dError("SetTextureStageState", d3d9Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTOP_DISABLE));

	for (DWORD i = 0; i < video->maxActiveTextures; ++i) {
		handleD3dError("SetSamplerState", d3d9Device->SetSamplerState(i, D3DSAMP_MINFILTER, 1));
		handleD3dError("SetSamplerState", d3d9Device->SetSamplerState(i, D3DSAMP_MAGFILTER, 2));
		handleD3dError("SetSamplerState", d3d9Device->SetSamplerState(i, D3DSAMP_MIPFILTER, 1));
		handleD3dError("SetSamplerState", d3d9Device->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, 0));
		handleD3dError("SetSamplerState", d3d9Device->SetSamplerState(i, D3DSAMP_MAXMIPLEVEL, 01));
		handleD3dError("SetSamplerState", d3d9Device->SetSamplerState(i, D3DSAMP_MINFILTER, 1));
		handleD3dError("SetSamplerState", d3d9Device->SetSamplerState(i, D3DSAMP_MINFILTER, 1));
		handleD3dError("SetSamplerState", d3d9Device->SetSamplerState(i, D3DSAMP_ADDRESSU, 3));
		handleD3dError("SetSamplerState", d3d9Device->SetSamplerState(i, D3DSAMP_ADDRESSV, 3));
		handleD3dError("SetTextureStageState", d3d9Device->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, 0));
		handleD3dError("SetTextureStageState", d3d9Device->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, 0));
	}

	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	handleD3dError("SetTransform", d3d9Device->SetTransform(D3DTS_TEXTURE0, &identity));

	d3d9Device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	d3d9Device->SetRenderState(D3DRS_ALPHAREF, 1);
	d3d9Device->SetRenderState(D3DRS_ALPHAFUNC, 7);
}

void CreateSharedVertexBuffers(IDirect3DDevice9* device) {
	IDirect3DVertexBuffer9* vbuffer;
	if (D3DLOG(device->CreateVertexBuffer(
		112, // Space for 4 vertices
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW, // 28 bytes per vertex
		D3DPOOL_SYSTEMMEM,
		&vbuffer,
		nullptr)) != D3D_OK) {
		throw TempleException("Couldn't create shared vertex buffer");
	}
	video->blitVBuffer = new Direct3DVertexBuffer8Adapter(vbuffer);
	
	if (D3DLOG(device->CreateVertexBuffer(
		140, // Space for 5 vertices
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW, // 28 bytes per vertex
		D3DPOOL_SYSTEMMEM,
		&vbuffer,
		nullptr)) != D3D_OK) {
		throw TempleException("Couldn't create shared vertex buffer");
	}
	videoFuncs.globalFadeVBuffer = new Direct3DVertexBuffer8Adapter(vbuffer);

	if (D3DLOG(device->CreateVertexBuffer(
		72, // 2 vertices (odd)
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_XYZ, // 36 byte per vertex
		D3DPOOL_SYSTEMMEM,
		&vbuffer,
		nullptr)) != D3D_OK) {
		throw TempleException("Couldn't create shared vertex buffer");
	}
	videoFuncs.sharedVBuffer1 = new Direct3DVertexBuffer8Adapter(vbuffer);

	if (D3DLOG(device->CreateVertexBuffer(
		56, // 2 vertices
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW, // 28 bytes per vertex
		D3DPOOL_SYSTEMMEM,
		&vbuffer,
		nullptr)) != D3D_OK) {
		throw TempleException("Couldn't create shared vertex buffer");
	}
	videoFuncs.sharedVBuffer2 = new Direct3DVertexBuffer8Adapter(vbuffer);

	if (D3DLOG(device->CreateVertexBuffer(
		7168, // 256 vertices
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW, // 28 bytes per vertex
		D3DPOOL_SYSTEMMEM,
		&vbuffer,
		nullptr)) != D3D_OK) {
		throw TempleException("Couldn't create shared vertex buffer");
	}
	videoFuncs.sharedVBuffer3 = new Direct3DVertexBuffer8Adapter(vbuffer);

	if (D3DLOG(device->CreateVertexBuffer(
		4644, // 129 device
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_XYZ, // 36 byte per vertex
		D3DPOOL_SYSTEMMEM,
		&vbuffer,
		nullptr)) != D3D_OK) {
		throw TempleException("Couldn't create shared vertex buffer");
	}
	videoFuncs.sharedVBuffer4 = new Direct3DVertexBuffer8Adapter(vbuffer);
}

void ResizeBuffers(int width, int height) {

	if (!video->d3dDevice) {
		return;
	}

	auto device = video->d3dDevice->delegate;

	// TODO: Handle non d3d9ex case
	auto presentParams = CreatePresentParams();
	device->ResetEx(&presentParams, nullptr);

	video->width = width;
	video->height = height;
	video->halfWidth = video->width * 0.5f;
	video->halfHeight = video->height * 0.5f;

	video->current_width = width;
	video->current_height = height;
	videoFuncs.updateProjMatrices(videoFuncs.tigMatrices2);

	videoFuncs.GameFreeVideoBuffers();
	videoFuncs.GameCreateVideoBuffers();

	graphics.UpdateScreenSize(width, height);

	temple_set<0x10D24E14>(width);

	// Mouse cursor disasppers after resizing
	mouseFuncs.RefreshCursor();

	gameSystemFuncs.ResizeScreen(width, height);

	mouseFuncs.SetBounds(width, height);
}

void ReadCaps(IDirect3DDevice9Ex* device, uint32_t minTexWidth, uint32_t minTexHeight) {

	D3DCAPS9 caps;
	if (D3DLOG(device->GetDeviceCaps(&caps)) != D3D_OK) {
		throw TempleException("Unable to retrieve Direct3D device caps");
	}
	video->maxActiveLights = min<DWORD>(8, caps.MaxActiveLights);

	/*
	Several sanity checks follow
	*/
	if (!(caps.SrcBlendCaps & D3DPBLENDCAPS_SRCALPHA)) {
		logger->error("source D3DPBLENDCAPS_SRCALPHA is missing");
	}
	if (!(caps.SrcBlendCaps & D3DPBLENDCAPS_ONE)) {
		logger->error("source D3DPBLENDCAPS_ONE is missing");
	}
	if (!(caps.SrcBlendCaps & D3DPBLENDCAPS_ZERO)) {
		logger->error("source D3DPBLENDCAPS_ZERO is missing");
	}
	if (!(caps.DestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)) {
		logger->error("destination D3DPBLENDCAPS_INVSRCALPHA is missing");
	}
	if (!(caps.DestBlendCaps & D3DPBLENDCAPS_ONE)) {
		logger->error("destination D3DPBLENDCAPS_ONE is missing");
	}
	if (!(caps.DestBlendCaps & D3DPBLENDCAPS_ZERO)) {
		logger->error("destination D3DPBLENDCAPS_ZERO is missing");
	}

	if (caps.MaxSimultaneousTextures < 4) {
		logger->error("less than 4 active textures possible: {}", caps.MaxSimultaneousTextures);
	}
	if (caps.MaxTextureBlendStages < 4) {
		logger->error("less than 4 texture blend stages possible: {}", caps.MaxTextureBlendStages);
	}
	video->maxActiveTextures = 4; // We do not accept less than 4

	if (!(caps.TextureOpCaps & D3DTOP_DISABLE)) {
		logger->error("texture op D3DTOP_DISABLE is missing");
	}
	if (!(caps.TextureOpCaps & D3DTOP_SELECTARG1)) {
		logger->error("texture op D3DTOP_SELECTARG1 is missing");
	}
	if (!(caps.TextureOpCaps & D3DTOP_SELECTARG2)) {
		logger->error("texture op D3DTOP_SELECTARG2 is missing");
	}
	if (!(caps.TextureOpCaps & D3DTOP_BLENDTEXTUREALPHA)) {
		logger->error("texture op D3DTOP_BLENDTEXTUREALPHA is missing");
	}
	if (!(caps.TextureOpCaps & D3DTOP_BLENDCURRENTALPHA)) {
		logger->error("texture op D3DTOP_BLENDCURRENTALPHA is missing");
	}
	if (!(caps.TextureOpCaps & D3DTOP_MODULATE)) {
		logger->error("texture op D3DTOP_MODULATE is missing");
	}
	if (!(caps.TextureOpCaps & D3DTOP_ADD)) {
		logger->error("texture op D3DTOP_ADD is missing");
	}
	if (!(caps.TextureOpCaps & D3DTOP_MODULATEALPHA_ADDCOLOR)) {
		logger->error("texture op D3DTOP_MODULATEALPHA_ADDCOLOR is missing");
	}
	if (caps.MaxTextureWidth < minTexWidth || caps.MaxTextureHeight < minTexHeight) {
		auto msg = format("minimum texture resolution of {}x{} is not supported. Supported: {}x{}",
		                  minTexWidth, minTexHeight, caps.MaxTextureWidth, caps.MaxTextureHeight);
		throw TempleException(msg);
	}

	/*
		Vermutlich kein Effekt
	*/
	if (video->makesSthLarger) {
		video->neverReadFlag1 = 4096;
		video->neverReadFlag2 = 16;
	} else {
		video->neverReadFlag1 = 2048;
		video->neverReadFlag2 = 0;
	}

	if ((caps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0) {
		logger->error("Textures must be power of two");
		video->capPowerOfTwoTextures = true;
	}
	if ((caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) != 0) {
		logger->error("Textures must be square");
		video->capSquareTextures = true;
	}
}

int __cdecl HookedCleanUpBuffers() {
	logger->error("Buffer cleanup called");
	return 0;
}

/*
	Video mode switching no longer needs to happen.
*/
int __cdecl HookedSetVideoMode(int adapter, int nWidth, int nHeight, int bpp, int refresh, int flags) {
	return 0;
}

int __cdecl VideoStartup(TigConfig* settings) {
	memset(video, 0, 4796);

	bool windowed = config.windowed;

	video->adapter = 0;

	// create window call
	if (!CreateMainWindow(settings)) {
		return 17;
	}

	graphics.InitializeDirect3d();

	video->width = settings->width;
	video->height = settings->height;
	video->halfWidth = video->width * 0.5f;
	video->halfHeight = video->height * 0.5f;

	// Seems always enabled in default config and never read
	temple_set<0x11E7570C, int>((settings->flags & 4) != 0);

	video->current_bpp = settings->bpp;

	temple_set<0x10D250E0, int>(0);
	temple_set<0x10D250E4, int>(1);
	temple_set<0x10300914, int>(-1);

	// Unused mkscreenshot related pointer
	// temple_set<0x10D2511C, int>(0);

	/*
		This stuff doesn't really seem to be used.
	*/
	uint32_t v3 = 0x10D24CAC;
	do {
		temple_set(v3, 0);
		v3 += 12;
	} while (v3 < 0x10D24D6C);

	v3 = 0x10D24C8C;
	do {
		temple_set(v3, 0);
		v3 += 8;
	} while (v3 < 0x10D24CAC);

	// These may actually no longer be needed since we replaced the referencing subsystems directly
	temple_set<0x10D25134>(settings->createBuffers);
	temple_set<0x10D25138>(settings->freeBuffers);

	memcpy(temple_address<0x11E75840>(), settings, 0x4C);

	uiText.Initialize();

	return 0;
}

struct TempleTextureType {
	D3DFORMAT d3dFormat;
	int fallbackIndex;
};

struct TempleTextureTypeTable {
	TempleTextureType formats[8];
};

GlobalStruct<TempleTextureTypeTable, 0x102A05A8> textureFormatTable;

bool __cdecl AllocTextureMemory(Direct3DDevice8Adapter* adapter, int w, int h, int flags, Direct3DTexture8Adapter** textureOut, int* textureTypePtr) {
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

/*
	The original function could not handle PCs with >4GB ram
*/
void GetSystemMemory(int* totalMem, int* availableMem) {
	MEMORYSTATUS status;

	GlobalMemoryStatus(&status);

	// Max 1GB because of process space limitations
	*totalMem = min<SIZE_T>(1024 * 1024 * 1024, status.dwTotalPhys);
	*availableMem = min<SIZE_T>(1024 * 1024 * 1024, status.dwAvailPhys);
}

// They keycode parameter can be ignored
void __cdecl TakeScreenshot(int) {

	if (!video->d3dDevice) {
		return;
	}

	// Calculate size of screenshot
	RECT rect;
	GetWindowRect(video->hwnd, &rect);
	auto width = rect.right - rect.left;
	auto height = rect.bottom - rect.top;

	// Calculate output filename
	auto folder = GetScreenshotFolder();
	wstring path;
	if (!folder.empty()) {
		folder.append(L"\\");
	}
	for (auto i = 1; i <= 9999; ++i) {
		path = format(L"{}ToEE{:04d}.jpg", folder, i);
		if (!PathFileExistsW(path.c_str())) {
			break;
		}
	}

	// Take screenshot
	auto device = video->d3dDevice->delegate;
	IDirect3DSurface9* surface;
	device->CreateOffscreenPlainSurface(width, height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surface, nullptr);
	if (surface) {
		device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface);
		D3DXSaveSurfaceToFileW(path.c_str(), D3DXIFF_JPG, surface, nullptr, nullptr);
		surface->Release();
	}

}

// Delegate to the new present frame
static int HookedPresentFrame() {
	if (!graphics.Present()) {
		return 1;
	}
	return 0;
}

// Delegate to the new begin frame
static int HookedBeginFrame() {
	if (!graphics.BeginFrame()) {
		return 1;
	}
	return 0;
}

static void HookedUpdateProjMatrices(const TigMatrices& matrices) {
	TigMatrices modifiedParams;
	// - 0.5f might be better here
	modifiedParams.xOffset = matrices.xOffset + 0.5f;
	modifiedParams.yOffset = matrices.yOffset + 0.5f;
	modifiedParams.scale = matrices.scale;

	videoFuncs.updateProjMatrices(&modifiedParams);

}

// Take screenshots for the savegame
static void TakeSaveScreenshots() {
	graphics.TakeScaledScreenshot("save\\temps.jpg", 64, 48);
	graphics.TakeScaledScreenshot("save\\templ.jpg", 256, 192);	
}

void hook_graphics() {
	/*
		These assertions are based on mallocs or memsets in the code that allow us to deduce the original struct
		size.
	*/
	static_assert(sizeof(VideoData) == 4796, "Video Data struct has the wrong size.");

	// We only differ between borderless and normal window mode.
	videoFuncs.startupFlags = SF_WINDOW;

	MH_CreateHook(videoFuncs.updateProjMatrices, HookedUpdateProjMatrices, reinterpret_cast<LPVOID*>(&videoFuncs.updateProjMatrices));

	// Hook into present frame to do after-frame stuff
	MH_CreateHook(temple_address(0x101DCB80), HookedPresentFrame, nullptr);
	MH_CreateHook(temple_address(0x101D8870), HookedBeginFrame, nullptr);

	MH_CreateHook(videoFuncs.SetVideoMode, HookedSetVideoMode, reinterpret_cast<LPVOID*>(&videoFuncs.SetVideoMode));
	MH_CreateHook(videoFuncs.CleanUpBuffers, HookedCleanUpBuffers, reinterpret_cast<LPVOID*>(&videoFuncs.CleanUpBuffers));

	// We hook the entire video subsystem initialization function
	MH_CreateHook(temple_address<0x101DC6E0>(), VideoStartup, nullptr);
	MH_CreateHook(temple_address<0x101DBC80>(), AllocTextureMemory, nullptr);
	MH_CreateHook(temple_address<0x101E0750>(), GetSystemMemory, nullptr);
	MH_CreateHook(temple_address<0x101DBD80>(), TakeScreenshot, nullptr);
	MH_CreateHook(temple_address<0x10002830>(), TakeSaveScreenshots, nullptr);

	hook_movies();
}

static struct ExternalGraphicsFuncs : AddressTable {

	/*
		Advances the clock used by the general shader to animate textures and such.
	*/
	void (__cdecl *AdvanceShaderClock)(float timeInMs);

	/*
		Frees texture memory if more than the memory budget is allocated.
	*/
	void (__cdecl *FreeTextureMemory)();

	/*
		Renders a 2d quad on screen.
	*/
	int (__cdecl *RenderTexturedQuad)(D3DVECTOR* vertices, float* u, float* v, int textureId, D3DCOLOR color);

	/*
		Sadly no idea what this does. Looks like some indexbuffer rotating cache?
	*/
	void (__cdecl *sub_101EF8B0)(void);

	/*
		Converts from screen loc to tile.
	*/
	bool (__cdecl *ScreenToLoc)(int64_t x, int64_t y, locXY &locOut);

	// Is the full-screen overlay for fading out/in enabled?
	bool* gfadeEnable;
	// If it's enabled, what color does it have?
	D3DCOLOR* gfadeColor;

	void (__cdecl *ShakeScreen)(float amount, float duration);

	ExternalGraphicsFuncs() {
		rebase(AdvanceShaderClock, 0x101E0A30);
		rebase(FreeTextureMemory, 0x101EDF00);

		rebase(gfadeEnable, 0x10D25118);
		rebase(gfadeColor, 0x10D24A28);

		rebase(RenderTexturedQuad, 0x101d90b0);
		rebase(sub_101EF8B0, 0x101EF8B0);

		rebase(ScreenToLoc, 0x100290C0);
		rebase(ShakeScreen, 0x10005840);
	}
} externalGraphicsFuncs;

Graphics::Graphics() {
	memset(mScreenCorners, 0, sizeof(mScreenCorners));
}

bool Graphics::BeginFrame() {
	// ToEE supports nested begin/present calls and silently ignores them
	if (++mFrameDepth > 1) {
		return true;
	}

	auto clearColor = D3DCOLOR_ARGB(0, 0, 0, 0);

	auto device = video->d3dDevice->delegate;
	auto result = device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 1.0f, 0);

	if (handleD3dError("Clear", result) != D3D_OK) {
		return false;
	}

	result = device->BeginScene();

	if (handleD3dError("BeginScene", result) != D3D_OK) {
		return false;
	}

	// Advance time of shader clock, used for texture animation
	auto now = graphics_clock::now();
	auto frameTime = chrono::duration_cast<chrono::milliseconds>(now - mLastFrameStart);
	externalGraphicsFuncs.AdvanceShaderClock(static_cast<float>(frameTime.count()));
	mLastFrameStart = graphics_clock::now();

	return true;
}

bool Graphics::Present() {
	if (--mFrameDepth > 0) {
		return true;
	}

	externalGraphicsFuncs.FreeTextureMemory();

	RenderGFade();

	if (D3DLOG(mDevice->EndScene()) != D3D_OK) {
		return false;
	}

	auto result = mDevice->Present(nullptr, nullptr, nullptr, nullptr);

	if (result != S_OK && result != S_PRESENT_OCCLUDED) {
		LogD3dError("Present()", result);
		if (result == D3DERR_DEVICELOST) {
			ResetDevice();
		}
		return false;
	}

	/*
	++dword_10D250F0; <- FPS counter frame count
	*/
	externalGraphicsFuncs.sub_101EF8B0();

	return true;
}

void Graphics::ResetDevice() {

	while (true) {
		// While the cooperation level says
		auto result = mDevice->TestCooperativeLevel();

		// Direct3D documentation tells us to wait and retry periodically if device is lost
		while (result == D3DERR_DEVICELOST) {
			logger->trace("Direct3D9 device is lost. Waiting for chance to restore it");
			msgFuncs.ProcessSystemEvents();
			Sleep(100);
			result = mDevice->TestCooperativeLevel();
		}

		if (result == D3D_OK) {
			return; // Unlikely, but... okay
		}

		// There's also D3DERR_DRIVERINTERNALERROR, but I don't think we can recover from that
		assert(result == D3DERR_DEVICENOTRESET);

		logger->debug("Resetting Direct3D device");

		FreeResources();

		auto presentParams = CreatePresentParams();
		if ((result = D3DLOG(mDevice->Reset(&presentParams))) != D3D_OK) {
			if (result == D3DERR_DEVICELOST) {
				continue;
			}
			throw TempleException("Unable to reset the Direct3d device after it being lost.");
		}

		logger->debug("Successfully reset Direct3D device");

		CreateResources();

		break;
	}

}

void Graphics::FreeResources() {
	logger->info("Freeing Direct3D video memory resources");

	videoFuncs.buffersFreed = false;
	videoFuncs.CleanUpBuffers();
	// videoFuncs.GameFreeVideoBuffers();

	// Handled by CleanUpBuffers
	//FreeD3dResource(video->blitVBuffer);
	//FreeD3dResource(videoFuncs.globalFadeVBuffer);
	//FreeD3dResource(videoFuncs.sharedVBuffer1);
	//FreeD3dResource(videoFuncs.sharedVBuffer2);
	//FreeD3dResource(videoFuncs.sharedVBuffer3);
	//FreeD3dResource(videoFuncs.sharedVBuffer4);

	FreeD3dResource(mBackBuffer);
	FreeD3dResource(mBackBufferDepth);

	FreeD3dResource(mSceneSurface);
	FreeD3dResource(mSceneDepthSurface);

}

void Graphics::CreateResources() {

	if (!renderStates) {
		auto newRenderStates(CreateLegacyRenderStates());
		renderStates.swap(newRenderStates);
	}

	renderStates->Reset();

	videoFuncs.buffersFreed = false;

	CreateSharedVertexBuffers(mDevice);

	// Get the currently attached backbuffer
	if (D3DLOG(mDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &mBackBuffer)) != D3D_OK) {
		throw TempleException("Unable to retrieve the back buffer");
	}
	if (D3DLOG(mDevice->GetDepthStencilSurface(&mBackBufferDepth)) != D3D_OK) {
		throw TempleException("Unable to retrieve depth/stencil surface from device");
	}

	memset(&mBackBufferDesc, 0, sizeof(mBackBufferDesc));
	if (D3DLOG(mBackBuffer->GetDesc(&mBackBufferDesc)) != D3D_OK) {
		throw TempleException("Unable to retrieve back buffer description");
	}

	// Store back buffer size
	videoFuncs.backbufferWidth = mBackBufferDesc.Width;
	videoFuncs.backbufferHeight = mBackBufferDesc.Height;

	// Create surfaces for the scene
	D3DLOG(mDevice->CreateRenderTarget(
		config.renderWidth,
		config.renderHeight,
		mBackBufferDesc.Format,
		mBackBufferDesc.MultiSampleType,
		mBackBufferDesc.MultiSampleQuality,
		FALSE,
		&mSceneSurface,
		nullptr));

	D3DLOG(mDevice->CreateDepthStencilSurface(
		config.renderWidth,
		config.renderHeight,
		D3DFMT_D16,
		mBackBufferDesc.MultiSampleType,
		mBackBufferDesc.MultiSampleQuality,
		TRUE,
		&mSceneDepthSurface,
		nullptr));

	// This is always the same pointer although it's callback 2 of the GameStartConfig	
	videoFuncs.create_partsys_vertex_buffers();
	videoFuncs.GameCreateVideoBuffers();

	// After a reset, the D3D cursor is hidden
	mouseFuncs.RefreshCursor();
}

void Graphics::UpdateScreenSize(int w, int h) {
	mScreenCorners[1].x = static_cast<float>(w);
	mScreenCorners[2].x = static_cast<float>(w);
	mScreenCorners[2].y = static_cast<float>(h);
	mScreenCorners[3].y = static_cast<float>(h);
}

void Graphics::UpdateWindowSize(int w, int h) {
	mWindowWidth = w;
	mWindowHeight = h;
	// TODO: Update back buffer accordingly

	RefreshSceneRect();
}

bool Graphics::ScreenToTile(int x, int y, locXY& tileOut) {
	auto result = externalGraphicsFuncs.ScreenToLoc(x, y, tileOut);
	if (!result) {
		tileOut.locx = 0;
		tileOut.locy = 0;
	}
	return result;
}

void Graphics::ShakeScreen(float amount, float duration) {
	externalGraphicsFuncs.ShakeScreen(amount, duration);
}

void Graphics::RenderGFade() {
	static float quadU[] = {0, 1, 1, 0};
	static float quadV[] = {0, 1, 0, 1};

	if (externalGraphicsFuncs.gfadeEnable) {
		externalGraphicsFuncs.RenderTexturedQuad(mScreenCorners, quadU, quadV, 0, *externalGraphicsFuncs.gfadeColor);
	}
}

void Graphics::RefreshSceneRect() {

	// GFade is drawn after the scene buffer, so this is in window space
	UpdateScreenSize(config.windowWidth, config.windowHeight);

	/*
		Calculates the rectangle within the back buffer that the scene
		will be drawn in. This accounts for "fit to width/height" scenarios
		where the back buffer has a different aspect ratio.
	*/
	float w = static_cast<float>(graphics.windowWidth());
	float h = static_cast<float>(graphics.windowHeight());
	float wFactor = (float)w / config.renderWidth;
	float hFactor = (float)h / config.renderHeight;
	mSceneScale = min(wFactor, hFactor);
	int screenW = (int)round(mSceneScale * config.renderWidth);
	int screenH = (int)round(mSceneScale * config.renderHeight);

	// Center on screen
	RECT rect;
	rect.left = (mWindowWidth - screenW) / 2;
	rect.top = (mWindowHeight - screenH) / 2;
	rect.right = rect.left + screenW;
	rect.bottom = rect.top + screenH;
	mSceneRect = rect;

}

void Graphics::InitializeDirect3d() {

	HRESULT d3dresult;

	/*
	Set some global flags.
	*/
	video->unusedCap = 0;
	video->makesSthLarger = 0;
	video->capPowerOfTwoTextures = 0;
	video->capSquareTextures = 0;
	video->neverReadFlag1 = 0;
	video->neverReadFlag2 = 0;
	video->dword_11E7572C = 0;
	video->enableMipMaps = config.mipmapping;

	if (config.useDirect3d9Ex) {
		d3dresult = D3DLOG(Direct3DCreate9Ex(D3D_SDK_VERSION, &mDirect3d9));
		if (d3dresult != D3D_OK) {
			throw TempleException("Unable to create Direct3D9 interface.");
		}
	} else {
		mDirect3d9 = static_cast<IDirect3D9Ex*>(Direct3DCreate9(D3D_SDK_VERSION));
		if (!mDirect3d9) {
			throw TempleException("Unable to create Direct3D9 interface.");
		}
	}
	video->d3d = new Direct3D8Adapter;
	video->d3d->delegate = mDirect3d9;

	/** START OF OLD WINDOWED INIT */
	// At this point we only do a GetDisplayMode to check the resolution. We could also do this elsewhere
	D3DDISPLAYMODE displayMode;
	d3dresult = mDirect3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);
	if (d3dresult != D3D_OK) {
		handleD3dError("GetAdapterDisplayMode", d3dresult);
		throw TempleException("Unable to query display mode for primary adapter.");
	}

	// We need at least 1024x768
	if (displayMode.Width < 1024 || displayMode.Height < 768) {
		throw TempleException("You need at least a display resolution of 1024x768.");
	}

	// This is only really used by alloc_texture_mem and the init func
	video->adapterformat = D3DFMT_X8R8G8B8;
	video->current_bpp = 32;

	auto presentParams = CreatePresentParams();

	// presentParams.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
	// presentParams.MultiSampleQuality = 0;

	// Nvidia drivers seriously barf on D3d9ex if we use software vertex processing here, as ToEE specifies.
	// I think we are safe with hardware vertex processing, since HW T&L has been standard for more than 10 years.
	if (config.useDirect3d9Ex) {
		logger->info("Creating Direct3D9Ex device.");
		d3dresult = D3DLOG(mDirect3d9->CreateDeviceEx(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			video->hwnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&presentParams,
			nullptr,
			&mDevice));
	} else {
		logger->info("Creating Direct3D9 device.");
		d3dresult = D3DLOG(mDirect3d9->CreateDevice(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			video->hwnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&presentParams,
			reinterpret_cast<IDirect3DDevice9**>(&mDevice)));
	}
	if (d3dresult != D3D_OK) {
		throw TempleException("Unable to create Direct3D9 device!");
	}
	video->d3dDevice = new Direct3DDevice8Adapter;
	video->d3dDevice->delegate = mDevice;

	video->fullscreen = false;
	video->unk2 = false;
	video->gammaSupported = false;

	// TODO: color bullshit is not yet done (tig_d3d_init_handleformat et al)

	// Get the device caps for real this time.
	ReadCaps(mDevice, 1024, 1024);

	D3DXMatrixIdentity(&video->stru_11E75788);
	D3DXMatrixIdentity(&video->matrix_identity);

	IDirect3DDevice9* device = video->d3dDevice->delegate;
	SetDefaultRenderStates(device);

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

	videoFuncs.tigMovieInitialized = true;
	videoFuncs.tig_font_related_init();
	videoFuncs.updateProjMatrices(videoFuncs.tigMatrices2);

	graphics.CreateResources();
}

void Graphics::TakeScaledScreenshot(const string& filename, int width, int height) {
	logger->debug("Creating screenshot with size {}x{} in {}", width, height, filename);
	
	if (config.disableScreenshots)
		return;

	auto device = graphics.device();

	if (config.debugMessageEnable)
		logger->debug("Device obtained {}\n", (int)device);

	// Get display mode to get screen resolution
	D3DDISPLAYMODE displayMode;
	if (D3DLOG(device->GetDisplayMode(0, &displayMode)) != D3D_OK) {
		logger->error("Unable to get the adapter display mode.");
		return;
	}
	if (config.debugMessageEnable)
		logger->debug("Display Mode obtained: {} {} {} {}\n", displayMode.Width, displayMode.Height, displayMode.RefreshRate , displayMode.Format );

	// Create an offscreen surface to contain the frontbuffer data
	CComPtr<IDirect3DSurface9> fbSurface;
	if (D3DLOG(device->CreateOffscreenPlainSurface(displayMode.Width, displayMode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &fbSurface, nullptr)) != D3D_OK) {
		logger->error("Unable to create offscreen surface for copying the frontbuffer");
		return;
	}
	if (config.debugMessageEnable)
		logger->debug("Offscreen surface created for copying the front buffer \n");

	if (D3DLOG(device->GetFrontBufferData(0, fbSurface)) != D3D_OK) {
		logger->error("Unable to copy the front buffer.");
		return;
	}
	if (config.debugMessageEnable)
		logger->debug("Front buffer copied.\n");

	auto sceneSurface = graphics.sceneSurface();

	if (config.debugMessageEnable)
		logger->debug("Scene surface obtained: {}\n", (int)sceneSurface);

	D3DSURFACE_DESC desc;
	sceneSurface->GetDesc(&desc);
	if (config.debugMessageEnable)
		logger->debug("Scene Surface Desc. : Format {} Type {} Usage {}  Pool {} MultisampleType {} Quality {}  W {} H {}\n", desc.Format, desc.Type, desc.Usage, desc.Pool, desc.MultiSampleType, desc.MultiSampleQuality, desc.Width, desc.Height);

	CComPtr<IDirect3DSurface9> stretchedScene;
	if (D3DLOG(device->CreateRenderTarget(width, height, desc.Format, desc.MultiSampleType, desc.MultiSampleQuality, false, &stretchedScene, NULL)) != D3D_OK) {
		return;
	}

	if (config.debugMessageEnable)
		logger->debug("Stretched Scene CreateRenderTarget result obtained\n");
	
	if (D3DLOG(device->StretchRect(sceneSurface, nullptr, stretchedScene, nullptr, D3DTEXF_LINEAR)) != D3D_OK) {
		logger->error("Unable to copy front buffer to target surface for screenshot");
		return;
	}
	if (config.debugMessageEnable)
		logger->debug("Stretched Scene copied from front buffer \n");

	CComPtr<ID3DXBuffer> buffer;
	if (!SUCCEEDED(D3DXSaveSurfaceToFileInMemory(&buffer, D3DXIFF_JPG, stretchedScene, nullptr, nullptr))) {
		logger->error("Unable to save screenshot surface to JPEG file.");
		return;
	}
	if (config.debugMessageEnable)
		logger->debug("JPEG file buffer created from surface\n");

	// We have to write using tio or else it goes god knows where
	auto fh = tio_fopen(filename.c_str(), "w+b");

	if (config.debugMessageEnable)
		logger->debug("File created: {}\n", fh->filename);

	if (tio_fwrite(buffer->GetBufferPointer(), 1, buffer->GetBufferSize(), fh) != buffer->GetBufferSize()) {
		logger->error("Unable to write screenshot to disk due to an IO error.");
		tio_fclose(fh);
		tio_remove(filename.c_str());
	} else {
		if (config.debugMessageEnable)
			logger->debug("File written! Closing... \n");
		tio_fclose(fh);
	}
}
