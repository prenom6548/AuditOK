/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2023 kirito <baijunjie@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef UKUI_FILE_METADATA_TAGLIBEXTRACTOR_H
#define UKUI_FILE_METADATA_TAGLIBEXTRACTOR_H

#include "extractor-plugin.h"

namespace UkuiFileMetadata{
class TaglibExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukui.ukuifilemetadata.ExtractorPlugin"
                      FILE "taglib-extractor.json")
    Q_INTERFACES(UkuiFileMetadata::ExtractorPlugin)
public:
    explicit TaglibExtractor(QObject* parent = nullptr);

    void extract(ExtractionResult* result) override;
    QStringList mimetypes() const override;

    friend class TaglibExtractorTest;
};
}




#endif //UKUI_FILE_METADATA_TAGLIBEXTRACTOR_H
