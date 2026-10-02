/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "wayland-manager.h"

/*
 *ukui-kwin：不支持台式机亮度调节故不支持相关信号发送，
 * ukui-kwin色温对USD的需求：
 * 0.响应相关gsettings的设置，如色温配置调整，强调色。
 * 1.计算经纬度。
 * 2.获取当前分钟色温设置到ukui-kwin。
 * 3.设置完毕色温后需要在脚本中写入disable，避免在X环境中起冲突。
*/

/*
 * wlcom暂时使用wayland的处理，待libkscreen升级显示器名称与wlcom一致后再迁移到x-manager接入色温与亮度。
*/
WaylandManager *WaylandManager::m_gammaManager = nullptr;
WaylandManager::WaylandManager()
{
    init(UsdBaseClass::isWayland());

//    m_disableKwin = new QTimer(this);
//    m_disableKwin->setSingleShot(true);
//    connect(m_disableKwin, SIGNAL(timeout()), this, SLOT(doDisableKwinTimerOut()), Qt::DirectConnection);
}

WaylandManager::~WaylandManager()
{

}

WaylandManager *WaylandManager::GammaManagerNew()
{
    if (m_gammaManager == nullptr) {
        m_gammaManager = new WaylandManager();
    }

    return m_gammaManager;
}

bool WaylandManager::Start()
{
    m_pGmLocation->start();
    doColorSettingsChanged("");
    USD_LOG(LOG_DEBUG,"....");
    return true;
}

void WaylandManager::Stop()
{
    destroyQobject(m_disableKwin);
}

void WaylandManager::doDisableKwinTimerOut()
{
    disableNightColorWithOutSignal();
}

void WaylandManager::sendConfigChangedSignal(QStringList list)
{
    QDBusMessage notifySignal =
            QDBusMessage::createSignal("/kwinrc", "org.kde.kconfig.notify", "ConfigChanged");

    QHash<QString, QByteArrayList> signalArgs;
    QByteArrayList signalArray;

    Q_FOREACH (const QString &msg, list) {
        signalArray.append(msg.toUtf8());
    }

    signalArgs.insert("NightColor",signalArray);
    notifySignal.setArguments({QVariant::fromValue(signalArgs)});
    QDBusConnection::sessionBus().send(notifySignal);
}

void WaylandManager::syncColorSetToKwinWithSetSingleProp(int temp)
{
    // Register custom types for DBus communication
    qDBusRegisterMetaType<QByteArrayList>();
    qDBusRegisterMetaType<QHash<QString, QByteArrayList>>();



    // Define the properties that have changed
    QStringList changeProps;
    changeProps.append("NightColor.Mode");

    // Get the path to kwinrc config file
    const QString kwinrcPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/kwinrc");

    // Open the kwinrc config file
    QSettings kwinrcConfig(kwinrcPath, QSettings::IniFormat);

    kwinrcConfig.beginGroup("NightColor");

    // Set the Active property based on the enabled state
    kwinrcConfig.setValue("Active", true);

    kwinrcConfig.setValue("Mode", "Constant");
    kwinrcConfig.setValue("NightTemperature", temp);
    kwinrcConfig.sync();
    // Emit a signal indicating the changed properties
    sendConfigChangedSignal(changeProps);
}

int WaylandManager::setTemperature(int temp)
{
    QHash<QString, QVariant> nightConfig;
    QDBusInterface colorIft("org.ukui.KWin",
                            "/ColorCorrect",
                            "org.ukui.kwin.ColorCorrect",
                            QDBusConnection::sessionBus());

    QDBusMessage result = colorIft.call("nightColorInfo");

    if (result.type() != QDBusMessage::ReplyMessage) {
        USD_LOG(LOG_ERR,"call error...ready call syncColorSetToKwinOpenkylin()");
        syncColorSetToKwinWithSetSingleProp(temp);
        return true;
    }

    nightConfig.insert(KWIN_COLOR_ACTIVE, true);
    nightConfig.insert(KWIN_COLOR_MODE, 3);
    nightConfig.insert(KWIN_NIGHT_TEMP, temp);
    colorIft.asyncCall("setNightColorConfig", nightConfig);

    USD_LOG(LOG_DEBUG, "active:%d,mode:%d,temp:%d",nightConfig[KWIN_COLOR_ACTIVE].toBool(), nightConfig[KWIN_COLOR_MODE].toInt(),
            nightConfig[KWIN_NIGHT_TEMP].toInt());

    return true;
}
//kwin 禁用x11下相关功能
void WaylandManager::disableNightColorWithOutSignal()
{
    // Register custom types for DBus communication
    qDBusRegisterMetaType<QByteArrayList>();
    qDBusRegisterMetaType<QHash<QString, QByteArrayList>>();

    // Define the properties that have changed
    QStringList changeProps;
    changeProps.append("NightColor.Mode");

    // Get the path to kwinrc config file
    QString kwinrcPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/kwinrc");
    QFile configFile(kwinrcPath);
    if (false == configFile.exists()) {
        kwinrcPath = (QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/ukui-kwinrc"));
    }
    // Open the kwinrc config file
    QSettings kwinrcConfig(kwinrcPath, QSettings::IniFormat);

    // Begin the group for NightColor
    kwinrcConfig.beginGroup("NightColor");

    // Remove the Active property
    kwinrcConfig.remove("Active");

    // End the NightColor group and sync the changes to the config file
    kwinrcConfig.endGroup();
    kwinrcConfig.sync();

    USD_LOG(LOG_DEBUG,"signalShot to disbale for next login with X........");
}

