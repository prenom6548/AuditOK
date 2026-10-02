#include <QTemporaryDir>
#include <QTest>

#include "cppjieba/idf-trie/idf-trie.h"
#include "cppjieba/segment-trie/segment-trie.h"
#include "cppjieba/Unicode.hpp"

using namespace cppjieba;

namespace {
const std::string kDictPath = DICT_INSTALL_PATH "/jieba.dict.utf8";
const std::string kUserDictPath = DICT_INSTALL_PATH "/user.dict.utf8";
const std::string kIdfPath = DICT_INSTALL_PATH "/idf.utf8";

std::string cachePath(const QTemporaryDir &dir, const char *name)
{
    return dir.filePath(QLatin1String(name)).toStdString();
}

RuneArray decodeRuneArray(const std::string &sentence)
{
    RuneArray runes;
    const bool ok = DecodeRunesInString(sentence, runes);
    Q_ASSERT(ok);
    return runes;
}

RuneStrArray decodeRunes(const std::string &sentence)
{
    RuneStrArray runes;
    const bool ok = DecodeRunesInString(sentence, runes);
    Q_ASSERT(ok);
    return runes;
}
}

class CppJiebaFixtureDictTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void idfTrieSkipsCommentsEmptyAndMalformedLines();
    void dictTrieSkipsMalformedLinesAndTracksUserSingleWords();
};

void CppJiebaFixtureDictTest::idfTrieSkipsCommentsEmptyAndMalformedLines()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    IdfTrie trie(kIdfPath, cachePath(dir, "idf-cache"));

    QCOMPARE(trie.Find("bad-idf-line"), -1.0);
    QVERIFY(trie.Find("系统") > 0.0);
    QVERIFY(trie.Find("蓝翔") > 0.0);
    QVERIFY(trie.GetIdfAverage() > 0.0);
}

void CppJiebaFixtureDictTest::dictTrieSkipsMalformedLinesAndTracksUserSingleWords()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    DictTrie trie(kDictPath, kUserDictPath, cachePath(dir, "dict-cache"));

    QVERIFY(trie.Find("系统") != nullptr);
    QVERIFY(trie.Find("蓝") != nullptr);
    QVERIFY(trie.Find("bad") == nullptr);

    const RuneArray userSingle = decodeRuneArray("蓝");
    QVERIFY(!userSingle.empty());
    QVERIFY(trie.IsUserDictSingleChineseWord(userSingle[0]));

    const std::string sentence = "系统蓝翔";
    const RuneStrArray runes = decodeRunes(sentence);
    std::vector<DatDag> dags;
    trie.FindDatDag(runes.begin(), runes.end(), dags, 1);
    QCOMPARE(dags.size(), runes.size());
    QVERIFY(std::all_of(dags.begin(), dags.end(), [](const DatDag &dag) {
        return !dag.nexts.empty();
    }));

    std::vector<WordRange> words;
    trie.FindWordRange(runes.begin(), runes.end(), words);
    QVERIFY(!words.empty());
}

QTEST_MAIN(CppJiebaFixtureDictTest)

#include "cppjieba-fixture-dict-test.moc"
