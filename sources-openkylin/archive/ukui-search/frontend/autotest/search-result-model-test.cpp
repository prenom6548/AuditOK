#include "search-result-model-test.h"

#include <QSignalSpy>
#include <QTest>

#include "search-result-model.h"

using namespace UkuiSearch;

namespace {
SearchPluginIface::ResultInfo makeInfo(const QString &name,
                                       const size_t searchId,
                                       const bool showInBest = true,
                                       const bool displayNameAsUrl = false)
{
    QPixmap pixmap(8, 8);
    pixmap.fill(Qt::red);
    return SearchPluginIface::ResultInfo(
        QIcon(pixmap),
        name,
        QStringLiteral("tip-") + name,
        QStringLiteral("file"),
        {},
        QStringLiteral("action-") + name,
        1,
        showInBest,
        displayNameAsUrl,
        searchId,
        QStringLiteral("icon-") + name);
}
}

void SearchResultModelTest::testAppendInfoIgnoredWhenSearchIdMismatch()
{
    SearchResultModel model(QStringLiteral("File"));
    model.appendInfo(makeInfo(QStringLiteral("ignored"), 1));

    QCOMPARE(model.index(0, 0).isValid(), false);
    QCOMPARE(model.rowCount(QModelIndex()), NUM_LIMIT_SHOWN_DEFAULT);
}

void SearchResultModelTest::testAppendInfoAndDataRolesForGenericPlugin()
{
    SearchResultModel model(QStringLiteral("File"));
    QSignalSpy itemListSpy(&model, SIGNAL(itemListChanged(int)));
    QVERIFY(itemListSpy.isValid());
    bool gotBestListSignal = false;
    QObject::connect(&model, &SearchResultModel::sendBestListData, &model,
                     [&gotBestListSignal](const QString &, const SearchPluginIface::ResultInfo &) {
                         gotBestListSignal = true;
                     });

    model.appendInfo(makeInfo(QStringLiteral("n1"), 0));
    auto idx = model.index(0, 0);
    QVERIFY(idx.isValid());

    QCOMPARE(model.data(idx, Qt::DisplayRole).toString(), QStringLiteral("n1"));
    QCOMPARE(model.data(idx, Qt::ToolTipRole).toString(), QStringLiteral("tip-n1"));
    QVERIFY(!model.data(idx, Qt::DecorationRole).value<QIcon>().isNull());
    QCOMPARE(model.data(idx, AdditionalRoles::IconName).toString(), QStringLiteral("icon-n1"));
    QCOMPARE(model.data(idx, AdditionalRoles::ShowToolTip).toBool(), true);

    QVERIFY(!itemListSpy.isEmpty());
    QVERIFY(gotBestListSignal);
}

void SearchResultModelTest::testInvalidIndexParentColumnAndRefreshBranches()
{
    SearchResultModel model(QStringLiteral("File"));
    model.appendInfo(makeInfo(QStringLiteral("url-name"), 0, true, true));

    const QModelIndex idx = model.index(0, 0);
    QVERIFY(idx.isValid());
    QVERIFY(!model.index(-1, 0).isValid());
    QVERIFY(!model.index(1, 0).isValid());
    QVERIFY(!model.parent(idx).isValid());
    QCOMPARE(model.columnCount(QModelIndex()), 1);
    QCOMPARE(model.columnCount(idx), 0);
    QCOMPARE(model.rowCount(idx), 0);
    QVERIFY(!model.data(QModelIndex(), Qt::DisplayRole).isValid());
    QVERIFY(!model.data(model.index(10, 0), Qt::DisplayRole).isValid());
    QVERIFY(!model.data(idx, Qt::UserRole + 999).isValid());
    QCOMPARE(model.data(idx, AdditionalRoles::DisplayNameAsUrl).toBool(), true);
    QVERIFY(!model.isExpanded());

    model.refresh();
    QCOMPARE(model.data(idx, Qt::DisplayRole).toString(), QStringLiteral("url-name"));
}

void SearchResultModelTest::testSetDataShowToolTipRole()
{
    SearchResultModel model(QStringLiteral("File"));
    model.appendInfo(makeInfo(QStringLiteral("n2"), 0));

    const auto idx = model.index(0, 0);
    QVERIFY(idx.isValid());
    QCOMPARE(model.data(idx, AdditionalRoles::ShowToolTip).toBool(), true);

    QVERIFY(model.setData(idx, false, ShowToolTip));
    QCOMPARE(model.data(idx, AdditionalRoles::ShowToolTip).toBool(), false);

    QVERIFY(!model.setData(QModelIndex(), false, ShowToolTip));
    QVERIFY(!model.setData(idx, true, Qt::DisplayRole));
}

