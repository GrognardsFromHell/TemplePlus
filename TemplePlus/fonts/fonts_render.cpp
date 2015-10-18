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
	int firstIdx;
	int lastIdx;
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

static std::pair<int, int> MeasureCharRun(cstring_view<> text,
                                          const TigTextStyle& style,
                                          const TigRect& extents,
                                          int extentsWidth,
                                          const TigFont& font,
                                          int linePadding,
                                          bool lastLine) {
	auto lineWidth = 0;
	auto wordCountWithPadding = 0;
	auto wordWidth = 0;
	auto wordCount = 0;

	const auto tabWidth = style.field4c - extents.x;

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
		} else if (ch == '@' && nextCh == 't') {
			++it; // Skip the t

			if (tabWidth == 0) {
				break;
			}

			wordWidth += tabWidth;
		} else if (ch == '\n') {
			if (lineWidth + wordWidth <= extentsWidth) {
				wordCount++;
				if (lineWidth + wordWidth <= extentsWidth + linePadding) {
					wordCountWithPadding++;
				}
				lineWidth += wordWidth;
				wordWidth = 0;
			}
			break;
		} else if (isspace(ch)) {
			if (lineWidth + wordWidth <= extentsWidth) {
				wordCount++;
				if (lineWidth + wordWidth <= extentsWidth + linePadding) {
					wordCountWithPadding++;
				}
				lineWidth += wordWidth + style.tracking;
				wordWidth = 0;
			} else {
				// Stop if we have run out of space on this line
				break;
			}
		} else {
			auto glyphIdx = ch - FirstFontChar;
			if (tig_font_is_english) {
				if ((glyphIdx < -1 || glyphIdx > '_') && ch != '\n') {
					logger->warn("Tried to display character {} in text '{}'", glyphIdx, text);
					glyphIdx = -1;
				}
			}

			wordWidth += style.kerning + font.glyphs[glyphIdx].width_line;
		}
	}

	// Handle the last word, if we're at the end of the string
	if (it == text.end() && wordWidth > 0) {
		if (lineWidth + wordWidth <= extentsWidth) {
			wordCount++;
			lineWidth += wordWidth;
			if (lineWidth + wordWidth <= extentsWidth + linePadding) {
				wordCountWithPadding++;
			}
		} else if (style.flags & 0x4000) {
			// The word would actually not fit, but we're the last
			// thing in the string and we truncate with ...
			lineWidth += wordWidth;
			wordCount++;
			wordCountWithPadding++;
		}
	}

	// Ignore the padding if we'd not print ellipsis anyway
	if (!lastLine || it == text.end() || !(style.flags & 0x4000)) {
		wordCountWithPadding = wordCount;
	}

	return std::make_pair(wordCountWithPadding, lineWidth);
}

static bool HasMoreText(cstring_view<> text, int tabWidth) {
	// We're on the last line and truncation is active
	// This will seek to the next word
	auto it = text.begin();
	for (; it != text.end(); ++it) {
		auto curChar = *it;
		auto nextChar = '\0';
		if (it + 1 != text.end()) {
			nextChar = *(it + 1);
		}

		// Handles @0 - @9 and skips the number
		if (curChar == '@' && isdigit(nextChar)) {
			++it;
			continue;
		}

		if (curChar == '@' && nextChar == 't') {
			++it;
			if (tabWidth > 0) {
				continue;
			}
		}

		if (curChar != '\n' && !isspace(curChar)) {
			return true;
		}
	}

	return false;
}

struct ScanWordResult {
	size_t firstIdx;
	size_t lastIdx;
	int idxBeforePadding = 0;
	int width = 0;
	int fullWidth = 0; // Ignores padding
	bool drawEllipsis = false;
};

static ScanWordResult ScanWord(const char* text,
                               int firstIdx,
                               int textLength,
                               int tabWidth,
                               bool lastLine,
                               const TigFont& font,
                               const TigTextStyle& style,
                               int remainingSpace) {

	ScanWordResult result;
	result.firstIdx = firstIdx;

	auto i = firstIdx;
	for (; i < textLength; i++) {
		auto curCh = text[i];
		auto nextCh = '\0';
		if (i + 1 < textLength) {
			nextCh = text[i + 1];
		}

		// Simply skip @t without increasing the width
		if (curCh == '@' && isdigit(nextCh)) {
			i++; // Skip the number
			continue;
		}

		// @t will advance the width up to the next tabstop
		if (curCh == '@' && nextCh == 't') {
			i++; // Skip the t
			if (tabWidth > 0) {
				if (style.flags & 0x4000) {
					result.fullWidth += tabWidth;
					if (result.fullWidth > remainingSpace) {
						result.drawEllipsis = true;
						continue;
					}
					// The idx right before the width - padding starts
					result.idxBeforePadding = i;
				}
				result.width += tabWidth;
			}
			continue;
		}

		auto glyphIdx = curCh - FirstFontChar;
		if (tig_font_is_english) {
			if ((glyphIdx < -1 || glyphIdx > '_') && curCh != '\n') {
				logger->warn("Tried to display character {} in text '{}'", glyphIdx, text);
				glyphIdx = -1;
			}
		}

		if (curCh == '\n') {
			if (lastLine && style.flags & 0x4000) {
				result.drawEllipsis = true;
			}
			break;
		}

		if (isspace(curCh)) {
			break;
		}

		if (style.flags & 0x4000) {
			result.fullWidth += font.glyphs[glyphIdx].width_line + style.kerning;
			if (result.fullWidth > remainingSpace) {
				result.drawEllipsis = true;
				continue;
			}
			result.idxBeforePadding = i;
		}
		result.width += font.glyphs[glyphIdx].width_line + style.kerning;
	}

	result.lastIdx = i;
	return result;
}

