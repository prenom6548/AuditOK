#include <QDir>
#include <QDBusConnection>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <cstring>
#include <memory>
#include <sys/inotify.h>
#include <unistd.h>

#define private public
#include "filesystemwatcher/file-system-watcher.h"
#include "filesystemwatcher/file-system-watcher-private.h"
#undef private

using namespace UkuiSearch;

namespace {

class FakePrivateDirWatcherService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.search.fileindex")

public Q_SLOTS:
    QStringList currentIndexableDir() const
    {
        return QStringList{QDir::homePath()};
    }

    QStringList blackListOfDir(const QString &) const
    {
        return {};
    }

    QStringList blockDirsForUser() const
    {
        return {};
    }

    QStringList currentBlackListOfIndex() const
    {
        return {};
    }

    QStringList extraDirs() const
    {
        return {};
    }

    QStringList currentSearchableDir() const
    {
        return {};
    }

    QStringList searchableDirForSearchApplication() const
    {
        return {};
    }

    QStringList extraDirsOfPaths(const QStringList &) const
    {
        return {};
    }

    int appendSearchDir(const QString &)
    {
        return 0;
    }

    void removeSearchDir(const QString &)
    {
    }

    int appendIndexableListItem(const QString &)
    {
        return 0;
    }

    void removeIndexableListItem(const QString &)
    {
    }

    void initDbusService()
    {
    }
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
        m_objectRegistered = m_connection.registerObject(path, object,
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
}

class FileSystemWatcherWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void addRemoveClearAndTraverseUseTemporaryTree();
    void realInotifyEventsEmitSignals();
    void renameAccessPermissionAndDeleteEventsCoverMoreBranches();
    void syntheticEventsCoverDirectoryLifecycleBranches();
    void overflowAndHiddenSyntheticEventsAreIgnored();

private:
    FakePrivateDirWatcherService m_dirWatcherService;
    std::unique_ptr<ScopedDbusRegistration> m_dirWatcherRegistration;
};

static bool writeFile(const QString &path, const QByteArray &data)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(data);
    return true;
}

static bool spyContainsPathAndDir(const QSignalSpy &spy, const QString &path, bool isDir)
{
    for (const QList<QVariant> &args : spy) {
        if (args.value(0).toString() == path && args.value(1).toBool() == isDir) {
            return true;
        }
    }
    return false;
}

static bool accessibleSpyContains(const QSignalSpy &spy, const QString &path, bool isDir, bool accessible)
{
    for (const QList<QVariant> &args : spy) {
        if (args.value(0).toString() == path
            && args.value(1).toBool() == isDir
            && args.value(2).toBool() == accessible) {
            return true;
        }
    }
    return false;
}

static bool spyContainsPath(const QSignalSpy &spy, const QString &path)
{
    for (const QList<QVariant> &args : spy) {
        if (args.value(0).toString() == path) {
            return true;
        }
    }
    return false;
}

static QByteArray makeInotifyEvent(int wd, uint32_t mask, const QByteArray &name = QByteArray())
{
    const int len = qMax(1, name.size() + 1);
    QByteArray bytes(static_cast<int>(sizeof(inotify_event)) + len, '\0');
    auto *event = reinterpret_cast<inotify_event *>(bytes.data());
    event->wd = wd;
    event->mask = mask;
    event->cookie = 0;
    event->len = static_cast<uint32_t>(len);
    if (!name.isEmpty()) {
        std::memcpy(event->name, name.constData(), static_cast<size_t>(name.size()));
    }
    return bytes;
}

static bool processSyntheticEvents(FileSystemWatcher &watcher, const QList<QByteArray> &events)
{
    int pipefd[2];
    if (::pipe(pipefd) != 0) {
        return false;
    }

    QByteArray payload;
    for (const QByteArray &event : events) {
        payload.append(event);
    }

    const bool writeOk = ::write(pipefd[1], payload.constData(), static_cast<size_t>(payload.size()))
            == payload.size();
    ::close(pipefd[1]);
    if (writeOk) {
        watcher.eventProcess(pipefd[0]);
    }
    ::close(pipefd[0]);
    return writeOk;
}

void FileSystemWatcherWhiteboxTest::initTestCase()
{
    if (QDBusConnection::sessionBus().isConnected()) {
        m_dirWatcherRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.ukui.search.fileindex.service"),
                                                                  QStringLiteral("/org/ukui/search/privateDirWatcher"),
                                                                  &m_dirWatcherService));
    }
}

