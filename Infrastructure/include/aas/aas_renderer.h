
#pragma once

#include <graphics/materials.h>
#include <infrastructure/meshes.h>

namespace gfx {
	class RenderingDevice;
	class MdfMaterialFactory;
	struct MdfRenderOverrides;
	class ShapeRenderer2d;
	class ShapeRenderer3d;
}

namespace aas {

	using AasStatePtr = std::unique_ptr<struct AasRenderData>;

	struct AasRenderSubmeshData;

	class Renderer : public gfx::AnimatedModelRenderer {
	public:
		Renderer(
			gfx::AnimatedModelFactory &aasFactory,
			gfx::RenderingDevice &device,
			gfx::ShapeRenderer2d &shapeRenderer2d,
			gfx::ShapeRenderer3d &shapeRenderer3d,
			gfx::MdfMaterialFactory &mdfFactory);
		~Renderer();

		void Render(gfx::AnimatedModel *model,
			const gfx::AnimatedModelParams& params,
			std::span<gfx::Light3d> lights,
			const gfx::MdfRenderOverrides *materialOverrides = nullptr) override;

		void RenderWithoutMaterial(gfx::AnimatedModel *model,
			const gfx::AnimatedModelParams& params);

		void RenderGeometryShadow(gfx::AnimatedModel *model,
			const gfx::AnimatedModelParams& params,
			const gfx::Light3d &globalLight,
			float alpha);

		void RenderShadowMapShadow(std::span<gfx::AnimatedModel*> models,
			std::span<const gfx::AnimatedModelParams*> modelParams,
			const XMFLOAT3 &center,
			float radius,
			float height,
			const XMFLOAT4 &lightDir,
			float alpha,
			bool softShadows);

	private:
		gfx::AnimatedModelFactory &mAasFactory;
		gfx::RenderingDevice &mDevice;
		gfx::ShapeRenderer2d &mShapeRenderer2d;
		gfx::ShapeRenderer3d &mShapeRenderer3d;
		gfx::Material mGeometryShadowMaterial;
		gfx::MdfMaterialFactory &mMdfFactory;
		
		// Shadow map related state
		gfx::RenderTargetTexturePtr mShadowTarget; // Shadow map texture
		gfx::RenderTargetTexturePtr mShadowTargetTmp; // Temp buffer for gauss blur
		gfx::Material mShadowMapMaterial;
		gfx::Material mGaussBlurHor; // Material for horizontal pass of gauss blur
		gfx::Material mGaussBlurVer; // Material for vertical pass of gauss blur

		AasRenderSubmeshData &GetSubmeshData(AasRenderData& aasState,
			int submeshId,
			gfx::Submesh &submesh);

		static gfx::Material CreateGeometryShadowMaterial(gfx::RenderingDevice &device);
		static gfx::Material CreateShadowMapMaterial(gfx::RenderingDevice &device);
		static gfx::Material CreateGaussBlurMaterial(gfx::RenderingDevice &device,
			const gfx::RenderTargetTexturePtr &texture,
			bool horizontal);

		void RecalcNormals(
			int vertexCount,
			const DirectX::XMFLOAT4* pos,
			DirectX::XMFLOAT4* normals,
			int primCount,
			const uint16_t* indices);

		static constexpr int ShadowMapWidth = 256;
		static constexpr int ShadowMapHeight = 256;
	};

}
