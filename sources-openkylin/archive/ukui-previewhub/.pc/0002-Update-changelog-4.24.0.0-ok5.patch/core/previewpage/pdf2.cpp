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

#include "previewpage/pdf2.h"

Pdf2PreviewPage::Pdf2PreviewPage(QWidget *parent) : BasePreviewPage(parent)
{
    m_pdf_widget = new QPdfWidget(this);
    m_pdf_widget->showFullScreen();
    base_layout->addWidget(m_pdf_widget);
}

void Pdf2PreviewPage::cancel() {
    //m_pdf_widget->closeDocument();
}

void Pdf2PreviewPage::updateInfo(PreviewHubFileInfo *info) {
    m_pdf_widget->loadFile(info->filePath());
}

void Pdf2PreviewPage::updateInfo(QString path) {
    m_pdf_widget->loadFile(path);
}
