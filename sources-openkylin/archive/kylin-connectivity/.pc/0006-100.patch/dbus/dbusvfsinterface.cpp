#include "dbusvfsinterface.h"

DbusVfsInterface::DbusVfsInterface(MainWindow *window, QObject *parent) : QObject(parent), m_window(window)
{
    FileInfo::registerMetaType();
    FileInfoList::registerMetaType();
    connect(m_window, &MainWindow::sigConnected, this, &DbusVfsInterface::sigDeviceUpdate);
    connect(m_window, &MainWindow::sigDisconnect, this, [=]() { Q_EMIT sigDeviceUpdate(QString("")); });
    connect(m_window, &MainWindow::sigFileInfoList, this, [=](const QMap<QString, FileInfo> &fileMap) {
        m_fileMap.clear();
        m_fileMap = fileMap;
        FileInfoList list;
        QMap<QString, FileInfo>::const_iterator it = fileMap.begin();
        while (it != fileMap.end()) {
            list.addFileInfo(it.value());
            it++;
        }
        qInfo() << "Publish file list." << list.size();
        Q_EMIT sigFileInfoList(list);
    });
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

void DbusVfsInterface::getFileInfoList(QString path)
{
    qInfo() << "External request for file list." << path;
    m_window->list(path);
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
