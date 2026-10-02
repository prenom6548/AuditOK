/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Meihong <hemeihong@kylinos.cn>
 *
 */

#ifndef SENDTOCONNECTIVITY_H
#define SENDTOCONNECTIVITY_H

#include <QObject>
#include <QProcess>
#include <menu-plugin-iface.h>
#include <QDBusInterface>
#include "send-to-connectivity_global.h"

namespace Peony {

class PEONYQTENGRAMPAMENUPLUGINSHARED_EXPORT SendToKConnectivity
    : public QObject
    , public MenuPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID MenuPluginInterface_iid FILE "common.json")
    Q_INTERFACES(Peony::MenuPluginInterface)
public:
    explicit SendToKConnectivity(QObject *parent = nullptr);

    QString testPlugin() override
    {
        return "";
    }
    bool isEnable() override
    {
        return m_enable;
    }
    void setEnable(bool enable) override
    {
        m_enable = enable;
    }
    const QString description() override
    {
        return tr("Send to a connectivity");
    }
    const QString name() override
    {
        return "Peony Qt Send to a kylin connectivity";
    }
    const QIcon icon() override
    {
        return QIcon::fromTheme("");
    }
    PluginInterface::PluginType pluginType() override
    {
        return PluginInterface::MenuPlugin;
    }

    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;

private:
    bool m_enable;
    QProcess *m_process = nullptr;
};

}  // namespace Peony

#endif  // SENDTOCONNECTIVITY_H
