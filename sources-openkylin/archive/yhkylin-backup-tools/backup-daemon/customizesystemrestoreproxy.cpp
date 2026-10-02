/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "customizesystemrestoreproxy.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <unistd.h>
#include <sys/reboot.h>
#include "../common/utils.h"
#include "mymountproxy.h"
#include "translations.h"
#include <QDBusInterface>
#include <QDBusPendingReply>

IMPLEMENT_DYNCREATE(CustomizeSystemRestoreProxy)

/**
 * @brief 构造函数
 */
CustomizeSystemRestoreProxy::CustomizeSystemRestoreProxy()
{
    m_bSuccess = false;
    m_isOem = false;
    m_extraProcess = nullptr;
}

/**
 * @brief 析构函数
 */
CustomizeSystemRestoreProxy::~CustomizeSystemRestoreProxy()
{
}

/**
 * @brief 环境检测
 * @return false,检测失败;true,检测成功
 */
bool CustomizeSystemRestoreProxy::checkEnvEx()
{
    qDebug() << "CustomizeSystemRestoreProxy::checkEnvEx invoke begin";

    // 1、检测xml中的还原点是否还存在
    QString uniXmlPath = Utils::getSysRootPath() + BACKUP_XML_PATH;
    uniXmlPath.replace("//", "/");
    ParseBackupList uniParse(uniXmlPath);
    m_backupPoint = uniParse.findBackupPointByUuid(m_backupWrapper.m_uuid);
    qDebug()<<__LINE__<<__func__<<"出厂备份"<<m_backupWrapper.m_uuid;
    qDebug()<<__LINE__<<__func__<<"出厂备份111"<<m_backupPoint.m_uuid;
    QString xmlPath = Utils::getSysRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    if (m_backupPoint.isNull())
    {
        qDebug()<<__LINE__<<__func__<<"出厂备份222"<<m_backupWrapper.m_uuid;

         m_backupPoint = parse.findBackupPointByUuid(m_backupWrapper.m_uuid);
    }
    else
    {
        qDebug()<<__LINE__<<__func__<<"出厂备份333"<<m_backupWrapper.m_uuid;
        // 兼容早期备份，
//        if (m_backupWrapper.m_uuid == AUTO_BACKUP_UUID)
//        {
//            m_backupPoint.m_iPosition = BackupPosition::CUSTOMIZE;
//            parse.updateItem(m_backupPoint);
//            uniParse.deleteItem(m_backupPoint.m_uuid);
//        }
    }

    if (m_backupPoint.isNull()) {
        qCritical("xml中还原点不存在");
        emit checkResult(int(BackupResult::INC_NOT_FOUND_DIR));
        return false;
    }

    // 2、检测.user.txt是否存在
    m_userFile = Utils::getSysRootPath() + m_backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/" + PATHS_USER_FILE;
    m_userFile.replace("//", "/");
    if (!Utils::fileExists(m_userFile)) {
        qCritical(".user.txt文件不存在");
        emit checkResult(int(BackupResult::WRITE_BACKUP_PATHS_TO_USER_FAILED));
        return false;
    }

    // 3、检测.exclude.user.txt是否存在
    m_excludeUserFile = Utils::getSysRootPath() + m_backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/" + EXCLUDE_PATHS_USER_FILE;
    m_excludeUserFile.replace("//", "/");
    if (!Utils::fileExists(m_excludeUserFile)) {
        qCritical(".exclude.user.txt文件不存在");
        emit checkResult(int(BackupResult::WRITE_EXCLUDE_BACKUP_PATHS_TO_USER_FAILED));
        return false;
    }

    // 4、检测还原点是否存在
    m_backupPath = Utils::getSysRootPath() + m_backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + m_backupWrapper.m_uuid + "/data";
    m_backupPath.replace("//", "/");
    if (Utils::isDirEmpty(m_backupPath)) {
        qCritical("还原点{uuid}/data目录不存在");
        emit checkResult(int(BackupResult::INC_NOT_FOUND_DIR));
        return false;
    }
    m_imgFileName = m_backupPath + "/" + UDISK_MKSQUASHFS_IMG_NAME;
    if (!Utils::fileExists(m_imgFileName)) {
        QString bootDir = m_backupPath + "/boot";
        if (Utils::isDirEmpty(bootDir)) {
            qCritical("还原点{uuid}/data/dst.img文件不存在");
            emit checkResult(int(BackupResult::INC_NOT_FOUND_DIR));
            return false;
        } else {
            // 非压缩备份点
            m_backupPointType = RSYNC_BACKUP;
        }
    } else {
        m_backupPointType = MKSQUASHFS_COMPRESS;
    }

    m_curUuid = m_backupWrapper.m_uuid;

    emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));

    qDebug() << "CustomizeSystemRestoreProxy::checkEnvEx invoke end";
    return true;
}

