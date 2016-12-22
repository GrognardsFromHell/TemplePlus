#include "stdafx.h"

#include <graphics/dynamictexture.h>

#include "fogrenderer.h"
#include <temple/dll.h>
#include <common.h>

using namespace gfx;

#pragma pack(push, 1)
struct FogOfWarVertex {
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};
#pragma pack(pop)

FogOfWarRenderer::FogOfWarRenderer(MdfMaterialFactory& mdfFactory, RenderingDevice& device)
	: mMdfFactory(mdfFactory),
	  mDevice(device),
	  mNumSubtilesX(temple::GetRef<uint32_t>(0x10820458)),
	  mNumSubtilesY(temple::GetRef<uint32_t>(0x10824490)),
	  mBufferBinding(device.GetShaders().LoadVertexShader("fogofwar_vs")) {

	mBlurredFogWidth = (mNumSubtilesX / 4) * 4 + 8;
	mBlurredFogHeight = (mNumSubtilesY / 4) * 4 + 8;

	mBlurredFogTexture = mDevice.CreateDynamicTexture(BufferFormat::A8, mBlurredFogWidth, mBlurredFogHeight);
	mBlurredFog.resize(mBlurredFogWidth * mBlurredFogHeight);

	auto vs = device.GetShaders().LoadVertexShader("fogofwar_vs");
	auto ps = device.GetShaders().LoadPixelShader("fogofwar_ps");
	BlendSpec blendState;
	blendState.blendEnable = true;
	blendState.srcBlend = BlendOperand::SrcAlpha;
	blendState.destBlend = BlendOperand::InvSrcAlpha;
	DepthStencilSpec depthStencilState;
	depthStencilState.depthEnable = false;
	RasterizerSpec rasterizerState;
	SamplerSpec samplerState;
	samplerState.addressU = TextureAddress::Clamp;
	samplerState.addressV = TextureAddress::Clamp;
	samplerState.magFilter = TextureFilterType::Linear;
	samplerState.minFilter = TextureFilterType::Linear;
	std::vector<MaterialSamplerSpec> samplers{
		{ mBlurredFogTexture, samplerState }
	};

	mMaterial = std::make_unique<Material>(device.CreateMaterial(blendState, depthStencilState, rasterizerState, samplers, vs, ps));

	uint16_t indices[6]{0, 2, 1, 2, 0, 3};
	mIndexBuffer = device.CreateIndexBuffer(indices);
	mVertexBuffer = device.CreateEmptyVertexBuffer(sizeof(FogOfWarVertex) * 4);

	mBufferBinding.AddBuffer(mVertexBuffer, 0, sizeof(FogOfWarVertex))
	              .AddElement(VertexElementType::Float3, VertexElementSemantic::Position)
	              .AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);
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

void FogOfWarRenderer::Render() {

	static auto& sFoggingEnabled = temple::GetRef<uint32_t>(0x108254A0);
	if (!(sFoggingEnabled & 1)) {
		return;
	}

	gfx::PerfGroup perfGroup(mDevice, "Fog Of War");

	// Reset the blurred buffer
	eastl::fill(mBlurredFog.begin(), mBlurredFog.end(), 0);

	static auto sOpaquePattern = FogBlurKernel::Create(0xFF);
	static auto sHalfTransparentPattern = FogBlurKernel::Create(0xA0);

	static auto& fogCheckData = temple::GetRef<uint8_t*>(0x108A5498);

	// Get [0,0] of the subtile data
	auto fogCheckSubtile = &fogCheckData[0];

	for (size_t y = 0; y < mNumSubtilesY; y++) {
		for (size_t x = 0; x < mNumSubtilesX; x++) {
			auto fogState = *fogCheckSubtile;

			// Bit 2 -> Currently in LoS of the party
			if (!(fogState & 2)) {
				uint8_t* patternSrc;
				// Bit 3 -> Explored
				if (fogState & 4) {
					patternSrc = &sHalfTransparentPattern.patterns[x & 3][0][0];
				} else {
					patternSrc = &sOpaquePattern.patterns[x & 3][0][0];
				}

				// Now we copy 5 rows of 2 dwords each, to apply
				// the filter-kernel to the blurred fog map
				for (auto row = 0; row < 5; ++row) {
					auto src = (uint64_t*) &patternSrc[row * 8];
					auto roundedDownX = (x / 4) * 4;
					auto dest = (uint64_t*) &mBlurredFog[(y + row) * mBlurredFogWidth + roundedDownX];
					// Due to how the kernel is layed out, the individual bytes in this 8-byte addition will never carry
					// over to the next higher byte, thus this is equivalent to 8 separate 1-byte additions
					*dest += *src;
				}
			}
			++fogCheckSubtile;
		}

		// Skips the last subtile of the row and the first of the next row
		// fogCheckSubtile += 2;

	}

	static auto& fogMinX = temple::GetRef<uint64_t>(0x10824468);
	static auto& fogMinY = temple::GetRef<uint64_t>(0x108EC4C8);

	auto mFogOriginX = (uint32_t) fogMinX;
	auto mFogOriginY = (uint32_t) fogMinY;

	mBlurredFogTexture->Update<uint8_t>(gsl::as_span(&mBlurredFog[0], mBlurredFog.size()));

	// Use only the relevant subportion of the texture
	auto umin = 2.5f / (float)mBlurredFogWidth;
	auto vmin = 2.5f / (float)mBlurredFogHeight;
	auto umax = (mNumSubtilesX - 0.5f) / (float)mBlurredFogWidth;
	auto vmax = (mNumSubtilesY - 0.5f) / (float)mBlurredFogHeight;

	FogOfWarVertex mVertices[4];
	mVertices[0].pos.x = (mFogOriginX * 3) * INCH_PER_SUBTILE;
	mVertices[0].pos.y = 0;
	mVertices[0].pos.z = (mFogOriginY * 3) * INCH_PER_SUBTILE;
	mVertices[0].uv = {umin, vmin};

	mVertices[1].pos.x = (mFogOriginX * 3 + mNumSubtilesX) * INCH_PER_SUBTILE;
	mVertices[1].pos.y = 0;
	mVertices[1].pos.z = (mFogOriginY * 3) * INCH_PER_SUBTILE;
	mVertices[1].uv = {umax, vmin};

	mVertices[2].pos.x = (mFogOriginX * 3 + mNumSubtilesX) * INCH_PER_SUBTILE;
	mVertices[2].pos.y = 0;
	mVertices[2].pos.z = (mFogOriginY * 3 + mNumSubtilesY) * INCH_PER_SUBTILE;
	mVertices[2].uv = {umax, vmax};

	mVertices[3].pos.x = (mFogOriginX * 3) * INCH_PER_SUBTILE;
	mVertices[3].pos.y = 0;
	mVertices[3].pos.z = (mFogOriginY * 3 + mNumSubtilesY) * INCH_PER_SUBTILE;
	mVertices[3].uv = {umin, vmax};

	mVertexBuffer->Update<FogOfWarVertex>(mVertices);

	mBufferBinding.Bind();
	mDevice.SetIndexBuffer(*mIndexBuffer);

	mDevice.SetMaterial(*mMaterial);
	mDevice.SetVertexShaderConstant(0, StandardSlotSemantic::ViewProjMatrix);

	mDevice.DrawIndexed(gfx::PrimitiveType::TriangleList, 4, 6);

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

