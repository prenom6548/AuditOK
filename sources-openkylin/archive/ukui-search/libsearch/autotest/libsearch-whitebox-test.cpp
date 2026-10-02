#include <xapian.h>

#include <QDBusConnection>
#include <QFile>
#include <QJsonDocument>
#include <QGuiApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTextBoundaryFinder>
#include <QTest>
#include <cstdlib>
#include <memory>
#include <unistd.h>

#define private public
#define protected public
#include "appdata/app-info-table-private.h"
#include "common.h"
#include "date-format-helper.h"
#include "index/index-monitor.h"
#include "index/index-status-recorder.h"
#include "index/file-content-indexer.h"
#include "searchinterface/searchtasks/file-content-search-task.h"
#include "searchinterface/searchtasks/file-search-task.h"
#include "searchinterface/ukui-search-task.h"
#include "index/pending-file-queue.h"
#include "pluginmanage/plugin-info.h"
#include "pluginmanage/search-task-plugin-manager.h"
#include "security-control-config.h"
#undef protected
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

void ensureSearchTaskConfigDirs()
{
    QDir().mkpath(QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search"));
}

void addSearchTaskTerms(Xapian::Document &doc, const QString &keyword)
{
    const QString lowerKeyword = keyword.toLower();
    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Grapheme, lowerKeyword);
    int start = 0;
    int position = 1;
    for (; boundaryFinder.position() != -1; boundaryFinder.toNextBoundary()) {
        const int end = boundaryFinder.position();
        if (boundaryFinder.boundaryReasons() & QTextBoundaryFinder::EndOfItem) {
            doc.add_posting(QUrl::toPercentEncoding(lowerKeyword.mid(start, end - start)).toStdString(), position++);
        }
        start = end;
    }
}

void writeFileSearchTaskDatabase(const QList<QPair<QString, QString>> &entries, const QString &keyword)
{
    ensureSearchTaskConfigDirs();
    Xapian::WritableDatabase database(INDEX_PATH.toStdString(), Xapian::DB_CREATE_OR_OVERWRITE);
    int sortKey = 0;
    for (const auto &entry : entries) {
        Xapian::Document document;
        document.set_data(entry.first.toStdString());
        document.add_value(1, entry.second.toStdString());
        document.add_value(2, QDateTime::currentDateTimeUtc().addSecs(++sortKey).toString(QStringLiteral("yyyyMMddHHmmsszzz")).toStdString());
        addSearchTaskTerms(document, keyword);
        database.add_document(document);
    }
    database.commit();
}

}

class ManagedSearchTaskStub : public SearchTaskPluginIface
{
    Q_OBJECT

public:
    explicit ManagedSearchTaskStub(bool searching = false)
        : m_searching(searching)
    {
    }

    PluginType pluginType() override { return PluginType::SearchTaskPlugin; }
    const QString name() override { return QStringLiteral("Managed Search Task Stub"); }
    const QString description() override { return QStringLiteral("Managed Search Task Stub"); }
    const QIcon icon() override { return QIcon(); }
    void setEnable(bool enable) override { m_enable = enable; }
    bool isEnable() override { return m_enable; }
    void setController(const SearchController &searchController) override { m_controller = searchController; }
    QString getCustomSearchType() override { return QStringLiteral("managed-stub"); }
    SearchProperty::SearchType getSearchType() override { return SearchProperty::SearchType::Custom; }
    void startSearch() override { ++m_startCalls; m_searching = true; }
    void stop() override { m_searching = false; }
    bool isSearching() override { return m_searching; }

    bool m_enable = true;
    bool m_searching = false;
    int m_startCalls = 0;
    SearchController m_controller;
};

class LibsearchWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void pluginInfoReturnsConstructorValues();
    void dateFormatUpdatesAndHidesDefaultFormat();
    void securityControlConfigParsesAndClearsJson();
    void pendingFileQueueMergesPrunesAndFlushesCache();
    void appInfoTablePrivateCalculatesDesktopIdentity();
    void fileSearchTaskCoversMetadataQueryAndFilters();
    void fileSearchTaskDirectSearchEnqueuesMatchedFiles();
    void fileSearchTaskDirectSearchCoversModeVariants();
    void fileSearchTaskDirectSearchCoversSkipAndNoMatchBranches();
    void fileSearchTaskDirectSearchCoversLabelsMissingPathsAndIconProperty();
    void fileSearchTaskFilterCoversDirectoryDepthAndLabels();
    void fileSearchTaskSearchWithIndexCoversErrorAndResults();
    void fileSearchTaskStartSearchCopiesControllerAndFinishes();
    void fileSearchTaskRunCoversDuplicateDirsAndBlockedSkipHelpers();
    void fileContentSearchTaskCoversMetadataAndFilter();
    void fileContentSearchTaskRunCoversDisabledAndErrorBranches();
    void fileContentSearchTaskIndexedSearchAndAsyncStart();
    void ukuiSearchTaskCoversControllerFacadeAndErrorPath();
    void fileSearchTaskDirectSearchNonRecursiveAndEmptyDirs();
    void fileSearchTaskDirectSearchWithManyProperties();
    void fileSearchTaskFilterWithAllFileTypes();
    void fileSearchTaskStartSearchWithNoDirs();
    void searchTaskPluginManagerCoversManagedPluginBranches();
    void indexMonitorCoversReplicaDefaultsAndSingletonLifecycle();

private:
    FakePrivateDirWatcherService *m_dirWatcherService = nullptr;
    std::unique_ptr<ScopedDbusRegistration> m_dirWatcherRegistration;
};

void LibsearchWhiteboxTest::initTestCase()
{
    ensureSearchTaskConfigDirs();
    m_dirWatcherService = new FakePrivateDirWatcherService;
    m_dirWatcherRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.ukui.search.fileindex.service"),
                                                              QStringLiteral("/org/ukui/search/privateDirWatcher"),
                                                              m_dirWatcherService));
}

void LibsearchWhiteboxTest::cleanupTestCase()
{
    m_dirWatcherRegistration.reset();
    delete m_dirWatcherService;
    m_dirWatcherService = nullptr;
}

void LibsearchWhiteboxTest::pluginInfoReturnsConstructorValues()
{
    PluginInfo internal(QStringLiteral("files"), true, 10, false, false);
    QCOMPARE(internal.name(), QStringLiteral("files"));
    QCOMPARE(internal.order(), 10);
    QVERIFY(internal.isEnable());
    QVERIFY(!internal.isFixed());
    QVERIFY(!internal.isExternal());
    QVERIFY(internal.path().isEmpty());

    PluginInfo external(QStringLiteral("custom"), false, -1, true, true, QStringLiteral("/tmp/plugin.so"));
    QCOMPARE(external.name(), QStringLiteral("custom"));
    QCOMPARE(external.order(), -1);
    QVERIFY(!external.isEnable());
    QVERIFY(external.isFixed());
    QVERIFY(external.isExternal());
    QCOMPARE(external.path(), QStringLiteral("/tmp/plugin.so"));
}

void LibsearchWhiteboxTest::dateFormatUpdatesAndHidesDefaultFormat()
{
    DateFormat &format = DateFormat::getInstance();

    format.updateDateFormat(QStringLiteral("yyyy-MM-dd"));
    QCOMPARE(format.format(), QStringLiteral("yyyy-MM-dd hh:mm:ss"));

    format.updateDateFormat(QStringLiteral("yyyy/MM/dd"));
    QVERIFY(format.format().isEmpty());

    format.updateDateFormat(QStringLiteral("MM-dd-yyyy"));
    QCOMPARE(format.format(), QStringLiteral("MM-dd-yyyy hh:mm:ss"));
}

