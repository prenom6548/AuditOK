#include <QTest>

#include <algorithm>

#include "chinese-segmentation.h"
#include "chinese-segmentation-private.h"

using namespace std;

namespace {
template <typename T>
bool containsWord(const vector<T> &words, const string &word)
{
    return any_of(words.begin(), words.end(), [&word](const T &item) {
        return item.word == word;
    });
}

bool containsString(const vector<string> &words, const string &word)
{
    return find(words.begin(), words.end(), word) != words.end();
}
}

class ChineseSegmentationTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void singletonReturnsSameInstance();
    void cutsSentenceWithAllPublicSegmenters();
    void extractsKeywordsAndTags();
    void stringSegmentOverloadNormalizesPunctuation();
    void privateImplementationSupportsLocalLifetime();
    void emptyInputsReturnEmptyResults();
};

void ChineseSegmentationTest::singletonReturnsSameInstance()
{
    QCOMPARE(ChineseSegmentation::getInstance(), ChineseSegmentation::getInstance());
}

void ChineseSegmentationTest::cutsSentenceWithAllPublicSegmenters()
{
    ChineseSegmentation *segment = ChineseSegmentation::getInstance();
    const string sentence = "银河麒麟系统";

    const vector<string> mixStrings = segment->callMixSegmentCutStr(sentence);
    QVERIFY(!mixStrings.empty());
    QVERIFY(containsString(mixStrings, "银河") || containsString(mixStrings, "麒麟"));

    const vector<Word> mixWords = segment->callMixSegmentCutWord(sentence);
    QVERIFY(!mixWords.empty());
    QVERIFY(containsWord(mixWords, "银河") || containsWord(mixWords, "麒麟"));

    const vector<Word> fullWords = segment->callFullSegment(sentence);
    QVERIFY(!fullWords.empty());

    const vector<Word> queryWords = segment->callQuerySegment(sentence);
    QVERIFY(!queryWords.empty());

    const vector<Word> hmmWords = segment->callHMMSegment(sentence);
    QVERIFY(!hmmWords.empty());

    const vector<Word> mpWords = segment->callMPSegment(sentence);
    QVERIFY(!mpWords.empty());
}

void ChineseSegmentationTest::extractsKeywordsAndTags()
{
    ChineseSegmentation *segment = ChineseSegmentation::getInstance();

    const vector<KeyWord> keywords = segment->callSegment(string("银河麒麟系统稳定运行"));
    QVERIFY(!keywords.empty());

    const string tag = segment->lookUpTagOfWord("系统");
    QVERIFY(!tag.empty());

    const vector<pair<string, string>> taggedWords = segment->getTagOfWordsInSentence("银河麒麟系统");
    QVERIFY(!taggedWords.empty());
    QVERIFY(any_of(taggedWords.begin(), taggedWords.end(), [](const pair<string, string> &item) {
        return !item.first.empty() && !item.second.empty();
    }));
}

void ChineseSegmentationTest::stringSegmentOverloadNormalizesPunctuation()
{
    ChineseSegmentation *segment = ChineseSegmentation::getInstance();
    QString sentence = QStringLiteral("银河\t麒麟，系统。稳定");

    const vector<KeyWord> keywords = segment->callSegment(sentence);
    QVERIFY(!keywords.empty());
    QVERIFY(!sentence.contains(QLatin1Char('\t')));
    QVERIFY(!sentence.contains(QStringLiteral("，")));
    QVERIFY(!sentence.contains(QStringLiteral("。")));
}

void ChineseSegmentationTest::privateImplementationSupportsLocalLifetime()
{
    QString sentence = QStringLiteral("银河\t麒麟，系统。稳定");

    {
        ChineseSegmentationPrivate local;

        const vector<KeyWord> keywords = local.callSegment(string("银河麒麟系统稳定运行"));
        QVERIFY(!keywords.empty());

        const vector<KeyWord> qstringKeywords = local.callSegment(sentence);
        QVERIFY(!qstringKeywords.empty());
        QVERIFY(!sentence.contains(QLatin1Char('\t')));

        const vector<string> mixStrings = local.callMixSegmentCutStr("银河麒麟系统");
        QVERIFY(!mixStrings.empty());

        const vector<Word> mixWords = local.callMixSegmentCutWord("银河麒麟系统");
        QVERIFY(!mixWords.empty());

        QVERIFY(!local.lookUpTagOfWord("系统").empty());
        QVERIFY(!local.getTagOfWordsInSentence("银河麒麟系统").empty());
        QVERIFY(!local.callFullSegment("银河麒麟系统").empty());
        QVERIFY(!local.callQuerySegment("银河麒麟系统").empty());
        QVERIFY(!local.callHMMSegment("银河麒麟系统").empty());
        QVERIFY(!local.callMPSegment("银河麒麟系统").empty());
    }
}

void ChineseSegmentationTest::emptyInputsReturnEmptyResults()
{
    ChineseSegmentation *segment = ChineseSegmentation::getInstance();
    QString emptyText;

    QVERIFY(segment->callSegment(string()).empty());
    QVERIFY(segment->callSegment(emptyText).empty());
    QVERIFY(segment->callMixSegmentCutStr(string()).empty());
    QVERIFY(segment->callMixSegmentCutWord(string()).empty());
    QVERIFY(!segment->lookUpTagOfWord(string()).empty());
    QVERIFY(segment->getTagOfWordsInSentence(string()).empty());
    QVERIFY(segment->callFullSegment(string()).empty());
    QVERIFY(segment->callQuerySegment(string()).empty());
    QVERIFY(segment->callHMMSegment(string()).empty());
    QVERIFY(segment->callMPSegment(string()).empty());
}

QTEST_MAIN(ChineseSegmentationTest)

#include "chinese-segmentation-test.moc"
