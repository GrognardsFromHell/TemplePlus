
#pragma once

#define QT_STATICPLUGIN

#include <QQmlExtensionPlugin>

#include "legacytextrenderer.h"

class TPQmlPlugin : public QQmlExtensionPlugin {
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface/1.0")
public:
	~TPQmlPlugin();

	void initializeEngine(QQmlEngine *engine, const char *uri) override;

	void registerTypes(const char *uri) override;
	
private:
	std::unique_ptr<LegacyTextRenderer> mLegacyTextRenderer;

};
