
#pragma once

#include <graphics/buffers.h>
#include <graphics/device.h>
#include <graphics/mdfmaterials.h>
#include <graphics/bufferbinding.h>

enum class LightningRenderState {
	DontRender,
	StartRender,
	Rendering
};

/**
 * Handles rendering of such effects as chain lightning and lightning strike.
 */
class LightningRenderer {
friend class LightningRenderHooks;
public:
	LightningRenderer(gfx::MdfMaterialFactory &mdfFactory,
		gfx::RenderingDevice &device);
	~LightningRenderer();

	void Render();

private:
	void Render(size_t vertexCount,
		XMFLOAT4 *vertices,
		XMFLOAT4 *normals,
		XMCOLOR *diffuse,
		XMFLOAT2 *uv,
		size_t primCount,
		uint16_t *indices);

	gfx::MdfMaterialFactory &mMdfFactory;
	gfx::RenderingDevice &mDevice;

	gfx::MdfRenderMaterialPtr mMaterial;

	LightningRenderState mCallLightningState = LightningRenderState::DontRender;
	uint32_t mCallLightningStart = 0;

	gfx::VertexBufferPtr mVertexBuffer;
	gfx::IndexBufferPtr mIndexBuffer;
	gfx::BufferBinding mBufferBinding;
};
