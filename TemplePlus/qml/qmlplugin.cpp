
#define _CRT_SECURE_NO_WARNINGS

#include <QQmlEngine>
#include <QQmlContext>
#include <QFontDatabase>

#include <infrastructure/vfs.h>

#include "qmlplugin.h"
#include "qmlglobals.h"

#include "legacytextitem.h"
#include "networkaccessmanager.h"
#include "mestranslator.h"
#include "qmlsystemfonts.h"

#include <infrastructure/format.h>
#include <infrastructure/logging.h>
#include <spdlog/sinks/file_sinks.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>

TPQmlPlugin::~TPQmlPlugin() = default;

#ifdef QTCREATORPLUGIN
#include <QLibrary>
#include <QDir>

#include <infrastructure/folderutils.h>
#include <temple/vfs.h>

#include <infrastructure/INI.h>
typedef INI <std::string, std::string, std::string> ini_t;

#include <objbase.h>

// Bits of hacky Windows API to find the path to this plugin's DLL
static QDir GetPluginPath() {
	HMODULE module;
	static int varHandle; // Just used for an address in this DLLs address space
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)&varHandle, &module);

	wchar_t filename[MAX_PATH];
	auto filenameSize = GetModuleFileName(module, filename, MAX_PATH);

	QString dllFilename = QString::fromWCharArray(filename, filenameSize);

	QFileInfo fileInfo(dllFilename);
	return fileInfo.absoluteDir();
}

#endif

void TPQmlPlugin::initializeEngine(QQmlEngine *engine, const char *uri) {

	qDebug() << "Starting QML plugin for TemplePlus";

#ifdef QTCREATORPLUGIN

    // Initialize COM if it hasn't happened yet
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (!SUCCEEDED(hr)) {
        qDebug() << "Unable to initialize COM: " << hr;
    } else {
        qDebug() << "Initialized COM";
    }

	QDir configDir(QString::fromStdWString(GetUserDataFolder()));
	QString configFile(configDir.absoluteFilePath("TemplePlus.ini"));

	if (!QFile::exists(configFile)) {
		qDebug() << "Couldn't find config file" << configFile << ". Please launch TP at least once.";
		return;
	}

	qDebug() << "Loading config from" << configFile;

	ini_t ini(configFile.toStdString(), true);

	if (!ini.select("TemplePlus")) {
		qDebug() << "TemplePlus doesn't seem to be configured correctly. Please launch TP at least once to configure your ToEE directory.";
		return;
	}

	auto toeeDir = QDir(QString::fromStdString(ini.get("toeeDir")));

	qDebug() << "Using ToEE from" << toeeDir.path();

	// It's necessary to add the dir to the DLL search path for zlib
	SetDllDirectory(toeeDir.path().toStdWString().c_str());

	auto library = new QLibrary(toeeDir.absoluteFilePath("tio.dll"), engine);
	if (!library->load()) {
		qDebug() << "Unable to load tio.dll:" << library->errorString();
		return;
	}
	
	SetDllDirectory(nullptr);

	vfs = std::make_unique<temple::TioVfs>();

	// Add the standard archives we'd expect (we can skip the module)
	auto tioVfs = (temple::TioVfs*)vfs.get();
	tioVfs->AddPath(toeeDir.absoluteFilePath("ToEE1.dat").toStdString());
	tioVfs->AddPath(toeeDir.absoluteFilePath("ToEE2.dat").toStdString());
	tioVfs->AddPath(toeeDir.absoluteFilePath("ToEE3.dat").toStdString());
	tioVfs->AddPath(toeeDir.absoluteFilePath("ToEE4.dat").toStdString());
	tioVfs->AddPath(toeeDir.absoluteFilePath("data").toStdString());

	// Tough one... try finding tpdata heh
	auto pluginPath = GetPluginPath();
	if (!pluginPath.exists()) {
		qDebug() << "Unable to determine path of QML plugin" << pluginPath;
		return;
	}
	do {
		
		if (pluginPath.cd("tpdata")) {
			break;
		}

	} while (pluginPath.cdUp());
	
	qDebug() << "Using tpdata path:" << pluginPath.absolutePath();
	tioVfs->AddPath(pluginPath.absolutePath().toStdString());

	// Init logging
	spdlog::drop_all(); // Reset all previous loggers
	auto nullSink = std::make_shared<spdlog::sinks::null_sink_mt>();
	logger = spdlog::create("core", { nullSink });

	static TPNetworkAccessManagerFactory namFactory;

	engine->setNetworkAccessManagerFactory(&namFactory);

#endif

	// Install translator
	QCoreApplication::installTranslator(new MesTranslator(engine));

	// Get Arial from the system instead of shipping it ourselves
	InstallQmlSystemFonts();

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

	// Also add the needed fonts to the legacy renderer
	mLegacyTextRenderer = std::make_unique<LegacyTextRenderer>();
	mLegacyTextRenderer->AddFont("Scurlock", "art/interface/FONTS/SCURLOCK/scurlock-48/scurlock-48.fnt");

	// Expose many global properties to QML
	mGlobals = std::make_unique<TPQmlGlobals>(engine);
	engine->rootContext()->setContextObject(mGlobals.get());

}

void TPQmlPlugin::registerTypes(const char *uri) {
	qDebug() << "Registering QML types";
	qmlRegisterType<LegacyTextItem>(uri, 1, 0, "LegacyText");
	qmlRegisterTypeNotAvailable(uri, 1, 0, "DummyType", "This is a dummy type, do not use.");
}

#ifndef QTCREATORPLUGIN
Q_IMPORT_PLUGIN(TPQmlPlugin);
#endif
