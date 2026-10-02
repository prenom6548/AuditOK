/*
 * Peony-Qt
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 */

#ifndef TASKBARMENUPPLUGIN_H
#define TASKBARMENUPPLUGIN_H

#include <QObject>
#include <QtDBus>
#include <atomic>
#include "menu-plugin-iface.h"

namespace Peony {

class TaskbarMenuPlugin : public QObject, public MenuPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID MenuPluginInterface_iid FILE "taskbar-menu-plugin.json")
    Q_INTERFACES(Peony::MenuPluginInterface)

public:
    explicit TaskbarMenuPlugin(QObject *parent = nullptr);

    PluginInterface::PluginType pluginType() override {
        return PluginInterface::MenuPlugin;
    }
    const QString name() override {
        return tr("Peony Taskbar Extension");
    }
    const QString description() override {
        return tr("Add/Remove desktop file to taskbar.");
    }
    const QIcon icon() override {
        return QIcon::fromTheme("pin-joblist-symbolic");
    }
    void setEnable(bool enable) override {
        m_enable = enable;
    }
    bool isEnable() override {
        return m_enable;
    }

    QString testPlugin() override {
        return "test taskbar plugin";
    }
    QList<QAction *> menuActions(Types types, const QString &uri,
                                 const QStringList &selectionUris) override;

Q_SIGNALS:
    void versionChecked(bool isV10SP1);

private:
    bool isV10SP1();
    bool checkQuickLauncher(const QString &desktopFile);
    bool addQuickLauncher(const QString &desktopFile);
    bool removeQuickLauncher(const QString &desktopFile);
    QString getDesktopFilePath(const QString &uri);
    void checkVersion();

    bool m_enable = true;
    bool m_isV10SP1 = false;
    std::atomic<bool> m_versionChecked{false};
};

}

#endif // TASKBARMENUPPLUGIN_H
