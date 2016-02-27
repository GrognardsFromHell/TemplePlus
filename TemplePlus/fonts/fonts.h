
#pragma once

#include <gsl/gsl.h>

struct ScanWordResult;
struct GlyphVertex2d;
struct TigFont;
struct TigTextStyle;
struct TigRect;
using namespace gsl;

namespace gfx {
	class RenderingDevice;
	class ShapeRenderer2d;
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
public:
	TextLayouter(gfx::RenderingDevice& device, gfx::ShapeRenderer2d& shapeRenderer);

	void LayoutAndDraw(gsl::cstring_span<> text, const TigFont &font, TigRect& extents, TigTextStyle& style);

private:
	void DrawBackground(const TigRect& rect, const TigTextStyle& style);
	int GetGlyphIdx(char ch, const char *text);
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

	static const char *sEllipsis;
	FontRenderer mRenderer;
	gfx::ShapeRenderer2d& mShapeRenderer;
};
