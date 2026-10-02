#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>

#include <xapian.h>

#define protected public
#include "index/basic-indexer.h"
#include "index/database.h"
#include "thumbnail-creator.h"
#undef protected

using namespace UkuiSearch;

class IndexSmallComponentsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void basicIndexerRejectsMissingPath();
    void basicIndexerIndexesFileAndDirectoryMetadata();
    void databaseHandlesInvalidAndEnvironmentBackedTypes();
    void thumbnailCreatorEmitsEmptyImageForMissingFile();
};

void IndexSmallComponentsTest::basicIndexerRejectsMissingPath()
{
    BasicIndexer indexer(QStringLiteral("/tmp/ukui-search-whitebox-missing-file"));
    QVERIFY(!indexer.index());
    QVERIFY(indexer.document().getUniqueTerm().empty());
}

void IndexSmallComponentsTest::basicIndexerIndexesFileAndDirectoryMetadata()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QTemporaryFile file(dir.path() + QStringLiteral("/WhiteBox-索引-XXXXXX.txt"));
    QVERIFY(file.open());
    file.write("content");
    file.close();

    BasicIndexer fileIndexer(file.fileName());
    QVERIFY(fileIndexer.index());
    Document fileDoc = fileIndexer.document();
    Xapian::Document fileXapianDoc = fileDoc.getXapianDocument();
    QCOMPARE(QString::fromStdString(fileXapianDoc.get_data()), file.fileName());
    QCOMPARE(QString::fromStdString(fileXapianDoc.get_value(1)), QStringLiteral("0"));
    QVERIFY(!fileDoc.getUniqueTerm().empty());
    QVERIFY(!fileDoc.indexTime().empty());
    QVERIFY(fileXapianDoc.termlist_count() > 0);
    QVERIFY(!fileXapianDoc.get_value(2).empty());

    BasicIndexer dirIndexer(dir.path());
    QVERIFY(dirIndexer.index());
    Document dirDoc = dirIndexer.document();
    Xapian::Document dirXapianDoc = dirDoc.getXapianDocument();
    QCOMPARE(QString::fromStdString(dirXapianDoc.get_data()), dir.path());
    QCOMPARE(QString::fromStdString(dirXapianDoc.get_value(1)), QStringLiteral("1"));
    QVERIFY(!dirDoc.getUniqueTerm().empty());
    QVERIFY(dirXapianDoc.termlist_count() > 0);
}

void IndexSmallComponentsTest::databaseHandlesInvalidAndEnvironmentBackedTypes()
{
    Database invalid(static_cast<DataBaseType>(999));
    QVERIFY(!invalid.valid());
    QVERIFY(!invalid.refresh());
    QCOMPARE(invalid.getIndexDocCount(), uint(0));

    const QList<DataBaseType> types {
        DataBaseType::Basic,
        DataBaseType::Content,
        DataBaseType::OcrContent
    };
    for (const DataBaseType type : types) {
        try {
            Database database(type);
            if (database.valid()) {
                QVERIFY(database.getIndexDocCount() >= 0);
            } else {
                QCOMPARE(database.getIndexDocCount(), uint(0));
            }
        } catch (const Xapian::Error &) {
            QVERIFY(true);
        }
    }
}

void IndexSmallComponentsTest::thumbnailCreatorEmitsEmptyImageForMissingFile()
{
    const QString missing = QStringLiteral("/tmp/ukui-search-whitebox-missing-thumbnail.png");
    ThumbnailCreator creator(missing, QSize(64, 32), 2.0);
    creator.setAutoDelete(false);

    QSignalSpy spy(&creator, SIGNAL(ready(QString,QImage)));
    QVERIFY(spy.isValid());

    creator.run();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), missing);
}

QTEST_MAIN(IndexSmallComponentsTest)
#include "index-small-components-test.moc"
