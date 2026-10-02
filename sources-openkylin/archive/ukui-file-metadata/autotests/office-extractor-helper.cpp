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

#include "extractors/office-extractor.h"
#include "mime-utils.h"
#include "simple-extraction-result.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QMimeDatabase>

#include <cstdio>

using namespace UkuiFileMetadata;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s <file>\n", argv[0]);
        return 2;
    }

    const QString fileName = QFileInfo(QString::fromLocal8Bit(argv[1])).absoluteFilePath();
    QMimeDatabase mimeDb;
    const QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();

    OfficeExtractor plugin;
    SimpleExtractionResult result(fileName, mimeType,
                                  ExtractionResult::ExtractPlainText | ExtractionResult::ExtractMetaData);
    plugin.extract(&result);

    const QByteArray utf8 = result.text().toUtf8();
    const QByteArray sha256 =
        QCryptographicHash::hash(
            utf8,
            QCryptographicHash::Sha256).toHex();

    std::printf("mime=%s\ntext_len=%d\ntext_sha256=%s\n",
                mimeType.toLocal8Bit().constData(),
                result.text().size(),
                sha256.constData());
    return 0;
}
