/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "autosystemrestoreproxy.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <unistd.h>
#include <sys/reboot.h>
#include "../common/utils.h"
#include "mymountproxy.h"
#include "translations.h"



IMPLEMENT_DYNCREATE(AutoSystemRestoreProxy)

/**
 * @brief 构造函数
 */
AutoSystemRestoreProxy::AutoSystemRestoreProxy()
{
    m_bSuccess = false;
    m_isOem = false;
    m_extraProcess = nullptr;
}

/**
 * @brief 析构函数
 */
AutoSystemRestoreProxy::~AutoSystemRestoreProxy()
{
}

/**
 * @brief 环境检测
 * @return false,检测失败;true,检测成功
 */
bool AutoSystemRestoreProxy::checkEnvEx()
{
    qDebug() << "AutoSystemRestoreProxy::checkEnvEx invoke begin";

    // 1、检测xml中的还原点是否还存在
    QString xmlPath = Utils::getBackupRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    qDebug()<<__LINE__<<__func__<<"自动备份点："<<m_backupWrapper.m_uuid;
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
    if(!m_pOstree->checkOstreeRepo("",OstreeInterface::BackupType::SYSTEM_UPDATE))
    {
        return false;
    }

    m_curUuid = m_backupWrapper.m_uuid;

    emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));

    qDebug() << "AutoSystemRestoreProxy::checkEnvEx invoke end";
    return true;
}

/**
 * @brief 执行还原逻辑
 */
void AutoSystemRestoreProxy::doWorkEx()
{
    qDebug() << "AutoSystemRestoreProxy::doWorkEx invoke begin";

    // 1、校验
    if (!checkEnvEx())
        return ;

    // 2、还原系统
    restoreSystem();

    qDebug() << "AutoSystemRestoreProxy::doWorkEx invoke end";
}

/**
 * @brief 备份准备
 * @return true,准备成功；false，准备失败
 */
bool AutoSystemRestoreProxy::doPrepare()
{
    qDebug() << "AutoSystemRestoreProxy::doPrepare invoke begin";


    // 2、停止安全初始化服务，以防过高的CPU占有率，因为还原时安全初始化服务会逐个文件打标记，造成cpu占有率超高系统卡顿

    // 停止网络服务，以防网络更新
    //Utils::stopNetwork();

    qDebug() << "AutoSystemRestoreProxy::doPrepare invoke end";
    return true;
}

/**
 * @brief 将dst.img文件挂载到/backup/imgbackup目录
 * @return
 */
