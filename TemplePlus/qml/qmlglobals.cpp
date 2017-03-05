
#include <QDebug>

#include "qmlglobals.h"

#include <sound.h>
#include "../config/config.h"
#include "movies.h"

struct MovieRecord {
	uint32_t movieId;
	int soundId;
};

static const std::vector<MovieRecord> sMovieRecords{
	{ 1000, 24 },
	{ 1009, -1 },
	{ 1007, -1 },
	{ 1012, -1 },
	{ 1002, -1 },
	{ 1015, -1 },
	{ 1005, -1 },
	{ 1010, -1 },
	{ 1004, -1 },
	{ 1013, -1 },
	{ 1006, -1 },
	{ 1016, -1 },
	{ 1001, -1 },
	{ 1011, -1 },
	{ 1008, -1 },
	{ 1014, -1 },
	{ 1003, -1 },
	{ 1017, -1 },
	{ 304, -1 },
	{ 300, -1 },
	{ 303, -1 },
	{ 301, -1 },
	{ 302, -1 },
	{ 1009, -1 }
};

void TPQmlGlobals::playSound(int soundId) {
	sound.MssPlaySound(soundId);
}

QJSValue TPQmlGlobals::cinematics()
{
	QString moviesSeen = QString::fromStdString(config.GetVanillaString("movies_seen"));

	auto arr = mEngine->newArray(0);
	size_t count = 0;

	for (size_t i = 0; i < sMovieRecords.size(); i++) {
		auto &movieRec = sMovieRecords[i];
		QString entry = QString("(%1,%2)").arg(movieRec.movieId).arg(movieRec.soundId);
		if (moviesSeen.contains(entry)) {
			QJSValue obj = mEngine->newObject();
			obj.setProperty("id", i);
			QString name = tr(qPrintable(QString("mainmenu:%1").arg(2000 + i)));
			obj.setProperty("name", name);
			arr.setProperty(count++, obj);
		}
	}

	return arr;
}

#include <QtGui/5.8.0/QtGui/private/qguiapplication_p.h>

void TPQmlGlobals::playCinematic(const QJSValue & cinematic)
{
	int index = cinematic.property("id").toInt();
	if (index < 0 || index >= (int) sMovieRecords.size()) {
		qDebug() << "Trying to play invalid movie: " << index;
	}

	auto &movieRecord = sMovieRecords[index];

	qDebug() << "Playing cinematic" << movieRecord.movieId << "Sound" << movieRecord.soundId;
	
	// Strip window focus so key events go to us
	QWindowSystemInterface::handleWindowActivated(nullptr);

	if (movieRecord.soundId != -1) {
		sound.PlaySound(movieRecord.soundId);
	}	

	movieFuncs.PlayMovieId(movieRecord.movieId, 0, 0);
}

bool TPQmlGlobals::qtCreator() const {
	return false;
}
