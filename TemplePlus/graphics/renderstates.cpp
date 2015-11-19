#include "stdafx.h"

#include <infrastructure/renderstates.h>
#include <temple/dll.h>

#include "renderstates_data.h"

/*
class LegacyRenderStates : public RenderStates {
public:
	explicit LegacyRenderStates(Graphics& g);
	~LegacyRenderStates();

	const D3DMATRIX& Get3dProjectionMatrix() override;

	void SetProjectionMatrix(const D3DMATRIX& matrix) override {
		mRequestedData.mProjMatrix = matrix;
	}

	void SetViewMatrix(const D3DMATRIX& matrix) override {
		mRequestedData.mViewMatrix = matrix;
	}

	void SetZEnable(bool enable) override {
		mRequestedData.mZEnable = enable;
	}

	void SetFillMode(D3DFILLMODE fillMode) override {
		mRequestedData.mFillMode = fillMode;
	}

	void SetZWriteEnable(bool enable) override {
		mRequestedData.mZWriteenable = enable;
	}

	void SetAlphaTestEnable(bool enable) override {
		mRequestedData.mAlphatestenable = enable;
	}

	void SetSrcBlend(D3DBLEND blend) override {
		mRequestedData.mSrcBlend = blend;
	}

	void SetDestBlend(D3DBLEND blend) override {
		mRequestedData.mDestBlend = blend;
	}

	void SetCullMode(D3DCULL cullMode) override {
		mRequestedData.mCullmode = cullMode;
	}

	void SetAlphaBlend(bool enable) override {
		mRequestedData.mAlphablendenable = enable;
	}

	void SetLighting(bool enable) override {
		mRequestedData.mLighting = enable;
	}

	void SetColorVertex(bool enable) override {
		mRequestedData.mColorVertex = enable;
	}

	void SetColorWriteEnable(bool enableRed, bool enableGreen, bool enableBlue, bool enableAlpha) override;

	void SetZFunc(D3DCMPFUNC func) override {
		mRequestedData.mZFunc = func;
	}

	void SetSpecularEnable(bool enable) override {
		mRequestedData.mSpecularenable = enable;
	}

	void SetTexture(int sampler, IDirect3DTexture9* texture) override {
		mGraphics.device()->SetTexture(sampler, texture);
	}

	void SetTextureColorOp(int sampler, D3DTEXTUREOP op) override {
		mRequestedData.mTextureColorOp[sampler] = op;
	}

	void SetTextureColorArg1(int sampler, DWORD arg) override {
		mRequestedData.mTextureColorArg1[sampler] = arg;
	}

	void SetTextureColorArg2(int sampler, DWORD arg) override {
		mRequestedData.mTextureColorArg2[sampler] = arg;
	}

	void SetTextureAlphaOp(int sampler, D3DTEXTUREOP op) override {
		mRequestedData.mTextureAlphaOp[sampler] = op;
	}

	void SetTextureAlphaArg1(int sampler, DWORD arg) override {
		mRequestedData.mTextureAlphaArg1[sampler] = arg;
	}

	void SetTextureAlphaArg2(int sampler, DWORD arg) override {
		mRequestedData.mTextureAlphaArg2[sampler] = arg;
	}

	void SetTextureCoordIndex(int sampler, int index) override {
		mRequestedData.mTextureCoordIndex[sampler] = index;
	}

	void SetTextureMipFilter(int sampler, D3DTEXTUREFILTERTYPE type) override {
		mRequestedData.mTextureMipFilter[sampler] = type;
	}

	void SetTextureMagFilter(int sampler, D3DTEXTUREFILTERTYPE type) override {
		mRequestedData.mTextureMagFilter[sampler] = type;
	}

	void SetTextureMinFilter(int sampler, D3DTEXTUREFILTERTYPE type) override {
		mRequestedData.mTextureMinFilter[sampler] = type;
	}

	void SetTextureAddressU(int sampler, D3DTEXTUREADDRESS mode) override {
		mRequestedData.mTextureAddressU[sampler] = mode;
	}

	void SetTextureAddressV(int sampler, D3DTEXTUREADDRESS mode) override {
		mRequestedData.mTextureAddressV[sampler] = mode;
	}

	void SetTextureTransformFlags(int sampler, D3DTEXTURETRANSFORMFLAGS flags) override {
		mRequestedData.mTextureTransformFlags[sampler] = flags;
	}

	bool IsLighting() override {
		return mCurrentData.mLighting;
	}

	void SetFVF(DWORD fvf) override {
		mRequestedData.mFvf = fvf;
	}

	DWORD GetFVF() override {
		return mRequestedData.mFvf;
	}

	void SetStreamSource(int streamIdx, IDirect3DVertexBuffer9* buffer, int stride) override {
		mGraphics.device()->SetStreamSource(streamIdx, buffer, 0, stride);
	}

	void SetIndexBuffer(IDirect3DIndexBuffer9* buffer, int baseIdx) override {
		mGraphics.device()->SetIndices(buffer);
		mRequestedData.mBaseIdx = baseIdx;
	}

	void Commit() override;
	void WriteCurrentToDevice();
	void Reset() override;

private:
	Graphics& mGraphics;

	RenderStatesData mCurrentData;
	RenderStatesData mRequestedData;
};

std::unique_ptr<RenderStates> CreateLegacyRenderStates(Graphics& g) {
	return std::make_unique<LegacyRenderStates>(g);
}

static struct RenderStatesDataAddresses : temple::AddressTable {
	D3DMATRIX* projMatrix3d;

	RenderStatesDataAddresses() {
		rebase(projMatrix3d, 0x11E75788);
	}
} addresses;

RenderStatesData::RenderStatesData() {
	D3DXMatrixIdentity((D3DXMATRIX*)&mProjMatrix);
	D3DXMatrixIdentity((D3DXMATRIX*)&mViewMatrix);
}

LegacyRenderStates::LegacyRenderStates(Graphics& g)
	: mGraphics(g) {
}

LegacyRenderStates::~LegacyRenderStates() {
}

const D3DMATRIX& LegacyRenderStates::Get3dProjectionMatrix() {
	return *addresses.projMatrix3d;
}

void LegacyRenderStates::SetColorWriteEnable(bool enableRed, bool enableGreen, bool enableBlue, bool enableAlpha) {
	DWORD mask = 0;
	if (enableRed) {
		mask |= D3DCOLORWRITEENABLE_RED;
	}
	if (enableGreen) {
		mask |= D3DCOLORWRITEENABLE_GREEN;
	}
	if (enableBlue) {
		mask |= D3DCOLORWRITEENABLE_BLUE;
	}
	if (enableAlpha) {
		mask |= D3DCOLORWRITEENABLE_ALPHA;
	}
	mRequestedData.mColorwriteenable = mask;
}

void LegacyRenderStates::Commit() {
	return;

	auto device = mGraphics.device();

	if (memcmp(&mRequestedData.mProjMatrix, &mCurrentData.mProjMatrix, sizeof(mRequestedData.mProjMatrix))) {
		mCurrentData.mProjMatrix = mRequestedData.mProjMatrix;
		device->SetTransform(D3DTS_PROJECTION, &mCurrentData.mProjMatrix);
	}
	if (memcmp(&mRequestedData.mViewMatrix, &mCurrentData.mViewMatrix, sizeof(mRequestedData.mViewMatrix))) {
		mCurrentData.mViewMatrix = mRequestedData.mViewMatrix;
		device->SetTransform(D3DTS_VIEW, &mRequestedData.mViewMatrix);
	}
	if (mRequestedData.mZEnable != mCurrentData.mZEnable) {
		mCurrentData.mZEnable = mRequestedData.mZEnable;
		device->SetRenderState(D3DRS_ZENABLE, mRequestedData.mZEnable);
	}
	if (mRequestedData.mFillMode != mCurrentData.mFillMode) {
		mCurrentData.mFillMode = mRequestedData.mFillMode;
		device->SetRenderState(D3DRS_FILLMODE, mRequestedData.mFillMode);
	}
	if (mRequestedData.mZWriteenable != mCurrentData.mZWriteenable) {
		mCurrentData.mZWriteenable = mRequestedData.mZWriteenable;
		device->SetRenderState(D3DRS_ZWRITEENABLE, mRequestedData.mZWriteenable);
	}
	if (mRequestedData.mAlphatestenable != mCurrentData.mAlphatestenable) {
		mCurrentData.mAlphatestenable = mRequestedData.mAlphatestenable;
		device->SetRenderState(D3DRS_ALPHATESTENABLE, mRequestedData.mAlphatestenable);
	}
	if (mRequestedData.mSrcBlend != mCurrentData.mSrcBlend) {
		mCurrentData.mSrcBlend = mRequestedData.mSrcBlend;
		device->SetRenderState(D3DRS_SRCBLEND, mRequestedData.mSrcBlend);
	}
	if (mRequestedData.mDestBlend != mCurrentData.mDestBlend) {
		mCurrentData.mDestBlend = mRequestedData.mDestBlend;
		device->SetRenderState(D3DRS_DESTBLEND, mRequestedData.mDestBlend);
	}
	if (mRequestedData.mCullmode != mCurrentData.mCullmode) {
		mCurrentData.mCullmode = mRequestedData.mCullmode;
		device->SetRenderState(D3DRS_CULLMODE, mRequestedData.mCullmode);
	}
	if (mRequestedData.mAlphablendenable != mCurrentData.mAlphablendenable) {
		mCurrentData.mAlphablendenable = mRequestedData.mAlphablendenable;
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, mRequestedData.mAlphablendenable);
	}
	if (mRequestedData.mLighting != mCurrentData.mLighting) {
		mCurrentData.mLighting = mRequestedData.mLighting;
		device->SetRenderState(D3DRS_LIGHTING, mRequestedData.mLighting ? TRUE : FALSE);
	}
	if (mRequestedData.mColorVertex != mCurrentData.mColorVertex) {
		mCurrentData.mColorVertex = mRequestedData.mColorVertex;
		device->SetRenderState(D3DRS_COLORVERTEX, mRequestedData.mColorVertex);
	}
	if (mRequestedData.mColorwriteenable != mCurrentData.mColorwriteenable) {
		mCurrentData.mColorwriteenable = mRequestedData.mColorwriteenable;
		device->SetRenderState(D3DRS_COLORWRITEENABLE, mRequestedData.mColorwriteenable);
	}
	if (mRequestedData.mZFunc != mCurrentData.mZFunc) {
		mCurrentData.mZFunc = mRequestedData.mZFunc;
		device->SetRenderState(D3DRS_ZFUNC, mRequestedData.mZFunc);
	}
	if (mRequestedData.mSpecularenable != mCurrentData.mSpecularenable) {
		mCurrentData.mSpecularenable = mRequestedData.mSpecularenable;
		device->SetRenderState(D3DRS_SPECULARENABLE, mRequestedData.mSpecularenable ? TRUE : FALSE);
	}
	if (mRequestedData.mZBias != mCurrentData.mZBias) {
		mCurrentData.mZBias = mRequestedData.mZBias;
		// TODO device->SetRenderState(D3DRS_ZBIAS, new_state->zbias);
	}

	for (auto sampler = 0; sampler < 4; sampler++) {
		auto colorOp = mRequestedData.mTextureColorOp[sampler];
		if (colorOp != mCurrentData.mTextureColorOp[sampler]) {
			mCurrentData.mTextureColorOp[sampler] = colorOp;
			device->SetTextureStageState(sampler, D3DTSS_COLOROP, colorOp);
		}
		auto colorArg1 = mRequestedData.mTextureColorArg1[sampler];
		if (colorArg1 != mCurrentData.mTextureColorArg1[sampler]) {
			mCurrentData.mTextureColorArg1[sampler] = colorArg1;
			device->SetTextureStageState(sampler, D3DTSS_COLORARG1, colorArg1);
		}
		auto colorArg2 = mRequestedData.mTextureColorArg2[sampler];
		if (colorArg2 != mCurrentData.mTextureColorArg2[sampler]) {
			mCurrentData.mTextureColorArg2[sampler] = colorArg2;
			device->SetTextureStageState(sampler, D3DTSS_COLORARG2, colorArg2);
		}
		auto alphaOp = mRequestedData.mTextureAlphaOp[sampler];
		if (alphaOp != mCurrentData.mTextureAlphaOp[sampler]) {
			mCurrentData.mTextureAlphaOp[sampler] = alphaOp;
			device->SetTextureStageState(sampler, D3DTSS_ALPHAOP, alphaOp);
		}
		auto alphaArg1 = mRequestedData.mTextureAlphaArg1[sampler];
		if (alphaArg1 != mCurrentData.mTextureAlphaArg1[sampler]) {
			mCurrentData.mTextureAlphaArg1[sampler] = alphaArg1;
			device->SetTextureStageState(sampler, D3DTSS_ALPHAARG1, alphaArg1);
		}
		auto alphaArg2 = mRequestedData.mTextureAlphaArg2[sampler];
		if (alphaArg2 != mCurrentData.mTextureAlphaArg2[sampler]) {
			mCurrentData.mTextureAlphaArg2[sampler] = alphaArg2;
			device->SetTextureStageState(sampler, D3DTSS_ALPHAARG2, alphaArg2);
		}
		auto texCoordIdx = mRequestedData.mTextureCoordIndex[sampler];
		if (texCoordIdx != mCurrentData.mTextureCoordIndex[sampler]) {
			mCurrentData.mTextureCoordIndex[sampler] = texCoordIdx;
			device->SetTextureStageState(sampler, D3DTSS_TEXCOORDINDEX, texCoordIdx);
		}
		auto transform = mRequestedData.mTextureTransformFlags[sampler];
		if (transform != mCurrentData.mTextureTransformFlags[sampler]) {
			mCurrentData.mTextureTransformFlags[sampler] = transform;
			device->SetTextureStageState(sampler, D3DTSS_TEXTURETRANSFORMFLAGS, transform);
		}
		auto mipFilter = mRequestedData.mTextureMipFilter[sampler];
		if (mipFilter != mCurrentData.mTextureMipFilter[sampler]) {
			mCurrentData.mTextureMipFilter[sampler] = mipFilter;
			device->SetSamplerState(sampler, D3DSAMP_MIPFILTER, mipFilter);
		}
		auto minFilter = mRequestedData.mTextureMinFilter[sampler];
		if (minFilter != mCurrentData.mTextureMinFilter[sampler]) {
			mCurrentData.mTextureMinFilter[sampler] = minFilter;
			device->SetSamplerState(sampler, D3DSAMP_MINFILTER, minFilter);
		}
		auto magFilter = mRequestedData.mTextureMagFilter[sampler];
		if (magFilter != mCurrentData.mTextureMagFilter[sampler]) {
			mCurrentData.mTextureMagFilter[sampler] = magFilter;
			device->SetSamplerState(sampler, D3DSAMP_MAGFILTER, magFilter);
		}
		auto addressU = mRequestedData.mTextureAddressU[sampler];
		if (addressU != mCurrentData.mTextureAddressU[sampler]) {
			mCurrentData.mTextureAddressU[sampler] = addressU;
			device->SetSamplerState(sampler, D3DSAMP_ADDRESSU, addressU);
		}
		auto addressV = mRequestedData.mTextureAddressV[sampler];
		if (addressV != mCurrentData.mTextureAddressV[sampler]) {
			mCurrentData.mTextureAddressV[sampler] = addressV;
			device->SetSamplerState(sampler, D3DSAMP_ADDRESSV, addressV);
		}
	}
	// NOTE: We do not support base indices anymore

	if (mCommitCallback) {
		mCommitCallback();
	}
}

void LegacyRenderStates::WriteCurrentToDevice() {
	return;
	auto device = mGraphics.device();

	device->SetTransform(D3DTS_PROJECTION, &mCurrentData.mProjMatrix);
	device->SetTransform(D3DTS_VIEW, &mCurrentData.mViewMatrix);
	device->SetRenderState(D3DRS_ZENABLE, mCurrentData.mZEnable);
	device->SetRenderState(D3DRS_FILLMODE, mCurrentData.mFillMode);
	device->SetRenderState(D3DRS_ZWRITEENABLE, mCurrentData.mZWriteenable);
	device->SetRenderState(D3DRS_ALPHATESTENABLE, mCurrentData.mAlphatestenable);
	device->SetRenderState(D3DRS_SRCBLEND, mCurrentData.mSrcBlend);
	device->SetRenderState(D3DRS_DESTBLEND, mCurrentData.mDestBlend);
	device->SetRenderState(D3DRS_CULLMODE, mCurrentData.mCullmode);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, mCurrentData.mAlphablendenable);
	device->SetRenderState(D3DRS_LIGHTING, mCurrentData.mLighting);
	device->SetRenderState(D3DRS_COLORVERTEX, mCurrentData.mColorVertex);
	device->SetRenderState(D3DRS_COLORWRITEENABLE, mCurrentData.mColorwriteenable);
	device->SetRenderState(D3DRS_ZFUNC, mCurrentData.mZFunc);
	device->SetRenderState(D3DRS_SPECULARENABLE, mCurrentData.mSpecularenable);
	// TODO device->SetRenderState(D3DRS_ZBIAS, new_state->zbias);

	for (auto sampler = 0; sampler < 4; sampler++) {
		device->SetTextureStageState(sampler, D3DTSS_COLOROP, mCurrentData.mTextureColorOp[sampler]);
		device->SetTextureStageState(sampler, D3DTSS_COLORARG1, mCurrentData.mTextureColorArg1[sampler]);
		device->SetTextureStageState(sampler, D3DTSS_COLORARG2, mCurrentData.mTextureColorArg2[sampler]);
		device->SetTextureStageState(sampler, D3DTSS_ALPHAOP, mCurrentData.mTextureAlphaOp[sampler]);
		device->SetTextureStageState(sampler, D3DTSS_ALPHAARG1, mCurrentData.mTextureAlphaArg1[sampler]);
		device->SetTextureStageState(sampler, D3DTSS_ALPHAARG2, mCurrentData.mTextureAlphaArg2[sampler]);
		device->SetTextureStageState(sampler, D3DTSS_TEXCOORDINDEX, mCurrentData.mTextureCoordIndex[sampler]);
		device->SetTextureStageState(sampler, D3DTSS_TEXTURETRANSFORMFLAGS, mCurrentData.mTextureTransformFlags[sampler]);
		device->SetSamplerState(sampler, D3DSAMP_MIPFILTER, mCurrentData.mTextureMipFilter[sampler]);
		device->SetSamplerState(sampler, D3DSAMP_MINFILTER, mCurrentData.mTextureMinFilter[sampler]);
		device->SetSamplerState(sampler, D3DSAMP_MAGFILTER, mCurrentData.mTextureMagFilter[sampler]);
		device->SetSamplerState(sampler, D3DSAMP_ADDRESSU, mCurrentData.mTextureAddressU[sampler]);
		device->SetSamplerState(sampler, D3DSAMP_ADDRESSV, mCurrentData.mTextureAddressV[sampler]);
	}

	device->SetFVF(mCurrentData.mFvf);

	if (mCommitCallback) {
		mCommitCallback();
	}
}

void LegacyRenderStates::Reset() {

	mCurrentData = RenderStatesData();
	mRequestedData = mCurrentData;

	WriteCurrentToDevice();

}
*/
