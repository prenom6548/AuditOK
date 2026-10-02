/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
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
    QString time = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");
    // Utils::writeBackupLog(time + "," + m_curUuid + "," + QString::number(m_backupWrapper.m_type) + ",,," + QString::number(m_backupWrapper.m_frontUid));
    Utils::writeBackupLog(time + "," + m_curUuid + "," + QString::number(m_backupWrapper.m_type) + ",,,," + m_backupPoint.m_backupName);

    Utils::updateSyncFile();
    Utils::wait(2);
    QString fileIfSync = Utils::getSysRootPath() + FILE_IF_SYNC;
    fileIfSync.replace("//", "/");
    QFileInfo file(fileIfSync);
    QDateTime beginTime = file.fileTime(QFileDevice::FileModificationTime);
    QProcess::execute("sync");
    Utils::wait(20);
    Utils::updateSyncFile();
    while (1) {
        Utils::wait(2);
        QFileInfo file1(fileIfSync);
        QDateTime UpdateTime = file1.fileTime(QFileDevice::FileModificationTime);
        if (UpdateTime > beginTime)
            break;
    }

    // 最后，再一次还原home目录以查缺补漏，因为系统还原过程中~/.config等目录中的部分文件可能会更新，会引起部分bug（如任务栏固定图标还原不完整等）
    this->rsyncAgain();
    ::sync();

    // 2209后新增了一些依赖，还原到以前的4.0.13版后缺少依赖无法运行，故此最后还得还原backup-daemon和kybackup本身
    // 兼容Qt库从5.12.8升级到5.15，系统还原的最后也将备份还原工具本身还原
    // QString version = Utils::getBackupVersion();
    // if (version.contains("4.0.13"))
    if (Utils::getSysRootPath() != CASPER_ROOT_PATH)
    {
        QString msgOut;
        Utils::executeCMD("echo  0 > /etc/kysec/tmp/.status", msgOut);
        qDebug() << msgOut;

        // initrd.img已经还原为旧状态，需要将新版本的backup-auto-efi等脚本文件更新到initrd.img中，这样我们修改后的新逻辑才能生效
        QString msg;
        Utils::executeCMD("update-initramfs -u", msg);
        qDebug() << msg;

        // 解决bug:182576 【2203自适应升级2309】【2303自适应升级2309】升级成功还原系统失败，应该是有更新包修改了UEFI固件查找引导器文件的引导地址
        // 故系统还原完成后，重新执行下grub-install命令
        Utils::executeCMD("grub-install", msg);
	Utils::executeCMD("update-grub", msg);
        qDebug() << msg;

//                // 写入标记：rsync_backup_self:${UUID}到文件/etc/file_if_sync中，表示需要还原backup-daemon和kybackup本身
//                QString line("rsync_backup_self:");
//                line += m_curUuid;
//                Utils::syncWriteFile(fileIfSync, line);
//                this->undoRestoreData();

        // 加一层保险（因为有些机型上使用"update-initramfs -u"命令会失败，如：华为9006c的2203版等）
//                QString version = Utils::getBackupVersion();
//                if (Utils::isHuawei990() && version.contains("4.0.13"))
        // 华为9a0等机型设定1分钟后关机重启，原因：运行时还原备份还原工具本身可能会造成运行中的备份还原崩溃，从而无法执行到后面的自动重启代码，故在此预设。
        // 不再只华为机型使用此方法，将全部机型都改为此方法，缩短操作链
    //            if (Utils::isHuawei990())

            this->restartSystemAfterOneMinute();

            this->rsyncBackupSelf();

        ::sync();
    }
}

/**
 * @brief 还原备份还原工具自身
 * @note 在undoRestoreData基础上增加的一层保险（因为有些机型上使用"update-initramfs -u"命令会失败，如：华为9006c的2203版等）
 *       本来想在/etc/profile.d/目录或/etc/init.d/目录等下面增加一个.sh脚本文件以开机时自动执行，但试验了几种方式均都未执行，原因未知，怀疑是安全禁止了。
 */
void SystemRestoreWorker::rsyncBackupSelf()
{
    QString destPath = Utils::getSysRootPath() + "/usr/bin/";
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

    QString destPath = Utils::getSysRootPath() + "/home/";
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
