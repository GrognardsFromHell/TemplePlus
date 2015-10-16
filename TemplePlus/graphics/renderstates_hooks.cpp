#include "stdafx.h"

#include <infrastructure/renderstates.h>
#include <util/fixes.h>
#include <d3d8adapter.h>
#include <temple/dll.h>

#include "renderstates_data.h"

#pragma pack(push, 1)

struct TigRenderStates {
	D3DXMATRIX projMatrix;
	D3DXMATRIX viewMatrix;
	int zEnable;
	D3DFILLMODE fillMode;
	BOOL zwriteenable;
	BOOL alphatestenable;
	D3DBLEND srcblend;
	D3DBLEND destblend;
	D3DCULL cullmode;
	BOOL alphablendenable;
	BOOL lighting;
	BOOL colorvertex;
	DWORD colorwriteenable;
	D3DCMPFUNC zfunc;
	BOOL specularenable;
	int zbias;
	Direct3DTexture8Adapter* texture[4];
	D3DTEXTUREOP tex_colorop[4];
	DWORD tex_colorarg1[4];
	DWORD tex_colorarg2[4];
	D3DTEXTUREOP tex_alphaop[4];
	DWORD tex_alphaarg1[4];
	DWORD tex_alphaarg2[4];
	DWORD tex_coordindex[4];
	D3DTEXTUREFILTERTYPE tex_mipfilter[4];
	D3DTEXTUREFILTERTYPE tex_magfilter[4];
	D3DTEXTUREFILTERTYPE tex_minfilter[4];
	D3DTEXTUREADDRESS tex_addressu[4];
	D3DTEXTUREADDRESS tex_addressv[4];
	D3DTEXTURETRANSFORMFLAGS tex_transformflags[4];
	DWORD vertexattribs;
	Direct3DVertexBuffer8Adapter* vertexbuffers[4];
	int vertexstrides[4];
	Direct3DIndexBuffer8Adapter* indexbuffer;
	int basevertexindex;
};
#pragma pack(pop)

static struct RenderStatesAddresses : temple::AddressTable {
	// The render states that are currently requested by the system
	TigRenderStates* renderStates;

	// The render states that we believe are currently active on the device
	TigRenderStates* comittedStates;

	RenderStatesAddresses() {
		rebase(renderStates, 0x10EF2F10);
		rebase(comittedStates, 0x10EF30D8);
	}
} addresses;

static class RenderStatesHooks : public TempleFix {
public:
	const char* name() override {
		return "Render States Hooks";
	}

	void apply() override;

	// Commits the given states
	static void CommitState(const TigRenderStates* states);

} fix;

void RenderStatesHooks::apply() {
	replaceFunction(0x101F0A20, CommitState);
}

void RenderStatesHooks::CommitState(const TigRenderStates*states) {
	 
	// Make sure we know when lighting was changed
	renderStates->SetCommitCallback([] {
		addresses.renderStates->lighting = renderStates->IsLighting() ? TRUE : FALSE;
	});

	// This is slow but only necessary until we've replaced all draw calls
	renderStates->SetProjectionMatrix(states->projMatrix);
	renderStates->SetViewMatrix(states->viewMatrix);
	renderStates->SetZEnable(states->zEnable == TRUE);
	renderStates->SetFillMode(states->fillMode);
	renderStates->SetZWriteEnable(states->zwriteenable == TRUE);
	renderStates->SetAlphaTestEnable(states->alphatestenable == TRUE);
	renderStates->SetSrcBlend(states->srcblend);
	renderStates->SetDestBlend(states->destblend);
	renderStates->SetCullMode(states->cullmode);
	renderStates->SetAlphaBlend(states->alphablendenable == TRUE);
	renderStates->SetLighting(states->lighting == TRUE);
	renderStates->SetColorVertex(states->colorvertex == TRUE);
	
	auto enableRed = !!(states->colorwriteenable & D3DCOLORWRITEENABLE_RED);
	auto enableGreen = !!(states->colorwriteenable & D3DCOLORWRITEENABLE_GREEN);
	auto enableBlue = !!(states->colorwriteenable & D3DCOLORWRITEENABLE_BLUE);
	auto enableAlpha = !!(states->colorwriteenable & D3DCOLORWRITEENABLE_ALPHA);
	renderStates->SetColorWriteEnable(enableRed, enableGreen, enableBlue, enableAlpha);

	renderStates->SetZFunc(states->zfunc);
	renderStates->SetSpecularEnable(states->specularenable == TRUE);	
	// TODO	int zbias;

	for (int sampler = 0; sampler < 4; ++sampler) {
		auto texture = states->texture[sampler] 
			? GetTextureDelegate(states->texture[sampler]) 
			: nullptr;
		renderStates->SetTexture(sampler, texture);

		renderStates->SetTextureColorOp(sampler, states->tex_colorop[sampler]);
		renderStates->SetTextureColorArg1(sampler, states->tex_colorarg1[sampler]);
		renderStates->SetTextureColorArg2(sampler, states->tex_colorarg2[sampler]);
		renderStates->SetTextureAlphaOp(sampler, states->tex_alphaop[sampler]);
		renderStates->SetTextureAlphaArg1(sampler, states->tex_alphaarg1[sampler]);
		renderStates->SetTextureAlphaArg2(sampler, states->tex_alphaarg2[sampler]);
		renderStates->SetTextureCoordIndex(sampler, states->tex_coordindex[sampler]);
		renderStates->SetTextureMipFilter(sampler, states->tex_mipfilter[sampler]);
		renderStates->SetTextureMinFilter(sampler, states->tex_minfilter[sampler]);
		renderStates->SetTextureMagFilter(sampler, states->tex_magfilter[sampler]);
		renderStates->SetTextureAddressU(sampler, states->tex_addressu[sampler]);
		renderStates->SetTextureAddressV(sampler, states->tex_addressv[sampler]);
		renderStates->SetTextureTransformFlags(sampler, states->tex_transformflags[sampler]);
	}

	renderStates->SetFVF(states->vertexattribs);

	for (int stream = 0; stream < 4; ++stream) {
		auto buffer = states->vertexbuffers[stream]
			? GetVertexBufferDelegate(states->vertexbuffers[stream])
			: nullptr;
		auto stride = states->vertexstrides[stream];
		renderStates->SetStreamSource(stream, buffer, stride);
	}

	auto indexBuffer = states->indexbuffer 
		? GetIndexBufferDelegate(states->indexbuffer) 
		: nullptr;
	renderStates->SetIndexBuffer(indexBuffer, states->basevertexindex);
	
	renderStates->Commit();
}

