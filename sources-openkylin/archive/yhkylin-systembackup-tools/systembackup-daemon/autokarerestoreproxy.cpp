/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "autokarerestoreproxy.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <unistd.h>
#include <sys/reboot.h>
#include "../common/utils.h"
#include "mymountproxy.h"
#include "translations.h"



IMPLEMENT_DYNCREATE(AutoKareRestoreProxy)

/**
 * @brief 构造函数
 */
AutoKareRestoreProxy::AutoKareRestoreProxy()
{
    m_bSuccess = false;
    m_isOem = false;
    m_extraProcess = nullptr;
}

/**
 * @brief 析构函数
 */
AutoKareRestoreProxy::~AutoKareRestoreProxy()
{
}

/**
 * @brief 环境检测
 * @return false,检测失败;true,检测成功
 */
bool AutoKareRestoreProxy::checkEnvEx()
{
    qDebug() << "AutoKareRestoreProxy::checkEnvEx invoke begin";

    //1、检测xml中的还原点是否还存在
    QString xmlPath = Utils::getBackupRootPath() + SYS_KARE_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    QList<ParseBackupList::BackupPoint> points = parse.getBackupPointList();
    if(points.size() == 0)
    {
        qCritical("xml中还原点不存在");
        emit checkResult(int(BackupResult::INC_NOT_FOUND_DIR));
        return false;
    }
    m_lastBackupPoint = points.last();

    emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));

    qDebug() << "AutoKareRestoreProxy::checkEnvEx invoke end";
    return true;
}

/**
 * @brief 执行还原逻辑
 */
void AutoKareRestoreProxy::doWorkEx()
{
    qDebug() << "AutoKareRestoreProxy::doWorkEx invoke begin";

    // 1、校验
    if (!checkEnvEx())
        return ;

    // 2、还原系统
    restoreSystem();

    qDebug() << "AutoKareRestoreProxy::doWorkEx invoke end";
}

/**
 * @brief 备份准备
 * @return true,准备成功；false，准备失败
 */
bool AutoKareRestoreProxy::doPrepare()
{
    qDebug() << "AutoKareRestoreProxy::doPrepare invoke begin";


    // 2、停止安全初始化服务，以防过高的CPU占有率，因为还原时安全初始化服务会逐个文件打标记，造成cpu占有率超高系统卡顿

    // 停止网络服务，以防网络更新
    //Utils::stopNetwork();

    qDebug() << "AutoKareRestoreProxy::doPrepare invoke end";
    return true;
}

/**
 * @brief 将dst.img文件挂载到/backup/imgbackup目录
 * @return
 */
