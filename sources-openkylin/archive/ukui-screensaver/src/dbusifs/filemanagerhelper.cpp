#include "filemanagerhelper.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>

const static QString fileService = QStringLiteral("org.freedesktop.FileManager1");
const static QString fileManagerPath = QStringLiteral("/org/freedesktop/FileManager1");
const static QString fileManagerInterface = QStringLiteral("org.freedesktop.FileManager1");

FileManagerHelper::FileManagerHelper(QObject *parent)
    : QObject{parent}
{}

FileManagerHelper::~FileManagerHelper() {}

void FileManagerHelper::init()
{
    if (!m_fileManagerInterface) {
        m_fileManagerInterface = new QDBusInterface(
            fileService, fileManagerPath, fileManagerInterface, QDBusConnection::sessionBus(), this);
    }
}

bool FileManagerHelper::showFolders(QStringList path)
{
    init();
    QDBusPendingCall result = m_fileManagerInterface->asyncCall("ShowFolders", path, "");
    if (result.reply().type() == QDBusMessage::ErrorMessage) {
        qWarning() << "ShowFolders error!!!!!!!!!!!!!!";
        return false;
    }
    return true;
}
