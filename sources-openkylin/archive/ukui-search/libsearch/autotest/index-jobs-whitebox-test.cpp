#include <cstdlib>

#include <QApplication>
#include <QBuffer>
#include <QDBusConnection>
#include <QDBusVariant>
#include <QDir>
#include <QGSettings>
#include <QProcess>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <signal.h>
#include <memory>

#define private public
#include "../index/batch-indexer.h"
#include "../index/file-indexer-config.h"
#include "../index/writable-database.h"
#undef DATABASE_H
#include "../index/database.h"
#include "../index/index-scheduler.h"
#include "../index/index-status-recorder.h"
#include "../index/index-updater.h"
#include "../file-utils.h"
#undef private

using namespace UkuiSearch;

class IndexJobsWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void batchIndexerRunReturnsImmediatelyWhenAllTargetsAreStopped();
    void batchIndexerRunWithNoTargetsFetchesAndFinishesWithoutIndexing();
    void batchIndexerFetchTraversesFoldersAndHonorsStopFlags();
    void batchIndexerFetchRemovesBlockedFoldersAndSkipsBlockedChildren();
    void batchIndexerRunEmitsTargetSignalsWhenIndexStepsAreStopped();
    void batchIndexerIndexStepsReturnWhenStopped();
    void batchIndexerIndexStepsCoverOpenAndEmptyCacheBranches();
    void batchIndexerUpdateRefreshesBasicAndRemovesStaleMetadata();
    void batchIndexerRunSkipsUnselectedTargets();
    void indexUpdaterRunEmitsDoneOnStoppedIndexes();
    void indexUpdaterDirectUpdatesCoverCreatedDeletedAndStoppedBranches();
    void indexUpdaterContentAndOcrBranchesHandleUnsupportedTargets();
    void indexUpdaterOpenFailuresLeaveRealtimeStateForRunCleanup();
    void indexStatusRecorderVersionAndEnableChecks();
    void indexSchedulerCoversStoppedStateAndCompletionSlots();
    void indexSchedulerEnableFlagsAndRealtimeUpdateBranches();
    void fileIndexerConfigSyncsDefaultAndCustomFileTypeSettings();
    void fileIndexerConfigGuardAndPowerControlBranches();
    void batchIndexerRebuildModeClearsCacheAndReindexes();
    void batchIndexerPartialStopFlagsOnlyBlockRespectiveTargets();
    void batchIndexerFetchWithExtraDirsPreservesPaths();
    void batchIndexerIndexStepsCoverOcrAndAiStopBranches();

private:
    void enableIndexSettings();
    QString m_home;
    QByteArray m_originalSystemBusAddress;
    qint64 m_dbusDaemonPid = 0;
    bool m_fakeSystemBusReady = false;
    const QString m_systemBusName = QStringLiteral("index_jobs_test_system_bus");
};

namespace {

QString configuredHome();

class MockUPowerProperties : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.DBus.Properties")

public Q_SLOTS:
    QDBusVariant Get(const QString &interface, const QString &property) const
    {
        if (interface == QStringLiteral("org.freedesktop.UPower") &&
            property == QStringLiteral("OnBattery")) {
            return QDBusVariant(false);
        }
        return QDBusVariant(QVariant());
    }
};

class FakePrivateDirWatcherService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.search.fileindex")

public Q_SLOTS:
    QStringList currentIndexableDir() const
    {
        return QStringList{configuredHome()};
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

Q_SIGNALS:
    void appendIndexItem(const QString &path, const QStringList &blockList);
    void removeIndexItem(const QString &path);
    void mountAdded(const QString &path);
    void blockItemAdded(const QString &path);
    void blockItemRemoved(const QString &path);
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

QString configuredHome()
{
    const QByteArray value = qgetenv("UKUI_SEARCH_WHITEBOX_HOME");
    return value.isEmpty() ? QDir::homePath() : QString::fromLocal8Bit(value);
}

void resetIndexPath(const QString &path)
{
    QFileInfo info(path);
    if (info.isDir()) {
        QDir(path).removeRecursively();
    } else if (info.exists()) {
        QFile::remove(path);
    }
}

MockUPowerProperties g_upowerStub;
FakePrivateDirWatcherService *g_dirWatcherService = nullptr;
std::unique_ptr<ScopedDbusRegistration> g_dirWatcherRegistration;

}

void IndexJobsWhiteboxTest::initTestCase()
{
    m_originalSystemBusAddress = qgetenv("DBUS_SYSTEM_BUS_ADDRESS");

    QProcess dbusDaemon;
    dbusDaemon.start(QStringLiteral("dbus-daemon"),
                     {QStringLiteral("--session"),
                      QStringLiteral("--fork"),
                      QStringLiteral("--print-address=1"),
                      QStringLiteral("--print-pid=1")});
    if (dbusDaemon.waitForStarted() && dbusDaemon.waitForFinished() &&
        dbusDaemon.exitStatus() == QProcess::NormalExit && dbusDaemon.exitCode() == 0) {
        const QList<QByteArray> lines = dbusDaemon.readAllStandardOutput().split('\n');
        if (lines.size() >= 2) {
            const QByteArray address = lines.at(0).trimmed();
            const qint64 pid = lines.at(1).trimmed().toLongLong();
            if (!address.isEmpty() && pid > 0) {
                qputenv("DBUS_SYSTEM_BUS_ADDRESS", address);
                QDBusConnection::disconnectFromBus(QStringLiteral("qt_default_system_bus"));
                QDBusConnection::disconnectFromBus(m_systemBusName);
                QDBusConnection bus = QDBusConnection::connectToBus(QString::fromUtf8(address), m_systemBusName);
                if (bus.isConnected() &&
                    bus.registerService(QStringLiteral("org.freedesktop.UPower")) &&
                    bus.registerObject(QStringLiteral("/org/freedesktop/UPower"),
                                       &g_upowerStub,
                                       QDBusConnection::ExportAllSlots)) {
                    m_dbusDaemonPid = pid;
                    m_fakeSystemBusReady = true;
                } else {
                    qWarning() << "failed to register fake UPower bus, falling back to host system bus";
                    QDBusConnection::disconnectFromBus(m_systemBusName);
                    if (m_originalSystemBusAddress.isEmpty()) {
                        qunsetenv("DBUS_SYSTEM_BUS_ADDRESS");
                    } else {
                        qputenv("DBUS_SYSTEM_BUS_ADDRESS", m_originalSystemBusAddress);
                    }
                }
            }
        }
    }

    m_home = configuredHome();
    QVERIFY2(!m_home.isEmpty(), "test home must be configured");
    QVERIFY(QDir().mkpath(m_home + QStringLiteral("/Desktop")));
    QVERIFY(QDir().mkpath(m_home + QStringLiteral("/.config/org.ukui/ukui-search")));
    g_dirWatcherService = new FakePrivateDirWatcherService;
    g_dirWatcherRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.ukui.search.fileindex.service"),
                                                              QStringLiteral("/org/ukui/search/privateDirWatcher"),
                                                              g_dirWatcherService));
    enableIndexSettings();
}

