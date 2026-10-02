#include "pending-app-info-queue-test.h"

#include <QSignalSpy>
#include <QTest>

#define private public
#include "app-db-manager.h"
#include "pending-app-info-queue.h"
#undef private

using namespace UkuiSearch;

extern "C" void resetAppDBManagerStubCalls();
extern "C" int appDBManagerStubInsertCalls();
extern "C" int appDBManagerStubUpdateCalls();
extern "C" int appDBManagerStubDeleteCalls();
extern "C" int appDBManagerStubLocaleCalls();
extern "C" int appDBManagerStubLaunchCalls();
extern "C" int appDBManagerStubFavoritesCalls();
extern "C" int appDBManagerStubTopCalls();
extern "C" int appDBManagerStubRefreshCalls();
extern "C" int appDBManagerStubSymlinkCalls();
extern "C" int appDBManagerStubValueSetCalls();

namespace {

PendingAppInfo appInfo(const QString &path, PendingAppInfo::HandleTypes type)
{
    return PendingAppInfo(path, type);
}

void resetQueue(PendingAppInfoQueue &queue)
{
    QMutexLocker locker(&PendingAppInfoQueue::s_mutex);
    queue.m_cache.clear();
    queue.m_pendingAppInfos.clear();
    queue.m_handleTimes = 0;
    Q_EMIT queue.stopTimer();
    resetAppDBManagerStubCalls();
}

} // namespace

void PendingAppInfoQueueTest::testSingletonAndTimerSignals()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    QCOMPARE(&PendingAppInfoQueue::getAppInfoQueue(), &queue);
    resetQueue(queue);

    QSignalSpy minSpy(&queue, SIGNAL(startMinProcessTimer()));
    QSignalSpy maxSpy(&queue, SIGNAL(startMaxProcessTimer()));
    QVERIFY(minSpy.isValid());
    QVERIFY(maxSpy.isValid());

    queue.enqueue(appInfo(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::Insert));

    QCOMPARE(minSpy.count(), 1);
    QCOMPARE(maxSpy.count(), 1);
    QCOMPARE(queue.m_handleTimes, 1);
    QCOMPARE(queue.m_cache.size(), 1);
    QCOMPARE(queue.m_cache.first().path(), QStringLiteral("/tmp/app.desktop"));
    QCOMPARE(queue.m_cache.first().handleType(), PendingAppInfo::HandleTypes(PendingAppInfo::Insert));
}

void PendingAppInfoQueueTest::testEnqueueMergesUpdateFlagsForSameDesktopFile()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    PendingAppInfo favorites(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::UpdateFavorites, 4);
    PendingAppInfo top(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::UpdateTop, -1, 9);
    PendingAppInfo launch(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::UpdateLaunchTimes, -1, -1, true, 2);

    queue.enqueue(favorites);
    queue.enqueue(top);
    queue.enqueue(launch);

    QCOMPARE(queue.m_cache.size(), 1);
    const PendingAppInfo merged = queue.m_cache.first();
    QVERIFY(merged.handleType().testFlag(PendingAppInfo::UpdateFavorites));
    QVERIFY(merged.handleType().testFlag(PendingAppInfo::UpdateTop));
    QVERIFY(merged.handleType().testFlag(PendingAppInfo::UpdateLaunchTimes));
    QCOMPARE(merged.favoritesState(), 4);
    QCOMPARE(merged.topState(), 9);
    QCOMPARE(merged.launchTimes(), 1);
    QCOMPARE(queue.m_handleTimes, 3);
}

void PendingAppInfoQueueTest::testDeleteReplacesEarlierOperations()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    queue.enqueue(appInfo(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::Insert));
    queue.enqueue(appInfo(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::UpdateAll));
    queue.enqueue(appInfo(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::Delete));

    QCOMPARE(queue.m_cache.size(), 1);
    QCOMPARE(queue.m_cache.first().path(), QStringLiteral("/tmp/app.desktop"));
    QCOMPARE(queue.m_cache.first().handleType(), PendingAppInfo::HandleTypes(PendingAppInfo::Delete));
}

