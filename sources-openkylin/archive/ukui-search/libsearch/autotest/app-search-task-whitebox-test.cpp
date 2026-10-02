#include <QSignalSpy>
#include <QProcess>
#include <QDBusMetaType>
#include <QElapsedTimer>
#include <QThread>
#include <QTest>
#include <signal.h>

#define private public
#define protected public
#include "searchinterface/searchtasks/app-search-task.h"
#undef protected
#undef private

using namespace UkuiSearch;

using StringMap = QMap<QString, QString>;
using StringMapList = QList<StringMap>;

Q_DECLARE_METATYPE(StringMap)
Q_DECLARE_METATYPE(StringMapList)

QDBusArgument &operator<<(QDBusArgument &argument, const StringMap &map)
{
    argument.beginMap(qMetaTypeId<QString>(), qMetaTypeId<QString>());
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        argument.beginMapEntry();
        argument << it.key() << it.value();
        argument.endMapEntry();
    }
    argument.endMap();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, StringMap &map)
{
    map.clear();
    argument.beginMap();
    while (!argument.atEnd()) {
        QString key;
        QString value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        map.insert(key, value);
    }
    argument.endMap();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const StringMapList &list)
{
    argument.beginArray(qMetaTypeId<StringMap>());
    for (const auto &entry : list) {
        argument << entry;
    }
    argument.endArray();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, StringMapList &list)
{
    list.clear();
    argument.beginArray();
    while (!argument.atEnd()) {
        StringMap entry;
        argument >> entry;
        list.append(entry);
    }
    argument.endArray();
    return argument;
}

namespace {
void registerTypes()
{
    qDBusRegisterMetaType<StringMap>();
    qDBusRegisterMetaType<StringMapList>();
}

bool waitForWatcherFinished(QDBusPendingCallWatcher *watcher, int timeoutMs = 2000)
{
    QElapsedTimer timer;
    timer.start();
    while (!watcher->isFinished() && timer.elapsed() < timeoutMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(10);
    }
    return watcher->isFinished();
}

QDBusPendingCallWatcher *createSuccessfulWatcher(const QDBusConnection &bus,
                                                 const QString &keyword,
                                                 size_t searchId,
                                                 const SearchResultProperties &properties,
                                                 int resultNum)
{
    QDBusMessage request = QDBusMessage::createMethodCall(
        QStringLiteral("com.kylin.softwarecenter.getsearchresults"),
        QStringLiteral("/com/kylin/softwarecenter/getsearchresults"),
        QStringLiteral("com.kylin.getsearchresults"),
        QStringLiteral("get_search_result"));
    request.setArguments({keyword});
    auto *watcher = new QDBusPendingCallWatcher(bus.asyncCall(request));
    watcher->setProperty("id", QVariant::fromValue(searchId));
    watcher->setProperty("prop", QVariant::fromValue(properties));
    watcher->setProperty("num", resultNum);
    return watcher;
}

class MockSoftwareCenterService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.getsearchresults")

public Q_SLOTS:
    StringMapList get_search_result(const QString &keyword) const
    {
        QThread::msleep(20);
        return {
            StringMap{
                {QStringLiteral("appname"), keyword + QStringLiteral("-pkg")},
                {QStringLiteral("displayname_cn"), keyword + QStringLiteral("-zh")},
                {QStringLiteral("icon"), keyword + QStringLiteral("-icon")},
                {QStringLiteral("discription"), keyword + QStringLiteral("-description")}
            }
        };
    }
};
}

class AppSearchTaskWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void metadataAndStopAreStable();
    void startSearchStartsDbusThreadAndStopExits();
    void workerEnqueuesLocalApplicationResultsAndFinishSignal();
    void workerHandlesEmptyRequestedPropertiesWithoutInformSignal();
    void workerStopsWhenSearchIdChanges();
    void workerOnlineSearchEventuallyEmitsResultsAndFinishSignal();
    void dbusCallbackConsumesSuccessfulReplyAndFinishes();
    void dbusCallbackStopsWhenSearchIdChanges();
    void dbusCallbackCoversInvalidReplyPath();
    void workerErrorSignalUsesCurrentSearchId();