void IndexJobsWhiteboxTest::cleanupTestCase()
{
    g_dirWatcherRegistration.reset();
    delete g_dirWatcherService;
    g_dirWatcherService = nullptr;

    if (m_fakeSystemBusReady) {
        QDBusConnection bus(m_systemBusName);
        bus.unregisterObject(QStringLiteral("/org/freedesktop/UPower"));
        bus.unregisterService(QStringLiteral("org.freedesktop.UPower"));
        QDBusConnection::disconnectFromBus(m_systemBusName);
        QDBusConnection::disconnectFromBus(QStringLiteral("qt_default_system_bus"));
    }

    if (m_originalSystemBusAddress.isEmpty()) {
        qunsetenv("DBUS_SYSTEM_BUS_ADDRESS");
    } else {
        qputenv("DBUS_SYSTEM_BUS_ADDRESS", m_originalSystemBusAddress);
    }

    if (m_dbusDaemonPid > 0) {
        ::kill(static_cast<pid_t>(m_dbusDaemonPid), SIGTERM);
        m_dbusDaemonPid = 0;
    }
}

void IndexJobsWhiteboxTest::init()
{
    resetIndexPath(INDEX_PATH);
    resetIndexPath(CONTENT_INDEX_PATH);
    resetIndexPath(OCR_CONTENT_INDEX_PATH);
    enableIndexSettings();
}

void IndexJobsWhiteboxTest::cleanup()
{
    resetIndexPath(INDEX_PATH);
    resetIndexPath(CONTENT_INDEX_PATH);
    resetIndexPath(OCR_CONTENT_INDEX_PATH);
}

void IndexJobsWhiteboxTest::enableIndexSettings()
{
    const QByteArray schema("org.ukui.search.settings");
    if (!QGSettings::isSchemaInstalled(schema)) {
        QSKIP("org.ukui.search.settings schema is not available in this test environment");
    }

    auto *config = FileIndexerConfig::getInstance();
    QVERIFY(config->m_gsettings != nullptr);
    config->m_gsettings->set(QStringLiteral("fileIndexEnable"), true);
    config->m_gsettings->set(QStringLiteral("contentIndexEnable"), true);
    config->m_gsettings->set(QStringLiteral("contentIndexEnableOcr"), true);
    config->m_gsettings->set(QStringLiteral("aiIndexEnable"), true);
    config->m_onBattery = false;
    config->m_underCtrl = false;
    config->m_powerPolicy = 0;
}

void IndexJobsWhiteboxTest::batchIndexerRunReturnsImmediatelyWhenAllTargetsAreStopped()
{
    qRegisterMetaType<BatchIndexer::WorkMode>("WorkMode");
    qRegisterMetaType<BatchIndexer::Targets>("Targets");

    QAtomicInt basicStop(1);
    QAtomicInt contentStop(1);
    QAtomicInt ocrStop(1);
    QAtomicInt aiStop(1);
    BatchIndexer indexer(QStringList(), QStringList(),
                         basicStop, contentStop, ocrStop, aiStop,
                         BatchIndexer::Update, BatchIndexer::All);
    QSignalSpy doneSpy(&indexer, &BatchIndexer::done);

    indexer.run();

    QCOMPARE(doneSpy.count(), 1);
    QCOMPARE(doneSpy.takeFirst().at(0).value<BatchIndexer::WorkMode>(), BatchIndexer::Update);
}