// Copy from TP render states into the render states found in ToEE
void ResetLegacyRenderStates() {

	RenderStatesData defaultData;
	auto states = addresses.renderStates;

	// This is slow but only necessary until we've replaced all draw calls
	states->projMatrix = defaultData.mProjMatrix;
	states->viewMatrix = defaultData.mViewMatrix;
	states->zEnable = defaultData.mZEnable ? TRUE : FALSE;
	states->fillMode = defaultData.mFillMode;
	states->zwriteenable = defaultData.mZWriteenable ? TRUE : FALSE;
	states->alphatestenable = defaultData.mAlphatestenable ? TRUE : FALSE;
	states->srcblend = defaultData.mSrcBlend;
	states->destblend = defaultData.mDestBlend;
	states->cullmode = defaultData.mCullmode;
	states->alphablendenable = defaultData.mAlphablendenable ? TRUE : FALSE;
	states->lighting = defaultData.mLighting ? TRUE : FALSE;
	states->colorvertex = defaultData.mColorVertex ? TRUE : FALSE;
	states->colorwriteenable = defaultData.mColorwriteenable;
	states->zfunc = defaultData.mZFunc;
	states->specularenable = defaultData.mSpecularenable ? TRUE : FALSE;
	// TODO	int zbias;

	for (int sampler = 0; sampler < 4; ++sampler) {
		states->texture[sampler] = nullptr;

		states->tex_colorop[sampler] = defaultData.mTextureColorOp[sampler];
		states->tex_colorarg1[sampler] = defaultData.mTextureColorArg1[sampler];
		states->tex_colorarg2[sampler] = defaultData.mTextureColorArg2[sampler];
		states->tex_alphaop[sampler] = defaultData.mTextureAlphaOp[sampler];
		states->tex_alphaarg1[sampler] = defaultData.mTextureAlphaArg1[sampler];
		states->tex_alphaarg2[sampler] = defaultData.mTextureAlphaArg2[sampler];
		states->tex_coordindex[sampler] = defaultData.mTextureCoordIndex[sampler];
		states->tex_mipfilter[sampler] = defaultData.mTextureMipFilter[sampler];
		states->tex_minfilter[sampler] = defaultData.mTextureMinFilter[sampler];
		states->tex_magfilter[sampler] = defaultData.mTextureMagFilter[sampler];
		states->tex_addressu[sampler] = defaultData.mTextureAddressU[sampler];
		states->tex_addressv[sampler] = defaultData.mTextureAddressV[sampler];
		states->tex_transformflags[sampler] = defaultData.mTextureTransformFlags[sampler];
	}

	states->vertexattribs = defaultData.mFvf;

	for (int stream = 0; stream < 4; ++stream) {
		states->vertexbuffers[stream] = nullptr;
		states->vertexstrides[stream] = 0;
	}

	states->indexbuffer = nullptr;
	states->basevertexindex = 0;
}

void CopyLightingState() {
	renderStates->SetLighting(addresses.renderStates->lighting == TRUE);
	renderStates->SetZEnable(addresses.renderStates->zEnable == TRUE);
}
