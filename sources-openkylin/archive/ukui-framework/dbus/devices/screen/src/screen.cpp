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

#include "screen.h"
/*
 * TODO: 亮度全用globalmanager作为后端，不再分开使用电源管理和gamma。
 * 目前暂不调整。
*/
#include <QDBusConnection>
Screen::Screen(QObject *parent):
    QObject(parent)
{
    m_pColorGsettingsManager = new GsettingsManager(this,
                                                    UKUI_SETTINGS_DAEMON_COLOR_SCHEMA,
                                                    setColorChangedHandler());

    m_pAutoGsettingsManager = new GsettingsManager(this,
                                                    UKUI_SETTINGS_DAEMON_COLOR_SCHEMA,
                                                    setAutoBrightChangedHandler());

    m_pGlobalManager  = new QDBusInterface(DBUS_XRANDR_NAME,
                                            DBUS_GC_BRIGHTNESS_PATH,
                                            DBUS_GC_BRIGHTNESS_INTERFACE,
                                            QDBusConnection::sessionBus(),
                                            this);

    QDBusConnection::sessionBus().connect(DBUS_XRANDR_NAME,
                                          DBUS_GC_BRIGHTNESS_PATH,
                                          DBUS_GC_BRIGHTNESS_INTERFACE,
                                          "primaryBrightnessChangedEnd",
                                          this,
                                          SLOT(doPrimaryScreenBrightnessChanged(uint)));

    QDBusConnection::sessionBus().connect(DBUS_XRANDR_NAME,
                                                  DBUS_GC_BRIGHTNESS_PATH,
                                                  DBUS_GC_BRIGHTNESS_INTERFACE,
                                                  "screenBrightnessChanged",
                                                  this,
                                                  SLOT(doScreenBrightnessChanged(QString, uint)));

    m_outputModeEnum = QMetaEnum::fromType<eScreenMode>();

    qRegisterMetaType<OutputGammaInfo>("OutputGammaInfo");
    qRegisterMetaType<OutputGammaInfoList>("OutputGammaInfoList");
    qDBusRegisterMetaType<OutputGammaInfo>();
    qDBusRegisterMetaType<OutputGammaInfoList>();

    m_brightControlByUPM = brightControlByUPM();

    if (Screen::eBrightnessControl::PowerManager == m_brightControlByUPM) {
        m_pscreen = new ScreenBrightNode(this);
    } else if (Screen::eBrightnessControl::Gamma == m_brightControlByUPM) {
        m_pscreen = new ScreenGamma(this);//brightness
    }
}

Screen::~Screen()
{

}

bool Screen::SetScreenBrightness(QString name, uint screenBrightness)
{
    if (Screen::eBrightnessControl::Unknown == m_brightControlByUPM) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support"));
        }
        return false;
    }

    PEEK_RESULT_BOOL(m_pscreen->setScreenBrightness(name, screenBrightness));
}

uint Screen::GetScreenBrightness(QString name)
{
    if (Screen::eBrightnessControl::Unknown == m_brightControlByUPM) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support"));
        }
        return 100;
    }

    QDBusReply<OutputGammaInfoList> reply = callGamma("getScreensGammaList", MY_NAME);
    for(auto outputInfo: reply.value()) {
        qDebug()<<outputInfo.outputName << outputInfo.brignthess << outputInfo.gamma<<outputInfo.temperature;
        if (!name.compare(outputInfo.outputName, Qt::CaseInsensitive)) {
            return outputInfo.brignthess;
        }
    }

    if (calledFromDBus()) {
    sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("can't find the screen:%1").arg(name));
    }
    return 100;
}

bool Screen::SetScreensBrightness(OutputGammaInfoList outputsInfo)
{
    if (Screen::eBrightnessControl::Unknown == m_brightControlByUPM) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support"));
        }
        return false;
    }

    PEEK_RESULT_BOOL(m_pscreen->setScreensBrightness(outputsInfo));
}

bool Screen::SetAllScreenSameBrightness(uint brighenss)
{
    if (Screen::eBrightnessControl::Unknown == m_brightControlByUPM) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support"));
        }
        return false;
    }

    PEEK_RESULT_BOOL(m_pscreen->setAllScreenSameBrightness(brighenss));
}

OutputGammaInfoList Screen::GetAllScreenBrightness()
{
    if (Screen::eBrightnessControl::Unknown == m_brightControlByUPM) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support"));
        }
        return OutputGammaInfoList();
    }

    return m_pscreen->getAllScreenBrightness();
}

/**
 * TODO 调用统一接口查询，设置屏幕亮度
 */
bool Screen::SetPrimaryScreenBrightness(uint screenBrightness)
{
    QDBusReply<bool> reply = callBrighenss("setPrimaryBrightness", screenBrightness);
    if (false == reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("call dbus error."));
        }
        return false;
    }

    return true;
}

uint Screen::GetPrimaryScreenBrightness()
{
    QDBusReply<uint> reply = callBrighenss("getPrimaryBrightness");
    if (false == reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("call dbus error"));
        }
        return 0;
    }

    return reply.value();
}

