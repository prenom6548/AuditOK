/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co, Ltd.
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
 * Authors: Lu Hongxu <lhongxu@outlook.com>
 *
 */

#ifndef CONTENTPREVIEWPAGE_H
#define CONTENTPREVIEWPAGE_H

#include <QStackedWidget>
#include <memory>

#include "preview-file-type.h"
#include "previewhub_filewatcher.h"

class PreviewPageManger;

class ContentPreviewPage : public QStackedWidget
{
    Q_OBJECT
public:
    explicit ContentPreviewPage(QWidget *parent = nullptr);
    ~ContentPreviewPage();
    void prepare(const QString &uri);
    void startPreview();
    void cancel();
    void closePreviewPage() {
        deleteLater();
    }

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    QString m_current_uri;
    PreviewPageManger *m_page_manger = nullptr;

    QWidget *m_empty_tab_widget;
    std::shared_ptr<PreviewFileType> m_file_info;
    PreviewHubFileWatcher *m_watcher = nullptr;

    bool m_support = true;
};

#endif // CONTENTPREVIEWPAGE_H
