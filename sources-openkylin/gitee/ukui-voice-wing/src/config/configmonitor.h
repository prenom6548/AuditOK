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

#ifndef CONFIGMONITOR_H
#define CONFIGMONITOR_H

#include <QObject>
#include <QGSettings>

class ConfigMonitor : public QObject
{
    Q_OBJECT
public:
    static ConfigMonitor *instance();

    bool enabled() const;
    QString shortcut() const;


Q_SIGNALS:
    void enabledChanged(bool enabled);
    void shortcutChanged(const QString &shortcut);

private Q_SLOTS:
    void onGSettingsChanged(const QString &key);

private:
    explicit ConfigMonitor(QObject *parent = nullptr);

    void initConfig();
    void migrateLegacyShortcut();

    QGSettings *m_gsettings = nullptr;

    bool m_enabledCache = false;
    QString m_shortcutCache = QStringLiteral("<Super>c");

    static constexpr const char *SCHEMA_ID = "org.ukui.voice-wing";
    static constexpr const char *KEY_ENABLED = "enabled";
    static constexpr const char *KEY_SHORTCUT = "shortcut";

    static constexpr const char *LEGACY_SCHEMA_ID =
        "org.ukui.SettingsDaemon.plugins.media-keys.kylin-aiassistant";
    static constexpr const char *LEGACY_KEY_SHORTCUT = "shortcutkeyKylinAi";
};

#endif // CONFIGMONITOR_H
