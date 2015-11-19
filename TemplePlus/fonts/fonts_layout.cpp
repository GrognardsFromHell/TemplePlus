#pragma once

#include "stdafx.h"

#include <tig/tig_font.h>

#include <graphics/shaperenderer2d.h>

#include "fonts.h"

using namespace gfx;

const char* TextLayouter::sEllipsis = "...";

struct ScanWordResult {
	size_t firstIdx;
	size_t lastIdx;
	int idxBeforePadding = 0;
	int width = 0;
	int fullWidth = 0; // Ignores padding
	bool drawEllipsis = false;
};

TextLayouter::TextLayouter(RenderingDevice& device, ShapeRenderer2d &shapeRenderer) 
	: mRenderer(device), mShapeRenderer(shapeRenderer) {
}

void TextLayouter::LayoutAndDraw(gsl::cstring_view<> text, const TigFont& font, TigRect& extents, TigTextStyle& style) {

	if (text.length() == 0) {
		return;
	}

	auto lastLine = false;
	auto extentsWidth = extents.width;
	auto extentsHeight = extents.height;
	auto textLength = text.length();
	if (!extentsWidth) {
		TigFontMetrics metrics;
		metrics.text = &text[0];
		metrics.width = extents.width;
		metrics.height = extents.height;
		tigFont.Measure(style, metrics);

		extents.width = metrics.width;
		extents.height = metrics.height;
		extentsWidth = metrics.width;
		extentsHeight = metrics.height;
	}

	if (style.flags & 0x400) {
		TigRect rect;
		rect.x = extents.x - 3;
		rect.y = extents.y - 3;

		if (extents.width <= 0)
			rect.width = extentsWidth + 6;
		else
			rect.width = extents.width + 6;
		if (extents.height <= 0)
			rect.height = extents.height + 6;
		else
			rect.height = extents.height + 6;
		DrawBackground(rect, style);
	}

	if (style.flags & 0x800) {
		XMFLOAT2 topLeft;
		topLeft.x = extents.x - 1.0f - 3.0f;
		topLeft.y = extents.y - 1.0f - 3.0f;

		float width;
		if (extents.width <= 0)
			width = (float)extentsWidth;
		else
			width = (float)extents.width;

		float height;
		if (extents.height <= 0)
			height = (float)extentsHeight;
		else
			height = (float)extents.height;

		XMFLOAT2 bottomRight;
		bottomRight.x = width + extents.x - 1.0f + 3.0f;
		bottomRight.y = height + extents.y - 1.0f + 3.0f;

		mShapeRenderer.DrawRectangleOutline(
			topLeft,
			bottomRight,
			XMCOLOR(0, 0, 0, 1.0f)
		);
	}

	// TODO: Check if this can even happen since we measure the text
	// if the width hasn't been constrained
	if (!extentsWidth) {
		mRenderer.RenderRun(
			text,
			extents.x,
			extents.y,
			extents,
			style,
			font
		);
		return;
	}

	// Is there only space for one line?
	const int ellipsisWidth = 3 * (style.kerning + font.glyphs[GetGlyphIdx('.', sEllipsis)].width_line);
	auto linePadding = 0;
	if (extents.y + 2 * font.largestHeight > extents.y + extents.height) {
		lastLine = true;
		if (style.flags & 0x4000) {
			linePadding = -ellipsisWidth;
		}
	}

	if (textLength <= 0)
		return;

	const auto tabWidth = style.field4c - extents.x;

	auto currentY = extents.y;
	for (size_t startOfWord = 0; startOfWord < textLength; ++startOfWord) {
		int wordsOnLine, lineWidth;
		std::tie(wordsOnLine, lineWidth) = MeasureCharRun(text.sub(startOfWord),
		                                                  style,
		                                                  extents,
		                                                  extentsWidth,
		                                                  font,
		                                                  linePadding,
		                                                  lastLine);

		auto currentX = 0;
		for (auto wordIdx = 0; wordIdx < wordsOnLine; ++wordIdx) {

			auto remainingSpace = extentsWidth + linePadding - currentX;

			auto wordInfo(ScanWord(&text[0],
			                       startOfWord,
			                       textLength,
			                       tabWidth,
			                       lastLine,
			                       font,
			                       style,
			                       remainingSpace));

			auto lastIdx = wordInfo.lastIdx;
			auto wordWidth = wordInfo.width;

			if (lastLine && style.flags & 0x4000) {
				if (currentX + wordInfo.fullWidth > extentsWidth) {
					lastIdx = wordInfo.idxBeforePadding;
				} else {
					if (!HasMoreText(text.sub(lastIdx), tabWidth)) {
						wordInfo.drawEllipsis = false;
						wordWidth = wordInfo.fullWidth;
					}
				}
			}

			startOfWord = lastIdx;
			if (startOfWord < textLength && isspace(text[startOfWord])) {
				wordWidth += style.tracking;
			}

			// This means this is not the last word in this line
			if (wordIdx + 1 < wordsOnLine) {
				startOfWord++;
			}

			// Draw the word
			auto x = extents.x + currentX;
			// This centers the line
			if (style.flags & 0x10) {
				x += (extentsWidth - lineWidth) / 2;
			}

			mRenderer.RenderRun(
				text.sub(wordInfo.firstIdx, lastIdx - wordInfo.firstIdx),
				x,
				currentY,
				extents,
				style,
				font);

			currentX += wordWidth;

			// We're on the last line, the word has been truncated, ellipsis needs to be drawn
			if (lastLine && style.flags & 0x4000 && wordInfo.drawEllipsis) {
				mRenderer.RenderRun({ sEllipsis, strlen(sEllipsis) },
					extents.x + currentX,
					currentY,
					extents,
					style,
					font);
				return;
			}
		}

		// Advance to next line
		currentY += font.largestHeight;
		if (currentY + 2 * font.largestHeight > extents.y + extents.height) {
			lastLine = true;
			if (style.flags & 0x4000) {
				linePadding = ellipsisWidth;
			}
		}
	}

}