void LibsearchWhiteboxTest::securityControlConfigParsesAndClearsJson()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString validPath = dir.filePath(QStringLiteral("security.json"));
    QFile validFile(validPath);
    QVERIFY(validFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    validFile.write(R"({"ukui-search":{"appBlackList":["blocked.desktop"],"appWhiteList":["allowed.desktop"]}})");
    validFile.close();

    SecurityControlConfig &config = SecurityControlConfig::instance();
    config.parseData(QStringLiteral("/not/existing/security.json"));
    config.parseData(validPath);
    QVERIFY(config.isUnderSecurityControl());
    QVERIFY(config.isBlackListEnable());
    QVERIFY(config.isWhiteListEnable());
    QCOMPARE(config.appBlackList(), QStringList{QStringLiteral("blocked.desktop")});
    QCOMPARE(config.appWhiteList(), QStringList{QStringLiteral("allowed.desktop")});

    config.onConfigFileChanged(QStringLiteral("not-current-user"), QStringList{QStringLiteral("ukui-search")});
    QVERIFY(config.isUnderSecurityControl());

    config.m_configPath = validPath;
    config.onConfigFileChanged(QString::fromLocal8Bit(qgetenv("USER")), QStringList{QStringLiteral("other-app")});
    QVERIFY(config.isUnderSecurityControl());

    config.onSecurityControlExited();
    QVERIFY(!config.isUnderSecurityControl());
    QVERIFY(!config.isBlackListEnable());
    QVERIFY(!config.isWhiteListEnable());
    QVERIFY(config.appBlackList().isEmpty());
    QVERIFY(config.appWhiteList().isEmpty());

    const QString invalidPath = dir.filePath(QStringLiteral("invalid.json"));
    QFile invalidFile(invalidPath);
    QVERIFY(invalidFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    invalidFile.write("{");
    invalidFile.close();
    config.parseData(invalidPath);
    QVERIFY(!config.isUnderSecurityControl());

    const QString emptyAppPath = dir.filePath(QStringLiteral("empty-app.json"));
    QFile emptyAppFile(emptyAppPath);
    QVERIFY(emptyAppFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    emptyAppFile.write(R"({"other-app":{"appBlackList":["ignored.desktop"]}})");
    emptyAppFile.close();
    config.parseData(emptyAppPath);
    QVERIFY(!config.isUnderSecurityControl());

    const QString emptyListsPath = dir.filePath(QStringLiteral("empty-lists.json"));
    QFile emptyListsFile(emptyListsPath);
    QVERIFY(emptyListsFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    emptyListsFile.write(R"({"ukui-search":{"appBlackList":[],"appWhiteList":[]}})");
    emptyListsFile.close();
    config.parseData(emptyListsPath);
    QVERIFY(config.isUnderSecurityControl());
    QVERIFY(!config.isBlackListEnable());
    QVERIFY(!config.isWhiteListEnable());
    QVERIFY(config.appBlackList().isEmpty());
    QVERIFY(config.appWhiteList().isEmpty());

    const QString directoryPath = dir.filePath(QStringLiteral("not-readable-dir"));
    QVERIFY(QDir().mkpath(directoryPath));
    config.onSecurityControlExited();
    config.parseData(directoryPath);
    QVERIFY(!config.isUnderSecurityControl());
}

void LibsearchWhiteboxTest::pendingFileQueueMergesPrunesAndFlushesCache()
{
    PendingFileQueue *queue = PendingFileQueue::getInstance();
    QVERIFY(queue != nullptr);
    int updateCount = 0;
    QVector<PendingFile> emittedFiles;
    connect(queue, &PendingFileQueue::filesUpdate, this, [&updateCount, &emittedFiles](const QVector<PendingFile> &files) {
        ++updateCount;
        emittedFiles = files;
    });

    PendingFile created(QStringLiteral("/tmp/libsearch-whitebox/a.txt"));
    created.setCreated();
    queue->enqueue(created);
    QCOMPARE(queue->m_cache.size(), 1);
    QVERIFY(queue->m_cache.first().isCreated());

    PendingFile modified(QStringLiteral("/tmp/libsearch-whitebox/a.txt"));
    modified.setModified();
    queue->enqueue(modified);
    QCOMPARE(queue->m_cache.size(), 1);
    QVERIFY(queue->m_cache.first().isCreated());
    QVERIFY(queue->m_cache.first().isModified());

    PendingFile child(QStringLiteral("/tmp/libsearch-whitebox/dir/child.txt"));
    child.setCreated();
    queue->enqueue(child);
    QVERIFY(queue->m_cache.size() >= 2);

    PendingFile deletedDir(QStringLiteral("/tmp/libsearch-whitebox/dir"));
    deletedDir.setIsDir();
    deletedDir.setDeleted();
    queue->enqueue(deletedDir);
    QVERIFY(std::none_of(queue->m_cache.cbegin(), queue->m_cache.cend(), [](const PendingFile &file) {
        return file.path() == QStringLiteral("/tmp/libsearch-whitebox/dir/child.txt");
    }));
    QVERIFY(std::any_of(queue->m_cache.cbegin(), queue->m_cache.cend(), [](const PendingFile &file) {
        return file.path() == QStringLiteral("/tmp/libsearch-whitebox/dir") && file.shouldRemoveIndex();
    }));

    queue->processCache();
    QVERIFY(queue->m_cache.isEmpty());
    QCOMPARE(updateCount, 1);
    QCOMPARE(emittedFiles.size(), 2);

    queue->forceFinish();
    delete queue;
}

void LibsearchWhiteboxTest::appInfoTablePrivateCalculatesDesktopIdentity()
{
    qputenv("XDG_DATA_DIRS", "/usr/share:/opt/share");

    QCOMPARE(AppInfoTablePrivate::calcDesktopFileId(QStringLiteral("/usr/share/applications/org/demo/App.desktop")),
             QStringLiteral("org-demo-App.desktop"));
    QCOMPARE(AppInfoTablePrivate::calcDesktopFileId(QStringLiteral("/opt/share/applications/simple.desktop")),
             QStringLiteral("simple.desktop"));
    QCOMPARE(AppInfoTablePrivate::calcDesktopFileId(QStringLiteral("/outside/path/fallback.desktop")),
             QStringLiteral("fallback.desktop"));

    QCOMPARE(AppInfoTablePrivate::desktopPathPriority(QStringLiteral("/usr/share/applications/a.desktop")), 0);
    QCOMPARE(AppInfoTablePrivate::desktopPathPriority(QStringLiteral("/opt/share/applications/a.desktop")), 1);
    QCOMPARE(AppInfoTablePrivate::desktopPathPriority(QStringLiteral("/outside/path/a.desktop")), -1);
}

void LibsearchWhiteboxTest::fileSearchTaskCoversMetadataQueryAndFilters()
{
    FileSearchTask task;
    QCOMPARE(task.pluginType(), PluginInterface::PluginType::SearchTaskPlugin);
    QCOMPARE(task.name(), QStringLiteral("File"));
    QCOMPARE(task.description(), QStringLiteral("File search."));
    QCOMPARE(task.getCustomSearchType(), QStringLiteral("File"));
    QCOMPARE(task.getSearchType(), SearchProperty::SearchType::File);
    QVERIFY(task.isEnable());
    QVERIFY(!task.isSearching());
    task.setEnable(false);
    QVERIFY(task.isEnable());

    SearchController controller;
    controller.initDataQueue();
    controller.setOnlySearchDir(true);
    controller.setOnlySearchFile(true);
    FileSearchWorker worker(&task, &controller);
    QVERIFY(worker.creatQueryForFileSearch().empty());

    controller.setOnlySearchFile(false);
    controller.setOnlySearchDir(false);
    controller.setMatchAllIfNoKeyword(true);
    QVERIFY(!worker.creatQueryForFileSearch().empty());

    controller.addKeyword(QStringLiteral("Demo"));
    controller.setMatchAllIfNoKeyword(false);
    controller.setOnlySearchDir(true);
    QVERIFY(!worker.creatQueryForFileSearch().empty());

    controller.setOnlySearchDir(false);
    controller.setOnlySearchFile(true);
    QVERIFY(!worker.creatQueryForFileSearch().empty());

    controller.setOnlySearchFile(false);
    QVERIFY(!worker.creatQueryForFileSearch().empty());

    Xapian::Document document;
    document.set_data("/tmp/libsearch-whitebox/file.txt");
    FileSearchFilter nullFilter(nullptr);
    QVERIFY(nullFilter(document));
    FileSearchFilter filter(&worker);
    QVERIFY(filter(document));
    QVERIFY(!FileSearchFilter::checkFileLabel(QStringLiteral("/not/existing/file"), QStringList{QStringLiteral("label")}));
}

void LibsearchWhiteboxTest::fileSearchTaskDirectSearchEnqueuesMatchedFiles()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("sub")));

    QFile matched(dir.filePath(QStringLiteral("demo-file.txt")));
    QVERIFY(matched.open(QIODevice::WriteOnly | QIODevice::Truncate));
    matched.write("demo");
    matched.close();

    QFile nested(dir.filePath(QStringLiteral("sub/nested-demo.txt")));
    QVERIFY(nested.open(QIODevice::WriteOnly | QIODevice::Truncate));
    nested.write("nested");
    nested.close();

    FileSearchTask task;
    SearchController controller;
    DataQueue<ResultItem> *queue = controller.initDataQueue();
    controller.refreshSearchId();
    controller.addKeyword(QStringLiteral("demo"));
    controller.addSearchDir(dir.path() + QStringLiteral("/"));
    controller.setRecurse(true);
    controller.setMaxResultNum(5);
    controller.setInformNum(1);
    controller.setResultProperties(SearchProperty::SearchType::File,
                                   SearchResultProperties{SearchProperty::SearchResultProperty::FilePath,
                                                          SearchProperty::SearchResultProperty::FileIconName,
                                                          SearchProperty::SearchResultProperty::FileName,
                                                          SearchProperty::SearchResultProperty::ModifiedTime});

    int informCount = 0;
    connect(&task, &FileSearchTask::reachInformNum, this, [&informCount]() {
        ++informCount;
    });

    FileSearchWorker worker(&task, &controller);
    worker.m_validDirectories << dir.path();
    worker.m_blackList << QStringList{};

    QVERIFY(worker.directSearch());
    QVERIFY(informCount >= 1);
    QVERIFY(queue->length() >= 2);

    ResultItem first = queue->dequeue();
    QVERIFY(first.getItemKey().contains(dir.path()));
    QVERIFY(!first.getValue(SearchProperty::SearchResultProperty::FilePath).toString().isEmpty());
    QVERIFY(!first.getValue(SearchProperty::SearchResultProperty::FileName).toString().isEmpty());

    controller.setOnlySearchDir(true);
    controller.setOnlySearchFile(true);
    QVERIFY(worker.creatQueryForFileSearch().empty());

    QVERIFY(!worker.isOrUnderExtraDir(dir.path(), true));
    QVERIFY(!worker.underBlocked(dir.path()));
}

