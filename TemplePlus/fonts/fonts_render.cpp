#include <stdafx.h>

#include <graphics/graphics.h>
#include <infrastructure/renderstates.h>
#include <infrastructure/textures.h>
#include <graphics/render_hooks.h>

#include "util/fixes.h"
#include "tig/tig.h"
#include "tig/tig_font.h"

using namespace gsl;

// First character found in the FNT files
constexpr auto FirstFontChar = '!';

#pragma pack(push, 1)
struct GlyphVertex2d {
	float u;
	float v;
	float x;
	float y;
	D3DCOLOR diffuse;
};

struct GlyphVertex3d {
	D3DXVECTOR4 pos;
	D3DCOLOR diffuse;
	D3DXVECTOR2 uv;
};
#pragma pack(pop)

static void ConvertVertex(const GlyphVertex2d& vertex2d, GlyphVertex3d& vertex3d, const gfx::Size& textureSize) {
	vertex3d.pos.x = vertex2d.x;
	vertex3d.pos.y = vertex2d.y;
	vertex3d.pos.z = 0.5f;
	vertex3d.pos.w = 1.0f;

	vertex3d.diffuse = vertex2d.diffuse;

	vertex3d.uv.x = vertex2d.u / textureSize.width;
	vertex3d.uv.y = vertex2d.v / textureSize.height;
}


#if 0
int FontDrawDoWork(TigTextStyle* style, const char*& text, const TigFontDrawArgs& args, const TigFont& font) {

// signed int __usercall tig_font_draw_dowork@<eax>(tig_text_style *style@<ecx>, const char **text, TigFontDrawArgs *args, tig_font *font)
	auto func = temple::GetPointer<void*>(0x101E93E0);

	__asm {
		push ecx;
		mov ecx, style;
		push font;
		push args;
		push text;
		call func;
		add esp, 0xC;
		pop ecx;
	}

// TODO
	return 0;
}

int FontDrawDoWork0(const GlyphVertex2d* vertices, int textureId, signed int glyphCount) {

// signed int __usercall tig_font_draw_dowork@<eax>(tig_text_style *style@<ecx>, const char **text, TigFontDrawArgs *args, tig_font *font)
	auto func = temple::GetPointer<void*>(0x101E8790);
	__asm {
		push ebx;
		push ecx;
		mov ecx, textureId;
		mov eax, vertices;
		mov ebx, glyphCount;
		call func;
		pop ecx;
		pop ebx;
	}

// TODO
	return 0;
}
#endif

static constexpr auto sMaxGlyphFiles = 4;

struct GlyphFileState {
	static constexpr auto MaxGlyphs = 800;
	int glyphCount = 0;
	std::array<GlyphVertex2d, MaxGlyphs * 4> vertices;
};

#include "fonts.h"

struct FontRenderer::Impl : public ResourceListener {
	
	Impl(Graphics &g) : mRegistration(g, this) {
	}
	
	void CreateResources(Graphics&) override;
	void FreeResources(Graphics&) override;

	CComPtr<IDirect3DIndexBuffer9> mIndexBuffer;
	GlyphFileState mFileState[sMaxGlyphFiles];
	ResourceListenerRegistration mRegistration;

};


FontRenderer::FontRenderer(Graphics& g) : mImpl(std::make_unique<Impl>(g)) {
}

FontRenderer::~FontRenderer() {
}

