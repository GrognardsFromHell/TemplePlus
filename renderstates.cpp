
#include "stdafx.h"
#include "renderstates.h"
#include "util/addresses.h"
#include "d3d8to9/d3d8to9_texture.h"
#include "d3d8to9/d3d8to9_vertexbuffer.h"
#include "d3d8to9/d3d8to9_indexbuffer.h"

RenderStates renderStates;

#pragma pack(push, 1)
struct TigRenderStates {
	D3DXMATRIX projMatrix;
	D3DXMATRIX viewMatrix;
	int zEnable;
	int fillMode;
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
	Direct3DTexture8Adapter *texture[4];
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
	Direct3DVertexBuffer8Adapter *vertexbuffers[4];
	int vertexstrides[4];
	Direct3DIndexBuffer8Adapter *indexbuffer;
	int basevertexindex;
};
#pragma pack(pop)

static struct RenderStatesAddresses : AddressTable {
	// The render states that are currently requested by the system
	TigRenderStates *renderStates;

	// The render states that we believe are currently active on the device
	TigRenderStates *comittedStates;

	// Reads the current device state into comittedStates
	void(__cdecl *ReadInitialState)();
	
	// Commits the given states
	void(__cdecl *CommitState)(const TigRenderStates *states);

	RenderStatesAddresses() {
		rebase(renderStates, 0x10EF2F10);
		rebase(comittedStates, 0x10EF30D8);
				
		rebase(ReadInitialState, 0x101F06F0);
		rebase(CommitState, 0x101F0A20);

		for (int i = 0; i < 4; ++i) {
			textureAdapters[i] = new Direct3DTexture8Adapter;
			vertexBufferAdapters[i] = new Direct3DVertexBuffer8Adapter;
		}
		indexBufferAdapter = new Direct3DIndexBuffer8Adapter;
	}

	Direct3DTexture8Adapter *textureAdapters[4];
	Direct3DVertexBuffer8Adapter *vertexBufferAdapters[4];
	Direct3DIndexBuffer8Adapter *indexBufferAdapter;

} addresses;

void RenderStates::SetProjectionMatrix(const D3DMATRIX& matrix) {
	addresses.renderStates->projMatrix = matrix;
}

void RenderStates::SetViewMatrix(const D3DMATRIX& matrix) {
	addresses.renderStates->viewMatrix = matrix;
}

void RenderStates::SetZEnable(bool enable) {
	addresses.renderStates->zEnable = enable;
}

void RenderStates::SetFillMode(D3DFILLMODE fillMode) {
	addresses.renderStates->fillMode = fillMode;
}

void RenderStates::SetZWriteEnable(bool enable) {
	addresses.renderStates->zwriteenable = enable;
}

void RenderStates::SetAlphaTestEnable(bool enable) {
	addresses.renderStates->alphatestenable = enable;
}

void RenderStates::SetSrcBlend(D3DBLEND blend) {
	addresses.renderStates->srcblend = blend;
}

void RenderStates::SetCullMode(D3DCULL cullMode) {
	addresses.renderStates->cullmode = cullMode;
}

void RenderStates::SetAlphaBlend(bool enable) {
	addresses.renderStates->alphablendenable = enable;
}

void RenderStates::SetLighting(bool enable) {
	addresses.renderStates->lighting = enable;
}

void RenderStates::SetColorVertex(bool enable) {
	addresses.renderStates->colorvertex = enable;
}

void RenderStates::SetColorWriteEnable(bool enable) {
	addresses.renderStates->colorwriteenable = enable;
}

void RenderStates::SetZFunc(D3DCMPFUNC func) {
	addresses.renderStates->zfunc = func;
}

void RenderStates::SetSpecularEnable(bool enable) {
	addresses.renderStates->specularenable = enable;
}

void RenderStates::SetTexture(int sampler, IDirect3DTexture9* texture) {
	// If our adapter is already set, we need to replace it with a new one...
	if (addresses.renderStates->texture[sampler] == addresses.textureAdapters[sampler]
		&& addresses.renderStates->texture[sampler]->delegate != texture) {
		delete addresses.textureAdapters[sampler];
		addresses.textureAdapters[sampler] = new Direct3DTexture8Adapter;
	}

	addresses.renderStates->texture[sampler] = addresses.textureAdapters[sampler];
	addresses.textureAdapters[sampler]->delegate = texture;
}

