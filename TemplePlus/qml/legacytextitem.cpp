
#include <QBrush>
#include <QPainter>

#include "legacytextitem.h"

LegacyTextItem::LegacyTextItem(QQuickItem *parent) : QQuickPaintedItem(parent) {

}

void LegacyTextItem::paint(QPainter *painter) {
	QBrush brush(QColor("#007430"));
	QPen pen(QColor("#ff0000"));

	painter->setBrush(brush);
	painter->setPen(pen);
	painter->setRenderHint(QPainter::Antialiasing);

	painter->drawRect(0, 0, 50, 50);	
}

QString LegacyTextItem::getText() const {
	return mText;
}

void LegacyTextItem::setText(QString text) {
	mText = text;
}
