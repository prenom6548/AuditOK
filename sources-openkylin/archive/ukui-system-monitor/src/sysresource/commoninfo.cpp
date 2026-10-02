/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "commoninfo.h"
#include <QDebug>

// Cpu 信息
// 解析单行CPU数据
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QString>
#include <QVector>
#include <QDebug>
#include <QDir>
#include <QRegularExpression>

// 获取挂载点列表（替代 glibtop_get_mountlist）
QList<MountEntry> diskInfo::getMountList(bool showAllFs)
{
    QList<MountEntry> entries;
    QFile file("/proc/mounts");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return entries;
    }

    QTextStream stream(&file);
    QString line;

    while (!(line = stream.readLine()).isEmpty()) {
        // 格式: device mountdir type options dump pass
        QStringList parts = line.split(' ');
        if (parts.size() < 4) continue;

        MountEntry entry;
        entry.device = parts[0];
        entry.mountdir = parts[1].replace("\\040", " ");  // 解码空格
        entry.type = parts[2];
        entry.options = parts[3];

        // 过滤伪文件系统（如不显示全部）
        if (!showAllFs) {
            static const QStringList pseudoFs = {
                "proc", "sysfs", "devtmpfs", "devpts", "tmpfs",
                "cgroup", "cgroup2", "pstore", "bpf", "tracefs",
                "debugfs", "hugetlbfs", "mqueue", "overlay", "squashfs"
            };
            if (pseudoFs.contains(entry.type)) continue;
        }

        entries.append(entry);
    }

    return entries;
}

// 获取文件系统使用统计（替代 glibtop_get_fsusage）
bool diskInfo::getFsUsage(const QString &mountdir, FsUsage &usage)
{
    struct statvfs buf;

    if (statvfs(mountdir.toUtf8().constData(), &buf) != 0) {
        return false;
    }

    usage.block_size = buf.f_frsize;  // 基本块大小（非 f_bsize）

    // 总块数 × 块大小 = 总容量
    usage.total = (unsigned long long)buf.f_blocks * buf.f_frsize;

    // 空闲块数 × 块大小 = 空闲空间
    usage.free = (unsigned long long)buf.f_bfree * buf.f_frsize;

    // 可用块数 × 块大小 = 非root用户可用
    usage.available = (unsigned long long)buf.f_bavail * buf.f_frsize;

    // 已使用 = 总 - 空闲
    usage.used = usage.total - usage.free;

    // 保留 = 空闲 - 可用（root保留）
    usage.reserved = usage.free - usage.available;

    return true;
}

#include <kysdk/kysdk-base/libkydiagnostics.h>

CpuUsageData parseCpuLine(const QString& line)
{
    CpuUsageData data;
    QStringList parts = line.split(' ', QString::SkipEmptyParts);

    if (parts.size() < 2) {
        qWarning() << "无效的CPU行:" << line;
        return data;
    }

    // 设置CPU标识符
    data.cpuId = parts[0];

    // 解析各个时间字段
    if (parts.size() > 1) {
        data.user = parts[1].toULongLong();
    }
    if (parts.size() > 2) {
        data.nice = parts[2].toULongLong();
    }
    if (parts.size() > 3) {
        data.system = parts[3].toULongLong();
    }
    if (parts.size() > 4) {
        data.idle = parts[4].toULongLong();
    }
    if (parts.size() > 5) {
        data.iowait = parts[5].toULongLong();
    }
    if (parts.size() > 6) {
        data.irq = parts[6].toULongLong();
    }
    if (parts.size() > 7) {
        data.softirq = parts[7].toULongLong();
    }
    if (parts.size() > 8) {
        data.steal = parts[8].toULongLong();
    }
    if (parts.size() > 9) {
        data.guest = parts[9].toULongLong();
    }
    if (parts.size() > 10) {
        data.guest_nice = parts[10].toULongLong();
    }

    return data;
}

