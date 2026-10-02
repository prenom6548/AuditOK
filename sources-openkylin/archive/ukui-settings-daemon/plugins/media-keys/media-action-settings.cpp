/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#include "media-action-settings.h"
#include <QDBusInterface>
#include <QDBusReply>
extern "C" {
#include "clib-syslog.h"
#include "media-action-settings.h"
}
#define POINTER_SCHEMA          "org.ukui.SettingsDaemon.plugins.input-device-manager"
#define POWER_SCHEMA            "org.ukui.power-manager"
#define SESSION_SCHEMA          "org.ukui.session"
#define SHOT_SCHEMA             "org.ukui.screenshot"
#define UKUI_TOUCHPAD_SCHEMA    "org.ukui.peripherals-touchpad"

const static QString s_pointerKey = "locate-pointer";

const static QString s_powerKey = "button-power";

const static QString s_winKey = "win-key-release";

const static QString s_shortRunKey = "isrunning";

const static QString s_touchpadEnableKey = "touchpad-enabled";

const static QString s_brightnessAC = "brightness-ac";

const static QString s_canSetBrightness = "canSetBrightness";

Q_GLOBAL_STATIC(MediaActionSettings, s_mediaActionSettings)
MediaActionSettings::MediaActionSettings(QObject *parent) : QObject(parent)
{

}

MediaActionSettings *MediaActionSettings::instance()
{
    return s_mediaActionSettings;
}

void MediaActionSettings::initSettings()
{
    m_touchpadSettings = initSettings(UKUI_TOUCHPAD_SCHEMA, s_touchpadEnableKey);
    m_mouseSettings = initSettings(POINTER_SCHEMA, s_pointerKey);
    m_powerSettings = initSettings(POWER_SCHEMA, s_powerKey);
    m_sessionSettings = initSettings(SESSION_SCHEMA, s_winKey);
    m_shotSettings = initSettings(SHOT_SCHEMA, s_shortRunKey);
    initCanSetBrightness();

    m_isEnhancementBrightness = isEnhancementBrightness();
}

QGSettings* MediaActionSettings::initSettings(const QByteArray &schema, const QString& key)
{
    if (QGSettings::isSchemaInstalled(schema)) {
        QGSettings* settings = new QGSettings(schema);
        if (settings->keys().contains(key)) {
            m_settingsData.insert(key, settings->get(key));
        }
        connect(settings, &QGSettings::changed, this, [=](const QString& _key) {
            if (key == _key) {
                m_settingsData.insert(key, settings->get(key));
            }
        });
        return settings;
    }
    USD_LOG(LOG_DEBUG,"init %s error",schema.data());
    return nullptr;
}

void MediaActionSettings::initCanSetBrightness()
{
    QDBusInterface powerInterface("org.ukui.powermanagement",
                                  "/",
                                  "org.ukui.powermanagement.interface",
                                  QDBusConnection::systemBus());
    QDBusReply<bool> replay = powerInterface.call("CanSetBrightness");
    if(replay.isValid()) {
        m_settingsData.insert(s_canSetBrightness, replay.value());
    }
}

bool MediaActionSettings::isEnhancementBrightness() const
{
    QDBusInterface powerInterface("org.ukui.powermanagement",
                                  "/",
                                  "org.ukui.powermanagement.interface",
                                  QDBusConnection::systemBus());

    if (!powerInterface.isValid()) {
        return false;
    }

    QDBusReply<bool> reply = powerInterface.call("IsBrightnessEnhancementSupported");
    if(!reply.isValid()) {
        QString error = reply.error().message();
        SYS_LOG(LOG_WARNING, "errorMsg:%s", error.toLatin1().data());
    }

    return reply.value();
}

int MediaActionSettings::getMinBrightness() const
{
    return 0;
}

int MediaActionSettings::getMaxBrightness() const
{
      return (m_isEnhancementBrightness && m_powerSettings->get("brightness-enhancement-switch").toBool()) ?
                  G_ENHANCED_MAX_BRIGHTNESS : G_DEFAULT_MAX_BRIGHTNESS;
}

