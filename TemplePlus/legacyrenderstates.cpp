
#include "stdafx.h"

#include <infrastructure/renderstates.h>
#include <temple/dll.h>
#include <d3d8adapter.h>

#include "graphics/graphics.h"

class LegacyRenderStates : public RenderStates {
public:
	LegacyRenderStates(Graphics &g) : mGraphics(g) {}

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

private:
	Graphics &mGraphics;
};

std::unique_ptr<RenderStates> CreateLegacyRenderStates(Graphics &g) {
	return std::make_unique<LegacyRenderStates>(g);
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
			textureAdapters[i] = CreateTextureAdapter(nullptr);
			vertexBufferAdapters[i] = CreateVertexBufferAdapter(nullptr);
		}
		indexBufferAdapter = CreateIndexBufferAdapter(nullptr);
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
		&& GetTextureDelegate(addresses.renderStates->texture[sampler]) != texture) {
		DeleteTextureAdapter(addresses.textureAdapters[sampler]);
		addresses.textureAdapters[sampler] = CreateTextureAdapter(nullptr);
	}

	addresses.renderStates->texture[sampler] = addresses.textureAdapters[sampler];
	SetTextureDelegate(addresses.textureAdapters[sampler], texture);
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
		&& GetVertexBufferDelegate(addresses.renderStates->vertexbuffers[streamIdx]) != buffer) {
		DeleteVertexBufferAdapter(addresses.vertexBufferAdapters[streamIdx]);
		addresses.vertexBufferAdapters[streamIdx] = CreateVertexBufferAdapter(nullptr);
	}
	addresses.renderStates->vertexbuffers[streamIdx] = addresses.vertexBufferAdapters[streamIdx];
	SetVertexBufferDelegate(addresses.vertexBufferAdapters[streamIdx], buffer);
}

void LegacyRenderStates::SetIndexBuffer(IDirect3DIndexBuffer9* buffer, int baseIdx) {
	// If our adapter is already set, we need to replace it with a new one...
	if (addresses.renderStates->indexbuffer == addresses.indexBufferAdapter
		&& GetIndexBufferDelegate(addresses.renderStates->indexbuffer) != buffer) {
		DeleteIndexBufferAdapter(addresses.indexBufferAdapter);
		addresses.indexBufferAdapter = CreateIndexBufferAdapter(nullptr);
	}

	addresses.renderStates->indexbuffer = addresses.indexBufferAdapter;
	SetIndexBufferDelegate(addresses.indexBufferAdapter, buffer);
}

void LegacyRenderStates::Commit() {
	static_assert(sizeof(TigRenderStates) == 0x1C4, "TigRenderStates has the wrong size.");

	addresses.CommitState(addresses.renderStates);
}

