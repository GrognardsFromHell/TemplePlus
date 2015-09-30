#pragma once

#include <infrastructure/renderstates.h>
#include <atlcomcli.h>

class EditorRenderStates : public RenderStates {
public:
	explicit EditorRenderStates(IDirect3DDevice9* iDirect3DDevice9)
		: mDevice(iDirect3DDevice9) {
	}

	const D3DMATRIX& Get3dProjectionMatrix() override;
	void SetProjectionMatrix(const D3DMATRIX& matrix) override;
	void SetViewMatrix(const D3DMATRIX& matrix) override;
	void SetZEnable(bool enable) override;
	void SetFillMode(D3DFILLMODE fillMode) override;
	void SetZWriteEnable(bool enable) override;
	void SetAlphaTestEnable(bool enable) override;
	void SetSrcBlend(D3DBLEND blend) override;
	void SetDestBlend(D3DBLEND blend) override;
	void SetCullMode(D3DCULL cullMode) override;
	void SetAlphaBlend(bool enable) override;
	void SetLighting(bool enable) override;
	bool IsLighting() override;
	void SetColorVertex(bool enable) override;
	void SetColorWriteEnable(bool enable) override;
	void SetZFunc(D3DCMPFUNC func) override;
	void SetSpecularEnable(bool enable) override;
	void SetTexture(int sampler, IDirect3DTexture9* texture) override;
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
	void SetFVF(DWORD fvf) override;
	void SetStreamSource(int streamIdx, IDirect3DVertexBuffer9* buffer, int stride) override;
	void SetIndexBuffer(IDirect3DIndexBuffer9* buffer, int baseIdx) override;
	void Commit() override;
	void Reset() override;
	void Update3dProjMatrix(float w, float h, float xTrans, float yTrans, float scale);
private:
	D3DMATRIX m3dProjMatrix;
	CComPtr<IDirect3DDevice9> mDevice;
};