bool Screen::IncreaseBuiltinScreenBrightness()
{
    if (Screen::eBrightnessControl::PowerManager != m_brightControlByUPM) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support dimming."));
        }
        return false;
    }

    QDBusReply<void> reply = callMediaKeys("externalDoAction", Screen::eShutcutKey::IncreaseBrightness, MY_NAME);
    if (false == reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support dimming."));
        }
        return false;
    }

    return true;
}

bool Screen::DecreaseBuiltinBrightness()
{
    if (Screen::eBrightnessControl::PowerManager != m_brightControlByUPM) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support dimming."));
        }
        return false;
    }

    QDBusReply<void> reply = callMediaKeys("externalDoAction", Screen::eShutcutKey::DecreaseBrightness, MY_NAME);
    if (false == reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("externalDoAction call error."));
        }
        return false;
    }

    return true;
}

QString Screen::GetScreenMode()
{
    QDBusReply<int> reply = callXrandr("getScreenMode", MY_NAME);
    if (false == reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("call dbus error"));
        }
        return "";
    }

    return m_outputModeEnum.valueToKey(reply.value());
}

bool Screen::SetScreenMode(const QString &mode)
{
    QDBusReply<int> reply = callXrandr("setScreenMode", mode, MY_NAME);
    if (false == reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("call dbus error"));
        }
        return false;
    }

    return true;
}

bool Screen::SetScreensParam(const QString &screenParam)
{
    QDBusReply<int> reply = callXrandr("setScreensParam", MY_NAME, screenParam);
    if (false == reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("call dbus error"));
        }
        return false;
    }

    return true;
}

QString Screen::GetScreensParam()
{
    QDBusReply<QString> reply = callXrandr("getScreensParam", MY_NAME);
    if (false == reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("call dbus error"));
        }
        return "";
    }

    return reply.value();
}

bool Screen::SetEyeCareEnabled(bool state)
{
    QString ret;
    if (calledFromDBus() && !m_pColorGsettingsManager->peekActionIsValid(UKUI_SETTINGS_DAEMON_COLOR_MAIN_SWITCH, ret)) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support"));
        return false;
    }
    m_pColorGsettingsManager->set(UKUI_SETTINGS_DAEMON_COLOR_MAIN_SWITCH, state);
    
    if (state) {
        m_pColorGsettingsManager->set(UKUI_SETTINGS_DAEMON_COLOR_MAIN_SWITCH, state);
    }
    return ret.isEmpty();
}

bool Screen::IsEyeCareEnabled()
{
   return m_pColorGsettingsManager->get(UKUI_SETTINGS_DAEMON_COLOR_MAIN_SWITCH).toBool();
}

bool Screen::SetNightModeEnable(bool state)
{
    QString ret;
    if (calledFromDBus() && !m_pColorGsettingsManager->peekActionIsValid(UKUI_SETTINGS_DAEMON_COLOR_NIGHT_MODE, ret)) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support"));
        return false;
    }
    m_pColorGsettingsManager->set(UKUI_SETTINGS_DAEMON_COLOR_NIGHT_MODE, state);
    
    if (state) {
        m_pColorGsettingsManager->set(UKUI_SETTINGS_DAEMON_COLOR_MAIN_SWITCH, state);
    }
    return ret.isEmpty();
}

bool Screen::IsNightModeEnable()
{
    return  m_pColorGsettingsManager->get(UKUI_SETTINGS_DAEMON_COLOR_NIGHT_MODE).toBool();
}

bool Screen::SetScreenScale(double scale)
{
    QDBusReply<bool> reply = callXrandr("SetScreenScale",scale);
    if (false == reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(reply.error().type(), reply.error().message());
        }
        return false;
    }

    return reply.value();
}

double Screen::GetScreenScale()
{
    QDBusReply<double> reply = callXrandr(__FUNCTION__);
    if (false == reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(reply.error().type(), reply.error().message());
        }
        return false;
    }

    return reply.value();
}

bool Screen::IsAutoBrightnessEnable()
{
    if (!m_pAutoGsettingsManager->get(UKUI_SETTINGS_AUTO_BRIGHTNESS_HAD_SENSOR).toBool()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support"));
        }
        return false;
    }

    return m_pAutoGsettingsManager->get(UKUI_SETTINGS_AUTO_BRIGHTNESS_ENABLE).toBool();
}

void Screen::SetAutoBrightBrightnessEnable(bool enable)
{
    if (!m_pAutoGsettingsManager->get(UKUI_SETTINGS_AUTO_BRIGHTNESS_HAD_SENSOR).toBool()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QStringLiteral("The machine does not support"));
        }
        return;
    }

    m_pAutoGsettingsManager->set(UKUI_SETTINGS_AUTO_BRIGHTNESS_ENABLE, enable);
}

