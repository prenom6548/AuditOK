#include "dbusvfsinterface.h"

DbusVfsInterface::DbusVfsInterface(MainWindow *window, QObject *parent) : QObject(parent), m_window(window)
{
    FileInfo::registerMetaType();
    connect(m_window, &MainWindow::sigConnected, this, &DbusVfsInterface::sigDeviceUpdate);
    connect(m_window, &MainWindow::sigDisconnect, this, [=]() { Q_EMIT sigDeviceUpdate(QString("")); });
}

QString DbusVfsInterface::getDeviceName()
{
    qInfo() << "Get current external connection request name.";
    return m_window->getDeviceName();
}

QString DbusVfsInterface::getMountDir()
{
    qInfo() << "External request to get the current mount directory.";
    return MOUNT_DIR;
}

void DbusVfsInterface::downFile(QStringList fromPathList, QString targetPath)
{
    qInfo() << "External request for download file." << targetPath;
    QList<FileInfo> list;
    for (int i = 0; i < fromPathList.size(); i++) {
        list.append(m_fileMap.find(fromPathList.value(i)).value());
    }
    m_window->downFile(list, targetPath);
}
