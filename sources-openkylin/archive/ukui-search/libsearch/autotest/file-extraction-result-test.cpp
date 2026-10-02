#include "file-extraction-result-test.h"

#include <QTest>

#include "file-extraction-result.h"

using namespace UkuiSearch;

void FileExtractionResultTest::testAddAppendAndTypes()
{
    FileExtractionResult result(QStringLiteral("file:///tmp/demo.txt"), QStringLiteral("text/plain"));

    result.add(UkuiFileMetadata::Property::Title, QStringLiteral("Demo"));
    result.add(UkuiFileMetadata::Property::Author, QStringLiteral("Tester"));
    result.addType(UkuiFileMetadata::Type::Document);
    result.addType(UkuiFileMetadata::Type::Image);
    result.append(QStringLiteral("first line\nsecond\rline"));

    QCOMPARE(result.properties().value(UkuiFileMetadata::Property::Title).toString(), QStringLiteral("Demo"));
    QCOMPARE(result.properties().value(UkuiFileMetadata::Property::Author).toString(), QStringLiteral("Tester"));
    QCOMPARE(result.types().size(), 2);
    QCOMPARE(result.types().at(0), UkuiFileMetadata::Type::Document);
    QCOMPARE(result.types().at(1), UkuiFileMetadata::Type::Image);
    QCOMPARE(result.text(), QStringLiteral("first linesecond line"));
}

void FileExtractionResultTest::testAssignmentCopiesStoredData()
{
    FileExtractionResult source(QStringLiteral("file:///tmp/source.txt"));
    source.add(UkuiFileMetadata::Property::Comment, QStringLiteral("comment"));
    source.addType(UkuiFileMetadata::Type::Archive);
    source.append(QStringLiteral("payload"));

    FileExtractionResult assigned(QStringLiteral("file:///tmp/assigned.txt"));
    assigned = source;

    QCOMPARE(assigned.properties().value(UkuiFileMetadata::Property::Comment).toString(), QStringLiteral("comment"));
    QCOMPARE(assigned.types().at(0), UkuiFileMetadata::Type::Archive);
    QCOMPARE(assigned.text(), QStringLiteral("payload"));
}

QTEST_MAIN(FileExtractionResultTest)
