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


#include "wlcom-manager.h"

/*
 *wlcom环境色温对USD的需求：
 * 0.响应相关gsettings的设置，如色温配置调整，强调色。
 * 1.计算经纬度。
 * 2.获取当前分钟色温值。
 * 3.如果当前与预期存在差异则需要渐变。
 * 4.使用wlcom接口设置色温。
 * 5.监听屏幕增加，减少以及主屏切换的信号，并进行记录。
 * 6.使用wlcom接口设置色温，并发送主屏改变信号，屏幕改变信号。
*/

WlcomManager *WlcomManager::m_gammaManager = nullptr;
WlcomManager::WlcomManager()
{
    init(UsdBaseClass::isWayland());
    m_pGmDbus        = new GmDbus(this);
    m_pGmAdaptor     = new GmAdaptor(m_pGmDbus);

    m_xrandrInterface  = new QDBusInterface(DBUS_XRANDR_NAME,
                                            DBUS_XRANDR_PATH,
                                            DBUS_XRANDR_INTERFACE,
                                            QDBusConnection::sessionBus(),
                                            this);
    if (m_xrandrInterface->isValid()) {
        connect(m_xrandrInterface, SIGNAL(screensParamChanged(QString)), this, SIGNAL(doScreenParamChanged(QString)));
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService(DBUS_GM_NAME)) {
        sessionBus.registerObject(DBUS_GM_PATH,
                                  m_pGmDbus,
                                  QDBusConnection::ExportAllContents);
        USD_LOG(LOG_DEBUG, "register gamma manager dbus success");
    } else {
        USD_LOG(LOG_ERR, "register dbus error");
    }

    m_startTimer.setSingleShot(false);
    connect(&m_startTimer, SIGNAL(timeout()), this, SLOT(doStartTimerOut()));
    disableNightColorWithOutSignal();
}

WlcomManager::~WlcomManager()
{

}

WlcomManager *WlcomManager::WlcomManagerNew()
{
    if (m_gammaManager == nullptr) {
        m_gammaManager = new WlcomManager();
    }

    return m_gammaManager;
}

bool WlcomManager::Start()
{
    m_startTimer.start(1500);
    USD_LOG(LOG_DEBUG, ".");
    return true;
}

void WlcomManager::Stop()
{

}

int WlcomManager::setTemperature(int temp)
{
    QDBusInterface colorIft("org.ukui.Wlcom ",
                            "/com/kylin/Wlcom/Output",
                            "com.kylin.Wlcom.Output",
                            QDBusConnection::sessionBus());

    Q_FOREACH(const QString str , m_outputList) {
          colorIft.call("SetBrightness", str, temp);
          USD_LOG(LOG_DEBUG,"set %s temp to %d",str.toLatin1().data(),temp);
    }
    return temp;
}

void WlcomManager::doScreensParamChanged(QString param)
{

}

void WlcomManager::doStartTimerOut()
{
    QDBusReply<QString> replay = m_xrandrInterface->call(DBUS_XRANDR_GET_SCREEN_PARAM, qAppName());
    if (replay.isValid()) {
        if (replay.value().toUtf8().length() < 10) {
            USD_LOG(LOG_DEBUG,"reget output screen");
            return;
        }
        getScreenInfo(replay.value().toUtf8().data());
    }

    m_startTimer.stop();
    doColorSettingsChanged("");
    m_pGmLocation->start();
}

void WlcomManager::getScreenInfo(QString param)
{
    QStringList outputList;
    QJsonDocument parser;
    QVariantList outputs = parser.fromJson(param.toUtf8().data()).toVariant().toList();
    USD_LOG_SHOW_PARAMS(param.toLatin1().data());
    for (const auto& screenInfo : outputs) {
        const QString& outputName = ((screenInfo.toMap())[("metadata")].toMap())["name"].toString();
        if (((screenInfo.toMap())[("primary")].toBool())) {
            m_primaryOutputName = outputName;
        }
        outputList << outputName;
    }
    m_outputList = outputList;
    Q_FOREACH(const QString str , outputList) {
        USD_LOG_SHOW_PARAMS(str.toLatin1().data());
    }

    USD_LOG_SHOW_PARAMS(m_primaryOutputName.toLatin1().data());
}


//kwin 禁用x11下相关功能
void WlcomManager::disableNightColorWithOutSignal()
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

