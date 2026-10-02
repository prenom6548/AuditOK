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

#include "ofd-extractor.h"
#include "zip-reader.h"
#include <algorithm>
#include <QStringList>
#include <QFileInfo>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QDebug>

#define MAX_CONTENT_LENGTH 20480000

using namespace UkuiFileMetadata;

const QStringList supportedMimeTypes = {
        QStringLiteral("application/ofd"),
        QStringLiteral("application/zip")
};

OfdExtractor::OfdExtractor(QObject *parent) : ExtractorPlugin(parent) {

}

/**
 * 参考标准 GB/T33190-2016
 * 参考文档 https://www.doc88.com/p-5354924116081.html
 */
void OfdExtractor::extract(ExtractionResult *result) {
    QFileInfo info(result->inputUrl());
    if (!info.exists() || info.isDir()) {
        return;
    }

    if (info.suffix() != "ofd") {
        return;
    }

    ZipReader archive(result->inputUrl());
    if (!archive.open()) {
        return;
    }

    result->addType(Type::Document);

    if (result->inputFlags() & ExtractionResult::Flag::ExtractMetaData) {
        QByteArray xmlData;
        if (archive.readEntry(QStringLiteral("OFD.xml"), &xmlData)) {
            QXmlStreamReader reader(xmlData);
            QStringList keywords;
            while (!reader.atEnd()) {
                if (reader.readNextStartElement()) {
                    if (reader.name().toString() == "Title") {
                        result->add(Property::Title, reader.readElementText());
                        continue;
                    }
                    if (reader.name().toString() == "Author") {
                        result->add(Property::Author, reader.readElementText());
                        continue;
                    }
                    if (reader.name().toString() == "Subject") {
                        result->add(Property::Subject, reader.readElementText());
                        continue;
                    }
                    if (reader.name().toString() == "Abstract") {
                        result->add(Property::Description, reader.readElementText());
                        continue;
                    }
                    if (reader.name().toString() == "CreationDate") {
                        result->add(Property::CreationDate, reader.readElementText());
                        continue;
                    }
                    if (reader.name().toString() == "Creator") {
                        result->add(Property::Generator, reader.readElementText());
                        continue;
                    }

                    if (reader.name().toString() == "Keyword") {
                        keywords.append(reader.readElementText());
                    }
                }
            }

            if (!keywords.isEmpty()) {
                result->add(Property::Keywords, keywords);
            }
        }
    }

    if (!(result->inputFlags() & ExtractionResult::Flag::ExtractPlainText)) {
        return;
    }

    // GB/T 33190-2016允许多个 Doc_x 目录，这里保持现有行为，仅处理 Doc_0。
    const QRegularExpression pagePattern(QStringLiteral("^Doc_0/Pages/Page_(\\d+)/Content\\.xml$"));
    QStringList pageEntries;
    for (const auto &file : archive.entryNames()) {
        if (pagePattern.match(file).hasMatch()) {
            pageEntries << file;
        }
    }
    std::sort(pageEntries.begin(), pageEntries.end(), [&pagePattern](const QString &left, const QString &right) {
        const auto leftMatch = pagePattern.match(left);
        const auto rightMatch = pagePattern.match(right);
        return leftMatch.captured(1).toInt() < rightMatch.captured(1).toInt();
    });

    QString textContent;
    for (const QString &filename : pageEntries) {
        bool limitReached = false;
        if (!archive.processEntry(filename,
                                  [&textContent, &limitReached](QIODevice *device) {
                                      QXmlStreamReader reader(device);
                                      while (!reader.atEnd() && !reader.hasError()) {
                                          if (!reader.readNextStartElement()) {
                                              if (reader.hasError()) {
                                                  qWarning() << reader.errorString() << reader.error() << "line:" << reader.lineNumber() << "column:" << reader.columnNumber();
                                                  break;
                                              }
                                          }

                                          if (reader.name().toString() == "TextCode") {
                                              textContent.append(reader.readElementText());
                                              if (textContent.length() >= MAX_CONTENT_LENGTH / 3) {
                                                  limitReached = true;
                                                  break;
                                              }
                                          }
                                      }
                                      return true;
                                  })) {
            continue;
        }
        if (limitReached) {
            result->append(textContent);
            return;
        }
    }
    result->append(textContent);
}

QStringList OfdExtractor::mimetypes() const {
    return supportedMimeTypes;
}
