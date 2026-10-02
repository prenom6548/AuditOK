/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "partition_usage.h"
#include "datastruct.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/sysInfo/swaps.h"
#include "../PluginService/base_unit/string_unit.h"

#include <QDebug>

using namespace KServer;
namespace KInstaller {
namespace Partman {
qint64 getSizeofByte(const QString& value)
{
    const float pref = getRegex("^(\\d+\\.?\\d+", value).toFloat();
    if(value.contains("KB")) {
        return static_cast<qint64>(pref * kKibiByte);
    }
    if(value.contains("MB")) {
        return static_cast<qint64>(pref * kMebiByte);
    }
    if(value.contains("GB")) {
        return static_cast<qint64>(pref * kGibiByte);
    }
    if(value.contains("TB")) {
        return static_cast<qint64>(pref * kTebiByte);
    }
    if(value.contains("PB")) {
        return static_cast<qint64>(pref * kPebiByte);
    }
    return -1;
}

bool getBtrFSSizeInfo(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand(
                "btrfs", {"filesystem", "show", partitionpath}, dir, output, error, exitcode)) {
        return false;
    }
    QString totalStr = "";
    QString usedStr = "";
    for(const QString&line : output.split('\n')) {
        if(line.contains(partitionpath)) {
            totalStr = getRegex("size\\s*([^\\s]*)\\s", line);
        } else if(line.contains("Total devices")) {
            usedStr = getRegex("used\\s*([^\\s]*)", line);
        }
    }
    total = getSizeofByte(totalStr);
    freeface = total - getSizeofByte(usedStr);
    return (total > -1 && freeface >-1);
}

bool getExt2SizeInfo(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand(
                "dumpe2fs", {"-h", partitionpath}, dir, output, error, exitcode)) {
        return false;
    }
    int blockSize = 0;
    qint64 totalBlocks = 0;
    qint64 freeBlocks = 0;

    for(const QString& line : output.split('\n')) {
        if(line.contains("Block count:")) {
            const QString m = getRegex("Block count:\\s+(\\d+)", line);
            if(!m.isEmpty()) {
                totalBlocks = m.toLongLong();
            }
        } else if(line.contains("Free blocks:")) {
            const QString m = getRegex("Free blocks:\\s+(\\d+)", line);
            if(!m.isEmpty()) {
                freeBlocks = m.toLongLong();
            }
        } else if(line.contains("Block size:")) {
            const QString m =getRegex("Block size:\\s+(\\d+)", line);
            if(!m.isEmpty()) {
                blockSize = m.toInt();
            }
        }
    }
    freeface = blockSize * freeBlocks;
    total = blockSize * totalBlocks;
    return true;
}

bool getFat16SizeInfo(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    KServer::KCommand::getInstance()->RunScripCommand(
                    "dosfsck", {"-n", "-v", partitionpath}, dir, output, error, exitcode);
    if(!error.isEmpty()) {
        qWarning() << "dosfsck failed:" << error;
        return false;
    }

    int clusterSize = 0;
    qint64 startByte = 0;
    qint64 totalCluster = 0;
    qint64 usedCluster = 0;

    for(const QString& line : output.split('\n')) {
        if(line.contains("bytes per cluster")) {
            clusterSize =  (line.trimmed().split(' ').at(0)).trimmed().toInt();
        } else if(line.contains("Data area starts at")) {
            startByte = line.split(' ').at(5).toLongLong();
        } else if(line.contains(partitionpath)) {
            const QStringList parts = line.split(' ').at(3).split('/');
            totalCluster = parts.at(1).toLongLong();
            usedCluster = parts.at(0).toLongLong();
        }
    }

   total =totalCluster * clusterSize;
   freeface = total - startByte - usedCluster * clusterSize;
   return true;
}

bool getSwapSizeInfo(const QString& partitionpath, qint64& freeface, qint64& total)
{
    const SwapItemList swapItems = getprocSwaps();
    for(const SwapItem& item : swapItems) {
        if(item.filename == partitionpath) {
            total = item.size;
            freeface = item.size - item.used;
            return true;
        }
    }
    freeface = 0;
    total = 0;
    return true;
}

bool getJfsSizeInfo(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    const QString param(QString("echo dm | jfs_debugfs %1").arg(partitionpath));
    if(!KServer::KCommand::getInstance()->RunScripCommand("sh", {"-c", param}, dir, output, error, exitcode)) {
        return false;
    }

    int blockSize = 0;
    qint64 totalBlocks = 0;
    qint64 freeBlocks = 0;
    for(const QString& line : output.split('\n')) {
        if(line.startsWith("Aggregate Block Size:")) {
            blockSize = line.split(':').at(1).trimmed().toInt();
        } else if(line.contains("dn_mapsize:")) {
            const QString item = getRegex("dn_mapsize:\\s*([^\\s]+)", line);
            totalBlocks = item.toLongLong(nullptr, 16);
        } else if(line.contains("dn_nfree:")) {
            const QString item = getRegex("dn_nfree:\\s*([^\\s]+)", line);
            freeBlocks = item.toLongLong(nullptr, 16);
        }
    }
    if(freeBlocks > 0 && totalBlocks > 0 && blockSize > 0) {
        freeface = freeBlocks * blockSize;
        total = totalBlocks * blockSize;
        return true;
    } else {
        return false;
    }
    return true;
}

