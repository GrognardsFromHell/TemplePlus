
#include "stdafx.h"

#include "fogrenderer.h"
#include <util/fixes.h>
#include <temple/dll.h>

using namespace gfx;

static class FogOfWarRendererHooks : public TempleFix {
public:
	const char* name() override {
		return "Lightning Render Hooks";
	}
	void apply() override {
		replaceFunction(0x1002F180, Render);
	}

	static FogOfWarRenderer* renderer;

	static int Render();

} hooks;

FogOfWarRenderer* FogOfWarRendererHooks::renderer = nullptr;

#pragma pack(push, 1)
struct FogOfWarVertex {
	XMFLOAT3 pos;
	XMCOLOR diffuse;
};
#pragma pack(pop)

FogOfWarRenderer::FogOfWarRenderer(MdfMaterialFactory& mdfFactory,
	RenderingDevice& device) : mMdfFactory(mdfFactory), mDevice(device) {

	auto vs = device.GetShaders().LoadVertexShader("fogofwar_vs");
	auto ps = device.GetShaders().LoadPixelShader("fogofwar_ps");
	BlendState blendState;
	blendState.blendEnable = true;
	blendState.srcBlend = D3DBLEND_SRCALPHA;
	blendState.destBlend = D3DBLEND_INVSRCALPHA; 
	DepthStencilState depthStencilState;
	depthStencilState.depthEnable = false;
	RasterizerState rasterizerState;
	rasterizerState.cullMode = D3DCULL_NONE;
	std::vector<MaterialSamplerBinding> samplers;
	mMaterial = std::make_unique<Material>(blendState, depthStencilState, rasterizerState, samplers, vs, ps);

	mIndexBuffer = device.CreateEmptyIndexBuffer(0x1800);
	mVertexBuffer = device.CreateEmptyVertexBuffer(sizeof(FogOfWarVertex) * 0x400);

	mBufferBinding.AddBuffer(mVertexBuffer, 0, sizeof(FogOfWarVertex))
		.AddElement(VertexElementType::Float3, VertexElementSemantic::Position)
		.AddElement(VertexElementType::Color, VertexElementSemantic::Color);

	FogOfWarRendererHooks::renderer = this;
}

FogOfWarRenderer::~FogOfWarRenderer() {
	FogOfWarRendererHooks::renderer = nullptr;
}

void FogOfWarRenderer::Render() {
	static auto fog_of_war_render = temple::GetPointer<int()>(0x10030830);
	fog_of_war_render();

	RenderNew();
}

void FogOfWarRenderer::Render(size_t vertexCount, XMFLOAT4* positions, XMCOLOR *diffuse, size_t primCount, uint16_t* indices) {

	auto vbLock = mVertexBuffer->Lock<FogOfWarVertex>(vertexCount);

	for (size_t i = 0; i < vertexCount; ++i) {
		auto &vertex = vbLock[i];
		vertex.pos.x = positions[i].x;
		vertex.pos.y = positions[i].y;
		vertex.pos.z = positions[i].z;

		vertex.diffuse = diffuse[i];
	}

	vbLock.Unlock();

	mIndexBuffer->Update({ indices, primCount * 3 });

	mBufferBinding.Bind();
	mDevice.GetDevice()->SetIndices(mIndexBuffer->GetBuffer());

	mDevice.SetMaterial(*mMaterial);
	mDevice.SetVertexShaderConstant(0, StandardSlotSemantic::ViewProjMatrix);

	mDevice.GetDevice()->DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,
		0,
		0,
		vertexCount,
		0,
		primCount);

}

struct FogBlurKernel {
	// 4 kernels (shifted by 0,1,2,3 pixels right), each has
	// 5 rows with 8 columns (for shift-compensation)
	uint8_t patterns[4][5][8]; // [xShift][y][x]
	FogBlurKernel() {
		memset(patterns, 0, sizeof(patterns));
	}

	static FogBlurKernel Create(uint8_t totalPatternValue);
};

void FogOfWarRenderer::RenderNew() {
	static auto sOpaquePattern = FogBlurKernel::Create(0xFF);
	static auto sHalfTransparentPattern = FogBlurKernel::Create(0xA0);

	// Validation code
	/*static auto render_fogging_related_0 = temple::GetPointer<void(int a1, uint8_t *aout)>(0x1002f020);
	uint8_t opaqueRef[160];
	uint8_t transpRef[160];
	render_fogging_related_0(0xFF, &opaqueRef[0]);
	render_fogging_related_0(0xA0, &transpRef[0]);

	if (memcmp(transpRef, sHalfTransparentPattern.patterns, 160)) {
		throw TempleException("Blah");
	}
	if (memcmp(opaqueRef, sOpaquePattern.patterns, 160)) {
		throw TempleException("Blah");
	}*/

}

int FogOfWarRendererHooks::Render() {
	if (renderer) {
		static auto& positionsCount = temple::GetRef<uint32_t>(0x1080FA80);
		static auto positions = temple::GetPointer<XMFLOAT4>(0x10820468);
		static auto& indexCount = temple::GetRef<uint32_t>(0x108EC594);
		static auto indices = temple::GetPointer<uint16_t>(0x108E94C8);
		static auto diffuse = temple::GetPointer<XMCOLOR>(0x108244A0);

		renderer->Render(positionsCount, positions, diffuse, indexCount / 3, indices);

		positionsCount = 0;
		indexCount = 0;
	}
	return 0;
}

FogBlurKernel FogBlurKernel::Create(uint8_t totalPatternValue) {
	
	FogBlurKernel result;

	// Precompute 5x5 inverted blur kernel for distributing the total pattern value
	float weights[5][5];	
	auto weightSum = 0.0f;
	auto weightOut = &weights[0][0];
	for (auto y = -2; y <= 2; y++) {
		for (auto x = -2; x <= 2; x++) {
			auto strength = 3.0f - sqrtf((float)(x * x + y * y));
			*weightOut++ = strength;
			weightSum += strength;
		}
	}

	// Now use the computed weights to calculate the actual pattern based on the requested total value
	auto patternSum = 0;
	
	for (auto y = 0; y < 5; ++y) {
		for (auto x = 0; x < 5; ++x) {
			auto pixelValue = (uint8_t)(weights[y][x] / weightSum * totalPatternValue);
			result.patterns[0][y][x] = pixelValue;
			patternSum += pixelValue;
		}
	}

	// This assigns any remainder of the total pattern value (loss due to rounding down above)
	// to the center pixel of the 5x5 kernel
	result.patterns[0][2][2] += totalPatternValue - patternSum;

	// Now create 3 versions of the pattern that are shifted by 1-3 pixels to the right
	// This is for optimization reasons since addition is done using 32-bit (4 bytes) addition
	// and the resulting patterns are in reality 8 pixels wide
	for (auto xShift = 1; xShift <= 3; ++xShift) {
		for (auto y = 0; y < 5; ++y) {
			for (auto x = 0; x < 5; ++x) {
				// Output here is shifted by {1,2,3} pixels right
				result.patterns[xShift][y][x + xShift] = result.patterns[0][y][x];
			}
		}
	}	

	return result;
}
