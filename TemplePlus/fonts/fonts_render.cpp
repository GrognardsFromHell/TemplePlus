
#include "stdafx.h"

#include <graphics/device.h>
#include <graphics/shaders.h>
#include <graphics/materials.h>
#include <graphics/bufferbinding.h>

#include "tig/tig.h"
#include "tig/tig_font.h"
#include "fonts.h"

using namespace gsl;
using namespace gfx;

// First character found in the FNT files
constexpr auto FirstFontChar = '!';

#pragma pack(push, 1)
struct GlyphVertex2d {
	float u;
	float v;
	float x;
	float y;
	XMCOLOR diffuse;
};

struct GlyphVertex3d {
	XMFLOAT3 pos;
	XMCOLOR diffuse;
	XMFLOAT2 uv;
};
#pragma pack(pop)

static void ConvertVertex(const GlyphVertex2d& vertex2d, GlyphVertex3d& vertex3d, const gfx::Size& textureSize) {
	vertex3d.pos.x = vertex2d.x;
	vertex3d.pos.y = vertex2d.y;
	vertex3d.pos.z = 0.5f;

	vertex3d.diffuse = vertex2d.diffuse;

	vertex3d.uv.x = vertex2d.u / textureSize.width;
	vertex3d.uv.y = vertex2d.v / textureSize.height;
}

static constexpr auto sMaxGlyphFiles = 4;

struct GlyphFileState {
	static constexpr auto MaxGlyphs = 800;
	int glyphCount = 0;
	std::array<GlyphVertex2d, MaxGlyphs * 4> vertices;
};

struct FontRenderer::Impl : public ResourceListener {
	
	Impl(RenderingDevice &device) 
		: mDevice(device), mRegistration(device, this), mMaterial(CreateMaterial(device)), mBufferBinding(mMaterial.GetVertexShader()) {
	}
	
	void CreateResources(RenderingDevice&) override;
	void FreeResources(RenderingDevice&) override;

	RenderingDevice& mDevice;
	IndexBufferPtr mIndexBuffer;
	GlyphFileState mFileState[sMaxGlyphFiles];

	Material mMaterial;

	BufferBinding mBufferBinding;

	ResourceListenerRegistration mRegistration;

	static Material CreateMaterial(RenderingDevice &device);

};

Material FontRenderer::Impl::CreateMaterial(RenderingDevice &device) {

	BlendSpec blendState;
	blendState.blendEnable = true;
	blendState.srcBlend = BlendOperand::SrcAlpha;
	blendState.destBlend = BlendOperand::InvSrcAlpha;

	DepthStencilSpec depthStencilState;
	depthStencilState.depthEnable = false;
			
	std::vector<MaterialSamplerSpec> samplers {
		{ nullptr, {} }
	};

	auto vertexShader(device.GetShaders().LoadVertexShader("font_vs"));
	auto pixelShader(device.GetShaders().LoadPixelShader("textured_simple_ps"));

	return device.CreateMaterial(
		blendState,
		depthStencilState,
		{},
		samplers,
		vertexShader,
		pixelShader
	);
}

FontRenderer::FontRenderer(RenderingDevice& device)
	: mImpl(std::make_unique<Impl>(device)) {
}

FontRenderer::~FontRenderer() {
}