bool getReiser4SizeInfo(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand("debugfs.reiser4",
                     {"--force", "--yes", partitionpath}, dir, output, error, exitcode)) {
        return false;
    }
    int blockSize = 0;
    qint64 freeBlocks = 0;
    qint64 totalBlocks =0;

    for(const QString& line : output.split('\n')) {
        if(line.startsWith("blksize:")) {
            blockSize = line.split(':').at(1).trimmed().toInt();
        } else if(line.startsWith("blocks:")) {
            totalBlocks = line.split(':').at(1).trimmed().toLongLong();
        } else if(line.startsWith("free blocks:")) {
            freeBlocks = line.split(':').at(1).trimmed().toLongLong();
        }
    }
    if(freeBlocks > 0 && totalBlocks > 0 && blockSize > 0) {
        total = totalBlocks * blockSize;
        freeface = freeBlocks * blockSize;
        return true;
    } else {
        return false;
    }
    return true;
}


bool getReiserSizeInfo(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand("debugreiserfs",
                     {"-d", partitionpath}, dir, output, error, exitcode)) {
        return false;
    }
    int blockSize = 0;
    qint64 freeBlocks = 0;
    qint64 totalBlocks =0;

    for(const QString& line : output.split('\n')) {
        if(line.startsWith("Count of blocks on the device:")) {
            totalBlocks = line.split(':').last().trimmed().toLongLong();
        } else if(line.startsWith("Blocksize:")) {
            blockSize = line.split(':').last().trimmed().toInt();
        } else if(line.startsWith("Free blocks (count of blocks)")) {
            freeBlocks = line.split(':').last().trimmed().toLongLong();
        }
    }
    if(freeBlocks > 0 && totalBlocks > 0 && blockSize > 0) {
        total = totalBlocks * blockSize;
        freeface = freeBlocks * blockSize;
        return true;
    } else {
        return false;
    }
    return true;
}

bool getXFSSizeInfo(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand("xfs_db",
                     {"-c sb", "-c print", "-r", partitionpath}, dir, output, error,exitcode)) {
        return false;
    }

    if(output.isEmpty()) {
        return false;
    }

    int blockSize = 0;
    qint64 freeBlocks = 0;
    qint64 totalBlocks =0;

    for(const QString& line : output.split('\n')) {
        if(line.startsWith("fdblocks")) {
            freeBlocks = line.split('=').last().trimmed().toLongLong();
        } else if(line.startsWith("dblocks")) {
            totalBlocks = line.split('=').last().trimmed().toLongLong();
        } else if(line.startsWith("blocksize")) {
            blockSize = line.split('=').last().trimmed().toInt();
        }
    }
    if(freeBlocks > 0 && totalBlocks > 0 && blockSize > 0) {
        total = totalBlocks * blockSize;
        freeface = freeBlocks * blockSize;
        return true;
    } else {
        return false;
    }
    return true;
}

bool getNtFSSizeInfo(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand(
                    "ntfsinfo", {"-mf", partitionpath}, dir, output, error, exitcode))
    {
        return false;
    }

    int clusterSize = 0;
    qint64 totalCluster = 0;
    qint64 freeCluster = 0;

    for(const QString& line : output.split('\n')) {
        if(line.contains("Cluster Size")) {
            const QString m =  getRegex("Cluster Size:\\s+(\\d+)", line);
            if(!m.isEmpty()) {
                clusterSize = m.toInt();
            }
        } else if(line.contains("Volume Size in Clusters:")) {
            const QString m =  getRegex("Volume Size in Clusters:\\s+(\\d+)", line);
            if(!m.isEmpty()) {
                totalCluster = m.toLongLong();
            }
        } else if(line.contains("Free Clusters:")) {
            const QString m =  getRegex("Free Clusters:\\s+(\\d+)", line);
            if(!m.isEmpty()) {
                freeCluster = m.toLongLong();
            }
        }
    }

    if(freeCluster > 0 && totalCluster > 0 && clusterSize > 0) {
        freeface = freeCluster * clusterSize;
        total = totalCluster * clusterSize;
        return true;
    } else {
        return false;
    }
    return true;
}

bool getSizeInfo(const QString& partitionpath, FSType fs, qint64& freeface, qint64& total)
{
    bool isOK = false ;
    switch (fs) {
    case FSType::BtrFS:
        isOK = getBtrFSSizeInfo(partitionpath, freeface, total);
        break;
    case FSType::Ext2:
    case FSType::Ext3:
    case FSType::Ext4:
    case FSType::KylinData:
        isOK = getExt2SizeInfo(partitionpath, freeface, total);
        break;
    case FSType::EFI:
    case FSType::Fat16:
    case FSType::Fat32:
        isOK = getFat16SizeInfo(partitionpath, freeface, total);
        break;
    case FSType::Jfs:
        isOK = getJfsSizeInfo(partitionpath, freeface, total);
        break;
    case FSType::LinuxSwap:
        isOK = getSwapSizeInfo(partitionpath, freeface, total);
        break;
    case FSType::Reiser4:
        isOK = getReiser4SizeInfo(partitionpath, freeface, total);
        break;
    case FSType::ReiserFS:
        isOK = getReiserSizeInfo(partitionpath, freeface, total);
        break;
    case FSType::XFS:
        isOK = getXFSSizeInfo(partitionpath, freeface, total);
        break;
    case FSType::NtFS:
        isOK = getNtFSSizeInfo(partitionpath, freeface, total);
        break;
    default:
        break;
    }
    if(!isOK) {
        freeface = -1;
        total = -1;
        qWarning() << "failed to read usage:" << partitionpath;
    }
    return isOK;
}
}
}
