#include "..\include\temple\aasrenderer.h"
#include "..\include\temple\aasrenderer.h"
#include "..\include\temple\aasrenderer.h"
#include "..\include\temple\aasrenderer.h"

#include <graphics/materials.h>
#include <graphics/buffers.h>
#include <graphics/bufferbinding.h>
#include <graphics/device.h>
#include <graphics/shaders.h>
#include <graphics/mdfmaterials.h>

#include "temple/aasrenderer.h"

#include <infrastructure/logging.h>

using namespace gfx;

namespace temple {

static AasRenderer* aasRenderer = nullptr;

// Data to be stored for an AAS model so it can be rendered 
// more efficiently
struct AasRenderSubmeshData {	
	bool created = false;
	VertexBufferPtr posBuffer;
	VertexBufferPtr normalsBuffer;
	VertexBufferPtr uvBuffer;
	IndexBufferPtr idxBuffer;
	BufferBinding binding;

	AasRenderSubmeshData() = default;

	NO_COPY_OR_MOVE(AasRenderSubmeshData);
};

struct AasRenderData {
	std::array<AasRenderSubmeshData, 16> submeshes;

	AasRenderData() = default;
	NO_COPY_OR_MOVE(AasRenderData);
};

AasRenderer::AasRenderer(AasAnimatedModelFactory &aasFactory, 
						 RenderingDevice& device, 
						 MdfMaterialFactory &mdfFactory) 
	: mDevice(device), 
	  mMdfFactory(mdfFactory), 
	  mAasFactory(aasFactory),
	  mGeometryShadowMaterial(CreateGeometryShadowMaterial(device)) {
	Expects(!aasRenderer);

	/*
		When an AAS handle is freed by the factory, remove all associated
		rendering data here as well.
	*/
	mListenerHandle = aasFactory.AddFreeListener([&](AasHandle handle) {
		auto it = mRenderDataCache.find(handle);
		if (it != mRenderDataCache.end()) {
			mRenderDataCache.erase(it);
		}
	});

}

AasRenderer::~AasRenderer() {
	aasRenderer = nullptr;
	mAasFactory.RemoveFreeListener(mListenerHandle);
}

AasRenderSubmeshData& AasRenderer::GetSubmeshData(AasRenderData& renderData, int submeshId, Submesh & submesh)
{
	auto& submeshData = renderData.submeshes[submeshId];
	if (!submeshData.created) {
		submeshData.posBuffer = mDevice.CreateVertexBuffer(submesh.GetPositions());
		submeshData.normalsBuffer = mDevice.CreateVertexBuffer(submesh.GetNormals());
		submeshData.uvBuffer = mDevice.CreateVertexBuffer(submesh.GetUV());
		submeshData.idxBuffer = mDevice.CreateIndexBuffer(submesh.GetIndices());

		submeshData.binding.AddBuffer(submeshData.posBuffer, 0, sizeof(DirectX::XMFLOAT4))
			.AddElement(VertexElementType::Float4, VertexElementSemantic::Position);
		submeshData.binding.AddBuffer(submeshData.normalsBuffer, 0, sizeof(DirectX::XMFLOAT4))
			.AddElement(VertexElementType::Float4, VertexElementSemantic::Normal);
		submeshData.binding.AddBuffer(submeshData.uvBuffer, 0, sizeof(DirectX::XMFLOAT2))
			.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);

		submeshData.created = true;
	} else {
		submeshData.posBuffer->Update(submesh.GetPositions());
		submeshData.normalsBuffer->Update(submesh.GetNormals());
	}

	return submeshData;
}

Material AasRenderer::CreateGeometryShadowMaterial(gfx::RenderingDevice &device)
{
	BlendState blendState;
	blendState.blendEnable = true;
	blendState.srcBlend = D3DBLEND_SRCALPHA;
	blendState.destBlend = D3DBLEND_INVSRCALPHA;
	RasterizerState rasterizerState;
	rasterizerState.cullMode = D3DCULL_NONE;
	DepthStencilState depthStencilState;

	auto vs{ device.GetShaders().LoadVertexShader("shadow_geom_vs") };
	auto ps{ device.GetShaders().LoadPixelShader("diffuse_only_ps") };

	return Material(blendState, depthStencilState, rasterizerState, {}, vs, ps);
}