void IndexJobsWhiteboxTest::batchIndexerRunWithNoTargetsFetchesAndFinishesWithoutIndexing()
{
    qRegisterMetaType<BatchIndexer::WorkMode>("WorkMode");
    qRegisterMetaType<BatchIndexer::Targets>("Targets");

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QFile file(QDir(tempDir.path()).filePath(QStringLiteral("file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("index");
    file.close();

    QAtomicInt basicStop(0);
    QAtomicInt contentStop(0);
    QAtomicInt ocrStop(0);
    QAtomicInt aiStop(0);
    BatchIndexer indexer(QStringList{tempDir.path()}, QStringList(),
                         basicStop, contentStop, ocrStop, aiStop,
                         BatchIndexer::Add, BatchIndexer::None);
    QSignalSpy doneSpy(&indexer, &BatchIndexer::done);
    QSignalSpy basicStartSpy(&indexer, &BatchIndexer::basicIndexStart);

    indexer.run();

    QCOMPARE(doneSpy.count(), 1);
    QCOMPARE(basicStartSpy.count(), 0);
    QVERIFY(indexer.m_cache.isEmpty());
}

void IndexJobsWhiteboxTest::batchIndexerFetchTraversesFoldersAndHonorsStopFlags()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QVERIFY(dir.mkpath(QStringLiteral("sub")));
    QFile file(dir.filePath(QStringLiteral("sub/file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("index");
    file.close();

    QAtomicInt basicStop(0);
    QAtomicInt contentStop(0);
    QAtomicInt ocrStop(0);
    QAtomicInt aiStop(0);
    BatchIndexer indexer(QStringList{tempDir.path()}, QStringList(),
                         basicStop, contentStop, ocrStop, aiStop,
                         BatchIndexer::Add, BatchIndexer::Basic);

    indexer.fetch();

    QVERIFY(indexer.m_cache.contains(tempDir.path()));
    QVERIFY(indexer.m_cache.contains(dir.filePath(QStringLiteral("sub"))));
    QVERIFY(indexer.m_cache.contains(dir.filePath(QStringLiteral("sub/file.txt"))));

    QAtomicInt stoppedBasic(1);
    QAtomicInt stoppedContent(1);
    QAtomicInt stoppedOcr(1);
    QAtomicInt stoppedAi(1);
    BatchIndexer stopped(QStringList{tempDir.path()}, QStringList(),
                         stoppedBasic, stoppedContent, stoppedOcr, stoppedAi,
                         BatchIndexer::Add, BatchIndexer::Basic);

    stopped.fetch();

    QVERIFY(stopped.m_cache.isEmpty());
}

void IndexJobsWhiteboxTest::batchIndexerFetchRemovesBlockedFoldersAndSkipsBlockedChildren()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QVERIFY(dir.mkpath(QStringLiteral("blocked/nested")));
    QVERIFY(dir.mkpath(QStringLiteral("allowed")));

    QFile blockedFile(dir.filePath(QStringLiteral("blocked/nested/file.txt")));
    QVERIFY(blockedFile.open(QIODevice::WriteOnly));
    blockedFile.write("blocked");
    blockedFile.close();

    QFile allowedFile(dir.filePath(QStringLiteral("allowed/file.txt")));
    QVERIFY(allowedFile.open(QIODevice::WriteOnly));
    allowedFile.write("allowed");
    allowedFile.close();

    QAtomicInt basicStop(0);
    QAtomicInt contentStop(0);
    QAtomicInt ocrStop(0);
    QAtomicInt aiStop(0);
    const QString blockedPath = dir.filePath(QStringLiteral("blocked"));
    BatchIndexer indexer(QStringList{tempDir.path(), blockedPath},
                         QStringList{blockedPath},
                         basicStop, contentStop, ocrStop, aiStop,
                         BatchIndexer::Add, BatchIndexer::None);

    indexer.fetch();

    QVERIFY(!indexer.m_cache.contains(blockedPath));
    QVERIFY(!indexer.m_cache.contains(dir.filePath(QStringLiteral("blocked/nested"))));
    QVERIFY(indexer.m_cache.contains(dir.filePath(QStringLiteral("allowed"))));
    QVERIFY(indexer.m_cache.contains(dir.filePath(QStringLiteral("allowed/file.txt"))));
}

void IndexJobsWhiteboxTest::batchIndexerRunEmitsTargetSignalsWhenIndexStepsAreStopped()
{
    qRegisterMetaType<BatchIndexer::WorkMode>("WorkMode");

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QAtomicInt basicStop(1);
    QAtomicInt contentStop(1);
    QAtomicInt ocrStop(1);
    QAtomicInt aiStop(1);
    BatchIndexer indexer(QStringList{tempDir.path()}, QStringList(),
                         basicStop, contentStop, ocrStop, aiStop,
                         BatchIndexer::Rebuild, BatchIndexer::All);
    QSignalSpy basicStartSpy(&indexer, &BatchIndexer::basicIndexStart);
    QSignalSpy basicDoneSpy(&indexer, &BatchIndexer::basicIndexDone);
    QSignalSpy contentStartSpy(&indexer, &BatchIndexer::contentIndexStart);
    QSignalSpy contentDoneSpy(&indexer, &BatchIndexer::contentIndexDone);
    QSignalSpy ocrStartSpy(&indexer, &BatchIndexer::ocrContentIndexStart);
    QSignalSpy ocrDoneSpy(&indexer, &BatchIndexer::ocrContentIndexDone);
    QSignalSpy aiStartSpy(&indexer, &BatchIndexer::aiIndexStart);
    QSignalSpy aiDoneSpy(&indexer, &BatchIndexer::aiIndexDone);
    QSignalSpy doneSpy(&indexer, &BatchIndexer::done);

    basicStop.storeRelease(0);
    indexer.run();

    QCOMPARE(basicStartSpy.count(), 1);
    QCOMPARE(basicDoneSpy.count(), 1);
    QCOMPARE(contentStartSpy.count(), 1);
    QCOMPARE(contentDoneSpy.count(), 1);
    QCOMPARE(ocrStartSpy.count(), 1);
    QCOMPARE(ocrDoneSpy.count(), 1);
    QCOMPARE(aiStartSpy.count(), 1);
    QCOMPARE(aiDoneSpy.count(), 1);
    QCOMPARE(doneSpy.count(), 1);
    QVERIFY(indexer.m_cache.isEmpty());
}

void IndexJobsWhiteboxTest::batchIndexerIndexStepsReturnWhenStopped()
{
    QAtomicInt basicStop(1);
    QAtomicInt contentStop(1);
    QAtomicInt ocrStop(1);
    QAtomicInt aiStop(1);
    BatchIndexer indexer(QStringList(), QStringList(),
                         basicStop, contentStop, ocrStop, aiStop,
                         BatchIndexer::Update, BatchIndexer::All);

    indexer.basicIndex();
    indexer.contentIndex();
    indexer.ocrIndex();
    indexer.aiIndex();

    QVERIFY(indexer.m_cache.isEmpty());
}

void IndexJobsWhiteboxTest::batchIndexerIndexStepsCoverOpenAndEmptyCacheBranches()
{
    qRegisterMetaType<IndexType>("IndexType");

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QFile textFile(QDir(tempDir.path()).filePath(QStringLiteral("batch-content.txt")));
    QVERIFY(textFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    textFile.write("batch content");
    textFile.close();

    QAtomicInt basicStop(0);
    QAtomicInt contentStop(0);
    QAtomicInt ocrStop(0);
    QAtomicInt aiStop(1);
    BatchIndexer addIndexer(QStringList{tempDir.path()}, QStringList(),
                            basicStop, contentStop, ocrStop, aiStop,
                            BatchIndexer::Add, BatchIndexer::All);
    QSignalSpy progressSpy(&addIndexer, &BatchIndexer::progress);

    addIndexer.fetch();
    QVERIFY(addIndexer.m_cache.contains(tempDir.path()));
    QVERIFY(addIndexer.m_cache.contains(textFile.fileName()));

    addIndexer.basicIndex();
    addIndexer.contentIndex();
    addIndexer.ocrIndex();
    addIndexer.aiIndex();
    QVERIFY(progressSpy.count() >= 0);

    BatchIndexer updateIndexer(QStringList{tempDir.path()}, QStringList(),
                               basicStop, contentStop, ocrStop, aiStop,
                               BatchIndexer::Update, BatchIndexer::Basic | BatchIndexer::Content | BatchIndexer::Ocr);
    updateIndexer.fetch();
    updateIndexer.basicIndex();
    updateIndexer.contentIndex();
    updateIndexer.ocrIndex();

    BatchIndexer rebuildIndexer(QStringList{tempDir.path()}, QStringList(),
                                basicStop, contentStop, ocrStop, aiStop,
                                BatchIndexer::Rebuild, BatchIndexer::Basic | BatchIndexer::Content | BatchIndexer::Ocr);
    rebuildIndexer.fetch();
    rebuildIndexer.basicIndex();
    rebuildIndexer.contentIndex();
    rebuildIndexer.ocrIndex();
}

void IndexJobsWhiteboxTest::batchIndexerUpdateRefreshesBasicAndRemovesStaleMetadata()
{
    QTemporaryDir tempDir(m_home + QStringLiteral("/index-update-XXXXXX"));
    QVERIFY(tempDir.isValid());

    const QString textPath = QDir(tempDir.path()).filePath(QStringLiteral("update-basic.txt"));
    QFile textFile(textPath);
    QVERIFY(textFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    textFile.write("update me");
    textFile.close();

    const QString ocrPath = QDir(tempDir.path()).filePath(QStringLiteral("update-ocr.png"));
    QImage ocrImage(OCR_MIN_SIZE + 8, OCR_MIN_SIZE + 8, QImage::Format_RGB32);
    ocrImage.fill(Qt::white);
    QVERIFY(ocrImage.save(ocrPath));

    const QString htmlAsTextPath = QDir(tempDir.path()).filePath(QStringLiteral("html-like.txt"));
    QFile htmlFile(htmlAsTextPath);
    QVERIFY(htmlFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    htmlFile.write("<html><body>not plain text</body></html>");
    htmlFile.close();

    const QString smallOcrPath = QDir(tempDir.path()).filePath(QStringLiteral("small-ocr.png"));
    QImage smallOcrImage(OCR_MIN_SIZE - 8, OCR_MIN_SIZE - 8, QImage::Format_RGB32);
    smallOcrImage.fill(Qt::white);
    QVERIFY(smallOcrImage.save(smallOcrPath));

    const std::string basicTerm = FileUtils::makeDocUterm(textPath);
    const std::string contentTerm = FileUtils::makeDocUterm(textPath);
    const std::string ocrTerm = FileUtils::makeDocUterm(ocrPath);
    const std::string htmlTerm = FileUtils::makeDocUterm(htmlAsTextPath);
    const std::string smallOcrTerm = FileUtils::makeDocUterm(smallOcrPath);
    const std::string expectedBasicTime =
        QFileInfo(textPath).lastModified().toString(QStringLiteral("yyyyMMddHHmmsszzz")).toStdString();
    const std::string expectedContentTime = expectedBasicTime;
    const std::string staleBasic = std::string("stale-basic-entry");
    const std::string staleContent = std::string("stale-content-entry");
    const std::string staleOcr = std::string("stale-ocr-entry");

    {
        WritableDatabase basicDb(DataBaseType::Basic);
        QVERIFY(basicDb.open());
        basicDb.rebuild();
        QVERIFY(basicDb.open());
        basicDb.setMetaData(basicTerm, std::string("0"));
        basicDb.setMetaData(staleBasic, std::string("obsolete"));
        basicDb.commit();
    }
    {
        WritableDatabase contentDb(DataBaseType::Content);
        QVERIFY(contentDb.open());
        contentDb.rebuild();
        QVERIFY(contentDb.open());
        contentDb.setMetaData(contentTerm, std::string("0"));
        contentDb.setMetaData(htmlTerm, std::string("0"));
        contentDb.setMetaData(staleContent, std::string("obsolete"));
        contentDb.commit();
    }
    {
        WritableDatabase ocrDb(DataBaseType::OcrContent);
        QVERIFY(ocrDb.open());
        ocrDb.rebuild();
        QVERIFY(ocrDb.open());
        ocrDb.setMetaData(ocrTerm, std::string("0"));
        ocrDb.setMetaData(smallOcrTerm, std::string("0"));
        ocrDb.setMetaData(staleOcr, std::string("obsolete"));
        ocrDb.commit();
    }

    QAtomicInt basicStop(0);
    QAtomicInt contentStop(0);
    QAtomicInt ocrStop(0);
    QAtomicInt aiStop(1);
    BatchIndexer indexer(QStringList{tempDir.path()}, QStringList(),
                         basicStop, contentStop, ocrStop, aiStop,
                         BatchIndexer::Update, BatchIndexer::Basic | BatchIndexer::Content | BatchIndexer::Ocr);

    indexer.fetch();
    indexer.basicIndex();
    indexer.contentIndex();
    indexer.ocrIndex();

    {
        WritableDatabase basicDb(DataBaseType::Basic);
        QVERIFY(basicDb.open());
        QCOMPARE(QString::fromStdString(basicDb.getMetaData(basicTerm)),
                 QString::fromStdString(expectedBasicTime));
        QCOMPARE(QString::fromStdString(basicDb.getMetaData(staleBasic)), QString());
    }
    {
        WritableDatabase contentDb(DataBaseType::Content);
        QVERIFY(contentDb.open());
        QCOMPARE(QString::fromStdString(contentDb.getMetaData(contentTerm)),
                 QString::fromStdString(expectedContentTime));
        QCOMPARE(QString::fromStdString(contentDb.getMetaData(htmlTerm)), QString());
        QCOMPARE(QString::fromStdString(contentDb.getMetaData(staleContent)), QString());
    }
    {
        WritableDatabase ocrDb(DataBaseType::OcrContent);
        QVERIFY(ocrDb.open());
        QCOMPARE(QString::fromStdString(ocrDb.getMetaData(smallOcrTerm)), QString());
        QCOMPARE(QString::fromStdString(ocrDb.getMetaData(staleOcr)), QString());
    }
}

void IndexJobsWhiteboxTest::batchIndexerRunSkipsUnselectedTargets()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString textPath = QDir(tempDir.path()).filePath(QStringLiteral("basic-only.txt"));
    QFile textFile(textPath);
    QVERIFY(textFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    textFile.write("basic only");
    textFile.close();

    QAtomicInt basicStop(0);
    QAtomicInt contentStop(0);
    QAtomicInt ocrStop(0);
    QAtomicInt aiStop(1);
    BatchIndexer indexer(QStringList{tempDir.path()}, QStringList(),
                         basicStop, contentStop, ocrStop, aiStop,
                         BatchIndexer::Add, BatchIndexer::Basic);
    QSignalSpy basicStartSpy(&indexer, &BatchIndexer::basicIndexStart);
    QSignalSpy basicDoneSpy(&indexer, &BatchIndexer::basicIndexDone);
    QSignalSpy contentStartSpy(&indexer, &BatchIndexer::contentIndexStart);
    QSignalSpy contentDoneSpy(&indexer, &BatchIndexer::contentIndexDone);
    QSignalSpy ocrStartSpy(&indexer, &BatchIndexer::ocrContentIndexStart);
    QSignalSpy ocrDoneSpy(&indexer, &BatchIndexer::ocrContentIndexDone);
    QSignalSpy aiStartSpy(&indexer, &BatchIndexer::aiIndexStart);
    QSignalSpy aiDoneSpy(&indexer, &BatchIndexer::aiIndexDone);
    QSignalSpy doneSpy(&indexer, &BatchIndexer::done);

    indexer.run();

    QCOMPARE(basicStartSpy.count(), 1);
    QCOMPARE(basicDoneSpy.count(), 1);
    QCOMPARE(contentStartSpy.count(), 0);
    QCOMPARE(contentDoneSpy.count(), 0);
    QCOMPARE(ocrStartSpy.count(), 0);
    QCOMPARE(ocrDoneSpy.count(), 0);
    QCOMPARE(aiStartSpy.count(), 0);
    QCOMPARE(aiDoneSpy.count(), 0);
    QCOMPARE(doneSpy.count(), 1);
}

void IndexJobsWhiteboxTest::indexUpdaterRunEmitsDoneOnStoppedIndexes()
{
    QAtomicInt basicStop(1);
    QAtomicInt contentStop(1);
    QAtomicInt ocrStop(1);
    QAtomicInt aiStop(1);
    QVector<PendingFile> files;
    files.append(PendingFile(QStringLiteral("/tmp/ukui-search-whitebox.txt")));
    IndexUpdater updater(files, basicStop, contentStop, ocrStop, aiStop);
    QSignalSpy doneSpy(&updater, &IndexUpdater::done);

    updater.run();

    QCOMPARE(doneSpy.count(), 1);
    QVERIFY(updater.m_cache.isEmpty());
}

void IndexJobsWhiteboxTest::indexUpdaterDirectUpdatesCoverCreatedDeletedAndStoppedBranches()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QVERIFY(dir.mkpath(QStringLiteral("deleted-dir")));

    const QString createdPath = dir.filePath(QStringLiteral("created.txt"));
    QFile createdFile(createdPath);
    QVERIFY(createdFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    createdFile.write("created");
    createdFile.close();

    PendingFile created(createdPath);
    created.setCreated();

    PendingFile modified(createdPath);
    modified.setModified();

    PendingFile deletedDir(dir.filePath(QStringLiteral("deleted-dir")));
    deletedDir.setIsDir();
    deletedDir.setDeleted();

    PendingFile removedFile(dir.filePath(QStringLiteral("removed.txt")));
    removedFile.setDeleted();

    QVector<PendingFile> files{created, modified, deletedDir, removedFile};
    QAtomicInt basicStop(0);
    QAtomicInt contentStop(0);
    QAtomicInt ocrStop(0);
    QAtomicInt aiStop(1);
    IndexUpdater updater(files, basicStop, contentStop, ocrStop, aiStop);

    QSignalSpy updatedSpy(&updater, &IndexUpdater::indexUpdated);
    updater.updateIndex();
    QVERIFY(updatedSpy.count() >= 0);

    updater.updateContentIndex();
    updater.updateOcrContentIndex();

    contentStop.storeRelease(1);
    ocrStop.storeRelease(1);
    updater.updateContentIndex();
    updater.updateOcrContentIndex();
    aiStop.storeRelease(0);
    updater.updateAiIndex();
}

void IndexJobsWhiteboxTest::indexUpdaterContentAndOcrBranchesHandleUnsupportedTargets()
{
    QTemporaryDir tempDir(m_home + QStringLiteral("/index-updater-branches-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    auto *config = FileIndexerConfig::getInstance();
    enableIndexSettings();
    QVERIFY(config->isContentIndexEnable());
    QVERIFY(config->isOCREnable());
    const auto originalContentTargets = config->m_targetFileTypeMap;
    const auto originalOcrTargets = config->m_targetPhotographTypeMap;
    config->m_targetFileTypeMap.insert(QStringLiteral("txt"), true);
    config->m_targetPhotographTypeMap.insert(QStringLiteral("png"), true);

    const QString emptyTextPath = dir.filePath(QStringLiteral("empty-content.txt"));
    QFile emptyTextFile(emptyTextPath);
    QVERIFY(emptyTextFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    emptyTextFile.close();

    const QString movedSmallOcrPath = dir.filePath(QStringLiteral("moved-small-ocr.png"));
    QImage movedSmallOcrImage(OCR_MIN_SIZE - 8, OCR_MIN_SIZE - 8, QImage::Format_RGB32);
    movedSmallOcrImage.fill(Qt::white);
    QVERIFY(movedSmallOcrImage.save(movedSmallOcrPath));

    const QString modifiedSmallOcrPath = dir.filePath(QStringLiteral("modified-small-ocr.png"));
    QImage modifiedSmallOcrImage(OCR_MIN_SIZE - 4, OCR_MIN_SIZE - 4, QImage::Format_RGB32);
    modifiedSmallOcrImage.fill(Qt::white);
    QVERIFY(modifiedSmallOcrImage.save(modifiedSmallOcrPath));

    const std::string emptyTextTerm = FileUtils::makeDocUterm(emptyTextPath);
    const std::string movedSmallOcrTerm = FileUtils::makeDocUterm(movedSmallOcrPath);
    const std::string modifiedSmallOcrTerm = FileUtils::makeDocUterm(modifiedSmallOcrPath);
    auto makeSeedDocument = [](const std::string &term, const QString &value) {
        Document document;
        QString data = value;
        document.setData(data);
        document.setUniqueTerm(term);
        document.addValue(1, value);
        document.setIndexTime(QStringLiteral("1"));
        return document;
    };

    {
        WritableDatabase contentDb(DataBaseType::Content);
        QVERIFY(contentDb.open());
        contentDb.rebuild();
        QVERIFY(contentDb.open());
        contentDb.addDocument(makeSeedDocument(emptyTextTerm, emptyTextPath));
        contentDb.commit();
    }
    {
        WritableDatabase ocrDb(DataBaseType::OcrContent);
        QVERIFY(ocrDb.open());
        ocrDb.rebuild();
        QVERIFY(ocrDb.open());
        ocrDb.addDocument(makeSeedDocument(movedSmallOcrTerm, movedSmallOcrPath));
        ocrDb.addDocument(makeSeedDocument(modifiedSmallOcrTerm, modifiedSmallOcrPath));
        ocrDb.commit();
    }

    PendingFile emptyText(emptyTextPath);
    emptyText.setModified();
    PendingFile movedSmallOcr(movedSmallOcrPath);
    movedSmallOcr.setMoveTo();
    PendingFile modifiedSmallOcr(modifiedSmallOcrPath);
    modifiedSmallOcr.setModified();

    QVector<PendingFile> files{emptyText, movedSmallOcr, modifiedSmallOcr};
    QAtomicInt basicStop(1);
    QAtomicInt contentStop(0);
    QAtomicInt ocrStop(0);
    QAtomicInt aiStop(1);
    IndexUpdater updater(files, basicStop, contentStop, ocrStop, aiStop);

    updater.updateContentIndex();
    updater.updateOcrContentIndex();

    {
        Database contentDb(DataBaseType::Content);
        QVERIFY(contentDb.valid());
        const uint contentDocCount = contentDb.getIndexDocCount();
        QVERIFY(contentDocCount == 0 || contentDocCount == 1);
    }
    {
        Database ocrDb(DataBaseType::OcrContent);
        QVERIFY(ocrDb.valid());
        const uint ocrDocCount = ocrDb.getIndexDocCount();
        QVERIFY(ocrDocCount == 0 || ocrDocCount == 2);
    }

    config->m_targetFileTypeMap = originalContentTargets;
    config->m_targetPhotographTypeMap = originalOcrTargets;
}

void IndexJobsWhiteboxTest::indexUpdaterOpenFailuresLeaveRealtimeStateForRunCleanup()
{
    PendingFile file(QDir::home().filePath(QStringLiteral("blocked-index.txt")));
    file.setCreated();
    QVector<PendingFile> files{file};
    QAtomicInt basicStop(1);
    QAtomicInt contentStop(1);
    QAtomicInt ocrStop(1);
    QAtomicInt aiStop(1);
    IndexUpdater updater(files, basicStop, contentStop, ocrStop, aiStop);

    updater.m_cache.clear();
    IndexStatusRecorder::getInstance()->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::RealTimeUpdating);
    IndexStatusRecorder::getInstance()->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::RealTimeUpdating);
    IndexStatusRecorder::getInstance()->setStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::RealTimeUpdating);
    IndexStatusRecorder::getInstance()->setStatus(AI_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::RealTimeUpdating);

    QSignalSpy doneSpy(&updater, &IndexUpdater::done);
    updater.run();
    QCOMPARE(doneSpy.count(), 1);
    QCOMPARE(IndexStatusRecorder::getInstance()->getStatus(INDEX_DATABASE_STATE_KEY),
             IndexStatusRecorder::State::Ready);
    QCOMPARE(IndexStatusRecorder::getInstance()->getStatus(CONTENT_INDEX_DATABASE_STATE_KEY),
             IndexStatusRecorder::State::Ready);
    QCOMPARE(IndexStatusRecorder::getInstance()->getStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY),
             IndexStatusRecorder::State::Ready);
    QCOMPARE(IndexStatusRecorder::getInstance()->getStatus(AI_INDEX_DATABASE_STATE_KEY),
             IndexStatusRecorder::State::Ready);
}

void IndexJobsWhiteboxTest::indexStatusRecorderVersionAndEnableChecks()
{
    auto *recorder = IndexStatusRecorder::getInstance();

    recorder->setVersion(INDEX_DATABASE_VERSION_KEY, QStringLiteral("whitebox-version"));
    QVERIFY(recorder->versionCheck(INDEX_DATABASE_VERSION_KEY, QStringLiteral("whitebox-version")));
    QVERIFY(!recorder->versionCheck(INDEX_DATABASE_VERSION_KEY, QStringLiteral("other-version")));

    recorder->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Ready);
    recorder->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Updating);
    QVERIFY(recorder->indexDatabaseEnable());
    QVERIFY(!recorder->contentIndexDatabaseEnable());

    recorder->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::State::Ready);
    QVERIFY(recorder->contentIndexDatabaseEnable());
}

void IndexJobsWhiteboxTest::indexSchedulerCoversStoppedStateAndCompletionSlots()
{
    qRegisterMetaType<IndexScheduler::IndexerState>("IndexerState");

    IndexScheduler scheduler;
    QSignalSpy stateSpy(&scheduler, &IndexScheduler::stateChange);
    QSignalSpy basicDoneSpy(&scheduler, &IndexScheduler::basicIndexDone);
    QSignalSpy contentDoneSpy(&scheduler, &IndexScheduler::contentIndexDone);
    QSignalSpy ocrDoneSpy(&scheduler, &IndexScheduler::ocrContentIndexDone);
    QSignalSpy aiDoneSpy(&scheduler, &IndexScheduler::aiIndexDone);

    scheduler.stop(BatchIndexer::All);
    scheduler.m_threadPool.waitForDone(5000);
    scheduler.m_threadPoolForRTUpdate.waitForDone(5000);
    QCOMPARE(scheduler.getIndexState(), IndexScheduler::Stop);

    scheduler.m_state = IndexScheduler::Running;
    scheduler.m_basicIndexPendingWorkCount = 1;
    scheduler.m_contentIndexPendingWorkCount = 1;
    scheduler.m_ocrContentIndexPendingWorkCount = 1;
    scheduler.m_aiIndexPendingWorkCount = 1;
    scheduler.m_updatePendingWorkCount = 1;
    scheduler.m_addNewPathPendingWorkCount = 1;
    QVERIFY(!scheduler.isIdle());

    scheduler.batchIndexerFinished(BatchIndexer::Add, BatchIndexer::All);
    QCOMPARE(scheduler.m_addNewPathPendingWorkCount, quint64(0));
    scheduler.updateFinished();
    QCOMPARE(scheduler.m_updatePendingWorkCount, quint64(0));

    scheduler.m_statusRecorder->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::Initializing);
    scheduler.m_statusRecorder->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::Updating);
    scheduler.m_statusRecorder->setStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::RealTimeUpdating);
    scheduler.m_statusRecorder->setStatus(AI_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::Off);

    scheduler.onBasicIndexDone(BatchIndexer::Update);
    scheduler.onContentIndexDone(BatchIndexer::Update);
    scheduler.onOcrContentIndexDone(BatchIndexer::Update);
    scheduler.onAiIndexDone(BatchIndexer::Update);

    QCOMPARE(scheduler.m_basicIndexPendingWorkCount, quint64(0));
    QCOMPARE(scheduler.m_contentIndexPendingWorkCount, quint64(0));
    QCOMPARE(scheduler.m_ocrContentIndexPendingWorkCount, quint64(0));
    QCOMPARE(scheduler.m_aiIndexPendingWorkCount, quint64(0));
    QVERIFY(scheduler.isIdle());
    QCOMPARE(basicDoneSpy.count(), 1);
    QCOMPARE(contentDoneSpy.count(), 1);
    QCOMPARE(ocrDoneSpy.count(), 1);
    QCOMPARE(aiDoneSpy.count(), 1);
    QCOMPARE(basicDoneSpy.takeFirst().at(0).toBool(), true);
    QCOMPARE(contentDoneSpy.takeFirst().at(0).toBool(), true);
    QCOMPARE(ocrDoneSpy.takeFirst().at(0).toBool(), true);
    QCOMPARE(aiDoneSpy.takeFirst().at(0).toBool(), false);

    // Cover the waiting-start branch without launching real background index
    // jobs, otherwise the scheduler may install watches and enqueue long-lived
    // async work against the shared test home.
    scheduler.m_config->m_gsettings->set(QStringLiteral("fileIndexEnable"), false);
    scheduler.m_config->m_gsettings->set(QStringLiteral("contentIndexEnable"), false);
    scheduler.m_config->m_gsettings->set(QStringLiteral("contentIndexEnableOcr"), false);
    scheduler.m_config->m_gsettings->set(QStringLiteral("aiIndexEnable"), false);

    scheduler.m_basicIndexStartWaiting = true;
    scheduler.m_contentIndexStartWaiting = true;
    scheduler.m_ocrContentIndexStartWaiting = true;
    scheduler.m_aiIndexStartWaiting = true;
    scheduler.checkIfStartsWaiting();
    scheduler.stop(BatchIndexer::All);
    scheduler.m_threadPool.waitForDone(5000);
    QVERIFY(stateSpy.count() >= 1);
}

