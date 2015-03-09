#include "stdafx.h"

#include "graphics.h"
#include "temple_functions.h"
#include "addresses.h"
#include "idxtables.h"
#include "config.h"

// #include "d3d8/d3d8.h"
#include "d3d8to9/d3d8to9.h"
#include <d3d9.h>

#include "tig_font.h"

IdxTableWrapper<TigShaderRegistryEntry> shaderRegistry(0x10D47544);

#pragma pack(push, 1)
struct TigRenderStates {
	D3DXMATRIX proj_matrix;
	D3DXMATRIX view_matrix;
	int zenable;
	int fillmode;
	int zwriteenable;
	int alphatestenable;
	int srcblend;
	int destblend;
	int cullmode;
	int alphablendenable;
	int lighting;
	int colorvertex;
	int colorwriteenable;
	int zfunc;
	int specularenable;
	int zbias;
	int texture[4];
	int tex_colorop[4];
	int tex_colorarg1[4];
	int tex_colorarg2[4];
	int tex_alphaop[4];
	int tex_alphaarg1[4];
	int tex_alphaarg2[4];
	int tex_coordindex[4];
	int tex_mipfilter[4];
	int tex_magfilter[4];
	int tex_minfilter[4];
	int tex_addressu[4];
	int tex_addressv[4];
	int tex_transformflags[4];
	int vertexattribs;
	int vertexbuffers[4];
	int vertexstrides[4];
	int indexbuffer;
	int basevertexindex;
};
#pragma pack(pop)

static_assert(sizeof(TigRenderStates) == 0x1C4, "TigRenderStates has the wrong size.");

/*
	Container for ToEE functions related to video
*/
struct VideoFuncs : AddressTable {
	bool (__fastcall *TigDirect3dInit)(TempleStartSettings* settings) = nullptr;
	bool (__cdecl *PresentFrame)() = nullptr;
	void (__cdecl *PlayLegalMovies)() = nullptr;
	void (__cdecl *PlayMovie)(char* filename, int, int, int);
	void (__cdecl *SetVideoMode)(int adapter, int nWidth, int nHeight, int bpp, int refresh, int flags);
	void (__cdecl *CleanUpBuffers)();
	void (__cdecl *ReadInitialState)();

	GlobalBool<0x10D25144> buffersFreed;
	GlobalPrimitive<uint32_t, 0x10D25148> currentFlags;
	/*
		Several subsystems use this to detect windowed mode.
		Although the entire startup config is copied to this location by 
		some portions of the game. We only write 0x20 (windowed) to this
		statically.
	*/
	GlobalPrimitive<uint32_t, 0x11E75840> startupFlags;

	GlobalPrimitive<Direct3DVertexBuffer8Adapter*, 0x10D25120> globalFadeVBuffer;
	GlobalPrimitive<Direct3DVertexBuffer8Adapter*, 0x10D25124> sharedVBuffer1;
	GlobalPrimitive<Direct3DVertexBuffer8Adapter*, 0x10D25128> sharedVBuffer2;
	GlobalPrimitive<Direct3DVertexBuffer8Adapter*, 0x10D2512C> sharedVBuffer3;
	GlobalPrimitive<Direct3DVertexBuffer8Adapter*, 0x10D25130> sharedVBuffer4;

	GlobalPrimitive<TigRenderStates, 0x10EF2F10> renderStates;
	GlobalPrimitive<TigRenderStates, 0x10EF30D8> activeRenderStates;
	GlobalPrimitive<float, 0x10D24D7C> fadeScreenRect;

	void rebase(Rebaser rebase) override {
		rebase(TigDirect3dInit, 0x101DAFB0);
		rebase(PresentFrame, 0x101DCB80);
		rebase(PlayMovie, 0x10034100);
		rebase(PlayLegalMovies, 0x10003AC0);
		rebase(SetVideoMode, 0x101DC870);
		rebase(CleanUpBuffers, 0x101D8640);
		rebase(ReadInitialState, 0x101F06F0);
	}
} videoFuncs;

/*
 * Size being cleared is 4796 byte in length
 * Start @ 0x1E74580
 */