void FileSystemWatcherWhiteboxTest::cleanupTestCase()
{
    m_dirWatcherRegistration.reset();
}

void FileSystemWatcherWhiteboxTest::addRemoveClearAndTraverseUseTemporaryTree()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString root = tempDir.path();
    const QString child = root + QStringLiteral("/child");
    const QString blocked = root + QStringLiteral("/blocked");
    QVERIFY(QDir(root).mkdir(QStringLiteral("child")));
    QVERIFY(QDir(root).mkdir(QStringLiteral("blocked")));
    QVERIFY(writeFile(child + QStringLiteral("/file.txt"), "content"));

    FileSystemWatcher watcher(true);
    watcher.addWatchWithBlackList(QStringList{root}, QStringList{blocked});

    QVERIFY(watcher.d->m_watchedRootPaths.contains(root));
    QVERIFY(watcher.d->m_watchPathHash.values().contains(root));
    QVERIFY(watcher.d->m_watchPathHash.values().contains(child));
    QVERIFY(!watcher.d->m_watchPathHash.values().contains(blocked));

    const QStringList traversed = watcher.traverse(child);
    QVERIFY(traversed.contains(child + QStringLiteral("/file.txt")));

    const QStringList removed = watcher.removeWatch(root);
    QVERIFY(removed.contains(root));
    QVERIFY(!watcher.d->m_watchedRootPaths.contains(root));

    watcher.addWatch(root);
    QVERIFY(!watcher.d->m_watchPathHash.isEmpty());
    watcher.clearAll();
    QVERIFY(watcher.d->m_watchPathHash.isEmpty());
    QVERIFY(watcher.d->m_watchedRootPaths.isEmpty());
    QVERIFY(watcher.d->m_inotifyFd > 0);
}

void FileSystemWatcherWhiteboxTest::realInotifyEventsEmitSignals()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString root = tempDir.path();
    const QString filePath = root + QStringLiteral("/watched.txt");

    FileSystemWatcher watcher(false,
                              FileSystemWatcher::WatchEvents(FileSystemWatcher::EventCreate |
                                                             FileSystemWatcher::EventModify |
                                                             FileSystemWatcher::EventDelete |
                                                             FileSystemWatcher::EventOpen |
                                                             FileSystemWatcher::EventCloseWrite |
                                                             FileSystemWatcher::EventAttributeChange));
    QSignalSpy createdSpy(&watcher, &FileSystemWatcher::created);
    QSignalSpy modifiedSpy(&watcher, &FileSystemWatcher::modified);
    QSignalSpy deletedSpy(&watcher, &FileSystemWatcher::deleted);
    QSignalSpy openedSpy(&watcher, &FileSystemWatcher::opened);
    QSignalSpy closedWriteSpy(&watcher, &FileSystemWatcher::closedWrite);
    QSignalSpy attributeSpy(&watcher, &FileSystemWatcher::attributeChanged);

    watcher.addWatch(root);
    QVERIFY(writeFile(filePath, "one"));
    QTRY_VERIFY_WITH_TIMEOUT(createdSpy.count() > 0, 2000);
    QTRY_VERIFY_WITH_TIMEOUT(closedWriteSpy.count() > 0, 2000);

    QVERIFY(writeFile(filePath, "two"));
    QTRY_VERIFY_WITH_TIMEOUT(modifiedSpy.count() > 0 || closedWriteSpy.count() > 1, 2000);

    QFile::setPermissions(filePath, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
    QTRY_VERIFY_WITH_TIMEOUT(attributeSpy.count() > 0, 2000);

    QFile readFile(filePath);
    QVERIFY(readFile.open(QIODevice::ReadOnly));
    readFile.readAll();
    readFile.close();
    QTRY_VERIFY_WITH_TIMEOUT(openedSpy.count() > 0, 2000);

    QVERIFY(QFile::remove(filePath));
    QTRY_VERIFY_WITH_TIMEOUT(deletedSpy.count() > 0, 2000);
}

