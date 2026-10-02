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

#ifndef PREVIEWHUB_HYBRID_PREVIEW_FACTORY_H
#define PREVIEWHUB_HYBRID_PREVIEW_FACTORY_H

#include <QObject>
#include <preview-page-plugin-iface.h>

class PreviewhubHybridPreviewFactory : public QObject, public Peony::PreviewPagePluginIface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PreviewPagePluginIface_iid FILE "common.json")
    Q_INTERFACES(Peony::PreviewPagePluginIface)

public:
    explicit PreviewhubHybridPreviewFactory(QObject *parent = nullptr);
    ~PreviewhubHybridPreviewFactory() override;

    PluginType pluginType() override { return PluginType::PreviewPagePlugin; }
    const QString name() override;
    const QString description() override;
    const QIcon icon() override;
    void setEnable(bool enable) override;
    bool isEnable() override;
    Peony::PreviewPageIface *createPreviewPage() override;

private:
    bool m_enable = true;
};

#endif
