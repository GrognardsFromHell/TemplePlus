
#pragma once

#include <gsl/gsl.h>

struct ScanWordResult;
struct GlyphVertex2d;
struct TigFont;
struct TigTextStyle;
struct TigRect;
struct TigFontMetrics;
using namespace gsl;

namespace gfx {
	class RenderingDevice;
	class ShapeRenderer2d;
	class TextEngine;
}

class FontRenderer {
public:
	explicit FontRenderer(gfx::RenderingDevice& g);
	~FontRenderer();

	void RenderRun(cstring_span<> text,
		int x,
		int y,
		const TigRect& bounds,
		TigTextStyle& style,
		const TigFont& font);

private:

	void RenderGlyphs(const GlyphVertex2d* vertices2d, int textureId, int glyphCount);
	static void Rotate2d(float& x, float& y,
		float rotCos, float rotSin,
		float centerX, float centerY);

	struct Impl;
	std::unique_ptr<Impl> mImpl;

};

/*
Separates a block of text given flags into words split up
on lines and renders them.
*/
class TextLayouter {
	friend class FontRenderer;
public:
	TextLayouter(gfx::RenderingDevice& device, gfx::ShapeRenderer2d& shapeRenderer);
	~TextLayouter();

	void LayoutAndDraw(gsl::cstring_span<> text, const TigFont &font, TigRect& extents, TigTextStyle& style);

	void Measure(const TigFont &font, const TigTextStyle &style, TigFontMetrics &metrics);

	static int GetGlyphIdx(char ch, const char *text);

private:
	void DrawBackgroundOrOutline(const TigRect& rect, const TigTextStyle& style);
	ScanWordResult ScanWord(const char* text,
		int firstIdx,
		int textLength,
		int tabWidth,
		bool lastLine,
		const TigFont& font,
		const TigTextStyle& style,
		int remainingSpace);
	std::pair<int, int> TextLayouter::MeasureCharRun(cstring_span<> text,
		const TigTextStyle& style,
		const TigRect& extents,
		int extentsWidth,
		const TigFont& font,
		int linePadding,
		bool lastLine);
	bool HasMoreText(cstring_span<> text, int tabWidth);

	void LayoutAndDrawVanilla(gsl::cstring_span<> text, 
		const TigFont &font, 
		TigRect& extents, 
		TigTextStyle& style);

	void MeasureVanilla(const TigFont &font, 
		const TigTextStyle &style, 
		TigFontMetrics &metrics) const;

	uint32_t MeasureVanillaLine(const TigFont &font, const TigTextStyle &style, const char *text) const;
	uint32_t MeasureVanillaParagraph(const TigFont &font, const TigTextStyle &style, const char *text) const;
	uint32_t CountLinesVanilla(uint32_t maxWidth, uint32_t maxLines, const char *text, const TigFont &font, const TigTextStyle &style) const;

	static const char *sEllipsis;
	gfx::TextEngine &mTextEngine;
	FontRenderer mRenderer;
	std::unique_ptr<class FontsMapping> mMapping;
	gfx::ShapeRenderer2d& mShapeRenderer;
};
