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

#ifndef UKUI_FILE_METADATA_OFDEXTRACTOR_H
#define UKUI_FILE_METADATA_OFDEXTRACTOR_H

#include "extractor-plugin.h"

namespace UkuiFileMetadata{
class OfdExtractor : public ExtractorPlugin
{
    Q_OBJECT
        Q_PLUGIN_METADATA(IID "org.ukui.ukuifilemetadata.ExtractorPlugin"
                                  FILE "ofd-extractor.json")
        Q_INTERFACES(UkuiFileMetadata::ExtractorPlugin)
    public:
        explicit OfdExtractor(QObject *parent = nullptr);
        void extract(ExtractionResult *result) override;
        QStringList mimetypes() const override;
    };
}




#endif //UKUI_FILE_METADATA_OFDEXTRACTOR_H