void LibsearchWhiteboxTest::fileSearchTaskDirectSearchCoversModeVariants()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral(".hidden-dir")));
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("visible-dir")));

    QFile visible(dir.filePath(QStringLiteral("visible-target.txt")));
    QVERIFY(visible.open(QIODevice::WriteOnly | QIODevice::Truncate));
    visible.write("visible");
    visible.close();

    QFile hidden(dir.filePath(QStringLiteral(".hidden-target.txt")));
    QVERIFY(hidden.open(QIODevice::WriteOnly | QIODevice::Truncate));
    hidden.write("hidden");
    hidden.close();

    QFile nested(dir.filePath(QStringLiteral("visible-dir/nested-target.txt")));
    QVERIFY(nested.open(QIODevice::WriteOnly | QIODevice::Truncate));
    nested.write("nested");
    nested.close();

    FileSearchTask task;

    SearchController fileOnlyController;
    DataQueue<ResultItem> *fileOnlyQueue = fileOnlyController.initDataQueue();
    fileOnlyController.refreshSearchId();
    fileOnlyController.addSearchDir(dir.path());
    fileOnlyController.addKeyword(QStringLiteral("target"));
    fileOnlyController.setOnlySearchFile(true);
    fileOnlyController.setRecurse(false);
    fileOnlyController.setMaxResultNum(1);
    fileOnlyController.setResultProperties(SearchProperty::SearchType::File,
                                           SearchResultProperties{SearchProperty::SearchResultProperty::FileName});

    FileSearchWorker fileOnlyWorker(&task, &fileOnlyController);
    fileOnlyWorker.m_validDirectories << dir.path();
    fileOnlyWorker.m_blackList << QStringList{};
    QVERIFY(fileOnlyWorker.directSearch());
    QCOMPARE(fileOnlyQueue->length(), 1);
    QVERIFY(fileOnlyQueue->dequeue().getValue(SearchProperty::SearchResultProperty::FileName).toString().contains(QStringLiteral("target")));

    SearchController dirOnlyController;
    DataQueue<ResultItem> *dirOnlyQueue = dirOnlyController.initDataQueue();
    dirOnlyController.refreshSearchId();
    dirOnlyController.addSearchDir(dir.path());
    dirOnlyController.addKeyword(QStringLiteral("visible"));
    dirOnlyController.setOnlySearchDir(true);
    dirOnlyController.setRecurse(false);
    dirOnlyController.setMaxResultNum(10);
    dirOnlyController.setResultProperties(SearchProperty::SearchType::File,
                                          SearchResultProperties{SearchProperty::SearchResultProperty::FilePath});

    FileSearchWorker dirOnlyWorker(&task, &dirOnlyController);
    dirOnlyWorker.m_validDirectories << dir.path();
    dirOnlyWorker.m_blackList << QStringList{};
    QVERIFY(dirOnlyWorker.directSearch());
    QCOMPARE(dirOnlyQueue->length(), 1);
    QVERIFY(QFileInfo(dirOnlyQueue->dequeue().getItemKey()).isDir());

    SearchController hiddenController;
    DataQueue<ResultItem> *hiddenQueue = hiddenController.initDataQueue();
    hiddenController.refreshSearchId();
    hiddenController.addSearchDir(dir.path());
    hiddenController.addKeyword(QStringLiteral("hidden-target"));
    hiddenController.setSearchHiddenFiles(true);
    hiddenController.setMaxResultNum(10);
    hiddenController.setResultProperties(SearchProperty::SearchType::File,
                                         SearchResultProperties{SearchProperty::SearchResultProperty::FilePath});

    FileSearchWorker hiddenWorker(&task, &hiddenController);
    hiddenWorker.m_validDirectories << dir.path();
    hiddenWorker.m_blackList << QStringList{};
    QVERIFY(hiddenWorker.directSearch());
    QCOMPARE(hiddenQueue->length(), 1);
    QVERIFY(hiddenQueue->dequeue().getItemKey().contains(QStringLiteral(".hidden-target.txt")));

    SearchController stoppedController;
    stoppedController.initDataQueue();
    const size_t oldSearchId = stoppedController.refreshSearchId();
    stoppedController.addSearchDir(dir.path());
    stoppedController.setMatchAllIfNoKeyword(true);
    stoppedController.stop();
    FileSearchWorker stoppedWorker(&task, &stoppedController);
    stoppedWorker.m_currentSearchId = oldSearchId;
    stoppedWorker.m_validDirectories << dir.path();
    stoppedWorker.m_blackList << QStringList{};
    QVERIFY(!stoppedWorker.directSearch());
}