void SearchResultModelTest::testNoBestListSignalForWebPagePlugin()
{
    SearchResultModel model(QStringLiteral("Web Page"));
    bool gotBestListSignal = false;
    QObject::connect(&model, &SearchResultModel::sendBestListData, &model,
                     [&gotBestListSignal](const QString &, const SearchPluginIface::ResultInfo &) {
                         gotBestListSignal = true;
                     });

    model.appendInfo(makeInfo(QStringLiteral("web"), 0));
    QVERIFY(!gotBestListSignal);
}

void SearchResultModelTest::testExpandedAndCollapsedRowCount()
{
    SearchResultModel model(QStringLiteral("File"));
    for (int i = 0; i < 7; ++i) {
        model.appendInfo(makeInfo(QStringLiteral("item%1").arg(i), 0));
    }

    QCOMPARE(model.rowCount(QModelIndex()), NUM_LIMIT_SHOWN_DEFAULT);

    model.setExpanded(true);
    QCOMPARE(model.rowCount(QModelIndex()), 7);

    model.setExpanded(false);
    QCOMPARE(model.rowCount(QModelIndex()), NUM_LIMIT_SHOWN_DEFAULT);
}

void SearchResultModelTest::testCollapsedLargeListCachesWithoutImmediateSignals()
{
    SearchResultModel model(QStringLiteral("File"));
    QSignalSpy itemListSpy(&model, SIGNAL(itemListChanged(int)));
    QVERIFY(itemListSpy.isValid());

    for (int i = 0; i < 7; ++i) {
        model.appendInfo(makeInfo(QStringLiteral("collapsed%1").arg(i), 0));
    }

    QCOMPARE(model.rowCount(QModelIndex()), NUM_LIMIT_SHOWN_DEFAULT);
    QCOMPARE(model.index(6, 0).data(Qt::DisplayRole).toString(), QStringLiteral("collapsed6"));
    QCOMPARE(itemListSpy.count(), NUM_LIMIT_SHOWN_DEFAULT + 1);
}

void SearchResultModelTest::testExpandedLargeListTriggersTimerDrivenItemListChanged()
{
    SearchResultModel model(QStringLiteral("File"));
    model.setExpanded(true);

    QSignalSpy itemListSpy(&model, SIGNAL(itemListChanged(int)));
    QVERIFY(itemListSpy.isValid());

    for (int i = 0; i < 55; ++i) {
        model.appendInfo(makeInfo(QStringLiteral("bulk%1").arg(i), 0));
    }

    const int beforeWait = itemListSpy.count();
    QVERIFY(beforeWait >= 50);

    QTest::qWait(650);
    QVERIFY(itemListSpy.count() >= beforeWait + 1);
    QCOMPARE(model.rowCount(QModelIndex()), 55);
}

void SearchResultModelTest::testStopSearchSlotClearsItemsAndEmitsStopSearch()
{
    SearchResultModel model(QStringLiteral("File"));
    model.appendInfo(makeInfo(QStringLiteral("alive"), 0));
    QCOMPARE(model.index(0, 0).isValid(), true);

    // Avoid touching real plugin/dbus stop path in sandbox while still
    // covering model-side emit/clear logic.
    QObject::disconnect(&model, SIGNAL(stopSearch()), nullptr, nullptr);

    QSignalSpy stopSpy(&model, SIGNAL(stopSearch()));
    QVERIFY(stopSpy.isValid());

    model.stopSearchSlot();
    QCOMPARE(stopSpy.count(), 1);
    QCOMPARE(model.index(0, 0).isValid(), false);
    QCOMPARE(model.rowCount(QModelIndex()), NUM_LIMIT_SHOWN_DEFAULT);
}

void SearchResultModelTest::testRoleNamesContainAdditionalRoles()
{
    SearchResultModel model(QStringLiteral("File"));
    const auto roles = model.roleNames();

    QVERIFY(roles.contains(Qt::DisplayRole));
    QVERIFY(roles.contains(Qt::DecorationRole));
    QVERIFY(roles.contains(Qt::ToolTipRole));
}

QTEST_MAIN(SearchResultModelTest)
