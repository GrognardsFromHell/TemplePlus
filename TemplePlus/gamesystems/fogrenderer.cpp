#include "stdafx.h"

#include <graphics/dynamictexture.h>

#include "fogrenderer.h"
#include <util/fixes.h>
#include <temple/dll.h>
#include <common.h>

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

FogOfWarRenderer::FogOfWarRenderer(MdfMaterialFactory& mdfFactory,
                                   RenderingDevice& device) : mMdfFactory(mdfFactory), mDevice(device) {

	mBlurredFogTexture = mDevice.CreateDynamicTexture(D3DFMT_A8, 256, 256);

	auto vs = device.GetShaders().LoadVertexShader("fogofwar_vs");
	auto ps = device.GetShaders().LoadPixelShader("fogofwar_ps");
	BlendState blendState;
	blendState.blendEnable = true;
	blendState.srcBlend = D3DBLEND_SRCALPHA;
	blendState.destBlend = D3DBLEND_INVSRCALPHA;
	DepthStencilState depthStencilState;
	depthStencilState.depthEnable = false;
	RasterizerState rasterizerState;
	// rasterizerState.cullMode = D3DCULL_NONE;
	SamplerState samplerState;
	samplerState.addressU = D3DTADDRESS_CLAMP;
	samplerState.addressV = D3DTADDRESS_CLAMP;
	std::vector<MaterialSamplerBinding> samplers {
		MaterialSamplerBinding(mBlurredFogTexture, samplerState)
	};

	mMaterial = std::make_unique<Material>(blendState, depthStencilState, rasterizerState, samplers, vs, ps);

	mIndexBuffer = device.CreateEmptyIndexBuffer(mIndices.size());
	mVertexBuffer = device.CreateEmptyVertexBuffer(sizeof(FogOfWarVertex) * mVertices.size());

	mBufferBinding.AddBuffer(mVertexBuffer, 0, sizeof(FogOfWarVertex))
	              .AddElement(VertexElementType::Float3, VertexElementSemantic::Position)
	              .AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord)
	              .AddElement(VertexElementType::Color, VertexElementSemantic::Color);

	FogOfWarRendererHooks::renderer = this;
}

FogOfWarRenderer::~FogOfWarRenderer() {
	FogOfWarRendererHooks::renderer = nullptr;
}

void FogOfWarRenderer::Render() {
	static auto fog_of_war_render = temple::GetPointer<int()>(0x10030830);
	// fog_of_war_render();

	RenderNew();
}