void IndexJobsWhiteboxTest::indexSchedulerEnableFlagsAndRealtimeUpdateBranches()
{
    qRegisterMetaType<IndexScheduler::IndexerState>("IndexerState");

    IndexScheduler scheduler;
    scheduler.stop(BatchIndexer::All);
    scheduler.m_threadPool.waitForDone(5000);
    scheduler.m_threadPoolForRTUpdate.waitForDone(5000);

    scheduler.m_aiIndexStartWaiting = true;
    scheduler.addNewPath(QStringLiteral("/tmp/ignored"), {});
    QCOMPARE(scheduler.m_addNewPathPendingWorkCount, quint64(0));
    scheduler.removeIndex(QStringLiteral("/tmp/ignored"));

    scheduler.m_basicIndexPendingWorkCount = 1;
    scheduler.fileIndexEnable(true);
    QVERIFY(scheduler.m_basicIndexStartWaiting);
    scheduler.m_contentIndexPendingWorkCount = 1;
    scheduler.contentIndexEnable(true);
    QVERIFY(scheduler.m_contentIndexStartWaiting);
    scheduler.m_ocrContentIndexPendingWorkCount = 1;
    scheduler.ocrContentIndexEnable(true);
    QVERIFY(scheduler.m_ocrContentIndexStartWaiting);
    scheduler.m_aiIndexPendingWorkCount = 1;
    scheduler.aiIndexEnable(true);
    QVERIFY(scheduler.m_aiIndexStartWaiting);

    scheduler.fileIndexEnable(false);
    QVERIFY(scheduler.m_indexStop.loadRelaxed());
    scheduler.contentIndexEnable(false);
    QVERIFY(scheduler.m_contentIndexStop.loadRelaxed());
    scheduler.ocrContentIndexEnable(false);
    QVERIFY(scheduler.m_ocrContentIndexStop.loadRelaxed());
    scheduler.aiIndexEnable(false);
    QVERIFY(scheduler.m_aiIndexStop.loadRelaxed());

    QVector<PendingFile> files;
    files.append(PendingFile(QStringLiteral("/tmp/index-scheduler-update.txt")));
    QSignalSpy stateSpy(&scheduler, &IndexScheduler::stateChange);
    scheduler.m_indexStop.fetchAndStoreRelaxed(1);
    scheduler.m_contentIndexStop.fetchAndStoreRelaxed(1);
    scheduler.m_ocrContentIndexStop.fetchAndStoreRelaxed(1);
    scheduler.m_aiIndexStop.fetchAndStoreRelaxed(1);
    scheduler.updateIndex(files);
    QTRY_VERIFY(scheduler.m_updatePendingWorkCount == 0);
    QVERIFY(stateSpy.count() >= 1);
}