/**
 * @brief 执行还原逻辑
 */
void CustomizeSystemRestoreProxy::doWorkEx()
{
    qDebug() << "CustomizeSystemRestoreProxy::doWorkEx invoke begin";

    // 1、校验
    if (!checkEnvEx())
        return ;

    // 2、还原系统
    restoreSystem();

    qDebug() << "CustomizeSystemRestoreProxy::doWorkEx invoke end";
}

/**
 * @brief 备份准备
 * @return true,准备成功；false，准备失败
 */
bool CustomizeSystemRestoreProxy::doPrepare()
{
    qDebug() << "CustomizeSystemRestoreProxy::doPrepare invoke begin";

    // 1、设置还原时数据源目录
    if (m_backupPointType == RSYNC_BACKUP) {
        m_srcPath = m_backupPath;
    } else
    {
        // mksquashfs压缩备份场景，dst.img文件需要挂载到目录
        if (!mountImg())
            return false;
    }

    // 2、停止安全初始化服务，以防过高的CPU占有率，因为还原时安全初始化服务会逐个文件打标记，造成cpu占有率超高系统卡顿
    Utils::stopKysecInit();
    // 停止网络服务，以防网络更新
    Utils::stopNetwork();

    // 3、以读写方式重新挂载boot分区，因为有的机器默认以只读挂载
    Utils::remountBoot();

    // 4、是否有/boot/efi目录，有则认为有efi分区，需重新rw挂载
    QString efiPath = Utils::getSysRootPath() + "/boot/efi";
    efiPath.replace("//", "/");
    if (!Utils::isDirEmpty(efiPath)) {
        // 重新rw读写挂载
        Utils::remountEfi();
    }

    qDebug() << "CustomizeSystemRestoreProxy::doPrepare invoke end";
    return true;
}

/**
 * @brief 将dst.img文件挂载到/backup/imgbackup目录
 * @return
 */
