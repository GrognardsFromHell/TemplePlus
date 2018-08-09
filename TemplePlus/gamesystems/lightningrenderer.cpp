
#include "stdafx.h"
#include "lightningrenderer.h"
#include <util/fixes.h>
#include <temple/dll.h>

using namespace gfx;

static class LightningRenderHooks : public TempleFix {
public:
	void RedirectChainLightningTargets();
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

		RedirectChainLightningTargets();
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

	ChainLightningTarget chainTargets[30];

} hooks;

LightningRenderer* LightningRenderHooks::renderer = nullptr;

#pragma pack(push, 1)
struct LightningVertex {
	XMFLOAT4 pos;
	XMFLOAT4 normal;
	XMCOLOR diffuse;
	XMFLOAT2 texCoord;
};
#pragma pack(pop)

LightningRenderer::LightningRenderer(MdfMaterialFactory& mdfFactory,
	RenderingDevice& device) : mMdfFactory(mdfFactory), mDevice(device), mBufferBinding(device.CreateMdfBufferBinding()) {

	mMaterial = mdfFactory.LoadMaterial("art/meshes/lightning.mdf");

	mIndexBuffer = device.CreateEmptyIndexBuffer(3588);
	mVertexBuffer = device.CreateEmptyVertexBuffer(sizeof(LightningVertex) * 1200);

	mBufferBinding.AddBuffer(mVertexBuffer, 0, sizeof(LightningVertex))
		.AddElement(VertexElementType::Float4, VertexElementSemantic::Position)
		.AddElement(VertexElementType::Float4, VertexElementSemantic::Normal)
		.AddElement(VertexElementType::Color, VertexElementSemantic::Color)
		.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);

	LightningRenderHooks::renderer = this;
}

LightningRenderer::~LightningRenderer() {
	LightningRenderHooks::renderer = nullptr;
}

void LightningRenderer::Render() {
	gfx::PerfGroup perfGroup(mDevice, "Lightning PFX");

	static auto render_pfx_lightning_related = temple::GetPointer<int()>(0x10087e60);
	render_pfx_lightning_related();
}

void LightningRenderer::Render(size_t vertexCount, XMFLOAT4* positions, XMFLOAT4* normals, XMCOLOR* diffuse, XMFLOAT2* uv, size_t primCount, uint16_t* indices) {

	if (!primCount)
		return;

	auto vbLock(mDevice.Map<LightningVertex>(*mVertexBuffer));
	
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

		vertex.diffuse = diffuse[i];

		vertex.texCoord = uv[i];
	}

	vbLock.Unmap();

	mIndexBuffer->Update(gsl::span(indices, primCount * 3));

	mBufferBinding.Bind();
	mDevice.SetIndexBuffer(*mIndexBuffer);

	mMaterial->Bind(mDevice, {});

	mDevice.DrawIndexed(
		gfx::PrimitiveType::TriangleList,
		vertexCount,
		primCount * 3);

}

void LightningRenderHooks::RedirectChainLightningTargets()
{
	int writeval;
	
	writeval = (int)&chainTargets[0].obj;
	write(0x1008866E + 1, &writeval, sizeof(void*));
	writeval = sizeof(int) + (int)&chainTargets[0].obj;
	write(0x10088673 + 2, &writeval, sizeof(void*));


	writeval = sizeof(int) + (int)&chainTargets[0].obj;
	write(0x1008868D + 1, &writeval, sizeof(void*));
	writeval = (int)&chainTargets[0].obj;
	write(0x10088692 + 2, &writeval, sizeof(void*));
	

	writeval = (int)&chainTargets[0].obj;
	write(0x100886B3 + 2, &writeval, sizeof(void*));
	writeval = sizeof(int) + (int)&chainTargets[0].obj;
	write(0x100886B9 + 2, &writeval, sizeof(void*));
	

	writeval = (int)&chainTargets[0].vec.x;
	write(0x100886BF + 2, &writeval, sizeof(void*));
	writeval = (int)&chainTargets[0].vec.y;
	write(0x100886DC + 2, &writeval, sizeof(void*));
	writeval = (int)&chainTargets[0].vec.z;
	write(0x100886EC + 2, &writeval, sizeof(void*));
	

	writeval = (int)&chainTargets[1];
	write(0x10088DAF + 1, &writeval, sizeof(void*));


	writeval = (int)&chainTargets[0].vec.z;
	write(0x100874FE + 1, &writeval, sizeof(void*));
	
}

int LightningRenderHooks::Render(int vertexCount, XMFLOAT4* vertices, XMFLOAT4* normals, XMCOLOR* diffuse, XMFLOAT2* uv, int primCount, uint16_t* indices, int shaderId) {
	if (renderer) {
		renderer->Render(vertexCount, vertices, normals, diffuse, uv, primCount, indices);
	}
	return 0;
}
