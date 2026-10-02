/*
 * Copyright 2023 KylinSoft Co., Ltd.
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

#include <QFileInfo>
#include <QString>
#include <QTextStream>
#include <QDebug>

#include "devicepowerinfo.h"

DevicePowerInfo::DevicePowerInfo(const QString &devType,
                                 const QString &devPath,
                                 const QString &devName,
                                 const QString &controlFile,
                                 QObject *parent) :
    QObject(parent), m_devType(devType), m_devPath(devPath), m_devName(devName), m_controlFile(controlFile)
{
    m_powerControlFilePath = devPath + controlFile;

    QFile file(m_powerControlFilePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream textStream(&file);
        m_defaultPowerValue = textStream.readLine();
        m_currentPowerValue = m_defaultPowerValue;
        qDebug() << "get" << m_powerControlFilePath << " default value:" << m_defaultPowerValue;
        file.close();
    }
}

QString DevicePowerInfo::getDevType()
{
    return m_devType;
}

QString DevicePowerInfo::getDevicePath()
{
    return m_devPath;
}

QString DevicePowerInfo::getDeviceName()
{
    return m_devName;
}

QString DevicePowerInfo::getCurrentPowerStat()
{
    return m_currentPowerValue;
}

QString DevicePowerInfo::getDefaultPowerStat()
{
    return m_defaultPowerValue;
}

bool DevicePowerInfo::setDevicePowerStat(const QString &stat)
{
    bool ret = false;

    if(getCurrentPowerStat() == stat) {
        //qDebug() << "getCurrentPowerStat() == stat" << stat << "not set";
        return false;
    }

    if (stat == "default" && getDevType() != "pcie_aspm") {
        qInfo() << getDeviceName() << m_powerControlFilePath << " write " << m_defaultPowerValue << " type:" << getDevType();
        ret = UpmCommonFile::write(m_devPath, m_controlFile, m_defaultPowerValue);
        m_currentPowerValue = m_defaultPowerValue;
    } else if(stat == "off" && getDevType() == "keyboard_backlight"){
        ret = UpmCommonFile::write(m_devPath, m_controlFile, "0");
        m_currentPowerValue = stat;
    } else if((stat == "bind" || stat == "unbind") && getDevType() == "touch_screen"){
        ret = UpmCommonFile::write(m_devPath, stat, m_devName);
        m_currentPowerValue = stat;
    } else {
        qInfo() << getDeviceName() << m_powerControlFilePath << " write " << stat << " type:" << getDevType();
        ret = UpmCommonFile::write(m_devPath, m_controlFile, stat);
        m_currentPowerValue = stat;
    }

    return ret;
}

