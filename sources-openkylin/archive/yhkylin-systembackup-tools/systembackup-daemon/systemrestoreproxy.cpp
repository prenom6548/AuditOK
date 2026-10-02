/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "systemrestoreproxy.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <unistd.h>
#include <sys/reboot.h>
#include "../common/utils.h"
#include "mymountproxy.h"
#include "translations.h"



IMPLEMENT_DYNCREATE(SystemRestoreProxy)

/**
 * @brief 构造函数
 */
SystemRestoreProxy::SystemRestoreProxy()
{
    m_bSuccess = false;
    m_isOem = false;
    m_extraProcess = nullptr;
}

/**
 * @brief 析构函数
 */
SystemRestoreProxy::~SystemRestoreProxy()
{
}

/**
 * @brief 环境检测
 * @return false,检测失败;true,检测成功
 */
bool SystemRestoreProxy::checkEnvEx()
{
    qDebug() << "SystemRestoreProxy::checkEnvEx invoke begin";


    if("00000000-0000-0000-0000-000000000000" == m_backupWrapper.m_uuid)
    {
        qDebug() << "还原类型是出厂还原:UUID="  << FACTORY_BACKUP_UUID;
        m_bFactoryRestore =  true;
    }

    qDebug() << "备份点存储类型"  << m_backupWrapper.m_iPosition;

    //根据是否本地或者外部存储类型，确认仓库位置以及标志
    if (m_backupWrapper.m_iPosition == BackupPosition::LOCAL)
    {
        qDebug() << "备份点路径前缀"  << m_backupWrapper.m_prefixDestPath;
        m_backupDir = m_backupWrapper.m_prefixDestPath ;
        m_isLocal = true;
    }
    else if (m_backupWrapper.m_iPosition == BackupPosition::UDISK)
    {
        m_backupDir = m_backupWrapper.m_prefixDestPath + "/" ;
        m_isLocal = false;
    }

    qDebug() << "备份点路径:m_backupDir="  << m_backupDir;


    // 1、检测xml中的还原点是否还存在
    QString xmlPath ;

    if(m_bFactoryRestore)
    {
        xmlPath = m_backupDir + FACTORY_BACKUP_XML_PATH;
    }
    else
    {
        xmlPath = m_backupDir + SYS_BACKUP_XML_PATH;
    }

    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    qDebug()<<__LINE__<<__func__<<"还原UUID："<<m_backupWrapper.m_uuid;
    m_backupPoint = parse.findBackupPointByUuid(m_backupWrapper.m_uuid);
    if (m_backupPoint.isNull())
    {
        qCritical("xml中还原点不存在");
        emit checkResult(int(BackupResult::INC_NOT_FOUND_DIR));
        return false;
    }

    //2.查询对应的目录commit哈希
    parse.getXmlCommitHashMap(m_backupPoint.m_systemHash,m_systemCommitHashMap,m_backupWrapper.m_uuid);
    if (m_systemCommitHashMap.size() == 0)
    {
        qCritical("存储点中没有对应哈希");
        emit checkResult(int(BackupResult::CHECK_COMMITHASH_FAIL));
        return false;
    }


    // 3、检测ostree仓库是否存在
    m_pOstree = new OstreeInterface(this);
    m_pOstree->setOperation(OstreeInterface::Operation::INIT);
    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::INIT);

    if(m_isLocal)
    {
        if(!m_pOstree->checkOstreeRepo("",m_bFactoryRestore ? OstreeInterface::BackupType::FACTORY : OstreeInterface::BackupType::SYSTEM))
        {
            qCritical("repo仓库不存在");
            emit checkResult(int(BackupResult::CHECK_REPO_FAIL));
            return false;
        }
    }
    else
    {
        if(!m_pOstree->checkOstreeRepo(m_backupDir + ".backup/",m_bFactoryRestore ? OstreeInterface::BackupType::FACTORY : OstreeInterface::BackupType::SYSTEM))
        {
            qCritical("repo仓库不存在");
            emit checkResult(int(BackupResult::CHECK_REPO_FAIL));
            return false;
        }
    }

    m_curUuid = m_backupWrapper.m_uuid;

    emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));

    qDebug() << "SystemRestoreProxy::checkEnvEx invoke end";
    return true;
}

/**
 * @brief 执行还原逻辑
 */
