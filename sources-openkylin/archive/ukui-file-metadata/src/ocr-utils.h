/*
    SPDX-FileCopyrightText: 2023 Junjie Bai <baijunjie@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef UKUI_FILE_METADATA_OCR_UTILS_H
#define UKUI_FILE_METADATA_OCR_UTILS_H

#include "ukui-file-metadata_global.h"
#include <QObject>

namespace UkuiFileMetadata {
class UKUIFILEMETADATA_EXPORT OcrUtils
{
public:
    static QString getTextInPicture(const QString &path);

private:
    explicit OcrUtils() = default;
};
}

#endif //UKUI_FILE_METADATA_OCR_UTILS_H
