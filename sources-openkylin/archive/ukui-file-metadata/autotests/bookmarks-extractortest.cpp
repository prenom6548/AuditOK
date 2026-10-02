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
#include "bookmarks-extractortest.h"
#include "extractors/bookmarks-extractor.h"
#include "indexerextractortestsconfig.h"
#include "mime-utils.h"
#include "simple-extraction-result.h"

#include <QTest>

using namespace UkuiFileMetadata;

QString testFilePath(const QString& name)
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH) + QLatin1String("/bookmarks/") + name;
}

static QStringList expectedContainsLines()
{
    QStringList lines;
    lines << QStringLiteral("dbusName: org.example.bookmarks");
    lines << QStringLiteral("file:///home/user/Documents");
    lines << QStringLiteral("https://example.com");
    lines << QStringLiteral("showintoolbar: yes");
    lines << QStringLiteral("folded: no");
    lines << QStringLiteral("toolbar: yes");
    lines << QStringLiteral("Docs");
    lines << QStringLiteral("Favorites");
    lines << QStringLiteral("Example");
    lines << QStringLiteral("Documents folder");
    lines << QStringLiteral("Example site");
    lines << QStringLiteral("icon: folder-documents");
    lines << QStringLiteral("mime-type: inode/directory");
    lines << QStringLiteral("time_added: 1234567890");
    lines << QStringLiteral("visit_count: 5");
    lines << QStringLiteral("metadata[https://example.org/bookmark-ext][file:///home/user/Documents].rating: favorite");
    lines << QStringLiteral("metadata[https://example.org/bookmark-ext][file:///home/user/Documents].rating@score: 5");
    lines << QStringLiteral("metadata[https://example.org/bookmark-ext][file:///home/user/Documents].flags.public: true");
    lines << QStringLiteral("metadata[http://freedesktop.org][file:///home/user/Documents].groups.group: pluma");
    lines << QStringLiteral("metadata[http://freedesktop.org][file:///home/user/Documents].applications.application@name: Firefox");
    lines << QStringLiteral("metadata[http://freedesktop.org][file:///home/user/Documents].applications.application@exec: 'firefox-esr %u'");
    lines << QStringLiteral("metadata[http://freedesktop.org][file:///home/user/Documents].applications.application@modified: 2026-02-06T09:50:28.485017Z");
    lines << QStringLiteral("metadata[http://freedesktop.org][file:///home/user/Documents].applications.application@count: 1");
    lines << QStringLiteral("metadata[http://freedesktop.org][file:///home/user/Documents].applications.application@name: Pluma");
    lines << QStringLiteral("metadata[http://freedesktop.org][file:///home/user/Documents].applications.application@exec: 'pluma %u'");
    lines << QStringLiteral("metadata[http://freedesktop.org][file:///home/user/Documents].applications.application@modified: 2026-02-06T09:50:33.347434Z");
    return lines;
}

static bool containsAny(const QStringList &parsedLines, const QStringList &candidates)
{
    for (const QString &candidate : candidates) {
        if (parsedLines.contains(candidate)) {
            return true;
        }
    }
    return false;
}

static void assertParsedTextLines(const QStringList &parsedLines)
{
    for (const QString &line : expectedContainsLines()) {
        QVERIFY2(parsedLines.contains(line), qPrintable(QStringLiteral("Missing extracted line: %1").arg(line)));
    }
    QVERIFY(containsAny(parsedLines, {QStringLiteral("attribute[xbel].app:profile: work"),
                                      QStringLiteral("attribute[xbel].profile: work")}));
    QVERIFY(containsAny(parsedLines, {QStringLiteral("attribute[bookmark].app:priority: high"),
                                      QStringLiteral("attribute[bookmark].priority: high")}));
    QVERIFY(containsAny(parsedLines, {QStringLiteral("attribute[folder].app:color: blue"),
                                      QStringLiteral("attribute[folder].color: blue")}));
    QVERIFY(containsAny(parsedLines, {QStringLiteral("attribute[alias].app:aliasType: quicklink"),
                                      QStringLiteral("attribute[alias].aliasType: quicklink")}));
    QVERIFY(containsAny(parsedLines, {QStringLiteral("attribute[separator].app:marker: main"),
                                      QStringLiteral("attribute[separator].marker: main")}));
}

void BookMarksExtractorTest::testMetaDataOnly()
{
    BookMarksExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test_bookmark.xbel"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractMetaData);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Text);
    QCOMPARE(result.text(), QString());
    QCOMPARE(result.properties().value(Property::Title), QVariant(QStringLiteral("Root Bookmarks")));
}

void BookMarksExtractorTest::testPlainText()
{
    BookMarksExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test_bookmark.xbel"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType, ExtractionResult::ExtractPlainText);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Text);
    const QStringList parsedLines = result.text().trimmed().split(QLatin1Char('\n'));
    assertParsedTextLines(parsedLines);
    QVERIFY(result.properties().isEmpty());
}

void BookMarksExtractorTest::testAllData()
{
    BookMarksExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test_bookmark.xbel"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Text);
    const QStringList parsedLines = result.text().trimmed().split(QLatin1Char('\n'));
    assertParsedTextLines(parsedLines);
    QCOMPARE(result.properties().value(Property::Title), QVariant(QStringLiteral("Root Bookmarks")));
}

QTEST_GUILESS_MAIN(BookMarksExtractorTest)
