#ifndef DBUSVFSINTERFACE_H
#define DBUSVFSINTERFACE_H

#define KYLIN_CONNECTIVITY_PATH "/"
#define KYLIN_CONNECTIVITY_SERVICE "com.kylinos.Multiterminal.Manager"
#define KYLIN_CONNECTIVITY_INTERFACE "com.kylinos.Multiterminal.Manager"

#include <QObject>
#include "fileinfo.h"
#include "mainwindow.h"

class DbusVfsInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", KYLIN_CONNECTIVITY_SERVICE)
public:
    DbusVfsInterface(MainWindow *window, QObject *parent = nullptr);

Q_SIGNALS:
    void sigDeviceUpdate(QString);

public Q_SLOTS:
    QString getDeviceName();
    QString getMountDir();
    void downFile(QStringList fromPathList, QString targetPath);

private:
    QMap<QString, FileInfo> m_fileMap;  // 缓存当前目录文件信息
    MainWindow *m_window = nullptr;
};

#endif  // DBUSVFSINTERFACE_H