void LibsearchWhiteboxTest::fileSearchTaskDirectSearchCoversSkipAndNoMatchBranches()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir root(dir.path());
    QVERIFY(root.mkpath(QStringLiteral("blocked")));
    QVERIFY(root.mkpath(QStringLiteral("real-dir")));

    QFile blockedFile(root.filePath(QStringLiteral("blocked/blocked-target.txt")));
    QVERIFY(blockedFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    blockedFile.write("blocked");
    blockedFile.close();

    QFile plainFile(root.filePath(QStringLiteral("plain.txt")));
    QVERIFY(plainFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    plainFile.write("plain");
    plainFile.close();

    QFile realFile(root.filePath(QStringLiteral("real-dir/real-target.txt")));
    QVERIFY(realFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    realFile.write("real");
    realFile.close();

    QFile::link(root.filePath(QStringLiteral("real-dir")), root.filePath(QStringLiteral("linked-dir")));

    FileSearchTask task;

    SearchController blockedController;
    DataQueue<ResultItem> *blockedQueue = blockedController.initDataQueue();
    blockedController.refreshSearchId();
    blockedController.addSearchDir(dir.path());
    blockedController.addKeyword(QStringLiteral("blocked-target"));
    blockedController.setRecurse(true);
    blockedController.setMaxResultNum(10);
    blockedController.setResultProperties(SearchProperty::SearchType::File,
                                          SearchResultProperties{SearchProperty::SearchResultProperty::FilePath});
    FileSearchWorker blockedWorker(&task, &blockedController);
    blockedWorker.m_validDirectories << dir.path();
    blockedWorker.m_blackList << dir.path();
    QVERIFY(blockedWorker.directSearch());
    QVERIFY(blockedQueue->isEmpty());

    SearchController noMatchController;
    DataQueue<ResultItem> *noMatchQueue = noMatchController.initDataQueue();
    noMatchController.refreshSearchId();
    noMatchController.addSearchDir(dir.path());
    noMatchController.addKeyword(QStringLiteral("missing-keyword"));
    noMatchController.setRecurse(false);
    noMatchController.setMaxResultNum(10);
    noMatchController.setResultProperties(SearchProperty::SearchType::File,
                                          SearchResultProperties{SearchProperty::SearchResultProperty::FilePath});
    FileSearchWorker noMatchWorker(&task, &noMatchController);
    noMatchWorker.m_validDirectories << dir.path();
    noMatchWorker.m_blackList << QStringList{};
    QVERIFY(noMatchWorker.directSearch());
    QVERIFY(noMatchQueue->isEmpty());

    SearchController matchAllController;
    DataQueue<ResultItem> *matchAllQueue = matchAllController.initDataQueue();
    matchAllController.refreshSearchId();
    matchAllController.addSearchDir(dir.path());
    matchAllController.setMatchAllIfNoKeyword(true);
    matchAllController.setOnlySearchFile(true);
    matchAllController.setRecurse(false);
    matchAllController.setMaxResultNum(10);
    matchAllController.setResultProperties(SearchProperty::SearchType::File,
                                           SearchResultProperties{SearchProperty::SearchResultProperty::FileName});
    FileSearchWorker matchAllWorker(&task, &matchAllController);
    matchAllWorker.m_validDirectories << dir.path();
    matchAllWorker.m_blackList << QStringList{};
    QVERIFY(matchAllWorker.directSearch());
    QVERIFY(matchAllQueue->length() >= 1);

    SearchController linkController;
    DataQueue<ResultItem> *linkQueue = linkController.initDataQueue();
    linkController.refreshSearchId();
    linkController.addSearchDir(dir.path());
    linkController.addKeyword(QStringLiteral("linked-dir"));
    linkController.setOnlySearchDir(true);
    linkController.setRecurse(true);
    linkController.setMaxResultNum(10);
    linkController.setResultProperties(SearchProperty::SearchType::File,
                                       SearchResultProperties{SearchProperty::SearchResultProperty::FilePath});
    FileSearchWorker linkWorker(&task, &linkController);
    linkWorker.m_validDirectories << dir.path();
    linkWorker.m_blackList << QStringList{};
    QVERIFY(linkWorker.directSearch());
    while (!linkQueue->isEmpty()) {
        linkQueue->dequeue();
    }

}

void LibsearchWhiteboxTest::fileSearchTaskDirectSearchCoversLabelsMissingPathsAndIconProperty()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir root(dir.path());

    QFile target(root.filePath(QStringLiteral("label-target.txt")));
    QVERIFY(target.open(QIODevice::WriteOnly | QIODevice::Truncate));
    target.write("target");
    target.close();

    FileSearchTask task;

    SearchController iconController;
    DataQueue<ResultItem> *iconQueue = iconController.initDataQueue();
    iconController.refreshSearchId();
    iconController.addSearchDir(dir.path());
    iconController.addKeyword(QStringLiteral("label-target"));
    iconController.setOnlySearchFile(true);
    iconController.setRecurse(false);
    iconController.setMaxResultNum(10);
    iconController.setResultProperties(SearchProperty::SearchType::File,
                                       SearchResultProperties{SearchProperty::SearchResultProperty::FilePath,
                                                              SearchProperty::SearchResultProperty::FileIconName,
                                                              SearchProperty::SearchResultProperty::ModifiedTime});
    FileSearchWorker iconWorker(&task, &iconController);
    iconWorker.m_validDirectories << root.filePath(QStringLiteral("missing-dir")) << dir.path();
    iconWorker.m_blackList << QStringList{};

    QVERIFY(iconWorker.directSearch());
    QCOMPARE(iconQueue->length(), 1);
    ResultItem result = iconQueue->dequeue();
    QCOMPARE(result.getValue(SearchProperty::SearchResultProperty::FilePath).toString(), target.fileName());
    QVERIFY(result.getValue(SearchProperty::SearchResultProperty::ModifiedTime).toDateTime().isValid());
    QVERIFY(result.getValue(SearchProperty::SearchResultProperty::FileIconName).isValid());

    SearchController labelController;
    DataQueue<ResultItem> *labelQueue = labelController.initDataQueue();
    labelController.refreshSearchId();
    labelController.addSearchDir(dir.path());
    labelController.addKeyword(QStringLiteral("label-target"));
    labelController.addFileLabel(QStringLiteral("missing-label"));
    labelController.setOnlySearchFile(true);
    labelController.setRecurse(false);
    labelController.setMaxResultNum(10);
    labelController.setResultProperties(SearchProperty::SearchType::File,
                                        SearchResultProperties{SearchProperty::SearchResultProperty::FilePath});
    FileSearchWorker labelWorker(&task, &labelController);
    labelWorker.m_validDirectories << dir.path();
    labelWorker.m_blackList << QStringList{};

    QVERIFY(labelWorker.directSearch());
    QVERIFY(labelQueue->length() <= 1);

    QSignalSpy errorSpy(&task, &FileSearchTask::searchError);
    iconWorker.sendErrorMsg(QStringLiteral("file-search-whitebox-error"));
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(errorSpy.takeFirst().at(1).toString(), QStringLiteral("file-search-whitebox-error"));
}

void LibsearchWhiteboxTest::fileSearchTaskFilterCoversDirectoryDepthAndLabels()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("child/grand")));

    const QString topPath = dir.filePath(QStringLiteral("top.txt"));
    QFile top(topPath);
    QVERIFY(top.open(QIODevice::WriteOnly | QIODevice::Truncate));
    top.write("top");
    top.close();

    const QString nestedPath = dir.filePath(QStringLiteral("child/grand/nested.txt"));
    QFile nested(nestedPath);
    QVERIFY(nested.open(QIODevice::WriteOnly | QIODevice::Truncate));
    nested.write("nested");
    nested.close();

    FileSearchTask task;
    SearchController nonRecursiveController;
    nonRecursiveController.initDataQueue();
    nonRecursiveController.refreshSearchId();
    nonRecursiveController.setRecurse(false);
    FileSearchWorker nonRecursiveWorker(&task, &nonRecursiveController);
    nonRecursiveWorker.m_validDirectories << dir.path();

    Xapian::Document topDoc;
    topDoc.set_data(topPath.toStdString());
    Xapian::Document nestedDoc;
    nestedDoc.set_data(nestedPath.toStdString());
    FileSearchFilter nonRecursiveFilter(&nonRecursiveWorker);
    QVERIFY(nonRecursiveFilter(topDoc));
    QVERIFY(!nonRecursiveFilter(nestedDoc));

    SearchController recursiveController;
    recursiveController.initDataQueue();
    recursiveController.refreshSearchId();
    recursiveController.setRecurse(true);
    FileSearchWorker recursiveWorker(&task, &recursiveController);
    recursiveWorker.m_validDirectories << dir.path();
    recursiveWorker.m_labels << QStringLiteral("missing-label");
    FileSearchFilter labelFilter(&recursiveWorker);
    QVERIFY(!labelFilter(topDoc));

    Xapian::Document outsideDoc;
    outsideDoc.set_data(QDir::temp().filePath(QStringLiteral("outside-libsearch-whitebox.txt")).toStdString());
    QVERIFY(!nonRecursiveFilter(outsideDoc));

    FileSearchFilter nullFilter(nullptr);
    QVERIFY(nullFilter(outsideDoc));
}

