/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "udiskdatarestoreproxy.h"
#include <QDateTime>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <unistd.h>
#include <sys/reboot.h>
#include "../common/utils.h"
#include "mymountproxy.h"
#include <QDBusInterface>
#include <QDBusPendingReply>

IMPLEMENT_DYNCREATE(UDiskDataRestoreProxy)

/**
 * @brief 构造函数
 */
UDiskDataRestoreProxy::UDiskDataRestoreProxy()
{
    m_bSuccess = false;
    m_isFinished = false;
    m_p = nullptr;
    m_isForce = false;
}

/**
 * @brief 析构函数
 */
UDiskDataRestoreProxy::~UDiskDataRestoreProxy()
{
    delete m_p;
}

/**
 * @brief 环境检测
 * @return false,检测失败;true,检测成功
 */
bool UDiskDataRestoreProxy::checkEnvEx()
{
    qDebug() << "UDiskDataRestoreProxy::checkEnvEx invoke begin";

    // 1、检测.user.txt是否存在
    m_userFile = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/" + PATHS_USER_FILE;
    if (!Utils::fileExists(m_userFile)) {
        qCritical(".user.txt文件不存在");
        emit checkResult(int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED));
        return false;
    }

    // 2、检测.exclude.user.txt是否存在
    m_excludeUserFile = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/" + EXCLUDE_PATHS_USER_FILE;
    if (!Utils::fileExists(m_excludeUserFile)) {
        qCritical(".exclude.user.txt文件不存在");
        emit checkResult(int(BackupResult::WRITE_EXCLUDE_BACKUP_PATHS_TO_USER_FAILED));
        return false;
    }

    // 3、检测还原点是否存在
    m_backupPath = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/data";
    if (Utils::isDirEmpty(m_backupPath)) {
        qCritical("还原点{uuid}/data目录不存在");
        emit checkResult(int(BackupResult::INC_NOT_FOUND_DIR));
        return false;
    }

    // 4、检测xml中的还原点是否还存在
    QString xmlPath = m_backupWrapper.m_prefixDestPath + BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    m_backupPoint = parse.findBackupPointByUuid(m_backupWrapper.m_uuid);
    if (m_backupPoint.m_uuid.isEmpty()) {
        qCritical("xml中还原点不存在");
        emit checkResult(int(BackupResult::INC_NOT_FOUND_DIR));
        return false;
    }

    emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));

    qDebug() << "UDiskDataRestoreProxy::checkEnvEx invoke end";
    return true;
}

/**
 * @brief 执行还原逻辑
 */
void UDiskDataRestoreProxy::doWorkEx()
{
    qDebug() << "UDiskDataRestoreProxy::doWorkEx invoke begin";

    // 1、校验
    if (!checkEnvEx())
        return ;

    // 2、还原数据
    restoreData();

    qDebug() << "UDiskDataRestoreProxy::doWorkEx invoke end";
}

/**
 * @brief 根据场景获取rsync命令参数
 * @param scene，场景
 * @return 组装好的rsync的参数信息
 */
QStringList UDiskDataRestoreProxy::getRsyncArgs(DataRestoreScene scene)
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
void UDiskDataRestoreProxy::restoreData()
{
    qDebug() << "UDiskDataRestoreProxy::restoreData invoke begin";

    QStringList args = getRsyncArgs(DataRestoreScene::DATA_RESTORE);

    m_srcPath = m_backupPath;
    QString destPath = Utils::getSysRootPath() + "/";
    destPath.replace("//", "/");
    args << m_srcPath + "/";
    args << destPath;

    m_p = new RsyncPathToDirProcess(this);
    connect(m_p, &RsyncPathToDirProcess::progress, this, &UDiskDataRestoreProxy::progress);
    connect(m_p, &RsyncPathToDirProcess::finished, this,  [&](bool result) {
        m_isForce = false;
        m_bSuccess = result;
        m_isFinished = true;
        if (result) {
            QString time = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");
            Utils::writeBackupLog(time + "," + m_backupWrapper.m_uuid + "," + QString::number(m_backupWrapper.m_type) + ",,," + QString::number(m_backupWrapper.m_frontUid)+ "," + m_backupPoint.m_backupName);
        }
        emit this->workResult(result);
    });

    QTimer::singleShot(1*1000, this, &UDiskDataRestoreProxy::checkUdiskExists);
    m_isFinished = false;
    m_bSuccess = false;
    m_p->start(args, false);

    qDebug() << "UDiskDataRestoreProxy::restoreData invoke end";
}

/**
 * @brief 监控移动设备是否还在
 * @return true-在；false-不在
 */
bool UDiskDataRestoreProxy::checkUdiskExists()
{
    if (!m_isFinished) {
        // 拔掉U盘后，没有响应的场景（怀疑可能是某应用程序关闭引起，希望不是dbus服务关掉了）
        if (m_isForce) {
            emit this->workResult(false);
            return false;
        }

        if (Utils::isDirEmpty(m_backupPath)) {
            if (m_p != nullptr)
                m_p->stop();
            // 10s钟后如果还没有退出，则强制退出
            QTimer::singleShot(10*1000, this, &UDiskDataRestoreProxy::checkUdiskExists);
            m_isForce = true;
        } else {
            QTimer::singleShot(1*1000, this, &UDiskDataRestoreProxy::checkUdiskExists);
        }
    }

    return true;
}