CpuUsageData cpuInfo::getTotalCpuUsageData()
{
    CpuUsageData data;

    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:/proc/stat";
        return data;
    }

    // 只读取第一行
    QByteArray line = file.readLine();
    QString lineStr = QString::fromUtf8(line).trimmed();

    // 确保是总CPU行（以"cpu "开头，后面有空格）
    if (lineStr.startsWith("cpu ")) {
        data = parseCpuLine(lineStr);
    }

    file.close();
    return data;
}

// 获取所有CPU使用数据
QVector<CpuUsageData> cpuInfo::getAllCpuUsageData()
{
    QVector<CpuUsageData> cpuData;

    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:/proc/stat";
        return cpuData;
    }

    QByteArray content = file.readAll();
    QString contentStr = QString::fromUtf8(content);
    QStringList lines = contentStr.split('\n', QString::SkipEmptyParts);
    for (const QString& line : lines) {
        if (line.startsWith("cpu")) {
            CpuUsageData data = parseCpuLine(line);
            cpuData.append(data);
        }
    }

    file.close();
    return cpuData;
}

QStringList cpuInfo::getAllCpuName()
{
    QStringList cpuList;

    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:/proc/stat";
        return cpuList;
    }

    QByteArray content = file.readAll();
    QString contentStr = QString::fromUtf8(content);
    QStringList lines = contentStr.split('\n', QString::SkipEmptyParts);
    for (const QString& line : lines) {
        if (line.startsWith("cpu")) {
            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            // 确保分割后有多个部分（至少第一列和至少一个数值）
            if (parts.size() > 1) {
                QString name = parts[0]; // 第一列作为key，例如 "cpu", "cpu0", "cpu1"
                cpuList.append(name);
            }
        }
    }

    file.close();
    return cpuList;
}

QVector<double> cpuInfo::calculateCpuUsage(const QVector<CpuUsageData>& prev,
                                             const QVector<CpuUsageData>& current)
{
    QVector<double> usagePercentages;

    if (prev.size() != current.size()) {
        qWarning() << "CPU数据数量不匹配";
        return usagePercentages;
    }

    for (int i = 0; i < current.size(); ++i) {
        quint64 prevActive = prev[i].active();
        quint64 prevTotal = prev[i].total();
        quint64 currActive = current[i].active();
        quint64 currTotal = current[i].total();

        quint64 activeDiff = currActive - prevActive;
        quint64 totalDiff = currTotal - prevTotal;

        if (totalDiff > 0) {
            double usage = static_cast<double>(activeDiff) / totalDiff * 100;
            usagePercentages.append(usage);
        } else {
            usagePercentages.append(0.0);
        }
    }

    return usagePercentages;
}


QString cpuInfo::getUtilization()
{
    return QString::number(kdk_rti_get_cpu_current_usage()*100, 'f', 2) + "%";
}

QString cpuInfo::getCurrentFrequency()
{
    if (kdk_cpu_get_freq_MHz() != nullptr) {
        float a = atol(kdk_cpu_get_freq_MHz());
        QString currentFreq = QString::number(a/1000, 'f', 2) + "GHz";
        return currentFreq;
    }
    return "";
}

// 需要sudo权限
QString cpuInfo::getFrequency()
{
    // 失败时返回-1
    float min = kdk_cpu_get_min_freq_MHz();
    float max = kdk_cpu_get_max_freq_MHz();

    if (min <= 0 || max <= 0) {
        return "";
    }

    QString minFreq = QString::number(min/1000, 'f', 2) + "GHz";
    QString maxFreq = QString::number(max/1000, 'f', 2) + "GHz";
    return minFreq + "~" + maxFreq;
}

QString cpuInfo::getUpTime()
{
    char *ret = kdk_cpu_get_running_time();
    QString runningTime =  QString::fromUtf8(ret);
    free(ret);
    return runningTime;
}

