/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
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

#include "configmonitor.h"
#include "logger.h"

ConfigMonitor *ConfigMonitor::instance()
{
    static ConfigMonitor inst;
    return &inst;
}

ConfigMonitor::ConfigMonitor(QObject *parent)
    : QObject(parent)
{
    initConfig();
}

void ConfigMonitor::migrateLegacyShortcut()
{
    if (!m_gsettings || !m_gsettings->keys().contains(KEY_SHORTCUT)) {
        return;
    }

    if (!QGSettings::isSchemaInstalled(LEGACY_SCHEMA_ID)) {
        return;
    }

    const QByteArray legacySchemaId(LEGACY_SCHEMA_ID);
    QGSettings legacySettings(legacySchemaId);
    if (!legacySettings.keys().contains(LEGACY_KEY_SHORTCUT)) {
        return;
    }

    const QString legacyShortcut = legacySettings.get(LEGACY_KEY_SHORTCUT).toString();
    if (legacyShortcut.isEmpty()) {
        return;
    }

    legacySettings.set(LEGACY_KEY_SHORTCUT, QString());
    VW_LOG(Logger::Level::Info) << "Migrated legacy shortcut to" << SCHEMA_ID
                                << KEY_SHORTCUT << ":" << legacyShortcut;
}

void ConfigMonitor::initConfig()
{
    if (!QGSettings::isSchemaInstalled(SCHEMA_ID)) {
        VW_LOG(Logger::Level::Error) << "GSettings schema not installed:" << SCHEMA_ID;
        return;
    }

    m_gsettings = new QGSettings(QByteArray(SCHEMA_ID), QByteArray(), this);
    if (!m_gsettings) {
        VW_LOG(Logger::Level::Error) << "Failed to create QGSettings for" << SCHEMA_ID;
        return;
    }

    migrateLegacyShortcut();

    connect(m_gsettings, &QGSettings::changed, this, &ConfigMonitor::onGSettingsChanged);

    if (m_gsettings->keys().contains(KEY_ENABLED)) {
        m_enabledCache = m_gsettings->get(KEY_ENABLED).toBool();
    } else {
        VW_LOG(Logger::Level::Error) << "Failed to get enabled from GSettings";
    }

    if (m_gsettings->keys().contains(KEY_SHORTCUT)) {
        m_shortcutCache = m_gsettings->get(KEY_SHORTCUT).toString();
    } else {
        VW_LOG(Logger::Level::Error) << "Failed to get shortcut from GSettings";
    }
}

bool ConfigMonitor::enabled() const
{
    if (!m_gsettings || !m_gsettings->keys().contains(KEY_ENABLED)) {
        VW_LOG(Logger::Level::Error) << "Failed to get enabled from GSettings";
        return false;
    }

    return m_enabledCache;
}

QString ConfigMonitor::shortcut() const
{
    if (!m_gsettings || !m_gsettings->keys().contains(KEY_SHORTCUT)) {
        VW_LOG(Logger::Level::Error) << "Failed to get shortcut from GSettings";
    }
    return m_shortcutCache;
}

void ConfigMonitor::onGSettingsChanged(const QString &key)
{
    if (!m_gsettings) {
        VW_LOG(Logger::Level::Error) << "Failed to get GSettings";
        return;
    }

    if (key == QString::fromUtf8(KEY_ENABLED)) {
        bool val = m_gsettings->get(KEY_ENABLED).toBool();
        if (val != m_enabledCache) {
            m_enabledCache = val;
            emit enabledChanged(val);
            VW_LOG(Logger::Level::Info) << "Enabled changed to" << val;
        }
        return;
    }

    if (key == QString::fromUtf8(KEY_SHORTCUT)) {
        QString val = m_gsettings->get(KEY_SHORTCUT).toString();
        if (val != m_shortcutCache) {
            m_shortcutCache = val;
            emit shortcutChanged(val);
            VW_LOG(Logger::Level::Info) << "Shortcut changed to" << val;
        }
    }
}
