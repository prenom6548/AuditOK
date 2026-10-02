/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "udisksystemrestoreproxy.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <unistd.h>
#include <sys/reboot.h>
#include "../common/utils.h"
#include "mymountproxy.h"
#include "myprocess/mksquashfsprocess.h"
#include <QDBusInterface>
#include <QDBusPendingReply>

IMPLEMENT_DYNCREATE(UDiskSystemRestoreProxy)

/**
 * @brief 构造函数
 */
UDiskSystemRestoreProxy::UDiskSystemRestoreProxy()
{
    m_isFinished = false;
    m_isForce = false;
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

    // 1、检测.user.txt是否存在
    m_userFile = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/" + PATHS_USER_FILE;
    m_userFile.replace("//", "/");
    if (!Utils::fileExists(m_userFile)) {
        qCritical(".user.txt文件不存在");
        emit checkResult(int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED));
        return false;
    }

    // 2、检测.exclude.user.txt是否存在
    m_excludeUserFile = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/" + EXCLUDE_PATHS_USER_FILE;
    m_excludeUserFile.replace("//", "/");
    if (!Utils::fileExists(m_excludeUserFile)) {
        qCritical(".exclude.user.txt文件不存在");
        emit checkResult(int(BackupResult::WRITE_EXCLUDE_BACKUP_PATHS_TO_USER_FAILED));
        return false;
    }

    // 3、检测还原点是否存在
    m_backupPath = m_backupWrapper.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/data";
    m_backupPath.replace("//", "/");
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
    if (!checkEnvEx())
        return ;

    // 2、还原efi(兼容旧版本的备份)
    if (!restoreEfi()) {
        qCritical("/boot/efi目录同步失败");
        emit checkResult(int(BackupResult::EFI_RSYNC_FAIL));
        return ;
    }

    // 3、还原系统
    if (doPrepare())
        restoreSystem();

    qDebug() << "UDiskSystemRestoreProxy::doWorkEx invoke end";
}

/**
 * @brief 还原efi(兼容旧版本的备份)
 * @return
 */
bool UDiskSystemRestoreProxy::restoreEfi()
{
    qDebug() << "UDiskSystemRestoreProxy::restoreEfi invoke begin";

    // 以读写方式重新挂载boot分区，因为有的机器默认以只读挂载
    Utils::remountBoot();

    // 是否有/boot/efi目录
    QString efiPath = Utils::getSysRootPath() + "/boot/efi";
    efiPath.replace("//", "/");
    if (!Utils::isDirEmpty(efiPath)) {
        // 1、修复源数据
        repairEfi();

        // 2、重新rw读写挂载
        Utils::remountEfi();

        // 3、同步efi
        return rsyncEfi();
    }

    qDebug() << "UDiskSystemRestoreProxy::restoreEfi invoke end";
    return true;
}

/**
 * @brief 修复efi目录
 */
void UDiskSystemRestoreProxy::repairEfi()
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
        QProcess::execute("sync");
    }
}

/**
 * @brief 同步efi
 */