struct VideoData {
	HINSTANCE hinstance;
	HWND hwnd;
	Direct3D8Adapter* d3d;
	Direct3DDevice8Adapter* d3dDevice;
	d3d8::D3DCAPS8 d3dCaps;
	char padding[124];
	DWORD unk2;
	D3DGAMMARAMP gammaRamp1;
	D3DGAMMARAMP gammaRamp2;
	float gammaRelated1; // Default = 1.0
	int gammaSupported; // Seems to be a flag 1/0
	float gammaRelated2;
	RECT screenSizeRect; // Seems to never be read from anywhere
	char junk[0x400];
	// Junk starts @ 11E75300
	// Resumes @ 11E75700
	DWORD current_bpp;
	bool fullscreen; // seems to never be read
	DWORD unk4; // apparently never read
	DWORD vsync; // apparently never read. is always 1 since startup flag 0x4 is always set
	DWORD unusedCap;
	DWORD makesSthLarger; // shadow maps maybe?
	DWORD capPowerOfTwoTextures; // indicates that power of two textures are necessary
	DWORD capSquareTextures; // indicates that textures must be square
	DWORD neverReadFlag1;
	DWORD neverReadFlag2;
	DWORD maxActiveTextures;
	DWORD dword_11E7572C; // This always seems to be 0, but seems to be read in buffer_free apparently
	DWORD dword_11E75730[8]; // never read or written to apparently
	bool enableMipMaps;
	DWORD maxActiveLights;
	DWORD framenumber;

	DWORD width;
	DWORD height;
	float halfWidth;
	float halfHeight;
	DWORD current_width; // What the difference between this and width is? no idea...
	DWORD current_height;
	DWORD adapter;
	DWORD mode; // seems to be an index to the current fullscreen mode
	D3DFORMAT adapterformat;
	DWORD current_refresh;
	Direct3DVertexBuffer8Adapter* blitVBuffer;
	D3DXMATRIX stru_11E75788;
	D3DXMATRIX matrix_identity;
	D3DVECTOR stru_11E75808;
	D3DVECTOR stru_11E75814;
	D3DVECTOR stru_11E75820;
	int dword_11E7582C; // this is for alignment only according to IdaPro
	int dword_11E75830;
	int dword_11E75834;
	int dword_11E75838;
};

static_assert(sizeof(VideoData) == 4796, "Video Data struct has the wrong size.");

GlobalBool<0x10D250EC> drawFps;
GlobalStruct<tig_text_style> drawFpsTextStyle(0x10D24DB0);

VideoData* video;
static AddressInitializer videoInitializer([](Rebaser rebase) {
	rebase(video, 0x11E74580); // 0x11E74580 to 0x11E7583C
});

// Our precompiled header swallows this somehow...
static const DWORD D3D_SDK_VERSION = 32;

