#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

#include <fstream>

#include "pinyin4cpp_dictTrie.h"

using namespace std;

namespace {
QString writeSingleWordDict(const QString &dirPath)
{
    const QString path = dirPath + QStringLiteral("/single-word.txt");
    ofstream out(path.toStdString());
    out << "# comment\n";
    out << "1:zhong:中\n";
    out << "2:zhong:中\n";
    out << "3:guo:国\n";
    out << "4:chong:重\n";
    out << "5:zhong:重\n";
    out.close();
    return path;
}

QString writeWordsDict(const QString &dirPath)
{
    const QString path = dirPath + QStringLiteral("/words.txt");
    ofstream out(path.toStdString());
    out << "# comment\n";
    out << "中国:zhong,guo\n";
    out << "重庆:chong,qing\n";
    out << "重庆:chong,qing\n";
    out << "重庆:zhong,qing\n";
    out.close();
    return path;
}
}

class Pinyin4cppDictTrieTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void loadsSourceDictionariesAndBuildsCache();
    void calcFileListMd5IgnoresMissingFiles();
};

void Pinyin4cppDictTrieTest::loadsSourceDictionariesAndBuildsCache()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString singlePath = writeSingleWordDict(tempDir.path());
    const QString wordsPath = writeWordsDict(tempDir.path());
    const QString cachePath = tempDir.path() + QStringLiteral("/dict-trie-cache.dat");

    Pinyin4cppDictTrie trie(singlePath.toStdString(), wordsPath.toStdString(), cachePath.toStdString());

    string oneWord = "中";
    string missingWord = "不存在";
    QCOMPARE(trie.Find(oneWord), string("zhong"));
    QCOMPARE(trie.Find(string("中国")), string("zhong,guo"));
    QCOMPARE(trie.Find(string("重庆")), string("chong,qing/zhong,qing"));
    QCOMPARE(trie.Find(string("重")), string("chong,重"));
    QVERIFY(trie.Contains(oneWord));
    QVERIFY(!trie.Contains(missingWord));
    QVERIFY(!trie.IsMultiTone(string("重庆")));
    QVERIFY(trie.IsMultiTone(missingWord));
    QVERIFY(trie.GetTotalDictSize() > 0);
    QVERIFY(QFileInfo::exists(cachePath));

    Pinyin4cppDictTrie attachedTrie(singlePath.toStdString(), wordsPath.toStdString(), cachePath.toStdString());
    QCOMPARE(attachedTrie.Find(string("中国")), string("zhong,guo"));
    QCOMPARE(attachedTrie.GetTotalDictSize(), trie.GetTotalDictSize());
}

void Pinyin4cppDictTrieTest::calcFileListMd5IgnoresMissingFiles()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString singlePath = writeSingleWordDict(tempDir.path());
    const QString wordsPath = writeWordsDict(tempDir.path());
    const QString missingPath = tempDir.path() + QStringLiteral("/missing.txt");

    size_t completeSize = 0;
    const string completeMd5 = CalcFileListMD5(
        singlePath.toStdString() + "|" + wordsPath.toStdString(), completeSize);

    size_t withMissingSize = 0;
    const string withMissingMd5 = CalcFileListMD5(
        singlePath.toStdString() + "|" + missingPath.toStdString() + ";" + wordsPath.toStdString(),
        withMissingSize);

    QVERIFY(!completeMd5.empty());
    QCOMPARE(withMissingMd5, completeMd5);
    QCOMPARE(withMissingSize, completeSize);
}

QTEST_MAIN(Pinyin4cppDictTrieTest)

#include "pinyin4cpp-dicttrie-test.moc"
