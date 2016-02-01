
#pragma once

#include <graphics/buffers.h>
#include <graphics/device.h>
#include <graphics/mdfmaterials.h>
#include <graphics/bufferbinding.h>

#pragma pack(push, 1)
struct FogOfWarVertex {
	XMFLOAT3 pos;
	XMCOLOR diffuse;
};
#pragma pack(pop)

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

	void DivideIntoSquares(int x, int y, int w, int h);
	bool TesellateSquare(int x, int y, int sideLength);
	void AddUniformSquare(int x, int y, int sideLength);
	void FlushBufferedTriangles();

	gfx::MdfMaterialFactory &mMdfFactory;
	gfx::RenderingDevice &mDevice;

	std::unique_ptr<gfx::Material> mMaterial;

	std::array<uint8_t, 0xFFFF> mBlurredFog;
	std::array<uint16_t, 0xFFFF> mVertexIdx;
	std::array<uint16_t, 0x400> mVertexLocs;
	std::array<FogOfWarVertex, 0x400> mVertices;
	std::array<uint16_t, 0x800> mIndices;
	uint8_t GetBlurredFog(int x, int y) const;
	uint16_t GetVertexIdx(int x, int y) const;
	void SetVertexIdx(int x, int y, uint16_t vertexIdx);
	bool IsVertexForSubtile(uint16_t idx, int x, int y) const;
	void SetVertexForSubtile(uint16_t idx, int x, int y);

	uint16_t AddVertex(int x, int y, int alpha);
	void AddTriangle(uint16_t idx1, uint16_t idx2, uint16_t idx3);
	
	size_t mVertexCount = 0;
	size_t mIndexCount = 0;

	gfx::VertexBufferPtr mVertexBuffer;
	gfx::IndexBufferPtr mIndexBuffer;
	gfx::BufferBinding mBufferBinding;

	static constexpr auto sSubtilesPerRow = 256;

	XMFLOAT2 mFogOrigin;

	void RenderNew();

};
	