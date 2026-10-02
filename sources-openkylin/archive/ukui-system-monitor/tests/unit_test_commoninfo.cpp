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
 */
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../src/sysresource/commoninfo.h"

class CommoninfoTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
    }

    static void TearDownTestSuite()
    {
    }
};

TEST_F(CommoninfoTest, cpuInfoTest)
{
    cpuInfo::getTotalCpuUsageData();
    QVector<CpuUsageData> cpu1 = cpuInfo::getAllCpuUsageData(); // 获取 cpu 数据
    cpuInfo::getAllCpuName(); // 获取cpu核数列表
    QVector<CpuUsageData> cpu2 = cpuInfo::getAllCpuUsageData();
    cpuInfo::calculateCpuUsage(cpu1, cpu2);// 计算每个核的CPU占用率

    cpuInfo::getUtilization();       // 获取总利用率
    cpuInfo::getCurrentFrequency();  // 获取当前频率
    cpuInfo::getFrequency();         // 获取频率
    cpuInfo::getUpTime();            // 获取运行时间
    cpuInfo::getSockets();           // 获取插槽
    cpuInfo::getLogicalProcessors(); // 获取逻辑处理器
    cpuInfo::getVirtualization();    // 获取虚拟化
    cpuInfo::getL1iCache();          // 获取 L1 缓存（指令）
    cpuInfo::getL1dCache();          // 获取 L1 缓存（数据）
    cpuInfo::getL2Cache();           // 获取 L2 缓存
    cpuInfo::getL3Cache();           // 获取 L3 缓存
    cpuInfo::getLoadAverage();       // 获取负载均衡
    cpuInfo::getFileDescriptors();   // 获取文件描述符数
    cpuInfo::getProcesses();         // 获取进程数
    cpuInfo::getThreads();           // 获取线程数
    cpuInfo::getHostName();          // 获取计算机名
    cpuInfo::getOSType();            // 获取类型
    cpuInfo::getVersion();           // 获取版本名称
}

TEST_F(CommoninfoTest, memInfoTest)
{
    memInfo::getUsed();              // 获取已使用
    memInfo::getAvailable();         // 获取可使用
    memInfo::getShared();            // 获取共享内存
    memInfo::getCached();            // 获取高速缓存
    memInfo::getBuffers();           // 获取数据缓存
    memInfo::getCachedSwap();        // 获取交换缓存区
    memInfo::getActive();            // 获取活跃的缓冲文件
    memInfo::getInActive();          // 获取不活跃的缓冲文件
    memInfo::getDirty();             // 获取脏页
    memInfo::getMapped();            // 获取映射大小
    memInfo::getTotalSwap();         // 获取交换空间大小
    memInfo::getFreeSwap();          // 获取可用交换空间
    memInfo::getSlab();              // 获取内核数据结构缓存
}

TEST_F(CommoninfoTest, netInfoTest)
{
    QStringList cardNameList = netInfo::getAllCards(); // 获取所有网卡列表

    QStringList upCardNameList = netInfo::getUpCards(); // 获取up状态网卡列表

    QString cardName = "enp5s0";

    if (!cardNameList.isEmpty()) {
        cardName = cardNameList.at(0);
    }

    std::string str = cardName.toStdString();
    const char *NCName = str.c_str();

    netInfo::getIPv4Address(NCName);     // 获取 IPv4 地址
    netInfo::getIPv6Address(NCName);     // 获取 IPv6 地址
    netInfo::getConnectType(NCName);     // 获取连接类型
    netInfo::getNetName(NCName);         // 获取网络名称
    netInfo::getSignalQuality(NCName);   // 获取信号质量
    netInfo::getSignalStrength(NCName);  // 获取信号强度
    netInfo::getNoiseLevel(NCName);      // 获取底噪
    netInfo::getMacAddress(NCName);      // 获取 MAC 地址
    netInfo::getSpeed(NCName);           // 获取速率
    netInfo::getRecvPackets(NCName);     // 获取接收包数量
    netInfo::getRecvTotal(NCName);       // 获取总计接收
    netInfo::getRecvErrors(NCName);      // 获取接收错误包
    netInfo::getRecvDropped(NCName);     // 获取接收丢弃包
    netInfo::getRecvFIFO(NCName);        // 获取接收 FIFO
    netInfo::getFrameErrors(NCName);     // 获取分组帧错误
    netInfo::getSendPackets(NCName);     // 获取发送包数量
    netInfo::getSendTotal(NCName);       // 获取总计发送
    netInfo::getSendErrors(NCName);      // 获取发送错误包
    netInfo::getSendDropped(NCName);     // 获取发送丢弃宝
    netInfo::getSendFIFO(NCName);        // 获取发送FIFO
    netInfo::getCarrierLoss(NCName);     // 获取载波损耗

    netInfo::getIPv4AddressList(NCName); // 获取ipv4地址列表
    netInfo::getIPv6AddressList(NCName); // 获取ipv6地址列表
}