bool AutoSystemRestoreProxy::mountImg()
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
QStringList AutoSystemRestoreProxy::getRsyncArgs(CustomizeSystemRestoreScene scene)
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
void AutoSystemRestoreProxy::restoreSystem()
{
    qDebug() << "AutoSystemRestoreProxy::restoreSystem invoke begin";

    qDebug() <<"当前还原的UUID："<<m_curUuid;
    //系统升级的备份点还原时，为保障断电后开机能继续还原，此处增加标志文件；还原成功后删除此文件；还原失败或断电还原未完成时，不删除此文件，等开启机器时init阶段根据是否存在标志文件继续进行系统还原。
    QString rollbackFile =  NEEDROOLBACKFILE;
    qDebug() <<"创建NEEDROOLBACKFILE："<< rollbackFile ;
    Utils::createRollBackFile(rollbackFile,m_curUuid);

    // 还原前准备
    doPrepare();

    //接入ostree
    m_pOstree = new OstreeInterface(this);
    QString repoPath = m_pOstree->getRepoPath("", OstreeInterface::BackupType::SYSTEM_UPDATE);

    //设置工作类型为还原
    m_pOstree->setTaskType(OstreeInterface::TaskType::RESTORE);

    //检查repo点是否存在
    if(!m_pOstree->checkOstreeRepo("",OstreeInterface::BackupType::SYSTEM_UPDATE))
    {
        //不存在检测repo失败
        emit checkResult(int(BackupResult::CHECK_REPO_FAIL));
    }

    // 绑定对应的信号与槽
    connect(m_pOstree, &OstreeInterface::progress, this, &AutoSystemRestoreProxy::progress);
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
            //this->rsyncFinished();

            // 系统升级的备份点还原时，为保障断电后开机能继续还原，此处增加标志文件；还原成功后删除此文件；还原失败或断电还原未完成时，不删除此文件，等开启机器时init阶段根据是否存在标志文件继续进行系统还原。
//            if (m_curUuid == AUTO_BACKUP_UUID)
//            {
//                QString rollbackFile = Utils::getBackupRootPath() + FLAG_FILE_SYSTEM_UPDATE_NEED_ROLLBACK;
//                rollbackFile.replace("//", "/");
//                QFile file(rollbackFile);
//                if (file.exists()) {
//                    file.remove();
//                    ::sync();
//                }
//            }

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

                    qDebug() << "系统哈希没有变化也强制执行一次set-default";
                    QStringList systemHashArgs = m_pOstree->getSystemRollBackArgs(m_hashNumber);
                    m_pOstree->setOperation(OstreeInterface::Operation::NOROLLBACK);
                    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::NOROLLBACK);
                    m_pOstree->start(systemHashArgs, true);
                }
            }
            else if(oper == OstreeInterface::Operation::ROLLBACK)
            {
                this->rsyncFinished();

                //目前/etc/ksaf/tmp/.status文件在还原之后，会使用ovl中数据，需要将对应ovl数据全部修改为8，从而触发打标
                QString msgOut;
                QString preCmd = "echo  8 > /ostree/pkgs/ovl-" ;
                QString cmdUpper =  preCmd + m_backupPoint.m_systemHash +"/etc-ovl/etc-upper/ksaf/tmp/.status";
                qDebug() << "cmdUpper： " << cmdUpper;
                Utils::executeCMD(cmdUpper.toStdString().c_str(), msgOut);
                qDebug() << msgOut;
                QString cmdTmpUpper =  preCmd + m_backupPoint.m_systemHash +"/etc-ovl/etc-tmpupper/ksaf/tmp/.status";
                qDebug() << "cmdTmpUpper " << cmdTmpUpper;
                Utils::executeCMD(cmdTmpUpper.toStdString().c_str(), msgOut);
                qDebug() << msgOut;

                //还原成功，清理 NEEDROOLBACKFILE 标志文件
                Utils::deleteFile(rollbackFile);

                //清理回滚的备份点
//                this->deleteBackupPointCommits();
                //清理回滚的备份点元数据
                this->deleteXmlBackupPoint();

                //再次同步数据
                ::sync();

                //补足进度信号，并等待3秒
                emit progress(100);
                Utils::wait(3);

                emit this->workResult(result);
                qDebug() << "系统版本有变化，完成ROLLBACK操作,准备重启！！！";
            }
            else if (oper == OstreeInterface::Operation::NOROLLBACK)
            {
                this->rsyncFinished();

                //目前/etc/ksaf/tmp/.status文件在还原之后，会使用ovl中数据，需要将对应ovl数据全部修改为8，从而触发打标
                QString msgOut;
                QString preCmd = "echo  8 > /ostree/pkgs/ovl-" ;
                QString cmdUpper =  preCmd + m_backupPoint.m_systemHash +"/etc-ovl/etc-upper/ksaf/tmp/.status";
                qDebug() << "cmdUpper： " << cmdUpper;
                Utils::executeCMD(cmdUpper.toStdString().c_str(), msgOut);
                qDebug() << msgOut;
                QString cmdTmpUpper =  preCmd + m_backupPoint.m_systemHash +"/etc-ovl/etc-tmpupper/ksaf/tmp/.status";
                qDebug() << "cmdTmpUpper " << cmdTmpUpper;
                Utils::executeCMD(cmdTmpUpper.toStdString().c_str(), msgOut);
                qDebug() << msgOut;


                //还原成功，清理 NEEDROOLBACKFILE 标志文件
                Utils::deleteFile(rollbackFile);

                //清理回滚的备份点
//                    this->deleteBackupPointCommits();
                //清理回滚的备份点元数据
                this->deleteXmlBackupPoint();

                //再次同步数据
                ::sync();

                //补足进度信号，并等待3秒
                emit progress(100);
                Utils::wait(3);

                emit this->workResult(result);

                qDebug() << "系统版本无变化，准备重启！！！";
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

    //先判断当前系统分支哈希是否有变化
    if(m_systemHash == m_backupPoint.m_systemHash)
    {
        m_bSameSystem = true;
        //调用不可变接口，获取要回滚的系统分支哈希number
        QStringList systemGetNumArgs= m_pOstree->getHashNumberArgs(m_systemHash);
        m_pOstree->setOperation(OstreeInterface::Operation::STATUS_TO_NUM);
        qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::STATUS_TO_NUM);
        m_pOstree->start(systemGetNumArgs, true);
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

        //执行rsync同步
        QString srcPath ;
        QString descPath ;
        if(it.key() == "home")
        {
            //先执行checkout导出
            QStringList checkoutArgs= m_pOstree->getCheckoutArgs(repoPath, it.key() ,it.value());
            m_pOstree->setOperation(OstreeInterface::Operation::CHECKOUT);
            qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::CHECKOUT);
            m_pOstree->setOperationPath(it.key());
            m_pOstree->start(checkoutArgs, true);

            //设定源目录和目标目录
            srcPath = TEMP_RESTORE_PATH + it.key();
            m_pOstree->setRsyncScense(OstreeInterface::RsyncScense::RESTORE_HOMEHIDDENDATA);
            QString xmlPath = Utils::getBackupRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
            xmlPath.replace("//", "/");
            ParseBackupList parse(xmlPath);
            m_backupPoint = parse.findBackupPointByUuid(m_backupWrapper.m_uuid);
            if (m_backupPoint.isNull())
            {
                qCritical("xml中还原点不存在");
                return;
            }
            descPath = "/data/home/" + m_backupPoint.m_user; // 还原到备份点里的用户主目录，而不是当前登录用户的用户主目录

            //执行rsync过程
            QStringList rsyncArgs= m_pOstree->getRsyncRepoArgs(srcPath, descPath);
            m_pOstree->setOperation(OstreeInterface::Operation::RSYNC);
            qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::RSYNC);
            m_pOstree->setOperationPath(it.key());
            m_pOstree->start(rsyncArgs, true);
        }
        else if (it.key() == "others")
        {
            //先执行checkout导出
            QStringList checkoutArgs= m_pOstree->getCheckoutArgs(repoPath, it.key() ,it.value());
            m_pOstree->setOperation(OstreeInterface::Operation::CHECKOUT);
            qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::CHECKOUT);
            m_pOstree->setOperationPath(it.key());
            m_pOstree->start(checkoutArgs, true);

            QString tmpModeFilePath = "";
            tmpModeFilePath = tmpModeFilePath + TEMP_OTHERS_PATH + "system-ovl.conf";

            QString destPath = "/ostree/pkgs/";
            bool bExist = Utils::isDirExists(destPath);
            if(!bExist)
            {
                if (!Utils::mkpath(destPath))
                {
                    qCritical() << QString("mkdir %1 failed !").arg(destPath) ;
                }
            }

            // 还原/ostree/pkgs/system-ovl.conf文件
            QStringList cpArgs= m_pOstree->getCpArgs(tmpModeFilePath, destPath);
            m_pOstree->setOperation(OstreeInterface::Operation::CP);
            qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::CP);
            m_pOstree->start(cpArgs, true);

            // 还原/ostree/pkgs/mm.lock文件
            QString tmpModeFlagFilePath = "";
            QString tmpModeFlagFile = "mm.lock";
            tmpModeFlagFilePath = tmpModeFlagFilePath + TEMP_OTHERS_PATH + tmpModeFlagFile;
            if (!Utils::fileExists(tmpModeFlagFilePath)) {
                qDebug() << tmpModeFlagFilePath << " not exists";
                if (!Utils::fileExists(destPath + tmpModeFlagFile)) {
                    qDebug() << destPath + tmpModeFlagFile << " not exists";
                } else {
                    QStringList args;
                    args << "-rf";
                    args << destPath + tmpModeFlagFile;
                    QProcess::execute("rm", args);
                }
            } else {
                QStringList cpArgs= m_pOstree->getCpArgs(tmpModeFlagFilePath, destPath);
                m_pOstree->setOperation(OstreeInterface::Operation::CP);
                qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::CP);
                m_pOstree->start(cpArgs, true);
            }
        }
        else
        {
            QString ovlPath = "";
            ovlPath = ovlPath + "/ostree/pkgs/" + "ovl-" + m_backupPoint.m_systemHash + "/";

            //先执行checkout导出
            QStringList checkoutArgs= m_pOstree->getCheckoutFullSyncArgs(repoPath, ovlPath ,it.value());
            m_pOstree->setOperation(OstreeInterface::Operation::CHECKOUT);
            qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::CHECKOUT);
            m_pOstree->setOperationPath(it.key());
            m_pOstree->start(checkoutArgs, true);

            //不相同的时候，ovl回滚路径不存在，需要创建路径