static bool TigInitDirect3D(TempleStartSettings* settings) {

	HRESULT d3dresult;

	if (settings->bpp < 32) {
		LOG(error) << "BPP settings must be 32-bit";
		return false;
	}

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
	video->enableMipMaps = (settings->flags & SF_MIPMAPPING) != 0;
	D3DXMatrixIdentity(&video->stru_11E75788);
	D3DXMatrixIdentity(&video->matrix_identity);

	IDirect3D9Ex* d3d9 = nullptr;
	if (config.useDirect3d9Ex) {
		d3dresult = Direct3DCreate9Ex(D3D_SDK_VERSION, &d3d9);
		if (d3dresult != D3D_OK) {
			LOG(error) << "Unable to create Direct3D9Ex device.";
			handleD3dError("TigInitDirect3D", d3dresult);
			return false;
		}
	} else {
		d3d9 = static_cast<IDirect3D9Ex*>(Direct3DCreate9(D3D_SDK_VERSION));
		if (!d3d9) {
			LOG(error) << "Unable to create Direct3D9 device.";
			return false;
		}
	}
	video->d3d = new Direct3D8Adapter;
	video->d3d->delegate = d3d9;

	/** START OF OLD WINDOWED INIT */

	// At this point we only do a GetDisplayMode to check the resolution. We could also do this elsewhere
	D3DDISPLAYMODE displayMode;
	d3dresult = d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);
	if (d3dresult != D3D_OK) {
		LOG(error) << "Unable to query display mode for primary adapter.";
		handleD3dError("GetAdapterDisplayMode", d3dresult);
		return false;
	}

	// We need at least 1024x768
	if (displayMode.Width < 1024 || displayMode.Height < 768) {
		LOG(error) << "You need at least a display resolution of 1024x768.";
		return false;
	}

	// This is only really used by alloc_texture_mem and the init func
	video->adapterformat = D3DFMT_A8R8G8B8;
	video->current_bpp = 32;
	settings->bpp = 32;

	/*
		Check for linear->srgb conversion support (also known as gamma correction)
	*/
	D3DCAPS9 caps;
	if ((d3dresult = d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps)) != D3D_OK) {
		LOG(error) << "Unable to retrieve device caps.";
		handleD3dError("GetDeviceCaps", d3dresult);
		return false;
	}

	enableLinearPresent = false;
	if ((caps.Caps3 & D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION) != 0) {
		LOG(info) << "Automatic gamma corection is supported by driver.";
		enableLinearPresent = true;
	}
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

	D3DPRESENT_PARAMETERS presentParams;
	memset(&presentParams, 0, sizeof(presentParams));

	presentParams.BackBufferFormat = D3DFMT_X8R8G8B8;
	// Using discard here allows us to do multisampling.
	presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParams.hDeviceWindow = video->hwnd;
	presentParams.Windowed = true;
	presentParams.EnableAutoDepthStencil = true;
	presentParams.AutoDepthStencilFormat = D3DFMT_D16;
	presentParams.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER | D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	presentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	// presentParams.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
	// presentParams.MultiSampleQuality = 0;

	// Nvidia drivers seriously barf on D3d9ex if we use software vertex processing here, as ToEE specifies.
	// I think we are safe with hardware vertex processing, since HW T&L has been standard for more than 10 years.
	IDirect3DDevice9Ex* d3d9Device;
	if (config.useDirect3d9Ex) {
		d3dresult = d3d9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, video->hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentParams, nullptr, &d3d9Device);
	} else {
		d3dresult = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, video->hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentParams, reinterpret_cast<IDirect3DDevice9**>(&d3d9Device));
	}
	if (d3dresult != D3D_OK) {
		LOG(error) << "Unable to create Direct3d9 device.";
		handleD3dError("CreateDevice", d3dresult);
		return false;
	}
	video->d3dDevice = new Direct3DDevice8Adapter;
	video->d3dDevice->delegate = d3d9Device;

	video->fullscreen = false;
	video->unk2 = false;
	video->gammaSupported = false;

	// TODO: color bullshit is not yet done (tig_d3d_init_handleformat et al)

	/** END OF OLD WINDOWED INIT */
	videoFuncs.currentFlags = settings->flags;

	/*
		Create several shared buffers. Most of these don't seem to be used much or ever.
	*/
	IDirect3DVertexBuffer9* vbuffer;
	if ((d3dresult = d3d9Device->CreateVertexBuffer(
		112, // Space for 4 vertices
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW, // 28 bytes per vertex
		D3DPOOL_SYSTEMMEM,
		&vbuffer,
		nullptr)) != D3D_OK) {
		handleD3dError("CreateVertexBuffer", d3dresult);
		return false;
	}
	video->blitVBuffer = new Direct3DVertexBuffer8Adapter(vbuffer);

	if ((d3dresult = d3d9Device->CreateVertexBuffer(
		140, // Space for 5 vertices
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW, // 28 bytes per vertex
		D3DPOOL_SYSTEMMEM,
		&vbuffer,
		nullptr)) != D3D_OK) {
		handleD3dError("CreateVertexBuffer", d3dresult);
		return false;
	}
	videoFuncs.globalFadeVBuffer = new Direct3DVertexBuffer8Adapter(vbuffer);

	if ((d3dresult = d3d9Device->CreateVertexBuffer(
		72, // 2 vertices (odd)
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_XYZ, // 36 byte per vertex
		D3DPOOL_SYSTEMMEM,
		&vbuffer,
		nullptr)) != D3D_OK) {
		handleD3dError("CreateVertexBuffer", d3dresult);
		return false;
	}
	videoFuncs.sharedVBuffer1 = new Direct3DVertexBuffer8Adapter(vbuffer);

	if ((d3dresult = d3d9Device->CreateVertexBuffer(
		56, // 2 vertices
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW, // 28 bytes per vertex
		D3DPOOL_SYSTEMMEM,
		&vbuffer,
		nullptr)) != D3D_OK) {
		handleD3dError("CreateVertexBuffer", d3dresult);
		return false;
	}
	videoFuncs.sharedVBuffer2 = new Direct3DVertexBuffer8Adapter(vbuffer);

	if ((d3dresult = d3d9Device->CreateVertexBuffer(
		7168, // 256 vertices
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW, // 28 bytes per vertex
		D3DPOOL_SYSTEMMEM,
		&vbuffer,
		nullptr)) != D3D_OK) {
		handleD3dError("CreateVertexBuffer", d3dresult);
		return false;
	}
	videoFuncs.sharedVBuffer3 = new Direct3DVertexBuffer8Adapter(vbuffer);

	if ((d3dresult = d3d9Device->CreateVertexBuffer(
		4644, // 129 vertices
		D3DUSAGE_DYNAMIC,
		D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_XYZ, // 36 byte per vertex
		D3DPOOL_SYSTEMMEM,
		&vbuffer,
		nullptr)) != D3D_OK) {
		handleD3dError("CreateVertexBuffer", d3dresult);
		return false;
	}
	videoFuncs.sharedVBuffer4 = new Direct3DVertexBuffer8Adapter(vbuffer);

	// Get the device caps for real this time.
	if ((d3dresult = d3d9Device->GetDeviceCaps(&caps)) != D3D_OK) {
		LOG(error) << "Unable to retrieve device caps.";
		handleD3dError("GetDeviceCaps", d3dresult);
		return false;
	}
	video->maxActiveLights = min(8, caps.MaxActiveLights);

	/*
		Several sanity checks follow
	*/
	if (!(caps.SrcBlendCaps & D3DPBLENDCAPS_SRCALPHA)) {
		LOG(info) << "source D3DPBLENDCAPS_SRCALPHA is missing";
	}
	if (!(caps.SrcBlendCaps & D3DPBLENDCAPS_ONE)) {
		LOG(info) << "source D3DPBLENDCAPS_ONE is missing";
	}
	if (!(caps.SrcBlendCaps & D3DPBLENDCAPS_ZERO)) {
		LOG(info) << "source D3DPBLENDCAPS_ZERO is missing";
	}
	if (!(caps.DestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)) {
		LOG(info) << "destination D3DPBLENDCAPS_INVSRCALPHA is missing";
	}
	if (!(caps.DestBlendCaps & D3DPBLENDCAPS_ONE)) {
		LOG(info) << "destination D3DPBLENDCAPS_ONE is missing";
	}
	if (!(caps.DestBlendCaps & D3DPBLENDCAPS_ZERO)) {
		LOG(info) << "destination D3DPBLENDCAPS_ZERO is missing";
	}

	if (caps.MaxSimultaneousTextures < 4) {
		LOG(info) << "less than 4 active textures possible: " << caps.MaxSimultaneousTextures;
	}
	if (caps.MaxTextureBlendStages < 4) {
		LOG(info) << "less than 4 texture blend stages possible: " << caps.MaxTextureBlendStages;
	}
	video->maxActiveTextures = 4; // We do not accept less than 4

	if (!(caps.TextureOpCaps & D3DTOP_DISABLE)) {
		LOG(info) << "texture op D3DTOP_DISABLE is missing";
	}
	if (!(caps.TextureOpCaps & D3DTOP_SELECTARG1)) {
		LOG(info) << "texture op D3DTOP_SELECTARG1 is missing";
	}
	if (!(caps.TextureOpCaps & D3DTOP_SELECTARG2)) {
		LOG(info) << "texture op D3DTOP_SELECTARG2 is missing";
	}
	if (!(caps.TextureOpCaps & D3DTOP_BLENDTEXTUREALPHA)) {
		LOG(info) << "texture op D3DTOP_BLENDTEXTUREALPHA is missing";
	}
	if (!(caps.TextureOpCaps & D3DTOP_BLENDCURRENTALPHA)) {
		LOG(info) << "texture op D3DTOP_BLENDCURRENTALPHA is missing";
	}
	if (!(caps.TextureOpCaps & D3DTOP_MODULATE)) {
		LOG(info) << "texture op D3DTOP_MODULATE is missing";
	}
	if (!(caps.TextureOpCaps & D3DTOP_ADD)) {
		LOG(info) << "texture op D3DTOP_ADD is missing";
	}
	if (!(caps.TextureOpCaps & D3DTOP_MODULATEALPHA_ADDCOLOR)) {
		LOG(info) << "texture op D3DTOP_MODULATEALPHA_ADDCOLOR is missing";
	}
	if (caps.MaxTextureWidth < settings->minTexWidth || caps.MaxTextureHeight < settings->minTexHeight) {
		LOG(info) << "minimum texture resolution of " << settings->minTexWidth << "x" << settings->minTexHeight
				<< " is not supported. Supported: " << caps.MaxTextureWidth << "x" << caps.MaxTextureHeight;
		return false;
	}

	if ((caps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0) {
		LOG(info) << "Textures must be power of two";
		video->capPowerOfTwoTextures = true;
	}
	if ((caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) != 0) {
		LOG(info) << "Textures must be square";
		video->capSquareTextures = true;
	}

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

	d3d9Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	d3d9Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	d3d9Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	d3d9Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	d3d9Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTOP_SELECTARG1);
	d3d9Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTOP_DISABLE);

	for (DWORD i = 0; i < video->maxActiveTextures; ++i) {
		d3d9Device->SetSamplerState(i, D3DSAMP_MINFILTER, 1);
		d3d9Device->SetSamplerState(i, D3DSAMP_MAGFILTER, 2);
		d3d9Device->SetSamplerState(i, D3DSAMP_MIPFILTER, 1);
		d3d9Device->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, 0);
		d3d9Device->SetSamplerState(i, D3DSAMP_MAXMIPLEVEL, 01);
		d3d9Device->SetSamplerState(i, D3DSAMP_MINFILTER, 1);
		d3d9Device->SetSamplerState(i, D3DSAMP_MINFILTER, 1);
		d3d9Device->SetSamplerState(i, D3DSAMP_ADDRESSU, 3);
		d3d9Device->SetSamplerState(i, D3DSAMP_ADDRESSV, 3);
		d3d9Device->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
		d3d9Device->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, 0);
	}

	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	d3d9Device->SetTransform(D3DTS_TEXTURE0, &identity);

	d3d9Device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	d3d9Device->SetRenderState(D3DRS_ALPHAREF, 1);
	d3d9Device->SetRenderState(D3DRS_ALPHAFUNC, 7);

	// Seems to be 4 VECTOR3's for the screen corners
	auto fadeScreenRect = videoFuncs.fadeScreenRect.ptr();
	fadeScreenRect[0] = 0;
	fadeScreenRect[1] = 0;
	fadeScreenRect[2] = 0;
	fadeScreenRect[3] = (float) video->current_width;
	fadeScreenRect[4] = 0;
	fadeScreenRect[5] = 0;
	fadeScreenRect[6] = (float)video->current_width;
	fadeScreenRect[7] = (float)video->current_height;
	fadeScreenRect[8] = 0;
	fadeScreenRect[9] = 0;
	fadeScreenRect[10] = (float)video->current_height;
	fadeScreenRect[11] = 0;

	video->unusedCap = 1; // Seems to be ref'd from light_init

	videoFuncs.ReadInitialState();
	memcpy(videoFuncs.renderStates.ptr(), videoFuncs.activeRenderStates.ptr(), sizeof(TigRenderStates));

	return true;
}

