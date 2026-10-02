/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QTextStream>

#include "extractor.h"
#include "extractor-manager.h"
#include "property-info.h"
#include "simple-extraction-result.h"

#include <iostream>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addOption({{"f", "fulltext"}, "Extract full text"});
    parser.addPositionalArgument(QStringLiteral("filename"), QStringLiteral("File to process"));
    parser.process(app);

    if (parser.positionalArguments().size() != 1) {
        qDebug() << "Exactly one argument is accepted";
        parser.showHelp(1);
    }
    bool extractFulltext = parser.isSet("fulltext");

    using UkuiFileMetadata::ExtractionResult;
    const ExtractionResult::Flags extractionLevel = (extractFulltext
        ? ExtractionResult::ExtractMetaData | ExtractionResult::ExtractPlainText
        : ExtractionResult::ExtractMetaData);

    QString url = QFileInfo(parser.positionalArguments().at(0)).absoluteFilePath();

    QMimeDatabase mimeDb;
    QString mimetype = mimeDb.mimeTypeForFile(url).name();

    UkuiFileMetadata::ExtractorManager extractors;
    QList<UkuiFileMetadata::Extractor*> exList = extractors.fetchExtractors(mimetype);

    QTextStream out(stdout);
    out << url << " " << mimetype << "\n";

    for (UkuiFileMetadata::Extractor* ex : exList) {
        out << "\tExtractor For: " << ex->mimetypes().join(QLatin1Char(' ')) << "\n";

        UkuiFileMetadata::SimpleExtractionResult result(url, mimetype, extractionLevel);
        ex->extract(&result);

        const UkuiFileMetadata::PropertyMultiMap multiMap= result.properties();
        UkuiFileMetadata::PropertyMultiMap::const_iterator it = multiMap.constBegin();
        for (; it != multiMap.constEnd(); it++) {
            out << "\t\t" << UkuiFileMetadata::PropertyInfo(it.key()).displayName() << ": "
                << it.value().toString() << " (" << it.value().typeName() << ")\n";
        }
        if (extractFulltext) {
            out << "\t\tText: " << result.text() << "\n";
        }
        out << "\t-------------\n";
    }

    return 0;
}