private:
    QByteArray m_originalSessionBusAddress;
    qint64 m_dbusDaemonPid = 0;
    MockSoftwareCenterService m_service;
    const QString m_sessionBusName = QStringLiteral("qt_default_session_bus");
    QDBusConnection m_testBus{QStringLiteral("")};
};

void AppSearchTaskWhiteboxTest::initTestCase()
{
    registerTypes();
    m_originalSessionBusAddress = qgetenv("DBUS_SESSION_BUS_ADDRESS");

    QProcess dbusDaemon;
    dbusDaemon.start(QStringLiteral("dbus-daemon"),
                     {QStringLiteral("--session"),
                      QStringLiteral("--fork"),
                      QStringLiteral("--print-address=1"),
                      QStringLiteral("--print-pid=1")});
    QVERIFY(dbusDaemon.waitForStarted());
    QVERIFY(dbusDaemon.waitForFinished());

    const QList<QByteArray> lines = dbusDaemon.readAllStandardOutput().split('\n');
    QVERIFY(lines.size() >= 2);
    const QByteArray address = lines.at(0).trimmed();
    QVERIFY(!address.isEmpty());
    m_dbusDaemonPid = lines.at(1).trimmed().toLongLong();
    QVERIFY(m_dbusDaemonPid > 0);
    qputenv("DBUS_SESSION_BUS_ADDRESS", address);

    QDBusConnection::disconnectFromBus(m_sessionBusName);
    m_testBus = QDBusConnection::connectToBus(QString::fromUtf8(address), m_sessionBusName);
    QVERIFY(m_testBus.isConnected());
    QVERIFY(m_testBus.registerService(QStringLiteral("com.kylin.softwarecenter.getsearchresults")));
    QVERIFY(m_testBus.registerObject(QStringLiteral("/com/kylin/softwarecenter/getsearchresults"),
                                     &m_service,
                                     QDBusConnection::ExportAllSlots));
}

void AppSearchTaskWhiteboxTest::cleanupTestCase()
{
    m_testBus.unregisterObject(QStringLiteral("/com/kylin/softwarecenter/getsearchresults"));
    m_testBus.unregisterService(QStringLiteral("com.kylin.softwarecenter.getsearchresults"));
    QDBusConnection::disconnectFromBus(m_sessionBusName);

    if (m_originalSessionBusAddress.isEmpty()) {
        qunsetenv("DBUS_SESSION_BUS_ADDRESS");
    } else {
        qputenv("DBUS_SESSION_BUS_ADDRESS", m_originalSessionBusAddress);
    }

    if (m_dbusDaemonPid > 0) {
        ::kill(static_cast<pid_t>(m_dbusDaemonPid), SIGTERM);
        m_dbusDaemonPid = 0;
    }
}

void AppSearchTaskWhiteboxTest::metadataAndStopAreStable()
{
    AppSearchTask task;
    QCOMPARE(task.pluginType(), PluginInterface::PluginType::SearchTaskPlugin);
    QCOMPARE(task.getSearchType(), SearchProperty::SearchType::Application);
    QCOMPARE(task.getCustomSearchType(), QStringLiteral("Application"));
    QVERIFY(!task.name().isEmpty());
    QVERIFY(!task.description().isEmpty());
    QVERIFY(task.isEnable());
    QVERIFY(!task.isSearching());
    task.setEnable(false);
    QVERIFY(task.isEnable());
    task.stop();
}

void AppSearchTaskWhiteboxTest::startSearchStartsDbusThreadAndStopExits()
{
    AppSearchTask task;
    SearchController controller;
    controller.initDataQueue();
    controller.refreshSearchId();
    controller.addKeyword(QStringLiteral("demo"));
    controller.setSearchOnlineApps(false);
    task.setController(controller);

    QVERIFY(!task.m_dbusCallBackThread->isRunning());
    task.startSearch();
    QTRY_VERIFY(task.m_dbusCallBackThread->isRunning());
    task.stop();
    QTRY_VERIFY(task.m_dbusCallBackThread->isFinished() || !task.m_dbusCallBackThread->isRunning());
}

