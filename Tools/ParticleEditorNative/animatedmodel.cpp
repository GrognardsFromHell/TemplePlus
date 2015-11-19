#include "api.h"
#include <infrastructure/meshes.h>
#include <temple/meshes.h>
#include <infrastructure/stringutil.h>
#include <atlcomcli.h>
#include <infrastructure/exception.h>
#include <infrastructure/renderstates.h>
#include <graphics/math.h>

#include <d3dx9math.h>
#include <temple/dll.h>
#include <iostream>

API gfx::AnimatedModelPtr* AnimatedModel_FromFiles(TempleDll* dll,
                                                   const wchar_t* skmFilename,
                                                   const wchar_t* skaFilename) {

	gfx::AnimatedModelParams params;

	auto model = dll->aasFactory.FromFilenames(ucs2_to_local(skmFilename),
	                                           ucs2_to_local(skaFilename),
	                                           gfx::EncodedAnimId(gfx::WeaponAnim::Idle),
	                                           params);

	return new gfx::AnimatedModelPtr(model);

}

#pragma pack(push, 4)
struct PointVertex {
	D3DVECTOR pos;
	float size;
	D3DCOLOR color;
};
#pragma pack(pop)
static_assert(temple::validate_size<PointVertex, 20>::value, "PointVertex has incorrect size");

class PointCloud {
public:
	explicit PointCloud(const CComPtr<IDirect3DDevice9>& iDirect3DDevice9)
		: mDevice(iDirect3DDevice9) {
	}

	void AddPoint(D3DVECTOR pos, D3DCOLOR color, float size) {
		PointVertex vertex{pos, size, color};
		mVertices.emplace_back(vertex);
	}

	void Render();

private:
	static constexpr DWORD sFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_PSIZE;

	CComPtr<IDirect3DVertexBuffer9> mBuffer;
	CComPtr<IDirect3DDevice9> mDevice;
	std::vector<PointVertex> mVertices;
	int mBufferSize = 0;

	void UpdateBuffer();
};

void PointCloud::UpdateBuffer() {
	int neededSize = sizeof(PointVertex) * mVertices.size();

	if (neededSize > mBufferSize) {
		mBuffer.Release();
		mBufferSize = neededSize;
		auto result = mDevice->CreateVertexBuffer(mBufferSize,
		                                          D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS,
		                                          sFvf,
		                                          D3DPOOL_DEFAULT,
		                                          &mBuffer,
		                                          nullptr
		);
		if (!SUCCEEDED(result)) {
			throw TempleException("Unable to create vertex buffer for point cloud rendering.");
		}
	}

	PointVertex* bufferData;
	auto result = mBuffer->Lock(0, neededSize, (void**)&bufferData, D3DLOCK_DISCARD);
	if (!SUCCEEDED(result)) {
		throw TempleException("Unable to lock vertex buffer while updating the point cloud");
	}

	memcpy(bufferData, mVertices.data(), neededSize);

	result = mBuffer->Unlock();
	if (!SUCCEEDED(result)) {
		throw TempleException("Unable to unlock vertex buffer while updating the point cloud");
	}
}

static DWORD CoerceToInteger(float value) {
	return *(DWORD*)&value;
}

void PointCloud::Render() {

	UpdateBuffer();

	renderStates->SetProjectionMatrix(renderStates->Get3dProjectionMatrix());

	renderStates->SetZEnable(true);
	renderStates->SetColorVertex(true);
	renderStates->SetZWriteEnable(false);
	renderStates->SetLighting(false);

	renderStates->SetFVF(sFvf);
	renderStates->SetStreamSource(0, mBuffer, sizeof(PointVertex));
	renderStates->SetTexture(0, nullptr);
	renderStates->SetTexture(1, nullptr);
	renderStates->SetTexture(2, nullptr);
	renderStates->SetTexture(3, nullptr);
	renderStates->SetIndexBuffer(nullptr, 0);
	renderStates->Commit();

	mDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
	mDevice->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
	mDevice->SetRenderState(D3DRS_POINTSIZE, CoerceToInteger(30.08f));
	mDevice->SetRenderState(D3DRS_POINTSIZE_MIN, CoerceToInteger(10.0f));
	mDevice->SetRenderState(D3DRS_POINTSIZE_MAX, CoerceToInteger(64.0f));
	mDevice->SetRenderState(D3DRS_POINTSCALE_A, CoerceToInteger(10.0f));
	mDevice->SetRenderState(D3DRS_POINTSCALE_B, CoerceToInteger(10.0f));
	mDevice->SetRenderState(D3DRS_POINTSCALE_C, CoerceToInteger(10.0f));

	mDevice->DrawPrimitive(D3DPT_POINTLIST, 0, mVertices.size());

	mDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
	mDevice->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);

	renderStates->SetZEnable(false);
	renderStates->SetAlphaBlend(false);
	renderStates->SetZWriteEnable(true);

}

API void AnimatedModel_Free(gfx::AnimatedModelPtr* handle) {
	delete handle;
}

API void AnimatedModel_Render(gfx::AnimatedModelPtr* handle, IDirect3DDevice9* device, float w, float h, float scale) {
	
	auto model = (*handle).get();

	PointCloud pointCloud(device);

	auto boneCount = model->GetBoneCount();
	gfx::AnimatedModelParams params;
	XMFLOAT4X4 matrix;
	for (auto i = 0; i < boneCount; ++i) {
		auto boneName = model->GetBoneName(i);
		if (!model->GetBoneWorldMatrixByName(params, boneName, &matrix)) {
			continue;
		}

		D3DXVECTOR3 pos{0,0,0};
		D3DVECTOR posWorld;
		D3DXVec3TransformCoord((D3DXVECTOR3*)&posWorld, &pos, (D3DXMATRIX*)&matrix);

		pointCloud.AddPoint(posWorld, D3DCOLOR_ARGB(255, 255, 255, 255), 1);
	}

	pointCloud.Render();

}

API void AnimatedModel_AdvanceTime(gfx::AnimatedModelPtr* handle, float time) {

	auto model = handle->get();

	gfx::AnimatedModelParams params;
	model->Advance(time, 0, 0, params);

}
