
#pragma once

#include <graphics/buffers.h>
#include <graphics/device.h>
#include <graphics/mdfmaterials.h>
#include <graphics/bufferbinding.h>
#include "common.h"


struct ChainLightningTarget
{
	objHndl obj;
	XMFLOAT3 vec;
	BOOL effectTriggered;
};

// const int testSizeofChainTgt = sizeof(ChainLightningTarget); // should be 24

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
	void RenderChainLightning();
private:
	void Render(size_t vertexCount,
		XMFLOAT4 *vertices,
		XMFLOAT4 *normals,
		XMCOLOR *diffuse,
		XMFLOAT2 *uv,
		size_t primCount,
		uint16_t *indices);

	void RenderMainArc(XMFLOAT3 from, XMFLOAT3 to, int segments, XMFLOAT3 normal, float colorRamp);
	void RenderForks(XMFLOAT3 from, XMFLOAT3 to, int segments, XMFLOAT3 normal, float colorRamp, int minLength, int maxLength);

	gfx::MdfMaterialFactory &mMdfFactory;
	gfx::RenderingDevice &mDevice;

	gfx::MdfRenderMaterialPtr mMaterial;
	
	gfx::VertexBufferPtr mVertexBuffer;
	gfx::IndexBufferPtr mIndexBuffer;
	gfx::BufferBinding mBufferBinding;
};
