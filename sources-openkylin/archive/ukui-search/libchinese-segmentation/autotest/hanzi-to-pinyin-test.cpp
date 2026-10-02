#include <QTest>

#include "hanzi-to-pinyin.h"
#include "hanzi-to-pinyin-private.h"

using namespace std;

class HanZiToPinYinTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void containsAndMultitoneOverloads();
    void convertsResultStyles();
    void convertsMultiCharacterWords();
    void convertsPolyphoneWords();
    void handlesUnknownTextByConfiguration();
    void fallsBackToSegmentationAndExtraDataPolicy();
    void segmentedFallbackCoversPolyphoneWordsAndSingleCharacterResults();
    void segmentedFallbackCoversSingleKnownWordsAndStyleSkips();
    void segmentedFallbackSplitsUnknownAsciiWords();
    void segmentedFallbackHandlesSingleToneAndSplitKnownCharacters();
    void defaultStyleSkipsLiteralComma();
    void privateImplementationCanBeDestroyed();
};

void HanZiToPinYinTest::containsAndMultitoneOverloads()
{
    HanZiToPinYin *pinyin = HanZiToPinYin::getInstance();

    string oneWord = "中";
    string missing = "不存在";
    const string constWord = "中";

    QVERIFY(pinyin->contains(oneWord));
    QVERIFY(!pinyin->contains(missing));
    QVERIFY(!pinyin->isMultiTone(oneWord));
    QVERIFY(!pinyin->isMultiTone(string("中")));
    QVERIFY(!pinyin->isMultiTone(constWord));
    QVERIFY(!pinyin->isMultiTone(static_cast<const string &&>(string("中"))));
}

void HanZiToPinYinTest::convertsResultStyles()
{
    HanZiToPinYin *pinyin = HanZiToPinYin::getInstance();
    QStringList results;

    pinyin->setConfig(PinyinDataStyle::Tone, SegType::NoSegmentation,
                      PolyphoneType::Enable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("中", results), 0);
    QVERIFY(results.contains(QStringLiteral("zhōng,zhòng")));

    pinyin->setConfig(PinyinDataStyle::Default, SegType::NoSegmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("中", results), 0);
    QCOMPARE(results, QStringList{QStringLiteral("zhong")});

    pinyin->setConfig(PinyinDataStyle::Tone2, SegType::NoSegmentation,
                      PolyphoneType::Enable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("中", results), 0);
    QVERIFY(results.contains(QStringLiteral("zho1ng,zho4ng")));

    pinyin->setConfig(PinyinDataStyle::Tone3, SegType::NoSegmentation,
                      PolyphoneType::Enable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("中", results), 0);
    QVERIFY(results.contains(QStringLiteral("zhong1,zhong4")));

    pinyin->setConfig(PinyinDataStyle::FirstLetter, SegType::NoSegmentation,
                      PolyphoneType::Enable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("中", results), 0);
    QCOMPARE(results, QStringList{QStringLiteral("z,z")});

    pinyin->setConfig(PinyinDataStyle::English, SegType::NoSegmentation,
                      PolyphoneType::Enable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("中", results), 0);
    QVERIFY(results.contains(QStringLiteral("zhōng,zhòng")));
}

void HanZiToPinYinTest::convertsMultiCharacterWords()
{
    HanZiToPinYin *pinyin = HanZiToPinYin::getInstance();
    QStringList results;

    pinyin->setConfig(PinyinDataStyle::Default, SegType::NoSegmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("中国", results), 0);
    QCOMPARE(results, QStringList({QStringLiteral("zhong"), QStringLiteral("guo")}));

    pinyin->setConfig(PinyinDataStyle::Tone, SegType::NoSegmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("中国", results), 0);
    QCOMPARE(results, QStringList({QStringLiteral("zhōng"), QStringLiteral("guó")}));

    pinyin->setConfig(PinyinDataStyle::FirstLetter, SegType::NoSegmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("中国", results), 0);
    QCOMPARE(results, QStringList({QStringLiteral("z"), QStringLiteral("g")}));
}

