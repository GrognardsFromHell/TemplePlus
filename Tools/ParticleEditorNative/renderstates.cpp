#include <d3dx9math.h>

#include "renderstates.h"

const D3DMATRIX& EditorRenderStates::Get3dProjectionMatrix() {
	return m3dProjMatrix;
}

void EditorRenderStates::SetProjectionMatrix(const D3DMATRIX& matrix) {
	mDevice->SetTransform(D3DTS_PROJECTION, &matrix);
}

void EditorRenderStates::SetViewMatrix(const D3DMATRIX& matrix) {
	mDevice->SetTransform(D3DTS_VIEW, &matrix);
}

void EditorRenderStates::SetZEnable(bool enable) {
	mDevice->SetRenderState(D3DRS_ZENABLE, enable ? TRUE : FALSE);
}

void EditorRenderStates::SetFillMode(D3DFILLMODE fillMode) {
	mDevice->SetRenderState(D3DRS_FILLMODE, fillMode);
}

void EditorRenderStates::SetZWriteEnable(bool enable) {
	mDevice->SetRenderState(D3DRS_ZWRITEENABLE, enable ? TRUE : FALSE);
}

void EditorRenderStates::SetAlphaTestEnable(bool enable) {
	mDevice->SetRenderState(D3DRS_ALPHATESTENABLE, enable ? TRUE : FALSE);
}

void EditorRenderStates::SetSrcBlend(D3DBLEND blend) {
	mDevice->SetRenderState(D3DRS_SRCBLEND, blend);
}

void EditorRenderStates::SetDestBlend(D3DBLEND blend) {
	mDevice->SetRenderState(D3DRS_DESTBLEND, blend);
}

void EditorRenderStates::SetCullMode(D3DCULL cullMode) {
	mDevice->SetRenderState(D3DRS_CULLMODE, cullMode);
}

void EditorRenderStates::SetAlphaBlend(bool enable) {
	mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, enable ? TRUE : FALSE);
}

void EditorRenderStates::SetLighting(bool enable) {
	mDevice->SetRenderState(D3DRS_LIGHTING, enable ? TRUE : FALSE);
}

bool EditorRenderStates::IsLighting() {
	DWORD lighting;
	mDevice->GetRenderState(D3DRS_LIGHTING, &lighting);
	return lighting != FALSE;
}

void EditorRenderStates::SetColorVertex(bool enable) {
	mDevice->SetRenderState(D3DRS_COLORVERTEX, enable ? TRUE : FALSE);
}

void EditorRenderStates::SetColorWriteEnable(bool enableRed, bool enableGreen, bool enableBlue, bool enableAlpha) {
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
	mDevice->SetRenderState(D3DRS_COLORWRITEENABLE, mask);
}

void EditorRenderStates::SetZFunc(D3DCMPFUNC func) {
	mDevice->SetRenderState(D3DRS_ZFUNC, func);
}

void EditorRenderStates::SetSpecularEnable(bool enable) {
	mDevice->SetRenderState(D3DRS_SPECULARENABLE, enable ? TRUE : FALSE);
}

void EditorRenderStates::SetTexture(int sampler, IDirect3DTexture9* texture) {
	mDevice->SetTexture(sampler, texture);
}

void EditorRenderStates::SetTextureColorOp(int sampler, D3DTEXTUREOP op) {
	mDevice->SetTextureStageState(sampler, D3DTSS_COLOROP, op);
}

void EditorRenderStates::SetTextureColorArg1(int sampler, DWORD arg) {
	mDevice->SetTextureStageState(sampler, D3DTSS_COLORARG1, arg);
}

void EditorRenderStates::SetTextureColorArg2(int sampler, DWORD arg) {
	mDevice->SetTextureStageState(sampler, D3DTSS_COLORARG2, arg);
}

void EditorRenderStates::SetTextureAlphaOp(int sampler, D3DTEXTUREOP op) {
	mDevice->SetTextureStageState(sampler, D3DTSS_ALPHAOP, op);
}

void EditorRenderStates::SetTextureAlphaArg1(int sampler, DWORD arg) {
	mDevice->SetTextureStageState(sampler, D3DTSS_ALPHAARG1, arg);
}

void EditorRenderStates::SetTextureAlphaArg2(int sampler, DWORD arg) {
	mDevice->SetTextureStageState(sampler, D3DTSS_ALPHAARG2, arg);
}

