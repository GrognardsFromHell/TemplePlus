#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "stdafx.h"
#include "tig_startup.h"

#include "d3d8to9/d3d8to9.h"

#include "tig_font.h"
#include "temple_functions.h"

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

struct TigMatrices {
	D3DMATRIX* matrix1;
	D3DMATRIX* matrix2;
	D3DMATRIX* matrix3;
};

/*
Container for ToEE functions related to video
*/
struct VideoFuncs : AddressTable {
	bool(__fastcall *TigDirect3dInit)(TigConfig* settings) = nullptr;
	bool (__cdecl *PresentFrame)() = nullptr;
	void (__cdecl *SetVideoMode)(int adapter, int nWidth, int nHeight, int bpp, int refresh, int flags);
	void (__cdecl *CleanUpBuffers)();
	void (__cdecl *ReadInitialState)();
	void (__cdecl *create_partsys_vertex_buffers)();
	void (__cdecl *tig_font_related_init)();
	void (__cdecl *matrix_related)(TigMatrices* matrices);
	void (__cdecl *GamelibResizeScreen)(uint32_t adapter, int width, int height, int bpp, int refresh, int flags);

	// current video format has to be in eax before calling this
	bool (__cdecl *tig_d3d_init_handleformat)();

	// These two callbacks are basically used by create buffers/free buffers to callback into the game layer (above tig layer)
	void (__cdecl *GameCreateVideoBuffers)();
	void (__cdecl *GameFreeVideoBuffers)();

	GlobalBool<0x10D25144> buffersFreed;
	GlobalPrimitive<uint32_t, 0x10D25148> currentFlags;
	GlobalStruct<TigMatrices, 0x10D24E00> tig_matrices2;

	/*
	Used to take screenshots (copy front buffer)
	and move video data into back buffer
	*/
	GlobalPrimitive<uint32_t, 0x11E7575C> backbufferWidth;
	GlobalPrimitive<uint32_t, 0x11E75760> backbufferHeight;

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
	GlobalBool<0x103010FC> tigMovieInitialized;

	GlobalPrimitive<TigRenderStates, 0x10EF2F10> renderStates;
	GlobalPrimitive<TigRenderStates, 0x10EF30D8> activeRenderStates;
	GlobalPrimitive<float, 0x10D24D7C> fadeScreenRect;

	VideoFuncs() {
		rebase(TigDirect3dInit, 0x101DAFB0);
		rebase(PresentFrame, 0x101DCB80);
		rebase(SetVideoMode, 0x101DC870);
		rebase(CleanUpBuffers, 0x101D8640);
		rebase(ReadInitialState, 0x101F06F0);
		rebase(create_partsys_vertex_buffers, 0x101E6E20);
		rebase(tig_font_related_init, 0x101E85C0);
		rebase(matrix_related, 0x101D8910);
		rebase(GameCreateVideoBuffers, 0x10001370);
		rebase(GameFreeVideoBuffers, 0x100013A0);
		rebase(tig_d3d_init_handleformat, 0x101D6F40);
		rebase(GamelibResizeScreen, 0x10002370);
	}
};

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

extern GlobalBool<0x10D250EC> drawFps;
extern GlobalStruct<tig_text_style, 0x10D24DB0> drawFpsTextStyle;
extern GlobalStruct<VideoData, 0x11E74580> video;
extern VideoFuncs videoFuncs;

void ResizeBuffers(int width, int height);
void hook_graphics();

#endif // GRAPHICS_H

