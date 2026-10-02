/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "systemrestoreworker.h"
#include <string>
#include <unistd.h>
#include <QFileInfo>
#include <QDateTime>
#include "../common/utils.h"
#include "translations.h"

SystemRestoreWorker::SystemRestoreWorker()
{
    m_p = nullptr;
    m_bRetainUserData = false;
}

SystemRestoreWorker::~SystemRestoreWorker()
{
    delete m_p;
}

// 还原结束
void SystemRestoreWorker::rsyncFinished()
{
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    // Utils::writeBackupLog(time + "," + m_curUuid + "," + QString::number(m_backupWrapper.m_type) + ",,," + QString::number(m_backupWrapper.m_frontUid));
    Utils::writeBackupLog(time + "," + m_curUuid + "," + QString::number(m_backupWrapper.m_type) + ",,,," + m_backupPoint.m_backupName);

    Utils::updateSyncFile();
    //Utils::wait(2);
    QString fileIfSync = Utils::getBackupRootPath() + FILE_IF_SYNC;
    fileIfSync.replace("//", "/");
    QFileInfo file(fileIfSync);
    QDateTime beginTime = file.fileTime(QFileDevice::FileModificationTime);
    QProcess::execute("sync");
    //Utils::wait(5);
    Utils::updateSyncFile();
    while (1) {
        Utils::waitMsec(10);
        QFileInfo file1(fileIfSync);
        QDateTime UpdateTime = file1.fileTime(QFileDevice::FileModificationTime);
        if (UpdateTime > beginTime)
            break;
    }

    // 最后，再一次还原home目录以查缺补漏，因为系统还原过程中~/.config等目录中的部分文件可能会更新，会引起部分bug（如任务栏固定图标还原不完整等）
    //this->rsyncAgain();
    ::sync();

    // initrd.img已经还原为旧状态，需要将新版本的backup-auto-efi等脚本文件更新到initrd.img中，这样我们修改后的新逻辑才能生效
    QString msg;
    Utils::executeCMD("update-initramfs -u", msg);
    qDebug() << msg;

    // 解决bug:182576 【2203自适应升级2309】【2303自适应升级2309】升级成功还原系统失败，应该是有更新包修改了UEFI固件查找引导器文件的引导地址
    // 故系统还原完成后，重新执行下grub-install命令
    Utils::executeCMD("grub-install", msg);
    qDebug() << msg;

    //执行update-grub操作
    Utils::executeCMD("LANG=zh_CN.UTF-8 update-grub", msg);
    qDebug() << msg;

    //2025-5-16  还原完成后  全盘打标
    //2025-11-14 修改：使用echo  4 > /sys/kernel/security/ksaf/status 替代软模式，有些情况下setmode会失败，用这个来替代
    //需要先设置软模式才可以写入status文件
    QString msgSoftmode;
    //Utils::executeCMD("setstatus softmode", msgSoftmode);
    Utils::executeCMD("echo  4 > /sys/kernel/security/ksaf/status", msgSoftmode);
    qDebug() << msgSoftmode;
    //设置status，触发全盘打标
    //2025-11-14 修改：用写入8替代写入0，打标同事对全盘打标和还原打标做了区分
    QString msgOut;
    Utils::executeCMD("echo  8 > /etc/ksaf/tmp/.status", msgOut);
    qDebug() << msgOut;
}

/**
 * @brief 还原备份还原工具自身
 * @note 在undoRestoreData基础上增加的一层保险（因为有些机型上使用"update-initramfs -u"命令会失败，如：华为9006c的2203版等）
 *       本来想在/etc/profile.d/目录或/etc/init.d/目录等下面增加一个.sh脚本文件以开机时自动执行，但试验了几种方式均都未执行，原因未知，怀疑是安全禁止了。
 */
void SystemRestoreWorker::rsyncBackupSelf()
{
    QString destPath = Utils::getBackupRootPath() + "/usr/bin/";
    destPath.replace("//", "/");
    Utils::mkpath(destPath);

    RsyncPathToDirProcess * process = new RsyncPathToDirProcess(this);
    QStringList args;
    args << "-avAHXr";
    args << m_srcPath + "/usr/bin/kybackup";
    args << destPath;
    process->start(args);

    args.clear();
    args << "-avAHXr";
    args << m_srcPath + "/usr/bin/backup-daemon";
    args << destPath;
    process->start(args);

    delete process;
}

/**
 * @brief 再一次同步
 * @note 意图通过多次同步查缺补漏，如：
 *       1、因为系统还原过程中~/.config等目录中的部分文件可能会更新，会引起部分bug：
 *          a. 配网，打开奇安信浏览器，触发自动更新（或者软件商店手动更新），且奇安信默认固定在任务栏；
 *          b. 下载安装微信，固定任务栏（新装任意第三方软件均可），会删除desktop文件
 *          c. 上面两种操作，都会触发任务栏配置更新；通过监控任务栏配置文件，在系统还原过程中，可以明显的看到，在还原1%的时候，配置文件还原到历史文件，桌面图标还原，然后下一秒配置文件就又更新成当前最新的了
 */
void SystemRestoreWorker::rsyncAgain()
{
    QString msg;
    QStringList list;
    // 保留用户数据还原直接
    if (m_bRetainUserData) {
        if (m_curUuid==FACTORY_BACKUP_UUID) {
            Utils::executeCMD("apt remove kylin-os-installer -y",msg);
                list << "-rf";
                list << "/home/*/.config/customAudio.xml";
                msg=Utils::executeCmd("rm",list);

        }
        qDebug()<<msg;

        return ;
    }

    QString destPath = Utils::getBackupRootPath() + "/home/";
    destPath.replace("//", "/");
    Utils::mkpath(destPath);

    RsyncPathToDirProcess * process = new RsyncPathToDirProcess(this);
    QStringList args;
    args << "-avAHXr";
    args << "--exclude=*/backup/snapshots";
    args << m_srcPath + "/home/";
    args << destPath;
    process->start(args);

    delete process;
}

void SystemRestoreWorker::plymountMode()
{

    QString msg;
    Utils::executeCMD("/sbin/plymouthd --mode=boot --attach-to-session", msg);
    qDebug() << msg;

    Utils::executeCMD("/bin/plymouth show-splash --wait", msg);
    qDebug() << msg;

    plymountMessage(Translations::instance().tr("Do not perform other operations during restore to avoid data loss"));

    Utils::executeCMD("systemctl stop lightdm", msg);
    qDebug() << msg;
}

void SystemRestoreWorker::plymountMessage(const QString& message)
{
    QString msg;
    std::string tmp = QString("/bin/plymouth message --text '%1' ").arg(message).toStdString();
    Utils::executeCMD(tmp.data(), msg);
    qDebug() << msg;
}

void SystemRestoreWorker::restartSystemAfterOneMinute()
{
    QString msg;
    Utils::executeCMD("shutdown -r +1", msg);
    qDebug() << msg;
}
