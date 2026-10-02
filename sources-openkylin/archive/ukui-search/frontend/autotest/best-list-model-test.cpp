#define private public
#include "best-list-model.h"
#undef private

#include "best-list-model-test.h"

#include <QSignalSpy>
#include <QTest>

using namespace UkuiSearch;

namespace {
SearchPluginIface::ResultInfo makeBestInfo(const QString &name,
                                           const QString &resourceType,
                                           const QString &actionKey,
                                           const bool showInBest = true)
{
    return SearchPluginIface::ResultInfo(
        QIcon(),
        name,
        QStringLiteral("tip-") + name,
        resourceType,
        {},
        actionKey,
        1,
        showInBest,
        false,
        0,
        QStringLiteral("icon-") + name);
}

void addRawItem(BestListModel &model,
                const QString &pluginId,
                const SearchPluginIface::ResultInfo &info)
{
    model.m_pluginIdList.append(pluginId);
    model.m_resourceTypes.append(info.resourceType);
    model.m_items.append(new OneResult(info));
}
}

void BestListModelTest::testDataAndSetData()
{
    BestListModel model;
    addRawItem(model,
               QStringLiteral("File"),
               makeBestInfo(QStringLiteral("shown"),
                            QStringLiteral("file"),
                            QStringLiteral("open")));

    const QModelIndex idx = model.index(0, 0);
    QVERIFY(idx.isValid());
    QCOMPARE(model.data(idx, Qt::DisplayRole).toString(), QStringLiteral("shown"));
    QCOMPARE(model.data(idx, Qt::ToolTipRole).toString(), QStringLiteral("tip-shown"));
    QCOMPARE(model.data(idx, AdditionalRoles::ShowToolTip).toBool(), true);
    QCOMPARE(model.data(idx, AdditionalRoles::DisplayNameAsUrl).toBool(), false);
    QVERIFY(!model.data(idx, Qt::UserRole + 99).isValid());
    QCOMPARE(model.getPluginInfo(idx), QStringLiteral("File"));
    QCOMPARE(model.getInfo(idx).actionKey, QStringLiteral("open"));

    QVERIFY(model.setData(idx, false, ShowToolTip));
    QCOMPARE(model.data(idx, AdditionalRoles::ShowToolTip).toBool(), false);
    QVERIFY(!model.setData(QModelIndex(), true, ShowToolTip));
    QVERIFY(!model.setData(idx, true, Qt::DisplayRole));
}

void BestListModelTest::testAdditionalRolesAndCounts()
{
    BestListModel model;
    addRawItem(model,
               QStringLiteral("File"),
               makeBestInfo(QStringLiteral("shown"),
                            QStringLiteral("file"),
                            QStringLiteral("open")));

    const QModelIndex idx = model.index(0, 0);
    QVERIFY(idx.isValid());
    QVERIFY(model.data(idx, Qt::DecorationRole).value<QIcon>().isNull());
    QCOMPARE(model.columnCount(idx), 0);
}

void BestListModelTest::testExpandedAndCollapsedRowCount()
{
    BestListModel model;
    for (int i = 0; i < 7; ++i) {
        addRawItem(model,
                   QStringLiteral("Plugin%1").arg(i),
                   makeBestInfo(QStringLiteral("name%1").arg(i),
                                QStringLiteral("type%1").arg(i),
                                QStringLiteral("action%1").arg(i)));
    }

    QCOMPARE(model.rowCount(QModelIndex()), NUM_LIMIT_SHOWN_DEFAULT);
    QVERIFY(!model.isExpanded());

    QSignalSpy itemListSpy(&model, SIGNAL(itemListChanged(int)));
    QVERIFY(itemListSpy.isValid());
    model.setExpanded(true);
    QCOMPARE(model.rowCount(QModelIndex()), 7);
    QVERIFY(model.isExpanded());
    QCOMPARE(itemListSpy.last().at(0).toInt(), 7);

    model.setExpanded(false);
    QCOMPARE(model.rowCount(QModelIndex()), NUM_LIMIT_SHOWN_DEFAULT);
}

void BestListModelTest::testAppendInfoIgnoredWhenNotBestMatch()
{
    BestListModel model;
    model.appendInfo(QStringLiteral("File"),
                     makeBestInfo(QStringLiteral("hidden"),
                                  QStringLiteral("file"),
                                  QStringLiteral("open"),
                                  false));

    QVERIFY(model.m_items.isEmpty());
    QVERIFY(model.m_pluginIdList.isEmpty());
    QVERIFY(model.m_resourceTypes.isEmpty());
}

