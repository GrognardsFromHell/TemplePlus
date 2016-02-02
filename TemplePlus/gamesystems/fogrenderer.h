
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

	void Render();

private:
	gfx::MdfMaterialFactory &mMdfFactory;
	gfx::RenderingDevice &mDevice;

	std::unique_ptr<gfx::Material> mMaterial;

	std::vector<uint8_t> mBlurredFog;
	size_t mBlurredFogWidth;
	size_t mBlurredFogHeight;
	
	gfx::DynamicTexturePtr mBlurredFogTexture;

	gfx::VertexBufferPtr mVertexBuffer;
	gfx::IndexBufferPtr mIndexBuffer;
	gfx::BufferBinding mBufferBinding;

	XMFLOAT2 mFogOrigin;

	uint32_t& mNumSubtilesX;
	uint32_t& mNumSubtilesY;

};
	