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
#include "office-extractor.h"
#include "binary-parser.h"
#include <QFile>

using namespace UkuiFileMetadata;

const QStringList supportedMimeTypes = {
    // Ambiguous legacy Office MIME aliases from different desktops/mime databases.
    // Keep them mapped here so OLE-backed files still reach the legacy extractor.
    QStringLiteral("application/msword"),
    QStringLiteral("application/vnd.ms-word"),
    QStringLiteral("application/x-msword"),
    QStringLiteral("application/vnd.ms-excel"),
    QStringLiteral("application/msexcel"),
    QStringLiteral("application/x-msexcel"),
    QStringLiteral("application/vnd.ms-powerpoint"),
    QStringLiteral("application/powerpoint"),
    QStringLiteral("application/mspowerpoint"),
    QStringLiteral("application/x-mspowerpoint"),
    QStringLiteral("application/wps-office.doc"),    //2003word
    QStringLiteral("application/wps-office.dot"),
    QStringLiteral("application/wps-office.wps"),
    QStringLiteral("application/wps-office.et"),     //2003excel
    QStringLiteral("application/wps-office.xls"),
    QStringLiteral("application/wps-office.dps"),    //2003powerpoint
    QStringLiteral("application/wps-office.ppt"),
    QStringLiteral("application/x-ole-storage"),     //OLE file,
    //file ends with pps&ppt is identifier as ole file, but ole file has too many sub types, need a filter
};

static bool isOleStorageFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "OLE check failed! Cannot open file: " << path;
        return false;
    }
    const QByteArray header = file.read(8);
    if (header.size() != 8) {
        qWarning() << "OLE check failed! Invalid Header size of" << path;
        return false;
    }
    return header == QByteArray::fromHex("D0CF11E0A1B11AE1");
}

OfficeExtractor::OfficeExtractor(QObject *parent) : ExtractorPlugin(parent)
{

}

void OfficeExtractor::extract(ExtractionResult *result)
{
    QFileInfo info(result->inputUrl());
    QString suffix = info.suffix();
    if (suffix == "doc" || suffix == "dot" || suffix == "wps" || suffix == "ppt" ||
            suffix == "pps" || suffix == "dps" || suffix == "et" || suffix == "xls") {
        if (!isOleStorageFile(result->inputUrl())) {
            qDebug() << "Skip non-OLE file for office extractor:" << result->inputUrl();
            return;
        }
        result->addType(Type::Document);
        if (result->inputFlags() & ExtractionResult::ExtractPlainText) {
            QString contents;
            KBinaryParser parser;
            parser.RunParser(result->inputUrl(), contents);
            result->append(contents);
        }
    } else {
        qDebug() << "The parsing of the type:" << result->inputMimetype() << " is not fully supported currently";
    }
}

QStringList OfficeExtractor::mimetypes() const
{
    return supportedMimeTypes;
}