void HanZiToPinYinTest::convertsPolyphoneWords()
{
    HanZiToPinYin *pinyin = HanZiToPinYin::getInstance();
    QStringList results;

    pinyin->setConfig(PinyinDataStyle::Tone, SegType::NoSegmentation,
                      PolyphoneType::Enable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("朝阳", results), 0);
    QVERIFY(results.contains(QStringLiteral("zhāo/cháo")));
    QVERIFY(results.contains(QStringLiteral("yáng/yáng")));

    pinyin->setConfig(PinyinDataStyle::Default, SegType::NoSegmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("朝阳", results), 0);
    QCOMPARE(results, QStringList({QStringLiteral("zhao"), QStringLiteral("yang")}));

    pinyin->setConfig(PinyinDataStyle::Tone3, SegType::NoSegmentation,
                      PolyphoneType::Enable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("朝阳", results), 0);
    QVERIFY(results.contains(QStringLiteral("zhao1/chao2")));
    QVERIFY(results.contains(QStringLiteral("yang2/yang2")));

    pinyin->setConfig(PinyinDataStyle::FirstLetter, SegType::NoSegmentation,
                      PolyphoneType::Enable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("朝阳", results), 0);
    QVERIFY(results.contains(QStringLiteral("z/c")));
    QVERIFY(std::any_of(results.begin(), results.end(), [](const QString &item) {
        return item.startsWith(QLatin1Char('y'));
    }));
}

void HanZiToPinYinTest::handlesUnknownTextByConfiguration()
{
    HanZiToPinYin *pinyin = HanZiToPinYin::getInstance();
    QStringList results;

    pinyin->setConfig(PinyinDataStyle::Default, SegType::NoSegmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("###", results), -1);
    QVERIFY(results.isEmpty());

    pinyin->setConfig(PinyinDataStyle::Default, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("###", results), 0);

    pinyin->setConfig(PinyinDataStyle::Default, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Delete);
    QCOMPARE(pinyin->getResults("###", results), 0);
    QVERIFY(results.isEmpty());
}

void HanZiToPinYinTest::fallsBackToSegmentationAndExtraDataPolicy()
{
    HanZiToPinYin *pinyin = HanZiToPinYin::getInstance();
    QStringList results;

    pinyin->setConfig(PinyinDataStyle::Default, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("银河麒麟", results), 0);
    QCOMPARE(results, QStringList({QStringLiteral("yin"), QStringLiteral("he"),
                                   QStringLiteral("qi"), QStringLiteral("lin")}));

    pinyin->setConfig(PinyinDataStyle::Default, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("###木头人", results), 0);
    QVERIFY(results.contains(QStringLiteral("#")));
    QVERIFY(results.contains(QStringLiteral("mu")));
    QVERIFY(results.contains(QStringLiteral("tou")));
    QVERIFY(results.contains(QStringLiteral("ren")));

    pinyin->setConfig(PinyinDataStyle::Default, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Delete);
    QCOMPARE(pinyin->getResults("###木头人", results), 0);
    QVERIFY(!results.contains(QStringLiteral("#")));
    QVERIFY(results.contains(QStringLiteral("mu")));
    QVERIFY(results.contains(QStringLiteral("tou")));
    QVERIFY(results.contains(QStringLiteral("ren")));
}

void HanZiToPinYinTest::segmentedFallbackCoversPolyphoneWordsAndSingleCharacterResults()
{
    HanZiToPinYin *pinyin = HanZiToPinYin::getInstance();
    QStringList results;

    pinyin->setConfig(PinyinDataStyle::Tone3, SegType::Segmentation,
                      PolyphoneType::Enable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("###朝阳", results), 0);
    QVERIFY(results.contains(QStringLiteral("#")));
    QVERIFY(results.contains(QStringLiteral("zhao1/chao2")));
    QVERIFY(results.contains(QStringLiteral("yang2/yang2")));

    pinyin->setConfig(PinyinDataStyle::Default, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Delete);
    QCOMPARE(pinyin->getResults("###朝阳", results), 0);
    QVERIFY(!results.contains(QStringLiteral("#")));
    QCOMPARE(results, QStringList({QStringLiteral("zhao"), QStringLiteral("yang")}));

    pinyin->setConfig(PinyinDataStyle::Default, SegType::NoSegmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("国", results), 0);
    QCOMPARE(results, QStringList{QStringLiteral("guo")});

    pinyin->setConfig(PinyinDataStyle::English, SegType::NoSegmentation,
                      PolyphoneType::Enable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("朝阳", results), 0);
    QVERIFY(results.contains(QStringLiteral("zhāo/cháo")));
    QVERIFY(results.contains(QStringLiteral("yáng/yáng")));
}