void RenderStates::SetTextureColorOp(int sampler, D3DTEXTUREOP op) {
	addresses.renderStates->tex_colorop[sampler] = op;
}

void RenderStates::SetTextureColorArg1(int sampler, DWORD arg) {
	addresses.renderStates->tex_colorarg1[sampler] = arg;
}

void RenderStates::SetTextureColorArg2(int sampler, DWORD arg) {
	addresses.renderStates->tex_colorarg2[sampler] = arg;
}

void RenderStates::SetTextureAlphaOp(int sampler, D3DTEXTUREOP op) {
	addresses.renderStates->tex_alphaop[sampler] = op;
}

void RenderStates::SetTextureAlphaArg1(int sampler, DWORD arg) {
	addresses.renderStates->tex_alphaarg1[sampler] = arg;
}

void RenderStates::SetTextureAlphaArg2(int sampler, DWORD arg) {
	addresses.renderStates->tex_alphaarg2[sampler] = arg;
}

void RenderStates::SetTextureCoordIndex(int sampler, int index) {
	addresses.renderStates->tex_coordindex[sampler] = index;
}

void RenderStates::SetTextureMipFilter(int sampler, D3DTEXTUREFILTERTYPE type) {
	addresses.renderStates->tex_mipfilter[sampler] = type;
}

void RenderStates::SetTextureMagFilter(int sampler, D3DTEXTUREFILTERTYPE type) {
	addresses.renderStates->tex_magfilter[sampler] = type;
}

void RenderStates::SetTextureMinFilter(int sampler, D3DTEXTUREFILTERTYPE type) {
	addresses.renderStates->tex_minfilter[sampler] = type;
}

void RenderStates::SetTextureAddressU(int sampler, D3DTEXTUREADDRESS mode) {
	addresses.renderStates->tex_addressu[sampler] = mode;
}

void RenderStates::SetTextureAddressV(int sampler, D3DTEXTUREADDRESS mode) {
	addresses.renderStates->tex_addressv[sampler] = mode;
}

void RenderStates::SetTextureTransformFlags(int sampler, D3DTEXTURETRANSFORMFLAGS flags) {
	addresses.renderStates->tex_transformflags[sampler] = flags;
}

void RenderStates::SetFVF(DWORD fvf) {
	addresses.renderStates->vertexattribs = fvf;
}

void RenderStates::SetStreamSource(int streamIdx, IDirect3DVertexBuffer9* buffer, int stride) {
	// If our adapter is already set, we need to replace it with a new one...
	if (addresses.renderStates->vertexbuffers[streamIdx] == addresses.vertexBufferAdapters[streamIdx]
		&& addresses.renderStates->vertexbuffers[streamIdx]->delegate != buffer) {
		delete addresses.vertexBufferAdapters[streamIdx];
		addresses.vertexBufferAdapters[streamIdx] = new Direct3DVertexBuffer8Adapter;
	}
	addresses.renderStates->vertexbuffers[streamIdx] = addresses.vertexBufferAdapters[streamIdx];
	addresses.vertexBufferAdapters[streamIdx]->delegate = buffer;
}

void RenderStates::SetIndexBuffer(IDirect3DIndexBuffer9* buffer, int baseIdx) {
	// If our adapter is already set, we need to replace it with a new one...
	if (addresses.renderStates->indexbuffer == addresses.indexBufferAdapter
		&& addresses.renderStates->indexbuffer->delegate != buffer) {
		delete addresses.indexBufferAdapter;
		addresses.indexBufferAdapter = new Direct3DIndexBuffer8Adapter;
	}

	addresses.renderStates->indexbuffer = addresses.indexBufferAdapter;
	addresses.indexBufferAdapter->delegate = buffer;
}

void RenderStates::Commit() {
	static_assert(sizeof(TigRenderStates) == 0x1C4, "TigRenderStates has the wrong size.");

	addresses.CommitState(addresses.renderStates);
}

void RenderStates::Reset() {
	addresses.ReadInitialState();
	memcpy(addresses.renderStates, addresses.comittedStates, sizeof(TigRenderStates));
}
