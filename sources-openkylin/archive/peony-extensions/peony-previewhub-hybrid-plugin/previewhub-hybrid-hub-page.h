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

#ifndef PREVIEWHUB_HYBRID_HUB_PAGE_H
#define PREVIEWHUB_HYBRID_HUB_PAGE_H

#include <QWidget>

#include <memory>

class QScrollArea;
class QShowEvent;

namespace Peony {
class FileWatcher;
}

class PreviewhubOpaquePreviewPage;

/**
 * @class PreviewhubHybridHubPage
 * @brief 混合预览的核心页面，上方显示 ukui-previewhub 预览内容，下方显示文件属性信息。
 *
 * 将预览控件注入到 DefaultPreviewPage 的滚动区域顶部，
 * 隐藏原有的图标区域，实现预览与文件信息的上下布局。
 * 预览区域随窗口宽度自适应，保持 4:3 宽高比。
 */
class PreviewhubHybridHubPage : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewhubHybridHubPage(QWidget *parent = nullptr);
    ~PreviewhubHybridHubPage() override;

    /** @brief 准备预览资源，加载文件信息并监听文件变化。 */
    void prepare(const QString &uri);

    /** @brief 启动预览显示，根据文件类型决定是否展示预览控件。 */
    void startPreview();

    /** @brief 取消当前预览，释放监听资源并隐藏预览控件。 */
    void cancel();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void setupScrollAreaStyle();
    void updateFileInfoContentAlignment();
    void injectPreviewWidget();
    void hideFileInfoIconArea();
    void updatePreviewGeometry();
    void scheduleHubPreviewStart();
    int availableColumnWidth() const;
    QScrollArea *fileInfoScrollArea() const;
    QWidget *fileInfoContentWidget() const;

    PreviewhubOpaquePreviewPage *m_page = nullptr;
    QWidget *m_previewHost = nullptr;
    QWidget *m_hubWidget = nullptr;

    std::shared_ptr<Peony::FileWatcher> m_watcher;
    QString m_uri;
    bool m_previewInjected = false;
    int m_hubPreviewGeneration = 0;
};

#endif
