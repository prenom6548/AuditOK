/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "filesystemworker.h"
#include "../util.h"

#include <stddef.h>
/*For PRIu64*/
#include <inttypes.h>
#include "datacdrom.h"

#include <kysdk/kysdk-system/libkydiskinfo.h>

#include "../sysresource/commoninfo.h"

typedef struct _DISK_INFO
{
    char devname[256];
    char mountdir[256];
    char type[256];
    gint percentage;
    guint64 btotal;
    guint64 bfree;
    guint64 bavail;
    guint64 bused;
    gint valid;
} DISK_INFO;

static const QStringList skipDevices = {
    "none", "tmpfs", "efivarfs", "overlay", "proc", "sysfs",
    "devtmpfs", "devpts", "cgroup", "cgroup2", "pstore", "bpf",
    "autofs",           // 自动挂载守护进程
    "fusectl",          // FUSE 控制
    "configfs",         // 内核配置
    "binfmt_misc"       // 二进制格式注册
};

static const QStringList skipTypes = {
    "tmpfs", "devtmpfs", "proc", "sysfs", "cgroup", "cgroup2",
    "pstore", "bpf", "tracefs", "debugfs", "hugetlbfs", "mqueue",
    "fuse",             // FUSE 基础类型
    "fuse.gvfsd-fuse",  // GNOME 虚拟文件系统
    "fuse.portal",      // Flatpak 门户
    "autofs",           // 自动挂载
    "configfs",         // 内核配置
    "binfmt_misc",      // 二进制格式
    "fusectl",          // FUSE 控制
    "overlay",          // 联合挂载
    "squashfs",         // 只读压缩（Snap/AppImage）
    "ecryptfs"
};

static void fsusage_stats(const FsUsage buf,
              guint64 *bused, guint64 *bfree, guint64 *bavail, guint64 *btotal,
              gint *percentage)
{
    guint64 total = buf.total;

    if (!total) {
        /* not a real device */
        *btotal = *bfree = *bavail = *bused = 0ULL;
        *percentage = 0;
    } else {
        int percent;
        *btotal = total;
        *bfree = buf.free;
        *bavail = buf.available;
        *bused = *btotal - *bfree;
        /* percent = 100.0f * *bused / *btotal; */
        percent = 100 * *bused / (*bused + *bavail);
        *percentage = CLAMP(percent, 0, 100);
    }
}

bool getDataCDRomCapacity(QString strDevId, quint64 &totalCapacity, quint64 &usedCapacity)
{
    if (!strDevId.startsWith("/dev/sr")) {
        return false;
    }

    quint64 uTotalCapacity = 0;
    DataCDROM *cdrom = new DataCDROM(strDevId);
    if (cdrom) {
        cdrom->getCDROMInfo();
        uTotalCapacity = cdrom->getCDROMCapacity();
        usedCapacity = cdrom->getCDROMUsedCapacity();
        delete cdrom;
        cdrom = nullptr;
    }
    if (uTotalCapacity > 0) {
        totalCapacity = uTotalCapacity;
        return true;
    } else {
        return false;
    }
}

DISK_INFO add_disk(const MountEntry entry, gboolean show_all_fs)
{
    Q_UNUSED(show_all_fs);
    DISK_INFO disk;
    memset(&disk, 0, sizeof(disk));
    disk.valid = 0;
    guint64 bused, bfree, bavail, btotal;
    gint percentage;
    FsUsage usage;
    diskInfo::getFsUsage(entry.mountdir, usage);
    if (usage.block_size == 0) {
        return disk;
    }

    if (entry.device == "none" || entry.device == "tmpfs" || entry.device == "efivarfs"
         || entry.device == "overlay" || entry.device == "loop" || entry.type == "loop") {
        return disk;
    }

    // 检查设备名
    if (skipDevices.contains(entry.device)) return disk;

    // 检查文件系统类型
    if (skipTypes.contains(entry.type)) return disk;

    fsusage_stats(usage, &bused, &bfree, &bavail, &btotal, &percentage);
    QByteArray devBa = entry.device.toUtf8();
    QByteArray mntBa = entry.mountdir.toUtf8();
    QByteArray typeBa = entry.type.toUtf8();

    strncpy(disk.devname, devBa.constData(), sizeof(disk.devname) - 1);
    strncpy(disk.mountdir, mntBa.constData(), sizeof(disk.mountdir) - 1);
    strncpy(disk.type, typeBa.constData(), sizeof(disk.type) - 1);

    qDebug() << "using kdk_get_diskinfo" << disk.devname << __FUNCTION__ << __LINE__;
    kdk_diskinfo *diskinfo = kdk_get_diskinfo(disk.devname);
    if (diskinfo != nullptr) {
        guint64 hardSize = diskinfo->total_size_MiB;
        disk.btotal = hardSize*1024*1024;
        kdk_free_diskinfo(diskinfo);
    }
    qDebug() << "finish kdk_free_diskinfo" << __FUNCTION__ << __LINE__ ;

    if (disk.btotal == 0) {
        qDebug() << "sdk got 0, using glibtop_get_fsusage " << __FUNCTION__ << __LINE__ ;
        disk.btotal = btotal;
    }
    disk.bfree = bfree;
    qDebug() << "using kdk_get_partition_available_space" << __FUNCTION__ << __LINE__ ;
    disk.bavail = kdk_get_partition_available_space(disk.devname)*1024*1024;
    if (disk.bavail == 0) {
        qDebug() << "sdk got 0, using glibtop_get_fsusage " << __FUNCTION__ << __LINE__ ;
        disk.bavail = bavail;
    }
    qDebug() << "finish kdk_get_partition_available_space" << __FUNCTION__ << __LINE__ ;
    disk.bused = disk.btotal - disk.bfree;
    disk.valid = 1;

    quint64 totalCapacity = 0;
    quint64 usedCapacity;
    if (getDataCDRomCapacity(disk.devname, totalCapacity, usedCapacity)) {
        disk.btotal = totalCapacity;
        disk.bused  = usedCapacity;
        disk.bavail = disk.bfree = disk.btotal - disk.bused;
    }
    disk.percentage = qreal(disk.bused)/disk.btotal*100;

    qDebug() << disk.devname << disk.btotal << disk.bfree << disk.bavail << disk.bused << disk.percentage;

    return disk;
}