void FontRenderer::RenderRun(cstring_span<> text,
                             int x,
                             int y,
                             const TigRect& bounds,
                             TigTextStyle& style,
                             const TigFont& font) {
	for (auto& state : mImpl->mFileState) {
		state.glyphCount = 0;
	}

	for (auto it = text.begin(); it != text.end(); ++it) {
		auto ch = *it;
		auto nextCh = '\0';
		if (it + 1 != text.end()) {
			nextCh = *(it + 1);
		}

		// @0 to @9 select one of the text colors
		if (ch == '@' && isdigit(nextCh)) {
			++it; // Skip the digit
			style.colorSlot = nextCh - '0';
			continue;
		}

		// Handle @t tabstop movement
		if (ch == '@' && nextCh == 't') {
			auto tabWidth = style.field4c - bounds.x;

			if (tabWidth > 0) {
				++it; // Skip the t

				auto tabCount = 1 + (x - bounds.x) / tabWidth;
				x = bounds.x + tabCount * tabWidth;
			}
			continue;
		}

		auto glyphIdx = *it - FirstFontChar;
		if (glyphIdx > font.glyphCount) {
			return; // Trying to render invalid character
		}

		if (isspace(ch)) {
			x += style.tracking;
			continue;
		}

		auto glyph = font.glyphs[glyphIdx];

		float u1 = (float) glyph.rect.x;
		float v1 = (float) glyph.rect.y;
		float u2 = (float) glyph.rect.x + glyph.rect.width;
		float v2 = (float) glyph.rect.y + glyph.rect.height;

		auto& state = mImpl->mFileState[glyph.fileIdx];

		TigRect destRect;
		destRect.x = x;
		destRect.y = y + font.baseline - glyph.base_line_y_offset;
		destRect.width = glyph.rect.width;
		destRect.height = glyph.rect.height;

		x += style.kerning + glyph.width_line;

		Expects(!(style.flags & 0x1000));
		Expects(!(style.flags & 0x2000));

		// Drop Shadow
		if (style.flags & 8) {
			auto vertexIdx = state.glyphCount * 4;
			auto shadowColor = 0xFF000000 | style.shadowColor->topLeft;

			// Top Left
			auto& vertexTL = state.vertices[vertexIdx];
			vertexTL.x = destRect.x + 1.0f;
			vertexTL.y = destRect.y + 1.0f;
			vertexTL.u = u1;
			vertexTL.v = v1;
			vertexTL.diffuse = shadowColor;

			// Top Right
			auto& vertexTR = state.vertices[vertexIdx + 1];
			vertexTR.x = destRect.x + destRect.width + 1.0f;
			vertexTR.y = destRect.y + 1.0f;
			vertexTR.u = u2;
			vertexTR.v = v1;
			vertexTR.diffuse = shadowColor;

			// Bottom Right
			auto& vertexBR = state.vertices[vertexIdx + 2];
			vertexBR.x = destRect.x + destRect.width + 1.0f;
			vertexBR.y = destRect.y + destRect.height + 1.0f;
			vertexBR.u = u2;
			vertexBR.v = v2;
			vertexBR.diffuse = shadowColor;

			// Bottom Left
			auto& vertexBL = state.vertices[vertexIdx + 3];
			vertexBL.x = destRect.x + 1.0f;
			vertexBL.y = destRect.y + destRect.height + 1.0f;
			vertexBL.u = u1;
			vertexBL.v = v2;
			vertexBL.diffuse = shadowColor;

			state.glyphCount++;

			if (state.glyphCount >= GlyphFileState::MaxGlyphs) {
				RenderGlyphs(&state.vertices[0], font.textureIds[glyph.fileIdx], state.glyphCount);
				state.glyphCount = 0;
			}
		}

		auto vertexIdx = state.glyphCount * 4;
		const auto& colorRect = style.textColor[style.colorSlot];

		// Top Left
		auto& vertexTL = state.vertices[vertexIdx];
		vertexTL.x = (float)destRect.x;
		vertexTL.y = (float)destRect.y;
		vertexTL.u = u1;
		vertexTL.v = v1;
		vertexTL.diffuse = colorRect.topLeft;

		// Top Right
		auto& vertexTR = state.vertices[vertexIdx + 1];
		vertexTR.x = (float)destRect.x + destRect.width;
		vertexTR.y = (float)destRect.y;
		vertexTR.u = u2;
		vertexTR.v = v1;
		vertexTR.diffuse = colorRect.topRight;

		// Bottom Right
		auto& vertexBR = state.vertices[vertexIdx + 2];
		vertexBR.x = (float)destRect.x + destRect.width;
		vertexBR.y = (float)destRect.y + destRect.height;
		vertexBR.u = u2;
		vertexBR.v = v2;
		vertexBR.diffuse = colorRect.bottomRight;

		// Bottom Left
		auto& vertexBL = state.vertices[vertexIdx + 3];
		vertexBL.x = (float)destRect.x;
		vertexBL.y = (float)destRect.y + destRect.height;
		vertexBL.u = u1;
		vertexBL.v = v2;
		vertexBL.diffuse = colorRect.bottomLeft;

		// Support rotations (i.e. for the radial menu)
		if (style.flags & TTSF_ROTATE) {
			float rotCenterX, rotCenterY;
			if (style.flags & TTSF_ROTATE_OFF_CENTER) {
				rotCenterX = style.rotationCenterX;
				rotCenterY = style.rotationCenterY;
			} else {
				rotCenterX = (float)bounds.x;
				rotCenterY = (float)bounds.y + font.baseline;
			}

			auto rotCos = cosf(style.rotation);
			auto rotSin = sinf(style.rotation);
			Rotate2d(vertexTL.x, vertexTL.y, rotCos, rotSin, rotCenterX, rotCenterY);
			Rotate2d(vertexTR.x, vertexTR.y, rotCos, rotSin, rotCenterX, rotCenterY);
			Rotate2d(vertexBR.x, vertexBR.y, rotCos, rotSin, rotCenterX, rotCenterY);
			Rotate2d(vertexBL.x, vertexBL.y, rotCos, rotSin, rotCenterX, rotCenterY);
		}

		state.glyphCount++;

		if (state.glyphCount >= GlyphFileState::MaxGlyphs) {
			RenderGlyphs(&state.vertices[0], font.textureIds[glyph.fileIdx], state.glyphCount);
			state.glyphCount = 0;
		}
	}

	// Flush the remaining state
	for (auto i = 0; i < sMaxGlyphFiles; ++i) {
		auto& state = mImpl->mFileState[i];
		if (state.glyphCount > 0) {
			RenderGlyphs(&state.vertices[0], font.textureIds[i], state.glyphCount);
		}
	}

}

