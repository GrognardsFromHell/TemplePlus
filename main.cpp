
#include "system.h"
#include "temple_functions.h"
#include "libraryholder.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (a.arguments().size() != 2) {
        qDebug() << "Usage: program <toee-dir>";
        return -1;
    }

    QString toeeDir = a.arguments().at(1);
    QFileInfo toeeDirInfo(toeeDir);
    toeeDir = toeeDirInfo.absoluteFilePath();

    // Set DLL search path and current directory
    if (!SetDllDirectoryW((LPWSTR)toeeDir.utf16())) {
        qDebug() << "Unable to set DLL directory.";
    }
    if (!SetCurrentDirectoryW((LPWSTR)toeeDir.utf16())) {
        qDebug() << "Unable to change working directory.";
    }

    // Initialize minhook
    MH_Initialize();

    LibraryHolder templeDll("temple.dll");

    if (!templeDll.valid()) {
        qDebug() << "Unable to load temple.dll from" << toeeDir;
        qDebug() << "Error:" << templeDll.errorText();
        return -2;
    }

    init_functions();
    init_hooks();

    // Get entry point
    temple_main = templeDll.getFun<_temple_main>("temple_main");

    QByteArray localCmdLineStart = ("\"" + toeeDir + "toee.exe\" -window").toLocal8Bit();
    auto ourModule = GetModuleHandleW(NULL);
    temple_main(ourModule, NULL, localCmdLineStart.data(), SW_SHOWDEFAULT);

    MH_Uninitialize();

    return a.exec();
}
