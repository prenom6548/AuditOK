/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "systemrestoreproxybak.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <unistd.h>
#include <sys/reboot.h>
#include "../common/utils.h"
#include "mymountproxy.h"
#include "translations.h"

IMPLEMENT_DYNCREATE(SystemRestoreProxyBak)

/**
 * @brief 构造函数
 */
SystemRestoreProxyBak::SystemRestoreProxyBak()
{
    m_bSuccess = false;
    m_isOem = false;
}

/**
 * @brief 析构函数
 */
SystemRestoreProxyBak::~SystemRestoreProxyBak()
{
    delete m_p;
}

/**
 * @brief 环境检测
 * @return false,检测失败;true,检测成功
 */
bool SystemRestoreProxyBak::checkEnvEx()
{
    qDebug() << "SystemRestoreProxyBak::checkEnvEx invoke begin";

    // 1、检测.user.txt是否存在
    m_userFile = Utils::getBackupRootPath() + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/" + PATHS_USER_FILE;
    m_userFile.replace("//", "/");
    if (!Utils::fileExists(m_userFile)) {
        qCritical(".user.txt文件不存在");
        emit checkResult(int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED));
        return false;
    }

    // 2、检测.exclude.user.txt是否存在
    m_excludeUserFile = Utils::getBackupRootPath() + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/" + EXCLUDE_PATHS_USER_FILE;
    m_excludeUserFile.replace("//", "/");
    if (!Utils::fileExists(m_excludeUserFile)) {
        qCritical(".exclude.user.txt文件不存在");
        emit checkResult(int(BackupResult::WRITE_EXCLUDE_BACKUP_PATHS_TO_USER_FAILED));
        return false;
    }

    // 3、检测还原点是否存在
    m_backupPath = Utils::getBackupRootPath() + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/data";
    m_backupPath.replace("//", "/");
    if (Utils::isDirEmpty(m_backupPath)) {
        qCritical("还原点{uuid}/data目录不存在");
        emit checkResult(int(BackupResult::INC_NOT_FOUND_DIR));
        return false;
    }

    // 4、检测xml中的还原点是否还存在
    QString xmlPath = Utils::getBackupRootPath() + BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    m_backupPoint = parse.findBackupPointByUuid(m_backupWrapper.m_uuid);
    if (m_backupPoint.m_uuid.isEmpty()) {
        qCritical("xml中还原点不存在");
        emit checkResult(int(BackupResult::INC_NOT_FOUND_DIR));
        return false;
    }

    m_curUuid = m_backupWrapper.m_uuid;

    emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));

    qDebug() << "SystemRestoreProxyBak::checkEnvEx invoke end";
    return true;
}

/**
 * @brief 执行还原逻辑
 */
void SystemRestoreProxyBak::doWorkEx()
{
    qDebug() << "SystemRestoreProxyBak::doWorkEx invoke begin";

    // 1、校验
    if (!checkEnvEx())
        return ;

    // 2、还原efi(兼容旧版本的备份)
    if (!restoreEfi()) {
        qCritical("/boot/efi目录同步失败");
        emit checkResult(int(BackupResult::EFI_RSYNC_FAIL));
        return ;
    }

    // 3、还原系统
    restoreSystem();

    qDebug() << "SystemRestoreProxyBak::doWorkEx invoke end";
}

/**
 * @brief 还原efi(兼容旧版本的备份)
 * @return
 */
bool SystemRestoreProxyBak::restoreEfi()
{
    qDebug() << "SystemRestoreProxyBak::restoreEfi invoke begin";

    // 以读写方式重新挂载boot分区，因为有的机器默认以只读挂载
    Utils::remountBoot();

    // 是否有/boot/efi目录
    QString efiPath = Utils::getBackupRootPath() + "/boot/efi";
    efiPath.replace("//", "/");
    if (!Utils::isDirEmpty(efiPath)) {
        // 1、修复源数据
        repairEfi();
        sync();

        // 2、重新rw读写挂载
        Utils::remountEfi();

        // 3、同步efi
        return rsyncEfi();
    }

    qDebug() << "SystemRestoreProxyBak::restoreEfi invoke end";
    return true;
}