void LibsearchWhiteboxTest::fileSearchTaskSearchWithIndexCoversErrorAndResults()
{
    ensureSearchTaskConfigDirs();
    QDir(INDEX_PATH).removeRecursively();

    FileSearchTask task;
    SearchController errorController;
    errorController.initDataQueue();
    errorController.refreshSearchId();
    errorController.addKeyword(QStringLiteral("missing-db"));
    FileSearchWorker errorWorker(&task, &errorController);

    QSignalSpy errorSpy(&task, &FileSearchTask::searchError);
    QVERIFY(!errorWorker.searchWithIndex());
    QCOMPARE(errorSpy.count(), 1);
    QVERIFY(errorSpy.takeFirst().at(1).toString().contains(QStringLiteral("Xapian Error")));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString filePath = dir.filePath(QStringLiteral("indexed-target.txt"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write("indexed");
    file.close();

    writeFileSearchTaskDatabase({{filePath, FILE_SEARCH_VALUE}}, QStringLiteral("indexed"));

    SearchController controller;
    DataQueue<ResultItem> *queue = controller.initDataQueue();
    controller.refreshSearchId();
    controller.addKeyword(QStringLiteral("indexed"));
    controller.setMaxResultNum(5);
    controller.setInformNum(1);
    controller.setResultProperties(SearchProperty::SearchType::File,
                                   SearchResultProperties{SearchProperty::SearchResultProperty::FilePath,
                                                          SearchProperty::SearchResultProperty::FileIconName,
                                                          SearchProperty::SearchResultProperty::FileName,
                                                          SearchProperty::SearchResultProperty::ModifiedTime});

    int informCount = 0;
    connect(&task, &FileSearchTask::reachInformNum, this, [&informCount]() {
        ++informCount;
    });

    FileSearchWorker worker(&task, &controller);
    worker.m_validDirectories << dir.path();
    QVERIFY(worker.searchWithIndex());
    QCOMPARE(queue->length(), 1);
    ResultItem result = queue->dequeue();
    QCOMPARE(result.getItemKey(), filePath);
    QCOMPARE(result.getValue(SearchProperty::SearchResultProperty::FilePath).toString(), filePath);
    QVERIFY(result.getAllValue().contains(SearchProperty::SearchResultProperty::FileIconName));
    QCOMPARE(result.getValue(SearchProperty::SearchResultProperty::FileName).toString(), QStringLiteral("indexed-target.txt"));
    QVERIFY(result.getValue(SearchProperty::SearchResultProperty::ModifiedTime).toDateTime().isValid());
    QCOMPARE(informCount, 1);

    controller.getDataQueue()->clear();
    const size_t oldSearchId = controller.getCurrentSearchId();
    controller.stop();
    FileSearchWorker stoppedWorker(&task, &controller);
    stoppedWorker.m_currentSearchId = oldSearchId;
    stoppedWorker.m_validDirectories << dir.path();
    QVERIFY(!stoppedWorker.searchWithIndex());
}

void LibsearchWhiteboxTest::fileSearchTaskStartSearchCopiesControllerAndFinishes()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QFile file(dir.filePath(QStringLiteral("async-target.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write("async");
    file.close();

    FileSearchTask task;
    SearchController controller;
    DataQueue<ResultItem> *queue = controller.initDataQueue();
    controller.refreshSearchId();
    controller.addSearchDir(dir.path());
    controller.addKeyword(QStringLiteral("async-target"));
    controller.setSearchHiddenFiles(true);
    controller.setRecurse(false);
    controller.setMaxResultNum(5);
    controller.setResultProperties(SearchProperty::SearchType::File,
                                   SearchResultProperties{SearchProperty::SearchResultProperty::FilePath});

    task.setController(controller);
    QSignalSpy finishedSpy(&task, &FileSearchTask::searchFinished);
    QVERIFY(!task.isSearching());
    task.startSearch();
    QTRY_VERIFY(task.isSearching() || finishedSpy.count() == 1);
    QTRY_COMPARE(finishedSpy.count(), 1);
    QVERIFY(queue->length() >= 1);
}

void LibsearchWhiteboxTest::fileSearchTaskRunCoversDuplicateDirsAndBlockedSkipHelpers()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QFile matched(dir.filePath(QStringLiteral("run-target.txt")));
    QVERIFY(matched.open(QIODevice::WriteOnly | QIODevice::Truncate));
    matched.write("run");
    matched.close();

    FileSearchTask task;
    SearchController controller;
    DataQueue<ResultItem> *queue = controller.initDataQueue();
    const size_t searchId = controller.refreshSearchId();
    controller.addSearchDir(dir.path());
    controller.addSearchDir(dir.path() + QStringLiteral("/"));
    controller.addSearchDir(dir.filePath(QStringLiteral("missing-dir")));
    controller.addKeyword(QStringLiteral("run-target"));
    controller.addFileLabel(QString());
    controller.addFileLabel(QStringLiteral("non-empty-label"));
    controller.setSearchHiddenFiles(true);
    controller.setRecurse(false);
    controller.setMaxResultNum(5);
    controller.setResultProperties(SearchProperty::SearchType::File,
                                   SearchResultProperties{SearchProperty::SearchResultProperty::FilePath});

    FileSearchWorker worker(&task, &controller);
    QSignalSpy finishedSpy(&task, &FileSearchTask::searchFinished);
    worker.run();
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(finishedSpy.takeFirst().at(0).value<size_t>(), searchId);
    QCOMPARE(worker.m_validDirectories, (QStringList{dir.path(), dir.path()}));
    QCOMPARE(worker.m_labels, QStringList{QStringLiteral("non-empty-label")});
    QVERIFY(queue->isEmpty());

    worker.m_skipCheckDirs << dir.path();
    QVERIFY(!worker.underBlocked(dir.filePath(QStringLiteral("child.txt"))));

    Xapian::Document blockedDoc;
    blockedDoc.set_data(dir.filePath(QStringLiteral("blocked.txt")).toStdString());
    FileSearchFilter blockedFilter(&worker);
    QVERIFY(!blockedFilter(blockedDoc));
}

void LibsearchWhiteboxTest::fileContentSearchTaskCoversMetadataAndFilter()
{
    FileContentSearchTask task;
    QCOMPARE(task.pluginType(), PluginInterface::PluginType::SearchTaskPlugin);
    QVERIFY(!task.name().isEmpty());
    QVERIFY(!task.description().isEmpty());
    QVERIFY(!task.getCustomSearchType().isEmpty());
    QCOMPARE(task.getSearchType(), SearchProperty::SearchType::FileContent);
    QVERIFY(!task.isSearching());

    task.setEnable(false);
    QVERIFY(!task.isEnable());

    SearchController controller;
    controller.initDataQueue();
    controller.addKeyword(QStringLiteral("系统"));
    FileContentSearchWorker worker(&task, &controller);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("content.txt"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write("content");
    file.close();

    Xapian::Document existingDoc;
    existingDoc.add_value(CONTENT_DATABASE_PATH_SLOT, path.toStdString());
    FileContentSearchFilter filter(&worker);
    QVERIFY(filter(existingDoc));

    controller.addSearchDir(dir.path());
    FileContentSearchWorker scopedWorker(&task, &controller);
    scopedWorker.m_validDirectories << dir.path();
    FileContentSearchFilter scopedFilter(&scopedWorker);
    QVERIFY(scopedFilter(existingDoc));

    Xapian::Document outsideDoc;
    outsideDoc.add_value(CONTENT_DATABASE_PATH_SLOT, QDir::temp().filePath(QStringLiteral("outside-content.txt")).toStdString());
    QFile outsideFile(QString::fromStdString(outsideDoc.get_value(CONTENT_DATABASE_PATH_SLOT)));
    QVERIFY(outsideFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    outsideFile.write("outside");
    outsideFile.close();
    QVERIFY(!scopedFilter(outsideDoc));
    outsideFile.remove();

    Xapian::Document missingDoc;
    missingDoc.add_value(CONTENT_DATABASE_PATH_SLOT, "/not/existing/content.txt");
    QVERIFY(!filter(missingDoc));

    QVERIFY(!worker.underBlocked(path));
    QVERIFY(!worker.createQuery().empty());
    worker.sendErrorMsg(QStringLiteral("content-error"));
}

void LibsearchWhiteboxTest::fileContentSearchTaskRunCoversDisabledAndErrorBranches()
{
    FileContentSearchTask disabledTask;
    SearchController disabledController;
    disabledController.initDataQueue();
    const size_t disabledSearchId = disabledController.refreshSearchId();
    disabledController.addSearchDir(QDir::tempPath() + QStringLiteral("/"));
    disabledTask.setController(disabledController);
    disabledTask.setEnable(false);

    QSignalSpy errorSpy(&disabledTask, &FileContentSearchTask::searchError);
    QSignalSpy finishedSpy(&disabledTask, &FileContentSearchTask::searchFinished);
    FileContentSearchWorker disabledWorker(&disabledTask, &disabledTask.m_searchController);
    disabledWorker.run();

    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(errorSpy.takeFirst().at(0).value<size_t>(), disabledSearchId);
    QCOMPARE(finishedSpy.count(), 0);
    QVERIFY(disabledWorker.m_validDirectories.contains(QDir::tempPath()));

    FileContentSearchTask enabledTask;
    SearchController enabledController;
    enabledController.initDataQueue();
    const size_t enabledSearchId = enabledController.refreshSearchId();
    enabledController.addKeyword(QStringLiteral("content"));
    enabledController.setResultProperties(SearchProperty::SearchType::FileContent,
                                          SearchResultProperties{SearchProperty::SearchResultProperty::FilePath});
    enabledTask.setController(enabledController);
    enabledTask.m_enable = true;
    IndexStatusRecorder::getInstance()->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::Ready);

    QSignalSpy enabledErrorSpy(&enabledTask, &FileContentSearchTask::searchError);
    FileContentSearchWorker enabledWorker(&enabledTask, &enabledTask.m_searchController);
    const bool searchOk = enabledWorker.execSearch();
    QVERIFY(searchOk || enabledErrorSpy.count() == 1);
    if (!searchOk) {
        QCOMPARE(enabledErrorSpy.takeFirst().at(0).value<size_t>(), enabledSearchId);
    }
}

void LibsearchWhiteboxTest::fileContentSearchTaskIndexedSearchAndAsyncStart()
{
    ensureSearchTaskConfigDirs();
    QDir(CONTENT_INDEX_PATH).removeRecursively();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString filePath = dir.filePath(QStringLiteral("content-indexed.txt"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write("indexed searchable content");
    file.close();

    fileContentIndexer indexer(filePath);
    QVERIFY(indexer.index());
    try {
        Xapian::WritableDatabase database(CONTENT_INDEX_PATH.toStdString(), Xapian::DB_CREATE_OR_OVERWRITE);
        database.add_document(indexer.document().getXapianDocument());
        database.commit();
    } catch (const Xapian::DatabaseLockError &e) {
        const QByteArray reason = QByteArray("shared content index database is locked: ")
                + QByteArray::fromStdString(e.get_description());
        QSKIP(reason.constData());
    }

    FileContentSearchTask task;
    task.setEnable(true);
    const QIcon taskIcon = task.icon();
    QVERIFY(taskIcon.isNull() || !taskIcon.name().isEmpty() || !taskIcon.availableSizes().isEmpty());
    IndexStatusRecorder::getInstance()->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::Updating);
    QVERIFY(task.isEnable());
    IndexStatusRecorder::getInstance()->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::Ready);

    SearchController controller;
    DataQueue<ResultItem> *queue = controller.initDataQueue();
    controller.refreshSearchId();
    controller.addSearchDir(dir.path() + QStringLiteral("/"));
    controller.addKeyword(QStringLiteral("indexed"));
    controller.setInformNum(1);
    controller.setMaxResultNum(5);
    controller.setResultProperties(SearchProperty::SearchType::FileContent,
                                   SearchResultProperties{SearchProperty::SearchResultProperty::FilePath,
                                                          SearchProperty::SearchResultProperty::FileIconName,
                                                          SearchProperty::SearchResultProperty::FileName,
                                                          SearchProperty::SearchResultProperty::ModifiedTime});

    task.setController(controller);
    QSignalSpy finishedSpy(&task, &FileContentSearchTask::searchFinished);
    int informCount = 0;
    connect(&task, &FileContentSearchTask::reachInformNum, this, [&informCount]() {
        ++informCount;
    });

    QVERIFY(!task.isSearching());
    task.startSearch();
    QTRY_VERIFY(task.isSearching() || finishedSpy.count() == 1);
    QTRY_COMPARE(finishedSpy.count(), 1);
    QCOMPARE(queue->length(), 1);
    ResultItem result = queue->dequeue();
    QCOMPARE(result.getItemKey(), filePath);
    QCOMPARE(result.getValue(SearchProperty::SearchResultProperty::FilePath).toString(), filePath);
    QCOMPARE(result.getValue(SearchProperty::SearchResultProperty::FileName).toString(), QStringLiteral("content-indexed.txt"));
    QVERIFY(result.getValue(SearchProperty::SearchResultProperty::FileIconName).isValid());
    QVERIFY(result.getValue(SearchProperty::SearchResultProperty::ModifiedTime).isValid());
    QCOMPARE(informCount, 1);

    FileContentSearchWorker skipWorker(&task, &controller);
    skipWorker.m_skipCheckDirs << dir.path();
    QVERIFY(!skipWorker.underBlocked(dir.filePath(QStringLiteral("child.txt"))));

    controller.getDataQueue()->clear();
    const size_t oldSearchId = controller.getCurrentSearchId();
    controller.stop();
    FileContentSearchWorker stoppedWorker(&task, &controller);
    stoppedWorker.m_currentSearchId = oldSearchId;
    QVERIFY(!stoppedWorker.execSearch());
}

void LibsearchWhiteboxTest::ukuiSearchTaskCoversControllerFacadeAndErrorPath()
{
    UkuiSearchTask task;
    int errorCount = 0;
    size_t errorSearchId = 0;
    QString errorMessage;
    connect(&task, &UkuiSearchTask::searchError, this, [&errorCount, &errorSearchId, &errorMessage](size_t id, const QString &message) {
        ++errorCount;
        errorSearchId = id;
        errorMessage = message;
    });

    QVERIFY(task.init() != nullptr);
    task.addSearchDir(QStringLiteral("/tmp"));
    task.addKeyword(QStringLiteral("demo"));
    task.addFileLabel(QStringLiteral("label"));
    task.setRecurse(false);
    task.setMatchAllIfNoKeyword(true);
    task.setOnlySearchFile(true);
    task.setOnlySearchDir(false);
    task.setSearchOnlineApps(false);
    task.setSearchHiddenFiles(true);
    task.setMaxResultNum(3);
    task.setInformNum(2);
    QVERIFY(task.setResultProperties(SearchProperty::SearchType::File,
                                     SearchResultProperties{SearchProperty::SearchResultProperty::FilePath,
                                                            SearchProperty::SearchResultProperty::FileName}));
    task.setCustomResultDataType(QStringLiteral("custom"), QStringList{QStringLiteral("name")});

    const size_t searchId = task.startSearch(SearchProperty::SearchType::Custom, QStringLiteral("not-registered"));
    QCOMPARE(errorCount, 1);
    QCOMPARE(errorSearchId, searchId);
    QVERIFY(errorMessage.contains(QStringLiteral("unregistered plugin")));
    QVERIFY(!task.isSearching(SearchProperty::SearchType::Custom, QStringLiteral("not-registered")));

    task.clearKeyWords();
    task.clearSearchDir();
    task.clearFileLabel();
    task.clearAllConditions();
    task.stop();
}

void LibsearchWhiteboxTest::searchTaskPluginManagerCoversManagedPluginBranches()
{
    ManagedPlugin managed(nullptr);
    QVERIFY(!managed.m_internalPlugins.contains(static_cast<size_t>(SearchProperty::SearchType::File)));
    QVERIFY(!managed.m_externalPlugin.contains(QStringLiteral("external")));
    QCOMPARE(managed.internalPlugin(SearchProperty::SearchType::File), nullptr);
    QCOMPARE(managed.externalPlugin(QStringLiteral("external")), nullptr);
    QVERIFY(!managed.insertInternalPlugin(SearchProperty::SearchType::File, nullptr));
    QVERIFY(!managed.insertExternalPlugin(QString(), nullptr));

    auto *fileTask = new FileSearchTask;
    QVERIFY(managed.insertInternalPlugin(SearchProperty::SearchType::File, fileTask));
    QCOMPARE(managed.m_internalPlugins.value(static_cast<size_t>(SearchProperty::SearchType::File)), fileTask);
    QVERIFY(!managed.insertInternalPlugin(SearchProperty::SearchType::File, new FileSearchTask));

    auto *contentTask = new FileContentSearchTask;
    QVERIFY(managed.insertExternalPlugin(QStringLiteral("content"), contentTask));
    QCOMPARE(managed.m_externalPlugin.value(QStringLiteral("content")), contentTask);
    QVERIFY(!managed.insertExternalPlugin(QStringLiteral("content"), new FileContentSearchTask));
    QVERIFY(!managed.insertExternalPlugin(QString(), new FileContentSearchTask));

    SearchTaskPluginManager *manager = SearchTaskPluginManager::getInstance();
    const QUuid uuid = QUuid::createUuid();
    QVERIFY(!manager->startSearch(uuid, SearchProperty::SearchType::File));
    QVERIFY(!manager->isSearching(uuid, SearchProperty::SearchType::File));
    QVERIFY(manager->getPlugin(SearchProperty::SearchType::File) != nullptr);
    delete manager->getPlugin(SearchProperty::SearchType::FileContent);
    QVERIFY(manager->getPlugin(SearchProperty::SearchType::Custom, QStringLiteral("missing")) == nullptr);

    manager->registerPluginPath(QStringLiteral("missing"), QStringLiteral("/not/existing/plugin.so"));
    QVERIFY(manager->m_loadedPluginPath.contains(QStringLiteral("missing")));
    QVERIFY(manager->getPlugin(SearchProperty::SearchType::Custom, QStringLiteral("missing")) == nullptr);

    const QUuid managedUuid = QUuid::createUuid();
    auto *managedPlugin = new ManagedPlugin(nullptr);
    auto *managedFileTask = new ManagedSearchTaskStub(false);
    auto *managedCustomTask = new ManagedSearchTaskStub(true);
    QVERIFY(managedPlugin->insertInternalPlugin(SearchProperty::SearchType::File, managedFileTask));
    QVERIFY(managedPlugin->insertExternalPlugin(QStringLiteral("custom-managed"), managedCustomTask));
    manager->m_managedPlugins.insert(managedUuid, managedPlugin);

    QVERIFY(manager->startSearch(managedUuid, SearchProperty::SearchType::File));
    QCOMPARE(managedFileTask->m_startCalls, 1);
    QVERIFY(manager->isSearching(managedUuid, SearchProperty::SearchType::File));
    QVERIFY(manager->startSearch(managedUuid, SearchProperty::SearchType::Custom, QStringLiteral("custom-managed")));
    QCOMPARE(managedCustomTask->m_startCalls, 1);
    QVERIFY(manager->isSearching(managedUuid, SearchProperty::SearchType::Custom, QStringLiteral("custom-managed")));
    QVERIFY(!manager->startSearch(managedUuid, SearchProperty::SearchType::Custom, QStringLiteral("missing-custom")));
    QVERIFY(!manager->isSearching(managedUuid, SearchProperty::SearchType::Custom, QStringLiteral("missing-custom")));

    const QUuid initUuid = QUuid::createUuid();
    SearchTaskPluginIface *fileTaskFromManager = manager->initPlugins(initUuid, SearchProperty::SearchType::File);
    QVERIFY(fileTaskFromManager != nullptr);
    QCOMPARE(manager->m_managedPlugins.value(initUuid)->internalPlugin(SearchProperty::SearchType::File), fileTaskFromManager);
    QVERIFY(manager->initPlugins(initUuid, SearchProperty::SearchType::File) == nullptr);
    QVERIFY(manager->initPlugins(QUuid::createUuid(), SearchProperty::SearchType::Custom, QStringLiteral("missing-custom")) == nullptr);

    manager->destroyPlugins(managedUuid);
    manager->destroyPlugins(initUuid);
    manager->destroyPlugins(uuid);
}

void LibsearchWhiteboxTest::indexMonitorCoversReplicaDefaultsAndSingletonLifecycle()
{
    qputenv("XDG_SESSION_TYPE", "x11");
    qputenv("DISPLAY", ":whitebox");

    IndexMonitor::startMonitor();
    IndexMonitor *monitor = IndexMonitor::self();
    QVERIFY(monitor != nullptr);
    QCOMPARE(IndexMonitor::self(), monitor);

    QVERIFY(monitor->currentIndexPaths().isEmpty());
    QVERIFY(monitor->indexState().isEmpty());
    QCOMPARE(monitor->basicIndexSize(), 0U);
    QCOMPARE(monitor->contentIndexSize(), 0U);
    QCOMPARE(monitor->ocrContentIndexSize(), 0U);
    QCOMPARE(monitor->aiIndexSize(), 0U);
    QCOMPARE(monitor->basicIndexProgress(), 0U);
    QCOMPARE(monitor->contentIndexProgress(), 0U);
    QCOMPARE(monitor->ocrContentIndexProgress(), 0U);
    QCOMPARE(monitor->aiIndexProgress(), 0U);
    QCOMPARE(monitor->basicIndexDocNum(), 0U);
    QCOMPARE(monitor->contentIndexDocNum(), 0U);
    QCOMPARE(monitor->ocrContentIndexDocNum(), 0U);
    QVERIFY(!monitor->basicIndexUpdating());
    QVERIFY(!monitor->contentIndexUpdating());
    QVERIFY(!monitor->ocrContentIndexUpdating());
    QVERIFY(!monitor->aiIndexUpdating());

    QSignalSpy offlineSpy(monitor, &IndexMonitor::serviceOffline);
    QMetaObject::invokeMethod(monitor, [monitor]() {
        Q_EMIT monitor->serviceOffline();
    }, Qt::QueuedConnection);
    QTRY_COMPARE(offlineSpy.count(), 1);

    IndexMonitor::stopMonitor();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    qputenv("XDG_SESSION_TYPE", "wayland");
    qputenv("WAYLAND_DISPLAY", "wayland-whitebox");
    IndexMonitor *waylandMonitor = IndexMonitor::self();
    QVERIFY(waylandMonitor != nullptr);
    QVERIFY(waylandMonitor->currentIndexPaths().isEmpty());
    IndexMonitor::stopMonitor();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

void LibsearchWhiteboxTest::fileSearchTaskDirectSearchNonRecursiveAndEmptyDirs()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("nonrec")));
    QFile f(dir.filePath(QStringLiteral("nonrec/match.txt")));
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
    f.write("match-content");
    f.close();

    FileSearchTask task;
    SearchController controller;
    controller.initDataQueue();
    controller.refreshSearchId();
    controller.addKeyword(QStringLiteral("match"));
    controller.addSearchDir(dir.filePath(QStringLiteral("nonrec")));
    controller.setRecurse(false);
    controller.setInformNum(1);

    QSignalSpy spy(&task, &FileSearchTask::searchFinished);
    task.setController(controller);
    task.startSearch();
    QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 1, 5000);
    task.stop();
}

void LibsearchWhiteboxTest::fileSearchTaskDirectSearchWithManyProperties()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile f(dir.filePath(QStringLiteral("props.txt")));
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
    f.write("property test content");
    f.close();

    FileSearchTask task;
    SearchController controller;
    controller.initDataQueue();
    controller.refreshSearchId();
    controller.addKeyword(QStringLiteral("property"));
    controller.addSearchDir(dir.path());
    controller.setRecurse(false);
    controller.setMaxResultNum(10);
    controller.setInformNum(1);
    controller.setResultProperties(SearchProperty::SearchType::File,
        SearchResultProperties{SearchProperty::SearchResultProperty::FilePath,
                               SearchProperty::SearchResultProperty::FileName,
                               SearchProperty::SearchResultProperty::FileIconName,
                               SearchProperty::SearchResultProperty::ModifiedTime});

    task.setController(controller);
    task.startSearch();
    QTest::qWait(500);
    task.stop();
}