void FogOfWarRenderer::Render(size_t vertexCount, XMFLOAT4* positions, XMCOLOR* diffuse, size_t primCount, uint16_t* indices) {

	auto vbLock = mVertexBuffer->Lock<FogOfWarVertex>(vertexCount);

	for (size_t i = 0; i < vertexCount; ++i) {
		auto& vertex = vbLock[i];
		vertex.pos.x = positions[i].x;
		vertex.pos.y = positions[i].y;
		vertex.pos.z = positions[i].z;

		vertex.diffuse = diffuse[i];
	}

	vbLock.Unlock();

	mIndexBuffer->Update({indices, primCount * 3});

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

// This calculates the highest power of two that is still less than w and h
static int GetFittingPowerOfTwoSquare(int w, int h) {
	auto dim = std::min<int>(w, h);

	for (auto i = dim & (dim - 1); i; i &= i - 1) {
		dim = i;
	}

	return dim;
}

void FogOfWarRenderer::DivideIntoSquares(int x, int y, int w, int h) {

	auto sideLength = GetFittingPowerOfTwoSquare(w, h);

	// Recursively handle the remainder of the rectangle defined by w,h
	if (sideLength < w) {
		DivideIntoSquares(x - sideLength, y + sideLength, w - sideLength, h);
	}
	if (sideLength < h) {
		// TODO: It seems odd that it would pass "sideLength" here for width
		DivideIntoSquares(x + sideLength, y + sideLength, sideLength, h - sideLength);
	}

	if (TesellateSquare(x, y, sideLength)) {
		AddUniformSquare(x, y, sideLength);
	}

}

bool FogOfWarRenderer::TesellateSquare(int x, int y, int sideLength) {

	if (sideLength > 1) {
		auto halfLength = sideLength / 2;

		// Recurse for each quadrant
		auto upperLeft = TesellateSquare(x, y, halfLength);
		auto upperRight = TesellateSquare(x - halfLength, halfLength + y, halfLength);
		auto lowerLeft = TesellateSquare(halfLength + x, halfLength + y, halfLength);
		auto lowerRight = TesellateSquare(x, y + 2 * halfLength, halfLength);
		if (upperRight && lowerLeft && lowerRight && upperLeft) {
			// Returns true if all quadrants returned true
			return true;
		}

		// Not all quadrants returned true, so we need to handle the ones that did,
		// because nothing has been rendered for them yet
		if (upperLeft) {
			AddUniformSquare(x, y, halfLength);
		}
		if (upperRight) {
			AddUniformSquare(x - halfLength, y + halfLength, halfLength);
		}
		if (lowerLeft) {
			AddUniformSquare(x + halfLength, y + halfLength, halfLength);
		}
		if (lowerRight) {
			AddUniformSquare(x, y + 2 * halfLength, halfLength);
		}
		return false;
	}

	auto fogTop = GetBlurredFog(x + 2, y + 2);
	auto fogLeft = GetBlurredFog(x + 1, y + 3);
	auto fogRight = GetBlurredFog(x + 3, y + 3);
	auto fogBottom = GetBlurredFog(x + 2, y + 4);
	auto fogCenter = GetBlurredFog(x + 2, y + 3);

	// True if all of them are mostly transparent (none has value > 7)
	// OR if the center is not different from any of the sides (tolerance of 3)
	if ((fogTop < 4 && fogRight < 4 && fogBottom < 4 && fogLeft < 4 && fogCenter < 4)
		|| (abs(fogTop - fogCenter) | abs(fogLeft - fogCenter) | abs(fogRight - fogCenter) | abs(fogBottom - fogCenter)) < 4) {
		return true;
	}

	// In the worst case we need 12 indices and 5 vertices
	if (mIndexCount + 12 > mIndices.size() || mVertexCount + 5 > mVertices.size()) {
		FlushBufferedTriangles();
	}

	// Top / Center / Bottom are nearly uniform
	if (abs(fogTop - fogCenter) < 4 && abs(fogBottom - fogCenter) < 4) {

		// Right / Center are nearly transparent
		if (fogRight < 4 && fogCenter < 4) {
			// One triangle for the left half
			AddTriangle(
				AddVertex(x, y, fogTop),				
				AddVertex(x - 1, y + 1, fogLeft),
				AddVertex(x, y + 2, fogBottom)
			);
			return false;
		}
		// Right / Center are nearly transparent
		if (fogLeft < 4 && fogCenter < 4) {
			// One triangle for the right half
			AddTriangle(
				AddVertex(x, y, fogTop),				
				AddVertex(x, y + 2, fogBottom),
				AddVertex(x + 1, y + 1, fogRight)
			);
			return false;
		}

		// Two triangles: Left half / right half
		auto rightIdx = AddVertex(x + 1, y + 1, fogRight);
		auto bottomIdx = AddVertex(x, y + 2, fogBottom);
		auto topIdx = AddVertex(x, y, fogTop);
		auto leftIdx = AddVertex(x - 1, y + 1, fogLeft);
		AddTriangle(topIdx, bottomIdx, rightIdx);
		AddTriangle(topIdx, leftIdx, bottomIdx);
		return false;
	}

	if (abs(fogLeft - fogCenter) < 4 && abs(fogRight - fogCenter) < 4) {

		// Top/Center are nearly transparent
		if (fogTop < 4 && fogCenter < 4) {
			// Triangle for the lower half
			AddTriangle(
				AddVertex(x - 1, y + 1, fogLeft),
				AddVertex(x, y + 2, fogBottom),
				AddVertex(x + 1, y + 1, fogRight)
			);
			return false;
		}

		// Bottom/Center are nearly transparent
		if (fogBottom < 4 && fogCenter < 4) {
			// Triangle for the upper half
			AddTriangle(
				AddVertex(x, y, fogTop),
				AddVertex(x - 1, y + 1, fogLeft),
				AddVertex(x + 1, y + 1, fogRight)
			);
			return false;
		}

		// Two triangles: upper half / lower half
		auto rightIdx = AddVertex(x + 1, y + 1, fogRight);
		auto bottomIdx = AddVertex(x, y + 2, fogBottom);
		auto topIdx = AddVertex(x, y, fogTop);
		auto leftIdx = AddVertex(x - 1, y + 1, fogLeft);
		AddTriangle(topIdx, leftIdx, rightIdx);
		AddTriangle(leftIdx, bottomIdx, rightIdx);
		return false;
	}

	// This is the worst case scenario in which we have to generate 4 triangles around the center
	auto rightIdx = AddVertex(x + 1, y + 1, fogRight);
	auto bottomIdx = AddVertex(x, y + 2, fogBottom);
	auto topIdx = AddVertex(x, y, fogTop);
	auto leftIdx = AddVertex(x - 1, y + 1, fogLeft);
	auto centerIdx = AddVertex(x, y + 1, fogCenter);
	AddTriangle(topIdx, leftIdx, centerIdx); // Upper left
	AddTriangle(leftIdx, bottomIdx, centerIdx); // Lower left
	AddTriangle(bottomIdx, rightIdx, centerIdx); // Lower right
	AddTriangle(rightIdx, topIdx, centerIdx); // Upper right

	return false;
}

void FogOfWarRenderer::AddUniformSquare(int x, int y, int sideLength) {

	// These coordinates basically assume a 45° rotated coordinate system
	auto fogTopLeft = GetBlurredFog(x + 2, y + 2);
	auto fogBottomRight = GetBlurredFog(x + 2, y + 2 + 2 * sideLength);
	auto fogTopRight = GetBlurredFog(x + 2 - sideLength, y + 2 + sideLength);
	auto fogBottomLeft = GetBlurredFog(x + 2 + sideLength, y + 2 + sideLength);

	if (fogTopLeft < 4 && fogTopRight < 4 && fogBottomLeft < 4 && fogBottomRight < 4) {
		return; // The quad is basically transparent
	}

	// Ensure enough space in the buffers
	if (mVertexCount + 4 > mVertices.size() || mIndexCount + 6 > mIndices.size()) {
		FlushBufferedTriangles();
	}

	// Add a quad to the buffer that covers the entire area
	auto topLeftIdx = AddVertex(x, y, fogTopLeft);
	auto topRightIdx = AddVertex(x - sideLength, y + sideLength, fogTopRight);
	auto bottomLeftIdx = AddVertex(x + sideLength, y + sideLength, fogBottomLeft);
	auto bottomRightIdx = AddVertex(x, y + 2 * sideLength, fogBottomRight);

	AddTriangle(topLeftIdx, bottomRightIdx, bottomLeftIdx);
	AddTriangle(topLeftIdx, topRightIdx, bottomRightIdx);
}

void FogOfWarRenderer::FlushBufferedTriangles() {

	if (mIndexCount == 0) {
		return; // Nothing to do
	}

	mIndexBuffer->Update({&mIndices[0], mIndexCount});
	mVertexBuffer->Update<FogOfWarVertex>({&mVertices[0], mVertexCount});

	mBufferBinding.Bind();
	mDevice.GetDevice()->SetIndices(mIndexBuffer->GetBuffer());

	mDevice.SetMaterial(*mMaterial);
	mDevice.SetVertexShaderConstant(0, StandardSlotSemantic::ViewProjMatrix);

	mDevice.GetDevice()->DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,
		0,
		0,
		mVertexCount,
		0,
		mIndexCount / 3);

	mIndexCount = 0;
	mVertexCount = 0;

}

