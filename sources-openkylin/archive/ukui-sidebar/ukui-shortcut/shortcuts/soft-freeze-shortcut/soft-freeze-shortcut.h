/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef SOFTFREEZESHORTCUT_H
#define SOFTFREEZESHORTCUT_H

#include "ukui-shortcut-plugin.h"
#include <QObject>
#include <QGSettings>
namespace UkuiShortcut {

class SoftFreezeShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "soft-freeze-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit SoftFreezeShortcutPlugin(QObject *parent = nullptr);

    QString pluginId() override { return QStringLiteral("SoftFreeze"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class SoftFreezeShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit SoftFreezeShortcut(QObject *parent = nullptr);
    ~SoftFreezeShortcut() override;
    QString pluginId() override;
    bool isEnable() override;
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;

private:
    inline void initMetaData();
    void freezeStateChanged(bool state);

private:
    QGSettings *m_gsettings = nullptr;
    bool m_state = false;
    bool m_isEnable = false;
    StatusInfo m_currentStatus;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
};


}
#endif // NIGHTMODESHORTCUT_H
