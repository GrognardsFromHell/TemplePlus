#pragma once

#include <graphics/buffers.h>
#include <graphics/device.h>
#include <graphics/mdfmaterials.h>
#include <graphics/bufferbinding.h>



/**
* Handles rendering of tiles for editing visibility/blocking.
*/
class TileRenderer {
	friend class TileRenderHooks;
public:

	TileRenderer(gfx::MdfMaterialFactory &mdfFactory,
		gfx::RenderingDevice &device);
	~TileRenderer();

	static void Enable(bool enable);
	static bool IsEnabled();

private:
	void Render(size_t vertexCount,
		XMFLOAT4 *vertices,
		XMFLOAT4 *normals,
		XMCOLOR *diffuse,
		XMFLOAT2 *uv,
		size_t primCount,
		uint16_t *indices,
		int shaderId);

	gfx::MdfMaterialFactory &mMdfFactory;
	gfx::RenderingDevice &mDevice;

	gfx::MdfRenderMaterialPtr mMaterial;

	gfx::VertexBufferPtr mVertexBuffer;
	gfx::IndexBufferPtr mIndexBuffer;
	gfx::BufferBinding mBufferBinding;
};