uint8_t FogOfWarRenderer::GetBlurredFog(int x, int y) const {
	return mBlurredFog[y * sSubtilesPerRow + x];
}

uint16_t FogOfWarRenderer::GetVertexIdx(int x, int y) const {
	return mVertexIdx[y * sSubtilesPerRow + x];
}

void FogOfWarRenderer::SetVertexIdx(int x, int y, uint16_t vertexIdx) {
	mVertexIdx[y * sSubtilesPerRow + x] = vertexIdx;
}

bool FogOfWarRenderer::IsVertexForSubtile(uint16_t idx, int x, int y) const {
	return mVertexLocs[idx] == x * sSubtilesPerRow + y;
}

void FogOfWarRenderer::SetVertexForSubtile(uint16_t idx, int x, int y) {
	mVertexLocs[idx] = x * sSubtilesPerRow + y;
}

void FogOfWarRenderer::AddTriangle(uint16_t idx1, uint16_t idx2, uint16_t idx3) {
	mIndices[mIndexCount++] = idx1;
	mIndices[mIndexCount++] = idx2;
	mIndices[mIndexCount++] = idx3;
}

uint16_t FogOfWarRenderer::AddVertex(int x, int y, int alpha) {

	auto vertexIdx = GetVertexIdx(x, y);
	if (vertexIdx >= mVertexCount || !IsVertexForSubtile(vertexIdx, x, y)) {
		vertexIdx = (uint16_t) mVertexCount++;

		SetVertexIdx(x, y, vertexIdx);
		SetVertexForSubtile(vertexIdx, x, y);

		mVertices[vertexIdx].pos.x = (mFogOriginX * 3 + x) * INCH_PER_SUBTILE;
		mVertices[vertexIdx].pos.y = 0;
		mVertices[vertexIdx].pos.z = (mFogOriginY * 3 + y) * INCH_PER_SUBTILE;
		mVertices[vertexIdx].diffuse = alpha << 24;
	}

	return vertexIdx;

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
	static auto& sFoggingEnabled = temple::GetRef<uint32_t>(0x108254A0);
	if (!(sFoggingEnabled & 1)) {
		return;
	}

	static auto sOpaquePattern = FogBlurKernel::Create(0xFF);
	static auto sHalfTransparentPattern = FogBlurKernel::Create(0xA0);

	static auto& numSubtilesY = temple::GetRef<uint64_t>(0x10824490);
	static auto& fogCheckData = temple::GetRef<uint8_t*>(0x108A5498);

	memset(&mBlurredFog[0], 0, (size_t) numSubtilesY * sSubtilesPerRow);

	static auto& numSubtilesX = temple::GetRef<uint64_t>(0x10820458);

	// Get [1,1] of the subtile data
	auto fogCheckSubtile = &fogCheckData[numSubtilesX + 1];

	for (auto y = 1; y < numSubtilesY - 1; y++) {
		for (auto x = 1; x < numSubtilesX - 1; x++) {
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
					auto src = &patternSrc[row * 8];
					auto dest = &mBlurredFog[(y + row) * sSubtilesPerRow + (x / 4) * 4];
					// Due to how the kernel is layed out, the individual bytes in this 8-byte addition will never carry
					// over to the next higher byte, thus this is equivalent to 8 separate 1-byte additions
					*((uint64_t*)dest) += *((uint64_t*)src);
				}
			}
			++fogCheckSubtile;
		}

		// Skips the last subtile of the row and the first of the next row
		fogCheckSubtile += 2;

	}

	// Validation code
	/*
	auto blurredFogOld = temple::GetPointer<uint8_t>(0x108103C8);
	if (memcmp(&mBlurredFog[0], blurredFogOld, (size_t)numSubtilesY * sSubtilesPerRow)) {
		throw TempleException("Blah");
	}
	static auto render_fogging_related_0 = temple::GetPointer<void(int a1, uint8_t *aout)>(0x1002f020);
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

	static auto& fogMinX = temple::GetRef<uint64_t>(0x10824468);
	static auto& fogMinY = temple::GetRef<uint64_t>(0x108EC4C8);
	static auto& fogOriginXOrg = temple::GetRef<float>(0x108A549C);
	static auto& fogOriginYOrg = temple::GetRef<float>(0x10820460);

	mFogOriginX = (uint32_t) fogMinX;
	mFogOriginY = (uint32_t) fogMinY;
	mFogOrigin.x = fogMinX * INCH_PER_TILE;
	mFogOrigin.y = fogMinY * INCH_PER_TILE;

	/*if (fogOriginXOrg != mFogOrigin.x || fogOriginYOrg != mFogOrigin.y) {
		throw TempleException("Blah");
	}*/

	auto topLeft = mDevice.GetCamera().ScreenToTileLegacy(0, 0);
	auto topRight = mDevice.GetCamera().ScreenToTileLegacy((int) mDevice.GetCamera().GetScreenWidth(), 0);
	auto bottomLeft = mDevice.GetCamera().ScreenToTileLegacy(0, (int) mDevice.GetCamera().GetScreenHeight());

	static auto screen_to_world = temple::GetPointer<void(int64_t x, int64_t y, float* a3, float* a4)>(0x10029570);

	float xOut, yOut;
	screen_to_world(0, 0, &xOut, &yOut);
	if (abs(xOut - topLeft.x) > 0.01 || abs(yOut - topLeft.y) > 0.01) {
		throw TempleException("Blah");
	}

	screen_to_world((int)mDevice.GetCamera().GetScreenWidth(), 0, &xOut, &yOut);
	if (abs(xOut - topRight.x) > 0.01 || abs(yOut - topRight.y) > 0.01) {
		throw TempleException("Blah");
	}

	screen_to_world(0, (int)mDevice.GetCamera().GetScreenHeight(), &xOut, &yOut);
	if (abs(xOut - bottomLeft.x) > 0.01 || abs(yOut - bottomLeft.y) > 0.01) {
		throw TempleException("Blah");
	}

	auto subtileXTopLeft = ceilf(topLeft.x / INCH_PER_SUBTILE);
	auto subtileYTopLeft = floorf(topLeft.y / INCH_PER_SUBTILE);

	auto subtileScreenWidth = (int)(subtileXTopLeft - fogMinX * 3);
	auto subtileScreenHeight = (int)(subtileYTopLeft - fogMinY * 3);

	auto v13 = subtileXTopLeft - (topRight.x / INCH_PER_SUBTILE);
	auto v14 = topRight.y / INCH_PER_SUBTILE - subtileYTopLeft;
	if (v14 > v13)
		v13 = v14;
	v13 = ceilf(v13);

	auto v15 = (bottomLeft.x / INCH_PER_SUBTILE) - subtileXTopLeft;
	auto v16 = (bottomLeft.y / INCH_PER_SUBTILE) - subtileYTopLeft;
	if (v16 > v15)
		v15 = v16;
	v15 = ceilf(v15);

	auto w1 = ((int)v13 + 3) & 0xFFFFFFFC;
	auto w2 = ((int)v15 + 4) & 0xFFFFFFFC;

	if (subtileScreenWidth < 0) {
		w1 += subtileScreenWidth;
		subtileScreenWidth = 0;
		if (w1 <= 0) {
			return; // This really should not happen...
		}
	}

	if (subtileScreenHeight < 0) {
		w2 += subtileScreenHeight;
		subtileScreenHeight = 0;
		if (w2 <= 0) {
			return; // This really should not happen...
		}
	}

	// DivideIntoSquares(subtileScreenWidth, subtileScreenHeight, w1, w2);

	mBlurredFogTexture->Update<uint8_t>({ &mBlurredFog[0], mBlurredFog.size() });

	mVertices[0].pos.x = (mFogOriginX * 3) * INCH_PER_SUBTILE;
	mVertices[0].pos.y = 0;
	mVertices[0].pos.z = (mFogOriginY * 3) * INCH_PER_SUBTILE;
	mVertices[0].diffuse = 0xFFFFFFFF;
	mVertices[0].uv = { 0, 0 };

	mVertices[1].pos.x = (mFogOriginX * 3 + 256) * INCH_PER_SUBTILE;
	mVertices[1].pos.y = 0;
	mVertices[1].pos.z = (mFogOriginY * 3) * INCH_PER_SUBTILE;
	mVertices[1].diffuse = 0xFFFFFFFF;
	mVertices[1].uv = { 1, 0 };

	mVertices[2].pos.x = (mFogOriginX * 3 + 256) * INCH_PER_SUBTILE;
	mVertices[2].pos.y = 0;
	mVertices[2].pos.z = (mFogOriginY * 3 + 256) * INCH_PER_SUBTILE;
	mVertices[2].diffuse = 0xFFFFFFFF;
	mVertices[2].uv = { 1, 1 };

	mVertices[3].pos.x = (mFogOriginX * 3) * INCH_PER_SUBTILE;
	mVertices[3].pos.y = 0;
	mVertices[3].pos.z = (mFogOriginY * 3 + 256) * INCH_PER_SUBTILE;
	mVertices[3].diffuse = 0xFFFFFFFF;
	mVertices[3].uv = { 0, 1 };
	mVertexCount = 4;
	mIndexCount = 0;

	AddTriangle(0, 2, 1);
	AddTriangle(2, 0, 3);

	FlushBufferedTriangles();

	/*render_fogging_related_1(subtileScreenWidth, subtileScreenHeight, w1, w2);
	if (fog_vertex_count)
		render_fogging_triangles();*/

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