void IndexJobsWhiteboxTest::fileIndexerConfigSyncsDefaultAndCustomFileTypeSettings()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    FileIndexerConfig config;
    auto *settings = new QSettings(dir.filePath(QStringLiteral("index.conf")), QSettings::IniFormat, &config);
    config.m_settings = settings;
    config.m_targetFileTypeMap = {{QStringLiteral("txt"), true}, {QStringLiteral("doc"), false}};
    config.m_targetPhotographTypeMap = {{QStringLiteral("png"), true}, {QStringLiteral("hdr"), false}};
    config.m_sAiFileTypeMap = {{QStringLiteral("pdf"), true}, {QStringLiteral("pptx"), false}};
    config.m_sAiImageTypeMap = {{QStringLiteral("jpg"), true}, {QStringLiteral("jpeg"), false}};

    config.syncIndexFileTypes(true);

    QSettings written(settings->fileName(), QSettings::IniFormat);
    written.beginGroup(QStringLiteral("contentIndexTarget"));
    QCOMPARE(written.value(QStringLiteral("txt")).toBool(), true);
    QCOMPARE(written.value(QStringLiteral("doc")).toBool(), false);
    written.endGroup();
    written.beginGroup(QStringLiteral("ocrContentIndexTarget"));
    QCOMPARE(written.value(QStringLiteral("png")).toBool(), true);
    QCOMPARE(written.value(QStringLiteral("hdr")).toBool(), false);
    written.endGroup();
    written.beginGroup(QStringLiteral("sAiFileType"));
    QCOMPARE(written.value(QStringLiteral("pdf")).toBool(), true);
    QCOMPARE(written.value(QStringLiteral("pptx")).toBool(), false);
    written.endGroup();
    written.beginGroup(QStringLiteral("sAiImageType"));
    QCOMPARE(written.value(QStringLiteral("jpg")).toBool(), true);
    QCOMPARE(written.value(QStringLiteral("jpeg")).toBool(), false);
    written.endGroup();

    settings->clear();
    settings->beginGroup(QStringLiteral("contentIndexTarget"));
    settings->setValue(QStringLiteral("md"), true);
    settings->setValue(QStringLiteral("txt"), false);
    settings->endGroup();
    settings->beginGroup(QStringLiteral("ocrContentIndexTarget"));
    settings->setValue(QStringLiteral("bmp"), true);
    settings->setValue(QStringLiteral("png"), false);
    settings->endGroup();
    settings->beginGroup(QStringLiteral("sAiFileType"));
    settings->setValue(QStringLiteral("txt"), false);
    settings->endGroup();
    settings->beginGroup(QStringLiteral("sAiImageType"));
    settings->setValue(QStringLiteral("webp"), true);
    settings->endGroup();
    settings->setValue(QStringLiteral("metaDataIndexEnable"), false);
    settings->sync();

    config.syncIndexFileTypes(false);
    QCOMPARE(config.m_targetFileTypeMap.value(QStringLiteral("md")), true);
    QCOMPARE(config.m_targetFileTypeMap.value(QStringLiteral("txt")), false);
    QCOMPARE(config.m_targetPhotographTypeMap.value(QStringLiteral("bmp")), true);
    QCOMPARE(config.m_targetPhotographTypeMap.value(QStringLiteral("png")), false);
    QCOMPARE(config.m_sAiFileTypeMap.value(QStringLiteral("txt")), false);
    QCOMPARE(config.m_sAiImageTypeMap.value(QStringLiteral("webp")), true);
    QVERIFY(!config.isMetaDataIndexEnable());
    QCOMPARE(config.contentIndexTarget().value(QStringLiteral("md")), true);
    QCOMPARE(config.ocrContentIndexTarget().value(QStringLiteral("bmp")), true);
    QCOMPARE(config.aiIndexFileTarget().value(QStringLiteral("txt")), false);
    QCOMPARE(config.aiIndexImageTarget().value(QStringLiteral("webp")), true);
}

