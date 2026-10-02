#include <QCryptographicHash>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>
#include <QTest>

#include <type_traits>

#include "file-utils.h"
#include "index/file-content-indexer.h"
#include "index/file-reader.h"
#include "chinese-segmentation.h"

using namespace UkuiSearch;

namespace UkuiSearch {

FileReader *FileReader::getInstance()
{
    static std::aligned_storage<sizeof(FileReader), alignof(FileReader)>::type storage;
    return reinterpret_cast<FileReader *>(&storage);
}

void FileReader::getTextContent(const QString &path, QString &textContent, const QString &suffix)
{
    Q_UNUSED(suffix)
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        textContent = QString::fromUtf8(file.readAll());
    }
}

QImage FileReader::getThumbnail(const QString &path, const QSize &targetSize, qreal dpr)
{
    Q_UNUSED(path)
    Q_UNUSED(targetSize)
    Q_UNUSED(dpr)
    return {};
}

std::string FileUtils::makeDocUterm(const QString &path)
{
    return QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Md5).toHex().toStdString();
}

}

ChineseSegmentation *ChineseSegmentation::getInstance()
{
    static std::aligned_storage<sizeof(ChineseSegmentation), alignof(ChineseSegmentation)>::type storage;
    return reinterpret_cast<ChineseSegmentation *>(&storage);
}

vector<KeyWord> ChineseSegmentation::callSegment(QString &sentence)
{
    vector<KeyWord> terms;
    if (sentence.isEmpty()) {
        return terms;
    }

    KeyWord first;
    first.word = "whitebox";
    first.offsets = {1, 3};
    first.weight = 2;
    terms.push_back(first);

    KeyWord second;
    second.word = "content";
    second.offsets = {5};
    second.weight = 1;
    terms.push_back(second);

    return terms;
}

vector<KeyWord> ChineseSegmentation::callSegment(const string &sentence)
{
    QString value = QString::fromStdString(sentence);
    return callSegment(value);
}

class FileContentIndexerWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void missingFileReturnsFalseAndKeepsDocumentEmpty();
    void emptyReadableFileReturnsFalse();
    void readableTextFileBuildsSearchDocument();
};

void FileContentIndexerWhiteboxTest::missingFileReturnsFalseAndKeepsDocumentEmpty()
{
    fileContentIndexer indexer(QStringLiteral("/tmp/ukui-search-whitebox-no-such-file.txt"));
    QVERIFY(!indexer.index());
    QVERIFY(indexer.document().getUniqueTerm().empty());
    QVERIFY(indexer.document().indexTime().empty());
}

void FileContentIndexerWhiteboxTest::emptyReadableFileReturnsFalse()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QFile file(dir.filePath(QStringLiteral("empty.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    fileContentIndexer indexer(file.fileName());
    QVERIFY(!indexer.index());
    QVERIFY(indexer.document().getUniqueTerm().empty());
}

void FileContentIndexerWhiteboxTest::readableTextFileBuildsSearchDocument()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QFile file(dir.filePath(QStringLiteral("whitebox-content.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("whitebox content for file content indexer");
    file.close();

    fileContentIndexer indexer(file.fileName());
    QVERIFY(indexer.index());

    const Document document = indexer.document();
    QCOMPARE(document.getUniqueTerm(), FileUtils::makeDocUterm(file.fileName()));
    QVERIFY(!document.indexTime().empty());

    const Xapian::Document xapianDocument = document.getXapianDocument();
    QCOMPARE(QString::fromStdString(xapianDocument.get_data()),
             QStringLiteral("whitebox content for file content indexer"));
    QCOMPARE(QString::fromStdString(xapianDocument.get_value(1)), file.fileName());
    QCOMPARE(QString::fromStdString(xapianDocument.get_value(2)), QStringLiteral("txt"));
    QVERIFY(xapianDocument.termlist_count() >= 3);
}

QTEST_MAIN(FileContentIndexerWhiteboxTest)

#include "file-content-indexer-whitebox-test.moc"