void SystemRestoreProxy::doWorkEx()
{
    qDebug() << "SystemRestoreProxy::doWorkEx invoke begin";

    // 1、校验
    if (!checkEnvEx())
        return ;

    // 2、还原系统
    restoreSystem();

    qDebug() << "SystemRestoreProxy::doWorkEx invoke end";
}

/**
 * @brief 备份准备
 * @return true,准备成功；false，准备失败
 */
bool SystemRestoreProxy::doPrepare()
{
    qDebug() << "SystemRestoreProxy::doPrepare invoke begin";


    // 2、停止安全初始化服务，以防过高的CPU占有率，因为还原时安全初始化服务会逐个文件打标记，造成cpu占有率超高系统卡顿

    // 停止网络服务，以防网络更新
    //Utils::stopNetwork();

    qDebug() << "SystemRestoreProxy::doPrepare invoke end";
    return true;
}

/**
 * @brief 将dst.img文件挂载到/backup/imgbackup目录
 * @return
 */
bool SystemRestoreProxy::mountImg()
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
QStringList SystemRestoreProxy::getRsyncArgs(CustomizeSystemRestoreScene scene)
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
void SystemRestoreProxy::restoreSystem()
{
    qDebug() << "SystemRestoreProxy::restoreSystem invoke begin";

    qDebug() <<"当前还原的UUID："<<m_curUuid;
    //系统升级的备份点还原时，为保障断电后开机能继续还原，此处增加标志文件；还原成功后删除此文件；还原失败或断电还原未完成时，不删除此文件，等开启机器时init阶段根据是否存在标志文件继续进行系统还原。
    QString rollbackFile =  NEEDROOLBACKFILE;
    qDebug() <<"创建NEEDROOLBACKFILE："<< rollbackFile ;
    Utils::createRollBackFile(rollbackFile,m_curUuid);

    // 还原前准备
    doPrepare();

    //接入ostree
    m_pOstree = new OstreeInterface(this);
    //设置工作类型为还原
    m_pOstree->setTaskType(OstreeInterface::TaskType::RESTORE);
    qDebug() << "设置工作类型为:还原" ;
    QString repoDir = m_backupDir + "/.backup/";
    QString repoPath = m_pOstree->getRepoPath(repoDir, m_bFactoryRestore ? OstreeInterface::BackupType::FACTORY : OstreeInterface::BackupType::SYSTEM);


    // 绑定对应的信号与槽

    //1.进度信号
    connect(m_pOstree, &OstreeInterface::progress, this, &SystemRestoreProxy::progress);
    //2.系统哈希
    connect(m_pOstree, &OstreeInterface::systemHash, this, [&](QString systemHash){
        m_systemHash = systemHash;
    });
    //3.哈希对应序列号
    connect(m_pOstree, &OstreeInterface::hashNumber, this, [&](QString numberStr){
        //数据获取从1开始，但是部署点排序从0开始，所以序列号减1
        int number = numberStr.toInt() - 1;
        m_hashNumber = QString::number(number);
        qDebug() << "查询部署点对应的序列号为：" << m_hashNumber;
    });
    //4.获取所有ostree哈希列表
    connect(m_pOstree, &OstreeInterface::ostreeHashs, this, [&](QStringList systemHashs){
        m_allSystemHash = systemHashs;
    });
    //5.获取新部署的哈希
    connect(m_pOstree, &OstreeInterface::pendingHash, this, [&](QString hash){
        m_factoryHash = hash;
    });




    //先判断是否属于出厂还原
    if(m_bFactoryRestore)   //执行出厂还原流程
    {
        //出厂还原需要清空所有部署点（A.0以及当前部署点除外）），然后通过A.0部署新的A.1出来，之后将所有的数据还原到A.1上，如果A.1存在，则不再新部署
        //出厂还原需要保留系统激活状态

        //出厂还原结束处理逻辑
        connect(m_pOstree, &OstreeInterface::finished, this,  [&](bool result) {
            if (result)
            {
                OstreeInterface::Operation oper =  m_pOstree->getOperation();
                if(oper == OstreeInterface::Operation::RM)
                {
                    this->rsyncFinished();

                    //还原成功，清理 NEEDROOLBACKFILE 标志文件
                    Utils::deleteFile(rollbackFile);

                    //出厂还原清理除出厂备份点之外的所有备份点元数据
                    //1、清理所有系统升级备份点及元数据
                    QString sysUpdateRepoPath = m_pOstree->getRepoPath("", OstreeInterface::BackupType::SYSTEM_UPDATE);
                    QString sysUpdateXmlPath = Utils::getBackupRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
                    sysUpdateXmlPath.replace("//", "/");
                    if (!Utils::deleteDirectory(sysUpdateRepoPath) || !Utils::deleteSingleFile(sysUpdateXmlPath)) {
                        qDebug() << "清理系统升级备份点或元数据失败";
                    }
                    //2、清理所有系统备份点及元数据
                    QString sysRepoPath = m_pOstree->getRepoPath("", OstreeInterface::BackupType::SYSTEM);
                    QString sysXmlPath = Utils::getBackupRootPath() + SYS_BACKUP_XML_PATH;
                    sysXmlPath.replace("//", "/");
                    if (!Utils::deleteDirectory(sysRepoPath) || !Utils::deleteSingleFile(sysXmlPath)) {
                        qDebug() << "清理系统备份点或元数据失败";
                    }

                    //再次同步数据
                    ::sync();

                    //补足进度信号，并等待3秒
                    emit progress(100);
                    Utils::wait(3);

                    emit this->workResult(result);
                    qDebug() << "系统版本无变化，准备重启！！！";
                }
            }
        });

        qDebug() << "出厂还原:开始";

        //1.根据哈希判断是否需要undeploy部署点

        //获取系统所有部署点哈希列表，赋值m_allSystemHash
        QStringList needUndeployHashArgs= m_pOstree->getNeedUndeployHashArgs();
        m_pOstree->setOperation(OstreeInterface::Operation::STATUS_TO_UNDEPLOY);
        qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::STATUS_TO_UNDEPLOY);
        m_pOstree->start(needUndeployHashArgs, true);

        //获取正在使用部署点哈希，赋值m_systemHash
        QStringList systemHashArgs= m_pOstree->getSystemHashArgs();
        m_pOstree->setOperation(OstreeInterface::Operation::STATUS);
        qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::STATUS);
        m_pOstree->start(systemHashArgs, true);

        qDebug() << "哈希m_backupPoint.m_systemHash：" << m_backupPoint.m_systemHash;
        qDebug() << "哈希m_systemHash：" << m_systemHash;


        for(int i =0 ; i < m_allSystemHash.size();i++)
        {
            qDebug() << "哈希：" << m_allSystemHash[i];
            //排除当前使用部署点以及出厂部署点
            if((m_backupPoint.m_systemHash == m_allSystemHash[i])  || (m_systemHash == m_allSystemHash[i]) || m_allSystemHash[i].isEmpty() )
            {
                continue;  
            }

            qDebug() << "要删除的哈希：" << m_allSystemHash[i];

            //获取要清空的部署点number
            QStringList systemGetNumArgs= m_pOstree->getHashNumberArgs(m_allSystemHash[i]);
            m_pOstree->setOperation(OstreeInterface::Operation::STATUS_TO_NUM);
            qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::STATUS_TO_NUM);
            m_pOstree->start(systemGetNumArgs, true);

            //其他部署点做清空
            QStringList undeployArgs= m_pOstree->getUndeployArgs(m_hashNumber);
            m_pOstree->setOperation(OstreeInterface::Operation::UNDEPLOY);
            qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::UNDEPLOY);
            m_pOstree->start(undeployArgs, true);

        }

        //2.需要通过deploy生成目标部署点

        //先执行remount操作
        QString preCmd = "/usr/lib/ostree/ostree-remount";
        QString msgOut;
        Utils::executeCMD(preCmd.toStdString().c_str(), msgOut);
        qDebug() << msgOut;

        //查询原始部署点的分支名，不再使用哈希
        QString getRefs = "ostree admin status|grep refspec | tail -1 |awk -F 'refspec:' '{print $2}'";
        QString msgOut1;
        Utils::executeCMD(getRefs.toStdString().c_str(), msgOut1);
        QString refsName = msgOut1.trimmed();
        qDebug() << "原始部署点的分支名:" << refsName;

        QStringList deployArgs= m_pOstree->getDeployArgs(refsName);
        m_pOstree->setOperation(OstreeInterface::Operation::DEPLOY);
        qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::DEPLOY);
        m_pOstree->start(deployArgs, true);

        //3.重新获取新部署的部署点哈希，作为目标部署点
        QStringList pendingHashArgs= m_pOstree->getPendingHashArgs();
        m_pOstree->setOperation(OstreeInterface::Operation::STATUS_TO_PENDING);
        qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::STATUS_TO_PENDING);
        m_pOstree->start(pendingHashArgs, true);

        //4.需要设立标志位，当系统再次重启之后，在Grub阶段，删除当前运行旧部署点

        //出厂还原过程中，当前部署点无法删除，当重启之后在Grub阶段
        QString undeployFile =  NEEDUNDEPLOYFILE;
        qDebug() <<"创建NEEDUNDEPLOYFILE："<< undeployFile ;
        Utils::createRollBackFile(undeployFile,m_systemHash);

        //5.还原对应ovl数据

        //6.还原home数据

        for (auto it = m_systemCommitHashMap.constBegin(); it != m_systemCommitHashMap.constEnd(); ++it)
        {
            qDebug() << it.key() << ":" << it.value();

            //如果存在空，跳过
            if(it.value().isEmpty())
            {
                continue;
            }

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
                //文件保护箱需要权限才能删除数据，或者使用dbus接口
                qDebug() << "清理保护箱数据" ;
                Utils::clearBox();
                //出厂还原过程，直接覆盖用户家目录下数据，所有用户清空
                m_pOstree->setRsyncScense(OstreeInterface::RsyncScense::RESTORE_SYSTEM);
                descPath = "/data/home/";

                QStringList rsyncArgs= m_pOstree->getRsyncRepoArgs(srcPath, descPath);
                m_pOstree->setOperation(OstreeInterface::Operation::RSYNC);
                qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::RSYNC);
                m_pOstree->setOperationPath(it.key());
                m_pOstree->start(rsyncArgs, true);
            }
            else if(it.key() == "others")
            {
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
                if (!Utils::fileExists(destPath + tmpModeFlagFile)) {
                    qDebug() << destPath + tmpModeFlagFile << " not exists";
                } else {
                    QStringList args;
                    args << "-rf";
                    args << destPath + tmpModeFlagFile;
                    QProcess::execute("rm", args);
                }
            }
            else
            {
                //出厂备份的存储路径多一层，和普通还原路径不同
                srcPath += "/" + it.key() + "-ovl";
                //ovl路径前置
                QString prePath ;
                prePath = "/ostree/pkgs/ovl-";
                m_pOstree->setRsyncScense(OstreeInterface::RsyncScense::RESTORE_SYSTEM);
                descPath = prePath + m_factoryHash + "/" + it.key() + "-ovl";
                qDebug() << "descPath" << descPath;

                QStringList rsyncArgs= m_pOstree->getRsyncRepoArgs(srcPath, descPath);
                m_pOstree->setOperation(OstreeInterface::Operation::RSYNC);
                qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::RSYNC);
                m_pOstree->setOperationPath(it.key());
                m_pOstree->start(rsyncArgs, true);
            }
        }

        //7.保留激活状态
        QStringList LicenseFiles;
        LicenseFiles << "/etc/LICENSE";
        LicenseFiles << "/etc/.kyinfo";
        LicenseFiles << "/etc/.kyactivation";
        LicenseFiles << "/etc/.kyhwid";
        LicenseFiles << "/etc/.product";
        LicenseFiles << "/usr/share/plymouth";

        for(const auto &dirPath : m_pathNameList)
        {
            for(const auto &ovlPath : m_ovlPathList)
            {
                for (const auto &whitelist : LicenseFiles) {
                    QString firstDir = whitelist.section('/', 1, 1);
                    if (firstDir == dirPath) {
                        if (whitelist.contains(firstDir)) {
                            QString remainingPath = whitelist.section('/', 2);
                            QString srcDirPath = "";
                            srcDirPath = srcDirPath + "/ostree/pkgs/" + "ovl-" + m_systemHash + "/" + dirPath + "-ovl/" + dirPath + "-" + ovlPath + "/" + remainingPath;
                            qDebug() << "srcDirPath: " << srcDirPath;
                            QString destDirPath = "";
                            destDirPath = destDirPath +  + "/ostree/pkgs/" + "ovl-" + m_factoryHash + "/" + dirPath + "-ovl/" + dirPath + "-" + ovlPath + "/" + remainingPath;
                            qDebug() << "destDirPath: " << destDirPath;
                            // 创建目标目录父目录
                            QString parentDestDirPath = "";
                            if (remainingPath.contains('/')) {
                                int lastSlash = remainingPath.lastIndexOf('/');
                                if (lastSlash > 0) {
                                    QString parentPath = remainingPath.left(lastSlash);
                                    QString tmpBaseDestDirPath = "";
                                    parentDestDirPath = tmpBaseDestDirPath + "/ostree/pkgs/" + "ovl-" + m_factoryHash + "/" + dirPath + "-ovl/" + dirPath + "-" + ovlPath + "/" + parentPath;
                                    bool bExist = Utils::isDirExists(parentDestDirPath);
                                    if(!bExist)
                                    {
                                        if (!Utils::mkpath(parentDestDirPath))
                                        {
                                            qCritical() << QString("mkdir %1 failed !").arg(parentDestDirPath);
                                            return;
                                        }
                                    }
                                }
                            } else {
                                parentDestDirPath = parentDestDirPath + "/ostree/pkgs/" + "ovl-" + m_factoryHash + "/" + dirPath + "-ovl/" + dirPath + "-" + ovlPath;
                            }
                            qDebug() << "parentDestDirPath: " << parentDestDirPath;
                            // 先清理目标目录或文件，再执行拷贝，防止多级目录还原不完全
                            if(!Utils::deletePath(destDirPath)) {
                                qDebug() << "deleting destDirPath failed!" << destDirPath;
                            }
                            if (QFileInfo::exists(srcDirPath))
                            {
                                QStringList cpArgs= m_pOstree->getCpArgs(srcDirPath, parentDestDirPath);
                                m_pOstree->setOperation(OstreeInterface::Operation::CP);
                                qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::CP);
                                m_pOstree->start(cpArgs, true);
                            }
                        }
                    }
                }
            }
        }

        //8.清理并触发重启
        //执行rm 清理
        QStringList rmArgs= m_pOstree->getRmArgs(TEMP_RESTORE_PATH);
        m_pOstree->setOperation(OstreeInterface::Operation::RM);
        qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::RM);
        m_pOstree->start(rmArgs, true);

        qDebug() << "出厂还原:结束";

    }
    else    //走普通系统还原的过程
    {
        //和出厂还原走不同的结束处理逻辑
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

                        //再次同步数据
                        ::sync();

                        //补足进度信号，并等待3秒
                        emit progress(100);
                        Utils::wait(1);

                        emit this->workResult(result);
                        qDebug() << "系统版本无变化，准备重启！！！";
                        //reboot(RB_AUTOBOOT);
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

                    //再次同步数据
                    ::sync();

                    //补足进度信号，并等待3秒
                    emit progress(100);
                    Utils::wait(1);

                    emit this->workResult(result);
                    qDebug() << "系统版本有变化，完成ROLLBACK操作,准备重启！！！";
                    //reboot(RB_AUTOBOOT);
                }


            }
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

        // 添加用户数据白名单，还原时保留用户数据
        QStringList whiteList;
        // (1)排除指纹数据，用户密码、角色、权限、生物识别等信息
        whiteList << "/var/lib/biometric-auth";
        whiteList << "/etc/passwd";
        whiteList << "/etc/shadow";
        whiteList << "/etc/group";
        whiteList << "/etc/gshadow";
        whiteList << "/etc/sudoers";
        // (2)云图片作为桌面背景的路径属于用户数据
        whiteList << "/var/lib/AccountsService";
        // (3)保留域用户数据
        whiteList << "/etc/sssd";
        whiteList << "/var/lib/sss";
        whiteList << "/usr/share/sssd";
        whiteList << "/etc/ipa";
        whiteList << "/etc/krb5.keytab";
        whiteList << "/etc/krb5.conf";
        whiteList << "/var/lib/ipa-client";
        whiteList << "/etc/nsswitch.conf";
        whiteList << "/etc/pam.d";
        whiteList << "/etc/hosts";
        whiteList << "/etc/hostname";
        whiteList << "/etc/hedron";
        whiteList << "/etc/kcm";
        whiteList << "/usr/hedron/hedronagent";
        whiteList << "/etc/ssl/certs";
        whiteList << "/usr/share/ca-certificates";
        whiteList << "/etc/NetworkManager";
        whiteList << "/var/lib/pam";
        whiteList << "/etc/kim";
        whiteList << "/var/lib/kim";
        whiteList << "/etc/systemd/system/multi-user.target.wants/kimbackend.service";
        whiteList << "/var/lib/polkit-1/localauthority/40-domain.d";
        whiteList << "/usr/lib/libsudo-kim.so";
        // (4)保留激活状态
        whiteList << "/etc/LICENSE";
        whiteList << "/etc/.kyinfo";
        whiteList << "/etc/.kyactivation";
        whiteList << "/etc/.kyhwid";
        whiteList << "/etc/.product";
        whiteList << "/usr/share/plymouth";

        //先判断当前系统分支哈希是否有变化
        if(m_systemHash == m_backupPoint.m_systemHash)
        {
            m_bSameSystem = true;

            // 创建临时目录/data/.backup/temp/xxx-ovl/xxx-xxxx
            if (createOverlayDirectories()) {
                qDebug() << "Directory creation completed successfully!";
            } else {
                qDebug() << "Directory creation failed!";
                return;
            }
            // 从当前部署点下拷贝最新的白名单数据，暂存到临时目录
            for(const auto &dirPath : m_pathNameList)
            {
                for(const auto &ovlPath : m_ovlPathList)
                {
                    for (const auto &whitelist : whiteList) {
                        QString firstDir = whitelist.section('/', 1, 1);
                        if (firstDir == dirPath) {
                            if (whitelist.contains(firstDir)) {
                                QString remainingPath = whitelist.section('/', 2);
                                QString srcDirPath = "";
                                srcDirPath = srcDirPath + "/ostree/pkgs/" + "ovl-" + m_systemHash + "/" + dirPath + "-ovl/" + dirPath + "-" + ovlPath + "/" + remainingPath;
                                qDebug() << "srcDirPath: " << srcDirPath;
                                // 创建目标目录父目录
                                QString parentDestDirPath = "";
                                if (remainingPath.contains('/')) {
                                    int lastSlash = remainingPath.lastIndexOf('/');
                                    if (lastSlash > 0) {
                                        QString parentPath = remainingPath.left(lastSlash);
                                        parentDestDirPath = TEMP_RESTORE_PATH + dirPath + "-ovl/" + dirPath + "-" + ovlPath + "/" + parentPath;
                                        bool bExist = Utils::isDirExists(parentDestDirPath);
                                        if(!bExist)
                                        {
                                            if (!Utils::mkpath(parentDestDirPath))
                                            {
                                                qCritical() << QString("mkdir %1 failed !").arg(parentDestDirPath);
                                                return;
                                            }
                                        }
                                    }
                                } else {
                                    parentDestDirPath = TEMP_RESTORE_PATH + dirPath + "-ovl/" + dirPath + "-" + ovlPath;
                                }
                                qDebug() << "parentDestDirPath: " << parentDestDirPath;
                                if (QFileInfo::exists(srcDirPath))
                                {
                                    QStringList cpArgs= m_pOstree->getCpArgs(srcDirPath, parentDestDirPath);
                                    m_pOstree->setOperation(OstreeInterface::Operation::CP);
                                    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::CP);
                                    m_pOstree->start(cpArgs, true);
                                }
                            }
                        }
                    }
                }
            }
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

            //如果存在空，跳过
            if(it.value().isEmpty())
            {
                continue;
            }

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
                QString xmlPath = m_backupDir + SYS_BACKUP_XML_PATH;
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
                //拼接需要执行备份的ovl目录
                QString ovlPath = "";
                ovlPath = ovlPath + "/ostree/pkgs/" + "ovl-" + m_backupPoint.m_systemHash + "/";

                //先执行checkout导出
                QStringList checkoutArgs= m_pOstree->getCheckoutFullSyncArgs(repoPath, ovlPath ,it.value());
                m_pOstree->setOperation(OstreeInterface::Operation::CHECKOUT);
                qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::CHECKOUT);
                m_pOstree->setOperationPath(it.key());
                m_pOstree->start(checkoutArgs, true);

                //TODO：后期用不可变接口替换白名单，或者采用参数来实现白名单不还原