void HanZiToPinYinTest::segmentedFallbackCoversSingleKnownWordsAndStyleSkips()
{
    HanZiToPinYin *pinyin = HanZiToPinYin::getInstance();
    QStringList results;

    pinyin->setConfig(PinyinDataStyle::Tone, SegType::Segmentation,
                      PolyphoneType::Enable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("###中", results), 0);
    QVERIFY(results.contains(QStringLiteral("zhōng,zhòng")));

    pinyin->setConfig(PinyinDataStyle::Default, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("###中", results), 0);
    QVERIFY(results.contains(QStringLiteral("zhong")));

    pinyin->setConfig(PinyinDataStyle::Default, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("😀🙃", results), 0);
    QVERIFY(!results.isEmpty());

    pinyin->setConfig(PinyinDataStyle::Default, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Delete);
    QCOMPARE(pinyin->getResults("😀🙃", results), 0);
    QVERIFY(results.isEmpty());

    pinyin->setConfig(PinyinDataStyle::Tone3, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("/", results), 0);
    QCOMPARE(results, QStringList{QStringLiteral("/")});

    pinyin->setConfig(PinyinDataStyle::FirstLetter, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults(",", results), 0);
    QCOMPARE(results, QStringList{QStringLiteral(",")});
}

void HanZiToPinYinTest::segmentedFallbackSplitsUnknownAsciiWords()
{
    HanZiToPinYin *pinyin = HanZiToPinYin::getInstance();
    QStringList results;

    pinyin->setConfig(PinyinDataStyle::Default, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("abc", results), 0);
    QVERIFY(results.contains(QStringLiteral("a")));
    QVERIFY(results.contains(QStringLiteral("b")));
    QVERIFY(results.contains(QStringLiteral("c")));

    pinyin->setConfig(PinyinDataStyle::Default, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Delete);
    QCOMPARE(pinyin->getResults("abc", results), 0);
    QVERIFY(results.isEmpty());
}

void HanZiToPinYinTest::segmentedFallbackHandlesSingleToneAndSplitKnownCharacters()
{
    HanZiToPinYin *pinyin = HanZiToPinYin::getInstance();
    QStringList results;

    pinyin->setConfig(PinyinDataStyle::Default, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("###国", results), 0);
    QVERIFY(results.contains(QStringLiteral("guo")));

    pinyin->setConfig(PinyinDataStyle::Default, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("中重", results), 0);
    QVERIFY(results.contains(QStringLiteral("zhong")));

    pinyin->setConfig(PinyinDataStyle::Tone, SegType::Segmentation,
                      PolyphoneType::Enable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults("中重", results), 0);
    QVERIFY(results.contains(QStringLiteral("zhōng,zhòng")));
}

void HanZiToPinYinTest::defaultStyleSkipsLiteralComma()
{
    HanZiToPinYin *pinyin = HanZiToPinYin::getInstance();
    QStringList results;

    pinyin->setConfig(PinyinDataStyle::Default, SegType::Segmentation,
                      PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(pinyin->getResults(",", results), 0);
    QCOMPARE(results, QStringList{QStringLiteral(",")});
}

void HanZiToPinYinTest::privateImplementationCanBeDestroyed()
{
    HanZiToPinYinPrivate local;
    QStringList results;
    string word = "国";

    local.setConfig(PinyinDataStyle::Default, SegType::NoSegmentation,
                    PolyphoneType::Disable, ExDataProcessType::Default);
    QCOMPARE(local.getResults(word, results), 0);
    QCOMPARE(results, QStringList{QStringLiteral("guo")});
}

QTEST_MAIN(HanZiToPinYinTest)

#include "hanzi-to-pinyin-test.moc"
