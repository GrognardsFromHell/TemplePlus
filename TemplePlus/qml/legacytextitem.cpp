
#include <QBrush>
#include <QPainter>

#include "legacytextitem.h"
#include "legacytextrenderer.h"

LegacyTextItem::LegacyTextItem(QQuickItem *parent) : QQuickPaintedItem(parent) {

	mFont = "Scurlock";

	mStyle.kerning = 0;
	mStyle.tracking = 10;

	mStyle.textColor = QColor(0, 100, 164);
	mStyle.gradient = true;
	mStyle.gradientColor = QColor(1, 65, 93);

	mStyle.dropShadow = true;
	mStyle.shadowColor = QColor(0, 0, 0, 128);

}

void LegacyTextItem::paint(QPainter *painter) {
	auto &renderer = LegacyTextRenderer::instance();
	renderer.RenderRun(0, 0, mText.toStdString(), mStyle, mFont, *painter);
}

QString LegacyTextItem::getText() const {
	return mText;
}

void LegacyTextItem::setText(QString text) {
	mText = text;
	
	auto size = LegacyTextRenderer::instance().Measure(mText.toStdString(), mStyle, mFont);
	setImplicitSize(size.width(), size.height());
}

void LegacyTextItem::setColor(QColor color)
{
	mStyle.textColor = color;
	update();
}

void LegacyTextItem::setGradient(bool enable)
{
	mStyle.gradient = enable;
	update();
}

void LegacyTextItem::setGradientColor(QColor color)
{
	mStyle.gradientColor = color;
	update();
}

void LegacyTextItem::setDropShadow(bool enable)
{
	mStyle.dropShadow = enable;
	update();
}

void LegacyTextItem::setDropShadowColor(QColor color)
{
	mStyle.shadowColor = color;
	update();
}
