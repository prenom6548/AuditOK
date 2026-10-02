#include <QTemporaryDir>
#include <QTest>

#include <algorithm>
#include <deque>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>

#define private public
#include "cppjieba/FullSegment.hpp"
#include "cppjieba/HMMSegment.hpp"
#include "cppjieba/Jieba.hpp"
#include "cppjieba/Unicode.hpp"
#include "cppjieba/limonp/Logging.hpp"
#include "cppjieba/limonp/StringUtil.hpp"
#include "cppjieba/MPSegment.hpp"
#include "cppjieba/MixSegment.hpp"
#include "cppjieba/PosTagger.hpp"
#include "cppjieba/QuerySegment.hpp"
#include "cppjieba/idf-trie/idf-trie.h"
#include "cppjieba/limonp/LocalVector.hpp"
#include "cppjieba/segment-trie/segment-trie.h"
#undef private

using namespace cppjieba;

namespace {
const std::string kDictPath = DICT_INSTALL_PATH "/jieba.dict.utf8";
const std::string kUserDictPath = DICT_INSTALL_PATH "/user.dict.utf8";
const std::string kIdfPath = DICT_INSTALL_PATH "/idf.utf8";
const std::string kHmmPath = DICT_INSTALL_PATH "/hmm_model.utf8";
const std::string kStopWordsPath = DICT_INSTALL_PATH "/stop_words.utf8";

std::string cachePath(const QTemporaryDir &dir, const char *name)
{
    return dir.filePath(QLatin1String(name)).toStdString();
}

bool containsString(const std::vector<std::string> &words, const std::string &word)
{
    return std::find(words.begin(), words.end(), word) != words.end();
}

bool containsWordRange(const std::string &sentence,
                       const std::vector<WordRange> &ranges,
                       const std::string &word)
{
    return std::any_of(ranges.begin(), ranges.end(), [&sentence, &word](const WordRange &range) {
        return GetStringFromRunes(sentence, range.left, range.right) == word;
    });
}

RuneStrArray decodeRunes(const std::string &sentence)
{
    RuneStrArray runes;
    const bool ok = DecodeRunesInString(sentence, runes);
    Q_ASSERT(ok);
    return runes;
}

RuneArray decodeRuneArray(const std::string &sentence)
{
    RuneArray runes;
    const bool ok = DecodeRunesInString(sentence, runes);
    Q_ASSERT(ok);
    return runes;
}

class StubSegment : public SegmentBase
{
public:
    void Cut(RuneStrArray::const_iterator begin,
             RuneStrArray::const_iterator end,
             std::vector<WordRange> &res,
             bool,
             size_t) const override
    {
        if (begin != end) {
            res.push_back(WordRange(begin, end - 1));
        }
    }

    void CutWithSentence(const std::string &sentence,
                         RuneStrArray::const_iterator begin,
                         RuneStrArray::const_iterator end,
                         std::vector<std::string> &res,
                         bool,
                         size_t) const override
    {
        if (begin != end) {
            res.push_back(GetStringFromRunes(sentence, begin, end - 1));
        }
    }

    void CutWithSentence(const std::string &sentence,
                         RuneStrArray::const_iterator begin,
                         RuneStrArray::const_iterator end,
                         std::unordered_map<std::string, KeyWord> &res,
                         bool,
                         size_t) const override
    {
        if (begin == end) {
            return;
        }

        const std::string word = GetStringFromRunes(sentence, begin, end - 1);
        KeyWord &entry = res[word];
        entry.word = word;
        entry.weight = 1.0;
        entry.offsets.push_back(begin->offset);
    }
};
}

class CppJiebaWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void localVectorCoversStackHeapAndCopyPaths();
    void idfTrieBuildsAttachesAndFindsWords();
    void dictTrieFindsDagAndWordRanges();
    void mpSegmentCutsTagsAndIgnoresSentenceOverloads();
    void posTaggerCoversDictionaryAndSpecialRules();
    void hmmModelCoversLineParsingEmitProbAndDefaults();
    void mixSegmentCoversHmmStopWordsAndKeywordMapBranches();
    void querySegmentAddsSubWordsAndCoversNoopOverloads();
    void preFilterCoversSpaceAndLengthBoundaries();
    void fullSegmentAndSegmentBaseCoverSeparatorAndNoopBranches();
    void jiebaCoversPublicCutAndKeywordWrappers();
    void stringUtilCoversEncodingAndPathHelpers();
};

