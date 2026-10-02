#include <QTest>

#include "index/ukui-search-qdbus.h"

using namespace UkuiSearch;

class UkuiSearchQdbusWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void constructAndCallUnavailableSystemServiceBranches();
};

void UkuiSearchQdbusWhiteboxTest::constructAndCallUnavailableSystemServiceBranches()
{
    UkuiSearchQDBus qdbus;

    qdbus.setInotifyMaxUserWatches();
    qdbus.addInotifyUserInstances(1);
    qdbus.addInotifyUserInstances(0);
}

QTEST_MAIN(UkuiSearchQdbusWhiteboxTest)

#include "ukui-search-qdbus-whitebox-test.moc"