QString cpuInfo::getSockets()
{
    return QString::number(kdk_cpu_get_sockets());
}

QString cpuInfo::getLogicalProcessors()
{
    return QString::number(kdk_cpu_get_process());
}

QString cpuInfo::getVirtualization()
{
    QString virtualization = kdk_cpu_get_virt();
    if (virtualization == nullptr) {
        virtualization = "NULL";
    }
    return virtualization;
}

// 需要sudo权限
QString cpuInfo::getL1iCache()
{
    return commom::format::Format_Memory(kdk_cpu_get_L1i_cache(), KDK_KILOBYTE);
}

QString cpuInfo::getL1dCache()
{
    return commom::format::Format_Memory(kdk_cpu_get_L1d_cache(), KDK_KILOBYTE);
}

QString cpuInfo::getL2Cache()
{
    return commom::format::Format_Memory(kdk_cpu_get_L2_cache(), KDK_KILOBYTE);
}

QString cpuInfo::getL3Cache()
{
    return commom::format::Format_Memory(kdk_cpu_get_L3_cache(), KDK_KILOBYTE);
}

QString cpuInfo::getLoadAverage()
{
    kdk_loadavg loadAvg = kdk_system_get_loadavg();
    QString avg1 = QString::number(loadAvg.loadavg_1m, 'f', 2);
    QString avg5 = QString::number(loadAvg.loadavg_5m, 'f', 2);
    QString avg15 = QString::number(loadAvg.loadavg_15m, 'f', 2);
    return avg1 + " " + avg5 + " " + avg15;
}

QString cpuInfo::getFileDescriptors()
{
    return QString::number(kdk_system_get_file_descriptor());
}

QString cpuInfo::getProcesses()
{
    return QString::number(kdk_system_get_process_nums());
}

QString cpuInfo::getThreads()
{
    return QString::number(kdk_system_get_thread_nums());
}

QString cpuInfo::getHostName()
{
    char *ret = kdk_system_get_hostName();
    QString hostname = QString::fromUtf8(ret);
    free(ret);
    return hostname;
}

QString cpuInfo::getOSType()
{
    char *ret = kdk_system_get_architecture();
    QString architecture = QString::fromUtf8(ret);
    free(ret);
    return architecture;
}

QString cpuInfo::getVersion()
{
    char *ret = kdk_system_get_version(0);
    QString version = QString::fromUtf8(ret);
    free(ret);
    return version;
}

// 内存信息
QString memInfo::getUsed()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_res_usage_KiB(), KDK_KILOBYTE);
}

QString memInfo::getAvailable()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_res_available_KiB(), KDK_KILOBYTE);
}

QString memInfo::getShared()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_shared_KiB(), KDK_KILOBYTE);
}

QString memInfo::getCached()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_cached_KiB(), KDK_KILOBYTE);
}

QString memInfo::getBuffers()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_buffers_KiB(), KDK_KILOBYTE);
}

QString memInfo::getCachedSwap()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_swap_cached_KiB(), KDK_KILOBYTE);
}

QString memInfo::getActive()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_active_KiB(), KDK_KILOBYTE);
}

QString memInfo::getInActive()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_inactive_KiB(), KDK_KILOBYTE);
}

QString memInfo::getDirty()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_dirty_KiB(), KDK_KILOBYTE);
}

QString memInfo::getMapped()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_map_KiB(), KDK_KILOBYTE);
}

QString memInfo::getTotalSwap()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_swap_total_KiB(), KDK_KILOBYTE);
}

QString memInfo::getFreeSwap()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_swap_free_KiB(), KDK_KILOBYTE);
}

QString memInfo::getSlab()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_slab_KiB(), KDK_KILOBYTE);
}

// 网络信息
QString netInfo::getIPv4Address(const char *strKey)
{
    return kdk_nc_get_private_ipv4(strKey);
}