//                if (!m_bSameSystem)
//                {
//                    m_pOstree->setRsyncScense(OstreeInterface::RsyncScense::RESTORE_SYSTEM);
//                    descPath = prePath + m_backupPoint.m_systemHash + "/" ;
//                    qDebug() << "descPath" << descPath;
//                }
//                else
//                {
//                    m_pOstree->setRsyncScense(OstreeInterface::RsyncScense::RESTORE_SYSTEM_SAMESYSTEM);
//                    descPath = prePath + m_backupPoint.m_systemHash + "/" ;
//                    qDebug() << "descPath" << descPath;
//                }
            }
        }

        qDebug() << "针对系统可变目录的还原操作:结束";

        qDebug() << "针对用户数据白名单的还原操作:开始";

        if (!m_bSameSystem)
        {
            // 从当前的部署点拷贝最新的白名单数据，替换到要切换的部署点中
            for(const auto &dirPath : m_pathNameList)
            {
                for(const auto &ovlPath : m_ovlPathList)
                {
                    for (const auto &whitelist : whiteList) {
                        QString firstDir = whitelist.section('/', 1, 1);
                        if (firstDir == dirPath) {
                            if (whitelist.contains(firstDir)) {
                                QString remainingPath = whitelist.section('/', 2);
                                QString srcDirPath = "";
                                srcDirPath = srcDirPath + "/ostree/pkgs/" + "ovl-" + m_systemHash + "/" + dirPath + "-ovl/" + dirPath + "-" + ovlPath + "/" + remainingPath;
                                qDebug() << "srcDirPath: " << srcDirPath;
                                QString destDirPath = "";
                                destDirPath = destDirPath + "/ostree/pkgs/" + "ovl-" + m_backupPoint.m_systemHash + "/" + dirPath + "-ovl/" + dirPath + "-" + ovlPath + "/" + remainingPath;
                                qDebug() << "destDirPath: " << destDirPath;
                                // 创建目标目录父目录
                                QString parentDestDirPath = "";
                                if (remainingPath.contains('/')) {
                                    int lastSlash = remainingPath.lastIndexOf('/');
                                    if (lastSlash > 0) {
                                        QString parentPath = remainingPath.left(lastSlash);
                                        QString tmpBaseDestDirPath = "";
                                        parentDestDirPath = tmpBaseDestDirPath + "/ostree/pkgs/" + "ovl-" + m_backupPoint.m_systemHash + "/" + dirPath + "-ovl/" + dirPath + "-" + ovlPath + "/" + parentPath;
                                        bool bExist = Utils::isDirExists(parentDestDirPath);
                                        if(!bExist)
                                        {
                                            if (!Utils::mkpath(parentDestDirPath))
                                            {
                                                qCritical() << QString("mkdir %1 failed !").arg(parentDestDirPath);
                                                return;
                                            }
                                        }
                                    }
                                } else {
                                    parentDestDirPath = parentDestDirPath + "/ostree/pkgs/" + "ovl-" + m_backupPoint.m_systemHash + "/" + dirPath + "-ovl/" + dirPath + "-" + ovlPath;
                                }
                                qDebug() << "parentDestDirPath: " << parentDestDirPath;
                                // 先清理目标目录或文件，再执行拷贝，防止多级目录还原不完全
                                if(!Utils::deletePath(destDirPath)) {
                                    qDebug() << "deleting destDirPath failed!" << destDirPath;
                                }
                                if (QFileInfo::exists(srcDirPath))
                                {
                                    QStringList cpArgs= m_pOstree->getCpArgs(srcDirPath, parentDestDirPath);
                                    m_pOstree->setOperation(OstreeInterface::Operation::CP);
                                    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::CP);
                                    m_pOstree->start(cpArgs, true);
                                }
                            }
                        }
                    }
                }
            }
        } else {
            // 从临时目录拷贝暂存的白名单数据，替换到当前部署点
            for(const auto &dirPath : m_pathNameList)
            {
                for(const auto &ovlPath : m_ovlPathList)
                {
                    for (const auto &whitelist : whiteList) {
                        QString firstDir = whitelist.section('/', 1, 1);
                        if (firstDir == dirPath) {
                            if (whitelist.contains(firstDir)) {
                                QString remainingPath = whitelist.section('/', 2);
                                QString srcDirPath = TEMP_RESTORE_PATH;
                                srcDirPath = srcDirPath + dirPath + "-ovl/" + dirPath + "-" + ovlPath + "/" + remainingPath;
                                qDebug() << "srcDirPath: " << srcDirPath;
                                QString destDirPath = "";
                                destDirPath = destDirPath + "/ostree/pkgs/" + "ovl-" + m_systemHash + "/" + dirPath + "-ovl/" + dirPath + "-" + ovlPath + "/" + remainingPath;
                                qDebug() << "destDirPath: " << destDirPath;
                                // 创建目标目录父目录
                                QString parentDestDirPath = "";
                                if (remainingPath.contains('/')) {
                                    int lastSlash = remainingPath.lastIndexOf('/');
                                    if (lastSlash > 0) {
                                        QString parentPath = remainingPath.left(lastSlash);
                                        QString tmpBaseDestDirPath = "";
                                        parentDestDirPath = tmpBaseDestDirPath + "/ostree/pkgs/" + "ovl-" + m_systemHash + "/" + dirPath + "-ovl/" + dirPath + "-" + ovlPath + "/" + parentPath;
                                        bool bExist = Utils::isDirExists(parentDestDirPath);
                                        if(!bExist)
                                        {
                                            if (!Utils::mkpath(parentDestDirPath))
                                            {
                                                qCritical() << QString("mkdir %1 failed !").arg(parentDestDirPath);
                                                return;
                                            }
                                        }
                                    }
                                } else {
                                    parentDestDirPath = parentDestDirPath + "/ostree/pkgs/" + "ovl-" + m_systemHash + "/" + dirPath + "-ovl/" + dirPath + "-" + ovlPath;
                                }
                                qDebug() << "parentDestDirPath: " << parentDestDirPath;
                                // 先清理目标目录或文件，再执行拷贝，防止多级目录还原不完全
                                if(!Utils::deletePath(destDirPath)) {
                                    qDebug() << "deleting destDirPath failed!" << destDirPath;
                                }
                                if (QFileInfo::exists(srcDirPath))
                                {
                                    QStringList cpArgs= m_pOstree->getCpArgs(srcDirPath, parentDestDirPath);
                                    m_pOstree->setOperation(OstreeInterface::Operation::CP);
                                    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::CP);
                                    m_pOstree->start(cpArgs, true);
                                }
                            }
                        }
                    }
                }
            }
        }

        qDebug() << "针对用户数据白名单的还原操作:结束";


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
    }

    //执行完毕，设置工作类型为空闲
    m_pOstree->setTaskType(OstreeInterface::TaskType::IDLE);
    qDebug() << "SystemRestoreProxy::restoreSystem invoke end";
}

