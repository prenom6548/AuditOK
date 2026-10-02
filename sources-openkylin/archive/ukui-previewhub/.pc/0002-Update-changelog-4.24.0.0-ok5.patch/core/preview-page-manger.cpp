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

#include "preview-page-manger.h"

PreviewPageManger::PreviewPageManger(ContentPreviewPage *preview_page, bool simplePreview) :
    the_preview_page(preview_page),
    m_current_page(nullptr),
    m_simple_preview(simplePreview)
{
}

PreviewPageManger::~PreviewPageManger() {
    qDeleteAll(m_page_pool);
    m_page_pool.clear();
}

BasePreviewPage *PreviewPageManger::createPage(PreviewType cas) {
    switch (cas) {
    case PreviewType::Text:
        return new TextPreviewPage;
    case PreviewType::Audio:
        return new AudioPreviewPage;
    case PreviewType::Video:
        return new VideoPreviewPage;
    case PreviewType::Image:
        return new ImagePreviewPage(m_simple_preview);
    case PreviewType::Pdf:
#ifdef USE_WEBENGINE_PREVIEW
#ifdef USE_QTPDF
        return new PdfPreviewPage;
#else
        return new Pdf2PreviewPage;
#endif
#else
        return new OtherPreviewPage;
#endif
    case PreviewType::OfficeDoc:
#ifdef USE_WEBENGINE_PREVIEW
#ifdef USE_OnlyOffice
        return new DocPreviewPage;
#else
        return new ToPdfPreviewPage;
#endif
#else
        return new OtherPreviewPage;
#endif
    default:
        return new OtherPreviewPage;
    }
}

void PreviewPageManger::startPreview(PreviewFileType *info) {
    auto type = info->getPreviewType();
    if (m_page_pool.contains(type)) {
        m_current_page = m_page_pool[type];
    } else {
        m_current_page = createPage(type);
        m_page_pool.insert(type, m_current_page);
        the_preview_page->addWidget(m_current_page);
    }
    m_current_page->updateInfo(info->m_info.get());
    the_preview_page->setCurrentWidget(m_current_page);
}

void PreviewPageManger::cancel() {
    if (m_current_page != nullptr)
        m_current_page->cancel();
}