QString netInfo::getIPv4Netmask(const char *strKey, const char *addr)
{
    char *netmask = kdk_nc_get_netmask(strKey, addr);
    QString _netmask = QString::fromUtf8(netmask);
    free(netmask);
    return _netmask;
}

QString netInfo::getIPv4Boardcast(const char *strKey, const char *addr)
{
    char *boardcast = kdk_nc_get_broadAddr(strKey, addr);
    QString _boardcast = QString::fromUtf8(boardcast);
    free(boardcast);
    return _boardcast;
}

QString netInfo::getIPv6Address(const char *strKey)
{
    return kdk_nc_get_private_ipv6(strKey);
}

QString netInfo::getIPv6NetmaskLen(const char *strKey, const char *addr)
{
    return QString::number(kdk_nc_get_mask_length(0, strKey, addr));
}

QString netInfo::getIPv6Scope(const char *strKey, const char *addr)
{
    return QString::number(kdk_nc_get_scope(strKey, addr));
}

QString netInfo::getConnectType(const char *strKey)
{
    return kdk_nc_get_conn_type(strKey);
}

QString netInfo::getNetName(const char *strKey)
{
    char *ret = kdk_nc_get_wifi_name(strKey);
    QString netName = QString::fromUtf8(ret);
    free(ret);
    return netName;
}

QString netInfo::getSignalQuality(const char *strKey)
{
    return QString::number(kdk_nc_get_wifi_signal_qual(strKey));
}

QString netInfo::getSignalStrength(const char *strKey)
{
     return QString::number(kdk_nc_get_wifi_signal_level(strKey));
}

QString netInfo::getNoiseLevel(const char *strKey)
{
    return QString::number(kdk_nc_get_wifi_noise(strKey));
}

QString netInfo::getMacAddress(const char *strKey)
{
    char *ret = kdk_nc_get_phymac(strKey);
    QString mac = QString::fromUtf8(ret);
    free(ret);
    return mac;
}

QString netInfo::getSpeed(const char *strKey)
{
    char *ret = kdk_nc_get_speed(strKey);
    QString speed = QString::fromUtf8(ret);
    free(ret);
    return speed.isEmpty() ? "0 Mb/s" : speed;
}

QString netInfo::getRecvPackets(const char *strKey)
{
    return QString::number(kdk_nc_get_rx_packets(strKey));
}

QString netInfo::getRecvTotal(const char *strKey)
{
    return commom::format::formatNetworkBrandWidth(kdk_nc_get_rx_bytes(strKey), true);
}

QString netInfo::getRecvErrors(const char *strKey)
{
    return QString::number(kdk_nc_get_rx_errors(strKey));
}

QString netInfo::getRecvDropped(const char *strKey)
{
    return QString::number(kdk_nc_get_rx_dropped(strKey));
}

QString netInfo::getRecvFIFO(const char *strKey)
{
    return QString::number(kdk_nc_get_rx_fifo_errors(strKey));
}

QString netInfo::getFrameErrors(const char *strKey)
{
    return QString::number(kdk_nc_get_rx_frame_errors(strKey));
}

QString netInfo::getSendPackets(const char *strKey)
{
    return QString::number(kdk_nc_get_tx_packets(strKey));
}

QString netInfo::getSendTotal(const char *strKey)
{
    return commom::format::formatNetworkBrandWidth(kdk_nc_get_tx_bytes(strKey), true);
}
QString netInfo::getSendErrors(const char *strKey)
{
    return QString::number(kdk_nc_get_tx_errors(strKey));
}

QString netInfo::getSendDropped(const char *strKey)
{
    return QString::number(kdk_nc_get_tx_dropped(strKey));
}

QString netInfo::getSendFIFO(const char *strKey)
{
    return QString::number(kdk_nc_get_tx_fifo_errors(strKey));
}

QString netInfo::getCarrierLoss(const char *strKey)
{
    return QString::number(kdk_nc_get_tx_carrier_errors(strKey));
}

