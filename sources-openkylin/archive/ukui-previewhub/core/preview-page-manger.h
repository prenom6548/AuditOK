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

#ifndef PREVIEWPAGEMANGER_H
#define PREVIEWPAGEMANGER_H

#include "previewpage/other.h"
#include "previewpage/audio.h"
#include "previewpage/text.h"
#include "previewpage/video.h"
#include "previewpage/image.h"
#ifdef USE_WEBENGINE_PREVIEW
#include "previewpage/html.h"
#include "previewpage/pdf2.h"
#include "previewpage/to-pdf.h"
#ifdef USE_OnlyOffice
#include "previewpage/onlyoffice.h"
#endif
#ifdef USE_QTPDF
#include "previewpage/pdf.h"
#endif
#endif

#include "content-preview-page.h"
#include "preview-file-type.h"

#include <QMap>

class PreviewPageManger
{
    typedef PreviewFileType::PreviewType PreviewType;
public:
    PreviewPageManger() = delete;
    explicit PreviewPageManger(ContentPreviewPage *preview_page, bool simplePreview);
    ~PreviewPageManger();

public:
    void cancel();
    void startPreview(PreviewFileType *info);

private:
    BasePreviewPage *createPage(PreviewType cas);

private:
    ContentPreviewPage *the_preview_page;

    QMap<PreviewType, BasePreviewPage *> m_page_pool;
    BasePreviewPage *m_current_page;
    bool m_simple_preview;
};

#endif // PREVIEWPAGEMANGER_H
