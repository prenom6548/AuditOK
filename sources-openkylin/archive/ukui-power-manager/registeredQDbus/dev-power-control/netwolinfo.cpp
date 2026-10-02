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

#include "netwolinfo.h"

NetWolInfo::NetWolInfo(QString name, QString type) : m_netWolName(name), m_devType(type)
{
    m_currentStat = "enabled";
    qDebug() << "NetWolInfo::NetWolInfo: name: " << name << ", currentStat: " << m_currentStat;
    m_defaultStat = getWOLStatus(name);
}

bool NetWolInfo::setNetWolStat(const QString &stat)
{
    qDebug() << QString("NetWolInfo::setNetWolStat stat : %1").arg(stat);
    if(m_currentStat == stat) {
        qDebug() << QString("NetWolInfo::setNetWolStat m_currentStat is %1").arg(stat);
        return false;
    }


    unsigned int  param;// = (stat == "enabled") ? 0x20 : 0x00;
    if(stat == "default")
    {
        param = m_defaultStat;
    }
    else 
    {
        param = (stat == "enabled") ? 0x20 : 0x00;
    }


    //QString output = executeCommand(command);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        //perror("socket 失败");
        qDebug() << "NetWolInfo::setNetWolStat: open socket failed";
        return false;
    }

    // 构造 ethtool 唤醒配置结构体
    struct ethtool_wolinfo wol;
    memset(&wol, 0, sizeof(wol));


    wol.cmd = ETHTOOL_SWOL;  // 命令：设置唤醒配置
    wol.wolopts = param;         // 唤醒掩码设为 0（禁用所有唤醒）; 0x20 开启唤醒源

    // 绑定到指定网卡（显式转换为 char* 匹配 __caddr_t 类型）
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, m_netWolName.toStdString().c_str(), IFNAMSIZ - 1);
    ifr.ifr_data = (char *)&wol;  // 关键修正：转换为 char*

    // 调用 ioctl 执行设置
    if (ioctl(fd, SIOCETHTOOL, &ifr) < 0) {
//        perror("ioctl 失败");
        qDebug() << "NetWolInfo::setNetWolStat::get wakeup mode failed";
        close(fd);
        return false;
    }

    close(fd);

    m_currentStat = stat;
    return true;
}

unsigned int NetWolInfo::getWOLStatus(const QString &netName)
{

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        //perror("socket 失败");
        qDebug() << "NetWolInfo::getWOLStatus: open socket failed";
        return 1;
    }

    struct ethtool_wolinfo wol;
    memset(&wol, 0, sizeof(wol));
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, netName.toStdString().c_str(), IFNAMSIZ - 1);
    ifr.ifr_data = (char *)&wol;

    // 2. 验证设置结果
    wol.cmd = ETHTOOL_GWOL;      // 获取当前唤醒配置命令
    if (ioctl(fd, SIOCETHTOOL, &ifr) < 0) {
        //perror("获取唤醒模式失败");
        qDebug() << "NetWolInfo::getWOLStatus::get wakeup mode failed";
        close(fd);
        return 1;
    }
    qDebug() << "NetWolInfo::getWOLStatus: " << wol.wolopts;
    return wol.wolopts;

}