void EditorRenderStates::SetTextureCoordIndex(int sampler, int index) {
	mDevice->SetTextureStageState(sampler, D3DTSS_TEXCOORDINDEX, index);
}

void EditorRenderStates::SetTextureMipFilter(int sampler, D3DTEXTUREFILTERTYPE type) {
	mDevice->SetSamplerState(sampler, D3DSAMP_MIPFILTER, type);
}

void EditorRenderStates::SetTextureMagFilter(int sampler, D3DTEXTUREFILTERTYPE type) {
	mDevice->SetSamplerState(sampler, D3DSAMP_MAGFILTER, type);
}

void EditorRenderStates::SetTextureMinFilter(int sampler, D3DTEXTUREFILTERTYPE type) {
	mDevice->SetSamplerState(sampler, D3DSAMP_MINFILTER, type);
}

void EditorRenderStates::SetTextureAddressU(int sampler, D3DTEXTUREADDRESS mode) {
	mDevice->SetSamplerState(sampler, D3DSAMP_ADDRESSU, mode);
}

void EditorRenderStates::SetTextureAddressV(int sampler, D3DTEXTUREADDRESS mode) {
	mDevice->SetSamplerState(sampler, D3DSAMP_ADDRESSV, mode);
}

void EditorRenderStates::SetTextureTransformFlags(int sampler, D3DTEXTURETRANSFORMFLAGS flags) {
	mDevice->SetTextureStageState(sampler, D3DTSS_TEXTURETRANSFORMFLAGS, flags);
}

void EditorRenderStates::SetFVF(DWORD fvf) {
	mDevice->SetFVF(fvf);
}

void EditorRenderStates::SetStreamSource(int streamIdx, IDirect3DVertexBuffer9* buffer, int stride) {
	mDevice->SetStreamSource(streamIdx, buffer, 0, stride);
}

void EditorRenderStates::SetIndexBuffer(IDirect3DIndexBuffer9* buffer, int baseIdx) {
	mDevice->SetIndices(buffer);
}

void EditorRenderStates::Commit() {
}

void EditorRenderStates::Reset() {
}

void EditorRenderStates::Update3dProjMatrix(float w, float h, float xTrans, float yTrans, float scale) {
	D3DXMATRIX projMatrix3d;
	D3DXMatrixIdentity(&projMatrix3d);

	D3DXMATRIX pM1;
	D3DXMatrixTranslation(&pM1, 0.0, 0.0, 0.5);
	D3DXMatrixMultiply(&projMatrix3d, &pM1, &projMatrix3d);
	auto sy = scale / (h * 0.5f);
	auto sx = scale / (w * 0.5f);
	D3DXMatrixScaling(&pM1, sx, sy, 0.00027560207f);
	D3DXMatrixMultiply(&projMatrix3d, &pM1, &projMatrix3d);
	D3DXMatrixRotationX(&pM1, -0.77539754f);
	D3DXMatrixMultiply(&projMatrix3d, &pM1, &projMatrix3d);
	auto screenZTrans = (yTrans - h * 0.5f) * -1.4285715f;
	auto screenXTrans = xTrans - w * 0.5f;
	D3DXMatrixTranslation(&pM1, screenXTrans, 0.0, screenZTrans);
	D3DXMatrixMultiply(&projMatrix3d, &pM1, &projMatrix3d);
	D3DXMatrixRotationY(&pM1, 2.3561945f);
	D3DXMatrixMultiply(&projMatrix3d, &pM1, &projMatrix3d);

	// Try figuring out, what in world coordinateson the x,z plane the screen center is
	D3DXMATRIX projMatrixInv;
	FLOAT det;
	D3DXMatrixInverse(&projMatrixInv, &det, &projMatrix3d);

	D3DXVECTOR3 vec = {0, 0, 0};
	D3DXVECTOR3 vecOut1;
	D3DXVec3TransformCoord(&vecOut1, &vec, &projMatrixInv);

	vec = {0, 0, 1};
	D3DXVECTOR3 vecOut2;
	D3DXVec3TransformCoord(&vecOut2, &vec, &projMatrixInv);

	D3DXVECTOR3 yAxis = vecOut2 - vecOut1;

	vecOut1 -= (vecOut1.y / yAxis.y) * yAxis;

	m3dProjMatrix = projMatrix3d;
}