/**
 * @brief 修复efi目录
 */
void SystemRestoreProxyBak::repairEfi()
{
    QString qsEfiPath = m_backupPath + "/efi";
    if (!Utils::isDirEmpty(qsEfiPath)) {
        // 存在/efi说明是老备份数据，尽量修正老数据
        QStringList args;
        args << "-f";
        args << qsEfiPath;
        QString newPath = m_backupPath + "/boot";
        args << newPath;
        QProcess::execute("mv", args);
    }
}

/**
 * @brief 同步efi
 */
bool SystemRestoreProxyBak::rsyncEfi()
{
    QString efiPath = m_backupPath + "/boot/efi/";
    if (Utils::isDirEmpty(efiPath))
        efiPath = efiPath = m_backupPath + "/efi/";
    if (Utils::isDirEmpty(efiPath))
        return true;

    QStringList args = getRsyncArgs(SystemRestoreScene::EFI_RESTORE);
    QString mountPath = Utils::getBackupRootPath() + "/boot/efi/";
    mountPath.replace("//", "/");

    args << efiPath << mountPath;

    m_p = new RsyncPathToDirProcess(this);
    bool result = m_p->start(args);
    delete m_p;
    m_p = nullptr;

    return result;
}

/**
 * @brief 根据场景获取rsync命令参数
 * @param scene，场景
 * @return 组装好的rsync的参数信息
 */
