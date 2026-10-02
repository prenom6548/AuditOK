/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef SHAREMENUPLUGIN_H
#define SHAREMENUPLUGIN_H

#include "peony-share-menu-plugin_global.h"
#include "menu-plugin-iface.h"

#include <QDebug>

namespace Peony {

class PEONYSHAREMENUPLUGINSHARED_EXPORT ShareMenuPlugin : public QObject, public MenuPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID MenuPluginInterface_iid FILE "common.json")
    Q_INTERFACES(Peony::MenuPluginInterface)

public:
    explicit ShareMenuPlugin(QObject *parent = nullptr);

    PluginInterface::PluginType pluginType() override {
        return PluginInterface::MenuPlugin;
    }

    const QString name() override {
        return tr("Peony-Qt Share Operation menu Extension");
    }

    const QString description() override {
        return tr("File share operation menu Extension");
    }

    const QIcon icon() override {
        return QIcon::fromTheme("emblem-link-symbolic");
    }

    void setEnable(bool enable) override {
        m_enable = enable;
    }

    bool isEnable() override {
        return m_enable;
    }

    QString testPlugin() override {
        return "test share operation";
    }

    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;

    bool checkOriginPath(const QString& selectFilePath, const QString& configFilePath);

    bool isSupportUri(const QString& uri);

private:
    bool m_enable = true;
};

}

#endif // SHAREMENUPLUGIN_H
