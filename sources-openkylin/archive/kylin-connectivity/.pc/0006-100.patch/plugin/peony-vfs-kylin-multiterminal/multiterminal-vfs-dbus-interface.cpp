/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: huheng <huheng@kylinos.cn>
 *
 */

#include "multiterminal-vfs-dbus-interface.h"
#include <sys/syslog.h>
#include <QMutexLocker>
#include <QDebug>
#include <QThread>
#include <QDBusReply>
#include <QFile>
#include <QEventLoop>
#include <QTimer>
#include "filecollection.h"

#define KYLIN_CONNECTIVITY_PATH "/"
#define KYLIN_CONNECTIVITY_SERVICE "com.kylinos.Multiterminal.Manager"
#define KYLIN_CONNECTIVITY_INTERFACE "com.kylinos.Multiterminal.Manager"

MultVfsDbusInterface *MultVfsDbusInterface::getInstance()
{
    static MultVfsDbusInterface gInstance;
    return &gInstance;
}

MultVfsDbusInterface::MultVfsDbusInterface(QObject *parent) : QObject(parent), m_sessionBusInterface(nullptr)
{
    FileInfo::registerMetaType();
    FileInfoList::registerMetaType();
}

MultVfsDbusInterface::~MultVfsDbusInterface()
{
    if (m_sessionBusInterface) {
        delete m_sessionBusInterface;
        m_sessionBusInterface = nullptr;
    }
}

void MultVfsDbusInterface::initDbusData()
{
    if (m_sessionBusInterface == nullptr) {
        m_sessionBusInterface = new QDBusInterface(KYLIN_CONNECTIVITY_SERVICE, KYLIN_CONNECTIVITY_PATH,
                                                   KYLIN_CONNECTIVITY_INTERFACE, QDBusConnection::sessionBus());
    }
    // QObject::connect(m_sessionBusInterface, SIGNAL(sigFileInfoList(FileInfoList)), this,
    //                  SLOT(slotFileInfoList(FileInfoList)));
    QObject::connect(m_sessionBusInterface, SIGNAL(sigDeviceUpdate(QString)), this, SLOT(slotDeviceUpdate(QString)));
}

QString MultVfsDbusInterface::getMountDir()
{
    QString mountDir;
    if (!m_mountDir.isEmpty()) {
        return m_mountDir;
    }
    if (m_sessionBusInterface->isValid()) {
        QDBusMessage msg = m_sessionBusInterface->call("getMountDir");
        QList<QVariant> list = msg.arguments();
        if (!list.isEmpty()) {
            QVariant var = list.value(0);
            mountDir = var.value<QString>();
            m_mountDir = mountDir;
        }
    }

    return mountDir;
}

QString MultVfsDbusInterface::getDeviceName()
{
    QString deviceName;
    if (!m_deviceName.isEmpty()) {
        return m_deviceName;
    }
    if (m_sessionBusInterface->isValid()) {
        QDBusMessage msg = m_sessionBusInterface->call("getDeviceName");
        QList<QVariant> list = msg.arguments();
        if (!list.isEmpty()) {
            QVariant var = list.value(0);
            deviceName = var.value<QString>();
            m_deviceName = deviceName;
        }
    }

    return deviceName;
}

QStringList MultVfsDbusInterface::getFileList(QString path)
{
    QMutexLocker locker(&m_mutex);
    if (m_sessionBusInterface && m_sessionBusInterface->isValid()) {
        if (path == m_oldPath) {
            // 防止多次请求相同数据
            return m_fileList;
        }
        QEventLoop loop;
        FileCollection *fileCollection = new FileCollection(m_mountDir + path);
        QObject::connect(fileCollection, &FileCollection::finish, this, [=, &loop](FileInfoList list) {
            m_infoList = list;
            QStringList dirList;
            QStringList fileList;
            for (int i = 0; i < list.size(); i++) {
                FileInfo fileInfo = list.value(i);
                if (fileInfo.type() != FileInfo::FileType::Dir) {
                    fileList.append(fileInfo.name());
                } else {
                    dirList.append(fileInfo.name());
                }
            }
            for (int i = 0; i < fileList.size(); i++) {
                dirList.append(fileList.value(i));
            }
            m_fileList = dirList;
            loop.quit();
            fileCollection->deleteLater();
        });
        m_fileList.clear();
        m_infoList.clear();
        // m_sessionBusInterface->call("getFileInfoList", path);
        bool result = true;
        fileCollection->start();
        loop.exec();
        if (!result) {
            m_oldPath = path;
            return m_fileList;
        }
        m_oldPath = path;
        return m_fileList;
    }
    m_oldPath = path;
    return m_fileList;
}

const FileInfoList &MultVfsDbusInterface::getInfoList()
{
    return m_infoList;
}

const FileInfo MultVfsDbusInterface::getFileInfo(QString fileName)
{
    return m_infoList.value(fileName);
}

void MultVfsDbusInterface::slotDeviceUpdate(QString deviceName)
{
    m_deviceName = deviceName;
    if (m_deviceName.isEmpty()) {
        m_fileList.clear();
        m_infoList.clear();
        m_oldPath.clear();
    }
    Q_EMIT sigDeviceUpdate(deviceName);
}

// void MultVfsDbusInterface::slotFileInfoList(FileInfoList list)
// {
//     m_infoList = list;
//     QStringList dirList;
//     QStringList fileList;
//     for (int i = 0; i < list.size(); i++) {
//         FileInfo fileInfo = list.value(i);
//         if (fileInfo.type() != FileInfo::FileType::Dir) {
//             fileList.append(fileInfo.name());
//         } else {
//             dirList.append(fileInfo.name());
//         }
//     }
//     for (int i = 0; i < fileList.size(); i++) {
//         dirList.append(fileList.value(i));
//     }
//     m_fileList = dirList;
//     Q_EMIT sigInfoFinish();
// }

QString MultVfsDbusInterface::getCurrentPath()
{
    return m_oldPath;
}