void CppJiebaWhiteboxTest::localVectorCoversStackHeapAndCopyPaths()
{
    limonp::LocalVector<int> empty;
    QVERIFY(empty.empty());
    QCOMPARE(empty.size(), static_cast<size_t>(0));
    QCOMPARE(empty.capacity(), limonp::LOCAL_VECTOR_BUFFER_SIZE);

    limonp::LocalVector<int> filled(3, 7);
    QCOMPARE(filled.size(), static_cast<size_t>(3));
    QCOMPARE(filled[0], 7);
    QCOMPARE(filled[2], 7);

    int raw[] = {1, 2, 3, 4};
    limonp::LocalVector<int> fromRange(raw, raw + 4);
    QCOMPARE(fromRange.size(), static_cast<size_t>(4));
    QCOMPARE(fromRange[3], 4);

    for (int i = 0; i < 40; ++i) {
        empty.push_back(i);
    }
    QCOMPARE(empty.size(), static_cast<size_t>(40));
    QVERIFY(empty.capacity() > limonp::LOCAL_VECTOR_BUFFER_SIZE);
    QCOMPARE(empty[17], 17);

    limonp::LocalVector<int> copied(empty);
    QCOMPARE(copied.size(), empty.size());
    QCOMPARE(copied[39], 39);

    limonp::LocalVector<int> assigned;
    assigned = copied;
    QCOMPARE(assigned.size(), copied.size());
    QCOMPARE(assigned[20], 20);
    assigned = assigned;
    QCOMPARE(assigned[20], 20);

    std::ostringstream stream;
    stream << assigned;
    QVERIFY(stream.str().find("\"0\"") != std::string::npos);

    assigned.clear();
    QVERIFY(assigned.empty());
    QCOMPARE(assigned.capacity(), limonp::LOCAL_VECTOR_BUFFER_SIZE);

    std::ostringstream emptyStream;
    emptyStream << assigned;
    QCOMPARE(emptyStream.str(), std::string("[]"));
}

void CppJiebaWhiteboxTest::idfTrieBuildsAttachesAndFindsWords()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const std::string cache = cachePath(dir, "idf-cache");

    IdfTrie trie(kIdfPath, cache);
    QCOMPARE(trie.Find("not-existing-idf-word"), -1.0);
    QVERIFY(trie.Find("编译") > 0.0);
    QVERIFY(trie.GetIdfAverage() > 0.0);

    IdfTrie attached(kIdfPath, cache);
    QCOMPARE(attached.Find("编译"), trie.Find("编译"));
    QCOMPARE(attached.GetIdfAverage(), trie.GetIdfAverage());

    IdfTrie vectorCtor(std::vector<std::string>{kIdfPath}, cachePath(dir, "idf-cache-vector"));
    QVERIFY(vectorCtor.Find("社会公众") > 0.0);
}

void CppJiebaWhiteboxTest::dictTrieFindsDagAndWordRanges()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const std::string cache = cachePath(dir, "dict-cache");

    DictTrie trie(kDictPath, kUserDictPath, cache);
    QVERIFY(trie.Find("系统") != nullptr);
    QVERIFY(trie.Find("区块链") != nullptr);
    QVERIFY(trie.Find("not-existing-dict-word") == nullptr);
    QVERIFY(trie.GetMinWeight() <= 3.14e+100);

    const RuneArray singleUserWord = decodeRuneArray("蓝");
    QVERIFY(!singleUserWord.empty());
    QVERIFY(!trie.IsUserDictSingleChineseWord(singleUserWord[0]));

    const std::string sentence = "银河麒麟系统区块链";
    const RuneStrArray runes = decodeRunes(sentence);
    std::vector<DatDag> dags;
    trie.FindDatDag(runes.begin(), runes.end(), dags);
    QCOMPARE(dags.size(), runes.size());
    QVERIFY(std::any_of(dags.begin(), dags.end(), [](const DatDag &dag) {
        return dag.nexts.size() > 1;
    }));

    std::vector<WordRange> words;
    trie.FindWordRange(runes.begin(), runes.end(), words);
    QVERIFY(!words.empty());
    QVERIFY(containsWordRange(sentence, words, "系统") || containsWordRange(sentence, words, "区块链"));

    std::vector<WordRange> shortWords;
    trie.FindWordRange(runes.begin(), runes.end(), shortWords, 1);
    QVERIFY(shortWords.size() >= words.size());

    DictTrie vectorCtor(std::vector<std::string>{kDictPath, kUserDictPath}, cachePath(dir, "dict-cache-vector"));
    QVERIFY(vectorCtor.Find("系统") != nullptr);
}

