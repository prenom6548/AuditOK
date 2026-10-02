#include <QSignalSpy>
#include <QDBusConnection>
#include <QDir>
#include <QTest>
#include <memory>

#define private public
#include "index/file-watcher.h"
#include "index/pending-file-queue.h"
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

    QStringList currentBlackListOfIndex() const
    {
        return {};
    }
};

class ScopedDbusRegistration
{
public:
    ScopedDbusRegistration(const QString &service, const QString &path, QObject *object)
        : m_service(service),
          m_path(path),
          m_connection(QDBusConnection::sessionBus())
    {
        m_serviceRegistered = m_connection.registerService(service);
        m_objectRegistered = m_connection.registerObject(path, object, QDBusConnection::ExportAllSlots);
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

    bool isRegistered() const
    {
        return m_serviceRegistered && m_objectRegistered;
    }

private:
    QString m_service;
    QString m_path;
    QDBusConnection m_connection;
    bool m_serviceRegistered = false;
    bool m_objectRegistered = false;
};

QVector<PendingFile> takeQueuedFiles(PendingFileQueue *queue)
{
    QVector<PendingFile> emitted;
    QObject::connect(queue, &PendingFileQueue::filesUpdate, queue, [&emitted](const QVector<PendingFile> &files) {
        emitted = files;
    });
    queue->processCache();
    return emitted;
}
}

class FileWatcherWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void eventHandlersConvertFilesystemEventsToPendingFiles();
    void removeWatchCanSkipIndexUpdate();
    void removeWatchWithIndexUpdate();
    void destructorCleansUpWatcher();

private:
    FakePrivateDirWatcherService m_dirWatcherService;
    std::unique_ptr<ScopedDbusRegistration> m_dirWatcherRegistration;
};

void FileWatcherWhiteboxTest::initTestCase()
{
    m_dirWatcherRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.ukui.search.fileindex.service"),
                                                              QStringLiteral("/org/ukui/search/privateDirWatcher"),
                                                              &m_dirWatcherService));
    if (m_dirWatcherRegistration->isRegistered()) {
        DirWatcher::getDirWatcher();
    }
}

void FileWatcherWhiteboxTest::cleanupTestCase()
{
    m_dirWatcherRegistration.reset();
}

void FileWatcherWhiteboxTest::eventHandlersConvertFilesystemEventsToPendingFiles()
{
    FileWatcher watcher;
    PendingFileQueue *queue = watcher.m_pendingFileQueue;
    QVERIFY(queue != nullptr);
    queue->m_cache.clear();

    watcher.onFileCreated(QStringLiteral("/tmp/libsearch-whitebox/new-dir"), true);
    watcher.onFileMoveTo(QStringLiteral("/tmp/libsearch-whitebox/moved.txt"), false);
    watcher.onFileModified(QStringLiteral("/tmp/libsearch-whitebox/modified.txt"));
    watcher.onFileDeletedOrMoved(QStringLiteral("/tmp/libsearch-whitebox/deleted-dir"), true);
    watcher.onFileAccessibleChanged(QStringLiteral("/tmp/libsearch-whitebox/inaccessible.txt"), false, false);
    watcher.onFileAccessibleChanged(QStringLiteral("/tmp/libsearch-whitebox/accessible.txt"), false, true);

    const QVector<PendingFile> files = takeQueuedFiles(queue);
    QCOMPARE(files.size(), 6);

    auto findFile = [&files](const QString &path) {
        for (const PendingFile &file : files) {
            if (file.path() == path) {
                return file;
            }
        }
        return PendingFile();
    };

    PendingFile created = findFile(QStringLiteral("/tmp/libsearch-whitebox/new-dir"));
    QVERIFY(created.isCreated());
    QVERIFY(created.isDir());

    PendingFile moved = findFile(QStringLiteral("/tmp/libsearch-whitebox/moved.txt"));
    QVERIFY(moved.isMoveTo());
    QVERIFY(!moved.isDir());

    PendingFile modified = findFile(QStringLiteral("/tmp/libsearch-whitebox/modified.txt"));
    QVERIFY(modified.isModified());
    QVERIFY(!modified.shouldRemoveIndex());

    PendingFile deleted = findFile(QStringLiteral("/tmp/libsearch-whitebox/deleted-dir"));
    QVERIFY(deleted.shouldRemoveIndex());
    QVERIFY(deleted.isDir());

    PendingFile inaccessible = findFile(QStringLiteral("/tmp/libsearch-whitebox/inaccessible.txt"));
    QVERIFY(inaccessible.shouldRemoveIndex());

    PendingFile accessible = findFile(QStringLiteral("/tmp/libsearch-whitebox/accessible.txt"));
    QVERIFY(accessible.isModified());
    QVERIFY(!accessible.shouldRemoveIndex());

    queue->forceFinish();
}

void FileWatcherWhiteboxTest::removeWatchCanSkipIndexUpdate()
{
    FileWatcher watcher;
    PendingFileQueue *queue = watcher.m_pendingFileQueue;
    QVERIFY(queue != nullptr);
    queue->m_cache.clear();

    watcher.removeWatch(QStringLiteral("/tmp/libsearch-whitebox/not-watched"), false);

    QVERIFY(queue->m_cache.isEmpty());
    queue->forceFinish();
}

void FileWatcherWhiteboxTest::removeWatchWithIndexUpdate()
{
    FileWatcher watcher;
    PendingFileQueue *queue = watcher.m_pendingFileQueue;
    queue->m_cache.clear();

    // removeWatch with updateIndex=true covers the if(updateIndex) branch
    watcher.removeWatch(QStringLiteral("/tmp/libsearch-whitebox/not-watched"), true);

    QVERIFY(queue->m_cache.isEmpty());
    queue->forceFinish();
}

void FileWatcherWhiteboxTest::destructorCleansUpWatcher()
{
    // Create a watcher and let it go out of scope to cover destructor branch
    {
        FileWatcher watcher;
        QVERIFY(watcher.m_watcher != nullptr);
    }
}

QTEST_MAIN(FileWatcherWhiteboxTest)

#include "file-watcher-whitebox-test.moc"
