#include "stdafx.h"

#include <temple/dll.h>
#include <infrastructure/renderstates.h>
#include <infrastructure/images.h>
#include <infrastructure/exception.h>
#include <infrastructure/logging.h>

#include <platform/d3d.h>

#include "graphics.h"
#include "legacyrenderstates.h"
#include "mainwindow.h"
#include "textures.h"

#include "util/config.h"
#include <location.h>
#include <tig/tig_msg.h>
#include <tig/tig_mouse.h>
#include <tio/tio.h>

Graphics* graphics = nullptr;

static struct ExternalGraphicsFuncs : temple::AddressTable {

	/*
		Advances the clock used by the general shader to animate textures and such.
	*/
	void (__cdecl *AdvanceShaderClock)(float timeInMs);

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
	bool (__cdecl *ScreenToLoc)(int64_t x, int64_t y, locXY& locOut);

	// Is the full-screen overlay for fading out/in enabled?
	bool* gfadeEnable;
	// If it's enabled, what color does it have?
	D3DCOLOR* gfadeColor;

	void (__cdecl *ShakeScreen)(float amount, float duration);

	ExternalGraphicsFuncs() {
		rebase(AdvanceShaderClock, 0x101E0A30);

		rebase(gfadeEnable, 0x10D25118);
		rebase(gfadeColor, 0x10D24A28);

		rebase(RenderTexturedQuad, 0x101d90b0);
		rebase(sub_101EF8B0, 0x101EF8B0);

		rebase(ScreenToLoc, 0x100290C0);
		rebase(ShakeScreen, 0x10005840);
	}
} externalGraphicsFuncs;

ResourceListener::~ResourceListener() {
}

Graphics::Graphics(MainWindow& mainWindow) : mMainWindow(mainWindow) {
	memset(mScreenCorners, 0, sizeof(mScreenCorners));
	InitializeDirect3d();

	UpdateWindowSize(config.windowWidth, config.windowHeight);

	// We should replace/remove this at some point
	if (graphics) {
		throw TempleException("There should only be a single graphics instance at a time");
	}
	graphics = this;

	// Use 25% or at most 256 MB for textures
	auto textureBudget = std::min<size_t>(16 * 1024 * 1024, mVideoMemory / 4);
	mTextureManager = std::make_unique<TextureManager>(mDevice, textureBudget);
	gfx::textureManager = mTextureManager.get();
}

Graphics::~Graphics() {
	if (graphics == this) {
		graphics = nullptr;
	}
	if (gfx::textureManager == mTextureManager.get()) {
		gfx::textureManager = nullptr;
	}
}

bool Graphics::BeginFrame() {
	// ToEE supports nested begin/present calls and silently ignores them
	if (++mFrameDepth > 1) {
		return true;
	}

	auto clearColor = D3DCOLOR_ARGB(0, 0, 0, 0);

	auto result = D3DLOG(mDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 1.0f, 0));

	if (result != D3D_OK) {
		return false;
	}

	result = D3DLOG(mDevice->BeginScene());

	if (result != D3D_OK) {
		return false;
	}

	// Advance time of shader clock, used for texture animation
	auto now = Clock::now();
	auto frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - mLastFrameStart);
	externalGraphicsFuncs.AdvanceShaderClock(static_cast<float>(frameTime.count()));
	mLastFrameStart = now;

	return true;
}

bool Graphics::Present() {
	if (--mFrameDepth > 0) {
		return true;
	}

	mTextureManager->FreeUnusedTextures();

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

	if (!mResourcesCreated) {
		logger->warn("Resources are already freed.");
		return;
	}

	for (auto listener : mResourcesListeners) {
		listener->FreeResources(*this);
	}

	mResourcesCreated = false;

	mBackBuffer.Release();
	mBackBufferDepth.Release();

	mSceneSurface.Release();
	mSceneDepthSurface.Release();

	mTextureManager->FreeAllTextures();

}