void CppJiebaWhiteboxTest::mpSegmentCutsTagsAndIgnoresSentenceOverloads()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DictTrie trie(kDictPath, kUserDictPath, cachePath(dir, "dict-cache"));
    MPSegment segment(&trie);

    const std::string sentence = "蓝翔系统123";
    const RuneStrArray runes = decodeRunes(sentence);
    std::vector<WordRange> ranges;
    segment.CutRuneArray(runes.begin(), runes.end(), ranges, true);
    QVERIFY(!ranges.empty());
    QVERIFY(containsWordRange(sentence, ranges, "系统"));

    std::vector<Word> words;
    segment.CutToWord(sentence, words);
    QVERIFY(!words.empty());

    std::vector<std::pair<std::string, std::string>> tagged;
    QVERIFY(!segment.Tag(sentence, tagged));
    QVERIFY(tagged.empty());

    const RuneArray userWord = decodeRuneArray("蓝");
    QVERIFY(!segment.IsUserDictSingleChineseWord(userWord[0]));
    QCOMPARE(segment.GetDictTrie(), &trie);

    std::vector<std::string> ignoredStrings;
    std::unordered_map<std::string, KeyWord> ignoredKeywords;
    segment.CutWithSentence(sentence, runes.begin(), runes.end(), ignoredStrings, true, MAX_WORD_LENGTH);
    segment.CutWithSentence(sentence, runes.begin(), runes.end(), ignoredKeywords, true, MAX_WORD_LENGTH);
    QVERIFY(ignoredStrings.empty());
    QVERIFY(ignoredKeywords.empty());
}

void CppJiebaWhiteboxTest::posTaggerCoversDictionaryAndSpecialRules()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DictTrie trie(kDictPath, kUserDictPath, cachePath(dir, "dict-cache"));
    MPSegment segment(&trie);
    PosTagger tagger;

    QCOMPARE(tagger.LookupTag("12345", segment), std::string(POS_M));
    QCOMPARE(tagger.LookupTag("abc123", segment), std::string(POS_ENG));
    QCOMPARE(tagger.LookupTag("不存在词", segment), std::string(POS_X));
    QCOMPARE(tagger.LookupTag("系统", segment), std::string("n"));
    QCOMPARE(tagger.LookupTag(std::string(1, static_cast<char>(0xff)), segment), std::string(POS_X));

    HMMModel model(kHmmPath);
    MixSegment mixSegment(&trie, &model, kStopWordsPath);

    std::vector<std::pair<std::string, std::string>> tags;
    QVERIFY(tagger.Tag("系统 abc123 12345 不存在词", tags, mixSegment));
    QVERIFY(std::any_of(tags.begin(), tags.end(), [](const auto &item) {
        return item.second == POS_M || item.second == POS_ENG || item.second == POS_X;
    }));
}

void CppJiebaWhiteboxTest::hmmModelCoversLineParsingEmitProbAndDefaults()
{
    HMMModel model(kHmmPath);

    QCOMPARE(model.statMap[HMMModel::B], 'B');
    QCOMPARE(model.statMap[HMMModel::E], 'E');
    QCOMPARE(model.statMap[HMMModel::M], 'M');
    QCOMPARE(model.statMap[HMMModel::S], 'S');
    QCOMPARE(model.emitProbVec.size(), static_cast<size_t>(HMMModel::STATUS_SUM));

    RuneArray runes = decodeRuneArray("的");
    QVERIFY(!runes.empty());
    const double fallback = -123.0;
    QCOMPARE(model.GetEmitProb(&model.emitProbB, 0, fallback), fallback);
    QVERIFY(model.GetEmitProb(&model.emitProbB, runes[0], fallback) != fallback
            || model.GetEmitProb(&model.emitProbE, runes[0], fallback) != fallback
            || model.GetEmitProb(&model.emitProbM, runes[0], fallback) != fallback
            || model.GetEmitProb(&model.emitProbS, runes[0], fallback) != fallback);

    std::istringstream lineStream("# comment\n\n  first useful line  \nsecond");
    std::ifstream *unused = nullptr;
    Q_UNUSED(unused);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString lineFile = dir.filePath(QStringLiteral("lines.txt"));
    {
        std::ofstream out(lineFile.toStdString());
        out << "# comment\n\n  payload  \n";
    }
    std::ifstream in(lineFile.toStdString());
    std::string line;
    QVERIFY(model.GetLine(in, line));
    QCOMPARE(line, std::string("payload"));
    QVERIFY(!model.GetLine(in, line));

    EmitProbMap emitMap;
    QVERIFY(model.LoadEmitProb("甲:-1.25,乙:-2.5", emitMap));
    RuneArray custom = decodeRuneArray("甲乙");
    QCOMPARE(model.GetEmitProb(&emitMap, custom[0], fallback), -1.25);
    QCOMPARE(model.GetEmitProb(&emitMap, custom[1], fallback), -2.5);
    QVERIFY(!model.LoadEmitProb("", emitMap));
    QVERIFY(!model.LoadEmitProb("bad-token", emitMap));
    QVERIFY(!model.LoadEmitProb("甲乙:-3.0", emitMap));

    HMMSegment segment(&model);
    const std::string sentence = "abc123系统";
    const RuneStrArray sentenceRunes = decodeRunes(sentence);
    std::vector<std::string> ignoredStrings;
    std::unordered_map<std::string, KeyWord> ignoredKeywords;
    segment.CutWithSentence(sentence, sentenceRunes.begin(), sentenceRunes.end(), ignoredStrings, true, MAX_WORD_LENGTH);
    segment.CutWithSentence(sentence, sentenceRunes.begin(), sentenceRunes.end(), ignoredKeywords, true, MAX_WORD_LENGTH);
    QVERIFY(ignoredStrings.empty());
    QVERIFY(ignoredKeywords.empty());

    const std::string mixedSentence = "系统abc";
    const RuneStrArray mixedRunes = decodeRunes(mixedSentence);
    std::vector<WordRange> mixedRanges;
    segment.CutRuneArray(mixedRunes.begin(), mixedRunes.end(), mixedRanges);
    QVERIFY(containsWordRange(mixedSentence, mixedRanges, "abc"));
}

