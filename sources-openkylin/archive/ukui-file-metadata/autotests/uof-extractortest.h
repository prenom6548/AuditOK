/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Modified by: baijunjie <baijunjie@kylinos.cn>
 *
 */

#ifndef UKUI_FILE_METADATA_UOFEXTRACTORTEST_H
#define UKUI_FILE_METADATA_UOFEXTRACTORTEST_H

#include <QObject>

namespace UkuiFileMetadata {

class UofExtractorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testContent_data();
    void testNoExtraction();
    void testContent();
    void testMetaDataOnly();
//    void testThumbnail();
};
}

#endif //UKUI_FILE_METADATA_UOFEXTRACTORTEST_H
