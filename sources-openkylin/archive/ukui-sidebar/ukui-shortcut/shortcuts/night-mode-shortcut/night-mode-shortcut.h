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

#ifndef NIGHTMODESHORTCUT_H
#define NIGHTMODESHORTCUT_H

#include <QObject>
#include <QGSettings>
#include "ukui-shortcut-plugin.h"

namespace UkuiShortcut {

class NightModeShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "night-mode-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit NightModeShortcutPlugin(QObject *parent = nullptr);

    QString pluginId() override { return QStringLiteral("NightModeShortcut"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class NightModeShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit NightModeShortcut(QObject *parent = nullptr);
    ~NightModeShortcut() override;

    QString pluginId() override {return QStringLiteral("NightModeShortcut");}
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;

private:
    inline void initMetaData();

private:
    QGSettings *m_gsettings = nullptr;
    bool m_isEnable = false;

    StatusInfo m_currentStatus;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
};

}
#endif // NIGHTMODESHORTCUT_H