QStringList netInfo::getUpCards()
{
    char **cardlist = kdk_nc_get_upcards();
    QStringList upCardList;
    int index = 0;
    while (cardlist != nullptr && cardlist[index]) {
        if (QString(cardlist[index]) != "lo") {
            upCardList.append(cardlist[index]);
        }
        index++;
    }
    kdk_nc_freeall(cardlist);
    return upCardList;
}

QStringList netInfo::getAllCards()
{
    char **cardlist = kdk_nc_get_cardlist();
    QStringList allCardList;
    int index = 0;
    while (cardlist != nullptr && cardlist[index]) {
        if (QString(cardlist[index]) != "lo") {
            allCardList.append(cardlist[index]);
        }
        index++;
    }
    kdk_nc_freeall(cardlist);
    return allCardList;
}

QList<INet4Addr> netInfo::getIPv4AddressList(const char *strKey)
{
    char **list = kdk_nc_get_ipv4(strKey);
    QList<INet4Addr> IPv4List;
    int index = 0;
    while (list != nullptr && list[index] != nullptr) {
        INet4Addr netAddr;
        netAddr.addr = list[index];
        netAddr.mask = getIPv4Netmask(strKey, list[index]);
        netAddr.bcast = getIPv4Boardcast(strKey, list[index]);
        IPv4List.append(netAddr);
        index++;
    }
    kdk_nc_freeall(list);
    return IPv4List;
}

QList<INet6Addr> netInfo::getIPv6AddressList(const char *strKey)
{
    char **list = kdk_nc_get_ipv6(strKey);
    QList<INet6Addr> IPv6List;
    int index = 0;
    while (list != nullptr && list[index] != nullptr) {
        INet6Addr netAddr;
        netAddr.addr = list[index];
        netAddr.prefixlen = getIPv6NetmaskLen(strKey, list[index]);
        netAddr.scope = getIPv6Scope(strKey, list[index]);
        IPv6List.append(netAddr);
        index++;
    }
    kdk_nc_freeall(list);
    return IPv6List;
}