void TextLayouter::DrawBackground(const TigRect& rect, const TigTextStyle& style) {
	
	float left = (float)rect.x;
	float top = (float)rect.y;
	float right = left + rect.width;
	float bottom = top + rect.height;

	std::array<Vertex2d, 4> corners;
	corners[0].pos = XMFLOAT3(left, top, 0.5f);
	corners[1].pos = XMFLOAT3(right, top, 0.5f);
	corners[2].pos = XMFLOAT3(right, bottom, 0.5f);
	corners[3].pos = XMFLOAT3(left, bottom, 0.5f);

	corners[0].diffuse = style.bgColor->topLeft;
	corners[1].diffuse = style.bgColor->topRight;
	corners[2].diffuse = style.bgColor->bottomRight;
	corners[3].diffuse = style.bgColor->bottomLeft;

	corners[0].uv = XMFLOAT2(0, 0);
	corners[1].uv = XMFLOAT2(0, 0);
	corners[2].uv = XMFLOAT2(0, 0);
	corners[3].uv = XMFLOAT2(0, 0);
	
	// Draw an untexture rectangle
	mShapeRenderer.DrawRectangle(corners, nullptr);

}

int TextLayouter::GetGlyphIdx(char ch, const char* text) {

	// First character found in the FNT files
	constexpr auto FirstFontChar = '!';
	static bool tig_font_is_english = true; // TODO

	auto glyphIdx = ch - FirstFontChar;

	if (tig_font_is_english) {
		if ((glyphIdx < -1 || glyphIdx > '_') && ch != '\n') {
			logger->warn("Tried to display character {} in text '{}'", glyphIdx, text);
			glyphIdx = -1;
		}
	}

	return glyphIdx;

}


std::pair<int, int> TextLayouter::MeasureCharRun(cstring_view<> text,
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
		}
		else if (ch == '@' && nextCh == 't') {
			++it; // Skip the t

			if (tabWidth == 0) {
				break;
			}

			wordWidth += tabWidth;
		}
		else if (ch == '\n') {
			if (lineWidth + wordWidth <= extentsWidth) {
				wordCount++;
				if (lineWidth + wordWidth <= extentsWidth + linePadding) {
					wordCountWithPadding++;
				}
				lineWidth += wordWidth;
				wordWidth = 0;
			}
			break;
		}
		else if (isspace(ch)) {
			if (lineWidth + wordWidth <= extentsWidth) {
				wordCount++;
				if (lineWidth + wordWidth <= extentsWidth + linePadding) {
					wordCountWithPadding++;
				}
				lineWidth += wordWidth + style.tracking;
				wordWidth = 0;
			}
			else {
				// Stop if we have run out of space on this line
				break;
			}
		}
		else {
			auto glyphIdx = GetGlyphIdx(ch, &text[0]);
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
		}
		else if (style.flags & 0x4000) {
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

bool TextLayouter::HasMoreText(cstring_view<> text, int tabWidth) {
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

ScanWordResult TextLayouter::ScanWord(const char* text,
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

		auto glyphIdx = GetGlyphIdx(curCh, &text[0]);

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