void PendingAppInfoQueueTest::testDeleteThenNonInsertKeepsOnlyDelete()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    queue.enqueue(appInfo(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::Delete));
    queue.enqueue(appInfo(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::UpdateAll));

    QCOMPARE(queue.m_cache.size(), 1);
    QCOMPARE(queue.m_cache.first().handleType(), PendingAppInfo::HandleTypes(PendingAppInfo::Delete));
}

void PendingAppInfoQueueTest::testRefreshDatabaseDuplicateRules()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    PendingAppInfo first(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::RefreshDataBase);
    first.setPathsNeedRefreshData({QStringLiteral("/tmp/a"), QStringLiteral("/tmp/b")});
    PendingAppInfo duplicate(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::RefreshDataBase);
    duplicate.setPathsNeedRefreshData({QStringLiteral("/tmp/a"), QStringLiteral("/tmp/b")});
    PendingAppInfo different(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::RefreshDataBase);
    different.setPathsNeedRefreshData({QStringLiteral("/tmp/c")});

    queue.enqueue(first);
    queue.enqueue(duplicate);
    QCOMPARE(queue.m_cache.size(), 1);
    QCOMPARE(queue.m_cache.first().pathsNeedRefreshData(), QStringList({QStringLiteral("/tmp/a"), QStringLiteral("/tmp/b")}));

    queue.enqueue(different);
    QCOMPARE(queue.m_cache.size(), 2);
    QCOMPARE(queue.m_cache.last().pathsNeedRefreshData(), QStringList({QStringLiteral("/tmp/c")}));
}

void PendingAppInfoQueueTest::testInsertAfterDeleteKeepsBothOperations()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    queue.enqueue(appInfo(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::Delete));
    queue.enqueue(appInfo(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::Insert));

    QCOMPARE(queue.m_cache.size(), 2);
    QCOMPARE(queue.m_cache.first().handleType(), PendingAppInfo::HandleTypes(PendingAppInfo::Delete));
    QCOMPARE(queue.m_cache.last().handleType(), PendingAppInfo::HandleTypes(PendingAppInfo::Insert));
}

void PendingAppInfoQueueTest::testPriorityOperationsKeepMostCompleteRefresh()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    queue.enqueue(appInfo(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::UpdateLocaleData));
    queue.enqueue(appInfo(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::Insert));
    queue.enqueue(appInfo(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::UpdateAll));

    QCOMPARE(queue.m_cache.size(), 1);
    QCOMPARE(queue.m_cache.first().handleType(), PendingAppInfo::HandleTypes(PendingAppInfo::Insert));
}

void PendingAppInfoQueueTest::testSymLinkOperationsAreNotMerged()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    queue.enqueue(appInfo(QStringLiteral("/tmp/sdk-link"), PendingAppInfo::UpdateSymLink));
    queue.enqueue(appInfo(QStringLiteral("/tmp/sdk-link"), PendingAppInfo::UpdateSymLink));

    QCOMPARE(queue.m_cache.size(), 2);
    QCOMPARE(queue.m_cache.first().handleType(), PendingAppInfo::HandleTypes(PendingAppInfo::UpdateSymLink));
    QCOMPARE(queue.m_cache.last().handleType(), PendingAppInfo::HandleTypes(PendingAppInfo::UpdateSymLink));
}

void PendingAppInfoQueueTest::testMaxTimerAlreadyActiveDoesNotEmitMaxStartAgain()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    QSignalSpy minSpy(&queue, SIGNAL(startMinProcessTimer()));
    QSignalSpy maxSpy(&queue, SIGNAL(startMaxProcessTimer()));
    QVERIFY(minSpy.isValid());
    QVERIFY(maxSpy.isValid());

    QMetaObject::invokeMethod(queue.m_maxProcessTimer, "start", Qt::BlockingQueuedConnection);
    queue.enqueue(appInfo(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::Insert));

    QCOMPARE(minSpy.count(), 1);
    QCOMPARE(maxSpy.count(), 0);
    QMetaObject::invokeMethod(queue.m_maxProcessTimer, "stop", Qt::BlockingQueuedConnection);
}