static bool TigCreateWindow(TempleStartSettings* settings) {
	bool windowed = config.windowed;
	bool unknownFlag = (settings->flags & 0x100) != 0;

	video->hinstance = settings->hinstance;

	WNDCLASSA wndClass;
	ZeroMemory(&wndClass, sizeof(WNDCLASSA));
	wndClass.style = CS_DBLCLKS;
	wndClass.lpfnWndProc = (WNDPROC) temple_address<0x101DE9A0>(); // tig_wndproc
	wndClass.hInstance = video->hinstance;
	wndClass.hIcon = LoadIconA(video->hinstance, "TIGIcon");
	wndClass.hCursor = LoadCursorA(0, MAKEINTRESOURCEA(IDC_ARROW));
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszClassName = "TIGClass";

	if (!RegisterClassA(&wndClass)) {
		return false;
	}

	auto screenWidth = GetSystemMetrics(SM_CXSCREEN);
	auto screenHeight = GetSystemMetrics(SM_CYSCREEN);

	RECT windowRect;
	HMENU menu;
	DWORD dwStyle;
	DWORD dwExStyle;

	if (!windowed) {
		settings->width = screenWidth;
		settings->height = screenHeight;

		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = screenWidth;
		windowRect.bottom = screenHeight;
		menu = 0;
		dwStyle = WS_POPUP;
		dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
		memcpy(&video->screenSizeRect, &windowRect, sizeof(RECT));
	} else {
		// Apparently this flag controls whether x,y are preset from the outside
		if (unknownFlag) {
			windowRect.left = settings->x;
			windowRect.top = settings->y;
			windowRect.right = settings->x + settings->width;
			windowRect.bottom = settings->y + settings->height;
		} else {
			windowRect.left = (screenWidth - settings->width) / 2;
			windowRect.top = (screenHeight - settings->height) / 2;
			windowRect.right = windowRect.left + settings->width;
			windowRect.bottom = windowRect.top + settings->height;
		}
		menu = LoadMenuA(settings->hinstance, "TIGMenu");
		dwStyle = WS_OVERLAPPEDWINDOW; //  WS_CAPTION | WS_CLIPCHILDREN | WS_SYSMENU | WS_GROUP;
		dwExStyle = 0;

		// Apparently 0x80 means window rect isn't adjusted. is this a half-implemented borderless fullscreen?
		if (!(settings->flags & 0x80)) {
			AdjustWindowRectEx(&windowRect, dwStyle, menu != 0, 0);
			// TODO: Adjust back
			//            v13 = Rect.right - Rect.left - v1->width;
			//            LODWORD(v13) = v13 - HIDWORD(v13);
			//            v14 = Rect.bottom - Rect.top - v1->height;
			//            v3 = ((signed int)v13 >> 1) + Rect.left;
			//            v4 = ((signed int)v13 >> 1) + Rect.right;
			//            v15 = v14 / 2 + Rect.top;
			//            v5 = v14 / 2 + Rect.bottom;
			//            Rect.left += (signed int)v13 >> 1;
			//            Rect.right += (signed int)v13 >> 1;
			//            Rect.top += v14 / 2;
			//            Rect.bottom += v14 / 2;
		}
	}

	dwStyle |= WS_VISIBLE;
	video->width = settings->width;
	video->height = settings->height;

	temple_set<0x10D24E0C>(0);
	temple_set<0x10D24E10>(0);
	temple_set<0x10D24E14>(settings->width);

	const char* windowTitle;
	// Apparently is a flag that indicates a custom window title
	if (settings->flags & 0x40) {
		windowTitle = settings->windowTitle;
	} else {
		windowTitle = "Temple of Elemental Evil - Cirlce of Eight";
	}

	DWORD windowWidth = windowRect.right - windowRect.left;
	DWORD windowHeight = windowRect.bottom - windowRect.top;
	video->hwnd = CreateWindowExA(
		dwExStyle,
		"TIGClass",
		windowTitle,
		dwStyle,
		windowRect.left,
		windowRect.top,
		windowWidth,
		windowHeight,
		0,
		menu,
		settings->hinstance,
		0);

	if (video->hwnd) {
		RECT clientRect;
		GetClientRect(video->hwnd, &clientRect);
		video->current_width = clientRect.right - clientRect.left;
		video->current_height = clientRect.bottom - clientRect.top;

		// Scratchbuffer size sometimes doesn't seem to be set by ToEE itself
		temple_set<0x10307284>(video->current_width);
		temple_set<0x10307288>(video->current_height);

		return true;
	}

	return false;
}

