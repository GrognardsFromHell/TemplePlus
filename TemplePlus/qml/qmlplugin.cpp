
#include <QQmlEngine>
#include <QFontDatabase>

#include <infrastructure/vfs.h>

#include "qmlplugin.h"

#include "legacytextitem.h"
#include "networkaccessmanager.h"

void TPQmlPlugin::initializeEngine(QQmlEngine *engine, const char *uri) {

	// Add pre-defined fonts
	auto fonts = vfs->Search("fonts/*.ttf");
	for (auto &font : fonts) {
		std::string filename = fmt::format("fonts/{}", font.filename);
		auto fontData(vfs->ReadAsBinary(filename));
		logger->info("Adding font {}", filename);
		QByteArray fontDataQt((char*)fontData.data(), fontData.size());
		int id = QFontDatabase::addApplicationFontFromData(fontDataQt);
		if (id == -1) {
			logger->warn("Unable to add font {}", filename);
		}
	}
}

void TPQmlPlugin::registerTypes(const char *uri) {
	qmlRegisterType<LegacyTextItem>(uri, 1, 0, "LegacyText");
	qmlRegisterTypeNotAvailable(uri, 1, 0, "DummyType", "This is a dummy type, do not use.");
}

Q_IMPORT_PLUGIN(TPQmlPlugin);