//            if(!m_bSameSystem)
//            {
//                if (!Utils::mkpath(descPath)) {
//                    qCritical() << QString("mkdir %1 failed !").arg(descPath) ;
//                    return ;
//                }
//            }
            QStringList rsyncArgs= m_pOstree->getRsyncRepoArgs(srcPath, descPath);
            m_pOstree->setOperation(OstreeInterface::Operation::RSYNC);
            qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::RSYNC);
            m_pOstree->setOperationPath(it.key());
            m_pOstree->start(rsyncArgs, true);
        }
    }

    qDebug() << "针对系统可变目录的还原操作:结束";

    //执行rm 清理
    QStringList rmArgs= m_pOstree->getRmArgs(TEMP_RESTORE_PATH);
    m_pOstree->setOperation(OstreeInterface::Operation::RM);
    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::RM);
    m_pOstree->start(rmArgs, true);

    qDebug() << "针对kylin-system-updater.db文件的还原操作:开始";

    QString backupPath = "/var/cache/kylin-system-updater/kylin-system-updater.db.bak";
    QString targetPath = "/var/cache/kylin-system-updater/kylin-system-updater.db";
    if (restoreFile(backupPath, targetPath)) {
        qDebug() << "备份恢复操作完成";
    } else {
        qDebug() << "备份恢复操作失败";
    }

    qDebug() << "针对kylin-system-updater.db文件的还原操作:结束";

