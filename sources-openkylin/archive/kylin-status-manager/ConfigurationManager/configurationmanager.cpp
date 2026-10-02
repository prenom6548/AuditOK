/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "configurationmanager.h"
#include <QString>
#include <QSettings>
#include <QFile>
#include <QDir>

#include <QDBusInterface>
#include <QDBusReply>


/*
 * tips:
 * 新增属性必须考虑默认值，考虑升级时配置文件不存在的情况下要给出默认值。
*/
static const QString s_isAutoRotateEnable = "Rotation/autoRotateEnbale";
static const QString s_deviceOrientation = "Rotation/deviceOrientation";
static const QString s_isAutoSwitchEnable = "DeviceMode/autoSwitchEnable";
static const QString s_isTabletMode = "DeviceMode/tabletMode";
static const QString s_isClassicalMode = "DeviceMode/classicalMode";
static const QString s_isImmersiveMode = "DeviceMode/immersiveMode";
static const QString s_isTabletModeEnabled = "DeviceMode/tabletModeEnabled";
static const QString s_isFirstSwitchDeviceMode = "StartupOptions/firstSwitchDeviceMode";
static const QString s_configFilePath = QDir::homePath() + "/.config/statusManager.ini";

ConfigurationManager::ConfigurationManager()
{
    QFile file(s_configFilePath);
    if (!file.exists()) {
        createConfigFile();
    }

    m_qsetting = new QSettings(s_configFilePath, QSettings::IniFormat, this);
}

void ConfigurationManager::createConfigFile()
{
    QSettings *configFile = new QSettings(s_configFilePath, QSettings::IniFormat);
    configFile->setValue(s_isAutoRotateEnable,false);
    configFile->setValue(s_deviceOrientation,"normal");
    configFile->setValue(s_isAutoSwitchEnable,true);
    configFile->setValue(s_isTabletMode,false);
    configFile->setValue(s_isFirstSwitchDeviceMode,true);
    configFile->setValue(s_isTabletModeEnabled,false);

    configFile->setValue(s_isClassicalMode,true);
    configFile->setValue(s_isImmersiveMode,false);
    delete configFile;
}

ConfigurationManager *ConfigurationManager::GetInstance()
{
    static ConfigurationManager s_configurationManager;
    return &s_configurationManager;
}

QString ConfigurationManager::getDeviceOrientation() const
{
    return m_qsetting->value(s_deviceOrientation).toString();
}

void ConfigurationManager::setDeviceOrientation(QString &orientation)
{
    m_qsetting->setValue(s_deviceOrientation, orientation);
}

bool ConfigurationManager::isTabletMode() const
{
    return m_qsetting->value(s_isTabletMode).toBool();
}

void ConfigurationManager::setTabletMode(bool tabletMode)
{
    m_qsetting->setValue(s_isTabletMode, tabletMode);
}

bool ConfigurationManager::isTabletModeEnabled() const
{
    return m_qsetting->value(s_isTabletModeEnabled, false).toBool();
}

void ConfigurationManager::setTabletModeEnabled(bool enabled)
{
    m_qsetting->setValue(s_isTabletModeEnabled, enabled);
}

bool ConfigurationManager::isAutoSwitchEnabled() const
{
    return m_qsetting->value(s_isAutoSwitchEnable).toBool();
}

void ConfigurationManager::setAutoSwitchEnabled(bool autoSwitchEnable)
{
    m_qsetting->setValue(s_isAutoSwitchEnable, autoSwitchEnable);
}

bool ConfigurationManager::isAutoRotateEnabled() const
{
    return m_qsetting->value(s_isAutoRotateEnable).toBool();
}

void ConfigurationManager::setAutoRotateEnabled(bool autoRotateEnable)
{
    m_qsetting->setValue(s_isAutoRotateEnable, autoRotateEnable);
}

bool ConfigurationManager::isFirstSwitchDeviceMode() const
{
    return m_qsetting->value(s_isFirstSwitchDeviceMode).toBool();
}

void ConfigurationManager::setFirstSwitchDeviceModeToOff()
{
    m_qsetting->setValue(s_isFirstSwitchDeviceMode, false);
}

QMap<QString, QVariant> ConfigurationManager::liteMode() const
{
    QMap<QString, QVariant> ret;

//    QDBusInterface globalManager("org.ukui.SettingsDaemon", \
//                          "/GlobalSignal", \
//                          "org.ukui.SettingsDaemon.GlobalSignal");

//    QDBusReply<QString> rpl  = globalManager.call("getUKUILiteAnimation");
//    ret.insert("animation", "normal");
//    ret.insert("function", "normal");
//    if (rpl.isValid()) {
//        ret.insert("function", rpl.value());
//    }

//    rpl  = globalManager.call("getUKUILiteFunction");
//    if (rpl.isValid()) {
//        ret.insert("animation", rpl.value());
//    }

    return ret;
}

bool ConfigurationManager::isClassicalMode() const
{
  return m_qsetting->value(s_isClassicalMode, true).toBool();
}

void ConfigurationManager::setClassicalMode(bool classicalMode)
{
    qDebug()<<__func__<<classicalMode;
    m_qsetting->setValue(s_isClassicalMode, classicalMode);
}

bool ConfigurationManager::isImmersiveMode() const
{
    return m_qsetting->value(s_isImmersiveMode, false).toBool();
}

void ConfigurationManager::setImmersiveMode(bool ImmersiveMode)
{
    m_qsetting->setValue(s_isImmersiveMode, ImmersiveMode);
}
