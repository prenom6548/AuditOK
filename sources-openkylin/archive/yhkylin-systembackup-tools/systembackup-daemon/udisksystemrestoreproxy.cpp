/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "udisksystemrestoreproxy.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <unistd.h>
#include <sys/reboot.h>
#include "../common/utils.h"
#include "mymountproxy.h"
#include "translations.h"



IMPLEMENT_DYNCREATE(UDiskSystemRestoreProxy)

/**
 * @brief 构造函数
 */
UDiskSystemRestoreProxy::UDiskSystemRestoreProxy()
{
    m_bSuccess = false;
    m_isOem = false;
    m_extraProcess = nullptr;
}

/**
 * @brief 析构函数
 */
UDiskSystemRestoreProxy::~UDiskSystemRestoreProxy()
{
}

/**
 * @brief 环境检测
 * @return false,检测失败;true,检测成功
 */
bool UDiskSystemRestoreProxy::checkEnvEx()
{
    qDebug() << "UDiskSystemRestoreProxy::checkEnvEx invoke begin";
    emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));
    qDebug() << "UDiskSystemRestoreProxy::checkEnvEx invoke end";
    return true;

    // 1、检测xml中的还原点是否还存在
    QString xmlPath = m_backupWrapper.m_prefixDestPath + UDISK_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    qDebug()<<__LINE__<<__func__<<"U盘备份点："<<m_backupWrapper.m_uuid;
    m_backupPoint = parse.findBackupPointByUuid(m_backupWrapper.m_uuid);
    if (m_backupPoint.isNull())
    {
        qCritical("xml中还原点不存在");
        emit checkResult(int(BackupResult::INC_NOT_FOUND_DIR));
        return false;
    }

    //查询对应的目录commit哈希
    parse.getXmlCommitHashMap(m_backupPoint.m_systemHash,m_systemCommitHashMap,m_backupWrapper.m_uuid);
    if (m_systemCommitHashMap.size() == 0)
    {
        qCritical("存储点中没有对应哈希");
        emit checkResult(int(BackupResult::CHECK_COMMITHASH_FAIL));
        return false;
    }

    // 4、检测还原点是否存在

    //1.检测ostree仓库是否存在
    m_pOstree = new OstreeInterface(this);
    m_pOstree->setOperation(OstreeInterface::Operation::INIT);
    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::INIT);
    if(!m_pOstree->checkOstreeRepo("",OstreeInterface::BackupType::SYSTEM))
    {
        return false;
    }

    m_curUuid = m_backupWrapper.m_uuid;

    emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));

    qDebug() << "UDiskSystemRestoreProxy::checkEnvEx invoke end";
    return true;
}

/**
 * @brief 执行还原逻辑
 */
void UDiskSystemRestoreProxy::doWorkEx()
{
    qDebug() << "UDiskSystemRestoreProxy::doWorkEx invoke begin";

    // 1、校验
    if (!checkEnvEx()){
        emit this->workResult(false);
        return ;
    }
    emit this->workResult(true);
    return;

    // 2、还原系统
    restoreSystem();

    qDebug() << "UDiskSystemRestoreProxy::doWorkEx invoke end";
}

/**
 * @brief 备份准备
 * @return true,准备成功；false，准备失败
 */
bool UDiskSystemRestoreProxy::doPrepare()
{
    qDebug() << "UDiskSystemRestoreProxy::doPrepare invoke begin";


    // 2、停止安全初始化服务，以防过高的CPU占有率，因为还原时安全初始化服务会逐个文件打标记，造成cpu占有率超高系统卡顿

    // 停止网络服务，以防网络更新
    //Utils::stopNetwork();

    qDebug() << "UDiskSystemRestoreProxy::doPrepare invoke end";
    return true;
}

/**
 * @brief 将dst.img文件挂载到/backup/imgbackup目录
 * @return
 */
