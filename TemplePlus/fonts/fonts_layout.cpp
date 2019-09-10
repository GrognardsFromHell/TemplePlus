#pragma once

#include "stdafx.h"

#include <tig/tig_font.h>

#include <graphics/shaperenderer2d.h>
#include <graphics/device.h>
#include <graphics/textengine.h>

#include "fonts.h"
#include "fonts_mapping.h"
#include <gamesystems/legacy.h>

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
	: mTextEngine(device.GetTextEngine()), mRenderer(device), mShapeRenderer(shapeRenderer) {
	mMapping = std::make_unique<FontsMapping>();
}

TextLayouter::~TextLayouter() = default;

void ApplyStyle(const TigTextStyle &style, int tabPos, gfx::TextStyle &textStyle) {
	if (tabPos > 0) {
		textStyle.tabStopWidth = (float)tabPos;
	}

	// Convert the color (optional for measurements)
	if (style.textColor) {
		textStyle.foreground.primaryColor = style.textColor->topLeft;
		if (style.textColor->topLeft != style.textColor->bottomRight) {
			textStyle.foreground.gradient = true;
			textStyle.foreground.secondaryColor = style.textColor->bottomRight;
		}
	}

	if (style.flags & 0x4000) {
		textStyle.trim = true;
	}

	// Layouting options
	if (style.flags & TTSF_CENTER) {
		textStyle.align = TextAlign::Center;
	}
	if (style.flags & TTSF_DROP_SHADOW) {
		textStyle.dropShadow = true;
		if (style.shadowColor) {
			textStyle.dropShadowBrush.primaryColor = style.shadowColor->topLeft;
			textStyle.dropShadowBrush.primaryColor.a = 255;
		}
	}
}

static FormattedText ProcessString(const TextStyle& defaultStyle, const TigTextStyle &tigStyle, gsl::cstring_span<> text)
{
	FormattedText result;
	result.defaultStyle = defaultStyle;
	result.text.reserve(text.size());

	bool inColorRange = false;
	bool inEscape = false;
	for (int i = 0; i < text.size(); i++) {
		auto ch = text[i];
		if (ch == '@') {
			inEscape = true;
		}
		else if (inEscape) {
			inEscape = false;

			if (ch == 't') {
				result.text.back() = '\t';
				continue;
			}
			else if (iswdigit(ch)) {
				auto colorIdx = ch - '0';

				// Remove the @ that we're about to remove from the previous color range
				if (inColorRange) {
					result.formats.back().length--;
				}

				// Remove last char (@)
				result.text.erase(result.text.size() - 1);

				if (colorIdx == 0 || !tigStyle.textColor) {
					// Return to the normal formatting
					inColorRange = false;
				}
				else {
					inColorRange = true;

					// Add a constrainted text style
					ConstrainedTextStyle newStyle(defaultStyle);
					newStyle.startChar = result.text.size();

					// Set the desired color
					newStyle.style.foreground.gradient = false;
					newStyle.style.foreground.primaryColor = tigStyle.textColor[colorIdx].topLeft;

					result.formats.emplace_back(std::move(newStyle));
				}
				continue;
			}

		}

		if (inColorRange) {
			// Extend the colored range by one char
			result.formats.back().length++;
		}
		result.text.push_back(ch);
	}

	return result;
}

void TextLayouter::LayoutAndDraw(gsl::cstring_span<> text, const TigFont& font, TigRect& extents, TigTextStyle& style) {

	if (text.length() == 0) {
		return;
	}

	// Get the base text format and check if we should render using the new or old algorithms
	auto it = mMapping->find(font.name);
	if (it == mMapping->end()) {
		// use the old font drawing algorithm
		LayoutAndDrawVanilla(text, font, extents, style);
		return;
	}

	// Use the new text engine style of drawing
	auto tabPos = style.field4c - extents.x;
	auto textStyle = it->second;
	ApplyStyle(style, tabPos, textStyle);

	// If the string contains an @ symbol, we need to assume it's a legacy formatted string that
	// we need to parse into the new format.
	bool isLegacyFormattedStr = std::find(text.begin(), text.end(), '@') != text.end();

	gfx::FormattedText formatted;
	if (isLegacyFormattedStr) {
		formatted = ProcessString(textStyle, style, text);
	} else {
		formatted.text = local_to_ucs2(to_string(text));
		formatted.defaultStyle = textStyle;
	}

	// Determine the real text width/height if necessary
	if (extents.width <= 0 || extents.height <= 0) {
		gfx::TextMetrics metrics;
		mTextEngine.MeasureText(formatted, metrics);
		if (extents.width <= 0) {
			extents.width = metrics.width;
		}
		if (extents.height <= 0) {
			extents.height = metrics.height;
		}
	}

	// Handle drawing of border/background
	if (style.flags & (TTSF_BACKGROUND | TTSF_BORDER)) {
		DrawBackgroundOrOutline(extents, style);
	}

	// Dispatch based on applied rotation
	if (style.flags & TTSF_ROTATE) {
		float angle = XMConvertToDegrees(style.rotation);
		XMFLOAT2 center{ 0,0 };
		if (style.flags & TTSF_ROTATE_OFF_CENTER) {
			center.x = style.rotationCenterX;
			center.y = style.rotationCenterY;
		}

		mTextEngine.RenderTextRotated(extents, angle, center, formatted);
	} else {
		mTextEngine.RenderText(extents, formatted);
	}

}

