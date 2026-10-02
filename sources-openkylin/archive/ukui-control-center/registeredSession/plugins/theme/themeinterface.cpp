/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "themeinterface.h"

#include <QDebug>
#include <QProcess>
#include <QDBusReply>

#define PERSONALISE_EFFECT_KEY "effect"

ThemeInterface::ThemeInterface():UKUI::ServiceObject(nullptr)
{
    m_wlcomDBus = UniversalInterface::self()->wlcomDbus();
    m_styleGSetting = UniversalInterface::self()->ukuiStyleGsettings();
    m_ukccPersonliseGSetting = UniversalInterface::self()->ukccPersonaliseGsettings();
    initWindowRadius();
}

bool ThemeInterface::initWindowRadius() {
    if (m_wlcomDBus) {
        QDBusReply<bool> isRenderingReply = m_wlcomDBus->call("IsHardwareRendering");
        if (isRenderingReply.isValid()) {
            bool isRendering = isRenderingReply.value();
            m_isHardwareRendering = isRendering;
            qDebug() << Q_FUNC_INFO << "isRendering" << isRendering;
            if (!isRendering) {
                if (m_ukccPersonliseGSetting != nullptr && m_ukccPersonliseGSetting->keys().contains("transparency")) {
                    m_ukccPersonliseGSetting->set("transparency", 1.0);
                    m_styleGSetting->set("menu-transparency", 100);
                } else {
                    qCritical() << "m_ukccPersonliseGSetting not contains the key: transparency";
                    return false;
                }
                if (m_styleGSetting != nullptr && m_styleGSetting->keys().contains("windowRadius")) {
                    m_styleGSetting->set("window-radius", "0,0");
                } else {
                    qCritical() << "m_styleGSetting not contains the key: window-radius";
                    return false;
                }
            }
        }
    } else {
        qCritical() << "wlcomDBus is null";
        return false;
    }
    return true;
}

bool ThemeInterface::isHardwareRendering() {
    qDebug() << Q_FUNC_INFO << "isHardwareRendering" << m_isHardwareRendering;
    return m_isHardwareRendering;
}

bool ThemeInterface::setEffectState(bool status) {
    qDebug() << Q_FUNC_INFO << "effectStateChange status" << status;
    // 仅写 GSettings；策略/重启后主题页从 effect 读开关状态
    if (m_ukccPersonliseGSetting == nullptr
        || !m_ukccPersonliseGSetting->keys().contains(PERSONALISE_EFFECT_KEY)) {
        qWarning() << Q_FUNC_INFO << "personalise gsettings unavailable or missing effect key";
        return false;
    }
    m_ukccPersonliseGSetting->set(PERSONALISE_EFFECT_KEY, status);
    emit effectChanged(status);
    return true;
}
