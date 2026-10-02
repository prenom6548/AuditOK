#include "search-controller-test.h"

#include <QTest>

#include "result-item.h"
#include "search-controller.h"

using namespace UkuiSearch;

void SearchControllerTest::testDefaultsAndConditionSetters()
{
    SearchController controller;

    QVERIFY(controller.getSearchDir().isEmpty());
    QVERIFY(controller.getKeyword().isEmpty());
    QVERIFY(controller.getFileLabel().isEmpty());
    QVERIFY(controller.isRecurse());
    QVERIFY(!controller.getMatchAllIfNoKeyword());
    QVERIFY(!controller.isKeywordSegmentationActived());
    QVERIFY(!controller.isSearchFileOnly());
    QVERIFY(!controller.isSearchDirOnly());
    QVERIFY(!controller.isSearchOnlineApps());
    QVERIFY(!controller.searchHiddenFiles());

    controller.addSearchDir(QStringLiteral("/tmp/a"));
    controller.addKeyword(QStringLiteral("keyword"));
    controller.addFileLabel(QStringLiteral("label"));
    controller.setRecurse(false);
    controller.setMatchAllIfNoKeyword(true);
    controller.setActiveKeywordSegmentation(true);
    controller.setOnlySearchFile(true);
    controller.setOnlySearchDir(true);
    controller.setSearchOnlineApps(true);
    controller.setSearchHiddenFiles(true);

    QCOMPARE(controller.getSearchDir(), QStringList{QStringLiteral("/tmp/a")});
    QCOMPARE(controller.getKeyword(), QStringList{QStringLiteral("keyword")});
    QCOMPARE(controller.getFileLabel(), QStringList{QStringLiteral("label")});
    QVERIFY(!controller.isRecurse());
    QVERIFY(controller.getMatchAllIfNoKeyword());
    QVERIFY(controller.isKeywordSegmentationActived());
    QVERIFY(controller.isSearchFileOnly());
    QVERIFY(controller.isSearchDirOnly());
    QVERIFY(controller.isSearchOnlineApps());
    QVERIFY(controller.searchHiddenFiles());
}

void SearchControllerTest::testClearMethods()
{
    SearchController controller;
    controller.addSearchDir(QStringLiteral("/tmp/a"));
    controller.addKeyword(QStringLiteral("keyword"));
    controller.addFileLabel(QStringLiteral("label"));

    controller.clearKeyWords();
    controller.clearSearchDir();
    controller.clearFileLabel();

    QVERIFY(controller.getKeyword().isEmpty());
    QVERIFY(controller.getSearchDir().isEmpty());
    QVERIFY(controller.getFileLabel().isEmpty());

    controller.addSearchDir(QStringLiteral("/tmp/b"));
    controller.addKeyword(QStringLiteral("again"));
    controller.addFileLabel(QStringLiteral("again-label"));
    controller.setRecurse(false);
    controller.setMatchAllIfNoKeyword(true);
    controller.setActiveKeywordSegmentation(true);
    controller.setOnlySearchFile(true);
    controller.setOnlySearchDir(true);
    controller.setSearchOnlineApps(true);

    controller.clearAllConditions();

    QVERIFY(controller.getKeyword().isEmpty());
    QVERIFY(controller.getSearchDir().isEmpty());
    QVERIFY(controller.getFileLabel().isEmpty());
    QVERIFY(controller.isRecurse());
    QVERIFY(!controller.getMatchAllIfNoKeyword());
    QVERIFY(!controller.isKeywordSegmentationActived());
    QVERIFY(!controller.isSearchFileOnly());
    QVERIFY(!controller.isSearchDirOnly());
    QVERIFY(!controller.isSearchOnlineApps());
}

void SearchControllerTest::testSearchIdAndBeginFinishCheck()
{
    SearchController controller;

    QCOMPARE(controller.getCurrentSearchId(), static_cast<size_t>(0));
    QCOMPARE(controller.refreshSearchId(), static_cast<size_t>(1));
    QVERIFY(controller.beginSearchIdCheck(1));
    controller.finishSearchIdCheck();

    QVERIFY(!controller.beginSearchIdCheck(2));
    controller.finishSearchIdCheck();

    controller.stop();
    QCOMPARE(controller.getCurrentSearchId(), static_cast<size_t>(2));
}

void SearchControllerTest::testDataQueueInitRefreshAndCopySharing()
{
    SearchController controller;
    QVERIFY(controller.getDataQueue() == nullptr);

    DataQueue<ResultItem> *queue = controller.initDataQueue();
    QVERIFY(queue != nullptr);
    QCOMPARE(controller.initDataQueue(), queue);

    ResultItem item(static_cast<size_t>(3));
    queue->enqueue(item);
    QVERIFY(!queue->isEmpty());

    QCOMPARE(controller.refreshDataqueue(), queue);
    QVERIFY(queue->isEmpty());

    SearchController copied(controller);
    QCOMPARE(copied.getDataQueue(), queue);

    SearchController assigned;
    assigned = controller;
    QCOMPARE(assigned.getDataQueue(), queue);
}

void SearchControllerTest::testRefreshDataQueueCreatesQueueWhenMissing()
{
    SearchController controller;

    DataQueue<ResultItem> *queue = controller.refreshDataqueue();

    QVERIFY(queue != nullptr);
    QCOMPARE(controller.getDataQueue(), queue);
    QVERIFY(queue->isEmpty());
}

void SearchControllerTest::testMoveAssignmentTransfersSharedState()
{
    SearchController source;
    source.addKeyword(QStringLiteral("move-keyword"));
    source.initDataQueue();

    SearchController moved;
    moved = std::move(source);

    QCOMPARE(moved.getKeyword(), QStringList{QStringLiteral("move-keyword")});
    QVERIFY(moved.getDataQueue() != nullptr);
}

void SearchControllerTest::testResultPropertyMaps()
{
    SearchController controller;
    SearchResultProperties properties;
    properties << SearchProperty::FileName << SearchProperty::FilePath;

    QVERIFY(controller.setResultProperties(SearchProperty::SearchType::File, properties));
    QCOMPARE(controller.getResultProperties(SearchProperty::SearchType::File), properties);
    QVERIFY(controller.getResultProperties(SearchProperty::SearchType::Application).isEmpty());

    controller.setCustomResultDataType(QStringLiteral("custom"), {QStringLiteral("name"), QStringLiteral("size")});
    QCOMPARE(controller.getCustomResultDataType(QStringLiteral("custom")),
             QStringList({QStringLiteral("name"), QStringLiteral("size")}));
    QVERIFY(controller.getCustomResultDataType(QStringLiteral("missing")).isEmpty());
}

void SearchControllerTest::testResultLimits()
{
    SearchController controller;
    QCOMPARE(controller.maxResults(), 100u);
    QCOMPARE(controller.informNum(), 0);

    controller.setMaxResultNum(7);
    controller.setInformNum(3);
    QCOMPARE(controller.maxResults(), 7u);
    QCOMPARE(controller.informNum(), 3);

    controller.setInformNum(-1);
    QCOMPARE(controller.informNum(), 3);
}

QTEST_MAIN(SearchControllerTest)
