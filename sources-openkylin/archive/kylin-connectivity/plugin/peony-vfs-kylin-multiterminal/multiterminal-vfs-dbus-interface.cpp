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

#include <QCoreApplication>

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
    moveToThread(qApp->thread());
    connect(this, &MultVfsDbusInterface::requestGetDeviceName, this, &MultVfsDbusInterface::getDeviceName, Qt::BlockingQueuedConnection);
    connect(this, &MultVfsDbusInterface::requestGetFileInfo, this, &MultVfsDbusInterface::getFileInfo, Qt::BlockingQueuedConnection);
    connect(this, &MultVfsDbusInterface::requestGetFileList, this, &MultVfsDbusInterface::getFileList, Qt::BlockingQueuedConnection);
    connect(this, &MultVfsDbusInterface::requestGetMountDir, this, &MultVfsDbusInterface::getMountDir, Qt::BlockingQueuedConnection);
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
    QObject::connect(m_sessionBusInterface, SIGNAL(sigDeviceUpdate(QString)), this, SLOT(slotDeviceUpdate(QString)));
}

QString MultVfsDbusInterface::getMountDir()
{
    if (QThread::currentThread() != qApp->thread()) {
        return requestGetMountDir();
    }

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
    if (QThread::currentThread() != qApp->thread()) {
        return requestGetDeviceName();
    }
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
    if (QThread::currentThread() != qApp->thread()) {
        return requestGetFileList(path);
    }

    QDir dir(m_mountDir + path);
    QStringList m_fileList = dir.entryList(QDir::AllEntries);
    m_fileList.removeOne(".");
    m_fileList.removeOne("..");
    m_oldPath = path;
    return m_fileList;
}

const FileInfo MultVfsDbusInterface::getFileInfo(QString filePath)
{
    if (QThread::currentThread() != qApp->thread()) {
        return requestGetFileInfo(filePath);
    }
    QFileInfo info(filePath);
    FileInfo fileinfo;
    fileinfo.setName(info.fileName());
    fileinfo.setPath(filePath);
    fileinfo.setSize(info.size());
    fileinfo.setDateTime(info.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
    if(info.isDir()) {
        fileinfo.setType(FileInfo::FileType::Dir);
    }
    
    return fileinfo;
}

void MultVfsDbusInterface::slotDeviceUpdate(QString deviceName)
{
    m_deviceName = deviceName;
    if (m_deviceName.isEmpty()) {
        m_fileList.clear();
        m_oldPath.clear();
    }
    Q_EMIT sigDeviceUpdate(deviceName);
}