int FontRenderFix::FontDraw(const char* text, TigRect* extents, TigTextStyle* style) {
	auto lastLine = false;
	style->field30 = 0;
	if (*addresses.stackSize < 1)
		return 3;

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
		args.xrlt = extents->x;
		args.yrlt = extents->y;
		args.rect = extents;
		args.firstIdx = 0;
		args.lastIdx = strlen(text);
		FontDrawDoWork(style, text, args, font);
		return 0;
	}

	// Haben nur Platz für eine Zeile???
	const int ellipsisWidth = 3 * (style->kerning + font.glyphs['.' - FirstFontChar].width_line);
	auto linePadding = 0;
	if (extents->y + 2 * font.largestHeight > extents->y + extents->height) {
		lastLine = true;
		if (style->flags & 0x4000) {
			linePadding = - ellipsisWidth;
		}
	}

	if (textLength <= 0)
		return 0;

	const auto tabWidth = style->field4c - extents->x;

	auto currentY = extents->y;
	for (size_t startOfWord = 0; startOfWord < textLength; ++startOfWord) {
		auto arg = ensure_z(text + startOfWord, textLength);
		int wordsOnLine, lineWidth;
		std::tie(wordsOnLine, lineWidth) = MeasureCharRun(arg,
		                                                  *style,
		                                                  *extents,
		                                                  extentsWidth,
		                                                  font,
		                                                  linePadding,
		                                                  lastLine);

		auto currentX = 0;
		for (auto wordIdx = 0; wordIdx < wordsOnLine; ++wordIdx) {

			auto remainingSpace = extentsWidth + linePadding - currentX;

			auto wordInfo(ScanWord(text, 
				startOfWord, 
				textLength, 
				tabWidth, 
				lastLine, 
				font, 
				*style, 
				remainingSpace));

			auto lastIdx = wordInfo.lastIdx;
			auto wordWidth = wordInfo.width;

			if (lastLine && style->flags & 0x4000) {
				if (currentX + wordInfo.fullWidth > extentsWidth) {
					lastIdx = wordInfo.idxBeforePadding;
				} else {
					auto remainingText = ensure_z(text + lastIdx, textLength);
					if (!HasMoreText(remainingText, tabWidth)) {
						wordInfo.drawEllipsis = false;
						wordWidth = wordInfo.fullWidth;
					}
				}
			}

			startOfWord = lastIdx;
			if (startOfWord < textLength && isspace(text[startOfWord])) {
				wordWidth += style->tracking;
			}

			// This means this is not the last word in this line
			if (wordIdx + 1 < wordsOnLine) {
				startOfWord++;
			}

			// Draw the word
			TigFontDrawArgs drawArgs;
			drawArgs.rect = extents;
			drawArgs.xrlt = extents->x + currentX;
			// This centers the line
			if (style->flags & 0x10) {
				drawArgs.xrlt += (extentsWidth - lineWidth) / 2;
			}
			drawArgs.yrlt = currentY;
			drawArgs.firstIdx = wordInfo.firstIdx;
			drawArgs.lastIdx = lastIdx;

			FontDrawDoWork(style, text, drawArgs, font);
			currentX += wordWidth;

			// We're on the last line, the word has been truncated, ellipsis needs to be drawn
			if (lastLine && style->flags & 0x4000 && wordInfo.drawEllipsis) {
				drawArgs.xrlt = extents->x + currentX;
				drawArgs.firstIdx = 0;
				drawArgs.lastIdx = strlen(sEllipsis);
				auto ellipsis = sEllipsis;
				FontDrawDoWork(style, ellipsis, drawArgs, font);
				return 0;
			}
		}

		// Advance to next line
		currentY += font.largestHeight;
		if (currentY + 2 * font.largestHeight > extents->y + extents->height) {
			lastLine = true;
			if (style->flags & 0x4000) {
				linePadding = ellipsisWidth;
			}
		}
	}

	return 0;
}