void FontRenderer::RenderGlyphs(const GlyphVertex2d* vertices2d, int textureId, int glyphCount) {
	auto texture = mImpl->mDevice.GetTextures().GetById(textureId);

	mImpl->mDevice.SetVertexShaderConstant(0, gfx::StandardSlotSemantic::UiProjMatrix);
	mImpl->mDevice.SetMaterial(mImpl->mMaterial);
	mImpl->mDevice.SetTexture(0, *texture);
	
	auto textureSize = texture->GetSize();

	const auto vertexCount = glyphCount * 4;

	eastl::fixed_vector<GlyphVertex3d, 32> vertices3d;
	vertices3d.reserve(vertexCount);

	for (int i = 0; i < vertexCount; i++) {
		auto &vertex3d = vertices3d.push_back();
		ConvertVertex(vertices2d[i], vertex3d, textureSize);
	}
		
	const auto bufferSize = sizeof(GlyphVertex3d) * vertexCount;

	auto buffer = mImpl->mDevice.CreateVertexBuffer<GlyphVertex3d>(vertices3d);

	mImpl->mBufferBinding
		.SetBuffer(0, buffer)
		.Bind();
	mImpl->mDevice.SetIndexBuffer(*mImpl->mIndexBuffer);

	mImpl->mDevice.DrawIndexed(gfx::PrimitiveType::TriangleList, vertexCount, glyphCount * 2 * 3);
	
}

void FontRenderer::Rotate2d(float& x, float& y, float rotCos, float rotSin, float centerX, float centerY) {
	auto newX = centerX + rotCos * (x - centerX) - rotSin * (y - centerY);
	auto newY = centerY + rotSin * (x - centerX) + rotCos * (y - centerY);
	x = newX;
	y = newY;
}

void FontRenderer::Impl::CreateResources(RenderingDevice& g) {

	auto vertexIdx = 0;
	std::array<uint16_t, GlyphFileState::MaxGlyphs * 6> indicesData;
	int j = 0;
	for (auto i = 0; i < GlyphFileState::MaxGlyphs; ++i) {
		// Counter clockwise quad rendering
		indicesData[j++] = vertexIdx + 0;
		indicesData[j++] = vertexIdx + 1;
		indicesData[j++] = vertexIdx + 2;
		indicesData[j++] = vertexIdx + 0;
		indicesData[j++] = vertexIdx + 2;
		indicesData[j++] = vertexIdx + 3;
		vertexIdx += 4;
	}

	mIndexBuffer = g.CreateIndexBuffer(indicesData);
	
	mBufferBinding.AddBuffer(nullptr, 0, sizeof(GlyphVertex3d))
		.AddElement(VertexElementType::Float3, VertexElementSemantic::Position)
		.AddElement(VertexElementType::Color, VertexElementSemantic::Color)
		.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);

}

void FontRenderer::Impl::FreeResources(RenderingDevice&) {
	mIndexBuffer = nullptr;
}