namespace commom {

bool buriedSettings(QString pluginName, QString settingsName, QString action, QString value)
{
    // 埋点数据
    char appName[] = "ukui-system-monitor";
    QByteArray actiontr = action.toLocal8Bit();  // toLocal8Bit 支持中文
    char *messageType = actiontr.data();

    KBuriedPoint pt[3];
    pt[0].key = "pluginName";
    std::string pluginStr = pluginName.toStdString();
    pt[0].value = pluginStr.c_str();

    pt[1].key = "settingsName";
    std::string settingStr = settingsName.toStdString();
    pt[1].value = settingStr.c_str();

    pt[2].key = "value";
    std::string valueStr = value.toStdString();
    pt[2].value = valueStr.c_str();

    if (kdk_buried_point(appName , messageType , pt , 3) == -1) {
        qDebug() << __FUNCTION__ << "messageType:" << action << "pluginName:" << pluginName
                 << "settingsName:" << settingsName << "value:" << value << "buried point fail !" << __LINE__;
        return false;
    }

    return true;
}

const QString format::Format_Memory(long data, KDKVolumeBaseType baseType)
{
    char resultData[50] = {0};

    const char* b = (std::to_string(data)+ UnitSuffixMem[baseType]).c_str();
    kdkVolumeBaseCharacterConvert(b, KDK_GIGABYTE, resultData);

    return data == 0 ? "0KB" : QString::fromUtf8(resultData);
}

const QString format::formatNetworkBrandWidth(guint64 size, bool isTotal)
{
    enum {
        K_INDEX,
        M_INDEX,
        G_INDEX,
        T_INDEX
    };
    char data[128] = {0};
    snprintf(data, sizeof(data), "%lu", size);
    char result_data[128] = {0};
    double m_speed = 0;
    KDKVolumeBaseType m_KDKVolumeBaseType;

    QList<guint64> factorList;
    factorList.append(G_GUINT64_CONSTANT(1) << 10);//KiB
    factorList.append(G_GUINT64_CONSTANT(1) << 20);//MiB
    factorList.append(G_GUINT64_CONSTANT(1) << 30);//GiB
    factorList.append(G_GUINT64_CONSTANT(1) << 40);//TiB

    guint64 factor;
    QString format;
    if (size < factorList.at(M_INDEX)) {
        m_KDKVolumeBaseType = KDK_KILOBYTE;
        factor = factorList.at(K_INDEX);
        if (isTotal) {
            format = "KB";
        } else {
            format = "KB/s";
        }
    } else if (size < factorList.at(G_INDEX)) {
        m_KDKVolumeBaseType = KDK_MEGABYTE;
        factor = factorList.at(M_INDEX);
        if (isTotal) {
            format = "MB";
        } else {
            format = "MB/s";
        }
    } else if (size < factorList.at(T_INDEX)) {
        m_KDKVolumeBaseType = KDK_GIGABYTE;
        factor = factorList.at(G_INDEX);
        if (isTotal) {
            format = "GB";
        } else {
            format = "GB/s";
        }
    } else {
        m_KDKVolumeBaseType = KDK_TERABYTE;
        factor = factorList.at(T_INDEX);
        if (isTotal) {
            format = "TB";
        } else {
            format = "TB/s";
        }
    }

    if (size > 1024 && kdkVolumeBaseCharacterConvert(data, m_KDKVolumeBaseType, result_data) == KDK_NOERR) {
        QString formatted_result = "";
        sscanf(result_data, "%lf", &m_speed);
        formatted_result.setNum(m_speed, 'f', 1);
        return formatted_result + format;
    } else {
        std::string formatted_result(make_string(g_strdup_printf("%.1f", size / (double)factor)));
        return QString::fromStdString(formatted_result) + format;
    }
}

const qreal format::chkNewMaxData(qreal qNewMaxData)
{
    qreal newMaxData = 0;
    if (qNewMaxData < 20 * 1024) {
        newMaxData = 20 * 1024;
    } else if (qNewMaxData >= 20 *1024 && qNewMaxData < 100 * 1024) {
        newMaxData = 100 * 1024;
    } else if (qNewMaxData >= 100 *1024 && qNewMaxData < 500 * 1024) {
        newMaxData = 500 * 1024;
    } else if (qNewMaxData >= 500 *1024 && qNewMaxData < 1 * 1024 * 1024) {
        newMaxData = 1 * 1024 * 1024;
    } else if (qNewMaxData >= 1 * 1024 * 1024 && qNewMaxData < 2 * 1024 * 1024) {
        newMaxData = 2 * 1024 * 1024;
    } else if (qNewMaxData >= 2 * 1024 * 1024 && qNewMaxData < 4 * 1024 * 1024) {
        newMaxData = 4 * 1024 * 1024;
    } else if (qNewMaxData >= 4 * 1024 * 1024 && qNewMaxData < 8 * 1024 * 1024) {
        newMaxData = 8 * 1024 * 1024;
    } else if (qNewMaxData >= 8 * 1024 * 1024 && qNewMaxData < 16 * 1024 * 1024) {
        newMaxData = 16 * 1024 * 1024;
    } else if (qNewMaxData >= 16 * 1024 * 1024 && qNewMaxData < 32 * 1024 * 1024) {
        newMaxData = 32 * 1024 * 1024;
    } else if (qNewMaxData >= 32 * 1024 * 1024 && qNewMaxData < 64 * 1024 * 1024) {
        newMaxData = 64 * 1024 * 1024;
    } else if (qNewMaxData >= 64 * 1024 * 1024 && qNewMaxData < 128 * 1024 * 1024) {
        newMaxData = 128 * 1024 * 1024;
    } else if (qNewMaxData >= 128 * 1024 * 1024 && qNewMaxData < 256 * 1024 * 1024) {
        newMaxData = 256 * 1024 * 1024;
    } else if (qNewMaxData >= 256 * 1024 * 1024 && qNewMaxData < 512 * 1024 * 1024) {
        newMaxData = 512 * 1024 * 1024;
    } else if (qNewMaxData >= 512 * 1024 * 1024 && qNewMaxData < 1024 * 1024 * 1024) {
        newMaxData = (quint64)1024 * 1024 * 1024;
    } else {
        newMaxData = (quint64)2 * 1024 * 1024 * 1024;
    }
    return newMaxData;
}

}


