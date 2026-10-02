#include "one-result-test.h"

#include <QTest>

#include "one-result.h"

using namespace UkuiSearch;

void OneResultTest::testDefaultShowToolTipIsTrue()
{
    SearchPluginIface::ResultInfo info;
    OneResult result(info);
    QVERIFY(result.showToolTip());
}

void OneResultTest::testSetShowToolTip()
{
    SearchPluginIface::ResultInfo info;
    OneResult result(info);

    result.setShowToolTip(false);
    QVERIFY(!result.showToolTip());

    result.setShowToolTip(true);
    QVERIFY(result.showToolTip());
}

void OneResultTest::testInfoKeepsInputFields()
{
    SearchPluginIface::ResultInfo info(
        QIcon(),
        "name",
        "tip",
        "file",
        {},
        "action",
        3,
        false,
        true,
        42,
        "icon-name");

    OneResult result(info);
    const auto &stored = result.info();
    QCOMPARE(stored.name, QString("name"));
    QCOMPARE(stored.toolTip, QString("tip"));
    QCOMPARE(stored.resourceType, QString("file"));
    QCOMPARE(stored.actionKey, QString("action"));
    QCOMPARE(stored.type, 3);
    QCOMPARE(stored.showInBestMatch, false);
    QCOMPARE(stored.displayNameAsUrl, true);
    QCOMPARE(stored.searchID, static_cast<size_t>(42));
    QCOMPARE(stored.iconName, QString("icon-name"));
}

QTEST_MAIN(OneResultTest)