void LegacyRenderStates::Reset() {

	auto device = mGraphics.device();
	auto states = addresses.comittedStates;

	device->GetTransform(D3DTS_PROJECTION, &states->projMatrix);
	device->GetTransform(D3DTS_VIEW, &states->viewMatrix);
	device->GetRenderState(D3DRS_ZENABLE, (DWORD*) &states->zEnable);
	device->GetRenderState(D3DRS_FILLMODE, (DWORD*)&states->fillMode);
	device->GetRenderState(D3DRS_ZWRITEENABLE, (DWORD*)&states->zwriteenable);
	device->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&states->alphatestenable);
	device->GetRenderState(D3DRS_SRCBLEND, (DWORD*)&states->srcblend);
	device->GetRenderState(D3DRS_DESTBLEND, (DWORD*)&states->destblend);
	device->GetRenderState(D3DRS_CULLMODE, (DWORD*)&states->cullmode);
	device->GetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD*)&states->alphablendenable);
	device->GetRenderState(D3DRS_LIGHTING, (DWORD*)&states->lighting);
	device->GetRenderState(D3DRS_COLORVERTEX, (DWORD*)&states->colorvertex);
	device->GetRenderState(D3DRS_COLORWRITEENABLE, (DWORD*)&states->colorwriteenable);
	device->GetRenderState(D3DRS_ZFUNC, (DWORD*)&states->zfunc);
	device->GetRenderState(D3DRS_SPECULARENABLE, (DWORD*)&states->specularenable);
	device->GetRenderState(D3DRS_DEPTHBIAS, (DWORD*)&states->zbias); // TODO: Validate
	
	for (int i = 0; i < 4; ++i) {
		CComPtr<IDirect3DBaseTexture9> texture;
		device->GetTexture(i, &texture);
		if (states->texture[i]) {
			DeleteTextureAdapter(states->texture[i]);
			states->texture[i] = nullptr;
		}
		if (texture) {
			IDirect3DTexture9 *tex2d;
			if (SUCCEEDED(texture.QueryInterface<IDirect3DTexture9>(&tex2d))) {
				tex2d->AddRef(); // TODO: Check the ownership semantics here...
				states->texture[i] = CreateTextureAdapter(tex2d);
			}
		}
		device->GetTextureStageState(i, D3DTSS_COLOROP, (DWORD*) &states->tex_colorop[i]);
		device->GetTextureStageState(i, D3DTSS_COLORARG1, (DWORD*)&states->tex_colorarg1[i]);
		device->GetTextureStageState(i, D3DTSS_COLORARG2, (DWORD*)&states->tex_colorarg2[i]);
		device->GetTextureStageState(i, D3DTSS_ALPHAOP, (DWORD*)&states->tex_alphaop[i]);
		device->GetTextureStageState(i, D3DTSS_ALPHAARG1, (DWORD*)&states->tex_alphaarg1[i]);
		device->GetTextureStageState(i, D3DTSS_ALPHAARG2, (DWORD*)&states->tex_alphaarg2[i]);
		device->GetTextureStageState(i, D3DTSS_TEXCOORDINDEX, (DWORD*)&states->tex_coordindex[i]);
		device->GetSamplerState(i, D3DSAMP_MIPFILTER, (DWORD*)&states->tex_mipfilter[i]);
		device->GetSamplerState(i, D3DSAMP_MAGFILTER, (DWORD*)&states->tex_magfilter[i]);
		device->GetSamplerState(i, D3DSAMP_MINFILTER, (DWORD*)&states->tex_minfilter[i]);
		device->GetSamplerState(i, D3DSAMP_ADDRESSU, (DWORD*)&states->tex_addressu[i]);
		device->GetSamplerState(i, D3DSAMP_ADDRESSV, (DWORD*)&states->tex_addressv[i]);
		device->GetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, (DWORD*)&states->tex_transformflags[i]);
	}

	device->GetFVF((DWORD*) &states->vertexattribs);
	
	for (int i = 0; i < 4; ++i) {
		CComPtr<IDirect3DVertexBuffer9> vertexBuffer;
		UINT offsetBytes, stride;
		device->GetStreamSource(i, &vertexBuffer, &offsetBytes, &stride);

		if (states->vertexbuffers[i]) {
			DeleteVertexBufferAdapter(states->vertexbuffers[i]);
			states->vertexbuffers[i] = nullptr;
		}

		if (vertexBuffer) {
			IDirect3DVertexBuffer9* vbRaw = vertexBuffer;
			vbRaw->AddRef(); // TODO: Check the ownership semantics here...
			states->vertexbuffers[i] = CreateVertexBufferAdapter(vbRaw);
		}

		states->vertexstrides[i] = stride;
	}

	CComPtr<IDirect3DIndexBuffer9> indexBuffer;
	device->GetIndices(&indexBuffer);

	if (states->indexbuffer) {
		DeleteIndexBufferAdapter(states->indexbuffer);
		states->indexbuffer = nullptr;
	}

	if (indexBuffer) {
		IDirect3DIndexBuffer9* indexBufferRaw = indexBuffer;
		indexBufferRaw->AddRef();
		states->indexbuffer = CreateIndexBufferAdapter(indexBufferRaw);
	}

	*addresses.renderStates = *addresses.comittedStates;
}
