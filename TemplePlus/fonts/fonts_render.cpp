#include <stdafx.h>

#include "util/fixes.h"
#include "tig/tig.h"
#include "tig/tig_font.h"
#include "graphics/render_hooks.h"

using namespace gsl;

static struct FontRenderAddresses : temple::AddressTable {

	int* stackSize;
	TigFont* loadedFonts;
	int* fontStack;

	FontRenderAddresses() {
		rebase(stackSize, 0x10EF2E50);
		rebase(loadedFonts, 0x10EF1448);
		rebase(fontStack, 0x10EF2C48);
	}
} addresses;

struct TigFontDrawArgs {
	const TigRect* rect;
	int xrlt;
	int yrlt;
	int field_c;
	int textLength;
};

static class FontRenderFix : public TempleFix {
public:
	const char* name() override {
		return "Font Rendering Replacement";
	}

	void apply() override;

	static int FontDraw(const char* text, TigRect* extents, TigTextStyle* style);
	static int FontDrawBg(TigRect* rect, TigTextStyle* style, const char* text);

	static int FontDrawDoWork(TigTextStyle* style, const char*& text, const TigFontDrawArgs& args, const TigFont& font);
} fix;

void FontRenderFix::apply() {
	replaceFunction(0x101EAF30, FontDraw);
}


int FontRenderFix::FontDrawBg(TigRect* rect, TigTextStyle* style, const char* text) {
	// srcRect doesnt really matter if we dont use a texture
	TigRect srcRect;
	srcRect.width = 1;
	srcRect.height = 1;
	srcRect.x = 0;
	srcRect.y = 0;

	TigRect destRect;
	destRect.x = rect->x;
	destRect.y = rect->y;

	if (rect->width) {
		destRect.width = rect->width;
		destRect.height = rect->height;
	} else {
		TigFontMetrics metrics;
		metrics.text = text;
		metrics.width = 0;
		metrics.height = 0;
		tigFont.Measure(*style, metrics);
		destRect.width = metrics.width;
		destRect.height = metrics.height;
	}

	Render2dArgs args;
	args.flags = Render2dArgs::FLAG_VERTEXCOLORS
		| Render2dArgs::FLAG_VERTEXALPHA;
	args.textureId = 0;
	args.srcRect = &srcRect;
	args.destRect = &destRect;
	args.vertexColors = &style->bgColor->topLeft;
	args.vertexZ = (float) INT_MAX ;
	return RenderHooks::TextureRender2d(&args);

}

