
#include "stdafx.h"
#include <util/fixes.h>
#include <temple/dll.h>
#include "ui_intgame_renderer.h"
#include <spell.h>


using namespace gfx;


static class IntgameRenderHooks : public TempleFix {
public:
	void apply() override {
		redirectCall(0x10108E7A, Render);
		redirectCall(0x101DCDAD, Render);
		redirectCall(0x101DCE38, Render);

		// hooks for the render calls for the path preview in combat
		redirectCall(0x10107524, Render);
		redirectCall(0x10107547, Render);

		// Aoo Intercept Arrow
		redirectCall(0x10109287, Render);
	}

	static IntgameRenderer* renderer;

	static int Render(int vertexCount,
		XMFLOAT4 *vertices,
		XMFLOAT4 *normals,
		XMCOLOR *diffuse,
		XMFLOAT2 *uv,
		int primCount,
		uint16_t *indices,
		int shaderId);

} hooks;

IntgameRenderer* IntgameRenderHooks::renderer = nullptr;

#pragma pack(push, 1)
struct IntgameVertex {
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMCOLOR diffuse;
	XMFLOAT2 texCoord;
};
#pragma pack(pop)

IntgameRenderer::IntgameRenderer(MdfMaterialFactory& mdfFactory,
	RenderingDevice& device) : mMdfFactory(mdfFactory), mDevice(device) {

	mMaterial = mdfFactory.LoadMaterial("art\\interface\\intgame_select\\spell_none-target_hostile.mdf");

	mIndexBuffer = device.CreateEmptyIndexBuffer(300);
	mVertexBuffer = device.CreateEmptyVertexBuffer(sizeof(IntgameVertex) * 1200);

	mBufferBinding.AddBuffer(mVertexBuffer, 0, sizeof(IntgameVertex))
		.AddElement(VertexElementType::Float3, VertexElementSemantic::Position)
		.AddElement(VertexElementType::Float3, VertexElementSemantic::Normal)
		.AddElement(VertexElementType::Color, VertexElementSemantic::Color)
		.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);

	IntgameRenderHooks::renderer = this;
}

IntgameRenderer::~IntgameRenderer() {
	IntgameRenderHooks::renderer = nullptr;
}

void IntgameRenderer::Render() {
	//static auto render_pfx_lightning_related = temple::GetPointer<int()>(0x10087e60);
	//render_pfx_lightning_related();
}

int IntgameRenderer::LoadIntgameShader(int spellEnum, int tgtResultClassif, int* shaderOut, int isFogged)
{
	//// incomplete stub
	//const char * spellEnumName = spellSys.GetSpellEnumNameFromEnum(spellEnum);
	//char filename[260];

	//std::string ocString = "_oc";
	//if (!isFogged)
	//{
	//	ocString = "";
	//}

	//
	//int shaderGetError = 0;

	//if (spellEnumName)
	//{
	//	_snprintf(filename, 260, "art\\interface\\intgame_select\\%s-%s%s.mdf", spellEnumName, intgamePickerTargetResultOutcomes[tgtResultClassif] , ocString.c_str());
	//	//RegisterShader
	//	//auto shaderGetError = mMdfFactory.LoadMaterial(filename);
	//}

	return 0;
}

void IntgameRenderer::Render(size_t vertexCount, XMFLOAT4* positions, XMFLOAT4* normals, XMCOLOR* diffuse, XMFLOAT2* uv, size_t primCount, uint16_t* indices, int shaderId) {

	auto vbLock = mVertexBuffer->Lock<IntgameVertex>(vertexCount);

	for (size_t i = 0; i < vertexCount; ++i) {
		auto &vertex = vbLock[i];
		vertex.pos.x = positions[i].x;
		vertex.pos.y = positions[i].y;
		vertex.pos.z = positions[i].z;

		vertex.normal.x = normals[i].x;
		vertex.normal.y = normals[i].y;
		vertex.normal.z = normals[i].z;

		vertex.diffuse = 0xFFFFFFFF;

		vertex.texCoord = uv[i];
	}

	vbLock.Unlock();

	mIndexBuffer->Update(gsl::as_span(indices, primCount * 3));

	mBufferBinding.Bind();
	mDevice.GetDevice()->SetIndices(mIndexBuffer->GetBuffer());

	mMaterial = mMdfFactory.GetById(shaderId);

	mMaterial->Bind(mDevice, {});

	mDevice.GetDevice()->DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,
		0,
		0,
		vertexCount,
		0,
		primCount);

}

int IntgameRenderHooks::Render(int vertexCount, XMFLOAT4* vertices, XMFLOAT4* normals, XMCOLOR* diffuse, XMFLOAT2* uv, int primCount, uint16_t* indices, int shaderId) {
	if (renderer) {
		renderer->Render(vertexCount, vertices, normals, diffuse, uv, primCount, indices, shaderId);
	}
	return 0;
}