//    // 系统升级的备份点还原时，为保障断电后开机能继续还原，此处增加标志文件；还原成功后删除此文件；还原失败或断电还原未完成时，不删除此文件，等开启机器时init阶段根据是否存在标志文件继续进行系统还原。
//    if (m_curUuid == AUTO_BACKUP_UUID) {
//        QString rollbackFile = Utils::getBackupRootPath() + FLAG_FILE_SYSTEM_UPDATE_NEED_ROLLBACK;
//        rollbackFile.replace("//", "/");
//        Utils::syncWriteFile(rollbackFile, QString("UUID=") + AUTO_BACKUP_UUID);
//    }

    m_pOstree->setTaskType(OstreeInterface::TaskType::IDLE);

    qDebug() << "AutoSystemRestoreProxy::restoreSystem invoke end";
}

/**
 * @brief 还原额外数据
 */
void AutoSystemRestoreProxy::restoreExtraData()
{
    qDebug() << "AutoSystemRestoreProxy::restoreExtraData invoke begin";

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

    QStringList args = getRsyncArgs(AutoSystemRestoreProxy::RESTORE_EXTRA_DATA);

    m_srcPath = m_backupPath;
    QString destPath = Utils::getBackupRootPath() + "/";
    destPath.replace("//", "/");
    args << m_extraDir + "/";
    args << destPath;

    m_extraProcess = new RsyncPathToDirProcess(this);
    m_extraProcess->start(args, false);

    qDebug() << "AutoSystemRestoreProxy::restoreExtraData invoke end";
}

