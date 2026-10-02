/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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

#ifndef QUICK_LOOK_PREVIEW_FACTORY_H
#define QUICK_LOOK_PREVIEW_FACTORY_H

#include <QObject>
#include <quick-look-plugin-iface.h>

class QuickLookPreviewFactory : public QObject, public Peony::QuickLookPluginIface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QuickLookPluginIface_iid FILE "common.json")
    Q_INTERFACES(Peony::QuickLookPluginIface)

public:
    explicit QuickLookPreviewFactory(QObject *parent = nullptr);
    ~QuickLookPreviewFactory() override = default;

    PluginType pluginType() override { return PluginType::QuickLookPlugin; }
    const QString name() override;
    const QString description() override;
    const QIcon icon() override;
    void setEnable(bool enable) override;
    bool isEnable() override;
    Peony::QuickLookWindowIface *createQuickLookWindow() override;

private:
    bool m_enable = true;
};

#endif