QHash<QString, OnKeyChanged> Screen::setColorChangedHandler()
{
    m_colorSettingsChangedHandlers[UKUI_SETTINGS_DAEMON_COLOR_MAIN_SWITCH] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_TOUCHPAD_ENABLE<<IsEyeCareEnabled();
#endif
        emit EyeCareChanged(IsAutoBrightnessEnable());
    };
    return m_colorSettingsChangedHandlers;
}

QHash<QString, OnKeyChanged> Screen::setAutoBrightChangedHandler()
{
    m_autoBrightSettingsChangedHandlers[UKUI_SETTINGS_DAEMON_AUTO_BRIGHTNESS] = [this]{
#ifdef _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<C_TOUCHPAD_ENABLE<<GetTouchpadEnabled();
#endif
        emit AutoBrightChanged(IsAutoBrightnessEnable());
    };
    return m_autoBrightSettingsChangedHandlers;
}

Screen::eBrightnessControl Screen::brightControlByUPM()
{
    QDBusInterface powerInterface(UKUI_POWER_MANAGER_DBUS_NAME,
                                  UKUI_POWER_MANAGER_DBUS_PATH,
                                  UKUI_POWER_MANAGER_DBUS_INTERFACE,
                                  QDBusConnection::systemBus());

    QDBusReply<bool> reply = powerInterface.call("CanSetBrightness");

    if (reply.isValid()) {
        if (reply.value()) {
            return Screen::eBrightnessControl::PowerManager;
        }
        return Screen::eBrightnessControl::Gamma;
    }
    qDebug()<<"链接电源管理systemBus失败"<<reply.error().message();
    return Screen::eBrightnessControl::Unknown;
}

void Screen::watchUpmRegisterDbusServer()
{
    qDebug() << "ready connect dbus server!";
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    auto watcher = new QDBusServiceWatcher(QStringLiteral(UKUI_POWER_MANAGER_DBUS_NAME), QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &Screen::serviceOwnerChanged);
#else
    connect(QDBusConnection::sessionBus().interface(), &QDBusConnectionInterface::serviceOwnerChanged, this, &Screen::serviceOwnerChanged);
#endif
}

template <typename...Args>
QDBusMessage Screen::callGamma(const QString &method, Args &&...args)
{
    QDBusInterface globalManagerInterface("org.ukui.SettingsDaemon",
                                          "/org/ukui/SettingsDaemon/GammaManager",
                                          "org.ukui.SettingsDaemon.GammaManager");

    return globalManagerInterface.call(method, std::forward<Args>(args)...);
}

template <typename...Args>
QDBusMessage Screen::callXrandr(const QString &method, Args &&...args)
{
    QDBusInterface globalManagerInterface("org.ukui.SettingsDaemon",
                                          "/org/ukui/SettingsDaemon/xrandr",
                                          "org.ukui.SettingsDaemon.xrandr");

    return globalManagerInterface.call(method, std::forward<Args>(args)...);
}

template <typename...Args>
QDBusMessage Screen::callBrighenss(const QString &method, Args &&...args)
{
    QDBusInterface globalManagerInterface("org.ukui.SettingsDaemon",
                                  "/GlobalBrightness",
                                  "org.ukui.SettingsDaemon.Brightness");


    return globalManagerInterface.call(method, std::forward<Args>(args)...);
}

template <typename...Args>
QDBusMessage Screen::callMediaKeys(const QString &method, Args &&...args)
{
    QDBusInterface mediaKeysInterface("org.ukui.SettingsDaemon",
                                      "/org/ukui/SettingsDaemon/MediaKeys",
                                      "org.ukui.SettingsDaemon.MediaKeys");


    return mediaKeysInterface.call(method, std::forward<Args>(args)...);
}

void Screen::serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    if (name.isEmpty()) {
        return;
    }

    if (!name.startsWith(UKUI_POWER_MANAGER_DBUS_NAME)) {
        return;
    }

    if (nullptr != m_pscreen) {
        delete m_pscreen;
        m_pscreen = nullptr;
    }

    m_brightControlByUPM = brightControlByUPM();
    if (Screen::eBrightnessControl::PowerManager == m_brightControlByUPM) {
        m_pscreen = new ScreenBrightNode(this);
    } else if (Screen::eBrightnessControl::Gamma == m_brightControlByUPM) {
        m_pscreen = new ScreenGamma(this);
    }

    disconnect(QDBusConnection::sessionBus().interface(), &QDBusConnectionInterface::serviceOwnerChanged, this, &Screen::serviceOwnerChanged);
}

void Screen::doPrimaryScreenBrightnessChanged(uint brightness)
{
 #ifdef _UKUI_FRAMEWORK_DEBUG
    qDebug()<<__FUNCTION__<<__LINE__<<":"<<brightness;
#endif
    emit PrimaryScreenBrightnessChanged(brightness);
}

void Screen::doScreenBrightnessChanged(QString screenName, uint brightness)
{
#ifdef _UKUI_FRAMEWORK_DEBUG
    qDebug()<<__FUNCTION__<<__LINE__<<","<<screenName<<brightness;
#endif

    emit ScreenBrightnessChanged(screenName, brightness);
}
