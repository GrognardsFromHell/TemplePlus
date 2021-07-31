#pragma once

#include <span>
#include <obj.h>

namespace gfx {
	struct AnimatedModelParams;
	class RenderingDevice;
	class WorldCamera;
	struct Light3d;
	class AnimatedModel;
	class MdfMaterialFactory;
	using MdfRenderMaterialPtr = std::shared_ptr<class MdfRenderMaterial>;
	using RenderTargetTexturePtr = std::shared_ptr<class RenderTargetTexture>;
}
namespace aas {
	class Renderer;
}

class GameSystems;

enum class ShadowType {
	ShadowMap,
	Geometry,
	Blob
};

class MapObjectRenderer {
public:

	MapObjectRenderer(GameSystems& gameSystems, 
		gfx::RenderingDevice& device, 
		gfx::WorldCamera& camera,
		gfx::MdfMaterialFactory &mdfFactory,
		aas::Renderer &aasRenderer);
	~MapObjectRenderer();

	void RenderMapObjects(int tileX1, int tileX2, int tileY1, int tileY2);
	void RenderObject(objHndl handle, bool showInvisible);
	void RenderObjectInUi(objHndl handle, int x, int y, float rotation, float scale);
	void RenderOccludedMapObjects(int tileX1, int tileX2, int tileY1, int tileY2);
	void RenderOccludedObject(objHndl handle);

	void RenderObjectHighlight(objHndl handle, const gfx::MdfRenderMaterialPtr &material);

	size_t GetRenderedLastFrame() const {
		return mRenderedLastFrame;
	}
	size_t GetTotalLastFrame() const {
		return mTotalLastFrame;
	}

	ShadowType GetShadowType() const {
		return mShadowType;
	}
	void SetShadowType(ShadowType type) {
		mShadowType = type;
	}

	void SetShowHighlight(bool enable) {
		mShowHighlights = enable;
	}
	bool GetShowHighlights() const {
		return mShowHighlights;
	}

	std::vector<gfx::Light3d> FindLights(LocAndOffsets atLocation, float radius);

private:
	GameSystems& mGameSystems;
	gfx::RenderingDevice& mDevice;
	gfx::WorldCamera& mCamera;
	aas::Renderer &mAasRenderer;
	ShadowType mShadowType = ShadowType::ShadowMap;
	gfx::MdfRenderMaterialPtr mBlobShadowMaterial;
	gfx::MdfRenderMaterialPtr mHighlightMaterial;
	gfx::MdfRenderMaterialPtr mOccludedMaterial;
	std::unique_ptr<class FrogGrappleController> mGrappleController;
	std::array<gfx::MdfRenderMaterialPtr, 10> mGlowMaterials;
	bool mShowHighlights = false;

	size_t mRenderedLastFrame = 0;
	size_t mTotalLastFrame = 0;

	/*
	Same as sin45 incidentally.
	The idea seems to be that the vertical height of the model is scaled
	according to the camera inclination of 45°.
	*/
	static constexpr float cos45 = 0.70709997f;

	bool IsObjectOnScreen(LocAndOffsets &location, float offsetZ, float radius, float renderHeight);
	void RenderMirrorImages(objHndl handle,
		const gfx::AnimatedModelParams &animParams,
		gfx::AnimatedModel &model,
		std::span<gfx::Light3d> lights);

	void RenderShadowMapShadow(objHndl handle, 
		const gfx::AnimatedModelParams &animParams, 
		gfx::AnimatedModel &model, 
		const gfx::Light3d& globalLight,
		int alpha);
	void RenderBlobShadow(objHndl handle, 
		gfx::AnimatedModel &model, 
		gfx::AnimatedModelParams &animParams,
		int alpha);

};