QVariant MediaActionSettings::getValue(const QString &key, const QVariant& value)
{
    return m_settingsData.value(key, value);
}

QVariant MediaActionSettings::getValue(const QString &key)
{
    return m_settingsData.value(key, QVariant());
}

QVariant MediaActionSettings::getPowerKeyState() const
{
    if (m_powerSettings && m_powerSettings->keys().contains(s_powerKey)) {
        return m_powerSettings->getEnum(s_powerKey);
    }

    USD_LOG(LOG_WARNING, "power settings is init error .");
    return QVariant();
}

void MediaActionSettings::setBrightnessValue(uint value)
{
    if (m_powerSettings == nullptr) {
        return;
    }
    USD_LOG_SHOW_PARAM1(value);
    m_powerSettings->set(s_brightnessAC, value);
}

int MediaActionSettings::getBrightnessValue() const
{
    if (m_powerSettings == nullptr) {
        return getMinBrightness();
    }

    return std::min(std::max(m_powerSettings->get(s_brightnessAC).toInt(),
                             getMinBrightness()),
                    getMaxBrightness());
}

int MediaActionSettings::updateBrightnessWithAction(ActionType action)
{
    if (isBrightnessOverridden(action)) {
        return getMinBrightness();
    }

    if (m_powerSettings == nullptr) {
        return getMinBrightness();
    }

    int brightness = getBrightnessValue();

    if (action == UsdEnumClass::eActionType::BRIGHT_UP_KEY) {
        brightness += C_STEP_BRIGHTNESS;
    } else if (action == UsdEnumClass::eActionType::BRIGHT_DOWN_KEY) {
        brightness -= C_STEP_BRIGHTNESS;
    }
    USD_LOG_SHOW_PARAM1(brightness);
    brightness = std::min(std::max(brightness,
                             getMinBrightness()),
                    getMaxBrightness());
    USD_LOG_SHOW_PARAM1(brightness);
    setBrightnessValue(brightness);

    return brightness;
}

bool MediaActionSettings::getCanSetBrightness()
{
    return getValue(s_canSetBrightness, false).toBool();
}

QVariant MediaActionSettings::getTouchpadState() const
{
    if (m_touchpadSettings == nullptr) {
        USD_LOG(LOG_WARNING, "touchpad settings is init error .");
        return QVariant();
    }

    return m_touchpadSettings->get(s_touchpadEnableKey);
}

void MediaActionSettings::setTouchpadState(bool state)
{
    if (m_touchpadSettings == nullptr) {
        return;
    }

    m_touchpadSettings->set(s_touchpadEnableKey, state);
}

void MediaActionSettings::setLocatePointer()
{
    if (m_mouseSettings == nullptr) {
        USD_LOG(LOG_WARNING, "locate pointer settings is error .");
        return;
    }

    if (!UsdBaseClass::isWayland()) {
        m_mouseSettings->set(s_pointerKey, !m_mouseSettings->get(s_pointerKey).toBool());
    } else {
        USD_LOG(LOG_WARNING, "skip locate pointer settings.");
    }
}

QVariant MediaActionSettings::getPowerSettings(const QString& key)
{
    if (m_powerSettings == nullptr) {
        return QVariant("");
    }

    return m_powerSettings->get(key);
}

void MediaActionSettings::setPowerSettings(const QString &key, const QVariant &value)
{
    if (m_powerSettings == nullptr) {
        return;
    }

    m_powerSettings->set(key, value);
}

bool MediaActionSettings::getSessionState()
{
    return getValue(s_winKey, false).toBool();
}

void  MediaActionSettings::rewriteBrightnessNode() const
{
    QDBusInterface upmInter("org.ukui.powermanagement",
                           "/",
                           "org.ukui.powermanagement.interface",
                           QDBusConnection::systemBus());
    upmInter.asyncCall("resetBrightness");
}

bool MediaActionSettings::isBrightnessOverridden(ActionType action) const
{
    if (action == UsdEnumClass::eActionType::BRIGHT_DOWN_KEY &&
            getBrightnessValue() == getMinBrightness()) {
            rewriteBrightnessNode();
            return true;
    }

    return false;
}
