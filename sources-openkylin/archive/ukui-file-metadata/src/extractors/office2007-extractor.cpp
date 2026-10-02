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
#include "office2007-extractor.h"
#include "thumbnail-utils.h"
#include "zip-reader.h"

#include <QFileInfo>
#include <QImage>
#include <QRegularExpression>
#include <QXmlStreamReader>

using namespace UkuiFileMetadata;

static const QString VERSION = "1.0";
static const QString PLUGIN_NAME = "Office2007";

namespace {

bool appendOfficeXmlText(ZipReader &archive, const QString &entryName, ExtractionResult *result)
{
    return archive.processEntry(entryName,
                                [result](QIODevice *device) {
                                    QXmlStreamReader reader(device);
                                    while (!reader.atEnd()) {
                                        if (reader.readNextStartElement() && reader.name().toString() == QLatin1String("t")) {
                                            result->append(reader.readElementText());
                                        }
                                    }
                                    return true;
                                },
                                Qt::CaseSensitive);
}

}

const QStringList supportedMimeTypes = {
    // Ambiguous legacy Office MIME aliases can still point to OOXML content
    // when the filename keeps an old suffix such as .doc/.xls/.ppt.
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
    QStringLiteral("application/wps-office.doc"),
    QStringLiteral("application/wps-office.xls"),
    QStringLiteral("application/wps-office.ppt"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.template"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideshow"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.template"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.template"),
};

Office2007Extractor::Office2007Extractor(QObject *parent) : ExtractorPlugin(parent)
{

}

void Office2007Extractor::extract(ExtractionResult *result)
{
    QFileInfo info = QFileInfo(result->inputUrl());
    if (!info.exists() || info.isDir()) {
        return;
    }

    ZipReader archive(result->inputUrl());
    if (!archive.open()) {
        return;
    }

    if (result->inputFlags() & ExtractionResult::ExtractThumbnail) {
        QString thumbnailPath;
        QByteArray relsData;
        const bool needExtract = ThumbnailUtils::needGenerateThumbnail(result, PLUGIN_NAME, VERSION) &&
                                 archive.readEntry(QStringLiteral("_rels/.rels"), &relsData, Qt::CaseSensitive);
        if (needExtract) {
            QXmlStreamReader reader(relsData);
            while (!reader.atEnd()) {
                if (reader.readNextStartElement() && reader.name().toString() == QLatin1String("Relationship")) {
                    const auto attributes = reader.attributes();
                    if (attributes.value(QStringLiteral("Type")) == QLatin1String("http://schemas.openxmlformats.org/package/2006/relationships/metadata/thumbnail")) {
                        thumbnailPath = attributes.value(QStringLiteral("Target")).toString();
                        break;
                    }
                }
            }
        }

        if (!thumbnailPath.isEmpty()) {
            QByteArray thumbnailData;
            if (archive.readEntry(thumbnailPath, &thumbnailData, Qt::CaseSensitive)) {
                QImage thumbnail;
                thumbnail.loadFromData(thumbnailData);
                ThumbnailUtils::setThumbnail(result, thumbnail, PLUGIN_NAME, VERSION);
            }
        }
    }

    if (result->inputFlags() & ExtractionResult::ExtractMetaData) {
        QByteArray xmlData;
        if (archive.readEntry(QStringLiteral("docProps/core.xml"), &xmlData, Qt::CaseSensitive)) {
            QXmlStreamReader reader(xmlData);
            while (!reader.atEnd()) {
               if (reader.readNextStartElement()) {
                  if (reader.name().toString() == "description") {
                      QString desc = reader.readElementText();
                      if (!desc.isEmpty()) {
                          result->add(Property::Description, desc);
                      }
                      continue;
                  } else if (reader.name().toString() == "subject") {
                      QString subject = reader.readElementText();
                      if (!subject.isEmpty()) {
                          result->add(Property::Subject, subject);
                      }
                      continue;
                  } else if (reader.name().toString() == "title") {
                      QString title = reader.readElementText();
                      if (!title.isEmpty()) {
                          result->add(Property::Title, title);
                      }
                      continue;
                  } else if (reader.name().toString() == "creator") {
                      QString creator = reader.readElementText();
                      if (!creator.isEmpty()) {
                          result->add(Property::Author, creator);
                      }
                      continue;
                  } else if (reader.name().toString() == "language") {
                      QString language = reader.readElementText();
                      if (!language.isEmpty()) {
                          result->add(Property::Language, language);
                      }
                      continue;
                  } else if (reader.name().toString() == "created") {
                      QString creationDate = reader.readElementText();
                      if (!creationDate.isEmpty()) {
                          result->add(Property::CreationDate, creationDate);
                      }
                      continue;
                  } else if (reader.name().toString() == "keywords") {
                      QString keywords = reader.readElementText();
                      if (!keywords.isEmpty()) {
                          result->add(Property::Keywords, keywords);
                      }
                      continue;
                  }
               }
            }
        }

        if (archive.readEntry(QStringLiteral("docProps/app.xml"), &xmlData, Qt::CaseSensitive)) {
            QXmlStreamReader reader(xmlData);
            while (!reader.atEnd()) {
                if (reader.readNextStartElement()) {
                    if (this->getSupportedMimeType(result->inputMimetype()) ==
                            QLatin1String("application/vnd.openxmlformats-officedocument.wordprocessingml.document")) {
                        if (reader.name().toString() == "Pages") {
                            bool ok = false;
                            int pageCount = reader.readElementText().toInt(&ok);
                            if (ok) {
                                result->add(Property::PageCount, pageCount);
                            }
                        } else if (reader.name().toString() == "Words") {
                            bool ok = false;
                            int wordCount = reader.readElementText().toInt(&ok);
                            if (ok) {
                                result->add(Property::WordCount, wordCount);
                            }
                        }
                    }

                    if (reader.name().toString() == "Application") {
                        QString generator = reader.readElementText();
                        if (!generator.isEmpty()) {
                            result->add(Property::Generator, generator);
                        }
                    }
                }
            }
        }
    }

    //extract document content
    if (result->inputFlags() & ExtractionResult::ExtractPlainText) {
        //word
        if (appendOfficeXmlText(archive, QStringLiteral("word/document.xml"), result)) {
            result->addType(Type::Document);
        //excel
        } else if (appendOfficeXmlText(archive, QStringLiteral("xl/sharedStrings.xml"), result)) {
            result->addType(Type::Document);
            result->addType(Type::Spreadsheet);
        } else {
            //powerpoint
            QStringList slideXmlList;
            const QRegularExpression slidePattern(QStringLiteral("^ppt/slides/slide[^/]*$"));
            for (const QString &slideFile : archive.entryNames()) {
                if (slidePattern.match(slideFile).hasMatch()) {
                    slideXmlList << slideFile;
                }
            }
            if (!slideXmlList.isEmpty()) {
                result->addType(Type::Document);
                result->addType(Type::Presentation);
                for (const QString &slideXmlFile : slideXmlList) {
                    appendOfficeXmlText(archive, slideXmlFile, result);
                }
            }
        }
    }
}

QStringList Office2007Extractor::mimetypes() const
{
    return supportedMimeTypes;
}