void Graphics::CreateResources() {

	if (!renderStates) {
		renderStates = CreateLegacyRenderStates(*this);
	}

	renderStates->Reset();

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

	for (auto listener : mResourcesListeners) {
		listener->CreateResources(*this);
	}

	mResourcesCreated = true;

	// After a reset, the D3D cursor is hidden
	mouseFuncs.RefreshCursor();
}

D3DPRESENT_PARAMETERS Graphics::CreatePresentParams() {
	D3DPRESENT_PARAMETERS presentParams;
	memset(&presentParams, 0, sizeof(presentParams));

	presentParams.BackBufferFormat = D3DFMT_X8R8G8B8;
	// Using discard here allows us to do multisampling.
	presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParams.hDeviceWindow = mMainWindow.GetHwnd();
	presentParams.Windowed = true;
	presentParams.EnableAutoDepthStencil = true;
	presentParams.AutoDepthStencilFormat = D3DFMT_D16;
	presentParams.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	presentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	return presentParams;
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

void Graphics::AddResourceListener(ResourceListener* listener) {
	mResourcesListeners.push_back(listener);
	if (mResourcesCreated) {
		listener->CreateResources(*this);
	}
}

void Graphics::RemoveResourceListener(ResourceListener* listener) {
	mResourcesListeners.remove(listener);
	if (mResourcesCreated) {
		listener->FreeResources(*this);
	}
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
	float w = static_cast<float>(windowWidth());
	float h = static_cast<float>(windowHeight());
	float wFactor = (float)w / config.renderWidth;
	float hFactor = (float)h / config.renderHeight;
	mSceneScale = std::min(wFactor, hFactor);
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

	if (config.useDirect3d9Ex) {
		logger->info("Using Direct3D9Ex mode");
		d3dresult = D3DLOG(Direct3DCreate9Ex(D3D_SDK_VERSION, &mDirect3d9));
		if (d3dresult != D3D_OK) {
			throw TempleException("Unable to create Direct3D9 interface.");
		}
	} else {
		logger->info("Using standard Direct3D9 mode");
		mDirect3d9 = static_cast<IDirect3D9Ex*>(Direct3DCreate9(D3D_SDK_VERSION));
		if (!mDirect3d9) {
			throw TempleException("Unable to create Direct3D9 interface.");
		}
	}

	/** START OF OLD WINDOWED INIT */
	// At this point we only do a GetDisplayMode to check the resolution. We could also do this elsewhere
	D3DDISPLAYMODE displayMode;
	d3dresult = D3DLOG(mDirect3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode));
	if (d3dresult != D3D_OK) {
		throw TempleException("Unable to query display mode for primary adapter.");
	}

	// We need at least 1024x768
	if (displayMode.Width < 1024 || displayMode.Height < 768) {
		throw TempleException("You need at least a display resolution of 1024x768.");
	}

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
				mMainWindow.GetHwnd(),
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&presentParams,
			nullptr,
			&mDevice));
	} else {
		logger->info("Creating Direct3D9 device.");
		d3dresult = D3DLOG(mDirect3d9->CreateDevice(
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				mMainWindow.GetHwnd(),
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&presentParams,
			reinterpret_cast<IDirect3DDevice9**>(&mDevice)));
	}

	if (d3dresult != D3D_OK) {
		throw TempleException("Unable to create Direct3D9 device!");
	}

	// TODO: color bullshit is not yet done (tig_d3d_init_handleformat et al)

	// Get the device caps for real this time.
	ReadCaps();

	SetDefaultRenderStates();

	CreateResources();
}

static constexpr int minTexWidth = 1024;
static constexpr int minTexHeight = 1024;

