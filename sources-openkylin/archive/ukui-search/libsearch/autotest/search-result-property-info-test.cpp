#include "search-result-property-info-test.h"

#include <QTest>

#include "search-result-property-info.h"

using namespace UkuiSearch;

void SearchResultPropertyInfoTest::testDefaultState()
{
    SearchResultPropertyInfo info;

    QCOMPARE(info.property(), SearchProperty::Invalid);
    QCOMPARE(info.name(), QStringLiteral("Invalid"));
    QVERIFY(info.displayName().isEmpty());
}

void SearchResultPropertyInfoTest::testKnownProperties()
{
    const QList<QPair<SearchProperty::SearchResultProperty, QString>> cases = {
        {SearchProperty::FilePath, QStringLiteral("FilePath")},
        {SearchProperty::FileName, QStringLiteral("FileName")},
        {SearchProperty::FileIconName, QStringLiteral("FileIconName")},
        {SearchProperty::ModifiedTime, QStringLiteral("ModifiedTime")},
        {SearchProperty::ApplicationDesktopPath, QStringLiteral("ApplicationDesktopPath")},
        {SearchProperty::ApplicationLocalName, QStringLiteral("ApplicationLocalName")},
        {SearchProperty::ApplicationIconName, QStringLiteral("ApplicationIconName")},
        {SearchProperty::ApplicationDescription, QStringLiteral("ApplicationDescription")},
        {SearchProperty::IsOnlineApplication, QStringLiteral("IsOnlineApplication")},
        {SearchProperty::ApplicationPkgName, QStringLiteral("ApplicationPkgName")},
    };

    for (const auto &oneCase : cases) {
        SearchResultPropertyInfo info(oneCase.first);
        QCOMPARE(info.property(), oneCase.first);
        QCOMPARE(info.name(), oneCase.second);
        QVERIFY(!info.displayName().isEmpty());
    }
}

void SearchResultPropertyInfoTest::testCopyAssignmentAndEquality()
{
    SearchResultPropertyInfo left(SearchProperty::ApplicationDescription);
    SearchResultPropertyInfo copied(left);
    QVERIFY(left == copied);

    SearchResultPropertyInfo assigned;
    assigned = left;
    QVERIFY(left == assigned);

    SearchResultPropertyInfo different(SearchProperty::FileIconName);
    QVERIFY(!(left == different));
}

void SearchResultPropertyInfoTest::testUnknownPropertyFallsBackToEmptyNames()
{
    SearchResultPropertyInfo info(static_cast<SearchProperty::SearchResultProperty>(999));

    QCOMPARE(info.property(), static_cast<SearchProperty::SearchResultProperty>(999));
    QVERIFY(info.name().isEmpty());
    QVERIFY(info.displayName().isEmpty());
}

QTEST_MAIN(SearchResultPropertyInfoTest)
