
#pragma once

#include <graphics/buffers.h>
#include <graphics/device.h>
#include <graphics/mdfmaterials.h>
#include <graphics/bufferbinding.h>

/**
* Handles rendering of such effects as chain lightning and lightning strike.
*/
class FogOfWarRenderer {
	friend class FogOfWarRendererHooks;
public:
	FogOfWarRenderer(gfx::MdfMaterialFactory &mdfFactory,
		gfx::RenderingDevice &device);
	~FogOfWarRenderer();

	void Render();

private:
	void Render(size_t vertexCount,
		XMFLOAT4 *positions,
		XMCOLOR *diffuse,
		size_t primCount,
		uint16_t *indices);

	void DivideIntoSquares(int a, int b, int c, int d);

	gfx::MdfMaterialFactory &mMdfFactory;
	gfx::RenderingDevice &mDevice;

	std::unique_ptr<gfx::Material> mMaterial;

	std::array<uint8_t, 0xFFFF> mBlurredFog;

	gfx::VertexBufferPtr mVertexBuffer;
	gfx::IndexBufferPtr mIndexBuffer;
	gfx::BufferBinding mBufferBinding;

	static constexpr auto sSubtilesPerRow = 256;

	XMFLOAT2 mFogOrigin;

	void RenderNew();

};
	