/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * thi
 */

#include "parserextension.h"

#include <ukui-file-metadata/extractor-manager.h>
#include <ukui-file-metadata/extractor.h>
#include <ukui-file-metadata/mime-utils.h>
#include <ukui-file-metadata/property-info.h>
#include <ukui-file-metadata/simple-extraction-result.h>

#include <QDebug>
#include <QString>
#include <iostream>

std::string ParserExtension::parserOffice(const std::string& filepath) {
    QString contents;
    QString url = QString::fromStdString(filepath);
    QString mimetype = UkuiFileMetadata::MimeUtils::strictMimeType(url, {}).name();
    qDebug() << url << " " << mimetype << "\n";
    // QString mimetype = "application/x-ole-storage";
    using UkuiFileMetadata::ExtractionResult;
    const ExtractionResult::Flags extractionLevel = (ExtractionResult::ExtractPlainText);

    UkuiFileMetadata::ExtractorManager extractors;
    QList<UkuiFileMetadata::Extractor*> exList = extractors.fetchExtractors(mimetype);

    for (UkuiFileMetadata::Extractor* ex : exList) {
        UkuiFileMetadata::SimpleExtractionResult result(url, mimetype, extractionLevel);
        ex->extract(&result);
        contents += result.text();
    }

    return contents.toStdString();
}