void PendingAppInfoQueueTest::testProcessCacheStopsTimersAndClearsPendingItems()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    QSignalSpy finishSpy(AppDBManager::getInstance(), SIGNAL(finishHandleAppDB()));
    QVERIFY(finishSpy.isValid());

    queue.enqueue(appInfo(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::Insert));
    queue.processCache();

    QCOMPARE(finishSpy.count(), 1);
    QCOMPARE(queue.m_cache.size(), 0);
    QCOMPARE(queue.m_pendingAppInfos.size(), 0);
    QCOMPARE(queue.m_handleTimes, 0);
}

void PendingAppInfoQueueTest::testProcessCacheNoopsWhenQueueIsEmpty()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    QSignalSpy finishSpy(AppDBManager::getInstance(), SIGNAL(finishHandleAppDB()));
    QVERIFY(finishSpy.isValid());

    queue.processCache();

    QCOMPARE(finishSpy.count(), 0);
    QCOMPARE(appDBManagerStubInsertCalls(), 0);
    QCOMPARE(queue.m_cache.size(), 0);
    QCOMPARE(queue.m_pendingAppInfos.size(), 0);
}

void PendingAppInfoQueueTest::testProcessCacheIgnoresCombinedPriorityHandleTypeButStillFinishes()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    QSignalSpy finishSpy(AppDBManager::getInstance(), SIGNAL(finishHandleAppDB()));
    QVERIFY(finishSpy.isValid());

    queue.enqueue(appInfo(QStringLiteral("/tmp/default.desktop"),
                          PendingAppInfo::Insert | PendingAppInfo::UpdateAll));
    queue.processCache();

    QCOMPARE(finishSpy.count(), 1);
    QCOMPARE(appDBManagerStubInsertCalls(), 0);
    QCOMPARE(appDBManagerStubUpdateCalls(), 0);
    QCOMPARE(appDBManagerStubDeleteCalls(), 0);
    QCOMPARE(queue.m_pendingAppInfos.size(), 0);
}

void PendingAppInfoQueueTest::testProcessCacheDispatchesSingleOperationTypes()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    PendingAppInfo refresh(QStringLiteral("/tmp/refresh.desktop"), PendingAppInfo::RefreshDataBase);
    refresh.setPathsNeedRefreshData({QStringLiteral("/tmp/apps")});
    PendingAppInfo valueSet(QStringLiteral("/tmp/value.desktop"), PendingAppInfo::SetValue);
    ApplicationInfoMap values;
    values[QStringLiteral("/tmp/value.desktop")].insert(ApplicationProperty::Favorites, 2);
    valueSet.setValue(values);

    queue.enqueue(appInfo(QStringLiteral("/tmp/delete.desktop"), PendingAppInfo::Delete));
    queue.enqueue(appInfo(QStringLiteral("/tmp/insert.desktop"), PendingAppInfo::Insert));
    queue.enqueue(appInfo(QStringLiteral("/tmp/update.desktop"), PendingAppInfo::UpdateAll));
    queue.enqueue(appInfo(QStringLiteral("/tmp/locale.desktop"), PendingAppInfo::UpdateLocaleData));
    queue.enqueue(refresh);
    queue.enqueue(valueSet);
    queue.enqueue(appInfo(QStringLiteral("/tmp/link.desktop"), PendingAppInfo::UpdateSymLink));

    queue.processCache();

    QCOMPARE(appDBManagerStubDeleteCalls(), 1);
    QCOMPARE(appDBManagerStubInsertCalls(), 1);
    QCOMPARE(appDBManagerStubUpdateCalls(), 1);
    QCOMPARE(appDBManagerStubLocaleCalls(), 1);
    QCOMPARE(appDBManagerStubRefreshCalls(), 1);
    QCOMPARE(appDBManagerStubValueSetCalls(), 1);
    QCOMPARE(appDBManagerStubSymlinkCalls(), 1);
}

void PendingAppInfoQueueTest::testProcessCacheDispatchesMergedFlagOperations()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    const QString path = QStringLiteral("/tmp/merged.desktop");
    queue.enqueue(PendingAppInfo(path, PendingAppInfo::UpdateLaunchTimes, -1, -1, true, 1));
    queue.enqueue(PendingAppInfo(path, PendingAppInfo::UpdateFavorites, 4));
    queue.enqueue(PendingAppInfo(path, PendingAppInfo::UpdateTop, -1, 7));

    queue.processCache();

    QCOMPARE(appDBManagerStubLaunchCalls(), 1);
    QCOMPARE(appDBManagerStubFavoritesCalls(), 1);
    QCOMPARE(appDBManagerStubTopCalls(), 1);
}