void TextLayouter::Measure(const TigFont &font, const TigTextStyle & style, TigFontMetrics & metrics)
{
	// Get the base text format and check if we should render using the new or old algorithms
	auto it = mMapping->find(font.name);
	if (it == mMapping->end()) {
		// use the old font drawing algorithm
		MeasureVanilla(font, style, metrics);
		return;
	}

	auto tabPos = style.field4c;
	auto textStyle = it->second;
	ApplyStyle(style, tabPos, textStyle);

	// Centering doesn't make sense for measuring if no width is given
	if (metrics.width == 0 && textStyle.align != TextAlign::Left) {
		textStyle.align = TextAlign::Left;
	}
	
	gfx::TextMetrics textMetrics;
	textMetrics.width = metrics.width;
	textMetrics.height = metrics.height;

	if (strchr(metrics.text, '@')) {
		auto formatted = ProcessString(textStyle, style, { metrics.text, (int)strlen(metrics.text) });
		mTextEngine.MeasureText(formatted, textMetrics);
	} else {
		mTextEngine.MeasureText(textStyle, metrics.text, textMetrics);
	}

	metrics.width = textMetrics.width;
	metrics.height = textMetrics.height;
	metrics.lineheight = textMetrics.lineHeight;
	metrics.lines = textMetrics.lines;
}