bool UDiskSystemRestoreProxy::mountImg()
{
    // 自定义路径系统备份，需要先将dst.img文件挂载到/backup/imgbackup目录
    // 1、检测目录/backup/imgbackup是否存在，不存在则创建此目录
    QString dstImgMountPath = Utils::getBackupRootPath() + BACKUP_IMGBACKUP_PATH;
    dstImgMountPath.replace("//", "/");
    Utils::mkpath(dstImgMountPath);

    // 2、先卸载/backup/imgbackup上的mount
    MountBackupProcess *processMount = new MountBackupProcess(this);
    processMount->umount(dstImgMountPath);

    // 3、将img文件挂载到/backup/imgbackup上
    if (!processMount->mount(m_imgFileName, dstImgMountPath)) {
        emit checkResult(int(BackupResult::RESTOREDIR_PREPARE_FAILED));
        delete processMount;
        return false;
    }

    m_srcPath = dstImgMountPath;

    delete processMount;

    return true;
}

/**
 * @brief 根据场景获取rsync命令参数
 * @param scene，场景
 * @return 组装好的rsync的参数信息
 */
QStringList UDiskSystemRestoreProxy::getRsyncArgs(CustomizeSystemRestoreScene scene)
{
    QStringList args;
    args << "-avAHXr";
    args << "--info=progress2";
    args << "--no-inc-recursive";
    args << "--ignore-missing-args";
    args << "--delete";

    QStringList excludes;

    switch (scene) {
    case CustomizeSystemRestoreScene::RESTORE_SYSTEM_WITH_DATA :
        args << "--exclude=/home";
        args << "--exclude=/root";
        args << "--exclude=/data";

        if (m_curUuid == FACTORY_BACKUP_UUID) {
            qDebug() << "FACTORY_BACKUP_UUID";
//            if (Utils::isDirExist(m_srcPath + "/home/oem"))
            {
                qDebug() << "Exists /home/oem";
                args << "--exclude=/etc/lightdm/lightdm.conf";
                args << "--exclude=/etc/localtime";
                args << "--exclude=/usr/share/zoneinfo";
                args << "--exclude=/etc/default/locale";
                args << "--exclude=/usr/share/i18n";
            }
        }
        // 保留指纹数据，用户密码、角色、权限、生物识别等信息不需要改变
        args << "--exclude=/var/lib/biometric-auth";
        args << "--exclude=/data/sec_storage_data";
        args << "--exclude=/etc/passwd";
        args << "--exclude=/etc/shadow";
        args << "--exclude=/etc/group";
        args << "--exclude=/etc/gshadow";
        args << "--exclude=/etc/sudoers";
        args << "--exclude=/data/home";
        args << "--exclude=/data/root";

        // 云图片作为桌面背景的路径属于用户数据
        args << "--exclude=/var/lib/AccountsService";

        // 域用户相关信息，还原后保持不退域
        args << "--exclude=/etc/sssd";
        args << "--exclude=/var/lib/sss";
        args << "--exclude=/usr/share/sssd";
        args << "--exclude=/etc/ipa";
        args << "--exclude=/etc/krb5.keytab";
        args << "--exclude=/etc/krb5.conf";
        args << "--exclude=/var/lib/ipa-client";
        args << "--exclude=/etc/nsswitch.conf";
        args << "--exclude=/etc/pam.d";
        args << "--exclude=/etc/hosts";
        args << "--exclude=/etc/hostname";
        args << "--exclude=/etc/hedron";
        args << "--exclude=/etc/kcm";
        args << "--exclude=/usr/hedron/hedronagent";
        args << "--exclude=/etc/.kyinfo";
        args << "--exclude=/etc/LICENSE";
        args << "--exclude=/etc/ssl/certs";
        args << "--exclude=/usr/share/ca-certificates";
        args << "--exclude=/etc/NetworkManager";
        args << "--exclude=/var/lib/pam";
        args << "--exclude=/etc/kim";
        args << "--exclude=/var/lib/kim";
        args << "--exclude=/etc/systemd/system/multi-user.target.wants/kimbackend.service";
        args << "--exclude=/var/lib/polkit-1/localauthority/40-domain.d";
        args << "--exclude=/lib/*/libsss_sudo.so";

        // 安装kylin
        args << "--exclude=/usr/share/applications/kylin-os-installer.desktop";
        args << "--exclude=*/.local/share/applications/kylin-os-installer.desktop";
        args << "--exclude=/etc/xdg/autostart/kylin-os-installer.desktop";

        // 系统更新的下面两个目录不要还原
        if (m_curUuid == AUTO_BACKUP_UUID) {
          //  args << "--exclude=/var/lib/kylin-system-updater";
           // args << "--exclude=/var/cache/kylin-system-updater";
            args << "--exclude=/var/lib/kylin-system-updater/remain";

        }

        // 此处不要break，因为还需要排除SYSTEM_RESTORE中的项

    case CustomizeSystemRestoreScene::SYSTEM_RESTORE :
        // 还原工具不还原自身
        args << "--exclude=/usr/bin/kylin-backup-daemon";
        args << "--exclude=/usr/bin/backup_mount_fstab";
        args << "--exclude=/usr/bin/restore-system";
        args << "--exclude=/usr/share/initramfs-tools/hooks/kylin-backup-hooks";
        args << "--exclude=/usr/share/initramfs-tools/scripts/init-bottom/kylin-backup";
        args << "--exclude=/usr/share/initramfs-tools/conf-hooks.d/kylin-backup";
        args << "--exclude=/etc/extra_backup_or_restore.conf";

        if (Utils::getBackupRootPath() != CASPER_ROOT_PATH) {
            // 试用模式时可以还原/target下面的备份还原工具
            args << "--exclude=/usr/bin/backup-daemon";
            args << "--exclude=/usr/bin/kybackup";
        }
        args << "--exclude=/usr/bin/mount_fstab_efi";
        args << "--exclude=/usr/bin/backup-auto-efi";
        args << "--exclude=/usr/bin/backup-auto";
        args << "--exclude=/usr/bin/rsync";
        args << "--exclude=/usr/share/rsync";
        args << "--exclude=/usr/share/initramfs-tools/hooks/kybackup-hooks";
        args << "--exclude=/usr/share/initramfs-tools/scripts/*/kybackup";

        // 还原后仍然保持激活状态
        args << "--exclude=/etc/LICENSE";
        args << "--exclude=/etc/.kyinfo";
        args << "--exclude=/etc/.kyactivation";
        args << "--exclude=/etc/.kyhwid";

        args << "--exclude=/usr/share/plymouth";
        // 文件安全箱
        args << "--exclude=/data/security-dir";

        // 自定义备份的路径也需要跳过，不进行还原
        Utils::excludeCustomizePath(excludes);
        args << "--exclude=*/backup/snapshots";
        for (const QString& item : excludes) {
            args << QString("--exclude=") + item;
        }

        args << "--exclude-from" << m_excludeUserFile;
        args << "--files-from" << m_userFile;

        break ;
    case CustomizeSystemRestoreScene::RESTORE_EXTRA_DATA :
        args << "--files-from" << m_extraUserFile;

        break ;
    default:
        break ;
    }

    return args;
}

