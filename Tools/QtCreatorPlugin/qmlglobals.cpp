
#include <QDebug>

#include "../../TemplePlus/qml/qmlglobals.h"

// Fake implementation for TPQMLGlobals
void TPQmlGlobals::playSound(int soundId) {
	qDebug() << "Playing sound" << soundId;
}

QJSValue TPQmlGlobals::cinematics()
{
	auto arr = mEngine->newArray(5);

	for (int i = 0; i < 5; i++) {
		QJSValue obj = mEngine->newObject();
		obj.setProperty("id", i);
		obj.setProperty("name", QString("Cinematic %1").arg(i + 1));
		arr.setProperty(i, obj);
	}

	return arr;
}

void TPQmlGlobals::playCinematic(const QJSValue &value) {
	qDebug() << "Playing cinematic";
}
