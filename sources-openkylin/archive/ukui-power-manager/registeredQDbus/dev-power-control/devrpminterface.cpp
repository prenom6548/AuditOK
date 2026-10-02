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

#include "devrpminterface.h"

DevRpmInterface::DevRpmInterface() {}

DevRpmInterface::~DevRpmInterface()
{
    qDeleteAll(m_devicePowerInfo);  // 删除所有指针所指向的对象
    m_devicePowerInfo.clear();
}

void DevRpmInterface::getDevicePowerInfo(){

}

void DevRpmInterface::printConfigMap()
{
    // QMap<QString, QString>::iterator it;
    // qDebug() << "configMap:";
    // for (it = m_powerConfigMap.begin(); it != m_powerConfigMap.end(); ++it) {
    //     qDebug() << it.key() << ":" << it.value();
    // }
    // qDebug() << "devFilterRules:";
    // for (it = m_devFilterRulesMap.begin(); it != m_devFilterRulesMap.end(); ++it) {
    //     qDebug() << it.key() << ":" << it.value();
    // }
    // qDebug() << "devIgnoreList:";
    // for (it = m_devIgnoreListMap.begin(); it != m_devIgnoreListMap.end(); ++it) {
    //     qDebug() << it.key() << ":" << it.value();
    // }
}

int DevRpmInterface::getDeviceNum()
{
    return m_devicePowerInfo.count();
}

QString DevRpmInterface::getDeviceName(int deviceIndex)
{
    if (deviceIndex < m_devicePowerInfo.count()) {
        return m_devicePowerInfo[deviceIndex]->getDeviceName();
    }
    return "False";
}

QString DevRpmInterface::getCurrentPowerStat(int deviceIndex)
{
    if (deviceIndex < m_devicePowerInfo.count()) {
        return m_devicePowerInfo[deviceIndex]->getCurrentPowerStat();
    }
    return "False";
}

QString DevRpmInterface::getDefaultPowerStat(int deviceIndex)
{
    if (deviceIndex < m_devicePowerInfo.count()) {
        return m_devicePowerInfo[deviceIndex]->getDefaultPowerStat();
    }
    return "False";
}
void DevRpmInterface::printAllDeviceInfo()
{
    //qDebug() << "pcie_aspm device";
    for (int i = 0; i < m_devicePowerInfo.count(); ++i) {
        qDebug() << "type:" << m_devicePowerInfo[i]->getDevType()
                 << "name:" << m_devicePowerInfo[i]->getDeviceName()
                 << "path:" << m_devicePowerInfo[i]->getDevicePath()
                 << "stat:" << m_devicePowerInfo[i]->getDefaultPowerStat();
    }
}

QMap<QString, QString> DevRpmInterface::getPowerConfig()
{
    return m_powerConfigMap;
}

QString DevRpmInterface::extractDeviceNameFromPath(const QString &devPath)
{
    QFile f(devPath + "/product");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString name = f.readLine();
        f.close();
        return name.remove('\n');
    }
    // 兜底用目录名
    return QFileInfo(devPath).fileName();
}