QStringList SystemRestoreProxyBak::getRsyncArgs(SystemRestoreScene scene)
{
    QStringList args;
    args << "-avAHXr";
    args << "--info=progress2";
    args << "--no-inc-recursive";
    args << "--ignore-missing-args";
    args << "--delete";

    QDir dataDir(m_srcPath + "/data");
    QFile file(m_srcPath + "/etc/uid_list");
    QDir efiDir(m_srcPath + "/boot/efi");
    QStringList excludes;

    switch (scene) {
    case SystemRestoreScene::RESTORE_SYSTEM_WITH_DATA :
        args << "--exclude=/home";
        args << "--exclude=/root";
        args << "--exclude=/data";

        if (m_curUuid == FACTORY_BACKUP_UUID) {
            qDebug() << "FACTORY_BACKUP_UUID";
//            if (Utils::isDirExist(m_srcPath + "/home/oem"))
            {
                qDebug() << "Exists /home/oem";
                args << "--exclude=/etc/lightdm/lightdm.conf";
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

        // 此处不要break，因为还需要排除SYSTEM_RESTORE中的项

    case SystemRestoreScene::SYSTEM_RESTORE :
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

        // 以前的出厂备份和grub备份没有备份/data，还原时需要判断/data是否存在，如不存在需要屏蔽掉，不然会将主机上的/data删除，造成问题
        // 此为兼容以前备份的老数据而改，等以后老的备份估计不存在了可已去掉
        if (!dataDir.exists()) {
            args << "--exclude=/data";
        }

        if (m_curUuid != FACTORY_BACKUP_UUID) {
            if (!file.exists()) {
                args << "--exclude=/etc/uid_list";
            }
        }

        // 为兼容以前的老备份数据，增加下面几行
        if (efiDir.isEmpty()) {
            args << QString("--exclude=/boot/efi");
        }
        // 系统安装后有的会将/data/home /data/root挂载到的/home /root上，实际文件是存放在/data/home /data/root下面
        Utils::excludeFstabBindPath(excludes);
        // 自定义备份的路径也需要跳过，不进行还原
        Utils::excludeCustomizePath(excludes);
        args << "--exclude=*/backup/snapshots";
        for (const QString& item : excludes) {
            QDir itemDir(m_srcPath + item);
            // 以后统一用/home /root这种路径， 兼容老备份数据（原来的U盘备份，在mksquashfs时排除bind挂载的任意一方时，都备份不上）
            if (item == "/data/home") {
                QDir homeDir(m_srcPath + "/home");
                if (!homeDir.isEmpty()) {
                    args << QString("--exclude=/data/home");
                } else if (!itemDir.isEmpty()) {
                    args << QString("--exclude=/home");
                } else {
                    args << QString("--exclude=/data/home");
                    args << QString("--exclude=/home");
                }
                continue;
            } else if (item == "/data/root") {
                QDir homeDir(m_srcPath + "/root");
                if (!homeDir.isEmpty()) {
                    args << QString("--exclude=/data/root");
                } else if (!itemDir.isEmpty()) {
                    args << QString("--exclude=/root");
                } else {
                    args << QString("--exclude=/data/root");
                    args << QString("--exclude=/root");
                }
                continue;
            }

            args << QString("--exclude=") + item;
        }

        args << "--exclude-from" << m_excludeUserFile;
        args << "--files-from" << m_userFile;

        break ;
    case SystemRestoreScene::EFI_RESTORE :
        break ;
    default:
        return args;
    }

    return args;
}

/**
 * @brief 系统还原
 */
void SystemRestoreProxyBak::restoreSystem()
{
    // 停止安全初始化服务，以防过高的CPU占有率，因为还原时安全初始化服务会逐个文件打标记，造成cpu占有率超高系统卡顿

    // 停止网络服务，以防网络更新
    Utils::stopNetwork();

    // 本地系统备份没有img挂载，故下面两个路径相等
    m_srcPath = m_backupPath;
    QString destPath = Utils::getBackupRootPath();

    QStringList args;
    // 自动更新的备份还原时保留用户数据
    if (m_curUuid == AUTO_BACKUP_UUID || m_backupWrapper.m_type == BackupType::RESTORE_SYSTEM_WITH_DATA) {
        args = getRsyncArgs(SystemRestoreScene::RESTORE_SYSTEM_WITH_DATA);
        m_bRetainUserData = true;
    } else {
        args = getRsyncArgs(SystemRestoreScene::SYSTEM_RESTORE);
    }

    args << m_srcPath + "/";
    destPath += "/";
    destPath.replace("//", "/");
    args << destPath;

    m_p = new RsyncPathToDirProcess(this);
    connect(m_p, &RsyncPathToDirProcess::progress, this, &SystemRestoreProxyBak::progress);
    connect(m_p, &RsyncPathToDirProcess::finished, this,  [&](bool result) {
        if (result) {
            this->rsyncFinished();

            emit this->workResult(result);
            if (m_isOem) {
                this->plymountMessage(Translations::instance().tr("The system will reboot automatically after the restore is successful"));
                Utils::wait(3);
            }
            Utils::wait(3);
            reboot(RB_AUTOBOOT);
        } else {
            emit this->workResult(result);
            if (m_isOem) {
                this->plymountMessage(Translations::instance().tr("Restore failed"));
            }
        }
    });

 //   m_isOem = false;
  //  if (m_curUuid == FACTORY_BACKUP_UUID) {
        // 判断是否OEM模式，如果是OEM模式则先退出图形界面，使用plymouth
    //    if (Utils::isDirExist(m_srcPath + "/home/oem")) {
            m_isOem = true;
            plymountMode();
            connect(m_p, &RsyncPathToDirProcess::progress, this, [&](int currentRate) {
                this->plymountMessage(Translations::instance().tr("Restore Progress : %1 %").arg(currentRate));
                if (100 == currentRate) {
                    Utils::wait(1);
                    this->plymountMessage(Translations::instance().tr("Saving cached data, please wait"));
                }
            });
      //  }
   // }
    m_p->start(args, false);
}

