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
#include "pop-window-helper.h"
Q_GLOBAL_STATIC(PopWindowHelper, s_popWindowHelper)

PopWindowHelper *PopWindowHelper::self()
{
    return s_popWindowHelper;
}

PopWindowHelper::PopWindowHelper(QObject *parent) : QObject(parent)
{
    initMaxVolume();
}

PopWindowHelper::~PopWindowHelper()
{
    freeWindow();
}

void PopWindowHelper::initWindow()
{
    // 此设备支持隐私开关，再进行相关的初始化操作。
    QDBusInterface prvacyBus(QStringLiteral("org.freedesktop.OEMPrivacyKey"),
                                               QStringLiteral("/org/freedesktop/OEMPrivacyKey"),
                                               QStringLiteral("org.freedesktop.OEMPrivacyKey.Manager"), QDBusConnection::systemBus());
    if (prvacyBus.isValid()) {
        QTimer::singleShot(3*1000, [=](){
            m_privacyOsd = new PrivacyOSD();
        });
    }
}

void PopWindowHelper::freeWindow()
{
    if (!m_privacyOsd) {
        m_privacyOsd->deleteLater();
        m_privacyOsd = nullptr;
    }
}

int PopWindowHelper::getMaxVolume()
{
    return m_maxVolume;
}

void PopWindowHelper::initMaxVolume()
{
    if(QGSettings::isSchemaInstalled(UKUI_SOUND)){
        m_soundSettings = new QGSettings(UKUI_SOUND);
        if (m_soundSettings->keys().contains(QStringLiteral(VOLUME_INCREASED))) {
            bool value = m_soundSettings->get(VOLUME_INCREASED).toBool();
            if (value) {
                if (m_soundSettings->keys().contains(QStringLiteral(VOLUME_INCREASED_VALUE))) {
                    m_maxVolume = m_soundSettings->get(VOLUME_INCREASED_VALUE).toInt();
                } else {
                    m_maxVolume = MAX_VOLUM_INCREASED;
                }
            } else {
                m_maxVolume = MAX_VOLUME_NORMAL;
            }
        }
        connect(m_soundSettings, SIGNAL(changed(const QString&)), this, SLOT(volumeIncreased(const QString&)), Qt::DirectConnection);
    } else {
        m_maxVolume = MAX_VOLUME_NORMAL;
    }
}

void PopWindowHelper::volumeIncreased(const QString &key)
{
    if (key == QStringLiteral(VOLUME_INCREASED)) {
        bool value = m_soundSettings->get(VOLUME_INCREASED).toBool();
        if (value ) {
            if (m_soundSettings->keys().contains(QStringLiteral(VOLUME_INCREASED_VALUE))) {
                m_maxVolume = m_soundSettings->get(VOLUME_INCREASED_VALUE).toInt();
            } else {
                m_maxVolume = MAX_VOLUM_INCREASED;
            }
        } else {
            m_maxVolume = MAX_VOLUME_NORMAL;
        }
    }
}

void PopWindowHelper::showVolumeBarByDbus(int currunt, int max, bool mute)
{
    QDBusMessage message = QDBusMessage::createMethodCall(FRAMEWORK_DBUS_ID,
                                    FRAMEWORK_DBUS_PATH,
                                    FRAMEWORK_DBUS_INTERFACE,
                                    "ShowVolumeBarWithMute");

    message << currunt << max << mute;
    QDBusConnection::sessionBus().asyncCall(message);
}

void PopWindowHelper::showBrightnessBarByDbus(int currunt, int max)
{
    QDBusMessage message = QDBusMessage::createMethodCall(FRAMEWORK_DBUS_ID,
                                    FRAMEWORK_DBUS_PATH,
                                    FRAMEWORK_DBUS_INTERFACE,
                                    "ShowBrightnessBar");

    message << currunt << max;
    QDBusConnection::sessionBus().asyncCall(message);
}

void PopWindowHelper::showCustomOsdByDbus(const QString& icon)
{
    QDBusMessage message = QDBusMessage::createMethodCall(FRAMEWORK_DBUS_ID,
                                    FRAMEWORK_DBUS_PATH,
                                    FRAMEWORK_DBUS_INTERFACE,
                                    "ShowCustomOSD");

    message << icon;
    QDBusConnection::sessionBus().asyncCall(message);
}
