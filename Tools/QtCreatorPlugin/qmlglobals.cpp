
#include <QDebug>

#include "../../TemplePlus/qml/qmlglobals.h"

// Fake implementation for TPQMLGlobals
void TPQmlGlobals::playSound(int soundId) {
	qDebug() << "Playing sound" << soundId;
}
