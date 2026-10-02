#include <QTemporaryDir>
#include <QTest>

#include <fstream>

#include "Traditional2Simplified_trie.h"

using namespace std;

namespace {
QString writeTraditionalDict(const QString &dirPath)
{
    const QString path = dirPath + QStringLiteral("/traditional.txt");
    ofstream out(path.toStdString());
    out << "# comment\n";
    out << "繁:繁\n";
    out << "體:体\n";
    out << "龍:龙\n";
    out << "門:门\n";
    out << "錯誤格式\n";
    out.close();
    return path;
}
}

class Traditional2SimplifiedTrieTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void loadAndQueryDictionary();
    void defaultCachePathStillLoads();
};

void Traditional2SimplifiedTrieTest::loadAndQueryDictionary()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString dictPath = writeTraditionalDict(tempDir.path());
    const QString cachePath = tempDir.path() + QStringLiteral("/traditional-cache.dat");

    Traditional2SimplifiedTrie trie({dictPath.toStdString()}, cachePath.toStdString());

    QCOMPARE(trie.Find(string("體")), string("体"));
    QCOMPARE(trie.Find(string("龍")), string("龙"));
    QCOMPARE(trie.Find(string("不存在")), string());
    QVERIFY(trie.IsTraditional(string("門")));
    QVERIFY(!trie.IsTraditional(string("简")));
}

void Traditional2SimplifiedTrieTest::defaultCachePathStillLoads()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString dictPath = writeTraditionalDict(tempDir.path());
    Traditional2SimplifiedTrie trie({dictPath.toStdString()});

    QCOMPARE(trie.Find(string("體")), string("体"));
    QCOMPARE(trie.Find(string("門")), string("门"));
}

QTEST_MAIN(Traditional2SimplifiedTrieTest)

#include "traditional2simplified-trie-test.moc"
