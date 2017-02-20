
#pragma once

#include <QObject>
#include <QJSValue>
#include <QJSEngine>

/**
 * This object is used as the root context for all QML components. This means
 * any property or method on this object is accessible from all QML components.
 */
class TPQmlGlobals : public QObject {
Q_OBJECT
Q_PROPERTY(QJSValue cinematics READ cinematics)
public:
	TPQmlGlobals(QJSEngine *engine) : mEngine(engine) {}

	Q_INVOKABLE void playSound(int soundId);

	QJSValue cinematics();

	Q_INVOKABLE void playCinematic(const QJSValue &cinematic);

private:
	QJSEngine *mEngine;

};