TEST_F(CommoninfoTest, diskInfoTest)
{
    QStringList diskList = diskInfo::getDiskName(); // 获取磁盘列表

    QString diskName = "";

    if (!diskList.isEmpty()) {
        diskName = diskList.at(0);
    }

    diskInfo::getDiskModel(diskName); // 获取磁盘型号
    diskInfo::getDiskCapacity(diskName); // 获取磁盘容量
    diskInfo::getDiskType(diskName); // 获取磁盘类型


    QList<MountEntry> entries = diskInfo::getMountList();
    for (int i = 0; i < entries.count(); i++) {
        // 如果是托盘，并且挂载点非根目录，则暂不获取数据，等切换界面后获取数据
        if (QString(entries[i].mountdir) != "/") {
            continue;
        }
        MountEntry entry = entries[i];
        FsUsage usage;
        diskInfo::getFsUsage(entry.mountdir, usage);
    }


    foreach (QString diskName, diskList) {
        DiskSpeedInfo::instance()->updateDiskSpeed(diskName);
        double readSpeed = DiskSpeedInfo::instance()->readSpeed(diskName);
        double writeSpeed = DiskSpeedInfo::instance()->writeSpeed(diskName);
        DiskSpeedInfo::instance()->readSpeedStr(diskName);
        DiskSpeedInfo::instance()->writeSpeedStr(diskName);
    }

}

TEST_F(CommoninfoTest, formatTest)
{
    commom::format::Format_Memory(65535, KDK_KILOBYTE);

    commom::format::chkNewMaxData(20 * 1024 -1);
    commom::format::chkNewMaxData(20 * 1024);
    commom::format::chkNewMaxData(100 *1024);
    commom::format::chkNewMaxData(500 *1024);
    commom::format::chkNewMaxData(1 * 1024 * 1024);
    commom::format::chkNewMaxData(2 * 1024 * 1024);
    commom::format::chkNewMaxData(4 * 1024 * 1024);
    commom::format::chkNewMaxData(8 * 1024 * 1024);
    commom::format::chkNewMaxData(16 * 1024 * 1024);
    commom::format::chkNewMaxData(32 * 1024 * 1024);
    commom::format::chkNewMaxData(64 * 1024 * 1024);
    commom::format::chkNewMaxData(128 * 1024 * 1024);
    commom::format::chkNewMaxData(256 * 1024 * 1024);
    commom::format::chkNewMaxData(512 * 1024 * 1024);
    commom::format::chkNewMaxData(1024 * 1024 * 1024);

    commom::format::formatNetworkBrandWidth(1024 - 1, true);
    commom::format::formatNetworkBrandWidth(1024 * 1024 - 1, true);
    commom::format::formatNetworkBrandWidth(1024 * 1024 * 1024 -1, true);
    commom::format::formatNetworkBrandWidth(1024 * 1024 * 1024 * 1024 -1, true);

    commom::format::formatNetworkBrandWidth(1024 - 1, false);
    commom::format::formatNetworkBrandWidth(1024 * 1024 - 1, false);
    commom::format::formatNetworkBrandWidth(1024 * 1024 * 1024 - 1, false);
    commom::format::formatNetworkBrandWidth(1024 * 1024 * 1024 * 1024 - 1, false);
}
