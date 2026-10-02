/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 *
 */

#ifndef FLIGHTMODESHORTCUT_H
#define FLIGHTMODESHORTCUT_H
#include "ukui-shortcut-plugin.h"
#include <QGSettings>

namespace UkuiShortcut {

class FlightModeShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "flight-mode-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit FlightModeShortcutPlugin(QObject *parent = nullptr);

    QString pluginId() override { return QStringLiteral("FlightModeShortcut"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class  FlightModeShortcut : public Shortcut
{
    Q_OBJECT
public:
    FlightModeShortcut();

    QString pluginId() override {return QStringLiteral("FlightModeShortcut");}
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;

private:
    inline void initMetaData();
    void stateChanged(int state);

    QGSettings *m_gsettings = nullptr;
    int m_state = -1;
    bool m_isEnable = true;
    StatusInfo m_statusInfo;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
};
}
#endif // FLIGHTMODESHORTCUT_H
