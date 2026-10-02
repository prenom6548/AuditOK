#include "dir-watcher-test.h"

#include <algorithm>

#include <QDir>
#include <QDBusConnection>
#include <QSignalSpy>
#include <QTest>
#include <memory>

#include "dir-watcher.h"
#include "search-dir-config.h"

using namespace UkuiSearch;

namespace {

class FakePrivateDirWatcherService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.search.fileindex")

public Q_SLOTS:
    QStringList currentIndexableDir() const
    {
        return m_currentIndexableDir;
    }

    QStringList blackListOfDir(const QString &) const
    {
        return m_blackListOfDir;
    }

    QStringList blockDirsForUser() const
    {
        return m_blockDirsForUser;
    }

    QStringList currentBlackListOfIndex() const
    {
        return m_currentBlackListOfIndex;
    }

    QStringList extraDirs() const
    {
        return m_extraDirs;
    }

    QStringList currentSearchableDir() const
    {
        return m_currentSearchableDir;
    }

    QStringList searchableDirForSearchApplication() const
    {
        return m_searchableDirForSearchApplication;
    }

    QStringList extraDirsOfPaths(const QStringList &paths) const
    {
        m_lastExtraDirsOfPathsInput = paths;
        return m_extraDirsOfPaths;
    }

    int appendSearchDir(const QString &path)
    {
        m_lastAppendedSearchDir = path;
        return m_appendSearchDirResult;
    }

    void removeSearchDir(const QString &path)
    {
        m_lastRemovedSearchDir = path;
    }

    int appendIndexableListItem(const QString &path)
    {
        m_lastAppendedIndexablePath = path;
        return m_appendIndexableResult;
    }

    void removeIndexableListItem(const QString &path)
    {
        m_lastRemovedIndexablePath = path;
    }

    void initDbusService()
    {
        m_initCalled = true;
    }

Q_SIGNALS:
    void appendIndexItem(const QString &path, const QStringList &blockList);
    void removeIndexItem(const QString &path);
    void mountAdded(const QString &path);
    void blockItemAdded(const QString &path);
    void blockItemRemoved(const QString &path);

public:
    mutable QStringList m_currentIndexableDir{QDir::homePath()};
    mutable QStringList m_blackListOfDir;
    mutable QStringList m_blockDirsForUser;
    mutable QStringList m_currentBlackListOfIndex;
    mutable QStringList m_extraDirs;
    mutable QStringList m_currentSearchableDir;
    mutable QStringList m_searchableDirForSearchApplication;
    mutable QStringList m_extraDirsOfPaths;
    mutable QStringList m_lastExtraDirsOfPathsInput;
    mutable QString m_lastAppendedSearchDir;
    mutable QString m_lastRemovedSearchDir;
    mutable QString m_lastAppendedIndexablePath;
    mutable QString m_lastRemovedIndexablePath;
    int m_appendSearchDirResult = 0;
    int m_appendIndexableResult = 0;
    bool m_initCalled = false;
};

class ScopedDbusRegistration
{
public:
    ScopedDbusRegistration(const QString &service,
                           const QString &path,
                           QObject *object,
                           const QDBusConnection &connection = QDBusConnection::sessionBus())
        : m_service(service),
          m_path(path),
          m_connection(connection)
    {
        if (!m_connection.isConnected()) {
            return;
        }
        m_serviceRegistered = m_connection.registerService(service);
        m_objectRegistered = m_connection.registerObject(path,
                                                         object,
                                                         QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
    }

    ~ScopedDbusRegistration()
    {
        if (m_objectRegistered) {
            m_connection.unregisterObject(m_path);
        }
        if (m_serviceRegistered) {
            m_connection.unregisterService(m_service);
        }
    }

private:
    QString m_service;
    QString m_path;
    QDBusConnection m_connection{QString()};
    bool m_serviceRegistered = false;
    bool m_objectRegistered = false;
};

FakePrivateDirWatcherService *g_dirWatcherService = nullptr;
std::unique_ptr<ScopedDbusRegistration> g_dirWatcherRegistration;

}

void DirWatcherTest::initTestCase()
{
    g_dirWatcherService = new FakePrivateDirWatcherService;
    g_dirWatcherRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.ukui.search.fileindex.service"),
                                                              QStringLiteral("/org/ukui/search/privateDirWatcher"),
                                                              g_dirWatcherService));
}

