
#include "stdafx.h"
#include "lightningrenderer.h"
#include <util/fixes.h>
#include <temple/dll.h>

using namespace gfx;

static class LightningRenderHooks : public TempleFix {
public:
	const char* name() override {
		return "Lightning Render Hooks";
	}
	void apply() override {
		redirectCall(0x10088147, Render);
		redirectCall(0x100882FC, Render);
		redirectCall(0x100885F5, Render);
		redirectCall(0x10088A43, Render);
		redirectCall(0x10088D75, Render);
		redirectCall(0x100891BB, Render);
		redirectCall(0x100894FB, Render);
		redirectCall(0x10089933, Render);
		redirectCall(0x10089C96, Render);
		redirectCall(0x10089EBC, Render);
	}

	static LightningRenderer* renderer;

	static int Render(int vertexCount,
		XMFLOAT4 *vertices,
		XMFLOAT4 *normals,
		XMCOLOR *diffuse,
		XMFLOAT2 *uv,
		int primCount,
		uint16_t *indices,
		int shaderId);

} hooks;

LightningRenderer* LightningRenderHooks::renderer = nullptr;

#pragma pack(push, 1)
struct LightningVertex {
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMCOLOR diffuse;
	XMFLOAT2 texCoord;
};
#pragma pack(pop)

LightningRenderer::LightningRenderer(MdfMaterialFactory& mdfFactory,
	RenderingDevice& device) : mMdfFactory(mdfFactory), mDevice(device) {

	mMaterial = mdfFactory.LoadMaterial("art/meshes/lightning.mdf");

	mIndexBuffer = device.CreateEmptyIndexBuffer(3588);
	mVertexBuffer = device.CreateEmptyVertexBuffer(sizeof(LightningVertex) * 1200);

	mBufferBinding.AddBuffer(mVertexBuffer, 0, sizeof(LightningVertex))
		.AddElement(VertexElementType::Float3, VertexElementSemantic::Position)
		.AddElement(VertexElementType::Float3, VertexElementSemantic::Normal)
		.AddElement(VertexElementType::Color, VertexElementSemantic::Color)
		.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);

	LightningRenderHooks::renderer = this;
}

LightningRenderer::~LightningRenderer() {
	LightningRenderHooks::renderer = nullptr;
}

void LightningRenderer::Render() {
	static auto render_pfx_lightning_related = temple::GetPointer<int()>(0x10087e60);
	render_pfx_lightning_related();
}

void LightningRenderer::Render(size_t vertexCount, XMFLOAT4* positions, XMFLOAT4* normals, XMCOLOR* diffuse, XMFLOAT2* uv, size_t primCount, uint16_t* indices) {

	auto vbLock = mVertexBuffer->Lock<LightningVertex>(vertexCount);
	
	for (size_t i = 0; i < vertexCount; ++i) {
		auto &vertex = vbLock[i];
		vertex.pos.x = positions[i].x;
		vertex.pos.y = positions[i].y;
		vertex.pos.z = positions[i].z;

		vertex.normal.x = normals[i].x;
		vertex.normal.y = normals[i].y;
		vertex.normal.z = normals[i].z;

		vertex.diffuse = diffuse[i];

		vertex.texCoord = uv[i];
	}

	vbLock.Unlock();

	mIndexBuffer->Update({ indices, primCount * 3 });

	mBufferBinding.Bind();
	mDevice.GetDevice()->SetIndices(mIndexBuffer->GetBuffer());

	mMaterial->Bind(mDevice, {});

	mDevice.GetDevice()->DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,
		0,
		0,
		vertexCount,
		0,
		primCount);

}

int LightningRenderHooks::Render(int vertexCount, XMFLOAT4* vertices, XMFLOAT4* normals, XMCOLOR* diffuse, XMFLOAT2* uv, int primCount, uint16_t* indices, int shaderId) {
	if (renderer) {
		renderer->Render(vertexCount, vertices, normals, diffuse, uv, primCount, indices);
	}
	return 0;
}