int FontRenderFix::FontDrawDoWork(TigTextStyle* style, const char*& text, const TigFontDrawArgs& args, const TigFont& font) {

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

// First character found in the FNT files
constexpr auto FirstFontChar = '!';
constexpr static auto sEllipsis = "...";
constexpr static auto tig_font_is_english = true; // TODO

static int MeasureCharRun(cstring_view<> text, const TigTextStyle& style, const TigRect& extents, int extentsWidth, const TigFont& font,
                          int &v89, int linePadding, bool v93) {
	int v84 = 0;
	const auto tabWidth = style.field4c - extents.x;

	auto lineWidth = 0;
	auto v74 = 0;

	// This seems to be special handling for the sequence "@t" and @0 - @9
	auto it = text.begin();
	for (; it != text.end(); ++it) {
		auto ch = *it;
		auto nextCh = '\0';
		if (it + 1 != text.end()) {
			nextCh = *(it + 1);
		}

		// Handles @0 to @9
		if (ch == '@' && isdigit(nextCh)) {
			++it; // Skip the number
		}
		else if (ch == '@' && nextCh == 't') {
			++it; // Skip the t

			if (tabWidth == 0) {
				break;
			}

			lineWidth += tabWidth;
		}
		else if (ch == '\n') {
			if (v89 + lineWidth <= extentsWidth) {
				++v74;
				if (v89 + lineWidth <= extentsWidth + linePadding)
					v84 = v74;
				v89 += lineWidth;
				lineWidth = 0;
			}
			break;
		} else if (isspace(ch)) {
			if (v89 + lineWidth > extentsWidth)
				break;
			++v74;
			if (v89 + lineWidth <= extentsWidth + linePadding)
				v84 = v74;
			v89 += lineWidth + style.tracking;
			lineWidth = 0;
		} else {

			auto glyphIdx = ch - FirstFontChar;
			if (tig_font_is_english) {
				if ((glyphIdx < -1 || glyphIdx > 95) && glyphIdx != 0xFFFFFFE9) {
					logger->warn("Tried to display character {} in text '{}'", glyphIdx, text);
					glyphIdx = -1;
				}
			}

			lineWidth += style.kerning + font.glyphs[glyphIdx].width_line;
		}
	}

	if (it == text.end()) {
		if (lineWidth) {
			if (lineWidth + v89 > extentsWidth) {
				if (style.flags & 0x4000) {
					v89 += lineWidth;
					++v74;
					v84 = v74;
				}
			} else {
				v89 += lineWidth;
				v74++;
				if (v89 + lineWidth <= extentsWidth + linePadding)
					v84 = v74;
			}
		}
	}
	if (!v93 || it == text.end() || !(style.flags & 0x4000))
		v84 = v74;

	return v84;
}

int FontRenderFix::FontDraw(const char* text, TigRect* extents, TigTextStyle* style) {
	auto glyphIdx = 0;
	auto v93 = false;
	auto drawEllipsis = false;
	style->field30 = 0;
	if (*addresses.stackSize < 1)
		return 3;

	auto currentX = extents->x;
	auto currentY = extents->y;
	auto extentsWidth = extents->width;
	auto extentsHeight = extents->height;
	auto textLength = strlen(text);
	auto font = addresses.loadedFonts[addresses.fontStack[0]];
	if (!extentsWidth) {
		TigFontMetrics metrics;
		metrics.text = text;
		metrics.width = extents->width;
		metrics.height = extents->height;
		tigFont.Measure(*style, metrics);

		extents->width = metrics.width;
		extents->height = metrics.height;
		extentsWidth = metrics.width;
		extentsHeight = metrics.height;
	}
	if (style->flags & 0x400) {
		TigRect rect;
		rect.x = extents->x - 3;
		rect.y = extents->y - 3;

		if (extents->width <= 0)
			rect.width = extentsWidth + 6;
		else
			rect.width = extents->width + 6;
		if (extents->height <= 0)
			rect.height = extents->height + 6;
		else
			rect.height = extents->height + 6;
		FontDrawBg(&rect, style, text);
	}

	if (style->flags & 0x800) {
		D3DXVECTOR2 topLeft;
		topLeft.x = extents->x - 1.0f - 3.0f;
		topLeft.y = extents->y - 1.0f - 3.0f;

		float width;
		if (extents->width <= 0)
			width = (float)extentsWidth;
		else
			width = (float)extents->width;

		float height;
		if (extents->height <= 0)
			height = (float)extentsHeight;
		else
			height = (float)extents->height;

		D3DXVECTOR2 bottomRight;
		bottomRight.x = width + extents->x - 1.0f + 3.0f;
		bottomRight.y = height + extents->y - 1.0f + 3.0f;
		RenderHooks::RenderRect(topLeft, bottomRight, 0xFF000000);
	}

	if (!extentsWidth) {
		TigFontDrawArgs args;
		args.yrlt = currentY;
		args.xrlt = currentX;
		args.rect = extents;
		args.field_c = 0;
		args.textLength = strlen(text);
		FontDrawDoWork(style, text, args, font);
		return 0;
	}

	// Haben nur Platz für eine Zeile???
	auto linePadding = 0;
	if (currentY + 2 * font.largestHeight > extents->y + extents->height) {
		v93 = true;
		if (style->flags & 0x4000) {
			glyphIdx = *sEllipsis - FirstFontChar;
			linePadding = -3 * (style->kerning + font.glyphs[glyphIdx].width_line);
		}
	}

	if (textLength <= 0)
		return 0;

	const auto tabWidth = style->field4c - extents->x;

	size_t v80 = 0;
	while (2) {
		auto v89 = 0;
		auto arg = ensure_z(text + v80, textLength);
		auto wordsOnLine = MeasureCharRun(arg, *style, *extents, extentsWidth, font, v89, linePadding, v93);

		// TODO: Check if v36 can be mapped to v80 1:1
		auto v36 = v80;
		for (auto wordIdx = 0; wordIdx < wordsOnLine && !drawEllipsis; ++wordIdx) {
			auto v81 = 0;
			auto v86 = 0;
			auto wordWidth = 0;
			auto v38 = v36;
			auto firstIdx = v36;
			auto v77 = v36;
				
			for (; v38 < textLength; v38++) {
				char curCh = text[v38];
				char nextCh = '\0';
				if (v38 + 1 < textLength) {
					nextCh = text[v38 + 1];
				}

				// Simply skip @t without increasing the width
				if (curCh == '@' && isdigit(nextCh)) {
					v38++; // Skip the number
					continue;
				}

				// @t will advance the width up to the next tabstop
				if (curCh == '@' && nextCh == 't') {
					v38++; // Skip the t
					if (tabWidth > 0) {
						v86 += tabWidth;
						if (style->flags & 0x4000) {
							auto v48 = currentX - extents->x + v86;
							if (v48 > extentsWidth + linePadding) {
								drawEllipsis = true;
								continue;
							}
							v81 = v38;
						}
						wordWidth += tabWidth;
					}
					continue;
				}

				glyphIdx = text[v38] - FirstFontChar;
				if (tig_font_is_english) {
					if ((glyphIdx < -1 || glyphIdx > '_') && text[v38] != '\n') {
						logger->warn("Tried to display character {} in text '{}'", glyphIdx, text);
						glyphIdx = -1;
					}
				}

				if (curCh == '\n') {
					v77 = v38;
					if (v93) {
						if (style->flags & 0x4000)
							drawEllipsis = true;
						break;
					}
					goto LABEL_168;
				}
					
				if (isspace(curCh)) {
					break;
				}
				if (style->flags & 0x4000) {
					if ((extentsWidth + linePadding) < currentX + v86 - extents->x) {
						drawEllipsis = true;
						continue;
					}
					v81 = v38;
				}
				wordWidth += font.glyphs[glyphIdx].width_line + style->kerning;
			}

			v77 = v38;

			int lastIdx;

			if (v93 && style->flags & 0x4000) {
				// We're on the last line and truncation is active
				// This will seek v38 to the next word
				for (; v38 < textLength; ++v38) {
					auto curChar = text[v38];
					char nextChar = '\0';
					if (v38 + 1 < textLength) {
						nextChar = text[v38 + 1];
					}

					// Handles @0 - @9 and skips the number
					if (curChar == '@' && isdigit(nextChar)) {
						++v38;
						continue;
					}

					if (curChar == '@' && nextChar == 't') {
						++v38;
						if (tabWidth > 0) {
							continue;
						}
					}

					if (curChar != '\n' && !isspace(curChar)) {
						break;
					}
				}
				bool lastWord = (v38 == textLength);

				if (currentX + v86 - extents->x <= extentsWidth) {
					lastIdx = v77;
					v36 = v77;
					v80 = v77;
					if (lastWord) {
						drawEllipsis = false;
						wordWidth = v86;
					}
					goto LABEL_143;
				}
				lastIdx = v81;
				v36 = v81;
			} else {
			LABEL_168:
				lastIdx = v38;
				v36 = v38;
			}
			v80 = v36;
		LABEL_143:
			if (isspace(glyphIdx + FirstFontChar) || v36 == textLength) {
				wordWidth += style->tracking;
			}

			// This means this is not the last word in this line
			if (wordIdx + 1 < wordsOnLine) {
				++v36;
				v80 = v36;
			}

			TigFontDrawArgs drawArgs;
			drawArgs.rect = extents;
			drawArgs.xrlt = currentX;
			// This centers the line
			if (style->flags & 0x10) {
				drawArgs.xrlt += (extentsWidth - v89) / 2;
			}
			drawArgs.yrlt = currentY;
			drawArgs.field_c = firstIdx;
			drawArgs.textLength = lastIdx;

			FontDrawDoWork(style, text, drawArgs, font);
			currentX += wordWidth;
		}

		// Resets X, advances to next line, has not run out of vertical space (v93)
		if (!v93 || !(style->flags & 0x4000)) {
			currentX = extents->x;
			currentY += font.largestHeight;
			if (currentY + 2 * font.largestHeight > extents->y + extents->height) {
				v93 = true;
				if (style->flags & 0x4000) {
					glyphIdx = *sEllipsis - FirstFontChar;
					linePadding = 3 * style->kerning - font.glyphs[glyphIdx].width_line;
				}
			}
			++v80;
			if (v80 >= textLength)
				return 0;
			continue;
		}

		break;
	}

	if (drawEllipsis) {
		TigFontDrawArgs drawArgs;
		drawArgs.rect = extents;
		drawArgs.xrlt = currentX;
		drawArgs.yrlt = currentY;
		drawArgs.field_c = 0;
		drawArgs.textLength = strlen(sEllipsis);
		auto ellipsis = sEllipsis;
		FontDrawDoWork(style, ellipsis, drawArgs, font);
	}
	return 0;

}