void __cdecl HookedSetVideoMode(int adapter, int nWidth, int nHeight, int bpp, int refresh, int flags) {
	LOG(info) << "set_video_mode adapter=" << adapter << " w=" << nWidth << " h="
			<< nHeight << " bpp=" << bpp << " refresh=" << refresh << " flags=" << flags;

	LOG(info) << "Buffers Freed: " << videoFuncs.buffersFreed;

	// We probabl need to create a shadow for this...
	bool changed =
		video->adapter != adapter
		|| video->current_bpp != bpp
		|| video->current_width != nWidth
		|| video->current_height != nHeight
		|| videoFuncs.currentFlags != flags;

	if (!changed) {
		return;
	}

	video->current_width = nWidth;
	video->current_height = nHeight;
	video->current_bpp = 32; // Never anything else
	// Adapter changes will not happen...
	video->adapter = adapter;
	videoFuncs.currentFlags = flags;

	if (!videoFuncs.buffersFreed) {
		videoFuncs.CleanUpBuffers();
	}

	video->halfWidth = nWidth * 0.5f;
	video->halfHeight = nHeight * 0.5f;

	/*
		Basically this is the inverse of CleanBuffers. Sadly it's not an easily callable function.
	*/

	/*config.bpp = bpp;
	config.width = nWidth;
	config.height = nHeight;
	config.framelimit = refresh;
	backbuffer_width = nWidth;
	backbuffer_height = nHeight;

	if (tig_d3d_init(&config, flags))
	{
		result = return_EAX_0();
		if (result)
			return result;
		unknown_libname_14();
		tig_movie_set_initialized();
		sub_101E85C0();
		sub_101D8910(&dword_10D24E00);
		buffers_freed = 0;
		if (set_video_mode_callback)
			set_video_mode_callback();
		v8 = (long double)current_width;
		dword_10D24D7C = 0;
		dword_10D24D80 = 0;
		dword_10D24D84 = 0;
		flt_10D24D88 = v8;
		dword_10D24D8C = 0;
		flt_10D24D94 = v8;
		dword_10D24D90 = 0;
		v9 = (long double)current_height;
		dword_10D24D9C = 0;
		dword_10D24DA0 = 0;
		dword_10D24DA8 = 0;
		flt_10D24D98 = v9;
		flt_10D24DA4 = v9;
	}*/

	return;
}

