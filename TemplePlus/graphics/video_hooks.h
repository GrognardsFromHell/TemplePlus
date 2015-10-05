
#pragma once

#include <d3d.h>
#include <d3dx9math.h>

#include <tig/tig_startup.h>
#include <tig/tig_font.h>

class MainWindow;
class Graphics;

struct TigMatrices {
	float xOffset;
	float yOffset;
	float scale;
};

struct Direct3D8Adapter;
struct Direct3DDevice8Adapter;
struct Direct3DVertexBuffer8Adapter;

/*
Container for ToEE functions related to video
*/
struct VideoFuncs : temple::AddressTable {
	bool(__fastcall *TigDirect3dInit)(TigConfig* settings) = nullptr;
	void(__cdecl *SetVideoMode)(int adapter, int nWidth, int nHeight, int bpp, int refresh, int flags);
	void(__cdecl *CleanUpBuffers)();

	void(__cdecl *create_partsys_vertex_buffers)();
	void(__cdecl *tig_font_related_init)();
	void(__cdecl *updateProjMatrices)(TigMatrices* matrices);
	void(__cdecl *GamelibResizeScreen)(uint32_t adapter, int width, int height, int bpp, int refresh, int flags);

	// current video format has to be in eax before calling this
	bool(__cdecl *tig_d3d_init_handleformat)();

	// These two callbacks are basically used by create buffers/free buffers to callback into the game layer (above tig layer)
	void(__cdecl *GameCreateVideoBuffers)();
	void(__cdecl *GameFreeVideoBuffers)();

	temple::GlobalBool<0x10D25144> buffersFreed;
	temple::GlobalPrimitive<uint32_t, 0x10D25148> currentFlags;
	TigMatrices *tigMatrices2;

	/*
	Used to take screenshots (copy front buffer)
	and move video data into back buffer
	*/
	temple::GlobalPrimitive<uint32_t, 0x11E7575C> backbufferWidth;
	temple::GlobalPrimitive<uint32_t, 0x11E75760> backbufferHeight;

	/*
	Several subsystems use this to detect windowed mode.
	Although the entire startup config is copied to this location by
	some portions of the game. We only write 0x20 (windowed) to this
	statically.
	*/
	temple::GlobalPrimitive<uint32_t, 0x11E75840> startupFlags;
		
	temple::GlobalPrimitive<Direct3DVertexBuffer8Adapter*, 0x10D25120> globalFadeVBuffer;
	temple::GlobalPrimitive<Direct3DVertexBuffer8Adapter*, 0x10D25124> sharedVBuffer1;
	temple::GlobalPrimitive<Direct3DVertexBuffer8Adapter*, 0x10D25128> sharedVBuffer2;
	temple::GlobalPrimitive<Direct3DVertexBuffer8Adapter*, 0x10D2512C> sharedVBuffer3;
	temple::GlobalPrimitive<Direct3DVertexBuffer8Adapter*, 0x10D25130> sharedVBuffer4;
	temple::GlobalBool<0x103010FC> tigMovieInitialized;

	temple::GlobalPrimitive<float, 0x10D24D7C> fadeScreenRect;

	VideoFuncs() {
		rebase(TigDirect3dInit, 0x101DAFB0);
		rebase(SetVideoMode, 0x101DC870);
		rebase(CleanUpBuffers, 0x101D8640);
		rebase(create_partsys_vertex_buffers, 0x101E6E20);
		rebase(tig_font_related_init, 0x101E85C0);
		rebase(updateProjMatrices, 0x101D8910);
		rebase(GameCreateVideoBuffers, 0x10001370);
		rebase(GameFreeVideoBuffers, 0x100013A0);
		rebase(tig_d3d_init_handleformat, 0x101D6F40);
		rebase(GamelibResizeScreen, 0x10002370);
		rebase(tigMatrices2, 0x10D24E00);
	}
};

/*
* Size being cleared is 4796 byte in length
* Start @ 0x11E74580
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
	TigMatrices viewParams;
};

extern temple::GlobalBool<0x10D250EC> drawFps;
extern temple::GlobalStruct<TigTextStyle, 0x10D24DB0> drawFpsTextStyle;
extern temple::GlobalStruct<VideoData, 0x11E74580> video;
extern VideoFuncs videoFuncs;
