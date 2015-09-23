
#include "stdafx.h"
#include <infrastructure/renderstates.h>
#include <temple/dll.h>
#include "d3d8to9_texture.h"
#include "d3d8to9_vertexbuffer.h"
#include "d3d8to9_indexbuffer.h"
#include "d3d.h"

class LegacyRenderStates : public RenderStates {
public:
	const D3DMATRIX& Get3dProjectionMatrix() override;
	void SetProjectionMatrix(const D3DMATRIX &matrix) override;
	void SetViewMatrix(const D3DMATRIX &matrix) override;
	void SetZEnable(bool enable) override;
	void SetFillMode(D3DFILLMODE fillMode) override;
	void SetZWriteEnable(bool enable) override;
	void SetAlphaTestEnable(bool enable) override;
	void SetSrcBlend(D3DBLEND blend) override;
	void SetCullMode(D3DCULL cullMode) override;
	void SetAlphaBlend(bool enable) override;
	void SetLighting(bool enable) override;
	void SetColorVertex(bool enable) override; // D3DRS_COLORVERTEX
	void SetColorWriteEnable(bool enable) override; // D3DRS_COLORWRITEENABLE
	void SetZFunc(D3DCMPFUNC func) override; // D3DRS_ZFUNC
	void SetSpecularEnable(bool enable) override;
	void SetTexture(int sampler, IDirect3DTexture9 *texture) override;
	void SetTextureColorOp(int sampler, D3DTEXTUREOP op) override;
	void SetTextureColorArg1(int sampler, DWORD arg) override;
	void SetTextureColorArg2(int sampler, DWORD arg) override;
	void SetTextureAlphaOp(int sampler, D3DTEXTUREOP op) override;
	void SetTextureAlphaArg1(int sampler, DWORD arg) override;
	void SetTextureAlphaArg2(int sampler, DWORD arg) override;
	void SetTextureCoordIndex(int sampler, int index) override;
	void SetTextureMipFilter(int sampler, D3DTEXTUREFILTERTYPE type) override;
	void SetTextureMagFilter(int sampler, D3DTEXTUREFILTERTYPE type) override;
	void SetTextureMinFilter(int sampler, D3DTEXTUREFILTERTYPE type) override;
	void SetTextureAddressU(int sampler, D3DTEXTUREADDRESS mode) override;
	void SetTextureAddressV(int sampler, D3DTEXTUREADDRESS mode) override;
	void SetTextureTransformFlags(int sampler, D3DTEXTURETRANSFORMFLAGS flags) override;

	void SetDestBlend(D3DBLEND blend) override;
	bool IsLighting() override;
	void SetFVF(DWORD fvf) override;
	void SetStreamSource(int streamIdx, IDirect3DVertexBuffer9 *buffer, int stride) override;
	void SetIndexBuffer(IDirect3DIndexBuffer9 *buffer, int baseIdx) override;

	void Commit() override;

	void Reset() override;
};

std::unique_ptr<RenderStates> CreateLegacyRenderStates() {
	return std::make_unique<LegacyRenderStates>();
}

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

static struct RenderStatesAddresses : temple::AddressTable {
	// The render states that are currently requested by the system
	TigRenderStates *renderStates;

	// The render states that we believe are currently active on the device
	TigRenderStates *comittedStates;

	D3DMATRIX *projMatrix3d;

	// Reads the current device state into comittedStates
	void(__cdecl *ReadInitialState)();
	
	// Commits the given states
	void(__cdecl *CommitState)(const TigRenderStates *states);

