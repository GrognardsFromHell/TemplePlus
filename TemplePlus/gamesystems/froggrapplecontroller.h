
#pragma once

#include <obj.h>
#include <graphics/bufferbinding.h>
#include <graphics/materials.h>
#include <graphics/mdfmaterials.h>

struct GrappleState;

/**
 * Controls the tongue grapple performed by giant frogs.
 */
class FrogGrappleController {
public:

	FrogGrappleController(gfx::RenderingDevice &device,
		gfx::MdfMaterialFactory &mdfFactory);

	bool IsGiantFrog(objHndl obj) const;

	void AdvanceAndRender(objHndl obj, 
		const gfx::AnimatedModelParams &animParams,
		gfx::AnimatedModel &model,
		std::span<gfx::Light3d> lights,
		float alpha);

private:
	static constexpr size_t VertexCount = 96;
	static constexpr size_t TriCount = 180;

	gfx::RenderingDevice& mDevice;
	gfx::MdfRenderMaterialPtr mTongueMaterial;
	gfx::BufferBinding mBufferBinding;
	gfx::VertexBufferPtr mVertexBuffer;
	gfx::IndexBufferPtr mIndexBuffer;

	void CreateIndexBuffer();
	void RenderTongue(const GrappleState &grappleState,
		const XMFLOAT4X4 &worldMatrixOrigin,
		std::span<gfx::Light3d> lights,
		float alpha);

	objHndl GetGrappledOpponent(objHndl giantFrog);
	void SetGrappleState(objHndl giantFrog, const GrappleState &state);
	GrappleState GetGrappleState(objHndl giantFrog);
	
};
