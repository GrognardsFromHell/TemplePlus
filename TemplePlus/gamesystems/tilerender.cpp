
#include "stdafx.h"
#include <util/fixes.h>
#include <temple/dll.h>
#include "tilerender.h"
#include <spell.h>


using namespace gfx;
static bool enabled_ = false;

static class TileRenderHooks : public TempleFix {
public:
	void apply() override {
		
		// RenderTileOverlays
		redirectCall(0x100AC066, Render);
		redirectCall(0x100AC0BF, Render);
		redirectCall(0x100AC1E6, Render);
		redirectCall(0x100AC30D, Render);
		redirectCall(0x100AC366, Render);
		redirectCall(0x100AC48D, Render);
		// SectorSvbOverlay
		redirectCall(0x100AAEE0, Render);
		redirectCall(0x100AAF1F, Render);
		redirectCall(0x100AAF5F, Render);
		redirectCall(0x100AAF9F, Render);
		redirectCall(0x100AB1BF, Render);
		redirectCall(0x100AB1FF, Render);
		redirectCall(0x100AB23F, Render);
		redirectCall(0x100AB27E, Render);
		redirectCall(0x100AB57F, Render);


	}

	static TileRenderer* renderer;

	static int Render(int vertexCount,
		XMFLOAT4 *vertices,
		XMFLOAT4 *normals,
		XMCOLOR *diffuse,
		XMFLOAT2 *uv,
		int primCount,
		uint16_t *indices,
		int shaderId);

} hooks;

TileRenderer* TileRenderHooks::renderer = nullptr;

#pragma pack(push, 1)
struct TileVertex {
	XMFLOAT4 pos;
	XMFLOAT4 normal;
	XMCOLOR diffuse;
	XMFLOAT2 texCoord;
};
#pragma pack(pop)

TileRenderer::TileRenderer(MdfMaterialFactory& mdfFactory,
	RenderingDevice& device) : mMdfFactory(mdfFactory), mDevice(device), mBufferBinding(device.CreateMdfBufferBinding()) {

	//mMaterial = mdfFactory.LoadMaterial("art\\interface\\intgame_select\\spell_none-target_hostile.mdf");
	mMaterial = mdfFactory.LoadMaterial("art\\interface\\intgame_select\\green-line_oc.mdf");
	

	mIndexBuffer = device.CreateEmptyIndexBuffer(300);
	mVertexBuffer = device.CreateEmptyVertexBuffer(sizeof(TileVertex) * 1200);

	mBufferBinding.AddBuffer(mVertexBuffer, 0, sizeof(TileVertex))
		.AddElement(VertexElementType::Float4, VertexElementSemantic::Position)
		.AddElement(VertexElementType::Float4, VertexElementSemantic::Normal)
		.AddElement(VertexElementType::Color, VertexElementSemantic::Color)
		.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);

	TileRenderHooks::renderer = this;
}

TileRenderer::~TileRenderer() {
	TileRenderHooks::renderer = nullptr;
}


void TileRenderer::Enable(bool enable){	
	enabled_ = enable;	
}

bool TileRenderer::IsEnabled()
{
	return enabled_;
}

void TileRenderer::Render(size_t vertexCount, XMFLOAT4* positions, XMFLOAT4* normals, XMCOLOR* diffuse, XMFLOAT2* uv, size_t primCount, uint16_t* indices, int shaderId) {

	auto vbLock(mDevice.Map<TileVertex>(*mVertexBuffer));

	for (size_t i = 0; i < vertexCount; ++i) {
		auto &vertex = vbLock[i];
		vertex.pos.x = positions[i].x;
		vertex.pos.y = positions[i].y;
		vertex.pos.z = positions[i].z;
		vertex.pos.w = 1;

		vertex.normal.x = normals[i].x;
		vertex.normal.y = normals[i].y;
		vertex.normal.z = normals[i].z;
		vertex.normal.w = 0;

		vertex.diffuse = 0xFFFFFFFF;

		vertex.texCoord = uv[i];
	}

	vbLock.Unmap();

	mIndexBuffer->Update(gsl::span(indices, primCount * 3));

	mBufferBinding.Bind();
	mDevice.SetIndexBuffer(*mIndexBuffer);

	mMaterial = mMdfFactory.GetById(shaderId);
	MdfRenderOverrides overrides;
	overrides.ignoreLighting = true;
	
	
	mMaterial->Bind(mDevice, {}, &overrides);

	mDevice.DrawIndexed(
		gfx::PrimitiveType::TriangleList,
		vertexCount,
		primCount * 3);

}

int TileRenderHooks::Render(int vertexCount, XMFLOAT4* vertices, XMFLOAT4* normals, XMCOLOR* diffuse, XMFLOAT2* uv, int primCount, uint16_t* indices, int shaderId) {
	if (!enabled_)
		return 0;

	if (renderer) {
		renderer->Render(vertexCount, vertices, normals, diffuse, uv, primCount, indices, shaderId);
	}
	return 0;
}