bool CustomizeSystemRestoreProxy::mountImg()
{
    // 自定义路径系统备份，需要先将dst.img文件挂载到/backup/imgbackup目录
    // 1、检测目录/backup/imgbackup是否存在，不存在则创建此目录
    QString dstImgMountPath = Utils::getSysRootPath() + BACKUP_IMGBACKUP_PATH;
    dstImgMountPath.replace("//", "/");
    Utils::mkpath(dstImgMountPath);

    // 2、先卸载/backup/imgbackup上的mount
    MountBackupProcess *processMount = new MountBackupProcess(this);
    processMount->umount(dstImgMountPath);

    // 3、将img文件挂载到/backup/imgbackup上
    if (!processMount->mount(m_imgFileName, dstImgMountPath)) {
        emit checkResult(int(BackupResult::RESTOREDIR_PREPARE_FAILED));
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
QStringList CustomizeSystemRestoreProxy::getRsyncArgs(CustomizeSystemRestoreScene scene)
{
    QStringList args;
    args << "-avAHXr";
    args << "--info=progress2";
    args << "--no-inc-recursive";
    args << "--ignore-missing-args";
    args << "--delete";

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
        args << "--exclude=/home/kylin/.box";

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
void CustomizeSystemRestoreProxy::restoreSystem()
{
    qDebug() << "CustomizeSystemRestoreProxy::restoreSystem invoke begin";

    // 还原前准备
    doPrepare();

    QString destPath = Utils::getSysRootPath();

    QStringList args;
    // 保留用户数据还原
    if (m_curUuid == AUTO_BACKUP_UUID || m_backupWrapper.m_type == BackupType::RESTORE_SYSTEM_WITH_DATA) {
        args = getRsyncArgs(CustomizeSystemRestoreScene::RESTORE_SYSTEM_WITH_DATA);
        m_bRetainUserData = true;
    } else {
            // 创建一个DBus接口
            QDBusInterface backupInterface("com.kylin.boxmanage", "/backup", "com.kylin.boxmanage.backup", QDBusConnection::systemBus());

            // 调用DBus方法
             QDBusPendingReply<int> reply = backupInterface.call("format_allBoxRootEntries");

            // 检查调用是否成功
            int resultCode = reply.value();
            if (resultCode == 0) {
                // 操作成功
                qDebug() << "DBus method call succeeded."<< resultCode;;
            } else {
                // 操作失败
                qDebug() << "DBus method call failed with error code:" << resultCode;
                // 在这里处理失败的情况，比如输出报错信息
                // 注意：这里可以根据需要添加更详细的错误处理逻辑
                }
            args = getRsyncArgs(CustomizeSystemRestoreScene::SYSTEM_RESTORE);
    }

    args << m_srcPath + "/";
    destPath += "/";
    destPath.replace("//", "/");
    args << destPath;

    m_p = new RsyncPathToDirProcess(this);
    connect(m_p, &RsyncPathToDirProcess::progress, this, &CustomizeSystemRestoreProxy::progress);
    connect(m_p, &RsyncPathToDirProcess::finished, this,  [&](bool result) {
        if (result) {
            this->rsyncFinished();

            // 系统升级的备份点还原时，为保障断电后开机能继续还原，此处增加标志文件；还原成功后删除此文件；还原失败或断电还原未完成时，不删除此文件，等开启机器时init阶段根据是否存在标志文件继续进行系统还原。
            if (m_curUuid == AUTO_BACKUP_UUID) {
                QString rollbackFile = Utils::getSysRootPath() + FLAG_FILE_SYSTEM_UPDATE_NEED_ROLLBACK;
                rollbackFile.replace("//", "/");
                QFile file(rollbackFile);
                if (file.exists()) {
                    file.remove();
                    ::sync();
                }
            }

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

  //  m_isOem = false;
   // if (m_curUuid == FACTORY_BACKUP_UUID) {
        // 判断是否OEM模式，如果是OEM模式则先退出图形界面，使用plymouth
     //   if (Utils::isDirExist(m_srcPath + "/home/oem")) {
            m_isOem = true;
            plymountMode();
            connect(m_p, &RsyncPathToDirProcess::progress, this, [&](int currentRate) {
                this->plymountMessage(Translations::instance().tr("Restore Progress : %1 %").arg(currentRate));
                if (100 == currentRate) {
                    Utils::wait(1);
                    this->plymountMessage(Translations::instance().tr("Saving cached data, please wait"));
                }
            });
        //}
   // }
    m_p->start(args, false);

    // 系统升级的备份点还原时，为保障断电后开机能继续还原，此处增加标志文件；还原成功后删除此文件；还原失败或断电还原未完成时，不删除此文件，等开启机器时init阶段根据是否存在标志文件继续进行系统还原。
    if (m_curUuid == AUTO_BACKUP_UUID) {
        QString rollbackFile = Utils::getSysRootPath() + FLAG_FILE_SYSTEM_UPDATE_NEED_ROLLBACK;
        rollbackFile.replace("//", "/");
        Utils::syncWriteFile(rollbackFile, QString("UUID=") + AUTO_BACKUP_UUID);
    }

    restoreExtraData();

    qDebug() << "CustomizeSystemRestoreProxy::restoreSystem invoke end";
}

/**
 * @brief 还原额外数据
 */
void CustomizeSystemRestoreProxy::restoreExtraData()
{
    qDebug() << "CustomizeSystemRestoreProxy::restoreExtraData invoke begin";

    m_extraDir = Utils::getSysRootPath() + m_backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid + "/extra_data";
    m_extraDir.replace("//", "/");
    if (Utils::isDirEmpty(m_extraDir)){
        qDebug() << m_extraUserFile << " is empty";
        return ;
    }

    m_extraUserFile = Utils::getSysRootPath() + m_backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid + "/" + EXTRA_PATHS_USER_FILE;
    m_extraUserFile.replace("//", "/");
    if (!Utils::fileExists(m_extraUserFile)) {
        qDebug() << m_extraUserFile << " not exists";
        return ;
    }

    QStringList args = getRsyncArgs(CustomizeSystemRestoreProxy::RESTORE_EXTRA_DATA);

    m_srcPath = m_backupPath;
    QString destPath = Utils::getSysRootPath() + "/";
    destPath.replace("//", "/");
    args << m_extraDir + "/";
    args << destPath;

    m_extraProcess = new RsyncPathToDirProcess(this);
    m_extraProcess->start(args, false);

    qDebug() << "CustomizeSystemRestoreProxy::restoreExtraData invoke end";
}