void CppJiebaWhiteboxTest::mixSegmentCoversHmmStopWordsAndKeywordMapBranches()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DictTrie trie(kDictPath, kUserDictPath, cachePath(dir, "dict-cache"));
    HMMModel model(kHmmPath);
    MixSegment segment(&trie, &model, kStopWordsPath);

    const std::string sentence = "蓝翔系统稳定运行abc123";
    const RuneStrArray runes = decodeRunes(sentence);

    std::vector<WordRange> mpRanges;
    segment.CutRuneArray(runes.begin(), runes.end(), mpRanges, false);
    QVERIFY(!mpRanges.empty());

    std::vector<WordRange> hmmRanges;
    segment.CutRuneArray(runes.begin(), runes.end(), hmmRanges, true);
    QVERIFY(!hmmRanges.empty());

    std::vector<std::string> words;
    segment.CutToStr(sentence, words, true);
    QVERIFY(!words.empty());
    QVERIFY(containsString(words, "系统") || containsString(words, "蓝翔"));

    WordRange whole(runes.begin(), runes.end());
    std::unordered_map<std::string, KeyWord> keywordMap;
    segment.CutToStr(sentence, whole, keywordMap, true);
    QVERIFY(!keywordMap.empty());
    QVERIFY(keywordMap.find("系统") != keywordMap.end() || keywordMap.find("蓝翔") != keywordMap.end());

    const std::string asciiSentence = "abc123";
    const RuneStrArray asciiRunes = decodeRunes(asciiSentence);
    std::unordered_map<std::string, KeyWord> asciiMap;
    segment.CutToStr(asciiSentence, WordRange(asciiRunes.begin(), asciiRunes.end()), asciiMap, true);
    QVERIFY(asciiMap.find("a") != asciiMap.end());
    QVERIFY(asciiMap.find("1") != asciiMap.end());
    QCOMPARE(asciiMap["a"].offsets.front(), static_cast<size_t>(0));

    const std::string singleChineseSentence = "中";
    const RuneStrArray singleChineseRunes = decodeRunes(singleChineseSentence);
    std::unordered_map<std::string, KeyWord> singleChineseMap;
    segment.CutToStr(singleChineseSentence,
                     WordRange(singleChineseRunes.begin(), singleChineseRunes.end()),
                     singleChineseMap,
                     true);
    QVERIFY(singleChineseMap.find("中") != singleChineseMap.end());

    MixSegment missingStopWordsSegment(&trie, &model, cachePath(dir, "missing-stop-words"));
    std::vector<std::pair<std::string, std::string>> missingStopWordsTags;
    QVERIFY(missingStopWordsSegment.Tag("系统", missingStopWordsTags));

    std::vector<std::pair<std::string, std::string>> tagged;
    QVERIFY(segment.Tag("系统 abc123", tagged));
    QCOMPARE(segment.LookupTag("abc123"), std::string(POS_ENG));
    QCOMPARE(segment.GetDictTrie(), &trie);
}

void CppJiebaWhiteboxTest::querySegmentAddsSubWordsAndCoversNoopOverloads()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DictTrie trie(kDictPath, kUserDictPath, cachePath(dir, "dict-cache"));
    HMMModel model(kHmmPath);
    QuerySegment segment(&trie, &model, kStopWordsPath);

    const std::string sentence = "一一对应系统";
    const RuneStrArray runes = decodeRunes(sentence);
    std::vector<WordRange> ranges;
    segment.CutRuneArray(runes.begin(), runes.end(), ranges, true);
    QVERIFY(!ranges.empty());
    QVERIFY(containsWordRange(sentence, ranges, "一一"));
    QVERIFY(containsWordRange(sentence, ranges, "一一对") || containsWordRange(sentence, ranges, "一一对应"));

    std::vector<Word> words;
    segment.CutToWord(sentence, words);
    QVERIFY(!words.empty());

    std::vector<std::string> ignoredStrings;
    std::unordered_map<std::string, KeyWord> ignoredKeywords;
    segment.CutWithSentence(sentence, runes.begin(), runes.end(), ignoredStrings, true, MAX_WORD_LENGTH);
    segment.CutWithSentence(sentence, runes.begin(), runes.end(), ignoredKeywords, true, MAX_WORD_LENGTH);
    QVERIFY(ignoredStrings.empty());
    QVERIFY(ignoredKeywords.empty());

    const RuneArray ascii = decodeRuneArray("abc123");
    QVERIFY(segment.IsAllAscii(ascii));
    const RuneArray chinese = decodeRuneArray("系统");
    QVERIFY(!segment.IsAllAscii(chinese));

}