int __cdecl VideoStartup(TempleStartSettings* settings) {
	memset(video, 0, 4796);

	bool windowed = config.windowed;

	if (windowed) {
		if (!settings->wndproc) {
			return 12;
		}
		temple_set<0x10D25C38>(settings->wndproc);
	}

	video->adapter = 0;

	// create window call
	if (!TigCreateWindow(settings)) {
		return 17;
	}

	if (!TigInitDirect3D(settings)) {
		video->hwnd = 0;
		return 17;
	}

	video->width = settings->width;
	video->height = settings->height;
	video->halfWidth = video->width * 0.5f;
	video->halfHeight = video->height * 0.5f;

	if (settings->flags & SF_FPS) {
		drawFps = true;
		temple_set<0x10D24DB0>(0);
		temple_set<0x10D24DB4>(5);
		temple_set<0x10D24DB8>(2);
		temple_set<0x10D24DBC>(0);

		temple_set<0x10D24DD8>(8);
		temple_set<0x10D24DDC>(-1);
		temple_set<0x10D24DE4>(temple_address<0x103008F4>());
		temple_set<0x10D24DE8>(temple_address<0x103008F4>());
		temple_set<0x10D24DEC>(temple_address<0x10300904>());
		temple_set<0x10D24DF0>(temple_address<0x103008F4>());
	} else {
		drawFps = false;
	}

	// Seems always enabled in default config and never read
	temple_set<0x11E7570C, int>((settings->flags & 4) != 0);

	video->current_bpp = settings->bpp;

	ShowCursor(windowed); // Show cursor in windowed mode

	temple_set<0x10D250E0, int>(0);
	temple_set<0x10D250E4, int>(1);
	temple_set<0x10300914, int>(-1);
	temple_set<0x10D2511C, int>(0);

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

	temple_set<0x10D25134>(settings->callback1);
	temple_set<0x10D25138>(settings->callback2);

	memcpy(temple_address<0x11E75840>(), settings, 0x4C);

	return 0;
}

