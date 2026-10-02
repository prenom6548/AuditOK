/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "deletebackupproxy.h"
#include <QProcess>
#include <QDateTime>
#include <QRegularExpression>
#include <QDebug>
#include "../common/utils.h"

IMPLEMENT_DYNCREATE(DeleteBackupProxy)

DeleteBackupProxy::DeleteBackupProxy()
{}

DeleteBackupProxy::~DeleteBackupProxy()
{}

void DeleteBackupProxy::doWorkEx()
{
    qDebug() << "DeleteBackupProxy::doWorkEx invoke begin";

    // 1、删除/backup/snapshots/backuplist.xml对应项
    if (!deleteXmlBackupPoint())
        return;

    // 2、删除/backup/snapshots/{uuid}对应目录
    deleteBackupPointDir();

    // 3、同时删除唯一识别文件中对应的条目
    Utils::deleteBackupUniqueRecord(m_backupWrapper.m_backupName);

    // 删除成功信号
    emit workResult(true);

    qDebug() << "DeleteBackupProxy::doWorkEx invoke end";
}

/**
 * @brief 删除/backup/snapshots/backuplist.xml对应项
 */
bool DeleteBackupProxy::deleteXmlBackupPoint()
{
    qDebug() << "DeleteBackupProxy::deleteXmlBackupPoint invoke begin";

    QString xmlPath;
    if (m_backupWrapper.m_iPosition == BackupPosition::LOCAL || m_backupWrapper.m_iPosition == BackupPosition::CUSTOMIZE)
        xmlPath = Utils::getSysRootPath() + BACKUP_XML_PATH;
    else
        xmlPath = m_backupWrapper.m_prefixDestPath + BACKUP_XML_PATH;

    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    // 查询节点
    m_backupPoint = parse.findBackupPointByUuid(m_backupWrapper.m_uuid);

    // 删除节点
    if (m_backupPoint.isNull())
    {
        // 系统更新的备份点独立了出来
        if (m_backupWrapper.m_uuid == AUTO_BACKUP_UUID)
        {
            xmlPath = Utils::getSysRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
            xmlPath.replace("//", "/");
            ParseBackupList xmlparse(xmlPath);
            // 查询节点
            m_backupPoint = xmlparse.findBackupPointByUuid(m_backupWrapper.m_uuid);
            if (xmlparse.deleteItem(m_backupWrapper.m_uuid) != ParseBackupList::SUCCESS) {
                qCritical() << QString("failed to delete %1 from backuplist.xml or sys_update_backuplist.xml!").arg(m_backupWrapper.m_uuid);
                emit workResult(false);
                return false;
            }
        }
    }
    else
    {
        if (!m_backupWrapper.m_uuid.contains(QRegularExpression("^\\{.*-.*-.*-.*-.*\\}$")) || parse.deleteItem(m_backupWrapper.m_uuid) != ParseBackupList::SUCCESS) {
            qCritical() << QString("failed to delete %1 from backuplist.xml!").arg(m_backupWrapper.m_uuid);
            emit workResult(false);
            return false;
        }
    }

    qDebug() << "DeleteBackupProxy::deleteXmlBackupPoint invoke end";
    return true;
}

/**
 * @brief 删除/backup/snapshots/uuid对应目录
 */
void DeleteBackupProxy::deleteBackupPointDir()
{
    qDebug() << "DeleteBackupProxy::deleteBackupPointDir invoke begin";

    QString uuidPath;
    if (m_backupPoint.m_iPosition == BackupPosition::LOCAL)
        uuidPath = Utils::getSysRootPath() + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid;
    else if (m_backupPoint.m_iPosition == BackupPosition::CUSTOMIZE)
        uuidPath = m_backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid;
    else
        uuidPath = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid;

    uuidPath.replace("//", "/");

    QStringList args;
    args << "-rf";
    args << uuidPath;
    QProcess::execute("rm", args);

    QString time = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");
    if (   int(BackupType::BACKUP_DATA) == m_backupPoint.m_type
        || int(BackupType::INC_BACKUP_DATA) == m_backupPoint.m_type) {
        Utils::writeBackupLog(time + "," + m_backupWrapper.m_uuid + "," + QString::number(m_backupWrapper.m_type) + ",,," + QString::number(m_backupWrapper.m_frontUid)+ "," + m_backupPoint.m_backupName);
    } else {
        Utils::writeBackupLog(time + "," + m_backupWrapper.m_uuid + "," + QString::number(m_backupWrapper.m_type) + ",,,," + m_backupPoint.m_backupName);
    }

    qDebug() << "DeleteBackupProxy::deleteBackupPointDir invoke end";
}