void TextLayouter::DrawBackgroundOrOutline(const TigRect& rect, const TigTextStyle& style) {
	
	float left = (float)rect.x;
	float top = (float)rect.y;
	float right = left + rect.width;
	float bottom = top + rect.height;

	left -= 3;
	top -= 3;
	right += 3;
	bottom += 3;

	if (style.flags & TTSF_BACKGROUND) {
		std::array<Vertex2d, 4> corners;
		corners[0].pos = XMFLOAT4(left, top, 0.5f, 1);
		corners[1].pos = XMFLOAT4(right, top, 0.5f, 1);
		corners[2].pos = XMFLOAT4(right, bottom, 0.5f, 1);
		corners[3].pos = XMFLOAT4(left, bottom, 0.5f, 1);

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

	if (style.flags & TTSF_BORDER) {
		XMFLOAT2 topLeft(left - 1, top - 1);
		XMFLOAT2 bottomRight(right + 1, bottom + 1);

		mShapeRenderer.DrawRectangleOutline(
			topLeft,
			bottomRight,
			XMCOLOR(0, 0, 0, 1.0f)
		);
	}

}

int TextLayouter::GetGlyphIdx(char ch, const char* text) {

	// First character found in the FNT files
	constexpr auto FirstFontChar = '!';
	const unsigned char FirstNonEnglish = 0xa0;
	const unsigned char FirstNonEnglishIdx = 92;
	auto tig_font_is_english = *gameSystemInitTable.fontIsEnglish; // TODO

	auto chUns = (unsigned char)ch;

	auto glyphIdx = chUns - FirstFontChar;

	
	if (chUns <= (unsigned char)'~')
		return glyphIdx;

	if (tig_font_is_english) {

		if (chUns >= FirstNonEnglish) {
			glyphIdx = chUns - ((unsigned char)FirstNonEnglish - FirstNonEnglishIdx);
		}
		else
		{
			switch(chUns)
			{
			case 0x82:
				return GetGlyphIdx(',', text);
			case 0x83:
				return GetGlyphIdx('f', text);
			case 0x84:
				return GetGlyphIdx(',', text);
			case 0x85: // elipsis
				return GetGlyphIdx(';', text);
			case 0x91:
			case 0x92:
				return GetGlyphIdx('\'', text);
			case 0x93:
			case 0x94:
				return GetGlyphIdx('"', text);
			case 0x95:
				return GetGlyphIdx('·', text);
			case 0x96:
			case 0x97:
				return GetGlyphIdx('-', text);
			default:
				return GetGlyphIdx('-', text); // speak english or die!!!
			}
		}

		if ((glyphIdx < -1 || ch > '~') && ch != '\n') {
			logger->warn("Tried to display character {} in text '{}'", glyphIdx, text);
			glyphIdx = -1;
		}
	}

	return glyphIdx;

}


std::pair<int, int> TextLayouter::MeasureCharRun(cstring_span<> text,
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
		else if (ch == '@' && nextCh == 't' && style.field4c > 0) {
			++it; // Skip the t

			// Treat tab stops as if they were whitespace, but instead of adding tracking,
			// add the space needed to reach the tab stop position
			if (lineWidth + wordWidth <= extentsWidth) {
				wordCount++;
				if (lineWidth + wordWidth <= extentsWidth + linePadding) {
					wordCountWithPadding++;
				}

				lineWidth += wordWidth;
				wordWidth = 0;

				// Increase the line width such that it continues at the tab stop location,
				// but do not move backwards (unsupported)
				auto currentX = extents.x + lineWidth;
				auto trackingNeeded = std::max<int>(0, style.field4c - currentX);
				lineWidth += trackingNeeded;
			}
			else {
				// Stop if we have run out of space on this line
				break;
			}
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
		else if ( ch <255 && ch > -1 && isspace(ch)) {
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
		else if ( ch  == '’') // special casing this motherfucker
		{
			ch ='\'';
			auto glyphIdx = GetGlyphIdx(ch, &text[0]);
			wordWidth += style.kerning + font.glyphs[glyphIdx].width_line;
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

bool TextLayouter::HasMoreText(cstring_span<> text, int tabWidth) {
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

		if (curChar == '@' && nextChar == 't'  && tabWidth > 0) {
			++it;
			continue;
		}

		if (curChar != '\n' && !isspace(curChar)) {
			return true;
		}
	}

	return false;
}

void TextLayouter::LayoutAndDrawVanilla(gsl::cstring_span<> text, const TigFont & font, TigRect & extents, TigTextStyle & style)
{
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

	if (style.flags & (TTSF_BACKGROUND | TTSF_BORDER)) {
		TigRect rect;
		rect.x = extents.x;
		rect.y = extents.y;
		rect.width = std::max<int>(extentsWidth, extents.width);
		rect.height = std::max<int>(extentsHeight, extents.height);
		DrawBackgroundOrOutline(rect, style);
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
	for (auto startOfWord = 0; startOfWord < textLength; ++startOfWord) {
		int wordsOnLine, lineWidth;
		std::tie(wordsOnLine, lineWidth) = MeasureCharRun(text.subspan(startOfWord),
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
				}
				else {
					if (!HasMoreText(text.subspan(lastIdx), tabWidth)) {
						wordInfo.drawEllipsis = false;
						wordWidth = wordInfo.fullWidth;
					}
				}
			}

			startOfWord = lastIdx;
			if (startOfWord + 1 < textLength && text[startOfWord] == '@' && text[startOfWord + 1] == 't') {
				// Extend the word width by the whitespace needed to move to the tabstop position from where
				// the current X position is (after the word we're currently rendering)
				wordWidth += std::max<int>(0, style.field4c - extents.x - (currentX + wordWidth));
				// Skip the "t" of "@t"
				startOfWord++;
			} else if (startOfWord < textLength &&  text[startOfWord] >= 0 &&  isspace(text[startOfWord])) {
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
			if ((int)wordInfo.firstIdx < 0 || (int)lastIdx < 0) {
				int dummy = 1;
				logger->error("Bad firstIdx at LayoutAndDraw! {}, {}", (int)wordInfo.firstIdx, (int)lastIdx);
			}
			else if (lastIdx >= wordInfo.firstIdx)
				mRenderer.RenderRun(
					text.subspan(wordInfo.firstIdx, lastIdx - wordInfo.firstIdx),
					x,
					currentY,
					extents,
					style,
					font);

			currentX += wordWidth;

			// We're on the last line, the word has been truncated, ellipsis needs to be drawn
			if (lastLine && style.flags & 0x4000 && wordInfo.drawEllipsis) {
				mRenderer.RenderRun(span(sEllipsis, strlen(sEllipsis)),
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

uint32_t TextLayouter::MeasureVanillaLine(const TigFont &font, const TigTextStyle &style, const char *text) const
{
	if (!text) {
		return 0;
	}

	auto result = 0;
	auto length = strlen(text);

	for (auto i = 0u; i < length; i++) {
		auto ch = text[i];

		// Skip @ characters if they are followed by a number between 0 and 9
		if (ch == '@' && i + 1 < length && text[i + 1] >= '0' && text[i + 1] <= '9') {
			i++;
			continue;
		}

		if ( ch >=0 && ch < 128 && isspace(ch)) {
			if (ch != '\n') {
				result += style.tracking;
			}
		} else {
			auto glyphIdx = GetGlyphIdx(ch, text);
			result += font.glyphs[glyphIdx].width_line + style.kerning;
		}
	}
	return result;
}

uint32_t TextLayouter::MeasureVanillaParagraph(const TigFont &font, const TigTextStyle &style, const char *text) const {

	// This is kinda bad... Text larger than 2000 chars will just crash
	assert(strlen(text) + 1 < 2000);
	char tempText[2000];
	strcpy(tempText, text);
	strcat(tempText, "\n");
	
	auto token = strtok(tempText, "\n");
	auto maxLineLen = 0u;
	
	while (token) {
		auto lineLen = MeasureVanillaLine(font, style, token);
		if (lineLen > maxLineLen) {
			maxLineLen = lineLen;
		}
		token = strtok(0, "\n");
	}

	return maxLineLen;
}

uint32_t TextLayouter::CountLinesVanilla(uint32_t maxWidth, uint32_t maxLines, const char * text, const TigFont & font, const TigTextStyle & style) const
{
	size_t length = strlen(text);

	if (length <= 0)
		return 1;

	auto lineWidth = 0u;
	auto lines = 1u;

	char ch;
	for (size_t i = 0; i < length; i++) {
		auto wordWidth = 0;
		
		// Measure the length of the current word
		for (; i < length; i++) {
			ch = text[i];
			if (ch == '’') // fix for this character that sometimes appears in vanilla
				ch = '\'';
			// Skip @[0-9]
			if (ch == '@' && i + 1 < length && text[i + 1] >= '0' && text[i + 1] <= '9') {
				i++;
				continue;
			}
			

			if ( ch < 255 && ch >=0){
				if (isspace(ch)) {
					break;
				}
			}

			auto glyphIdx = GetGlyphIdx(ch, text);
			wordWidth += font.glyphs[glyphIdx].width_line + style.kerning;
		}

		lineWidth += wordWidth;

		// If there's enough space in the maxWidth left and we're not at a newline
		// increase the linewidth and continue on.
		if (lineWidth <= maxWidth && ch != '\n') {
			if (ch < 255 && ch >= 0 && isspace(ch)) {
				lineWidth += style.tracking;
			}
			continue;
		}

		// We're either at a newline, or break the line here due to reaching the maxwidth
		lines++;

		// Reached the max number of lines -> quit
		if (maxLines && lines >= maxLines) {
			break;
		}

		if (lineWidth <= maxWidth) {
			// We reached a normal line break
			lineWidth = 0;
		} else {
			// We're breaking the line, so we'll keep the current word
			// width as the initial length of the new line
			lineWidth = wordWidth;
		}

		// Continuation indent
		if (style.flags & TTSF_CONTINUATION_INDENT) {
			lineWidth += 8 * style.tracking;
		}

		if (ch < 255 && ch >= 0 && isspace(ch)) {
			if (ch != '\n') {
				lineWidth += style.tracking;
			}
		}
	}

	return lines;
}

void TextLayouter::MeasureVanilla(const TigFont & font, const TigTextStyle & style, TigFontMetrics & metrics) const
{
	if (!metrics.width && strstr(metrics.text, "\n") && metrics.text) {
		metrics.width = MeasureVanillaParagraph(font, style, metrics.text);
	}

	if (!metrics.width)
	{
		metrics.width = MeasureVanillaLine(font, style, metrics.text);
		metrics.height = font.largestHeight;
		metrics.lines = 1;
		metrics.lineheight = font.largestHeight;
		return;
	}
		
	metrics.lines = 1; // Default
	if (metrics.height) {
		auto maxLines = metrics.height / font.largestHeight;
		if (!(style.flags & TTSF_TRUNCATE)) {
			maxLines++;
		}
		
		if (maxLines != 1) {
			metrics.lines = CountLinesVanilla(metrics.width, maxLines, metrics.text, font, style);
		}
	} else {
		if (!(style.flags & TTSF_TRUNCATE)) {
			metrics.lines = CountLinesVanilla(metrics.width, 0, metrics.text, font, style);
		}
	}

	if (!metrics.height) {
		metrics.height = metrics.lines * font.largestHeight;
		metrics.height -= - (font.baseline - font.largestHeight);
	}
	metrics.lineheight = font.largestHeight;
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

		if (curCh == '’')
			curCh = const_cast<char*>(text)[i] = '\'';

		// Simply skip @t without increasing the width
		if (curCh == '@' && isdigit(nextCh)) {
			i++; // Skip the number
			continue;
		}

		// @t will advance the width up to the next tabstop, but only if a tabstop has been specified
		if (curCh == '@' && nextCh == 't') {
			break; // Treat it as if it was whitespace
		}

		auto glyphIdx = GetGlyphIdx(curCh, &text[0]);

		if (curCh == '\n') {
			if (lastLine && style.flags & TTSF_TRUNCATE) {
				result.drawEllipsis = true;
			}
			break;
		}

		if ( curCh < 128 && curCh >0 && isspace(curCh)) {
			break;
		}

		if (style.flags & TTSF_TRUNCATE) {
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