bool UDiskSystemRestoreProxy::rsyncEfi()
{
    QString efiPath = m_backupPath + "/boot/efi/";
    if (Utils::isDirEmpty(efiPath))
        efiPath = efiPath = m_backupPath + "/efi/";
    if (Utils::isDirEmpty(efiPath))
        return true;

    QStringList args = getRsyncArgs(SystemRestoreScene::EFI_RESTORE);
    QString mountPath = Utils::getSysRootPath() + "/boot/efi/";
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
QStringList UDiskSystemRestoreProxy::getRsyncArgs(SystemRestoreScene scene)
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
            for (int i = 0; i < stringList.size(); ++i) {                   // 添加了对size()的检查
                QString PathBox = stringList.at(i);                         // 使用.at()而不是()，虽然()在这里也可以工作，但.at()在索引无效时会抛出更明确的异常
                args << QString("--exclude=") + PathBox;
            }
        } else {
            qDebug() << "没有获取到 Box Root Entries。";
        }
    } else {
        qDebug() << "D-Bus 方法调用失败:" << reply.errorMessage();
    }

    switch (scene) {
    case SystemRestoreScene::RESTORE_SYSTEM_WITH_DATA :
        args << "--exclude=/home";
        args << "--exclude=/root";
//        if (Utils::isHuawei990()) {
            args << "--exclude=/data";
//        } else {
//            args << "--exclude=/data/usershare";
//        }
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

        if (Utils::getSysRootPath() != CASPER_ROOT_PATH) {
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

        // 文件安全箱
        args << "--exclude=/data/security-dir";

        args << "--exclude=/usr/share/plymouth";

        // 以前的出厂备份和grub备份没有备份/data，还原时需要判断/data是否存在，如不存在需要屏蔽掉，不然会将主机上的/data删除，造成问题
        // 此为兼容以前备份的老数据而改，等以后老的备份估计不存在了可已去掉
        if (!dataDir.exists()) {
            args << "--exclude=/data";
        }

        if (!file.exists()) {
            args << "--exclude=/etc/uid_list";
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

        // 异机还原
        if (m_backupWrapper.m_isOtherMachine) {
            args << "--exclude=/etc/.bootinfo";
            args << "--exclude=/etc/fstab";
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
 * @brief 还原前准备
 * @return
 */
bool UDiskSystemRestoreProxy::doPrepare()
{
    qDebug() << "UDiskSystemRestoreProxy::doPrepare invoke begin";

    // 移动设备系统备份如果有img，则需要先将img挂载到/backup/imgbackup目录
    QString imgPath = m_backupPath + "/" + UDISK_MKSQUASHFS_IMG_NAME;
    if (Utils::fileExists(imgPath)) {
        // 1、检测目录/backup/imgbackup是否存在，不存在则创建此目录
        QString dstImgMountPath = Utils::getSysRootPath() + BACKUP_IMGBACKUP_PATH;
        dstImgMountPath.replace("//", "/");
        Utils::mkpath(dstImgMountPath);

        // 2、先卸载/backup/imgbackup上的mount
        MountBackupProcess *processMount = new MountBackupProcess(this);
        processMount->umount(dstImgMountPath);

        // 3、将img文件挂载到/backup/imgbackup上
        if (!processMount->mount(imgPath, dstImgMountPath)) {
            emit checkResult(int(BackupResult::RESTOREDIR_PREPARE_FAILED));
            delete processMount;
            return false;
        }

        m_srcPath = dstImgMountPath;
        delete processMount;
    } else
        m_srcPath = m_backupPath;

    qDebug() << "UDiskSystemRestoreProxy::doPrepare invoke end";
    return true;
}

/**
 * @brief 系统还原
 */
void UDiskSystemRestoreProxy::restoreSystem()
{
    qDebug() << "UDiskSystemRestoreProxy::restoreSystem invoke begin";

    // 理论上开始不会走下面这个U盘拔出的校验
    if (m_isForce) {
        qCritical("U盘已拔出");
        emit checkResult(int(BackupResult::INC_NOT_FOUND_DIR));
        return ;
    }

    // 停止安全初始化服务，以防过高的CPU占有率，因为还原时安全初始化服务会逐个文件打标记，造成cpu占有率超高系统卡顿
    Utils::stopKysecInit();
    // 停止网络服务，以防网络更新
    Utils::stopNetwork();

    QString destPath = Utils::getSysRootPath();

    QStringList args;
    // 自动更新的备份还原时保留用户数据
    if (m_backupWrapper.m_type == BackupType::RESTORE_SYSTEM_WITH_DATA) {
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
    connect(m_p, &RsyncPathToDirProcess::progress, this, &UDiskSystemRestoreProxy::progress);
    connect(m_p, &RsyncPathToDirProcess::finished, this,  [&](bool result) {
        m_isForce = false;
        if (result) {
            this->rsyncFinished();

            if (m_backupWrapper.m_isOtherMachine) {
                Utils::wait(5);
                updateGrubUUid();
                ::sync();
            }

            if (Utils::isDirEmpty(m_backupPath))
                result = false;
            emit this->workResult(result);
            m_isFinished = true;

            if (result) {
                Utils::wait(3);
                reboot(RB_AUTOBOOT);
            }
        }
    });

    QTimer::singleShot(1*1000, this, &UDiskSystemRestoreProxy::checkUdiskExists);
    m_isFinished = false;
    m_p->start(args, false);

    qDebug() << "UDiskSystemRestoreProxy::restoreSystem invoke end";
}

/**
 * @brief 异机还原时更新grub.cfg中的分区UUID
 */
void UDiskSystemRestoreProxy::updateGrubUUid()
{
    QString srcFstab = Utils::getSysRootPath() + BACKUP_IMGBACKUP_PATH;
    srcFstab += FSTAB_PATH;
    srcFstab.replace("//", "/");
    QHash<QString, QString> srcPartToUuid = Utils::getPartUuidMap(srcFstab);
    QString destFstab = Utils::getSysRootPath() + FSTAB_PATH;
    destFstab.replace("//", "/");
    QHash<QString, QString> destPartToUuid = Utils::getPartUuidMap(destFstab);

    QString findGrub = Utils::executeCmd("find /boot -name grub.cfg");
    QStringList grubs = findGrub.split("\n");
    for (const QString &grub : grubs) {
       if (grub.isEmpty())
           continue;
       QString root = QString("sed -i 's/%1/%2/g' %3").arg(srcPartToUuid.value("/")).arg(destPartToUuid.value("/")).arg(grub);
       qDebug() << Utils::executeCmd(root);

       QString boot = QString("sed -i 's/%1/%2/g' %3").arg(srcPartToUuid.value("/boot")).arg(destPartToUuid.value("/boot")).arg(grub);
       qDebug() << Utils::executeCmd(boot);

       QString swap = QString("sed -i 's/%1/%2/g' %3").arg(srcPartToUuid.value("swap")).arg(destPartToUuid.value("swap")).arg(grub);
       qDebug() << Utils::executeCmd(swap);
    }
}

/**
 * @brief 监控移动设备是否还在
 * @return true-在；false-不在
 */
bool UDiskSystemRestoreProxy::checkUdiskExists()
{
    if (!m_isFinished) {
        // 拔掉U盘后，没有响应的场景（怀疑可能是某应用程序关闭引起，希望不是dbus服务关掉了）
        if (m_isForce) {
            qCritical() << QString("强制退出");
            emit this->workResult(false);
            return false;
        }

        if (Utils::isDirEmpty(m_backupPath)) {
            qCritical() << QString("srcDir %s is not exist!").arg(m_backupPath);
            m_isForce = true;
            if (m_p != nullptr)
                m_p->stop();
            // 10s钟后如果还没有退出，则强制退出
            QTimer::singleShot(10*1000, this, &UDiskSystemRestoreProxy::checkUdiskExists);
        } else {
            QTimer::singleShot(1*1000, this, &UDiskSystemRestoreProxy::checkUdiskExists);
        }
    }

    return true;
}


