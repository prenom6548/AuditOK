/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#ifndef MOUNTBACKUPPROCESS_CPP
#define MOUNTBACKUPPROCESS_CPP

#include <QFile>
#include <QtDebug>
#include "mountbackupprocess.h"
#include "../../common/mydefine.h"
#include "../../common/utils.h"

MountBackupProcess::MountBackupProcess(QObject* parent)
    : QObject(parent)
    , m_p(new QProcess(this))
{
    connect(m_p, &QProcess::readyReadStandardError, this, [&]() {
        QByteArray err = m_p->readAllStandardError();
        qCritical("%s", err.data());
    });
}

bool MountBackupProcess::Do(const QString& diskUuid)
{
    QString rootPath(Utils::getBackupRootPath());

    // 若备份路径下的xml已经存在，则说明已经挂载过了
    QString backupXmlPath = rootPath + BACKUP_XML_PATH;
    backupXmlPath.replace("//", "/");
    QFile file(backupXmlPath);
    if (file.exists())
        return true;

    backupXmlPath = rootPath + SYS_UPDATE_BACKUP_XML_PATH;
    backupXmlPath.replace("//", "/");
    QFile xmlfile(backupXmlPath);
    if (xmlfile.exists())
        return true;

    QStringList arguments;
    QString backupPartion = rootPath + BACKUP_PATH;
    backupPartion.replace("//","/");
    arguments << QString("-o") << QString("rw") << QString("/dev/disk/by-uuid/%1").arg(diskUuid) << backupPartion;

    m_p->start("mount", arguments);
    if (!m_p->waitForStarted()) {
        qCritical("mount rw /backup process start failed!");
        return false;
    }

    if (!m_p->waitForFinished()) {
        qCritical("mount rw backup process end failed!");
        return false;
    }

    return true;
}

bool MountBackupProcess::umountBackupPartition()
{
    QString backupPath = Utils::getBackupRootPath() + BACKUP_PATH;
    backupPath.replace("//","/");
    QStringList arguments;
    arguments << "-R";
    arguments << backupPath;
    m_p->start("umount", arguments);
    if (!m_p->waitForStarted()) {
        qCritical("umount /backup process start failed!");
        return false;
    }

    if (!m_p->waitForFinished()) {
        qCritical("umount /backup process end failed!");
        return false;
    }

    return true;
}

/**
 * @brief 卸载目录挂载
 * @param mountPath，挂载的目录
 * @return
 */
bool MountBackupProcess::umount(const QString& mountPath)
{
    QStringList arguments;
    arguments << "-R";
    arguments << mountPath;
    m_p->start("umount", arguments);
    if (!m_p->waitForStarted()) {
        qCritical() << QString("umount %1 process start failed!").arg(mountPath);
        return false;
    }

    if (!m_p->waitForFinished()) {
        qCritical() << QString("umount %1 process end failed!").arg(mountPath);
        return false;
    }

    return true;
}

/**
 * @brief 挂载
 * @param source 源
 * @param target 目标目录
 * @return
 */
bool MountBackupProcess::mount(const QString& source, const QString& target, const QString& options)
{
    QStringList arguments;
    if (!options.isEmpty())
        arguments << options;
    arguments << source;
    arguments << target;

    m_p->start("mount", arguments);
    if (!m_p->waitForStarted()) {
        qCritical("mount process start failed!");
        return false;
    }

    if (!m_p->waitForFinished()) {
        qCritical("mount process end failed!");
        return false;
    }

    return true;
}

#endif // MOUNTBACKUPPROCESS_CPP