QString diskInfo::getDiskModel(const QString &strKey)
{
    std::string str = strKey.toStdString();
    const char *diskName = str.c_str();

    char *ret = kdk_get_hard_model(diskName);
    QString model = QString::fromUtf8(ret);
    free(ret);
    return model;
}

QString diskInfo::getDiskCapacity(const QString &strKey)
{
    std::string str = strKey.toStdString();
    const char *diskName = str.c_str();

    kdk_diskinfo *diskinfo = kdk_get_diskinfo(diskName);
    float size = diskinfo->total_size_MiB;
    kdk_free_diskinfo(diskinfo);

    return commom::format::Format_Memory(size, KDK_MEGABYTE);
}

QString diskInfo::getDiskType(const QString &strKey)
{
    std::string str = strKey.toStdString();
    const char *diskName = str.c_str();

    char *ret = kdk_get_hard_type(diskName);
    QString type = QString::fromUtf8(ret);
    free(ret);

    return type;
}

QStringList diskInfo::getDiskName()
{
    char **_disklist = kdk_get_hard_disk();
    QStringList diskList;
    int index = 0;
    while (_disklist != nullptr && _disklist[index]) {
        diskList.append(_disklist[index]);
        index++;
    }
    kdk_free_disklist(_disklist);

    return diskList;
}

DiskSpeedInfo *DiskSpeedInfo::m_pDiskSpeedInfo = nullptr;
DiskSpeedInfo *DiskSpeedInfo::instance()
{
    if (m_pDiskSpeedInfo == nullptr) {
        m_pDiskSpeedInfo = new DiskSpeedInfo;
    }
    return m_pDiskSpeedInfo;
}

DiskSpeedInfo::DiskSpeedInfo(QObject *parent) : QObject(parent)
{

}

DiskSpeedInfo::~DiskSpeedInfo()
{

}

double DiskSpeedInfo::readSpeed(const QString &strKey)
{
    return m_speedInfo[strKey].readSpeed;
}

double DiskSpeedInfo::writeSpeed(const QString &strKey)
{
    return m_speedInfo[strKey].writeSpeed;
}

QString DiskSpeedInfo::readSpeedStr(const QString &strKey)
{
    return m_speedInfo[strKey].readSpeedStr;
}

QString DiskSpeedInfo::writeSpeedStr(const QString &strKey)
{
    return m_speedInfo[strKey].writeSpeedStr;
}

void DiskSpeedInfo::updateDiskSpeed(const QString &strKey)
{
    std::string str = strKey.toStdString();
    const char *diskName = str.c_str();

    DiskSpeed tmpSpeed;

    tmpSpeed.readSpeed = kdk_get_disk_read_speed(diskName);
    tmpSpeed.readSpeedStr = commom::format::Format_Memory(tmpSpeed.readSpeed, KDK_KILOBYTE) + "/s";

    tmpSpeed.writeSpeed = kdk_get_disk_write_speed(diskName);
    tmpSpeed.writeSpeedStr = commom::format::Format_Memory(tmpSpeed.writeSpeed, KDK_KILOBYTE) + "/s";

    if (m_speedInfo.keys().contains(strKey)) {
        m_speedInfo[strKey] = tmpSpeed;
    } else {
        m_speedInfo.insert(strKey, tmpSpeed);
    }
}