void DirWatcherTest::cleanupTestCase()
{
    g_dirWatcherRegistration.reset();
    delete g_dirWatcherService;
    g_dirWatcherService = nullptr;
}

void DirWatcherTest::testSingletonAndForwardedSignals()
{
    DirWatcher *watcher = DirWatcher::getDirWatcher();
    QVERIFY(watcher != nullptr);
    QCOMPARE(DirWatcher::getDirWatcher(), watcher);

    SearchDirConfig config;
    QSignalSpy addedSpy(&config, SIGNAL(blockItemAdded(QString)));
    QSignalSpy removedSpy(&config, SIGNAL(blockItemRemoved(QString)));
    QSignalSpy appendSpy(watcher, SIGNAL(appendIndexItem(QString,QStringList)));
    QSignalSpy removeSpy(watcher, SIGNAL(removeIndexItem(QString)));

    QVERIFY(addedSpy.isValid());
    QVERIFY(removedSpy.isValid());
    QVERIFY(appendSpy.isValid());
    QVERIFY(removeSpy.isValid());

    Q_EMIT watcher->blockItemAdded(QStringLiteral("/tmp/blocked"));
    Q_EMIT watcher->blockItemRemoved(QStringLiteral("/tmp/unblocked"));
    watcher->sendAppendSignal(QStringLiteral("/tmp/path"), {QStringLiteral("/tmp/path/private")});
    watcher->sendRemoveSignal(QStringLiteral("/tmp/path"));

    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(addedSpy.takeFirst().at(0).toString(), QStringLiteral("/tmp/blocked"));
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.takeFirst().at(0).toString(), QStringLiteral("/tmp/unblocked"));

    QCOMPARE(appendSpy.count(), 1);
    QCOMPARE(appendSpy.takeFirst().at(0).toString(), QStringLiteral("/tmp/path"));
    QCOMPARE(removeSpy.count(), 1);
    QCOMPARE(removeSpy.takeFirst().at(0).toString(), QStringLiteral("/tmp/path"));
}

void DirWatcherTest::testServiceBackedMethodsReturnValuesAndInvokeDbusCalls()
{
    DirWatcher *watcher = DirWatcher::getDirWatcher();

    g_dirWatcherService->m_currentIndexableDir = QStringList{QStringLiteral("/srv/indexable")};
    g_dirWatcherService->m_currentBlackListOfIndex = QStringList{QStringLiteral("/srv/blacklist")};
    g_dirWatcherService->m_currentSearchableDir = QStringList{QStringLiteral("/srv/searchable")};
    g_dirWatcherService->m_searchableDirForSearchApplication = QStringList{QStringLiteral("/srv/app-searchable")};
    g_dirWatcherService->m_blackListOfDir = QStringList{QStringLiteral("/srv/private")};
    g_dirWatcherService->m_extraDirs = QStringList{QStringLiteral("/srv/extra")};
    g_dirWatcherService->m_extraDirsOfPaths = QStringList{QStringLiteral("/srv/from-paths")};
    g_dirWatcherService->m_blockDirsForUser = QStringList{QStringLiteral("/srv/user-blocked")};
    g_dirWatcherService->m_appendSearchDirResult = 7;
    g_dirWatcherService->m_appendIndexableResult = 9;
    g_dirWatcherService->m_initCalled = false;

    watcher->initDbusService();
    QVERIFY(g_dirWatcherService->m_initCalled);
    QCOMPARE(watcher->currentIndexableDir(), QStringList{QStringLiteral("/srv/indexable")});
    QCOMPARE(watcher->currentBlackListOfIndex(), QStringList{QStringLiteral("/srv/blacklist")});
    QCOMPARE(watcher->currentSearchableDir(), QStringList{QStringLiteral("/srv/searchable")});
    QCOMPARE(watcher->searchableDirForSearchApplication(), QStringList{QStringLiteral("/srv/app-searchable")});
    QCOMPARE(watcher->blackListOfDir(QStringLiteral("/tmp/query")), QStringList{QStringLiteral("/srv/private")});
    QCOMPARE(watcher->extraDirs(), QStringList{QStringLiteral("/srv/extra")});
    QCOMPARE(watcher->extraDirsOfPaths({QStringLiteral("/tmp/a"), QStringLiteral("/tmp/b")}),
             QStringList{QStringLiteral("/srv/from-paths")});
    QCOMPARE(g_dirWatcherService->m_lastExtraDirsOfPathsInput,
             QStringList({QStringLiteral("/tmp/a"), QStringLiteral("/tmp/b")}));
    QCOMPARE(watcher->getBlockDirsOfUser(), QStringList{QStringLiteral("/srv/user-blocked")});

    watcher->appendSearchDir(QStringLiteral("/tmp/add"));
    watcher->removeSearchDir(QStringLiteral("/tmp/remove"));
    watcher->appendIndexableListItem(QStringLiteral("/tmp/index"));
    watcher->removeIndexableListItem(QStringLiteral("/tmp/index-remove"));

    QCOMPARE(g_dirWatcherService->m_lastAppendedSearchDir, QStringLiteral("/tmp/add"));
    QCOMPARE(g_dirWatcherService->m_lastRemovedSearchDir, QStringLiteral("/tmp/remove"));
    QCOMPARE(g_dirWatcherService->m_lastAppendedIndexablePath, QStringLiteral("/tmp/index"));
    QCOMPARE(g_dirWatcherService->m_lastRemovedIndexablePath, QStringLiteral("/tmp/index-remove"));

    SearchDirConfig config;
    QCOMPARE(config.getBlockDirOfUser(), QStringList{QStringLiteral("/srv/user-blocked")});
}