FileSystemWorker::FileSystemWorker(QObject *parent)
    : QObject(parent)
{
}

FileSystemWorker::~FileSystemWorker()
{
    m_diskInfoList.clear();
}

void FileSystemWorker::onFileSystemListChanged(bool isTray)
{
    QStringList newDiskList;
    guint i;
    gboolean show_all_fs = TRUE;

    QList<MountEntry> entries = diskInfo::getMountList();
    for (i = 0; i < entries.count(); i++) {
        // 如果是托盘，并且挂载点非根目录，则暂不获取数据，等切换界面后获取数据
        if (isTray && QString(entries[i].mountdir) != "/") {
            continue;
        }
        DISK_INFO disk = add_disk(entries[i], show_all_fs);
        if (disk.valid == 1) {
            std::string formatted_dev = make_string(g_strdup(disk.devname));
            QString dev_name = QString::fromStdString(formatted_dev);
            //QString dev_name = QString(QLatin1String(disk.devname));
            newDiskList.append(dev_name);

            if (!this->isDeviceContains(dev_name)) {
                FileSystemData info;
                info.setDevName(dev_name);

                std::string formatted_mountdir(make_string(g_strdup(disk.mountdir)));
                std::string formatted_type(make_string(g_strdup(disk.type)));
                info.updateDiskInfo(QString::fromStdString(formatted_mountdir), QString::fromStdString(formatted_type),
                    formatByteCount(disk.btotal), formatByteCount(disk.bfree),
                    formatByteCount(disk.bavail), formatByteCount(disk.bused),
                    disk.percentage, disk.btotal, disk.bfree, disk.bavail, disk.bused);

//                info.updateDiskInfo(QString(QLatin1String(disk.mountdir)), QString(QLatin1String(disk.type)), QString(QLatin1String(g_format_size_full(disk.btotal, G_FORMAT_SIZE_DEFAULT))), QString(QLatin1String(g_format_size_full(disk.bfree, G_FORMAT_SIZE_DEFAULT))), QString(QLatin1String(g_format_size_full(disk.bavail, G_FORMAT_SIZE_DEFAULT))), QString(QLatin1String(g_format_size_full(disk.bused, G_FORMAT_SIZE_DEFAULT))), disk.percentage/*QString::number(disk.percentage).append("%")*/);
                this->addDiskInfo(dev_name, info);
            } else {//update info which had exists
                FileSystemData info;
                if (this->getDiskInfo(dev_name, info)) {

                    //std::string formatted_mountdir(make_string(g_strdup(disk.mountdir)));
                    std::string formatted_type(make_string(g_strdup(disk.type)));
                    info.updateDiskInfo(info.mountDir(), QString::fromStdString(formatted_type),
                        formatByteCount(disk.btotal), formatByteCount(disk.bfree),
                        formatByteCount(disk.bavail), formatByteCount(disk.bused),
                        disk.percentage, disk.btotal, disk.bfree, disk.bavail, disk.bused);

                    this->updateDiskInfo(dev_name, info);
                }
            }
        }
    }

    //remove the device whice not exists anymore
    for (auto device : m_diskInfoList.keys()) {
        bool foundDevice = false;
        for (auto devName : newDiskList) {
            if (devName == device) {
                foundDevice = true;
                break;
            }
        }

        if (!foundDevice) {
            m_diskInfoList.remove(device);//or erase???
        }
    }
}

bool FileSystemWorker::getDiskInfo(const QString &devname, FileSystemData& info)
{
    if (!m_diskInfoList.contains(devname)) {
         return false;
    } else {
        info = m_diskInfoList[devname];
        return true;
    }
}

QList<FileSystemData> FileSystemWorker::diskInfoList() const
{
    return m_diskInfoList.values();
}

QList<QString> FileSystemWorker::diskDevNameList() const
{
    return m_diskInfoList.keys();
}

void FileSystemWorker::addDiskInfo(const QString &devname, FileSystemData& info)
{
    if (!m_diskInfoList.contains(devname)) {
        m_diskInfoList[devname] = info;
    }
}

void FileSystemWorker::updateDiskInfo(QString devname, FileSystemData& info)
{
    if (m_diskInfoList.contains(devname)) {
        m_diskInfoList[devname] = info;
    }
}

bool FileSystemWorker::isDeviceContains(const QString &devname)
{
    return m_diskInfoList.keys().contains(devname);
}
