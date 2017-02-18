
#include <infrastructure/vfs.h>

#include <QDataStream>
#include <QFileInfo>
#include <QDir>
#include <QPainter>

#include "legacytextrenderer.h"

#include <infrastructure/exception.h>

static int GetGlyphIdx(char ch, const char* text) {

	// First character found in the FNT files
	constexpr auto FirstFontChar = '!';

	auto chUns = (unsigned char)ch;

	return chUns - FirstFontChar;

}

std::unique_ptr<LegacyTextRenderer> legacyTextRenderer;

struct GlyphPos {
	int x;
	int y;
	int w;
	int h;
	int fileIdx;
};

LegacyTextRenderer* LegacyTextRenderer::sInstance = nullptr;

void LegacyTextRenderer::AddFont(const std::string &name, const std::string& file)
{

	if (mFonts.contains(name)) {
		throw TempleException("Font {} already registered", name);
	}

	auto fntData(vfs->ReadAsBinary(file));
	QByteArray fntDataQt = QByteArray::fromRawData((char*) &fntData[0], fntData.size());

	QDataStream fntIn(&fntDataQt, QIODevice::ReadOnly);
	fntIn.setByteOrder(QDataStream::LittleEndian);

	int glyphCount, numberOfFiles, antiAliased, nameLength;
	LegacyFont &font = mFonts[name];
	fntIn >> font.baseline
		>> glyphCount
		>> numberOfFiles
		>> font.largestHeight
		>> font.fontsize
		>> antiAliased
		>> nameLength;
	std::string artFilename(nameLength, '\0');
	fntIn.readRawData(&artFilename[0], nameLength);

	font.glyphs.resize(glyphCount);

	QVector<GlyphPos> glyphPos(glyphCount);

	for (int i = 0; i < glyphCount; i++) {
		auto &glyph = font.glyphs[i];

		fntIn >> glyphPos[i].x
			>> glyphPos[i].y
			>> glyphPos[i].w
			>> glyphPos[i].h
			>> glyphPos[i].fileIdx
			>> glyph.widthLine
			>> glyph.widthLineXOffset
			>> glyph.baseLineYOffset;

	}

	QFileInfo fileInfo(QString::fromStdString(file));
	auto baseDir = fileInfo.dir();

	// Read in all the font image data files referenced by the FNT file
	for (int i = 0; i < numberOfFiles; i++) {
		auto filename = baseDir.filePath(QString("%1_%2.fntart").arg(QString::fromStdString(artFilename)).arg(i, 4, 10, QLatin1Char('0'))).toStdString();
		auto fntartData(vfs->ReadAsBinary(filename));
		
		QImage fntartImage(256, 256, QImage::Format_ARGB32);

		size_t idx = 0;
		for (int y = 0; y < 256; y++) {
			for (int x = 0; x < 256; x++) {
				uint8_t alpha = fntartData[idx++];

				fntartImage.setPixelColor(x, y, QColor(255, 255, 255, alpha));
			}
		}

		// Now that we have the font image loaded, go through all glyphs 
		// and extract their respective images
		for (int glyphIdx = 0; glyphIdx < font.glyphs.size(); glyphIdx++) {
			auto &pos = glyphPos[glyphIdx];

			if (pos.fileIdx != i) {
				continue; // Not in this file
			}

			auto &glyph = font.glyphs[glyphIdx];
			glyph.glyph = fntartImage.copy(pos.x, pos.y, pos.w, pos.h);
		}
	}

}

void LegacyTextRenderer::RenderRun(int x, int y, const std::string & text, const LegacyTextStyle & style, const std::string & fontName, QPainter & painter)
{
	auto it = mFonts.constFind(fontName);
	if (it == mFonts.end()) {
		return;
	}

	return RenderRun(x, y, text, style, it.value(), painter);
}

void LegacyTextRenderer::RenderRun(int x, int y, const std::string &text, const LegacyTextStyle & style, const LegacyFont &font, QPainter &painter)
{

	QPoint pos(x, y);
	for (auto ch : text) {
		RenderChar(pos, ch, style, font, painter);
	}

}

QSize LegacyTextRenderer::Measure(const std::string & text, const LegacyTextStyle &style, const LegacyFont &font)
{

	int w = 0;
	int h = font.baseline;
	for (auto ch : text) {
		if (ch < 128 && isspace(ch)) {
			w += style.tracking;
			continue;
		}

		int glyphIdx = GetGlyphIdx(ch, "");

		if (glyphIdx >= font.glyphs.size()) {
			continue;
		}
		
		auto &glyph = font.glyphs[glyphIdx];

		int fullHeight = font.baseline - glyph.baseLineYOffset + glyph.glyph.height();
		if (fullHeight > h) {
			h = fullHeight;
		}

		w += style.kerning + glyph.widthLine;
	}

	return{ w, h };

}

QSize LegacyTextRenderer::Measure(const std::string & text, const LegacyTextStyle & style, const std::string & font)
{
	auto it = mFonts.constFind(font);
	if (it == mFonts.end()) {
		return{ 0, 0 };
	}
	
	return Measure(text, style, it.value());
}

static void applyBrushImage(const LegacyFont &font, const LegacyFontGlyph &glyph, QImage &img, const QColor &from, const QColor &to) {
	
	for (int y = 0; y < img.height(); y++) {
		float f = y / float(img.height());
		int r = int(from.red() + (to.red() - from.red()) * f);
		int g = int(from.green() + (to.green() - from.green()) * f);
		int b = int(from.blue() + (to.blue() - from.blue()) * f);

		auto pixels = (QRgb*)img.scanLine(y);
		for (int x = 0; x < img.width(); x++) {
			auto alpha = qAlpha(pixels[x]);
			if (alpha != 0) {
				pixels[x] = qRgba(r, g, b, alpha);
			}
		}
	}

}

static void applyBrushColor(QImage &img, QColor color) {

	for (int y = 0; y < img.height(); y++) {
		auto pixels = (QRgb*)img.scanLine(y);
		for (int x = 0; x < img.width(); x++) {
			auto alpha = qAlpha(pixels[x]);
			if (alpha != 0) {
				pixels[x] = qRgba(color.red(), color.green(), color.blue(), alpha);
			}
		}
	}

}

void LegacyTextRenderer::RenderChar(QPoint &pos, char ch, const LegacyTextStyle & style, const LegacyFont & font, QPainter & painter)
{
	if (ch < 128 && isspace(ch)) {
		pos.rx() += style.tracking;
		return;
	}

	int glyphIdx = GetGlyphIdx(ch, "");
	
	if (glyphIdx >= font.glyphs.size()) {
		return; // Trying to render invalid character
	}

	auto &glyph = font.glyphs[glyphIdx];
	
	QPoint dest{
		pos.x(),
		pos.y() + font.baseline - glyph.baseLineYOffset
	};

	QImage img = glyph.glyph.copy();

	if (style.dropShadow) {
		applyBrushColor(img, style.shadowColor);
		painter.drawImage(dest.x() + 1, dest.y() + 1, img);
	}
	
	if (style.gradient) {
		applyBrushImage(font, glyph, img, style.textColor, style.gradientColor);
	} else {
		applyBrushColor(img, style.textColor);
	}

	painter.drawImage(dest, img);

	pos.rx() += style.kerning + glyph.widthLine;
	
}