void PendingAppInfoQueueTest::testProcessCacheDispatchesMergedPriorityAndValueFlags()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    const QString path = QStringLiteral("/tmp/merged-priority.desktop");
    ApplicationInfoMap values;
    values[path].insert(ApplicationProperty::Favorites, 6);

    PendingAppInfo valueSet(path, PendingAppInfo::SetValue);
    valueSet.setValue(values);

    queue.enqueue(PendingAppInfo(path, PendingAppInfo::UpdateFavorites, 2));
    queue.enqueue(appInfo(path, PendingAppInfo::Insert));
    queue.enqueue(appInfo(path, PendingAppInfo::UpdateAll));
    queue.enqueue(appInfo(path, PendingAppInfo::UpdateLocaleData));
    queue.enqueue(valueSet);

    queue.processCache();

    QCOMPARE(appDBManagerStubInsertCalls(), 1);
    QCOMPARE(appDBManagerStubUpdateCalls(), 1);
    QCOMPARE(appDBManagerStubLocaleCalls(), 1);
    QCOMPARE(appDBManagerStubFavoritesCalls(), 1);
    QCOMPARE(appDBManagerStubValueSetCalls(), 1);
}

void PendingAppInfoQueueTest::testProcessCacheDispatchesSparseMergedFlags()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    const QString favoritePath = QStringLiteral("/tmp/favorite-only.desktop");
    queue.enqueue(PendingAppInfo(favoritePath, PendingAppInfo::UpdateFavorites, 1));
    queue.processCache();

    QCOMPARE(appDBManagerStubInsertCalls(), 0);
    QCOMPARE(appDBManagerStubUpdateCalls(), 0);
    QCOMPARE(appDBManagerStubLocaleCalls(), 0);
    QCOMPARE(appDBManagerStubLaunchCalls(), 0);
    QCOMPARE(appDBManagerStubFavoritesCalls(), 1);
    QCOMPARE(appDBManagerStubTopCalls(), 0);
    QCOMPARE(appDBManagerStubValueSetCalls(), 0);

    resetQueue(queue);
    queue.enqueue(PendingAppInfo(QStringLiteral("/tmp/launch-only.desktop"),
                                 PendingAppInfo::UpdateLaunchTimes,
                                 -1,
                                 -1,
                                 true,
                                 3));
    queue.processCache();

    QCOMPARE(appDBManagerStubLaunchCalls(), 1);
    QCOMPARE(appDBManagerStubFavoritesCalls(), 0);
    QCOMPARE(appDBManagerStubTopCalls(), 0);
}

void PendingAppInfoQueueTest::testSetValueOperationsAreMerged()
{
    PendingAppInfoQueue &queue = PendingAppInfoQueue::getAppInfoQueue();
    resetQueue(queue);

    ApplicationInfoMap firstValue;
    firstValue[QStringLiteral("/tmp/app.desktop")].insert(ApplicationProperty::Favorites, 2);
    PendingAppInfo first(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::SetValue);
    first.setValue(firstValue);

    ApplicationInfoMap secondValue;
    secondValue[QStringLiteral("/tmp/app.desktop")].insert(ApplicationProperty::Top, 3);
    PendingAppInfo second(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::SetValue);
    second.setValue(secondValue);

    queue.enqueue(first);
    queue.enqueue(second);

    QCOMPARE(queue.m_cache.size(), 1);
    const auto values = queue.m_cache.first().value2BSet().value(QStringLiteral("/tmp/app.desktop"));
    QCOMPARE(values.value(ApplicationProperty::Favorites).toInt(), 2);
    QCOMPARE(values.value(ApplicationProperty::Top).toInt(), 3);
}

void PendingAppInfoQueueTest::cleanupTestCase()
{
    resetQueue(PendingAppInfoQueue::getAppInfoQueue());
}

QTEST_MAIN(PendingAppInfoQueueTest)