void FileSystemWatcherWhiteboxTest::renameAccessPermissionAndDeleteEventsCoverMoreBranches()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString root = tempDir.path();
    const QString watchedDir = root + QStringLiteral("/watched-dir");
    const QString nestedFile = watchedDir + QStringLiteral("/nested.txt");
    const QString watchedFile = root + QStringLiteral("/watched.txt");
    const QString renamedDir = root + QStringLiteral("/renamed-dir");
    const QString renamedFile = root + QStringLiteral("/renamed.txt");

    QVERIFY(QDir(root).mkdir(QStringLiteral("watched-dir")));
    QVERIFY(writeFile(nestedFile, "nested"));
    QVERIFY(writeFile(watchedFile, "alpha"));

    FileSystemWatcher watcher(true,
                              FileSystemWatcher::WatchEvents(FileSystemWatcher::EventAccess |
                                                             FileSystemWatcher::EventAttributeChange |
                                                             FileSystemWatcher::EventCloseRead |
                                                             FileSystemWatcher::EventDelete |
                                                             FileSystemWatcher::EventDeleteSelf |
                                                             FileSystemWatcher::EventMove |
                                                             FileSystemWatcher::EventMoveSelf));
    QSignalSpy accessedSpy(&watcher, &FileSystemWatcher::accessed);
    QSignalSpy attributeSpy(&watcher, &FileSystemWatcher::attributeChanged);
    QSignalSpy closeReadSpy(&watcher, &FileSystemWatcher::closedRead);
    QSignalSpy deletedSpy(&watcher, &FileSystemWatcher::deleted);
    QSignalSpy movedSpy(&watcher, &FileSystemWatcher::moved);
    QSignalSpy moveToSpy(&watcher, &FileSystemWatcher::moveTo);
    QSignalSpy accessibleSpy(&watcher, &FileSystemWatcher::accessibleChanged);

    watcher.addWatch(root);

    QVERIFY(QFile::rename(watchedFile, renamedFile));
    QTRY_VERIFY_WITH_TIMEOUT(spyContainsPathAndDir(movedSpy, watchedFile, false), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(spyContainsPathAndDir(moveToSpy, renamedFile, false), 3000);

    QFile readFile(renamedFile);
    QVERIFY(readFile.open(QIODevice::ReadOnly));
    readFile.readAll();
    readFile.close();
    QTRY_VERIFY_WITH_TIMEOUT(accessedSpy.count() > 0, 3000);
    QTRY_VERIFY_WITH_TIMEOUT(closeReadSpy.count() > 0, 3000);

    QVERIFY(QFile::setPermissions(renamedFile, {}));
    QTRY_VERIFY_WITH_TIMEOUT(attributeSpy.count() > 0, 3000);
    QTRY_VERIFY_WITH_TIMEOUT(accessibleSpyContains(accessibleSpy, renamedFile, false, false), 3000);
    QVERIFY(QFile::setPermissions(renamedFile, QFileDevice::ReadOwner | QFileDevice::WriteOwner));
    QTRY_VERIFY_WITH_TIMEOUT(accessibleSpyContains(accessibleSpy, renamedFile, false, true), 3000);

    QVERIFY(QDir(root).rename(QStringLiteral("watched-dir"), QStringLiteral("renamed-dir")));
    QTRY_VERIFY_WITH_TIMEOUT(spyContainsPathAndDir(movedSpy, watchedDir, true), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(spyContainsPathAndDir(moveToSpy, renamedDir, true), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(spyContainsPathAndDir(moveToSpy, renamedDir + QStringLiteral("/nested.txt"), false), 3000);

    QVERIFY(QFile::remove(renamedDir + QStringLiteral("/nested.txt")));
    QTRY_VERIFY_WITH_TIMEOUT(spyContainsPathAndDir(deletedSpy, renamedDir + QStringLiteral("/nested.txt"), false), 3000);

    QVERIFY(QDir(root).rmdir(QStringLiteral("renamed-dir")));
    QTRY_VERIFY_WITH_TIMEOUT(spyContainsPathAndDir(deletedSpy, renamedDir, true), 3000);
}

void FileSystemWatcherWhiteboxTest::syntheticEventsCoverDirectoryLifecycleBranches()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString root = tempDir.path();
    const QString attrDir = root + QStringLiteral("/attr-dir");
    const QString attrFile = attrDir + QStringLiteral("/nested.txt");
    const QString createdDir = root + QStringLiteral("/created-dir");
    const QString createdFile = createdDir + QStringLiteral("/created.txt");
    const QString deletedDir = root + QStringLiteral("/deleted-dir");
    const QString deletedFile = deletedDir + QStringLiteral("/deleted.txt");
    const QString movedFromDir = root + QStringLiteral("/moved-from-dir");
    const QString movedFromFile = movedFromDir + QStringLiteral("/moved.txt");
    const QString movedToDir = root + QStringLiteral("/moved-to-dir");
    const QString movedToFile = movedToDir + QStringLiteral("/moved-to.txt");
    const QString deleteSelfDir = root + QStringLiteral("/delete-self-dir");
    const QString deleteSelfFile = deleteSelfDir + QStringLiteral("/delete-self.txt");
    const QString moveSelfDir = root + QStringLiteral("/move-self-dir");
    const QString moveSelfFile = moveSelfDir + QStringLiteral("/move-self.txt");
    const QString mountDir = root + QStringLiteral("/mount-dir");

    QVERIFY(QDir(root).mkpath(QStringLiteral("attr-dir")));
    QVERIFY(QDir(root).mkpath(QStringLiteral("created-dir")));
    QVERIFY(QDir(root).mkpath(QStringLiteral("deleted-dir")));
    QVERIFY(QDir(root).mkpath(QStringLiteral("moved-from-dir")));
    QVERIFY(QDir(root).mkpath(QStringLiteral("moved-to-dir")));
    QVERIFY(QDir(root).mkpath(QStringLiteral("delete-self-dir")));
    QVERIFY(QDir(root).mkpath(QStringLiteral("move-self-dir")));
    QVERIFY(QDir(root).mkpath(QStringLiteral("mount-dir")));
    QVERIFY(writeFile(attrFile, "attr"));
    QVERIFY(writeFile(createdFile, "created"));
    QVERIFY(writeFile(deletedFile, "deleted"));
    QVERIFY(writeFile(movedFromFile, "moved-from"));
    QVERIFY(writeFile(movedToFile, "moved-to"));
    QVERIFY(writeFile(deleteSelfFile, "delete-self"));
    QVERIFY(writeFile(moveSelfFile, "move-self"));

    FileSystemWatcher watcher(true, FileSystemWatcher::WatchEvents(FileSystemWatcher::EventAll | FileSystemWatcher::EventUnmount));
    QSignalSpy createdSpy(&watcher, &FileSystemWatcher::created);
    QSignalSpy deletedSpy(&watcher, &FileSystemWatcher::deleted);
    QSignalSpy movedSpy(&watcher, &FileSystemWatcher::moved);
    QSignalSpy moveToSpy(&watcher, &FileSystemWatcher::moveTo);
    QSignalSpy unmountedSpy(&watcher, &FileSystemWatcher::unmounted);
    QSignalSpy accessibleSpy(&watcher, &FileSystemWatcher::accessibleChanged);
    const int rootWd = 1001;
    const int attrDirWd = 1002;
    const int attrFileWd = 1003;
    const int deletedDirWd = 1004;
    const int deletedFileWd = 1005;
    const int movedFromDirWd = 1006;
    const int movedFromFileWd = 1007;
    const int deleteSelfDirWd = 1008;
    const int deleteSelfFileWd = 1009;
    const int moveSelfDirWd = 1010;
    const int moveSelfFileWd = 1011;
    const int mountDirWd = 1012;

    watcher.d->m_watchPathHash.clear();
    watcher.d->m_watchedRootPaths.clear();
    watcher.d->m_watchPathHash.insert(rootWd, root);
    watcher.d->m_watchPathHash.insert(attrDirWd, attrDir);
    watcher.d->m_watchPathHash.insert(attrFileWd, attrFile);
    watcher.d->m_watchPathHash.insert(deletedDirWd, deletedDir);
    watcher.d->m_watchPathHash.insert(deletedFileWd, deletedFile);
    watcher.d->m_watchPathHash.insert(movedFromDirWd, movedFromDir);
    watcher.d->m_watchPathHash.insert(movedFromFileWd, movedFromFile);
    watcher.d->m_watchPathHash.insert(deleteSelfDirWd, deleteSelfDir);
    watcher.d->m_watchPathHash.insert(deleteSelfFileWd, deleteSelfFile);
    watcher.d->m_watchPathHash.insert(moveSelfDirWd, moveSelfDir);
    watcher.d->m_watchPathHash.insert(moveSelfFileWd, moveSelfFile);
    watcher.d->m_watchPathHash.insert(mountDirWd, mountDir);
    watcher.d->m_watchedRootPaths << deleteSelfDir << moveSelfDir << mountDir;

    QVERIFY(QFile::setPermissions(attrDir, {}));
    QVERIFY(processSyntheticEvents(watcher, {makeInotifyEvent(rootWd, IN_ATTRIB | IN_ISDIR, "attr-dir")}));
    QVERIFY(accessibleSpyContains(accessibleSpy, attrDir, true, false));
    QVERIFY(!watcher.d->m_watchPathHash.values().contains(attrDir));

    QVERIFY(QFile::setPermissions(attrDir, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));
    QVERIFY(processSyntheticEvents(watcher, {makeInotifyEvent(rootWd, IN_ATTRIB | IN_ISDIR, "attr-dir")}));
    QVERIFY(accessibleSpyContains(accessibleSpy, attrFile, false, true));

    QVERIFY(processSyntheticEvents(watcher, {makeInotifyEvent(rootWd, IN_CREATE | IN_ISDIR, "created-dir")}));
    QVERIFY(spyContainsPathAndDir(createdSpy, createdDir, true));
    QVERIFY(spyContainsPathAndDir(createdSpy, createdFile, false));

    QVERIFY(processSyntheticEvents(watcher, {makeInotifyEvent(rootWd, IN_DELETE | IN_ISDIR, "deleted-dir")}));
    QVERIFY(spyContainsPath(deletedSpy, deletedDir));
    QVERIFY(!watcher.d->m_watchPathHash.values().contains(deletedDir));

    QVERIFY(processSyntheticEvents(watcher, {makeInotifyEvent(rootWd, IN_MOVED_FROM | IN_ISDIR, "moved-from-dir")}));
    QVERIFY(spyContainsPath(movedSpy, movedFromDir));
    QVERIFY(!watcher.d->m_watchPathHash.values().contains(movedFromDir));

    QVERIFY(processSyntheticEvents(watcher, {makeInotifyEvent(rootWd, IN_MOVED_TO | IN_ISDIR, "moved-to-dir")}));
    QVERIFY(spyContainsPathAndDir(moveToSpy, movedToDir, true));
    QVERIFY(spyContainsPathAndDir(moveToSpy, movedToFile, false));

    QVERIFY(processSyntheticEvents(watcher, {makeInotifyEvent(deleteSelfDirWd, IN_DELETE_SELF)}));
    QVERIFY(spyContainsPath(deletedSpy, deleteSelfDir));
    QVERIFY(!watcher.d->m_watchedRootPaths.contains(deleteSelfDir));

    QVERIFY(processSyntheticEvents(watcher, {makeInotifyEvent(moveSelfDirWd, IN_MOVE_SELF)}));
    QVERIFY(spyContainsPath(movedSpy, moveSelfDir));
    QVERIFY(!watcher.d->m_watchedRootPaths.contains(moveSelfDir));

    QVERIFY(processSyntheticEvents(watcher, {makeInotifyEvent(mountDirWd, IN_UNMOUNT)}));
    QVERIFY(spyContainsPathAndDir(unmountedSpy, mountDir, false));
    QVERIFY(!watcher.d->m_watchedRootPaths.contains(mountDir));

    QVERIFY(QFile::setPermissions(attrDir, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));
}

void FileSystemWatcherWhiteboxTest::overflowAndHiddenSyntheticEventsAreIgnored()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    FileSystemWatcher watcher(false);
    watcher.d->m_watchPathHash.clear();
    watcher.d->m_watchPathHash.insert(1001, tempDir.path());

    QSignalSpy createdSpy(&watcher, &FileSystemWatcher::created);

    QVERIFY(processSyntheticEvents(watcher, {makeInotifyEvent(1001, IN_CREATE, ".hidden")}));
    QCOMPARE(createdSpy.count(), 0);

    QVERIFY(processSyntheticEvents(watcher, {makeInotifyEvent(-1, IN_Q_OVERFLOW, "x")}));
    QCOMPARE(createdSpy.count(), 0);
}

QTEST_MAIN(FileSystemWatcherWhiteboxTest)
#include "file-system-watcher-whitebox-test.moc"