/**
 * @brief 系统还原
 */
void UDiskSystemRestoreProxy::restoreSystem()
{
    qDebug() << "UDiskSystemRestoreProxy::restoreSystem invoke begin";

    // 还原前准备
    doPrepare();

    //接入ostree
    m_pOstree = new OstreeInterface(this);
    //设置工作类型为还原
    m_pOstree->setTaskType(OstreeInterface::TaskType::RESTORE);
    qDebug() << "设置工作类型为:还原" ;
    QString repoPath = m_pOstree->getRepoPath("", OstreeInterface::BackupType::SYSTEM);

    //检查repo点是否存在
    if(!m_pOstree->checkOstreeRepo("",OstreeInterface::BackupType::SYSTEM))
    {
        //不存在检测repo失败
        emit checkResult(int(BackupResult::CHECK_REPO_FAIL));
    }

    // 绑定对应的信号与槽
    connect(m_pOstree, &OstreeInterface::progress, this, &UDiskSystemRestoreProxy::progress);
    connect(m_pOstree, &OstreeInterface::systemHash, this, [&](QString systemHash){
        m_systemHash = systemHash;
    });
    connect(m_pOstree, &OstreeInterface::hashNumber, this, [&](QString numberStr){
        //数据获取从1开始，但是部署点排序从0开始，所以序列号减1
        int number = numberStr.toInt() - 1;
        m_hashNumber = QString::number(number);
        qDebug() << "回滚哈希对应的序列号为：" << m_hashNumber;
    });
    connect(m_pOstree, &OstreeInterface::finished, this,  [&](bool result) {
        if (result)
        {
            OstreeInterface::Operation oper =  m_pOstree->getOperation();
            if(oper == OstreeInterface::Operation::RM)
            {
                //不相同的时候，需要重新切换系统分支hash
                if(!m_bSameSystem)
                {
                    //调用不可变接口，获取当前系统分支哈希
                    qDebug() << "调用不可变接口，获取当前系统分支哈希";
                    QString res_hash = m_backupPoint.m_systemHash;
                    QStringList systemHashArgs = m_pOstree->getSystemRollBackArgs(m_hashNumber);
                    m_pOstree->setOperation(OstreeInterface::Operation::ROLLBACK);
                    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::ROLLBACK);
                    m_pOstree->start(systemHashArgs, true);
                }
                else
                {
                    this->rsyncFinished();
                    //补足进度信号，并等待3秒
                    emit progress(100);
                    Utils::wait(3);

                    emit this->workResult(result);
                    qDebug() << "系统版本无变化，准备重启！！！";
                    //reboot(RB_AUTOBOOT);
                }
            }
            else if(oper == OstreeInterface::Operation::ROLLBACK)
            {
                this->rsyncFinished();
                //补足进度信号，并等待3秒
                emit progress(100);
                Utils::wait(3);

                emit this->workResult(result);
                qDebug() << "系统版本有变化，完成ROLLBACK操作,准备重启！！！";
                //reboot(RB_AUTOBOOT);
            }


        }
//        else
//        {
//            emit this->workResult(result);
//            if (m_isOem) {
//                this->plymountMessage(Translations::instance().tr("Restore failed"));
//            }
//        }
    });

    qDebug() << "针对系统可变目录的还原操作:开始";


    //调用不可变接口，获取当前系统分支哈希
    QStringList systemHashArgs= m_pOstree->getSystemHashArgs();
    m_pOstree->setOperation(OstreeInterface::Operation::STATUS);
    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::STATUS);
    m_pOstree->start(systemHashArgs, true);

    //ovl路径前置
    QString prePath ;
    prePath = "/ostree/pkgs/ovl-";

    //先判断当前系统分支哈希是否有变化
    if(m_systemHash == m_backupPoint.m_systemHash)
    {
        m_bSameSystem = true;
    }
    else
    {
        m_bSameSystem = false;
        //调用不可变接口，获取要回滚的系统分支哈希number
        QStringList systemGetNumArgs= m_pOstree->getHashNumberArgs(m_backupPoint.m_systemHash);
        m_pOstree->setOperation(OstreeInterface::Operation::STATUS_TO_NUM);
        qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::STATUS_TO_NUM);
        m_pOstree->start(systemGetNumArgs, true);
    }

    qDebug() << "m_systemHash:" << m_systemHash;
    qDebug() << "m_backupPoint.m_systemHash:" << m_backupPoint.m_systemHash;

    qDebug() << m_systemCommitHashMap.size() << " : m_systemCommitHashMap ";
    for (auto it = m_systemCommitHashMap.constBegin(); it != m_systemCommitHashMap.constEnd(); ++it)
    {
        qDebug() << it.key() << ":" << it.value();

        //先设定备份步骤，用于计算备份进度
        m_pOstree->setBackUpStep(it.key());
        qDebug() << "设置备份步骤为:" << it.key() ;

        //先执行checkout导出
        QStringList checkoutArgs= m_pOstree->getCheckoutArgs(repoPath, it.key() ,it.value());
        m_pOstree->setOperation(OstreeInterface::Operation::CHECKOUT);
        qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::CHECKOUT);
        m_pOstree->setOperationPath(it.key());
        m_pOstree->start(checkoutArgs, true);

        //执行rsync同步
        QString srcPath = TEMP_RESTORE_PATH + it.key();
        QString descPath ;
        if(it.key() == "home")
        {
            m_pOstree->setRsyncScense(OstreeInterface::RsyncScense::RESTORE_HOMEHIDDENDATA);
            descPath = "/data/home/" + m_backupWrapper.m_frontUserName;
        }
        else
        {
            m_pOstree->setRsyncScense(OstreeInterface::RsyncScense::RESTORE_SYSTEM);
            descPath = prePath + m_backupPoint.m_systemHash + "/" + it.key() + "-ovl";
            qDebug() << "descPath" << descPath;

            //不相同的时候，ovl回滚路径不存在，需要创建路径
//            if(!m_bSameSystem)
//            {
//                if (!Utils::mkpath(descPath)) {
//                    qCritical() << QString("mkdir %1 failed !").arg(descPath) ;
//                    return ;
//                }
//            }
        }

        QStringList rsyncArgs= m_pOstree->getRsyncRepoArgs(srcPath, descPath);
        m_pOstree->setOperation(OstreeInterface::Operation::RSYNC);
        qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::RSYNC);
        m_pOstree->setOperationPath(it.key());
        m_pOstree->start(rsyncArgs, true);
    }

    qDebug() << "针对系统可变目录的还原操作:结束";

    //执行rm 清理
    QStringList rmArgs= m_pOstree->getRmArgs(TEMP_RESTORE_PATH);
    m_pOstree->setOperation(OstreeInterface::Operation::RM);
    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::RM);
    m_pOstree->start(rmArgs, true);