/*
	The original function could not handle PCs with >4GB ram
*/
void get_system_memory(int* totalMem, int* availableMem) {
	MEMORYSTATUS status;

	GlobalMemoryStatus(&status);

	// Max 1GB because of process space limitations
	*totalMem = min(1024 * 1024 * 1024, status.dwTotalPhys);
	*availableMem = min(1024 * 1024 * 1024, status.dwAvailPhys);
}

bool __cdecl HookedPresentFrame() {
	static bool dumpedPacketStructs = true;

	if (!dumpedPacketStructs) {
		for (const auto& node : shaderRegistry) {
			LOG(info) << "Registered shader: " << node.data->name;
		}

		auto defaultShader = shaderRegistry.get(0);
		if (defaultShader) {
			LOG(info) << "Found default shader!";
		} else {
			LOG(info) << "Did not find default shader!";
		}

		LOG(info) << "=================================================================================================";
		auto listEntry = *idxTablesList.ptr();
		while (listEntry) {
			LOG(info) << "Index Table allocated @ " << listEntry->sourceFile << ":" << listEntry->lineNumber;

			auto table = listEntry->table;
			LOG(info) << "   Buckets: " << table->bucketCount << " Item Count: " << table->itemCount << " Item Size: "
					<< table->itemSize << " Address: " << format("%x") % reinterpret_cast<uint32_t>(table);

			listEntry = listEntry->next;
		}
	}

	tig_font_extents extents;
	extents.x = 100;
	extents.y = 100;
	tigFont.Draw("Hello World!\nHow are you today?", &extents, drawFpsTextStyle);

	return videoFuncs.PresentFrame();
}