void AasRenderer::RecalcNormals(int vertexCount, const XMFLOAT4* pos, XMFLOAT4* normals, int primCount, const uint16_t* indices) {
	static XMFLOAT4 recalcNormalsBuffer[0x8000];
	memset(recalcNormalsBuffer, 0, vertexCount * sizeof(XMFLOAT4));

	// Process every TRI we have
	auto curIdx = indices;
	for (auto tri = 0; tri < primCount; ++tri) {
		// Indices of the three vertices making up this triangle
		auto idx1 = *curIdx++;
		auto idx2 = *curIdx++;
		auto idx3 = *curIdx++;

		auto pos1 = XMLoadFloat4(&pos[idx1]);
		auto pos2 = XMLoadFloat4(&pos[idx2]);
		auto pos3 = XMLoadFloat4(&pos[idx3]);

		auto v1to2(pos2 - pos1);
		auto v1to3(pos3 - pos1);

		// Calculate the surface normal of the surface defined 
		// by the two directional vectors
		auto surfNormal(XMVector3Cross(v1to2, v1to3) * -1);

		// The surface normal contributes to all three vertex normals
		XMStoreFloat4(&normals[idx1], surfNormal);
		XMStoreFloat4(&normals[idx2], surfNormal);
		XMStoreFloat4(&normals[idx3], surfNormal);
	}

	// Re-Normalize the normals we calculated
	for (auto i = 0; i < vertexCount; ++i) {
		auto normal(XMVector3Normalize(XMLoadFloat4(&normals[i])));
		XMStoreFloat4(&normals[i], normal);
	}
}

void AasRenderer::Render(gfx::AnimatedModel *model,
	const gfx::AnimatedModelParams& params,
	gsl::array_view<Light3d> lights,
	const MdfRenderOverrides *materialOverrides) {

	// Find or create render caching data for the model
	auto &renderData = mRenderDataCache[model->GetHandle()];
	if (!renderData) {
		renderData = std::make_unique<AasRenderData>();
	}
	
	auto materialIds(model->GetSubmeshes());
	for (size_t i = 0; i < materialIds.size(); ++i) {
		auto materialId = materialIds[i];
		auto submesh(model->GetSubmesh(params, i));
		
		// Remove special material marker in the upper byte and only 
		// use the actual shader registration id
		materialId &= 0x00FFFFFF;

		// Usually this should not happen, since it means there's  
		// an unbound replacement material
		if (materialId == 0) {
			continue;
		}

		auto material = mMdfFactory.GetById(materialId);
		
		if (!material) {
			logger->error("Legacy shader with id {} wasn't found.", materialId);
			continue;
		}

		material->Bind(mDevice, lights, materialOverrides);

		// Do we have to recalculate the normals?
		if (material->GetSpec()->recalculateNormals) {
			RecalcNormals(
				submesh->GetVertexCount(),
				submesh->GetPositions().data(),
				submesh->GetNormals().data(),
				submesh->GetPrimitiveCount(),
				submesh->GetIndices().data()
			);
		}

		auto &submeshData = GetSubmeshData(*renderData, i, *submesh);
		submeshData.binding.Bind();

		auto d3d = mDevice.GetDevice();
		d3d->SetIndices(submeshData.idxBuffer->GetBuffer());
		D3DLOG(d3d->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, submesh->GetVertexCount(), 0, submesh->GetPrimitiveCount()));

	}

}

void AasRenderer::RenderWithoutMaterial(gfx::AnimatedModel *model, 
	const gfx::AnimatedModelParams& params) {

	// Find or create render caching data for the model
	auto &renderData = mRenderDataCache[model->GetHandle()];
	if (!renderData) {
		renderData = std::make_unique<AasRenderData>();
	}

	auto materialIds(model->GetSubmeshes());
	for (size_t i = 0; i < materialIds.size(); ++i) {
		auto submesh(model->GetSubmesh(params, i));

		auto &submeshData = GetSubmeshData(*renderData, i, *submesh);
		submeshData.binding.Bind();

		auto d3d = mDevice.GetDevice();
		d3d->SetIndices(submeshData.idxBuffer->GetBuffer());
		D3DLOG(d3d->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, submesh->GetVertexCount(), 0, submesh->GetPrimitiveCount()));
	}
}

void AasRenderer::RenderGeometryShadow(gfx::AnimatedModel * model, 
	const gfx::AnimatedModelParams & params, 
	const gfx::Light3d & globalLight)
{
	
	// Find or create render caching data for the submesh
	auto &renderData = mRenderDataCache[model->GetHandle()];
	if (!renderData) {
		renderData = std::make_unique<AasRenderData>();
	}

	mDevice.SetMaterial(mGeometryShadowMaterial);

	auto d3d = mDevice.GetDevice();
	d3d->SetVertexShaderConstantF(0, &mDevice.GetCamera().GetViewProj()._11, 4);
	d3d->SetVertexShaderConstantF(4, &globalLight.dir.x, 1);
	XMFLOAT4 floats{ params.offsetZ, 0, 0, 0 };
	d3d->SetVertexShaderConstantF(5, &floats.x, 1);

	auto materialIds(model->GetSubmeshes());
	for (size_t i = 0; i < materialIds.size(); ++i) {
		auto submesh(model->GetSubmesh(params, i));
		
		auto &submeshData = GetSubmeshData(*renderData, i, *submesh);
		submeshData.binding.Bind();
				
		d3d->SetIndices(submeshData.idxBuffer->GetBuffer());
		D3DLOG(d3d->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, submesh->GetVertexCount(), 0, submesh->GetPrimitiveCount()));
	}

}

}
