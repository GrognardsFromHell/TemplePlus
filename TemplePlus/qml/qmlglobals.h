
#pragma once

#include <QObject>

/**
 * This object is used as the root context for all QML components. This means
 * any property or method on this object is accessible from all QML components.
 */
class TPQmlGlobals : public QObject {
Q_OBJECT
public:

	Q_INVOKABLE void playSound(int soundId);

};