bool AutoSystemRestoreProxy::restoreFile(const QString &backupPath, const QString &targetPath)
{
    // 检查备份文件是否存在
    if (!QFileInfo::exists(backupPath)) {
        qWarning() << "备份文件不存在:" << backupPath;
        return false;
    }

    // 如果目标文件已存在，先删除
    if (QFileInfo::exists(targetPath)) {
        if (!QFile::remove(targetPath)) {
            qWarning() << "无法删除已存在的目标文件:" << targetPath;
            return false;
        }
    }

    // 复制备份文件到目标路径
    if (!QFile::copy(backupPath, targetPath)) {
        qWarning() << "从备份恢复失败:" << backupPath << "->" << targetPath;
        return false;
    }

    qDebug() << "恢复成功:" << backupPath << "->" << targetPath;
    return true;
}

/**
 * @brief 清理回滚的备份点元数据
 */
bool AutoSystemRestoreProxy::deleteXmlBackupPoint()
{
    qDebug() << "AutoSystemRestoreProxy::deleteXmlBackupPoint invoke begin";

    QString xmlPath = Utils::getBackupRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList xmlparse(xmlPath);
    // 查询节点
    m_backupPoint = xmlparse.findBackupPointByUuid(m_backupWrapper.m_uuid);
    if (xmlparse.deleteItem(m_backupWrapper.m_uuid) != ParseBackupList::SUCCESS) {
        qCritical() << QString("failed to delete %1 from backuplist.xml or sys_update_backuplist.xml!").arg(m_backupWrapper.m_uuid);
        return false;
    } else {
        qDebug() << "deleteXmlBackupPoint succeeded! ";
        return true;
    }

    qDebug() << "AutoSystemRestoreProxy::deleteXmlBackupPoint invoke end";
}

/**
 * @brief 清理回滚的备份点
 */
bool AutoSystemRestoreProxy::deleteBackupPointCommits()
{
    qDebug() << "AutoSystemRestoreProxy::deleteBackupPointCommits invoke begin";

    m_pOstree = new OstreeInterface(this);

    //设置工作类型为备份点管理
    m_pOstree->setTaskType(OstreeInterface::TaskType::BACKUPPOINT_MANAGEMENT);
    qDebug() << "设置工作类型为:备份点管理" ;

    //检查repo点是否存在
    if(!m_pOstree->checkOstreeRepo("",OstreeInterface::BackupType::SYSTEM_UPDATE))
    {
        qDebug() << "备份点ostree仓库目录不存在";
        m_pOstree->setTaskType(OstreeInterface::TaskType::IDLE);
        return false;
    }

    //获取ostree仓库目录
    QString repoPath = m_pOstree->getRepoPath("", OstreeInterface::BackupType::SYSTEM_UPDATE);

    QStringList pathNameList = {"usr","etc","var","home"};

    // 第一步：删除分支，解除引用
    QStringList refsArgs = m_pOstree->getRefsArgs(repoPath, m_backupPoint.m_branchName);
    m_pOstree->setOperation(OstreeInterface::Operation::REFS);
    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::REFS);
    m_pOstree->start(refsArgs, true);
    // 第二步：删除4个commit对应的目录
    for (auto dirPath : pathNameList) {
        QString commitID = m_backupPoint.m_systemCommitHashMap.value(dirPath);
        QStringList pruneArgs = m_pOstree->getPruneArgs(repoPath, commitID);
        m_pOstree->setOperation(OstreeInterface::Operation::PRUNE);
        qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::PRUNE);
        m_pOstree->start(pruneArgs, true);
    }

    qDebug() << "AutoSystemRestoreProxy::deleteBackupPointCommits invoke end";
}