void CppJiebaWhiteboxTest::preFilterCoversSpaceAndLengthBoundaries()
{
    const std::unordered_set<Rune> symbols;

    {
        PreFilter filter(symbols, "");
        QVERIFY(!filter.HasNext());
        WordRange range = filter.Next();
        QCOMPARE(range.left, filter.sentence_.end());
        QCOMPARE(range.right, filter.sentence_.end());
    }

    {
        PreFilter filter(symbols, " abc");
        WordRange range = filter.Next();
        QCOMPARE(range.left, filter.sentence_.begin());
        QCOMPARE(range.right, filter.sentence_.begin() + 1);
        QVERIFY(filter.HasNext());

        range = filter.Next();
        QCOMPARE(GetStringFromRunes(" abc", range.left, range.right - 1), std::string("abc"));
        QVERIFY(!filter.HasNext());
    }

    {
        PreFilter filter(symbols, std::string(3, ' '));
        WordRange range(filter.sentence_.end(), filter.sentence_.end());
        QVERIFY(filter.Next(range));
        QCOMPARE(range.right, filter.sentence_.end());
        QVERIFY(!filter.HasNext());
    }

    {
        PreFilter filter(symbols, "  abc");
        WordRange range(filter.sentence_.end(), filter.sentence_.end());
        bool isNull = false;
        QVERIFY(filter.Next(range, isNull));
        QVERIFY(isNull);
        QCOMPARE(range.right, filter.sentence_.begin() + 2);
    }

    {
        PreFilter filter(symbols, std::string(3, ' '));
        WordRange range(filter.sentence_.end(), filter.sentence_.end());
        bool isNull = false;
        QVERIFY(!filter.Next(range, isNull));
        QVERIFY(!isNull);
    }

    {
        PreFilter filter(symbols, "abc def");
        WordRange range(filter.sentence_.end(), filter.sentence_.end());
        bool isNull = false;
        QVERIFY(filter.Next(range, isNull));
        QVERIFY(!isNull);
        QCOMPARE(GetStringFromRunes("abc def", range.left, range.right - 1), std::string("abc"));
    }

    {
        PreFilter filter(symbols, "abc");
        WordRange range(filter.sentence_.end(), filter.sentence_.end());
        bool isNull = false;
        QVERIFY(filter.Next(range, isNull));
        QVERIFY(!isNull);
        QCOMPARE(range.right, filter.sentence_.end());
    }

    {
        PreFilter filter(symbols, std::string(1025, 'a'));
        WordRange range(filter.sentence_.end(), filter.sentence_.end());
        bool isNull = false;
        QVERIFY(filter.Next(range, isNull));
        QVERIFY(!isNull);
        QCOMPARE(range.right - range.left, 1024);
    }

    {
        const std::string mixedWidth = "abc中";
        PreFilter filter(symbols, mixedWidth);
        WordRange range(filter.sentence_.end(), filter.sentence_.end());
        bool isNull = false;
        QVERIFY(filter.Next(range, isNull));
        QVERIFY(!isNull);
        QCOMPARE(GetStringFromRunes(mixedWidth, range.left, range.right - 1), std::string("abc"));
    }

    {
        const std::string invalidUtf8(1, static_cast<char>(0xff));
        PreFilter filter(symbols, invalidUtf8);
        QVERIFY(!filter.HasNext());
    }
}

