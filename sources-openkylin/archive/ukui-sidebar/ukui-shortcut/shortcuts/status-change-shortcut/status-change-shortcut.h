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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef STATUSCHANGESHORTCUT_H
#define STATUSCHANGESHORTCUT_H
#include "ukui-shortcut-plugin.h"

#include <QMutex>
#include <QDBusInterface>

namespace UkuiShortcut {

class StatusChangeShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "status-change-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit StatusChangeShortcutPlugin(QObject *parent = nullptr);

    QString pluginId() override { return QStringLiteral("StatusChangeShortcut"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class StatusChangeShortcut  : public Shortcut
{
    Q_OBJECT
public:
    StatusChangeShortcut();
    ~StatusChangeShortcut() override;

    QString pluginId() override;
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;

private Q_SLOTS:
    void statusChangeSlot(bool isTabletMode);
    void enableChanged(bool isEnable);

private:
    inline void initMetaData();
    void setStatusInfo(bool isTabletMode);
    void initInfo();

    bool m_isEnable = false;
    bool m_isTabletMode = false;
    StatusInfo m_currentStatusInfo;
    QDBusInterface *m_statusManagerIface = nullptr;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
};
}
#endif // STATUSCHANGESHORTCUT_H