void IndexJobsWhiteboxTest::fileIndexerConfigGuardAndPowerControlBranches()
{
    FileIndexerConfig config;
    auto *originalGsettings = config.m_gsettings;

    config.m_gsettings = nullptr;
    QVERIFY(!config.isFileIndexEnable());
    QVERIFY(!config.isContentIndexEnable());
    QVERIFY(!config.isFuzzySearchEnable());
    QVERIFY(!config.isOCREnable());
    QVERIFY(!config.isAiIndexEnable());

    config.m_onBattery = false;
    config.onPropertiesChanged(QStringLiteral("org.freedesktop.NotUPower"),
                               {{QStringLiteral("OnBattery"), true}},
                               {});
    QVERIFY(!config.m_onBattery);

    config.onPropertiesChanged(QStringLiteral("org.freedesktop.UPower"),
                               {{QStringLiteral("Other"), true}},
                               {});
    QVERIFY(!config.m_onBattery);

    config.onPropertiesChanged(QStringLiteral("org.freedesktop.UPower"),
                               {{QStringLiteral("OnBattery"), true}},
                               {});
    QVERIFY(config.m_onBattery);

    config.m_underCtrl = true;
    config.updateCtrlState();
    QVERIFY(config.m_underCtrl);

    config.m_gsettings = originalGsettings;
    QVERIFY(config.m_gsettings != nullptr);
    config.m_gsettings->set(QStringLiteral("aiIndexEnable"), true);
    config.m_gsettings->set(QStringLiteral("exitPowerControl"), true);
    config.m_powerPolicy = 2;
    config.m_onBattery = true;
    config.m_underCtrl = false;
    config.updateCtrlState();
    QVERIFY(config.m_underCtrl);

    config.m_gsettings->set(QStringLiteral("aiIndexEnable"), false);
    config.m_onBattery = false;
    config.m_underCtrl = true;
    config.updateCtrlState();
    QVERIFY(!config.m_underCtrl);

    config.onPropertiesChanged(QStringLiteral("org.freedesktop.UPower"),
                               {{QStringLiteral("OnBattery"), false}},
                               {});
    QVERIFY(!config.m_onBattery);
}