void CppJiebaWhiteboxTest::fullSegmentAndSegmentBaseCoverSeparatorAndNoopBranches()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    DictTrie trie(kDictPath, kUserDictPath, cachePath(dir, "dict-cache"));

    FullSegment fullSegment(&trie);
    const std::string sentence = "系统开发";
    const RuneStrArray runes = decodeRunes(sentence);
    std::vector<WordRange> ranges;
    fullSegment.CutRuneArray(runes.begin(), runes.end(), ranges, true);
    QVERIFY(!ranges.empty());
    QVERIFY(containsWordRange(sentence, ranges, "系统"));

    const std::string asciiSentence = "abc";
    const RuneStrArray asciiRunes = decodeRunes(asciiSentence);
    std::vector<WordRange> asciiRanges;
    fullSegment.CutRuneArray(asciiRunes.begin(), asciiRunes.end(), asciiRanges, true);
    QVERIFY(!asciiRanges.empty());
    QVERIFY(containsWordRange(asciiSentence, asciiRanges, "a"));

    std::vector<std::string> ignoredStrings;
    std::unordered_map<std::string, KeyWord> ignoredKeywords;
    fullSegment.CutWithSentence(sentence, runes.begin(), runes.end(), ignoredStrings, true, MAX_WORD_LENGTH);
    fullSegment.CutWithSentence(sentence, runes.begin(), runes.end(), ignoredKeywords, true, MAX_WORD_LENGTH);
    QVERIFY(ignoredStrings.empty());
    QVERIFY(ignoredKeywords.empty());

    StubSegment stub;
    std::vector<std::string> words;
    stub.CutToStr("系统 abc", words);
    QVERIFY(!words.empty());
    QVERIFY(containsString(words, "系统"));

    std::vector<std::string> asciiWords;
    stub.CutToStr("abc", asciiWords);
    QCOMPARE(asciiWords, std::vector<std::string>({"abc"}));

    std::vector<Word> cutWords;
    stub.CutToWord("系统 abc", cutWords);
    QVERIFY(!cutWords.empty());
    QCOMPARE(cutWords.front().word, std::string("系统"));

    std::unordered_map<std::string, KeyWord> keywordMap;
    const RuneStrArray allRunes = decodeRunes("系统abc");
    stub.CutToStr("系统abc", WordRange(allRunes.begin(), allRunes.end()), keywordMap);
    QVERIFY(keywordMap.find("系统abc") != keywordMap.end());

    std::vector<std::string> directWords;
    stub.CutToStr("系统abc", WordRange(allRunes.begin(), allRunes.end()), directWords);
    QCOMPARE(directWords, std::vector<std::string>({"系统abc"}));

    std::vector<std::string> emptyWords;
    stub.CutToStr("", emptyWords);
    QVERIFY(emptyWords.empty());

    std::vector<Word> emptyCutWords;
    stub.CutToWord("", emptyCutWords);
    QVERIFY(emptyCutWords.empty());

    QVERIFY(stub.ResetSeparators(""));
    QVERIFY(!stub.ResetSeparators("aa"));
    const std::string invalidUtf8(1, static_cast<char>(0xff));
    QVERIFY(!stub.ResetSeparators(invalidUtf8));
    QVERIFY(stub.ResetSeparators(SPECIAL_SEPARATORS));
}

void CppJiebaWhiteboxTest::jiebaCoversPublicCutAndKeywordWrappers()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Jieba jieba(kDictPath, kHmmPath, kUserDictPath, kIdfPath, kStopWordsPath, cachePath(dir, "jieba-cache"));

    std::vector<std::string> words;
    jieba.Cut("系统 abc123", words);
    QVERIFY(!words.empty());

    std::vector<Word> wordObjects;
    jieba.Cut("系统 abc123", wordObjects);
    QVERIFY(!wordObjects.empty());

    std::vector<std::string> allWords;
    jieba.CutAll("系统开发", allWords);
    QVERIFY(allWords.empty());

    std::vector<Word> allWordObjects;
    jieba.CutAll("系统开发", allWordObjects);
    QVERIFY(!allWordObjects.empty());

    std::vector<std::string> searchWords;
    jieba.CutForSearch("一一对应系统", searchWords);
    QVERIFY(searchWords.empty());

    std::vector<Word> searchWordObjects;
    jieba.CutForSearch("一一对应系统", searchWordObjects);
    QVERIFY(!searchWordObjects.empty());

    std::vector<std::string> hmmWords;
    jieba.CutHMM("系统abc", hmmWords);
    QVERIFY(hmmWords.empty());

    std::vector<Word> hmmWordObjects;
    jieba.CutHMM("系统abc", hmmWordObjects);
    QVERIFY(!hmmWordObjects.empty());

    std::vector<std::string> smallWords;
    jieba.CutSmall("系统开发", smallWords, 2);
    QVERIFY(smallWords.empty());

    std::vector<Word> smallWordObjects;
    jieba.CutSmall("系统开发", smallWordObjects, 2);
    QVERIFY(!smallWordObjects.empty());

    std::vector<std::pair<std::string, std::string>> tagged;
    jieba.Tag("系统 abc123", tagged);
    QVERIFY(!tagged.empty());
    QCOMPARE(jieba.LookupTag("abc123"), std::string(POS_ENG));

    jieba.ResetSeparators(SPECIAL_SEPARATORS);
    QVERIFY(jieba.GetDictTrie() != nullptr);
    QVERIFY(jieba.GetHMMModel() != nullptr);

    std::vector<std::string> keywordStrings;
    jieba.extractor.Extract("系统 编译 系统", keywordStrings, 2);
    QVERIFY(!keywordStrings.empty());

    std::vector<std::pair<std::string, double>> keywordPairs;
    jieba.extractor.Extract("系统 编译 系统", keywordPairs, 2);
    QVERIFY(!keywordPairs.empty());

    std::vector<KeyWord> keywords;
    jieba.extractor.Extract("系统 编译 系统", keywords, 2);
    QVERIFY(!keywords.empty());
    QVERIFY(std::all_of(keywords.begin(), keywords.end(), [](const KeyWord &keyword) {
        return !keyword.word.empty();
    }));
}

