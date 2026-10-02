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

#include "zip-readertest.h"
#include "indexerextractortestsconfig.h"
#include "zip-reader.h"

#include <QIODevice>
#include <QTest>
#include <type_traits>

using namespace UkuiFileMetadata;

static_assert(!std::is_copy_constructible<ZipReader>::value,
              "ZipReader must not be copy constructible");
static_assert(!std::is_copy_assignable<ZipReader>::value,
              "ZipReader must not be copy assignable");

namespace {

QString testFilePath(const QString &baseName, const QString &extension)
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH)
           + QLatin1Char('/')
           + baseName
           + QLatin1Char('.')
           + extension;
}

class ScopedEnvironmentValue
{
public:
    ScopedEnvironmentValue(const char *name, const QByteArray &value)
        : variableName(name)
        , hadValue(qEnvironmentVariableIsSet(name))
        , previousValue(qgetenv(name))
    {
        qputenv(variableName.constData(), value);
    }

    ~ScopedEnvironmentValue()
    {
        if (hadValue) {
            qputenv(variableName.constData(), previousValue);
        } else {
            qunsetenv(variableName.constData());
        }
    }

private:
    QByteArray variableName;
    bool hadValue = false;
    QByteArray previousValue;
};

}

void ZipReaderTest::testReadEntryReturnsFullContent()
{
    ZipReader reader(testFilePath(QStringLiteral("test_libreoffice"), QStringLiteral("docx")));
    QVERIFY(reader.open());

    QByteArray data;
    QVERIFY(reader.readEntry(QStringLiteral("word/document.xml"), &data, Qt::CaseSensitive));
    QVERIFY(!data.isEmpty());
    QVERIFY(data.contains("KFileMetaData"));
}

void ZipReaderTest::testProcessEntryCanStopEarly()
{
    ZipReader reader(testFilePath(QStringLiteral("test_libreoffice"), QStringLiteral("docx")));
    QVERIFY(reader.open());

    QByteArray prefix;
    const bool ok = reader.processEntry(QStringLiteral("word/document.xml"),
                                        [&prefix](QIODevice *device) {
                                            prefix = device->read(32);
                                            return prefix.size() == 32;
                                        },
                                        Qt::CaseSensitive);

    QVERIFY(ok);
    QCOMPARE(prefix.size(), 32);
}

void ZipReaderTest::testCp437EntryNamesUseZipDefaultCodec()
{
    const QString expectedEntryName = QStringLiteral("ü@.txt");
    ZipReader reader(testFilePath(QStringLiteral("test_zipreader_cp437_default_names"), QStringLiteral("zip")));
    QVERIFY(reader.open());

    QCOMPARE(reader.entryNames(), QStringList({expectedEntryName}));

    QByteArray data;
    QVERIFY(reader.readEntry(expectedEntryName, &data));
    QCOMPARE(data, QByteArray("cp437-default-content"));
}

void ZipReaderTest::testDefaultFallbackCodecsDoNotDependOnLocaleLanguage()
{
    ScopedEnvironmentValue lcAll("LC_ALL", QByteArrayLiteral("C.UTF-8"));
    ScopedEnvironmentValue lang("LANG", QByteArrayLiteral("zh_CN.UTF-8"));

    ZipReader reader(testFilePath(QStringLiteral("test_zipreader_cp437_default_names"), QStringLiteral("zip")));
    QCOMPARE(reader.fallbackFileNameCodecs(), QList<QByteArray>({QByteArrayLiteral("CP437")}));
}

void ZipReaderTest::testUnicodePathExtraFieldTakesPrecedence()
{
    const QString expectedEntryName = QStringLiteral("目录/测试.txt");
    ZipReader reader(testFilePath(QStringLiteral("test_zipreader_unicode_path_extra"), QStringLiteral("zip")));
    reader.setFallbackFileNameCodecs({QByteArrayLiteral("CP437")});
    QVERIFY(reader.open());

    QCOMPARE(reader.entryNames(), QStringList({expectedEntryName}));

    QByteArray data;
    QVERIFY(reader.readEntry(expectedEntryName, &data));
    QCOMPARE(data, QByteArray("unicode-extra-content"));
}

void ZipReaderTest::testInvalidUnicodePathExtraFieldFallsBackToConfiguredCodec()
{
    const QString expectedEntryName = QStringLiteral("目录/测试.txt");
    ZipReader reader(testFilePath(QStringLiteral("test_zipreader_unicode_path_extra_bad_crc"), QStringLiteral("zip")));
    reader.setFallbackFileNameCodecs({QByteArrayLiteral("GB18030")});
    QVERIFY(reader.open());

    QCOMPARE(reader.entryNames(), QStringList({expectedEntryName}));

    QByteArray data;
    QVERIFY(reader.readEntry(expectedEntryName, &data));
    QCOMPARE(data, QByteArray("unicode-extra-bad-crc-content"));
}

void ZipReaderTest::testUtf8EntryNamesRoundTrip()
{
    const QString expectedEntryName = QStringLiteral("目录/测试.txt");
    ZipReader reader(testFilePath(QStringLiteral("test_zipreader_utf8_names"), QStringLiteral("zip")));
    QVERIFY(reader.open());

    QCOMPARE(reader.entryNames(), QStringList({expectedEntryName}));

    QByteArray data;
    QVERIFY(reader.readEntry(expectedEntryName, &data));
    QCOMPARE(data, QByteArray("utf8-content"));
}

void ZipReaderTest::testLegacyEntryNamesUseFallbackCodecs()
{
    const QString expectedEntryName = QStringLiteral("目录/测试.txt");
    ZipReader reader(testFilePath(QStringLiteral("test_zipreader_gbk_names"), QStringLiteral("zip")));
    reader.setFallbackFileNameCodecs({QByteArrayLiteral("GB18030")});
    QVERIFY(reader.open());

    QCOMPARE(reader.entryNames(), QStringList({expectedEntryName}));

    QByteArray data;
    QVERIFY(reader.readEntry(expectedEntryName, &data));
    QCOMPARE(data, QByteArray("gbk-content"));
}

QTEST_GUILESS_MAIN(ZipReaderTest)