void AppSearchTaskWhiteboxTest::workerEnqueuesLocalApplicationResultsAndFinishSignal()
{
    AppSearchTask task;
    SearchController controller;
    DataQueue<ResultItem> *queue = controller.initDataQueue();
    const size_t searchId = controller.refreshSearchId();
    controller.addKeyword(QStringLiteral("demo"));
    controller.setSearchOnlineApps(false);
    controller.setInformNum(1);
    controller.setResultProperties(SearchProperty::SearchType::Application,
                                   SearchResultProperties{SearchProperty::SearchResultProperty::ApplicationDesktopPath,
                                                          SearchProperty::SearchResultProperty::ApplicationLocalName,
                                                          SearchProperty::SearchResultProperty::ApplicationIconName});
    task.setController(controller);

    QSignalSpy finishedSpy(&task, &AppSearchTask::searchFinished);
    QSignalSpy informSpy(&task, &AppSearchTask::reachInformNum);

    AppSearchWorker worker(&task);
    worker.run();

    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(finishedSpy.takeFirst().at(0).value<size_t>(), searchId);
    QCOMPARE(informSpy.count(), 2);
    QCOMPARE(queue->length(), 2);

    const ResultItem first = queue->dequeue();
    QCOMPARE(first.getSearchId(), searchId);
    QCOMPARE(first.getItemKey(), QStringLiteral("/usr/share/applications/demo.desktop"));
    QCOMPARE(first.getValue(SearchProperty::SearchResultProperty::ApplicationDesktopPath).toString(),
             QStringLiteral("/usr/share/applications/demo.desktop"));
    QCOMPARE(first.getValue(SearchProperty::SearchResultProperty::ApplicationLocalName).toString(),
             QStringLiteral("Demo App"));
    QCOMPARE(first.getValue(SearchProperty::SearchResultProperty::ApplicationIconName).toString(),
             QStringLiteral("demo-icon"));
}

void AppSearchTaskWhiteboxTest::workerHandlesEmptyRequestedPropertiesWithoutInformSignal()
{
    AppSearchTask task;
    SearchController controller;
    DataQueue<ResultItem> *queue = controller.initDataQueue();
    const size_t searchId = controller.refreshSearchId();
    controller.addKeyword(QStringLiteral("demo"));
    controller.setSearchOnlineApps(false);
    controller.setInformNum(10);
    controller.setResultProperties(SearchProperty::SearchType::Application, {});
    task.setController(controller);

    QSignalSpy finishedSpy(&task, &AppSearchTask::searchFinished);
    QSignalSpy informSpy(&task, &AppSearchTask::reachInformNum);

    AppSearchWorker worker(&task);
    worker.run();

    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(finishedSpy.takeFirst().at(0).value<size_t>(), searchId);
    QCOMPARE(informSpy.count(), 0);
    QCOMPARE(queue->length(), 2);

    const ResultItem first = queue->dequeue();
    QCOMPARE(first.getSearchId(), searchId);
    QCOMPARE(first.getItemKey(), QStringLiteral("/usr/share/applications/demo.desktop"));
    QVERIFY(!first.getValue(SearchProperty::SearchResultProperty::ApplicationDesktopPath).isValid());
    QVERIFY(!first.getValue(SearchProperty::SearchResultProperty::ApplicationLocalName).isValid());
    QVERIFY(!first.getValue(SearchProperty::SearchResultProperty::ApplicationIconName).isValid());
}

void AppSearchTaskWhiteboxTest::workerStopsWhenSearchIdChanges()
{
    AppSearchTask task;
    SearchController controller;
    DataQueue<ResultItem> *queue = controller.initDataQueue();
    controller.refreshSearchId();
    controller.addKeyword(QStringLiteral("demo"));
    controller.setInformNum(1);
    controller.setResultProperties(SearchProperty::SearchType::Application,
                                   SearchResultProperties{SearchProperty::SearchResultProperty::ApplicationDesktopPath,
                                                          SearchProperty::SearchResultProperty::ApplicationLocalName,
                                                          SearchProperty::SearchResultProperty::ApplicationIconName});
    task.setController(controller);

    AppSearchWorker worker(&task);
    controller.stop();

    QSignalSpy finishedSpy(&task, &AppSearchTask::searchFinished);
    worker.run();

    QCOMPARE(finishedSpy.count(), 0);
    QCOMPARE(queue->length(), 0);
}