void FontRenderer::RenderRun(array_view<const char> text,
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

		auto u1 = glyph.rect.x - 0.5f;
		auto v1 = glyph.rect.y - 0.5f;
		auto u2 = glyph.rect.x + glyph.rect.width + 0.5f;
		auto v2 = glyph.rect.y + glyph.rect.height + 0.5f;

		auto& state = mImpl->mFileState[glyph.fileIdx];

		TigRect destRect;
		destRect.x = x;
		destRect.y = y + font.baseline - glyph.base_line_y_offset;
		destRect.width = glyph.rect.width + 1;
		destRect.height = glyph.rect.height + 1;

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
		if (style.flags & 0x8000) {
			float rotCenterX, rotCenterY;
			if (style.flags & 0x10000) {
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

	auto texture = gfx::textureManager->GetById(textureId);
	auto deviceTexture = texture->GetDeviceTexture();
	if (!deviceTexture) {
		logger->error("Trying to render glyph from invalid device texture");
		return;
	}
	renderStates->SetTexture(0, deviceTexture);

	auto device = graphics->device();

	CComPtr<IDirect3DVertexBuffer9> buffer;
	const auto vertexCount = glyphCount * 4;
	const auto bufferSize = sizeof(GlyphVertex3d) * vertexCount;
	constexpr auto fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;

	if (D3DLOG(device->CreateVertexBuffer(bufferSize,
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		fvf,
		D3DPOOL_DEFAULT,
		&buffer,
		nullptr)) != D3D_OK) {
		return;
	}

	GlyphVertex3d* vertices;
	if (D3DLOG(buffer->Lock(0, bufferSize, (void**)&vertices, D3DLOCK_DISCARD)) != D3D_OK) {
		return;
	}

	auto textureSize = texture->GetSize();
	for (auto i = 0; i < vertexCount; ++i) {
		ConvertVertex(vertices2d[i], vertices[i], textureSize);
	}

	if (D3DLOG(buffer->Unlock()) != D3D_OK) {
		return;
	}

	renderStates->SetTextureColorOp(0, D3DTOP_MODULATE);
	renderStates->SetTextureColorArg1(0, D3DTA_TEXTURE);
	renderStates->SetTextureColorArg2(0, D3DTA_CURRENT);

	renderStates->SetTextureAlphaOp(0, D3DTOP_MODULATE);
	renderStates->SetTextureAlphaArg1(0, D3DTA_TEXTURE);
	renderStates->SetTextureAlphaArg2(0, D3DTA_CURRENT);

	renderStates->SetTextureMipFilter(0, D3DTEXF_LINEAR);
	renderStates->SetTextureMinFilter(0, D3DTEXF_LINEAR);
	renderStates->SetTextureMagFilter(0, D3DTEXF_LINEAR);

	renderStates->SetColorVertex(true);
	renderStates->SetAlphaTestEnable(true);
	renderStates->SetAlphaBlend(true);
	renderStates->SetSrcBlend(D3DBLEND_SRCALPHA);
	renderStates->SetDestBlend(D3DBLEND_INVSRCALPHA);

	renderStates->SetStreamSource(0, buffer, sizeof(GlyphVertex3d));
	renderStates->SetIndexBuffer(mImpl->mIndexBuffer, 0);
	renderStates->Commit();

	D3DLOG(device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertexCount, 0, glyphCount * 2));
}

void FontRenderer::Rotate2d(float& x, float& y, float rotCos, float rotSin, float centerX, float centerY) {
	auto newX = centerX + rotCos * (x - centerX) - rotSin * (y - centerY);
	auto newY = centerY + rotSin * (x - centerX) + rotCos * (y - centerY);
	x = newX;
	y = newY;
}

void FontRenderer::Impl::CreateResources(Graphics& g) {

	auto device = g.device();
	const auto idxBufferLength = sizeof(uint16_t) * 3 * 2 * GlyphFileState::MaxGlyphs;
	if (D3DLOG(device->CreateIndexBuffer(idxBufferLength,
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_DEFAULT,
		&mIndexBuffer,
		nullptr)) != D3D_OK) {
		throw TempleException("Unable to create glyph index buffer");
	}

	uint16_t* indices;
	if (D3DLOG(mIndexBuffer->Lock(0, idxBufferLength, (void**)&indices, D3DLOCK_DISCARD)) != D3D_OK) {
		throw TempleException("Unable to lock glyph index buffer");
	}

	auto vertexIdx = 0;
	for (auto i = 0; i < GlyphFileState::MaxGlyphs; ++i) {
		// Counter clockwise quad rendering
		*indices++ = vertexIdx + 0;
		*indices++ = vertexIdx + 1;
		*indices++ = vertexIdx + 2;
		*indices++ = vertexIdx + 0;
		*indices++ = vertexIdx + 2;
		*indices++ = vertexIdx + 3;
		vertexIdx += 4;
	}

	if (D3DLOG(mIndexBuffer->Unlock()) != D3D_OK) {
		throw TempleException("Unable to unlock glyph index buffer");
	}
}

void FontRenderer::Impl::FreeResources(Graphics&) {
	mIndexBuffer = nullptr;
}
