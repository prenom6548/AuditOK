#include <QTest>

#include "Traditional-to-Simplified.h"
#include "Traditional-to-Simplified-private.h"

using namespace std;

class Traditional2SimplifiedTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void singletonReturnsSameInstance();
    void convertsEmptySingleAndMultipleWords();
    void privateImplementationReturnsUnknownSingleWordAndDestroysCleanly();
    void reportsTraditionalCharacters();
};

void Traditional2SimplifiedTest::singletonReturnsSameInstance()
{
    QCOMPARE(Traditional2Simplified::getInstance(), Traditional2Simplified::getInstance());
}

void Traditional2SimplifiedTest::convertsEmptySingleAndMultipleWords()
{
    Traditional2Simplified *converter = Traditional2Simplified::getInstance();

    QCOMPARE(converter->getResults(string()), string());
    QCOMPARE(converter->getResults(string("體")), string("体"));
    QCOMPARE(converter->getResults(string("龍門")), string("龙门"));
    QCOMPARE(converter->getResults(string("abc")), string("abc"));
    QCOMPARE(converter->getResults(string("龍abc門")), string("龙abc门"));
}

void Traditional2SimplifiedTest::privateImplementationReturnsUnknownSingleWordAndDestroysCleanly()
{
    Traditional2SimplifiedPrivate local;

    QCOMPARE(local.getResults(string("龘")), string("龘"));
}

void Traditional2SimplifiedTest::reportsTraditionalCharacters()
{
    Traditional2Simplified *converter = Traditional2Simplified::getInstance();

    string traditional = "龍";
    string simplified = "龙";

    QVERIFY(converter->isTraditional(traditional));
    QVERIFY(!converter->isTraditional(simplified));
}

QTEST_MAIN(Traditional2SimplifiedTest)

#include "traditional2simplified-test.moc"
