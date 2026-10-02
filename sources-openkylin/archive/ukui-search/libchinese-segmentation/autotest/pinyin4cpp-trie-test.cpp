#include <QTemporaryDir>
#include <QTest>

#include <fstream>
#include <map>

#include "pinyin4cpp-trie.h"
#include "pinyin4cpp_dataTrie.h"

using namespace std;

namespace {
QString writeSingleWordDict(const QString &dirPath)
{
    const QString path = dirPath + QStringLiteral("/single.txt");
    ofstream out(path.toStdString());
    out << "# comment\n";
    out << "1:zhong:中\n";
    out << "2:zhong:中\n";
    out << "3:guo:国\n";
    out << "bad line\n";
    out.close();
    return path;
}

QString writeWordsDict(const QString &dirPath)
{
    const QString path = dirPath + QStringLiteral("/words.txt");
    ofstream out(path.toStdString());
    out << "# comment\n";
    out << "中国:zhōng,guó\n";
    out << "重庆:chóng,qìng\n";
    out << "bad line\n";
    out.close();
    return path;
}
}

class Pinyin4cppTrieTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void dataTrieBuildAndAttachRoundTrip();
    void trieLoadsFromCustomSourceFiles();
    void trieReusesCacheAndSupportsPrefixSearch();
    void trieRebuildsInvalidCache();
    void defaultTrieUsesInstalledDictionary();
};

void Pinyin4cppTrieTest::dataTrieBuildAndAttachRoundTrip()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    map<string, string> elements;
    elements["中"] = "zhong";
    elements["中国"] = "zhong,guo";

    const QString cachePath = tempDir.path() + QStringLiteral("/data-trie-cache.dat");
    Pinyin4cppDataTrie trie;

    QVERIFY(trie.InitBuildDat(elements, cachePath.toStdString(), string(32, 'a')));
    QCOMPARE(trie.Find(string("中")), string("zhong"));
    QCOMPARE(trie.Find(string("中国")), string("zhong,guo"));
    QCOMPARE(trie.Find(string("未知")), string());

    Pinyin4cppDataTrie attachedTrie;
    QVERIFY(attachedTrie.InitAttachDat(cachePath.toStdString(), string(32, 'a')));
    QCOMPARE(attachedTrie.Find(string("中")), string("zhong"));
    QVERIFY(!attachedTrie.InitAttachDat(cachePath.toStdString(), string(32, 'b')));
}

void Pinyin4cppTrieTest::trieLoadsFromCustomSourceFiles()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString singlePath = writeSingleWordDict(tempDir.path());
    const QString wordsPath = writeWordsDict(tempDir.path());
    const QString cachePath = tempDir.path() + QStringLiteral("/pinyin-trie-cache.dat");

    Pinyin4cppTrie trie({singlePath.toStdString(), wordsPath.toStdString()}, cachePath.toStdString());

    string oneWord = "中";
    string missingWord = "不存在";
    QCOMPARE(trie.Find(oneWord), string("zhōng,zhòng"));
    QCOMPARE(trie.Find(string("中国")), string("zhōng,guó"));
    QCOMPARE(trie.Find(string("重庆")), string("chóng,qìng"));
    QVERIFY(trie.Contains(oneWord));
    QVERIFY(!trie.Contains(missingWord));
    QVERIFY(!trie.IsMultiTone(string("中国")));
    QVERIFY(!trie.IsMultiTone(string("中")));
    QVERIFY(trie.IsMultiTone(string("不存在")));
}

void Pinyin4cppTrieTest::trieReusesCacheAndSupportsPrefixSearch()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString singlePath = writeSingleWordDict(tempDir.path());
    const QString wordsPath = writeWordsDict(tempDir.path());
    const QString cachePath = tempDir.path() + QStringLiteral("/pinyin-trie-cache.dat");
    const vector<string> sources{singlePath.toStdString(), wordsPath.toStdString()};

    {
        Pinyin4cppTrie trie(sources, cachePath.toStdString());
        QCOMPARE(trie.Find(string("中国")), string("zhōng,guó"));
        QVERIFY(QFileInfo(cachePath + QStringLiteral(VERSION)).exists());
    }

    {
        Pinyin4cppTrie trie(sources, cachePath.toStdString());
        QCOMPARE(trie.Find(string("重庆")), string("chóng,qìng"));

        Pinyin4cppTrie::result_pair_type results[4];
        const size_t count = trie.CommonPrefixSearch("中国人", results, 4);
        QVERIFY(count >= 1);
        QVERIFY(results[0].value >= 0);
        QVERIFY(trie.GetDataTrieArray() != nullptr);
        QVERIFY(trie.GetDataTrieSize() > 0);
        QVERIFY(trie.GetDataTrieTotalSize() > 0);
        QVERIFY(trie.GetCacheFileHeaderPtr() != nullptr);
        QVERIFY(trie.GetElementPtr() != nullptr);
    }
}

void Pinyin4cppTrieTest::trieRebuildsInvalidCache()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString singlePath = writeSingleWordDict(tempDir.path());
    const QString wordsPath = writeWordsDict(tempDir.path());
    const QString cachePath = tempDir.path() + QStringLiteral("/pinyin-trie-cache.dat");
    const QString versionedCache = cachePath + QStringLiteral(VERSION);
    {
        ofstream bad(versionedCache.toStdString());
        bad << "not a valid cache";
    }

    Pinyin4cppTrie trie({singlePath.toStdString(), wordsPath.toStdString()}, cachePath.toStdString());
    QCOMPARE(trie.Find(string("中")), string("zhōng,zhòng"));
    QVERIFY(QFileInfo(versionedCache).size() > static_cast<qint64>(sizeof(CacheFileHeaderBase)));
}

void Pinyin4cppTrieTest::defaultTrieUsesInstalledDictionary()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString cachePath = tempDir.path() + QStringLiteral("/default-pinyin-trie-cache.dat");

    Pinyin4cppTrie trie(cachePath.toStdString());
    QCOMPARE(trie.Find(string("中国")), string("zhōng,guó"));
    string present = "中国";
    string missing = "不存在不存在";
    QVERIFY(trie.Contains(present));
    QVERIFY(!trie.Contains(missing));
}

QTEST_MAIN(Pinyin4cppTrieTest)

#include "pinyin4cpp-trie-test.moc"
