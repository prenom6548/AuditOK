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

#include "previewpage/html.h"

HtmlPreviewPage::HtmlPreviewPage(QWidget *parent) : BasePreviewPage(parent)
{
    m_web_page = new QWebEnginePage(this);
    m_web_channel = new QWebChannel(this);
    m_web_page->setWebChannel(m_web_channel);

    m_web_view = new QWebEngineView(this);
    m_web_view->setPage(m_web_page);
    m_web_view->setContentsMargins(0, 0, 0, 0);

    base_layout->addWidget(m_web_view);
}

void HtmlPreviewPage::cancel() {
    qDebug() << "HtmlPreviewPage::cancel: ";
    //m_web_view->close()
}

void HtmlPreviewPage::updateInfo(PreviewHubFileInfo *info) {
    qDebug() << "HtmlPreviewPage::updateInfo: " << info->filePath();
    m_web_page->load(QUrl(info->uri()));
}