	RenderStatesAddresses() {
		rebase(renderStates, 0x10EF2F10);
		rebase(comittedStates, 0x10EF30D8);
		rebase(projMatrix3d, 0x11E75788);
				
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

const D3DMATRIX& LegacyRenderStates::Get3dProjectionMatrix() {
	return *addresses.projMatrix3d;
}

void LegacyRenderStates::SetProjectionMatrix(const D3DMATRIX& matrix) {
	addresses.renderStates->projMatrix = matrix;
}

void LegacyRenderStates::SetViewMatrix(const D3DMATRIX& matrix) {
	addresses.renderStates->viewMatrix = matrix;
}

void LegacyRenderStates::SetZEnable(bool enable) {
	addresses.renderStates->zEnable = enable;
}

void LegacyRenderStates::SetFillMode(D3DFILLMODE fillMode) {
	addresses.renderStates->fillMode = fillMode;
}

void LegacyRenderStates::SetZWriteEnable(bool enable) {
	addresses.renderStates->zwriteenable = enable;
}

void LegacyRenderStates::SetAlphaTestEnable(bool enable) {
	addresses.renderStates->alphatestenable = enable;
}

void LegacyRenderStates::SetSrcBlend(D3DBLEND blend) {
	addresses.renderStates->srcblend = blend;
}

void LegacyRenderStates::SetCullMode(D3DCULL cullMode) {
	addresses.renderStates->cullmode = cullMode;
}

void LegacyRenderStates::SetAlphaBlend(bool enable) {
	addresses.renderStates->alphablendenable = enable;
}

void LegacyRenderStates::SetLighting(bool enable) {
	addresses.renderStates->lighting = enable;
}

void LegacyRenderStates::SetColorVertex(bool enable) {
	addresses.renderStates->colorvertex = enable;
}

void LegacyRenderStates::SetColorWriteEnable(bool enable) {
	addresses.renderStates->colorwriteenable = enable;
}

void LegacyRenderStates::SetZFunc(D3DCMPFUNC func) {
	addresses.renderStates->zfunc = func;
}

void LegacyRenderStates::SetSpecularEnable(bool enable) {
	addresses.renderStates->specularenable = enable;
}

void LegacyRenderStates::SetTexture(int sampler, IDirect3DTexture9* texture) {
	// If our adapter is already set, we need to replace it with a new one...
	if (addresses.renderStates->texture[sampler] == addresses.textureAdapters[sampler]
		&& addresses.renderStates->texture[sampler]->delegate != texture) {
		delete addresses.textureAdapters[sampler];
		addresses.textureAdapters[sampler] = new Direct3DTexture8Adapter;
	}

	addresses.renderStates->texture[sampler] = addresses.textureAdapters[sampler];
	addresses.textureAdapters[sampler]->delegate = texture;
}

void LegacyRenderStates::SetTextureColorOp(int sampler, D3DTEXTUREOP op) {
	addresses.renderStates->tex_colorop[sampler] = op;
}

void LegacyRenderStates::SetTextureColorArg1(int sampler, DWORD arg) {
	addresses.renderStates->tex_colorarg1[sampler] = arg;
}

void LegacyRenderStates::SetTextureColorArg2(int sampler, DWORD arg) {
	addresses.renderStates->tex_colorarg2[sampler] = arg;
}

void LegacyRenderStates::SetTextureAlphaOp(int sampler, D3DTEXTUREOP op) {
	addresses.renderStates->tex_alphaop[sampler] = op;
}

void LegacyRenderStates::SetTextureAlphaArg1(int sampler, DWORD arg) {
	addresses.renderStates->tex_alphaarg1[sampler] = arg;
}

void LegacyRenderStates::SetTextureAlphaArg2(int sampler, DWORD arg) {
	addresses.renderStates->tex_alphaarg2[sampler] = arg;
}

void LegacyRenderStates::SetTextureCoordIndex(int sampler, int index) {
	addresses.renderStates->tex_coordindex[sampler] = index;
}

void LegacyRenderStates::SetTextureMipFilter(int sampler, D3DTEXTUREFILTERTYPE type) {
	addresses.renderStates->tex_mipfilter[sampler] = type;
}

void LegacyRenderStates::SetTextureMagFilter(int sampler, D3DTEXTUREFILTERTYPE type) {
	addresses.renderStates->tex_magfilter[sampler] = type;
}

void LegacyRenderStates::SetTextureMinFilter(int sampler, D3DTEXTUREFILTERTYPE type) {
	addresses.renderStates->tex_minfilter[sampler] = type;
}

void LegacyRenderStates::SetTextureAddressU(int sampler, D3DTEXTUREADDRESS mode) {
	addresses.renderStates->tex_addressu[sampler] = mode;
}

void LegacyRenderStates::SetTextureAddressV(int sampler, D3DTEXTUREADDRESS mode) {
	addresses.renderStates->tex_addressv[sampler] = mode;
}

void LegacyRenderStates::SetTextureTransformFlags(int sampler, D3DTEXTURETRANSFORMFLAGS flags) {
	addresses.renderStates->tex_transformflags[sampler] = flags;
}

void LegacyRenderStates::SetDestBlend(D3DBLEND blend) {
	addresses.renderStates->destblend = blend;
}

bool LegacyRenderStates::IsLighting() {
	return addresses.renderStates->lighting != FALSE;
}

void LegacyRenderStates::SetFVF(DWORD fvf) {
	addresses.renderStates->vertexattribs = fvf;
}

void LegacyRenderStates::SetStreamSource(int streamIdx, IDirect3DVertexBuffer9* buffer, int stride) {
	// If our adapter is already set, we need to replace it with a new one...
	if (addresses.renderStates->vertexbuffers[streamIdx] == addresses.vertexBufferAdapters[streamIdx]
		&& addresses.renderStates->vertexbuffers[streamIdx]->delegate != buffer) {
		delete addresses.vertexBufferAdapters[streamIdx];
		addresses.vertexBufferAdapters[streamIdx] = new Direct3DVertexBuffer8Adapter;
	}
	addresses.renderStates->vertexbuffers[streamIdx] = addresses.vertexBufferAdapters[streamIdx];
	addresses.vertexBufferAdapters[streamIdx]->delegate = buffer;
}

void LegacyRenderStates::SetIndexBuffer(IDirect3DIndexBuffer9* buffer, int baseIdx) {
	// If our adapter is already set, we need to replace it with a new one...
	if (addresses.renderStates->indexbuffer == addresses.indexBufferAdapter
		&& addresses.renderStates->indexbuffer->delegate != buffer) {
		delete addresses.indexBufferAdapter;
		addresses.indexBufferAdapter = new Direct3DIndexBuffer8Adapter;
	}

	addresses.renderStates->indexbuffer = addresses.indexBufferAdapter;
	addresses.indexBufferAdapter->delegate = buffer;
}

void LegacyRenderStates::Commit() {
	static_assert(sizeof(TigRenderStates) == 0x1C4, "TigRenderStates has the wrong size.");

	addresses.CommitState(addresses.renderStates);
}

void LegacyRenderStates::Reset() {
	addresses.ReadInitialState();
	memcpy(addresses.renderStates, addresses.comittedStates, sizeof(TigRenderStates));
}
