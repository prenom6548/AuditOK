/*
*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 */

#include "bookmarks-extractor.h"
#include "bookmarks-manager.h"

using namespace UkuiFileMetadata;

const QStringList supportedMimeTypes = {
    QStringLiteral("application/x-xbel")
};

BookMarksExtractor::BookMarksExtractor(QObject* parent) : ExtractorPlugin(parent)
{

}

void BookMarksExtractor::extract(ExtractionResult* result)
{
    if (!supportedMimeTypes.contains(result->inputMimetype())) {
        return;
    }

    result->addType(Type::Text);

    if (!(result->inputFlags() & (ExtractionResult::ExtractMetaData | ExtractionResult::ExtractPlainText))) {
        return;
    }

    const BookMarksManager manager(result->inputUrl());
    if (!manager.isValid()) {
        return;
    }

    if (result->inputFlags() & ExtractionResult::ExtractMetaData) {
        if (!manager.rootTitle().isEmpty()) {
            result->add(Property::Title, manager.rootTitle());
        }
    }

    if (result->inputFlags() & ExtractionResult::ExtractPlainText) {
        const QStringList lines = manager.textLines();
        if (!lines.isEmpty()) {
            result->append(lines.join(QLatin1Char('\n')));
        }
    }
}

QStringList BookMarksExtractor::mimetypes() const
{
    return supportedMimeTypes;
}