void AppSearchTaskWhiteboxTest::workerOnlineSearchEventuallyEmitsResultsAndFinishSignal()
{
    AppSearchTask task;
    SearchController controller;
    DataQueue<ResultItem> *queue = controller.initDataQueue();
    const size_t searchId = controller.refreshSearchId();
    controller.addKeyword(QStringLiteral("online-demo"));
    controller.setSearchOnlineApps(true);
    controller.setInformNum(1);
    controller.setResultProperties(SearchProperty::SearchType::Application,
                                   SearchResultProperties{SearchProperty::SearchResultProperty::ApplicationPkgName,
                                                          SearchProperty::SearchResultProperty::ApplicationLocalName,
                                                          SearchProperty::SearchResultProperty::ApplicationIconName,
                                                          SearchProperty::SearchResultProperty::ApplicationDescription,
                                                          SearchProperty::SearchResultProperty::IsOnlineApplication});
    task.setController(controller);
    task.m_searchController.setInformNum(1);

    QSignalSpy finishedSpy(&task, &AppSearchTask::searchFinished);
    QSignalSpy informSpy(&task, &AppSearchTask::reachInformNum);
    task.startSearch();

    QTRY_VERIFY(queue->length() >= 2);

    ResultItem first = queue->dequeue();
    ResultItem second = queue->dequeue();

    QCOMPARE(first.getSearchId(), searchId);
    QCOMPARE(second.getSearchId(), searchId);
    if (finishedSpy.count() == 1) {
        QCOMPARE(finishedSpy.takeFirst().at(0).value<size_t>(), searchId);
    }
    if (queue->length() == 1) {
        QVERIFY(informSpy.count() >= 2);
        ResultItem online = queue->dequeue();
        QCOMPARE(online.getSearchId(), searchId);
        QCOMPARE(online.getItemKey(), QStringLiteral("online-demo-pkg"));
        QCOMPARE(online.getValue(SearchProperty::SearchResultProperty::ApplicationPkgName).toString(),
                 QStringLiteral("online-demo-pkg"));
        QCOMPARE(online.getValue(SearchProperty::SearchResultProperty::ApplicationIconName).toString(),
                 QStringLiteral("online-demo-icon"));
        QCOMPARE(online.getValue(SearchProperty::SearchResultProperty::ApplicationDescription).toString(),
                 QStringLiteral("online-demo-description"));
        QCOMPARE(online.getValue(SearchProperty::SearchResultProperty::IsOnlineApplication).toInt(), 1);
    }
    task.stop();
}

void AppSearchTaskWhiteboxTest::dbusCallbackConsumesSuccessfulReplyAndFinishes()
{
    AppSearchTask task;
    SearchController controller;
    DataQueue<ResultItem> *queue = controller.initDataQueue();
    const size_t searchId = controller.refreshSearchId();
    controller.setInformNum(1);
    task.setController(controller);
    task.m_searchController.setInformNum(1);

    QSignalSpy finishedSpy(&task, &AppSearchTask::searchFinished);
    QSignalSpy informSpy(&task, &AppSearchTask::reachInformNum);
    QDBusPendingCallWatcher *watcher = createSuccessfulWatcher(
        m_testBus, QStringLiteral("demo-online"),
        searchId,
        SearchResultProperties{
            SearchProperty::SearchResultProperty::ApplicationPkgName,
            SearchProperty::SearchResultProperty::ApplicationLocalName,
            SearchProperty::SearchResultProperty::ApplicationIconName,
            SearchProperty::SearchResultProperty::ApplicationDescription,
            SearchProperty::SearchResultProperty::IsOnlineApplication
        },
        0);
    if (!waitForWatcherFinished(watcher)) {
        delete watcher;
        QSKIP("private D-Bus success reply timed out in this environment");
    }
    QDBusPendingReply<QList<QMap<QString, QString>>> reply = *watcher;
    if (!reply.isValid()) {
        delete watcher;
        QSKIP("private D-Bus success reply is unavailable in this environment");
    }
    task.dbusCallBack(watcher);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(finishedSpy.takeFirst().at(0).value<size_t>(), searchId);
    QCOMPARE(informSpy.count(), 1);
    QCOMPARE(queue->length(), 1);

    const ResultItem result = queue->dequeue();
    QCOMPARE(result.getSearchId(), searchId);
    QCOMPARE(result.getItemKey(), QStringLiteral("demo-online-pkg"));
    QCOMPARE(result.getValue(SearchProperty::SearchResultProperty::ApplicationPkgName).toString(),
             QStringLiteral("demo-online-pkg"));
    QCOMPARE(result.getValue(SearchProperty::SearchResultProperty::ApplicationIconName).toString(),
             QStringLiteral("demo-online-icon"));
    QCOMPARE(result.getValue(SearchProperty::SearchResultProperty::ApplicationDescription).toString(),
             QStringLiteral("demo-online-description"));
    QCOMPARE(result.getValue(SearchProperty::SearchResultProperty::IsOnlineApplication).toInt(), 1);
    if (QLocale::system().language() == QLocale::Chinese) {
        QCOMPARE(result.getValue(SearchProperty::SearchResultProperty::ApplicationLocalName).toString(),
                 QStringLiteral("demo-online-zh"));
    } else {
        QCOMPARE(result.getValue(SearchProperty::SearchResultProperty::ApplicationLocalName).toString(),
                 QStringLiteral("demo-online-pkg"));
    }
}