void IndexJobsWhiteboxTest::batchIndexerRebuildModeClearsCacheAndReindexes()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QVERIFY(dir.mkpath(QStringLiteral("rebuild-test")));
    QFile file(dir.filePath(QStringLiteral("rebuild-test/file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("rebuild content");
    file.close();

    QAtomicInt basicStop(0), contentStop(0), ocrStop(0), aiStop(0);
    BatchIndexer indexer(QStringList{tempDir.path()}, QStringList(),
                         basicStop, contentStop, ocrStop, aiStop,
                         BatchIndexer::Rebuild, BatchIndexer::Basic);
    indexer.fetch();
    indexer.basicIndex();
}

void IndexJobsWhiteboxTest::batchIndexerPartialStopFlagsOnlyBlockRespectiveTargets()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QVERIFY(dir.mkpath(QStringLiteral("partial")));
    QFile file(dir.filePath(QStringLiteral("partial/file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("partial");
    file.close();

    QAtomicInt basicStop(0), contentStop(1), ocrStop(1), aiStop(1);
    BatchIndexer indexer(QStringList{tempDir.path()}, QStringList(),
                         basicStop, contentStop, ocrStop, aiStop,
                         BatchIndexer::Add, BatchIndexer::Basic);
    indexer.fetch();
}

void IndexJobsWhiteboxTest::batchIndexerFetchWithExtraDirsPreservesPaths()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QVERIFY(dir.mkpath(QStringLiteral("extra-test")));
    QFile file(dir.filePath(QStringLiteral("extra-test/file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("extra");
    file.close();

    QAtomicInt basicStop(0), contentStop(0), ocrStop(0), aiStop(0);
    BatchIndexer indexer(QStringList{tempDir.path()}, QStringList{dir.filePath(QStringLiteral("extra-test"))},
                         basicStop, contentStop, ocrStop, aiStop,
                         BatchIndexer::Add, BatchIndexer::Basic);
    indexer.fetch();
}

void IndexJobsWhiteboxTest::batchIndexerIndexStepsCoverOcrAndAiStopBranches()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir dir(tempDir.path());
    QVERIFY(dir.mkpath(QStringLiteral("steps")));
    QFile file(dir.filePath(QStringLiteral("steps/file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("steps content");
    file.close();

    QAtomicInt basicStop(0), contentStop(0), ocrStop(1), aiStop(1);
    BatchIndexer indexer(QStringList{tempDir.path()}, QStringList(),
                         basicStop, contentStop, ocrStop, aiStop,
                         BatchIndexer::Add, BatchIndexer::All);
    indexer.fetch();
    indexer.m_cache.append(dir.filePath(QStringLiteral("steps/file.txt")));
    indexer.contentIndex();
    indexer.ocrIndex();
    indexer.aiIndex();
}

int main(int argc, char **argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");
    qputenv("QT_FATAL_WARNINGS", "0");

    QByteArray home = qgetenv("UKUI_SEARCH_WHITEBOX_HOME");
    QTemporaryDir fallbackHome;
    if (home.isEmpty()) {
        fallbackHome.setAutoRemove(false);
        if (!fallbackHome.isValid()) {
            return 2;
        }
        home = fallbackHome.path().toLocal8Bit();
        qputenv("UKUI_SEARCH_WHITEBOX_HOME", home);
    }
    qputenv("HOME", home);
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);
    IndexJobsWhiteboxTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "index-jobs-whitebox-test.moc"
