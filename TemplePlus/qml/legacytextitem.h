
#pragma once

#include <QQuickPaintedItem>
#include <QString>

class LegacyTextItem : public QQuickPaintedItem {
Q_OBJECT
Q_PROPERTY(QString text READ getText WRITE setText NOTIFY textChanged)
public:
	LegacyTextItem(QQuickItem *parent = 0);
	void paint(QPainter *painter) override;

	QString getText() const;
	void setText(QString text);

private:
	QString mText;

signals:
	void textChanged();
};