void LibsearchWhiteboxTest::fileSearchTaskFilterWithAllFileTypes()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QStringList exts{"txt", "md", "log", "xml", "json", "cpp", "h", "png"};
    for (const auto &ext : exts) {
        QFile f(dir.filePath(QStringLiteral("file.") + ext));
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write("filter test " + ext.toUtf8());
        f.close();
    }

    FileSearchTask task;
    SearchController controller;
    controller.initDataQueue();
    controller.refreshSearchId();
    controller.addKeyword(QStringLiteral("filter"));
    controller.addSearchDir(dir.path());
    controller.setRecurse(false);
    controller.setInformNum(1);
    task.setController(controller);
    task.startSearch();
    QTest::qWait(500);
    task.stop();
}

void LibsearchWhiteboxTest::fileSearchTaskStartSearchWithNoDirs()
{
    FileSearchTask task;
    SearchController controller;
    controller.initDataQueue();
    controller.refreshSearchId();
    controller.addKeyword(QStringLiteral("nothing"));
    controller.setInformNum(1);

    QSignalSpy spy(&task, &FileSearchTask::searchFinished);
    task.setController(controller);
    task.startSearch();
    QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 1, 5000);
    task.stop();
}

extern "C" void __gcov_dump();

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    LibsearchWhiteboxTest tc;
    const int status = QTest::qExec(&tc, argc, argv);
    __gcov_dump();
    _exit(status);
}

#include "libsearch-whitebox-test.moc"