//    // 系统升级的备份点还原时，为保障断电后开机能继续还原，此处增加标志文件；还原成功后删除此文件；还原失败或断电还原未完成时，不删除此文件，等开启机器时init阶段根据是否存在标志文件继续进行系统还原。
//    if (m_curUuid == AUTO_BACKUP_UUID) {
//        QString rollbackFile = Utils::getBackupRootPath() + FLAG_FILE_SYSTEM_UPDATE_NEED_ROLLBACK;
//        rollbackFile.replace("//", "/");
//        Utils::syncWriteFile(rollbackFile, QString("UUID=") + AUTO_BACKUP_UUID);
//    }

    //执行完毕，设置工作类型为空闲
    m_pOstree->setTaskType(OstreeInterface::TaskType::IDLE);
    qDebug() << "UDiskSystemRestoreProxy::restoreSystem invoke end";
}

/**
 * @brief 还原额外数据
 */
void UDiskSystemRestoreProxy::restoreExtraData()
{
    qDebug() << "UDiskSystemRestoreProxy::restoreExtraData invoke begin";

    m_extraDir = Utils::getBackupRootPath() + m_backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid + "/extra_data";
    m_extraDir.replace("//", "/");
    if (Utils::isDirEmpty(m_extraDir)){
        qDebug() << m_extraUserFile << " is empty";
        return ;
    }

    m_extraUserFile = Utils::getBackupRootPath() + m_backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid + "/" + EXTRA_PATHS_USER_FILE;
    m_extraUserFile.replace("//", "/");
    if (!Utils::fileExists(m_extraUserFile)) {
        qDebug() << m_extraUserFile << " not exists";
        return ;
    }

    QStringList args = getRsyncArgs(UDiskSystemRestoreProxy::RESTORE_EXTRA_DATA);

    m_srcPath = m_backupPath;
    QString destPath = Utils::getBackupRootPath() + "/";
    destPath.replace("//", "/");
    args << m_extraDir + "/";
    args << destPath;

    m_extraProcess = new RsyncPathToDirProcess(this);
    m_extraProcess->start(args, false);

    qDebug() << "UDiskSystemRestoreProxy::restoreExtraData invoke end";
}

