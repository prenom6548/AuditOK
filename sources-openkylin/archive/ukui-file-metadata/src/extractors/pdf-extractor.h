/*
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: baijunjie <baijunjie@kylinos.cn>
 *
 */
#ifndef PDFEXTRACTOR_H
#define PDFEXTRACTOR_H

#include "extractor-plugin.h"

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <poppler-qt5.h>
#else
#include <poppler-qt6.h>
#endif

namespace UkuiFileMetadata {

class PdfExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukui.ukuifilemetadata.ExtractorPlugin"
                      FILE "pdf-extractor.json")
    Q_INTERFACES(UkuiFileMetadata::ExtractorPlugin)

public:
    PdfExtractor(QObject *parent = nullptr);
    void extract(ExtractionResult *result) override;
    QStringList mimetypes() const override;

    friend class PdfExtractorTest;

};
}


#endif // PDFEXTRACTOR_H
