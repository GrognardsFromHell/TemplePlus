
#include "qmlglobals.h"

#include <sound.h>

void TPQmlGlobals::playSound(int soundId) {
	sound.MssPlaySound(soundId);
}
