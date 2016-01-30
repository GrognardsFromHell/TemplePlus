#pragma once

#include <graphics/buffers.h>
#include <graphics/device.h>
#include <graphics/mdfmaterials.h>
#include <graphics/bufferbinding.h>


enum class IntgameRenderState {
	DontRender,
	StartRender,
	Rendering
};

/**
* Handles rendering of intgame ui elements such as the path preview and spell targeting circles.
*/
class IntgameRenderer {
	friend class IntgameRenderHooks;
public:

	std::string intgamePickerTargetResultOutcomes[8];

	IntgameRenderer(gfx::MdfMaterialFactory &mdfFactory,
		gfx::RenderingDevice &device);
	~IntgameRenderer();

	void Render();
	int LoadIntgameShader(int spellEnum, int tgtResultClassif, int* shaderOut, int isFogged);

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

	IntgameRenderState mIntgameState = IntgameRenderState::DontRender;
	uint32_t mIntgameStart = 0;

	gfx::VertexBufferPtr mVertexBuffer;
	gfx::IndexBufferPtr mIndexBuffer;
	gfx::BufferBinding mBufferBinding;
};