void CppJiebaWhiteboxTest::stringUtilCoversEncodingAndPathHelpers()
{
    std::string mixedCase = "AbC";
    QCOMPARE(limonp::Lower(mixedCase), std::string("abc"));
    QCOMPARE(limonp::Upper(mixedCase), std::string("ABC"));

    std::string spaced = " \t value \n";
    QCOMPARE(limonp::Trim(spaced), std::string("value"));

    std::string dotted = "..value..";
    QCOMPARE(limonp::Trim(dotted, '.'), std::string("value"));

    std::vector<std::string> splitValues;
    limonp::Split("a,b,c", splitValues, ",", 1);
    QCOMPARE(splitValues.size(), static_cast<size_t>(2));
    QCOMPARE(splitValues.back(), std::string("b,c"));
    limonp::Split("", splitValues, ",");
    QVERIFY(splitValues.empty());
    QCOMPARE(limonp::Split("solo", ","), std::vector<std::string>({"solo"}));

    const std::vector<std::string> joinValues{"a", "b", "c"};
    QCOMPARE(limonp::Join(joinValues.begin(), joinValues.end(), std::string("|")), std::string("a|b|c"));
    std::string emptyJoinTarget = "unchanged";
    limonp::Join(joinValues.begin(), joinValues.begin(), emptyJoinTarget, std::string("|"));
    QCOMPARE(emptyJoinTarget, std::string("unchanged"));
    QCOMPARE(limonp::Join(joinValues.begin(), joinValues.begin(), std::string("|")), std::string(""));

    QVERIFY(!limonp::StartsWith("ab", "abc"));
    QVERIFY(limonp::StartsWith("abc", "ab"));
    QVERIFY(!limonp::EndsWith("ab", "abc"));
    QVERIFY(limonp::EndsWith("abc", "bc"));
    QVERIFY(!limonp::IsInStr("ab", 'z'));
    QVERIFY(limonp::IsInStr("ab", 'a'));
    QVERIFY(!limonp::IsSpace(0x100));
    QCOMPARE(limonp::TwocharToUint16('\x12', '\x34'), static_cast<uint16_t>(0x1234));

    std::vector<uint16_t> unicode16;
    QVERIFY(!limonp::Utf8ToUnicode(static_cast<const char *>(nullptr), 0, unicode16));
    const std::string utf8Narrow = std::string("A") + std::string("\xC2\xA2", 2) + "中";
    QVERIFY(limonp::Utf8ToUnicode(utf8Narrow, unicode16));
    QCOMPARE(unicode16.size(), static_cast<size_t>(3));
    const std::string invalidUtf8 = std::string("\xe4\xb8", 2);
    QVERIFY(!limonp::Utf8ToUnicode(invalidUtf8, unicode16));

    const std::string twoByte = std::string("\xC2\xA2", 2);
    std::vector<uint32_t> unicode32;
    const std::string utf8 = std::string("A中") + std::string("\xF0\x9F\x98\x80", 4);
    QVERIFY(limonp::Utf8ToUnicode32(utf8, unicode32));
    QCOMPARE(unicode32.size(), static_cast<size_t>(3));
    QVERIFY(limonp::Utf8ToUnicode32(twoByte, unicode32));
    QCOMPARE(unicode32, std::vector<uint32_t>({0xA2}));

    std::string utf8RoundTrip;
    limonp::Unicode32ToUtf8(unicode32.begin(), unicode32.end(), utf8RoundTrip);
    QCOMPARE(utf8RoundTrip, twoByte);
    QVERIFY(limonp::Utf8ToUnicode32(utf8, unicode32));
    const std::string incompleteUtf8 = std::string("\xF0\x9F\x98", 3);
    QVERIFY(!limonp::Utf8ToUnicode32(incompleteUtf8, unicode32));

    const std::vector<uint32_t> codePoints{0x24, 0xA2, 0x20AC, 0x1F600};
    limonp::Unicode32ToUtf8(codePoints.begin(), codePoints.end(), utf8RoundTrip);
    QCOMPARE(utf8RoundTrip, std::string("$") + std::string("\xC2\xA2", 2)
             + std::string("\xE2\x82\xAC", 3) + std::string("\xF0\x9F\x98\x80", 4));

    const std::vector<uint16_t> codeUnits{0x24, 0xA2, 0x4E2D};
    std::string utf16RoundTrip;
    limonp::UnicodeToUtf8(codeUnits.begin(), codeUnits.end(), utf16RoundTrip);
    QCOMPARE(utf16RoundTrip, std::string("$") + std::string("\xC2\xA2", 2) + "中");

    QCOMPARE(limonp::UnicodeToUtf8Bytes(0x7f), 1);
    QCOMPARE(limonp::UnicodeToUtf8Bytes(0x7ff), 2);
    QCOMPARE(limonp::UnicodeToUtf8Bytes(0xffff), 3);
    QCOMPARE(limonp::UnicodeToUtf8Bytes(0x10000), 4);

    std::vector<uint16_t> gbk;
    QVERIFY(limonp::GBKTrans(static_cast<const char *>(nullptr), 0, gbk));
    QVERIFY(gbk.empty());
    QVERIFY(limonp::GBKTrans(std::string("AZ"), gbk));
    QCOMPARE(gbk, std::vector<uint16_t>({static_cast<uint16_t>('A'), static_cast<uint16_t>('Z')}));
    const std::string gbkPair = std::string("\x81\x40", 2);
    QVERIFY(limonp::GBKTrans(gbkPair, gbk));
    std::string gbkRoundTrip;
    const std::vector<uint16_t> gbkMixed{static_cast<uint16_t>('A'), gbk.front()};
    limonp::GBKTrans(gbkMixed.begin(), gbkMixed.end(), gbkRoundTrip);
    QCOMPARE(gbkRoundTrip, std::string("A") + gbkPair);

    const char incompleteGbk[] = {static_cast<char>(0x81)};
    QVERIFY(!limonp::GBKTrans(incompleteGbk, sizeof(incompleteGbk), gbk));

    QCOMPARE(limonp::PathJoin("/tmp", "cache"), std::string("/tmp/cache"));
    QCOMPARE(limonp::PathJoin("/tmp/", "cache"), std::string("/tmp/cache"));

    std::ostringstream containers;
    containers << std::deque<int>{} << std::deque<int>{1, 2}
               << std::map<std::string, int>{}
               << std::map<std::string, int>{{"a", 1}, {"c", 3}}
               << std::set<int>{} << std::set<int>{2, 3}
               << std::unordered_map<std::string, int>{}
               << std::unordered_map<std::string, int>{{"b", 2}, {"d", 4}};
    QVERIFY(containers.str().find("[]") != std::string::npos);
    QVERIFY(containers.str().find("{}") != std::string::npos);
    QVERIFY(containers.str().find("1") != std::string::npos);
    QVERIFY(containers.str().find("a:1") != std::string::npos);
    QVERIFY(containers.str().find("c:3") != std::string::npos);
    QVERIFY(containers.str().find("2") != std::string::npos);
    QVERIFY(containers.str().find("b:2") != std::string::npos);
    QVERIFY(containers.str().find("d:4") != std::string::npos);

    QTemporaryDir fileDir;
    QVERIFY(fileDir.isValid());
    const QString streamPath = fileDir.filePath(QStringLiteral("stream.txt"));
    {
        std::ofstream out(streamPath.toStdString());
        out << std::string("payload");
    }
    std::ifstream in(streamPath.toStdString());
    std::string streamed;
    in >> streamed;
    QCOMPARE(streamed, std::string("payload"));

    RuneStrArray twoByteRunes;
    QVERIFY(DecodeRunesInString(twoByte, twoByteRunes));
    QCOMPARE(twoByteRunes.size(), static_cast<size_t>(1));
    QCOMPARE(EncodeRunesToString(twoByteRunes.begin(), twoByteRunes.end()), twoByte);
    std::string encodedTwoByte;
    EncodeRunesToString(twoByteRunes.begin(), twoByteRunes.end(), encodedTwoByte);
    QCOMPARE(encodedTwoByte, twoByte);
    QVERIFY(IsSingleWord(twoByte));

    RuneStrArray fourByteRunes;
    const std::string fourByte = std::string("\xF0\x9F\x98\x80", 4);
    QVERIFY(DecodeRunesInString(fourByte, fourByteRunes));
    QCOMPARE(fourByteRunes.size(), static_cast<size_t>(1));
    QCOMPARE(fourByteRunes[0].len, static_cast<uint32_t>(4));
    QCOMPARE(Utf8CharNum(fourByte.data(), fourByte.size()), static_cast<size_t>(1));
    QVERIFY(IsSingleWord(fourByte));

    RuneStrArray invalidRunes;
    QVERIFY(!DecodeRunesInString(incompleteUtf8, invalidRunes));
    QCOMPARE(Utf8CharNum(incompleteUtf8), static_cast<size_t>(0));
}

QTEST_MAIN(CppJiebaWhiteboxTest)

#include "cppjieba-whitebox-test.moc"
