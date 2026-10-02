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

#ifndef EYECAREMODESHORTCUT_H
#define EYECAREMODESHORTCUT_H

#include <QObject>
#include <QGSettings>
#include "ukui-shortcut-plugin.h"
#include <QtDBus>

namespace UkuiShortcut {
class EyeCareModeShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "eyecare-mode-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit EyeCareModeShortcutPlugin(QObject *parent = nullptr);
    QString pluginId() override { return QStringLiteral("EyeCareShortcut"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class EyeCareModeShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit EyeCareModeShortcut(QObject *parent = nullptr);
    ~EyeCareModeShortcut() override;
    QString pluginId() override {return QStringLiteral("EyeCareShortcut");}
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;
    void setAddedState(bool added) override;

private Q_SLOTS:
    void onQGSettingsChanged(const QString & key);
    void onDbusConnectorReady(QDBusInterface* interface);

private:
    inline void initMetaData();
    void trigger();
    void setIconButtonColor();
    void setEnable(bool isEnable);

private:
    QGSettings *m_gsettings = nullptr;
    bool m_isEnable = false;
    bool m_isAdded = false;
    StatusInfo m_currentStatus;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
};
}
#endif // EYECAREMODESHORTCUT_H