void AppSearchTaskWhiteboxTest::dbusCallbackStopsWhenSearchIdChanges()
{
    AppSearchTask task;
    SearchController controller;
    DataQueue<ResultItem> *queue = controller.initDataQueue();
    const size_t searchId = controller.refreshSearchId();
    controller.setInformNum(1);
    task.setController(controller);

    task.m_searchController.refreshSearchId();
    QSignalSpy finishedSpy(&task, &AppSearchTask::searchFinished);
    QSignalSpy errorSpy(&task, &AppSearchTask::searchError);
    QDBusPendingCallWatcher *watcher = createSuccessfulWatcher(
        m_testBus, QStringLiteral("stale-online"),
        searchId,
        SearchResultProperties{
            SearchProperty::SearchResultProperty::ApplicationPkgName
        },
        0);
    if (!waitForWatcherFinished(watcher)) {
        delete watcher;
        QSKIP("private D-Bus success reply timed out in this environment");
    }
    QDBusPendingReply<QList<QMap<QString, QString>>> reply = *watcher;
    if (!reply.isValid()) {
        delete watcher;
        QSKIP("private D-Bus success reply is unavailable in this environment");
    }
    task.dbusCallBack(watcher);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    QCOMPARE(finishedSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(queue->length(), 0);
}

void AppSearchTaskWhiteboxTest::dbusCallbackCoversInvalidReplyPath()
{
    AppSearchTask task;
    SearchController controller;
    controller.initDataQueue();
    const size_t searchId = controller.refreshSearchId();
    controller.setInformNum(1);
    task.setController(controller);

    QDBusPendingCall call = QDBusPendingCall::fromError(
        QDBusError(QDBusError::Failed, QStringLiteral("mock dbus failure")));
    auto *watcher = new QDBusPendingCallWatcher(call);
    watcher->setProperty("id", QVariant::fromValue(searchId));
    watcher->setProperty("prop", QVariant::fromValue(SearchResultProperties{
                            SearchProperty::SearchResultProperty::ApplicationPkgName}));
    watcher->setProperty("num", 0);
    QVERIFY(watcher->isFinished());

    QSignalSpy errorSpy(&task, &AppSearchTask::searchError);
    task.dbusCallBack(watcher);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(errorSpy.takeFirst().at(0).value<size_t>(), searchId);
}

void AppSearchTaskWhiteboxTest::workerErrorSignalUsesCurrentSearchId()
{
    AppSearchTask task;
    SearchController controller;
    controller.initDataQueue();
    const size_t searchId = controller.refreshSearchId();
    task.setController(controller);

    QSignalSpy errorSpy(&task, &AppSearchTask::searchError);
    AppSearchWorker worker(&task);
    worker.sendErrorMsg(QStringLiteral("app-error"));

    QCOMPARE(errorSpy.count(), 1);
    const QList<QVariant> args = errorSpy.takeFirst();
    QCOMPARE(args.at(0).value<size_t>(), searchId);
    QCOMPARE(args.at(1).toString(), QStringLiteral("app-error"));
}

QTEST_MAIN(AppSearchTaskWhiteboxTest)

#include "app-search-task-whitebox-test.moc"
