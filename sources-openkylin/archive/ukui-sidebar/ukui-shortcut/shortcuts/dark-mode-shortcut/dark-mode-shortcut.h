/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: an-yongtai <anyongtai@kylinos.cn>
 *
 */

#ifndef DARKMODESHORTCUT_H
#define DARKMODESHORTCUT_H

#include <QObject>
#include <QGSettings>
#include "ukui-shortcut-plugin.h"

namespace UkuiShortcut {
class DarkModeShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "dark-mode-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit DarkModeShortcutPlugin(QObject *parent = nullptr);
    QString pluginId() override { return QStringLiteral("DarkModeShortcut"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class DarkModeShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit DarkModeShortcut(QObject *parent = nullptr);
    ~DarkModeShortcut() override;
    QString pluginId() override {return QStringLiteral("DarkModeShortcut");}
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;
    void setAddedState(bool added) override;

private Q_SLOTS:
    void onQGSettingsChanged(const QString& key);

private:
    inline void initMetaData();
    void trigger();
    void setIconButtonColor();

private:
    QGSettings *m_gsettingsUkui = nullptr;
    QGSettings *m_gsettingsGtk = nullptr;
    bool m_isEnable = false;
    bool m_isAdded = false;
    StatusInfo m_currentStatus;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
};
}
#endif // DARKMODESHORTCUT_H
