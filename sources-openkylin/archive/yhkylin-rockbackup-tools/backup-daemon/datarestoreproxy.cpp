/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "datarestoreproxy.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <unistd.h>
#include <sys/reboot.h>
#include "../common/utils.h"
#include "mymountproxy.h"
#include "parsebackuplist.h"
#include <QDBusInterface>
#include <QDBusPendingReply>

IMPLEMENT_DYNCREATE(DataRestoreProxy)

/**
 * @brief 构造函数
 */
DataRestoreProxy::DataRestoreProxy()
{
    m_bSuccess = false;
    m_p = nullptr;
}

/**
 * @brief 析构函数
 */
DataRestoreProxy::~DataRestoreProxy()
{
    delete m_p;
}

/**
 * @brief 环境检测
 * @return false,检测失败;true,检测成功
 */
bool DataRestoreProxy::checkEnvEx()
{
    qDebug() << "DataRestoreProxy::checkEnvEx invoke begin";

    if (m_backupWrapper.m_iPosition == BackupPosition::CUSTOMIZE)
        m_prePath = m_backupWrapper.m_prefixDestPath;
    else
        m_prePath = Utils::getSysRootPath();

    // 1、检测.user.txt是否存在
    m_userFile = m_prePath + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/" + PATHS_USER_FILE;
    m_userFile.replace("//", "/");
    if (!Utils::fileExists(m_userFile)) {
        qCritical(".user.txt文件不存在");
        emit checkResult(int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED));
        return false;
    }

    // 2、检测.exclude.user.txt是否存在
    m_excludeUserFile = m_prePath + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/" + EXCLUDE_PATHS_USER_FILE;
    m_excludeUserFile.replace("//", "/");
    if (!Utils::fileExists(m_excludeUserFile)) {
        qCritical(".exclude.user.txt文件不存在");
        emit checkResult(int(BackupResult::WRITE_EXCLUDE_BACKUP_PATHS_TO_USER_FAILED));
        return false;
    }

    // 3、检测还原点是否存在
    m_backupPath = m_prePath + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/data";
    m_backupPath.replace("//", "/");
    if (Utils::isDirEmpty(m_backupPath)) {
        qCritical("还原点{uuid}/data目录不存在");
        emit checkResult(int(BackupResult::INC_NOT_FOUND_DIR));
        return false;
    }

    // 4、检测xml中的还原点是否还存在
    QString xmlPath = Utils::getSysRootPath() + BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    m_backupPoint = parse.findBackupPointByUuid(m_backupWrapper.m_uuid);
    if (m_backupPoint.m_uuid.isEmpty()) {
        qCritical("xml中还原点不存在");
        emit checkResult(int(BackupResult::INC_NOT_FOUND_DIR));
        return false;
    }

    emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));

    qDebug() << "DataRestoreProxy::checkEnvEx invoke end";
    return true;
}

/**
 * @brief 执行还原逻辑
 */
void DataRestoreProxy::doWorkEx()
{
    qDebug() << "DataRestoreProxy::doWorkEx invoke begin";

    // 1、校验
    if (!checkEnvEx())
        return ;

    // 2、还原数据
    restoreData();

    qDebug() << "DataRestoreProxy::doWorkEx invoke end";
}

/**
 * @brief 根据场景获取rsync命令参数
 * @param scene，场景
 * @return 组装好的rsync的参数信息
 */
QStringList DataRestoreProxy::getRsyncArgs(DataRestoreScene scene)
{
    QStringList args;
    args << "-avAHXr";
    args << "--info=progress2";
    args << "--no-inc-recursive";
    args << "--ignore-missing-args";
    args << "--delete";

    switch (scene) {
    case DataRestoreScene::DATA_RESTORE :
        args << "--files-from" << m_userFile;

        break ;
    default:
        return args;
    }

    // 文件保护箱
    QDBusInterface interface("com.kylin.boxmanage", "/backup", "com.kylin.boxmanage.backup", QDBusConnection::systemBus());

    // 调用D-Bus方法
    QDBusMessage reply = interface.call("get_allBoxRootEntries");

    // 检查回复是否有效
    if (reply.type() == QDBusMessage::ReplyMessage) {
        // 提取返回的数据
        QStringList stringList = reply.arguments().at(0).toStringList();

        // 检查列表是否为空，并打印结果
        if (!stringList.isEmpty()) {
            qDebug() << "获取到的 Box Root Entries 是:" << stringList;
            for (int i = 0; i < stringList.size(); ++i) { // 添加了对size()的检查
                QString PathBox = stringList.at(i); // 使用.at()而不是()，虽然()在这里也可以工作，但.at()在索引无效时会抛出更明确的异常
                args << QString("--exclude=") + PathBox;
            }
        } else {
            qDebug() << "没有获取到 Box Root Entries。";
        }
    } else {
        qDebug() << "D-Bus 方法调用失败:" << reply.errorMessage();
    }

    return args;
}

/**
 * @brief 数据还原
 */
void DataRestoreProxy::restoreData()
{
    QStringList args = getRsyncArgs(DataRestoreScene::DATA_RESTORE);

    m_srcPath = m_backupPath;
    QString destPath = Utils::getSysRootPath() + "/";
    destPath.replace("//", "/");
    args << m_srcPath + "/";
    args << destPath;

    m_p = new RsyncPathToDirProcess(this);
    connect(m_p, &RsyncPathToDirProcess::progress, this, &DataRestoreProxy::progress);
    connect(m_p, &RsyncPathToDirProcess::finished, this,  [&](bool result) {
        if (result) {
            QString time = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");
            Utils::writeBackupLog(time + "," + m_backupWrapper.m_uuid + "," + QString::number(m_backupWrapper.m_type) + ",,," + QString::number(m_backupWrapper.m_frontUid)+ "," + m_backupPoint.m_backupName);
        }
        emit this->workResult(result);
    });

    m_p->start(args, false);
}


