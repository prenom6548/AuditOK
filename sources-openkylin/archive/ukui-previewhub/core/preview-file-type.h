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

#ifndef PREVIEWFILETYPE_H
#define PREVIEWFILETYPE_H

#include <QString>

#include "previewhub_fileinfo.h"

class PreviewFileType
{
    friend class PreviewPageManger;
public:
    explicit PreviewFileType(const QString &uri);

    enum PreviewType {
        Text,
        Audio,
        Video,
        Image,
        Pdf,
        OfficeDoc,
        Other
    };

    PreviewType getPreviewType();
    QString getExtensionName();
    void sync();

    static std::shared_ptr<PreviewFileType> fromUri(const QString &uri);

private:
    std::shared_ptr<PreviewHubFileInfo> m_info;
};

#endif // PREVIEWFILETYPE_H
