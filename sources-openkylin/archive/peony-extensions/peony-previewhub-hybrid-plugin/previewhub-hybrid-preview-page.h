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

#ifndef PREVIEWHUB_HYBRID_PREVIEW_PAGE_H
#define PREVIEWHUB_HYBRID_PREVIEW_PAGE_H

#include <QStackedWidget>
#include <preview-page-plugin-iface.h>

namespace Peony {
class DefaultPreviewPage;
}

class PreviewhubHybridHubPage;

/**
 * @class PreviewhubHybridPreviewPage
 * @brief 混合预览页面，作为预览模式的调度器。
 *
 * 通过 QStackedWidget 在两种预览实现间切换：
 * 文件夹及通用文件使用 Peony 默认预览页（m_defaultPage）；
 * 文本、图片、音视频、PDF 及办公文档使用混合预览页（m_hubPage）。
 * 模式根据显式传入的预览类型或自动解析的 MIME 类型决定。
 */
class PreviewhubHybridPreviewPage : public QStackedWidget, public Peony::PreviewPageIface
{
    Q_OBJECT

public:
    explicit PreviewhubHybridPreviewPage(QWidget *parent = nullptr);
    ~PreviewhubHybridPreviewPage() override;

    void prepare(const QString &uri, Peony::PreviewPageIface::PreviewType type) override;
    void prepare(const QString &uri) override;
    void startPreview() override;
    void cancel() override;
    void closePreviewPage() override;

private:
    void updateModeAndDelegatePrepare(const QString &uri);
    static bool isHubMime(const QString &mime);
    static QString resolveMime(const QString &uri);
    static bool canUseHubPreview(const QString &uri);

    Peony::DefaultPreviewPage *m_defaultPage = nullptr;
    PreviewhubHybridHubPage *m_hubPage = nullptr;

    QString m_uri;
    bool m_explicitType = false;
    Peony::PreviewPageIface::PreviewType m_passedType = Peony::PreviewPageIface::Other;
    bool m_useHub = false;
};

#endif
