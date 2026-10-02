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

#ifndef NETWOLRPM_H
#define NETWOLRPM_H

#include <QProcess>
#include <vector>
#include <memory>
#include "devrpminterface.h"
#include "netwolinfo.h"
#define NET_WOL_PATH       "/sys/class/net/"
#define NET_WOL_FILE       "/power/wakeup"

class NetWolRpmDevice : public QObject, public DevRpmInterface
{
    Q_OBJECT
public:
    explicit NetWolRpmDevice(QObject *parent = nullptr);


private:
    void getDevicePowerInfo();
    void scanNetDevices();
    bool isVirtualIfaceName(const QString &iface);
    bool m_ethToolAvailable;
    std::vector<std::unique_ptr<NetWolInfo>> m_netWolInfoVec;
    QMap<QString, bool> originalWOLStates; // 保存原始WOL状态


      // 通过ethtool设置WOL状态
    bool setWOLByEthtool(const QString &interface, bool enable);

      // 通过sysfs设置WOL状态（备用方法）
    bool setWOLBySysfs(const QString &interface, bool enable);


public:
    bool setPowerStat(int deviceIndex, const QString &stat);
    bool setAllDevicePowerStat(const QString &stat);

       // 关闭指定网卡的WOL
    bool disableWOL(const QString &interface);

       // 开启指定网卡的WOL
    bool enableWOL(const QString &interface);

       // 关闭所有网卡的WOL并保存状态
    bool disableAllWOL();

       // 恢复所有网卡的WOL到初始状态
    bool restoreAllWOL();
};

#endif // NETWOLDEVICE_H