void BestListModelTest::testAppendInfoSkipsDuplicateResourceAndAction()
{
    BestListModel model;
    addRawItem(model,
               QStringLiteral("File"),
               makeBestInfo(QStringLiteral("existing"),
                            QStringLiteral("file"),
                            QStringLiteral("open")));

    QSignalSpy itemListSpy(&model, SIGNAL(itemListChanged(int)));
    QVERIFY(itemListSpy.isValid());

    model.appendInfo(QStringLiteral("Other"),
                     makeBestInfo(QStringLiteral("duplicate"),
                                  QStringLiteral("file"),
                                  QStringLiteral("open")));

    QCOMPARE(model.m_items.size(), 1);
    QCOMPARE(model.data(model.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("existing"));
    QCOMPARE(itemListSpy.count(), 0);
}

void BestListModelTest::testAppendInfoReplacesExistingPluginResult()
{
    BestListModel model;
    addRawItem(model,
               QStringLiteral("File"),
               makeBestInfo(QStringLiteral("old"),
                            QStringLiteral("file"),
                            QStringLiteral("open-old")));

    model.appendInfo(QStringLiteral("File"),
                     makeBestInfo(QStringLiteral("same-action"),
                                  QStringLiteral("new-file"),
                                  QStringLiteral("open-old")));
    QCOMPARE(model.m_items.size(), 1);
    QCOMPARE(model.data(model.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("old"));

    model.appendInfo(QStringLiteral("File"),
                     makeBestInfo(QStringLiteral("new"),
                                  QStringLiteral("new-file"),
                                  QStringLiteral("open-new")));
    QCOMPARE(model.m_items.size(), 1);
    QCOMPARE(model.data(model.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("new"));
    QCOMPARE(model.getInfo(model.index(0, 0)).actionKey, QStringLiteral("open-new"));
}

void BestListModelTest::testMoveInfoReordersItems()
{
    BestListModel model;
    addRawItem(model,
               QStringLiteral("A"),
               makeBestInfo(QStringLiteral("a"),
                            QStringLiteral("type-a"),
                            QStringLiteral("open-a")));
    addRawItem(model,
               QStringLiteral("B"),
               makeBestInfo(QStringLiteral("b"),
                            QStringLiteral("type-b"),
                            QStringLiteral("open-b")));
    addRawItem(model,
               QStringLiteral("C"),
               makeBestInfo(QStringLiteral("c"),
                            QStringLiteral("type-c"),
                            QStringLiteral("open-c")));

    model.moveInfo(QStringLiteral("C"), 1);
    QCOMPARE(model.getPluginInfo(model.index(0, 0)), QStringLiteral("C"));
    QCOMPARE(model.data(model.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("c"));

    model.moveInfo(QStringLiteral("C"), 99);
    QCOMPARE(model.getPluginInfo(model.index(2, 0)), QStringLiteral("C"));

    model.moveInfo(QStringLiteral("missing"), 1);
    QCOMPARE(model.m_items.size(), 3);
}

void BestListModelTest::testRemoveInfoAndStopSearchSlot()
{
    BestListModel model;
    addRawItem(model,
               QStringLiteral("File"),
               makeBestInfo(QStringLiteral("file"),
                            QStringLiteral("file"),
                            QStringLiteral("open-file")));
    addRawItem(model,
               QStringLiteral("App"),
               makeBestInfo(QStringLiteral("app"),
                            QStringLiteral("app"),
                            QStringLiteral("open-app")));

    QSignalSpy itemListSpy(&model, SIGNAL(itemListChanged(int)));
    QVERIFY(itemListSpy.isValid());
    model.removeInfo(QStringLiteral("File"));
    QCOMPARE(model.m_items.size(), 1);
    QCOMPARE(model.getPluginInfo(model.index(0, 0)), QStringLiteral("App"));
    QCOMPARE(itemListSpy.last().at(0).toInt(), 1);

    model.removeInfo(QStringLiteral("missing"));
    QCOMPARE(model.m_items.size(), 1);

    model.startSearch(QStringLiteral("keyword"));
    QCOMPARE(model.m_items.size(), 0);
    QCOMPARE(model.m_pluginIdList.size(), 0);
    QCOMPARE(model.m_resourceTypes.size(), 0);

    model.startSearch(QStringLiteral("keyword"));
    QCOMPARE(model.m_items.size(), 0);

    addRawItem(model,
               QStringLiteral("App"),
               makeBestInfo(QStringLiteral("app"),
                            QStringLiteral("app"),
                            QStringLiteral("open-app")));

    model.stopSearchSlot();
    QCOMPARE(model.m_items.size(), 0);
    QCOMPARE(model.m_pluginIdList.size(), 0);
    QCOMPARE(model.m_resourceTypes.size(), 0);
    QCOMPARE(itemListSpy.last().at(0).toInt(), 0);
}

void BestListModelTest::testInvalidIndexAndParentBehaviour()
{
    BestListModel model;
    addRawItem(model,
               QStringLiteral("File"),
               makeBestInfo(QStringLiteral("file"),
                            QStringLiteral("file"),
                            QStringLiteral("open")));

    QVERIFY(!model.index(-1, 0).isValid());
    QVERIFY(!model.index(1, 0).isValid());
    QVERIFY(!model.parent(model.index(0, 0)).isValid());
    QCOMPARE(model.columnCount(QModelIndex()), 1);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);
}

QTEST_MAIN(BestListModelTest)
