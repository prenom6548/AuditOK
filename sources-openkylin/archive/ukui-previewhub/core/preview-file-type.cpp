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

#include "preview-file-type.h"
#ifdef USE_WEBENGINE_PREVIEW
#include "libreoffice-runtime.h"
#endif

#include <QDebug>

PreviewFileType::PreviewFileType(const QString &uri):
    m_info(PreviewHubFileInfo::fromUri(uri))
{
//    m_extension_name = getExtensionName();
//    m_preview_type = getPreviewType();
}

std::shared_ptr<PreviewFileType> PreviewFileType::fromUri(const QString &uri) {
    return std::make_shared<PreviewFileType>(uri);
}

QString PreviewFileType::getExtensionName() {
    for (int i = m_info->filePath().size()-1; i >= 0; i--)
        if (m_info->filePath().at(i) == '.')
            return m_info->filePath().right(m_info->filePath().size()-i-1);
    return QLatin1String("");
}

PreviewFileType::PreviewType PreviewFileType::getPreviewType() {
    const QString path = m_info->filePath();
    const QString type = m_info->fileType();
    const QString mimeType = m_info->mimeType();
    qDebug() << "Path :" << path << " \nType:" << type << "\n mimeType:"<<mimeType;

    // Prevent .ncm files from being recognized as Video
    if (type == "unknown")
        return Other;
    if (mimeType.startsWith("text") ||
            mimeType == "application/xml" ||
            mimeType == "application/json" ||
            mimeType == "application/javascript" ||
            mimeType == "application/vnd.nokia.qt.qmakeprofile")
        return Text;
    if (mimeType.startsWith("audio"))
        return Audio;
    if (m_info->isVideoFile())
        return Video;
    if (m_info->isImageFile())
        return Image;
#ifdef USE_WEBENGINE_PREVIEW
    if (m_info->isPdfFile())
        return Pdf;
#ifdef USE_OnlyOffice
    if (m_info->isOfficeFile())
        return OfficeDoc;
#else
    if (m_info->isOfficeFile() && LibreOfficeRuntime::isAvailable())
        return OfficeDoc;
#endif
#endif
    return Other;
}

void PreviewFileType::sync() {
    if (m_info->displayName().isEmpty()) {
        m_info->syncRefresh();
    }
}
