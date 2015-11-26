
#pragma once

#include <graphics/materials.h>
#include <infrastructure/meshes.h>
#include <temple/meshes.h>

namespace gfx {
	class RenderingDevice;
	class MdfMaterialFactory;
	struct MdfRenderOverrides;
}

namespace temple {

	using AasStatePtr = std::unique_ptr<struct AasRenderData>;

	struct AasRenderSubmeshData;

	class AasRenderer : public gfx::AnimatedModelRenderer {
	public:
		AasRenderer(
			AasAnimatedModelFactory &aasFactory,
			gfx::RenderingDevice &device,
			gfx::MdfMaterialFactory &mdfFactory);
		~AasRenderer();

		void Render(gfx::AnimatedModel *model,
			const gfx::AnimatedModelParams& params,
			gsl::array_view<gfx::Light3d> lights,
			const gfx::MdfRenderOverrides *materialOverrides = nullptr) override;

		void RenderWithoutMaterial(gfx::AnimatedModel *model,
			const gfx::AnimatedModelParams& params);

		void RenderGeometryShadow(gfx::AnimatedModel *model,
			const gfx::AnimatedModelParams& params,
			const gfx::Light3d &globalLight);

	private:
		AasAnimatedModelFactory &mAasFactory;
		gfx::RenderingDevice &mDevice;
		gfx::Material mGeometryShadowMaterial;
		gfx::MdfMaterialFactory &mMdfFactory;
		AasFreeListenerHandle mListenerHandle;
		std::unordered_map<AasHandle, AasStatePtr> mRenderDataCache;

		AasRenderSubmeshData &GetSubmeshData(AasRenderData& aasState,
			int submeshId,
			gfx::Submesh &submesh);

		static gfx::Material CreateGeometryShadowMaterial(gfx::RenderingDevice &device);

		void RecalcNormals(
			int vertexCount,
			const DirectX::XMFLOAT4* pos,
			DirectX::XMFLOAT4* normals,
			int primCount,
			const uint16_t* indices);
	};

}
