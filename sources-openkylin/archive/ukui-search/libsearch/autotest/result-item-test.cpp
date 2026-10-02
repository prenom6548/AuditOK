#include "result-item-test.h"

#include <QDateTime>
#include <QTest>

#include "result-item.h"

using namespace UkuiSearch;

void ResultItemTest::testDefaultAndSetterGetterPaths()
{
    ResultItem item;

    QCOMPARE(item.getSearchId(), static_cast<size_t>(0));
    QVERIFY(item.getItemKey().isEmpty());
    QVERIFY(!item.getValue(SearchProperty::FileName).isValid());

    item.setSearchId(17);
    item.setItemKey(QStringLiteral("key"));
    item.setValue(SearchProperty::FileName, QStringLiteral("name.txt"));

    QCOMPARE(item.getSearchId(), static_cast<size_t>(17));
    QCOMPARE(item.getItemKey(), QStringLiteral("key"));
    QCOMPARE(item.getValue(SearchProperty::FileName).toString(), QStringLiteral("name.txt"));
}

void ResultItemTest::testConstructorsAndStoredMap()
{
    ResultItem byId(static_cast<size_t>(8));
    QCOMPARE(byId.getSearchId(), static_cast<size_t>(8));

    ResultItem byKey(QStringLiteral("desktop-id"));
    QCOMPARE(byKey.getItemKey(), QStringLiteral("desktop-id"));

    SearchResultPropertyMap values;
    values.insert(SearchProperty::FilePath, QStringLiteral("/tmp/a.txt"));
    values.insert(SearchProperty::ModifiedTime, QDateTime::fromSecsSinceEpoch(100));

    ResultItem full(static_cast<size_t>(9), QStringLiteral("item-9"), values);
    QCOMPARE(full.getSearchId(), static_cast<size_t>(9));
    QCOMPARE(full.getItemKey(), QStringLiteral("item-9"));
    QCOMPARE(full.getValue(SearchProperty::FilePath).toString(), QStringLiteral("/tmp/a.txt"));
    QCOMPARE(full.getAllValue().size(), 2);
}

void ResultItemTest::testCopyAssignmentAndMoveAssignment()
{
    ResultItem source(static_cast<size_t>(5));
    source.setItemKey(QStringLiteral("copy-me"));
    source.setValue(SearchProperty::ApplicationLocalName, QStringLiteral("Calculator"));

    ResultItem copied(source);
    QCOMPARE(copied.getSearchId(), static_cast<size_t>(5));
    QCOMPARE(copied.getItemKey(), QStringLiteral("copy-me"));
    QCOMPARE(copied.getValue(SearchProperty::ApplicationLocalName).toString(), QStringLiteral("Calculator"));

    ResultItem assigned;
    assigned = source;
    QCOMPARE(assigned.getSearchId(), static_cast<size_t>(5));
    QCOMPARE(assigned.getItemKey(), QStringLiteral("copy-me"));

    ResultItem moved;
    moved = ResultItem(static_cast<size_t>(12), QStringLiteral("move-me"), source.getAllValue());
    QCOMPARE(moved.getSearchId(), static_cast<size_t>(12));
    QCOMPARE(moved.getItemKey(), QStringLiteral("move-me"));
    QCOMPARE(moved.getValue(SearchProperty::ApplicationLocalName).toString(), QStringLiteral("Calculator"));
}

void ResultItemTest::testCopyAssignmentOverwritesExistingData()
{
    ResultItem source(static_cast<size_t>(21));
    source.setItemKey(QStringLiteral("fresh"));
    source.setValue(SearchProperty::FilePath, QStringLiteral("/tmp/fresh"));

    ResultItem target(static_cast<size_t>(1));
    target.setItemKey(QStringLiteral("stale"));
    target.setValue(SearchProperty::FileName, QStringLiteral("stale.txt"));

    target = source;

    QCOMPARE(target.getSearchId(), static_cast<size_t>(21));
    QCOMPARE(target.getItemKey(), QStringLiteral("fresh"));
    QCOMPARE(target.getValue(SearchProperty::FilePath).toString(), QStringLiteral("/tmp/fresh"));
    QVERIFY(!target.getValue(SearchProperty::FileName).isValid());
}

QTEST_MAIN(ResultItemTest)
