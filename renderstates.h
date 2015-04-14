
#pragma once

/*
	Defines the state tracker to be used for Direct3D rendering.
*/
class RenderStates {
public:

	void SetProjectionMatrix(const D3DMATRIX &matrix);
	void SetViewMatrix(const D3DMATRIX &matrix);
	void SetZEnable(bool enable);
	void SetFillMode(D3DFILLMODE fillMode);
	void SetZWriteEnable(bool enable);
	void SetAlphaTestEnable(bool enable);
	void SetSrcBlend(D3DBLEND blend);
	void SetCullMode(D3DCULL cullMode);
	void SetAlphaBlend(bool enable);
	void SetLighting(bool enable);
	void SetColorVertex(bool enable); // D3DRS_COLORVERTEX
	void SetColorWriteEnable(bool enable); // D3DRS_COLORWRITEENABLE
	void SetZFunc(D3DCMPFUNC func); // D3DRS_ZFUNC
	void SetSpecularEnable(bool enable);
	void SetTexture(int sampler, IDirect3DTexture9 *texture);
	void SetTextureColorOp(int sampler, D3DTEXTUREOP op);
	void SetTextureColorArg1(int sampler, DWORD arg);
	void SetTextureColorArg2(int sampler, DWORD arg);
	void SetTextureAlphaOp(int sampler, D3DTEXTUREOP op);
	void SetTextureAlphaArg1(int sampler, DWORD arg);
	void SetTextureAlphaArg2(int sampler, DWORD arg);
	void SetTextureCoordIndex(int sampler, int index);
	void SetTextureMipFilter(int sampler, D3DTEXTUREFILTERTYPE type);
	void SetTextureMagFilter(int sampler, D3DTEXTUREFILTERTYPE type);
	void SetTextureMinFilter(int sampler, D3DTEXTUREFILTERTYPE type);
	void SetTextureAddressU(int sampler, D3DTEXTUREADDRESS mode);
	void SetTextureAddressV(int sampler, D3DTEXTUREADDRESS mode);
	void SetTextureTransformFlags(int sampler, D3DTEXTURETRANSFORMFLAGS flags);

	void SetFVF(DWORD fvf);
	void SetStreamSource(int streamIdx, IDirect3DVertexBuffer9 *buffer, int stride);
	void SetIndexBuffer(IDirect3DIndexBuffer9 *buffer, int baseIdx);

	/*
		Writes the current changes to the device.
	*/
	void Commit();

	/*
		Reads the current device state and resets the request state to it.
	*/
	void Reset();
	
};

extern RenderStates renderStates;
