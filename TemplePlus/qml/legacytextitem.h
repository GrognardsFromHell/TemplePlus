
#pragma once

#include <QQuickPaintedItem>
#include <QString>

#include "legacytextrenderer.h"

class LegacyTextItem : public QQuickPaintedItem {
Q_OBJECT
Q_PROPERTY(QString text READ getText WRITE setText NOTIFY textChanged)
Q_PROPERTY(QColor color READ getColor WRITE setColor NOTIFY colorChanged)
Q_PROPERTY(bool gradient READ gradient WRITE setGradient NOTIFY gradientChanged)
Q_PROPERTY(QColor gradientColor READ getGradientColor WRITE setGradientColor NOTIFY gradientColorChanged)
Q_PROPERTY(bool dropShadow READ dropShadow WRITE setDropShadow NOTIFY dropShadowChanged)
Q_PROPERTY(QColor dropShadowColor READ getDropShadowColor WRITE setDropShadowColor NOTIFY dropShadowColorChanged)
public:
	LegacyTextItem(QQuickItem *parent = 0);
	void paint(QPainter *painter) override;

	QString getText() const;
	void setText(QString text);

	QColor getColor() const {
		return mStyle.textColor;
	}
	void setColor(QColor color);

	bool gradient() const {
		return mStyle.gradient;
	}
	void setGradient(bool enable);

	QColor getGradientColor() const {
		return mStyle.gradientColor;
	}
	void setGradientColor(QColor color);

	bool dropShadow() const {
		return mStyle.dropShadow;
	}
	void setDropShadow(bool enable);
	
	QColor getDropShadowColor() const {
		return mStyle.shadowColor;
	}
	void setDropShadowColor(QColor color);

private:
	QString mText;
	LegacyTextStyle mStyle;
	std::string mFont;

signals:
	void textChanged();
	void colorChanged();
	void gradientChanged();
	void gradientColorChanged();
	void dropShadowChanged();
	void dropShadowColorChanged();

};
