/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include "netwolrpm.h"

NetWolRpmDevice::NetWolRpmDevice(QObject *parent) : QObject(parent)
{
    m_powerConfigMap = UpmCustomHardwareConfig::self()->getConfig(UPM_GROUP_NETWOl);
    printConfigMap();
    m_controlFile = NET_WOL_FILE;
    getDevicePowerInfo();
}

void NetWolRpmDevice::getDevicePowerInfo()
{
    // 遍历 /sys/class/net 下的物理网卡，将其底层 device 的 power/wakeup 纳入统一管理
        QDir netDir(NET_WOL_PATH);
        if (!netDir.exists()) return;

        QStringList ifaces = netDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &iface : ifaces) {
            if (isVirtualIfaceName(iface)) continue;

            QString devLink = QString("%1/%2/device").arg(NET_WOL_PATH).arg(iface);
            QFileInfo devLinkInfo(devLink);
            if (!devLinkInfo.exists()) continue; // 虚拟接口没有 device

            QString devReal = QFileInfo(devLink + "/").canonicalFilePath();
            if (devReal.isEmpty()) devReal = devLinkInfo.absoluteFilePath();

            QString wakeFile = devReal + NET_WOL_FILE;
            QFileInfo wf(wakeFile);
            if (!wf.exists()) {
                qDebug() << "wakeupdevice: net iface" << iface << "no power/wakeup support";
                continue;
            }

            // 设备名：优先 product，否则取设备目录名
            QString devName = extractDeviceNameFromPath(devReal);

            //0x20:g ;   0x00:d
            //if(NetWolInfo::getWOLStatus(iface) == 0x20) 
            {
                m_netWolInfoVec.emplace_back(std::unique_ptr<NetWolInfo>(new NetWolInfo(iface, "netwol")));
                qDebug() << "add NetWolInfo net name: " << iface;
            }
        }
}

bool NetWolRpmDevice::isVirtualIfaceName(const QString &iface)
{
    // 常见虚拟接口前缀过滤
    static const QStringList prefixes = {"lo", "veth", "br", "virbr", "docker", "tun", "tap"};
    for (const QString &p : prefixes) {
        if (iface == p || iface.startsWith(p)) return true;
    }
    return false;
}

bool NetWolRpmDevice::setPowerStat(int deviceIndex, const QString &stat)
{

    if (stat == "enabled" || stat == "disabled") {
        if (deviceIndex < m_devicePowerInfo.count()) {
            if(m_ethToolAvailable) {
                return m_netWolInfoVec[deviceIndex]->setNetWolStat(stat);
            } else {
                return m_devicePowerInfo[deviceIndex]->setDevicePowerStat(stat);
            }
        }
    }
    return false;
}

bool NetWolRpmDevice::setAllDevicePowerStat(const QString &stat)
{
    qDebug() << "NetWolRpmDevice::setAllDevicePowerStat: " << stat;
    if (!(stat == "enabled" || stat == "disabled" || stat == "default")) {
        return false;
    }

    for (size_t i = 0; i < m_netWolInfoVec.size(); ++i) {
        m_netWolInfoVec[i]->setNetWolStat(stat);
    }

    return true;
}