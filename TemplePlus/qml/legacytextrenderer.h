
#pragma once

#include <memory>

#include <QMap>
#include <QVector>
#include <QLatin1String>
#include <QBrush>
#include <QImage>
#include <QSize>

#include <graphics/math.h>

class LegacyFontGlyph;

class LegacyFontGlyph {
public:
	QImage glyph;
	int widthLine;
	int widthLineXOffset;
	int baseLineYOffset;
};

class LegacyFont {
public:
	int baseline;
	int largestHeight;
	int fontsize;
	QVector<LegacyFontGlyph> glyphs;
};

struct LegacyTextStyle {
	int tracking;
	int kerning;
	bool dropShadow = false;
	QColor shadowColor;
	QColor textColor;
	bool gradient = false;
	QColor gradientColor;
};

class LegacyTextRenderer {
public:
	LegacyTextRenderer() {
		Q_ASSERT(!sInstance);
		sInstance = this;
	}
	~LegacyTextRenderer() {
		if (sInstance == this) {
			sInstance = nullptr;
		}
	}

	void AddFont(const std::string &name, const std::string &file);

	void RenderRun(int x,
		int y,
		const std::string &text,
		const LegacyTextStyle &style,
		const std::string &fontName,
		QPainter &painter);

	void RenderRun(int x,
		int y,
		const std::string &text,
		const LegacyTextStyle &style,
		const LegacyFont &font,
		QPainter &painter);

	QSize Measure(const std::string &text,
		const LegacyTextStyle &style,
		const LegacyFont &font);
	
	QSize Measure(const std::string &text,
		const LegacyTextStyle &style,
		const std::string &font);

	static LegacyTextRenderer &instance() {
		Q_ASSERT(sInstance);
		return *sInstance;
	}

private:
	QMap<std::string, LegacyFont> mFonts;

	void RenderChar(QPoint &pos,
		char ch,
		const LegacyTextStyle &style,
		const LegacyFont &font,
		QPainter &painter);
	
	static LegacyTextRenderer* sInstance;
};
