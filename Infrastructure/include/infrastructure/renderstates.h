
#pragma once

#include "../platform/d3d.h"

#include <memory>
#include <functional>

/*
	Defines the state tracker to be used for Direct3D rendering.
*/
class RenderStates {
public:
	virtual ~RenderStates() = 0;

	/*
		Returns the unmodified 3d projection matrix.
	*/
	virtual const D3DMATRIX &Get3dProjectionMatrix() = 0;

	// Context for draw calls and needs to go into the shader
	virtual void SetProjectionMatrix(const D3DMATRIX &matrix) = 0;
	virtual void SetViewMatrix(const D3DMATRIX &matrix) = 0;

	virtual void SetZEnable(bool enable) = 0;
	virtual void SetFillMode(D3DFILLMODE fillMode) = 0;
	virtual void SetZWriteEnable(bool enable) = 0;
	virtual void SetAlphaTestEnable(bool enable) = 0;
	virtual void SetSrcBlend(D3DBLEND blend) = 0;
	virtual void SetDestBlend(D3DBLEND blend) = 0;
	virtual void SetCullMode(D3DCULL cullMode) = 0;
	virtual void SetAlphaBlend(bool enable) = 0;
	virtual void SetZFunc(D3DCMPFUNC func) = 0; // D3DRS_ZFUNC
	virtual void SetTexture(int sampler, IDirect3DTexture9 *texture) = 0;
	virtual void SetTextureMipFilter(int sampler, D3DTEXTUREFILTERTYPE type) = 0;
	virtual void SetTextureMagFilter(int sampler, D3DTEXTUREFILTERTYPE type) = 0;
	virtual void SetTextureMinFilter(int sampler, D3DTEXTUREFILTERTYPE type) = 0;
	virtual void SetTextureAddressU(int sampler, D3DTEXTUREADDRESS mode) = 0;
	virtual void SetTextureAddressV(int sampler, D3DTEXTUREADDRESS mode) = 0;

	virtual void SetStreamSource(int streamIdx, IDirect3DVertexBuffer9 *buffer, int stride) = 0;
	virtual void SetIndexBuffer(IDirect3DIndexBuffer9 *buffer, int baseIdx) = 0;

	// Legacy FVF state
	virtual void SetLighting(bool enable) = 0;
	virtual bool IsLighting() = 0;
	virtual void SetColorVertex(bool enable) = 0; // D3DRS_COLORVERTEX
	virtual void SetColorWriteEnable(bool enableRed, bool enableGreen, bool enableBlue, bool enableAlpha) = 0; // D3DRS_COLORWRITEENABLE
	virtual void SetSpecularEnable(bool enable) = 0;
	virtual void SetTextureColorOp(int sampler, D3DTEXTUREOP op) = 0;
	// See D3DTA
	virtual void SetTextureColorArg1(int sampler, DWORD arg) = 0;
	virtual void SetTextureColorArg2(int sampler, DWORD arg) = 0;
	virtual void SetTextureAlphaOp(int sampler, D3DTEXTUREOP op) = 0;
	virtual void SetTextureAlphaArg1(int sampler, DWORD arg) = 0;
	virtual void SetTextureAlphaArg2(int sampler, DWORD arg) = 0;
	virtual void SetTextureCoordIndex(int sampler, int index) = 0;
	virtual void SetTextureTransformFlags(int sampler, D3DTEXTURETRANSFORMFLAGS flags) = 0;
	// See D3DFVF (i.e. D3DFVF_DIFFUSE)
	virtual void SetFVF(DWORD fvf) = 0;
	virtual DWORD GetFVF() = 0;

	/*
		Writes the current changes to the device.
	*/
	virtual void Commit() = 0;
	
	using CommitCallback = std::function<void()>;	
	const CommitCallback& GetCommitCallback() const {
		return mCommitCallback;
	}

	void SetCommitCallback(const CommitCallback& commitCallback) {
		mCommitCallback = commitCallback;
	}

	/*
		Reads the current device state and resets the request state to it.
	*/
	virtual void Reset() = 0;

protected:
	CommitCallback mCommitCallback = nullptr;	
};

inline RenderStates::~RenderStates() = default;

extern std::unique_ptr<RenderStates> renderStates;