void DirWatcherTest::testFallbackMethodsWithoutService()
{
    DirWatcher *watcher = DirWatcher::getDirWatcher();

    g_dirWatcherService->m_currentIndexableDir = QStringList{QDir::homePath()};
    g_dirWatcherService->m_currentBlackListOfIndex.clear();
    g_dirWatcherService->m_currentSearchableDir.clear();
    g_dirWatcherService->m_searchableDirForSearchApplication.clear();
    g_dirWatcherService->m_blackListOfDir.clear();
    g_dirWatcherService->m_extraDirs.clear();
    g_dirWatcherService->m_extraDirsOfPaths.clear();
    g_dirWatcherService->m_blockDirsForUser.clear();

    watcher->initDbusService();
    const QStringList indexableDirs = watcher->currentIndexableDir();
    const QStringList indexBlackList = watcher->currentBlackListOfIndex();
    const QStringList searchableDirs = watcher->currentSearchableDir();
    const QStringList appSearchableDirs = watcher->searchableDirForSearchApplication();
    const QStringList blackList = watcher->blackListOfDir(QStringLiteral("/tmp"));
    const QStringList extraDirs = watcher->extraDirs();
    const QStringList extraDirsOfPaths = watcher->extraDirsOfPaths({QStringLiteral("/tmp/a"), QStringLiteral("/tmp/b")});
    const QStringList blockDirs = watcher->getBlockDirsOfUser();

    QVERIFY(std::all_of(indexableDirs.cbegin(), indexableDirs.cend(), [](const QString &path) { return !path.isEmpty(); }));
    QVERIFY(std::all_of(indexBlackList.cbegin(), indexBlackList.cend(), [](const QString &path) { return !path.isEmpty(); }));
    QVERIFY(std::all_of(searchableDirs.cbegin(), searchableDirs.cend(), [](const QString &path) { return !path.isEmpty(); }));
    QVERIFY(std::all_of(appSearchableDirs.cbegin(), appSearchableDirs.cend(), [](const QString &path) { return !path.isEmpty(); }));
    QVERIFY(std::all_of(blackList.cbegin(), blackList.cend(), [](const QString &path) { return !path.isEmpty(); }));
    QVERIFY(std::all_of(extraDirs.cbegin(), extraDirs.cend(), [](const QString &path) { return !path.isEmpty(); }));
    QVERIFY(std::all_of(extraDirsOfPaths.cbegin(), extraDirsOfPaths.cend(), [](const QString &path) { return !path.isEmpty(); }));
    QVERIFY(std::all_of(blockDirs.cbegin(), blockDirs.cend(), [](const QString &path) { return !path.isEmpty(); }));

    watcher->appendSearchDir(QStringLiteral("/tmp/add"));
    watcher->removeSearchDir(QStringLiteral("/tmp/remove"));
    watcher->appendIndexableListItem(QStringLiteral("/tmp/index"));
    watcher->removeIndexableListItem(QStringLiteral("/tmp/index"));

    SearchDirConfig config;
    const QStringList configBlockDirs = config.getBlockDirOfUser();
    QVERIFY(std::all_of(configBlockDirs.cbegin(), configBlockDirs.cend(), [](const QString &path) { return !path.isEmpty(); }));
}

QTEST_MAIN(DirWatcherTest)

#include "dir-watcher-test.moc"