void Graphics::ReadCaps() {

	mVideoMemory = mDevice->GetAvailableTextureMem();

	if (D3DLOG(mDevice->GetDeviceCaps(&mCaps)) != D3D_OK) {
		throw TempleException("Unable to retrieve Direct3D device mCaps");
	}

	/*
	Several sanity checks follow
	*/
	if (!(mCaps.SrcBlendCaps & D3DPBLENDCAPS_SRCALPHA)) {
		logger->error("source D3DPBLENDCAPS_SRCALPHA is missing");
	}
	if (!(mCaps.SrcBlendCaps & D3DPBLENDCAPS_ONE)) {
		logger->error("source D3DPBLENDCAPS_ONE is missing");
	}
	if (!(mCaps.SrcBlendCaps & D3DPBLENDCAPS_ZERO)) {
		logger->error("source D3DPBLENDCAPS_ZERO is missing");
	}
	if (!(mCaps.DestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)) {
		logger->error("destination D3DPBLENDCAPS_INVSRCALPHA is missing");
	}
	if (!(mCaps.DestBlendCaps & D3DPBLENDCAPS_ONE)) {
		logger->error("destination D3DPBLENDCAPS_ONE is missing");
	}
	if (!(mCaps.DestBlendCaps & D3DPBLENDCAPS_ZERO)) {
		logger->error("destination D3DPBLENDCAPS_ZERO is missing");
	}

	if (mCaps.MaxSimultaneousTextures < 4) {
		logger->error("less than 4 active textures possible: {}", mCaps.MaxSimultaneousTextures);
	}
	if (mCaps.MaxTextureBlendStages < 4) {
		logger->error("less than 4 texture blend stages possible: {}", mCaps.MaxTextureBlendStages);
	}

	if (!(mCaps.TextureOpCaps & D3DTOP_DISABLE)) {
		logger->error("texture op D3DTOP_DISABLE is missing");
	}
	if (!(mCaps.TextureOpCaps & D3DTOP_SELECTARG1)) {
		logger->error("texture op D3DTOP_SELECTARG1 is missing");
	}
	if (!(mCaps.TextureOpCaps & D3DTOP_SELECTARG2)) {
		logger->error("texture op D3DTOP_SELECTARG2 is missing");
	}
	if (!(mCaps.TextureOpCaps & D3DTOP_BLENDTEXTUREALPHA)) {
		logger->error("texture op D3DTOP_BLENDTEXTUREALPHA is missing");
	}
	if (!(mCaps.TextureOpCaps & D3DTOP_BLENDCURRENTALPHA)) {
		logger->error("texture op D3DTOP_BLENDCURRENTALPHA is missing");
	}
	if (!(mCaps.TextureOpCaps & D3DTOP_MODULATE)) {
		logger->error("texture op D3DTOP_MODULATE is missing");
	}
	if (!(mCaps.TextureOpCaps & D3DTOP_ADD)) {
		logger->error("texture op D3DTOP_ADD is missing");
	}
	if (!(mCaps.TextureOpCaps & D3DTOP_MODULATEALPHA_ADDCOLOR)) {
		logger->error("texture op D3DTOP_MODULATEALPHA_ADDCOLOR is missing");
	}
	if (mCaps.MaxTextureWidth < minTexWidth || mCaps.MaxTextureHeight < minTexHeight) {
		auto msg = fmt::format("minimum texture resolution of {}x{} is not supported. Supported: {}x{}",
		                       minTexWidth, minTexHeight, mCaps.MaxTextureWidth, mCaps.MaxTextureHeight);
		throw TempleException(msg);
	}

	if ((mCaps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0) {
		logger->error("Textures must be power of two");
	}
	if ((mCaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) != 0) {
		logger->error("Textures must be square");
	}

}

void Graphics::TakeScaledScreenshot(const std::string& filename, int width, int height) {
	logger->debug("Creating screenshot with size {}x{} in {}", width, height, filename);

	auto device = mDevice;

	auto sceneSurface = mSceneSurface;

	D3DSURFACE_DESC desc;
	sceneSurface->GetDesc(&desc);

	// Support taking unscaled screenshots
	auto stretch = true;
	if (width == 0 || height == 0) {
		width = desc.Width;
		height = desc.Height;
		stretch = false;
	}

	// Create system memory surface to copy the screenshot to
	CComPtr<IDirect3DSurface9> sysMemSurface;
	if (D3DLOG(device->CreateOffscreenPlainSurface(width, height, desc.Format, D3DPOOL_SYSTEMMEM, &sysMemSurface, nullptr)) != D3D_OK) {
		logger->error("Unable to create offscreen surface for copying the screenshot");
		return;
	}

	if (stretch) {
		CComPtr<IDirect3DSurface9> stretchedScene;
		if (D3DLOG(device->CreateRenderTarget(width, height, desc.Format, desc.MultiSampleType, desc.MultiSampleQuality, false, &stretchedScene, NULL)) != D3D_OK) {
			return;
		}

		if (D3DLOG(device->StretchRect(sceneSurface, nullptr, stretchedScene, nullptr, D3DTEXF_LINEAR)) != D3D_OK) {
			logger->error("Unable to copy front buffer to target surface for screenshot");
			return;
		}

		if (D3DLOG(device->GetRenderTargetData(stretchedScene, sysMemSurface))) {
			logger->error("Unable to copy stretched render target to system memory.");
			return;
		}
	} else {
		if (D3DLOG(device->GetRenderTargetData(sceneSurface, sysMemSurface))) {
			logger->error("Unable to copy scene render target to system memory.");
			return;
		}
	}

	/*
		Get access to the pixel data for the surface and encode it to a JPEG.
	*/
	D3DLOCKED_RECT locked;
	if (D3DLOG(sysMemSurface->LockRect(&locked, nullptr, 0))) {
		logger->error("Unable to lock screenshot surface.");
		return;
	}

	// Quality is between 1 and 100
	auto quality = std::min(100, std::max(1, config.screenshotQuality));

	auto jpegData(gfx::EncodeJpeg(reinterpret_cast<uint8_t*>(locked.pBits),
	                              gfx::JpegPixelFormat::BGRX,
	                              width,
	                              height,
	                              quality,
	                              locked.Pitch));

	if (D3DLOG(sysMemSurface->UnlockRect())) {
		logger->error("Unable to unlock screenshot surface.");
		return;
	}

	// We have to write using tio or else it goes god knows where
	auto fh = tio_fopen(filename.c_str(), "w+b");
	if (tio_fwrite(jpegData.data(), 1, jpegData.size(), fh) != jpegData.size()) {
		logger->error("Unable to write screenshot to disk due to an IO error.");
		tio_fclose(fh);
		tio_remove(filename.c_str());
	} else {
		tio_fclose(fh);
	}
}

void Graphics::SetDefaultRenderStates() {
	/*
	SET DEFAULT RENDER STATES
	*/
	mDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	mDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	mDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	mDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

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
	mDevice->SetLight(0, &light);
	mDevice->SetRenderState(D3DRS_AMBIENT, 0);
	mDevice->SetRenderState(D3DRS_SPECULARENABLE, 0);
	mDevice->SetRenderState(D3DRS_LOCALVIEWER, 0);

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
	mDevice->SetMaterial(&material);

	D3DLOG(mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	D3DLOG(mDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	D3DLOG(mDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

	D3DLOG(mDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1));
	D3DLOG(mDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTOP_SELECTARG1));
	D3DLOG(mDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTOP_DISABLE));

	for (DWORD i = 0; i < 4; ++i) {
		D3DLOG(mDevice->SetSamplerState(i, D3DSAMP_MINFILTER, 1));
		D3DLOG(mDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, 2));
		D3DLOG(mDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, 1));
		D3DLOG(mDevice->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, 0));
		D3DLOG(mDevice->SetSamplerState(i, D3DSAMP_MAXMIPLEVEL, 01));
		D3DLOG(mDevice->SetSamplerState(i, D3DSAMP_MINFILTER, 1));
		D3DLOG(mDevice->SetSamplerState(i, D3DSAMP_MINFILTER, 1));
		D3DLOG(mDevice->SetSamplerState(i, D3DSAMP_ADDRESSU, 3));
		D3DLOG(mDevice->SetSamplerState(i, D3DSAMP_ADDRESSV, 3));
		D3DLOG(mDevice->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, 0));
		D3DLOG(mDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, 0));
	}

	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	D3DLOG(mDevice->SetTransform(D3DTS_TEXTURE0, &identity));

	mDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	mDevice->SetRenderState(D3DRS_ALPHAREF, 1);
	mDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
}