bool AutoKareRestoreProxy::mountImg()
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
QStringList AutoKareRestoreProxy::getRsyncArgs(CustomizeSystemRestoreScene scene)
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
void AutoKareRestoreProxy::restoreSystem()
{
    qDebug() << "AutoKareRestoreProxy::restoreSystem invoke begin";

    qDebug() <<"当前还原的UUID：" << m_lastBackupPoint.m_uuid;
    //系统升级的备份点还原时，为保障断电后开机能继续还原，此处增加标志文件；还原成功后删除此文件；还原失败或断电还原未完成时，不删除此文件，等开启机器时init阶段根据是否存在标志文件继续进行系统还原。
    QString rollbackFile =  NEEDROOLBACKKAREFILE;
    qDebug() <<"创建NEEDROOLBACKFILE："<< rollbackFile ;
    Utils::createRollBackFile(rollbackFile,m_curUuid);

    // 还原前准备
    doPrepare();
    qDebug() << "m_lastBackupPoint.m_karePaths:" << m_lastBackupPoint.m_karePaths;
    QStringList karePath = m_lastBackupPoint.m_karePaths.split(";");
    qDebug() << "karePath:" << karePath;
    int progress =0;
    int total = karePath.size();
    qDebug() << "total:" << total;
    //创建信号
    bool retRestore = true;
    m_p = new RsyncPathToDirProcess(this);
    connect(m_p, &RsyncPathToDirProcess::finished, this,  [&](bool result) {
        if (!result)
        {
            retRestore = result;
        }
    });
    qDebug() << "total:" << total;
    //加入目标路径
    for(int i=0; i < total; i++)
    {
        qDebug() << "total:" << total;
        QString  item = karePath[i];
        QStringList args;

        qDebug() << "total1:" << total;
        //1.rsync参数拼接
        args << "-avAHXr";
        args << "--info=progress2";
        args << "--no-inc-recursive";
        args << "--ignore-missing-args";
        args << "--delete";
        //拼接备份源路径
        QString temp = KARE_BACKUP_PATH;
        m_srcPath = temp + "/" + m_lastBackupPoint.m_uuid + "/data";
        m_srcPath.replace("//", "/");
        QString srcPath = m_srcPath + "/" + item + "/";
        srcPath.replace("//", "/");
        args << srcPath;

        qDebug() << "total2:" << total;

        //拼接备份目标路径
        QString descPath = item + "/";
        descPath.replace("//", "/");
        args << descPath;

        qDebug() << "还原指令" << args;
        m_p->start(args, true);

        //计算当前进度值
        progress = (double)(i + 1)/total * 100;
        qDebug() << "i:"  << i << "   total:"<< total;
        qDebug() << "当前进度"  << progress;
        emit this->progress(progress);
    }

    //执行成功之后，清理kare升级备份点
    deleteXmlBackupPoint();

    //数据落盘
    ::sync();

    //清理回滚标志文件
    rollbackFile.replace("//", "/");
    QFile file(rollbackFile);
    if (file.exists())
    {
        file.remove();
        ::sync();
    }

    emit this->workResult(retRestore);
    Utils::wait(3);

    qDebug() << "AutoKareRestoreProxy::restoreSystem invoke end";
}

/**
 * @brief 还原额外数据
 */
void AutoKareRestoreProxy::restoreExtraData()
{
    qDebug() << "AutoKareRestoreProxy::restoreExtraData invoke begin";

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

    QStringList args = getRsyncArgs(AutoKareRestoreProxy::RESTORE_EXTRA_DATA);

    m_srcPath = m_backupPath;
    QString destPath = Utils::getBackupRootPath() + "/";
    destPath.replace("//", "/");
    args << m_extraDir + "/";
    args << destPath;

    m_extraProcess = new RsyncPathToDirProcess(this);
    m_extraProcess->start(args, false);

    qDebug() << "AutoKareRestoreProxy::restoreExtraData invoke end";
}

bool AutoKareRestoreProxy::restoreFile(const QString &backupPath, const QString &targetPath)
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
bool AutoKareRestoreProxy::deleteXmlBackupPoint()
{
    qDebug() << "AutoKareRestoreProxy::deleteXmlBackupPoint invoke begin";

    QString xmlPath = Utils::getBackupRootPath() + SYS_KARE_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList xmlparse(xmlPath);
    // 查询节点
    qDebug() << "要删除的备份点XML：" << xmlPath;
    qDebug() << "要删除的备份点：" << m_lastBackupPoint.m_uuid;
    if (xmlparse.deleteItem(m_lastBackupPoint.m_uuid) != ParseBackupList::SUCCESS) {
        qCritical() << QString("failed to delete %1 from backuplist.xml or sys_update_backuplist.xml!").arg(m_lastBackupPoint.m_uuid);
        return false;
    } else {
        qDebug() << "deleteXmlBackupPoint succeeded! ";
        return true;
    }

    qDebug() << "AutoKareRestoreProxy::deleteXmlBackupPoint invoke end";
}

/**
 * @brief 清理回滚的备份点
 */
bool AutoKareRestoreProxy::deleteBackupPointCommits()
{
    qDebug() << "AutoKareRestoreProxy::deleteBackupPointCommits invoke begin";

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

    qDebug() << "AutoKareRestoreProxy::deleteBackupPointCommits invoke end";
}