/**
 * @brief 还原额外数据
 */
void SystemRestoreProxy::restoreExtraData()
{
    qDebug() << "SystemRestoreProxy::restoreExtraData invoke begin";

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

    QStringList args = getRsyncArgs(SystemRestoreProxy::RESTORE_EXTRA_DATA);

    m_srcPath = m_backupPath;
    QString destPath = Utils::getBackupRootPath() + "/";
    destPath.replace("//", "/");
    args << m_extraDir + "/";
    args << destPath;

    m_extraProcess = new RsyncPathToDirProcess(this);
    m_extraProcess->start(args, false);

    qDebug() << "SystemRestoreProxy::restoreExtraData invoke end";
}

bool SystemRestoreProxy::restoreFile(const QString &backupPath, const QString &targetPath) {
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

bool SystemRestoreProxy::createOverlayDirectories()
{
    bool bExist = Utils::isDirExists(TEMP_RESTORE_PATH);
    if(!bExist)
    {
        if (!Utils::mkpath(TEMP_RESTORE_PATH))
        {
            qCritical() << QString("mkdir %1 failed !").arg(TEMP_RESTORE_PATH) ;
            return false;
        }
    }

    // 遍历所有路径名
    for (const QString &pathName : m_pathNameList) {
        // 构建第一级目录路径
        QString firstLevelDirPath = QString("%1/%2-ovl").arg(TEMP_RESTORE_PATH).arg(pathName);
        bool bExist = Utils::isDirExists(firstLevelDirPath);
        if(!bExist)
        {
            if (!Utils::mkpath(firstLevelDirPath))
            {
                qCritical() << QString("mkdir %1 failed !").arg(firstLevelDirPath) ;
                return false;
            }
        }

        // 遍历ovl路径列表，创建第二级目录
        for (const QString &ovlPath : m_ovlPathList) {
            QString secondLevelDirPath = QString("%1/%2-%3").arg(firstLevelDirPath).arg(pathName).arg(ovlPath);
            bool bExist = Utils::isDirExists(secondLevelDirPath);
            if(!bExist)
            {
                if (!Utils::mkpath(secondLevelDirPath))
                {
                    qCritical() << QString("mkdir %1 failed !").arg(secondLevelDirPath) ;
                    return false;
                }
            }
        }
    }

    qDebug() << "All directories created successfully!";
    return true;
}