void HookedPlayLegalMovies() {
	if (!config.skipLegal) {
		videoFuncs.PlayLegalMovies();
	}
}

void HookdPlayMovie(char* filename, int a1, int a2, int a3) {
	// We skip the intro cinematic exactly once. So it can still be played
	// via the cinematics menu
	if (config.skipIntro && !strcmp(filename, "movies\\introcinematic.bik")) {
		static auto skippedIntro = false;
		if (!skippedIntro) {
			LOG(info) << "Skipping intro cinematic.";
			skippedIntro = true;
			return;
		}
	}

	videoFuncs.PlayMovie(filename, a1, a2, a3);
}

void hook_graphics() {
	// We only differ between borderless and normal window mode.
	videoFuncs.startupFlags = SF_WINDOW;

	MH_CreateHook(temple_address<0x101E0750>(), get_system_memory, NULL);

	// Hook into present frame to do after-frame stuff
	MH_CreateHook(videoFuncs.PresentFrame, HookedPresentFrame, reinterpret_cast<LPVOID*>(&videoFuncs.PresentFrame));
	MH_CreateHook(videoFuncs.PlayLegalMovies, HookedPlayLegalMovies, reinterpret_cast<LPVOID*>(&videoFuncs.PlayLegalMovies));
	MH_CreateHook(videoFuncs.PlayMovie, HookdPlayMovie, reinterpret_cast<LPVOID*>(&videoFuncs.PlayMovie));

	MH_CreateHook(videoFuncs.SetVideoMode, HookedSetVideoMode, reinterpret_cast<LPVOID*>(&videoFuncs.SetVideoMode));

	// We hook the entire video subsystem initialization function
	MH_CreateHook(temple_address<0x101DC6E0>(), VideoStartup, NULL);
}
