
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <QQmlEngine>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStringList>
#include <QLibrary>

#include "plugin.h"
#include "../../TemplePlus/ui/ui_qtquick_nam.h"

#include <infrastructure/vfs.h>
#include <temple/vfs.h>

void TioIntegration::initializeEngine(QQmlEngine *engine, const char *uri)
{
    // Figure out where we are in terms of path
    static int localVar = 0;
    HMODULE module;
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR) &localVar, &module)) {
        qDebug() << "Failed to get my own module with: " << GetLastError();
        return;
    }

    char pathRaw[MAX_PATH];
    GetModuleFileNameA(module, pathRaw, MAX_PATH);

    QString path = QFile::decodeName(pathRaw);
    qDebug() << "Determined path " << path;
    QFileInfo fileInfo(path);
    QString configFile = fileInfo.absoluteDir().absoluteFilePath("TioIntegration.ini");
    qDebug() << "Loading " << configFile;

    QSettings settings(configFile, QSettings::IniFormat);

    QString toeePath = settings.value("toeePath").toString();

    qDebug() << "Using ToEE Path: " << toeePath;

    if (toeePath.isEmpty() || !QDir(toeePath).exists()) {
        MessageBoxA(nullptr, "Configure toeePath in TioIntegration.ini", "Config Error", MB_OK|MB_ICONERROR);
        return;
    }

    QDir toeeDir(QDir::toNativeSeparators(toeePath));

    // This will put the ToEE dir into the search order and allow the zlib to be found
    auto dllPath = QDir::toNativeSeparators(toeeDir.absolutePath()).toStdWString();
    SetDllDirectoryW(dllPath.c_str());

    QString tioPath = QDir::toNativeSeparators(toeeDir.absoluteFilePath("tio.dll"));
    QLibrary *lib = new QLibrary(tioPath, engine);
    if (!lib->load()) {
        qDebug() << "Unable to load " << tioPath << ": " << lib->errorString();
        return;
    }

    QStringList archives;
    archives << toeeDir.absoluteFilePath("ToEE1.dat");
    archives << toeeDir.absoluteFilePath("ToEE2.dat");
    archives << toeeDir.absoluteFilePath("ToEE3.dat");
    archives << toeeDir.absoluteFilePath("ToEE4.dat");

    try {
        vfs = std::make_unique<temple::TioVfs>();
        auto tioVfs = (temple::TioVfs*)vfs.get();

        for (auto &archive : archives) {
            qDebug() << "Adding archive " << archive;
            tioVfs->AddPath(archive.toStdString());
        }

        qDebug() << "Registering Network Access Manager";
        engine->setNetworkAccessManagerFactory(new CustomNAMFactory);
    } catch (std::exception &e) {
        qDebug() << "Failed to install NAM: " << e.what();
    }
}

void TioIntegration::registerTypes(const char *uri)
{
    qmlRegisterTypeNotAvailable(uri, 1, 0, "DummyType", "This is just a dummy type");
}
