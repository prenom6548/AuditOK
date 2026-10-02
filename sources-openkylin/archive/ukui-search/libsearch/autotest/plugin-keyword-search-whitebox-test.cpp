#include <xapian.h>

#include <QApplication>
#include <QClipboard>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDomDocument>
#include <QFile>
#include <QImage>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextBoundaryFinder>
#include <QTest>
#include <QTimer>
#include <memory>
#include <signal.h>

#define private public
#define protected public
#include "aisearch/ai-search-plugin.h"
#include "dirwatcher/dir-watcher.h"
#include "global-settings-private.h"
#include "index/file-content-indexer.h"
#include "index/file-indexer-config.h"
#include "index/file-search-plugin.h"
#include "index/search-manager.h"
#include "notesearch/note-search-plugin.h"
#include "settingsearch/settings-search-plugin.h"
#include "websearch/web-search-plugin.h"
#undef protected
#undef private

using namespace UkuiSearch;

namespace {

class FakeAppLauncher : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.ProcessManager.AppLauncher")

public:
    QString lastDesktopFile;
    QStringList lastArguments;
    QString lastUrl;
    int launchAppCalls = 0;
    int launchUrlCalls = 0;

public Q_SLOTS:
    void LaunchAppWithArguments(const QString &desktopFile, const QStringList &arguments)
    {
        lastDesktopFile = desktopFile;
        lastArguments = arguments;
        ++launchAppCalls;
    }

    void LaunchDefaultAppWithUrl(const QString &url)
    {
        lastUrl = url;
        ++launchUrlCalls;
    }
};

class BrokenAppLauncher : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.ProcessManager.AppLauncher")
};

class FakeFileManager1Service : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.FileManager1")

public:
    QStringList lastItems;
    QString lastStartupId;

public Q_SLOTS:
    void ShowItems(const QStringList &items, const QString &startupId)
    {
        lastItems = items;
        lastStartupId = startupId;
    }
};

class FakeNoteService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.note.interface")

public:
    QStringList lastKeywords;
    QVariantMap replyMap;
    int delayMs = 0;

public Q_SLOTS:
    QVariantMap keywordMatch(const QStringList &keywords)
    {
        lastKeywords = keywords;
        if (delayMs > 0) {
            QTest::qSleep(delayMs);
        }
        return replyMap;
    }
};

class BrokenNoteService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.note.interface")
};

class FakeSettingsSearchService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.search.interface")

public Q_SLOTS:
    QVariantMap getSearchItems() const
    {
        return {};
    }

Q_SIGNALS:
    void searchItemsAdd(const QVariantMap &items);
    void searchItemsDelete(const QVariantMap &items);
};

class BrokenSettingsSearchService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.search.interface")
};

class FakeDateServerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.kysdk.DateInterface")

Q_SIGNALS:
    void ShortDateSignal(const QString &format);
};

class FakePrivateDirWatcherService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.search.fileindex")

public:
    QStringList indexableDirs{QDir::homePath()};
    QStringList blackListDirs;
    QStringList blockDirs;
    QStringList extraDirList;

public Q_SLOTS:
    QStringList currentIndexableDir() const
    {
        return indexableDirs;
    }

    QStringList blackListOfDir(const QString &) const
    {
        return blackListDirs;
    }

    QStringList blockDirsForUser() const
    {
        return blockDirs;
    }

    QStringList currentBlackListOfIndex() const
    {
        return {};
    }

    QStringList extraDirs() const
    {
        return extraDirList;
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

    bool isRegistered() const
    {
        return m_serviceRegistered && m_objectRegistered;
    }

private:
    QString m_service;
    QString m_path;
    QDBusConnection m_connection{QString()};
    bool m_serviceRegistered = false;
    bool m_objectRegistered = false;
};

class ScopedEnvVar
{
public:
    ScopedEnvVar(const QByteArray &name, const QByteArray &value)
        : m_name(name),
          m_hadValue(qEnvironmentVariableIsSet(name.constData())),
          m_oldValue(qgetenv(name.constData()))
    {
        qputenv(m_name, value);
    }

    ~ScopedEnvVar()
    {
        if (m_hadValue) {
            qputenv(m_name, m_oldValue);
        } else {
            qunsetenv(m_name);
        }
    }

private:
    QByteArray m_name;
    bool m_hadValue;
    QByteArray m_oldValue;
};

class ScopedIndexableDirs
{
public:
    ScopedIndexableDirs(FakePrivateDirWatcherService &service, const QStringList &indexableDirs)
        : m_service(service),
          m_oldIndexableDirs(service.indexableDirs)
    {
        m_service.indexableDirs = indexableDirs;
    }

    ~ScopedIndexableDirs()
    {
        m_service.indexableDirs = m_oldIndexableDirs;
    }

private:
    FakePrivateDirWatcherService &m_service;
    QStringList m_oldIndexableDirs;
};

class ScopedDirWatcherLists
{
public:
    ScopedDirWatcherLists(FakePrivateDirWatcherService &service,
                          const QStringList &indexableDirs,
                          const QStringList &blackListDirs,
                          const QStringList &blockDirs,
                          const QStringList &extraDirs)
        : m_service(service),
          m_oldIndexableDirs(service.indexableDirs),
          m_oldBlackListDirs(service.blackListDirs),
          m_oldBlockDirs(service.blockDirs),
          m_oldExtraDirs(service.extraDirList)
    {
        m_service.indexableDirs = indexableDirs;
        m_service.blackListDirs = blackListDirs;
        m_service.blockDirs = blockDirs;
        m_service.extraDirList = extraDirs;
    }

    ~ScopedDirWatcherLists()
    {
        m_service.indexableDirs = m_oldIndexableDirs;
        m_service.blackListDirs = m_oldBlackListDirs;
        m_service.blockDirs = m_oldBlockDirs;
        m_service.extraDirList = m_oldExtraDirs;
    }

private:
    FakePrivateDirWatcherService &m_service;
    QStringList m_oldIndexableDirs;
    QStringList m_oldBlackListDirs;
    QStringList m_oldBlockDirs;
    QStringList m_oldExtraDirs;
};

class ScopedSessionBusDaemon
{
public:
    bool start()
    {
        if (m_started) {
            return true;
        }

        if (!m_runtimeDir.isValid()) {
            return false;
        }

        const QString busPath = m_runtimeDir.filePath(QStringLiteral("bus"));
        QFile::remove(busPath);

        QProcess process;
        process.start(QStringLiteral("dbus-daemon"),
                      QStringList{QStringLiteral("--session"),
                                  QStringLiteral("--address=unix:path=%1").arg(busPath),
                                  QStringLiteral("--fork"),
                                  QStringLiteral("--print-address=1"),
                                  QStringLiteral("--print-pid=1")});
        if (!process.waitForFinished(5000) || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
            return false;
        }

        const QList<QByteArray> lines = process.readAllStandardOutput().split('\n');
        if (lines.size() < 2) {
            return false;
        }

        m_oldHadValue = qEnvironmentVariableIsSet("DBUS_SESSION_BUS_ADDRESS");
        m_oldAddress = qgetenv("DBUS_SESSION_BUS_ADDRESS");
        m_address = lines.at(0).trimmed();
        bool ok = false;
        m_pid = lines.at(1).trimmed().toLongLong(&ok);
        if (m_address.isEmpty() || !ok || m_pid <= 0) {
            return false;
        }

        qputenv("DBUS_SESSION_BUS_ADDRESS", m_address);
        m_started = true;
        return true;
    }

    ~ScopedSessionBusDaemon()
    {
        if (m_started && m_pid > 0) {
            kill(static_cast<pid_t>(m_pid), SIGTERM);
        }

        if (m_oldHadValue) {
            qputenv("DBUS_SESSION_BUS_ADDRESS", m_oldAddress);
        } else {
            qunsetenv("DBUS_SESSION_BUS_ADDRESS");
        }
    }

private:
    QTemporaryDir m_runtimeDir;
    QByteArray m_oldAddress;
    QByteArray m_address;
    qint64 m_pid = -1;
    bool m_oldHadValue = false;
    bool m_started = false;
};

void ensureSearchConfigDirs()
{
    QDir().mkpath(QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search"));
}

bool writeFileText(const QString &path, const QByteArray &content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    return file.write(content) == content.size();
}

bool prepareDefaultAppFixtures()
{
    const QString configDir = qEnvironmentVariable("XDG_CONFIG_HOME",
                                                   QDir::homePath() + QStringLiteral("/.config"));
    const QString dataHome = qEnvironmentVariable("XDG_DATA_HOME",
                                                  QDir::homePath() + QStringLiteral("/.local/share"));
    const QString applicationsDir = dataHome + QStringLiteral("/applications");
    if (!QDir().mkpath(configDir) || !QDir().mkpath(applicationsDir)) {
        return false;
    }

    return writeFileText(configDir + QStringLiteral("/mimeapps.list"),
                         QByteArray("[Default Applications]\n")
                         + QByteArray("text/plain=ukui-search-open-file.desktop\n")
                         + QByteArray("inode/directory=ukui-search-open-dir.desktop\n"))
        && writeFileText(applicationsDir + QStringLiteral("/ukui-search-open-file.desktop"),
                         QByteArray("[Desktop Entry]\n")
                         + QByteArray("Type=Application\n")
                         + QByteArray("Name=Ukui Search Open File\n")
                         + QByteArray("Exec=/bin/true %u\n")
                         + QByteArray("MimeType=text/plain;\n")
                         + QByteArray("NoDisplay=true\n"))
        && writeFileText(applicationsDir + QStringLiteral("/ukui-search-open-dir.desktop"),
                         QByteArray("[Desktop Entry]\n")
                         + QByteArray("Type=Application\n")
                         + QByteArray("Name=Ukui Search Open Directory\n")
                         + QByteArray("Exec=/bin/true %u\n")
                         + QByteArray("MimeType=inode/directory;\n")
                         + QByteArray("NoDisplay=true\n"));
}

void addFileSearchTerms(Xapian::Document &doc, const QString &keyword)
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

void writeFileSearchDatabase(const QList<QPair<QString, QString>> &entries, const QString &keyword)
{
    Xapian::WritableDatabase database(INDEX_PATH, Xapian::DB_CREATE_OR_OVERWRITE);
    int sortKey = 0;
    for (const auto &entry : entries) {
        Xapian::Document document;
        document.set_data(entry.first.toStdString());
        document.add_value(1, entry.second.toStdString());
        document.add_value(2, QByteArray::number(++sortKey).constData());
        addFileSearchTerms(document, keyword);
        database.add_document(document);
    }
    database.commit();
}

QList<SearchPluginIface::ResultInfo> drainResults(DataQueue<SearchPluginIface::ResultInfo> &queue)
{
    QList<SearchPluginIface::ResultInfo> results;
    while (!queue.isEmpty()) {
        results.append(queue.tryDequeue());
    }
    return results;
}

void closeModalMessageBoxes()
{
    QTimer::singleShot(0, []() {
        const auto widgets = QApplication::topLevelWidgets();
        for (QWidget *widget : widgets) {
            if (auto *box = qobject_cast<QMessageBox *>(widget)) {
                box->accept();
            }
        }
    });
}

void clickActionLabel(ActionLabel *label)
{
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
                             QPointF(1, 1),
                             Qt::LeftButton,
                             Qt::LeftButton,
                             Qt::NoModifier);
    QApplication::sendEvent(label, &releaseEvent);
}

QString readFileText(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    return QString::fromLocal8Bit(file.readAll());
}

QString longDateFormat()
{
    return QStringLiteral("yyyy-yyyy-yyyy-yyyy-yyyy-yyyy-yyyy-yyyy-yyyy-yyyy-yyyy-yyyy-yyyy-yyyy-yyyy-yyyy");
}

void emitShortDateFormat(FakeDateServerService &dateService, const QString &format)
{
    Q_EMIT dateService.ShortDateSignal(format);
    QCoreApplication::processEvents();
    QTest::qWait(50);
    QCoreApplication::processEvents();
}

}

class PluginKeywordSearchWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void webKeywordSearchEnqueuesResultSynchronously();
    void aiKeywordSearchCoversTimerAndNoModelWorkerPath();
    void fileSearchWorkersCoverMissingDatabaseBranches();
    void fileContentKeywordSearchAndWorkerHandleMissingDatabase();
    void noteAndSettingsKeywordSearchHandleUnavailableServices();
    void settingsMatchCoversInMemoryAddDeleteAndSearchBranches();
    void settingsPluginFallbackLaunchAndDbusParsingBranches();
    void settingsPluginOpenActionSuccessAndMatchInitDataWithValidService();
    void notePluginCoversMetadataDetailAndSafeActions();
    void aiPluginCoversDetailActionsAndResultInfoBranches();
    void webPluginCoversSearchResultAndDefaultAppBranches();
    void fileAndDirPluginsCoverDetailPageBranches();
    void fileContentPluginCoversDetailActionsAndHelpers();
    void settingsMatchCoversXmlEnvironmentAndNonEmptyDelete();
    void noteSearchWorkerCoversInvalidInterfaceAndStoppedSymbol();
    void noteSearchWorkerParsesSingleValueAndPrivateBusPaths();
    void searchManagerCreatesResultInfoAndDirectMatches();
    void searchManagerRunsAgainstRealDatabases();
    void directSearchRunTraversesHomeAndHonorsMismatchedSymbols();
    void webPluginCoversSchemaThemeAndPrivateBusBranches();

private:
    FakePrivateDirWatcherService m_dirWatcherService;
    std::unique_ptr<ScopedDbusRegistration> m_dirWatcherRegistration;
};

void PluginKeywordSearchWhiteboxTest::initTestCase()
{
    m_dirWatcherRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.ukui.search.fileindex.service"),
                                                              QStringLiteral("/org/ukui/search/privateDirWatcher"),
                                                              &m_dirWatcherService));
    if (m_dirWatcherRegistration->isRegistered()) {
        DirWatcher::getDirWatcher();
    }

    FakeSettingsSearchService bootstrapSettingsService;
    ScopedDbusRegistration bootstrapSettingsRegistration(QStringLiteral("org.ukui.ukcc.search"),
                                                         QStringLiteral("/"),
                                                         &bootstrapSettingsService);
    if (bootstrapSettingsRegistration.isRegistered()) {
        QVERIFY(SettingsMatch::getInstance()->m_interface != nullptr);
        QVERIFY(SettingsMatch::getInstance()->m_interface->isValid());
    }
}

void PluginKeywordSearchWhiteboxTest::cleanupTestCase()
{
    QTRY_VERIFY_WITH_TIMEOUT(!SettingsMatch::getInstance()->isRunning(), 10000);
    m_dirWatcherRegistration.reset();
}

void PluginKeywordSearchWhiteboxTest::webKeywordSearchEnqueuesResultSynchronously()
{
    WebSearchPlugin plugin;
    DataQueue<SearchPluginIface::ResultInfo> queue;

    QCOMPARE(plugin.name(), QStringLiteral("Web Page"));
    QCOMPARE(plugin.description(), QStringLiteral("Web Page"));
    QCOMPARE(plugin.getPluginName(), QStringLiteral("Web Page"));
    QCOMPARE(plugin.getActioninfo(0).size(), 1);
    plugin.setEnable(false);
    QVERIFY(!plugin.isEnable());
    plugin.setEnable(true);
    QVERIFY(plugin.isEnable());
    QVERIFY(plugin.detailPage(SearchPluginIface::ResultInfo()) != nullptr);

    const size_t firstId = plugin.KeywordSearch(QStringLiteral("line1\r\nline2"), &queue);
    QVERIFY(firstId > 0);
    QVERIFY(!queue.isEmpty());
    SearchPluginIface::ResultInfo result = queue.tryDequeue();
    QCOMPARE(result.name, QStringLiteral("line1  line2"));
    QCOMPARE(result.toolTip, QStringLiteral("line1  line2"));
    QCOMPARE(result.resourceType, QStringLiteral("web"));
    QCOMPARE(result.type, 0);
    QCOMPARE(result.searchID, firstId);
    QCOMPARE(result.actionKey, QStringLiteral("line1  line2"));
    QVERIFY(!result.icon.isNull());
    QVERIFY(queue.isEmpty());

    const size_t secondId = plugin.KeywordSearch(QStringLiteral("plain keyword"), &queue);
    QCOMPARE(secondId, firstId + 1);
    result = queue.tryDequeue();
    QCOMPARE(result.name, QStringLiteral("plain keyword"));
    QCOMPARE(result.actionKey, QStringLiteral("plain keyword"));
    plugin.stopSearch();
}

void PluginKeywordSearchWhiteboxTest::aiKeywordSearchCoversTimerAndNoModelWorkerPath()
{
    AiSearchPlugin plugin;
    DataQueue<SearchPluginIface::ResultInfo> queue;

    const size_t timerId = plugin.KeywordSearch(QStringLiteral("semantic keyword"), &queue);
    QVERIFY(timerId > 0);
    QTRY_VERIFY_WITH_TIMEOUT(!plugin.m_timer->isActive(), 2000);
    plugin.m_pool.waitForDone(3000);
    plugin.stopSearch();
    QVERIFY(AiSearchPlugin::uniqueSymbolAi > timerId);
    while (!queue.isEmpty()) {
        queue.tryDequeue();
    }

    AiSearch::s_supportState = true;
    AiSearch::s_aiNotSupported = false;
    AiSearchPlugin::mutex.lock();
    const size_t workerId = ++AiSearchPlugin::uniqueSymbolAi;
    AiSearchPlugin::mutex.unlock();
    AiSearch worker(&queue, workerId, QStringLiteral("semantic keyword"));
    worker.run();
    SearchPluginIface::ResultInfo result = queue.tryDequeue();
    if (!result.name.isEmpty()) {
        QCOMPARE(result.searchID, workerId);
        QCOMPARE(result.resourceType, QStringLiteral("file"));
        if (result.type == 1) {
            QCOMPARE(result.iconName, QStringLiteral("no-model-symbolic"));
            QVERIFY(!result.showInBestMatch);
            QVERIFY(result.displayNameAsUrl);
        } else {
            QCOMPARE(result.type, 0);
            QVERIFY(!result.actionKey.isEmpty());
            QVERIFY(!result.toolTip.isEmpty());
        }
    }

    AiSearch::s_supportState = false;
    AiSearch::s_aiNotSupported = true;
    AiSearch::destroySession();
}

void PluginKeywordSearchWhiteboxTest::fileSearchWorkersCoverMissingDatabaseBranches()
{
    DataQueue<SearchPluginIface::ResultInfo> queue;

    SearchManager::uniqueSymbolFile++;
    FileSearch fileSearch(&queue,
                          SearchManager::uniqueSymbolFile,
                          QStringLiteral("not-present"),
                          FILE_SEARCH_VALUE,
                          1,
                          0,
                          1);
    fileSearch.run();
    QVERIFY(queue.isEmpty());

    SearchManager::uniqueSymbolDir++;
    FileSearch dirSearch(&queue,
                         SearchManager::uniqueSymbolDir,
                         QStringLiteral("not-present"),
                         DIR_SEARCH_VALUE,
                         1,
                         0,
                         1);
    dirSearch.run();
    QVERIFY(queue.isEmpty());

    FileMatchDecider fileDecider;
    Xapian::Document fileDoc;
    fileDoc.set_data("/not/existing/file");
    QVERIFY(fileDecider(fileDoc));
}

void PluginKeywordSearchWhiteboxTest::fileContentKeywordSearchAndWorkerHandleMissingDatabase()
{
    FileContentSearchPlugin plugin;
    DataQueue<SearchPluginIface::ResultInfo> queue;

    const size_t id = plugin.KeywordSearch(QStringLiteral("content"), &queue);
    QVERIFY(id > 0);
    plugin.m_pool.waitForDone(3000);
    plugin.stopSearch();
    QVERIFY(SearchManager::uniqueSymbolContent > id);
    while (!queue.isEmpty()) {
        queue.tryDequeue();
    }

    SearchManager::uniqueSymbolContent++;
    FileContentMatchDecider contentDecider;
    Xapian::Document contentDoc;
    contentDoc.add_value(CONTENT_DATABASE_PATH_SLOT, "/not/existing/content");
    QVERIFY(contentDecider(contentDoc));

    // Attempt content search only if database exists and isn't locked
    const QString indexPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                              + QStringLiteral("/.config/org.ukui/ukui-search/content_index_data");
    if (QFile::exists(indexPath + QStringLiteral("/iamglass"))) {
        FileContentSearch contentSearch(&queue,
                                        SearchManager::uniqueSymbolContent,
                                        QStringLiteral("content"),
                                        false,
                                        0,
                                        1);
        contentSearch.run();
        while (!queue.isEmpty()) {
            queue.tryDequeue();
        }
    }
}

void PluginKeywordSearchWhiteboxTest::noteAndSettingsKeywordSearchHandleUnavailableServices()
{
    DataQueue<SearchPluginIface::ResultInfo> queue;

    NoteSearchPlugin notePlugin;
    delete notePlugin.m_interface;
    notePlugin.m_interface = new QDBusInterface(QString(),
                                                QStringLiteral("/org/ukui/note"),
                                                QStringLiteral("org.ukui.note.interface"),
                                                QDBusConnection::sessionBus(),
                                                &notePlugin);
    const size_t noteId = notePlugin.KeywordSearch(QStringLiteral("note"), &queue);
    QVERIFY(noteId > 0);
    notePlugin.m_pool.waitForDone(3000);
    notePlugin.stopSearch();
    QVERIFY(queue.isEmpty());
}

void PluginKeywordSearchWhiteboxTest::settingsMatchCoversInMemoryAddDeleteAndSearchBranches()
{
    FakeSettingsSearchService settingsService;
    ScopedDbusRegistration settingsRegistration(QStringLiteral("org.ukui.ukcc.search"),
                                                QStringLiteral("/"),
                                                &settingsService);
    if (!settingsRegistration.isRegistered()) {
        QSKIP("failed to register fake ukcc search service");
    }

    SettingsMatch match;
    match.m_dataMap.clear();
    match.m_searchMap.clear();

    QString syntheticKey = QStringLiteral("Synthetic/Plugin");
    match.m_dataMap.insert(syntheticKey,
                           QStringList{QStringLiteral("Alpha"),
                                       QStringLiteral("本地名"),
                                       QStringLiteral("pinyin-only")});
    QString shortKeyword = QStringLiteral("y");
    QVERIFY(!match.matchDataMap(syntheticKey, shortKeyword));
    QString longKeyword = QStringLiteral("pinyin");
    QVERIFY(match.matchDataMap(syntheticKey, longKeyword));
    match.m_dataMap.clear();

    QVariantMap displayItem;
    displayItem.insert(QStringLiteral("superordinate"), QStringLiteral("Display"));
    displayItem.insert(QStringLiteral("plugin"), QStringLiteral("Monitor"));
    displayItem.insert(QStringLiteral("subEnglish"), QStringLiteral("Brightness"));
    displayItem.insert(QStringLiteral("plugin") + QLocale::system().name(), QStringLiteral("Monitor Local"));
    displayItem.insert(QStringLiteral("sub") + QLocale::system().name(), QStringLiteral("Brightness Local"));

    match.handleData(displayItem, SettingsMatch::HandleType::Add);
    QVERIFY(match.m_dataMap.contains(QStringLiteral("Display/Monitor")));
    QVERIFY(match.m_dataMap.contains(QStringLiteral("Display/Monitor/Brightness")));
    QCOMPARE(match.m_searchMap.value(QStringLiteral("Display")).value(QStringLiteral("Monitor")).size(), 1);

    QString secondLevelKey = QStringLiteral("Display/Monitor");
    QString thirdLevelKey = QStringLiteral("Display/Monitor/Brightness");
    QString keyword = QStringLiteral("Monitor");
    QVERIFY(match.matchDataMap(secondLevelKey, keyword));

    DataQueue<SearchPluginIface::ResultInfo> secondLevelQueue;
    SettingsSearchPlugin::m_uniqueSymbolForSettings = 100;
    match.startSearch(keyword, 100, &secondLevelQueue);
    QVERIFY(!secondLevelQueue.isEmpty());
    while (!secondLevelQueue.isEmpty()) {
        const SearchPluginIface::ResultInfo result = secondLevelQueue.tryDequeue();
        QCOMPARE(result.searchID, static_cast<size_t>(100));
        QCOMPARE(result.resourceType, QStringLiteral("setting"));
        QVERIFY(result.name.contains(QStringLiteral("Monitor")) || result.name.contains(QStringLiteral("Local")));
    }

    keyword = QStringLiteral("Brightness");
    QVERIFY(match.matchDataMap(thirdLevelKey, keyword));

    SearchPluginIface::ResultInfo resultInfo;
    match.createResultInfo(resultInfo, match.m_dataMap.value(thirdLevelKey), thirdLevelKey);
    QCOMPARE(resultInfo.resourceType, QStringLiteral("setting"));
    QCOMPARE(resultInfo.actionKey, QStringLiteral("Monitor"));
    QVERIFY(resultInfo.name.contains(QStringLiteral("Monitor")) || resultInfo.name.contains(QStringLiteral("Local")));

    DataQueue<SearchPluginIface::ResultInfo> queue;
    QVERIFY(match.enqueueResultInfo(resultInfo, 100, &queue));
    QCOMPARE(queue.tryDequeue().searchID, static_cast<size_t>(100));
    QVERIFY(!match.enqueueResultInfo(resultInfo, 101, &queue));
    QVERIFY(queue.isEmpty());

    keyword = QStringLiteral("Brightness");
    match.startSearch(keyword, 100, &queue);
    QVERIFY(!queue.isEmpty());
    while (!queue.isEmpty()) {
        const SearchPluginIface::ResultInfo result = queue.tryDequeue();
        QCOMPARE(result.searchID, static_cast<size_t>(100));
        QCOMPARE(result.resourceType, QStringLiteral("setting"));
    }

    QString slash = QStringLiteral("/");
    match.startSearch(slash, 100, &queue);
    QVERIFY(queue.isEmpty());

    match.handleData(displayItem, SettingsMatch::HandleType::Delete);
    QVERIFY(!match.m_dataMap.contains(QStringLiteral("Display/Monitor/Brightness")));
    QVERIFY(!match.m_dataMap.contains(QStringLiteral("Display/Monitor")));

    SettingsSearch settingsSearch(&queue, QStringLiteral("Brightness"), 100);
    settingsSearch.run();
}

void PluginKeywordSearchWhiteboxTest::settingsPluginFallbackLaunchAndDbusParsingBranches()
{
    FakeSettingsSearchService settingsService;
    ScopedDbusRegistration settingsRegistration(QStringLiteral("org.ukui.ukcc.search"),
                                                QStringLiteral("/"),
                                                &settingsService);
    if (!settingsRegistration.isRegistered()) {
        QSKIP("failed to register fake ukcc search service");
    }

    SettingsSearchPlugin plugin;
    SearchPluginIface::ResultInfo shortResult;
    shortResult.name = QStringLiteral("Sound");
    shortResult.actionKey = QStringLiteral("Sound");
    QCOMPARE(plugin.detailPage(shortResult), plugin.m_detailPage);
    QCOMPARE(plugin.m_nameLabel->toolTip(), QString());

    const QIcon themeIcon = QIcon::fromTheme(QStringLiteral("applications-system"));
    shortResult.icon = themeIcon;
    plugin.detailPage(shortResult);
    Q_EMIT GlobalSettings::getInstance().valueChanged(ICON_THEME_KEY, QStringLiteral("whitebox"));
    QCoreApplication::processEvents();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString launcherPath = dir.filePath(QStringLiteral("ukui-control-center"));
    const QString outputPath = QStringLiteral("/var/tmp/settings-launch-whitebox.txt");
    QFile::remove(outputPath);
    QFile launcher(launcherPath);
    QVERIFY(launcher.open(QIODevice::WriteOnly | QIODevice::Truncate));
    launcher.write("#!/bin/sh\n");
    launcher.write("printf '%s ' \"$@\" > /var/tmp/settings-launch-whitebox.txt\n");
    launcher.close();
    QVERIFY(QFile::setPermissions(launcherPath,
                                  QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));

    ScopedEnvVar pathEnv("PATH", QFile::encodeName(dir.path() + QStringLiteral(":") + qEnvironmentVariable("PATH")));

    BrokenAppLauncher brokenLauncher;
    ScopedDbusRegistration registration(QStringLiteral("com.kylin.ProcessManager"),
                                        QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                        &brokenLauncher);
    plugin.openAction(0, QStringLiteral("Display"), 0);
    if (registration.isRegistered()) {
        QTRY_VERIFY_WITH_TIMEOUT(QFile::exists(outputPath), 3000);
        const QString launchedArgs = readFileText(outputPath);
        QVERIFY(launchedArgs.contains(QStringLiteral("-m ")));
        QVERIFY(launchedArgs.contains(QStringLiteral("display")));
    }

    QTRY_VERIFY_WITH_TIMEOUT(!SettingsMatch::getInstance()->isRunning(), 10000);
}

void PluginKeywordSearchWhiteboxTest::settingsPluginOpenActionSuccessAndMatchInitDataWithValidService()
{
    FakeAppLauncher launcher;
    ScopedDbusRegistration launcherRegistration(QStringLiteral("com.kylin.ProcessManager"),
                                                QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                                &launcher);
    FakeSettingsSearchService settingsService;
    ScopedDbusRegistration settingsRegistration(QStringLiteral("org.ukui.ukcc.search"),
                                                QStringLiteral("/"),
                                                &settingsService);

    SettingsMatch match;
    QVERIFY(match.m_interface != nullptr);
    if (settingsRegistration.isRegistered()) {
        QVERIFY(match.m_interface->isValid());
        match.initData();
        QVERIFY(match.m_dataMap.isEmpty());
        match.addItem(QVariantMap{});
        match.removeItem(QVariantMap{});
    }

    SettingsSearchPlugin plugin;
    plugin.openAction(0, QStringLiteral("Display"), 0);
    if (launcherRegistration.isRegistered()) {
        QCOMPARE(launcher.lastDesktopFile, QStringLiteral("/usr/share/applications/ukui-control-center.desktop"));
        QCOMPARE(launcher.lastArguments, (QStringList{QStringLiteral("-m"), QStringLiteral("display")}));
        QCOMPARE(launcher.launchAppCalls, 1);
    }

    QTRY_VERIFY_WITH_TIMEOUT(!SettingsMatch::getInstance()->isRunning(), 10000);
}

void PluginKeywordSearchWhiteboxTest::notePluginCoversMetadataDetailAndSafeActions()
{
    NoteSearchPlugin plugin;

    QCOMPARE(plugin.name(), QStringLiteral("Note Search"));
    QCOMPARE(plugin.description(), QStringLiteral("Note Search."));
    QCOMPARE(plugin.getPluginName(), QStringLiteral("Note Search"));
    QCOMPARE(plugin.pluginType(), SearchPluginIface::PluginType::SearchPlugin);
    QVERIFY(plugin.isEnable());
    plugin.setEnable(false);
    QVERIFY(!plugin.isEnable());
    plugin.setEnable(true);
    QVERIFY(plugin.isEnable());
    QCOMPARE(plugin.getActioninfo(0).size(), 1);

    plugin.openAction(999, QStringLiteral("ignored"), 0);

    SearchPluginIface::ResultInfo result;
    result.icon = QIcon();
    result.name = QString(300, QLatin1Char('n'));
    result.actionKey = QStringLiteral("note-key");
    result.description = QVector<SearchPluginIface::DescriptionInfo>()
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Note Description:"), QStringLiteral("alice note body")};

    plugin.m_keyword = QStringLiteral("alice");
    QWidget *page = plugin.detailPage(result);
    QCOMPARE(page, plugin.m_detailPage);
    QCOMPARE(plugin.m_currentActionKey, QStringLiteral("note-key"));
    QCOMPARE(plugin.m_pluginLabel->text(), QStringLiteral("Application"));
    QVERIFY(plugin.m_descFrame->isVisible() || !plugin.m_descFrame->isHidden());
    QVERIFY(plugin.m_descLabel->text().contains(QStringLiteral("alice"), Qt::CaseInsensitive));
    QVERIFY(!plugin.m_nameLabel->toolTip().isEmpty());

    SearchPluginIface::ResultInfo shortResult = result;
    shortResult.name = QStringLiteral("short note");
    shortResult.actionKey = QStringLiteral("short-note-key");
    shortResult.description = QVector<SearchPluginIface::DescriptionInfo>()
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Note Description:"), QStringLiteral("plain body")};
    QCOMPARE(plugin.detailPage(shortResult), plugin.m_detailPage);
    QCOMPARE(plugin.m_currentActionKey, QStringLiteral("short-note-key"));
    QCOMPARE(plugin.m_nameLabel->toolTip(), QString());

    Q_EMIT GlobalSettings::getInstance().valueChanged(ICON_THEME_KEY, QStringLiteral("whitebox"));
    QCoreApplication::processEvents();

    FakeAppLauncher launcher;
    ScopedDbusRegistration registration(QStringLiteral("com.kylin.ProcessManager"),
                                        QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                        &launcher);
    plugin.openAction(0, shortResult.actionKey, 0);
    if (registration.isRegistered()) {
        const QStringList expectedArguments{QStringLiteral("--show"), shortResult.actionKey};
        QCOMPARE(launcher.launchAppCalls, 1);
        QCOMPARE(launcher.lastDesktopFile, QStringLiteral("/usr/share/applications/ukui-notebook.desktop"));
        QCOMPARE(launcher.lastArguments, expectedArguments);
    }
    clickActionLabel(plugin.m_actionLabel1);
    if (registration.isRegistered()) {
        QCOMPARE(launcher.launchAppCalls, 2);
    }

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString launcherPath = dir.filePath(QStringLiteral("ukui-notebook"));
    const QString outputPath = dir.filePath(QStringLiteral("note-launch.txt"));
    QFile launcherScript(launcherPath);
    QVERIFY(launcherScript.open(QIODevice::WriteOnly | QIODevice::Truncate));
    launcherScript.write("#!/bin/sh\n");
    launcherScript.write("printf '%s ' \"$@\" > \"$NOTE_PLUGIN_OUT\"\n");
    launcherScript.close();
    QVERIFY(QFile::setPermissions(launcherPath,
                                  QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));
    ScopedEnvVar pathEnv("PATH", QFile::encodeName(dir.path() + QStringLiteral(":") + qEnvironmentVariable("PATH")));
    ScopedEnvVar outEnv("NOTE_PLUGIN_OUT", QFile::encodeName(outputPath));

    BrokenAppLauncher brokenLauncher;
    ScopedDbusRegistration brokenRegistration(QStringLiteral("com.kylin.ProcessManager"),
                                              QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                              &brokenLauncher);
    plugin.openAction(0, shortResult.actionKey, 0);
    if (brokenRegistration.isRegistered()) {
        QTRY_VERIFY_WITH_TIMEOUT(QFile::exists(outputPath), 3000);
        const QString launchedArgs = readFileText(outputPath);
        QVERIFY(launchedArgs.contains(QStringLiteral("--show ")));
        QVERIFY(launchedArgs.contains(shortResult.actionKey));
    }

    plugin.stopSearch();
}

void PluginKeywordSearchWhiteboxTest::aiPluginCoversDetailActionsAndResultInfoBranches()
{
    AiSearchPlugin plugin;

    QCOMPARE(plugin.name(), QStringLiteral("Ai Search"));
    QCOMPARE(plugin.description(), QStringLiteral("Content-Aware Search"));
    QCOMPARE(plugin.getPluginName(), QStringLiteral("Content-Aware Search"));
    QCOMPARE(plugin.pluginType(), SearchPluginIface::PluginType::SearchPlugin);
    QCOMPARE(plugin.getActioninfo(0).size(), 3);
    plugin.setEnable(false);
    QVERIFY(!plugin.isEnable());
    plugin.setEnable(true);
    QVERIFY(plugin.isEnable());

    plugin.openAction(999, QStringLiteral("ignored"), 0);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QTemporaryFile file(dir.path() + QStringLiteral("/ai-result-XXXXXX.txt"));
    QVERIFY(file.open());
    file.write("ai result");
    file.close();

    DataQueue<SearchPluginIface::ResultInfo> queue;
    AiSearch search(&queue, 1, QStringLiteral("ai"));
    SearchPluginIface::ResultInfo result;
    QVERIFY(search.createResultInfo(result, file.fileName()));
    QCOMPARE(result.resourceType, QStringLiteral("file"));
    QCOMPARE(result.actionKey, file.fileName());
    QCOMPARE(result.description.size(), 2);
    QVERIFY(!result.name.isEmpty());

    SearchPluginIface::ResultInfo missing;
    QVERIFY(!search.createResultInfo(missing, dir.path() + QStringLiteral("/missing.txt")));

    // SDK- and D-Bus-bound AI branches are already covered deterministically in
    // aiSearchPluginWhiteboxTest. Keep the aggregate test local-only so it does
    // not wait on real FileManager1 or data-management services.
    AiSearch::destroySession();
}

void PluginKeywordSearchWhiteboxTest::webPluginCoversSearchResultAndDefaultAppBranches()
{
    WebSearchPlugin plugin;
    DataQueue<SearchPluginIface::ResultInfo> queue;
    FakeAppLauncher launcher;
    ScopedDbusRegistration registration(QStringLiteral("com.kylin.ProcessManager"),
                                        QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                        &launcher);

    SearchPluginIface::ResultInfo result;
    result.name = QStringLiteral("line1 line2");
    result.actionKey = result.name;
    result.resourceType = QStringLiteral("web");
    result.type = 0;
    result.searchID = 1;
    QVERIFY(!result.name.contains(QLatin1Char('\r')));
    QVERIFY(!result.name.contains(QLatin1Char('\n')));
    QVERIFY(plugin.detailPage(result) != nullptr);
    QCOMPARE(plugin.getDefaultAppId("x-ukui-search-whitebox-missing-type"), QString());
    Q_EMIT GlobalSettings::getInstance().valueChanged(STYLE_NAME_KEY, QStringLiteral("ukui-dark"));
    QCoreApplication::processEvents();
    clickActionLabel(plugin.m_actionLabel1);

    plugin.m_webEngine.clear();
    plugin.KeywordSearch(QStringLiteral("default engine"), &queue);
    plugin.openAction(0, QStringLiteral("ignored"), 0);
    if (registration.isRegistered()) {
        QCOMPARE(launcher.lastUrl, QStringLiteral("https://baidu.com/s?word=default engine"));
    }

    plugin.m_webEngine = QStringLiteral("360");
    plugin.KeywordSearch(QStringLiteral("engine 360"), &queue);
    plugin.openAction(0, QStringLiteral("ignored"), 0);
    if (registration.isRegistered()) {
        QCOMPARE(launcher.lastUrl, QStringLiteral("https://www.so.com/s?q=engine 360"));
    }

    plugin.m_webEngine = QStringLiteral("sougou");
    plugin.KeywordSearch(QStringLiteral("engine sougou"), &queue);
    plugin.openAction(0, QStringLiteral("ignored"), 0);
    if (registration.isRegistered()) {
        QCOMPARE(launcher.lastUrl, QStringLiteral("https://www.sogou.com/web?query=engine sougou"));
    }

    plugin.m_webEngine = QStringLiteral("unknown");
    plugin.KeywordSearch(QStringLiteral("engine fallback"), &queue);
    plugin.openAction(0, QStringLiteral("ignored"), 0);
    if (registration.isRegistered()) {
        QCOMPARE(launcher.lastUrl, QStringLiteral("https://baidu.com/s?word=engine fallback"));
        QVERIFY(launcher.launchUrlCalls >= 4);
    }

    while (!queue.isEmpty()) {
        queue.tryDequeue();
    }
}

void PluginKeywordSearchWhiteboxTest::fileAndDirPluginsCoverDetailPageBranches()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString longFilePath = tempDir.filePath(QStringLiteral("long-file.txt"));
    const QString shortFilePath = tempDir.filePath(QStringLiteral("short.txt"));
    const QString noDefaultFilePath = tempDir.filePath(QStringLiteral("no-default.bin"));
    const QString longDirPath = tempDir.filePath(QStringLiteral("long-dir"));
    const QString shortDirPath = tempDir.filePath(QStringLiteral("short-dir"));
    QVERIFY(writeFileText(longFilePath, QByteArray("long file")));
    QVERIFY(writeFileText(shortFilePath, QByteArray("short file")));
    QVERIFY(writeFileText(noDefaultFilePath, QByteArray::fromHex("00010203040506070809aabbccddeeff")));
    QVERIFY(QDir().mkpath(longDirPath));
    QVERIFY(QDir().mkpath(shortDirPath));
    QVERIFY(prepareDefaultAppFixtures());

    FakeAppLauncher launcher;
    FakeFileManager1Service fileManager;
    FakeDateServerService dateService;
    ScopedDbusRegistration launcherRegistration(QStringLiteral("com.kylin.ProcessManager"),
                                                QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                                &launcher);
    ScopedDbusRegistration fileManagerRegistration(QStringLiteral("org.freedesktop.FileManager1"),
                                                   QStringLiteral("/org/freedesktop/FileManager1"),
                                                   &fileManager);
    ScopedDbusRegistration dateRegistration(QStringLiteral("com.kylin.kysdk.DateServer"),
                                            QStringLiteral("/com/kylin/kysdk/Date"),
                                            &dateService);
    QVERIFY(launcherRegistration.isRegistered());
    QVERIFY(fileManagerRegistration.isRegistered());
    QVERIFY(dateRegistration.isRegistered());

    FileSearchPlugin filePlugin;
    QCOMPARE(filePlugin.name(), QStringLiteral("File Search"));
    QCOMPARE(filePlugin.description(), QStringLiteral("File search."));
    QCOMPARE(filePlugin.getPluginName(), QStringLiteral("File Search"));
    QCOMPARE(filePlugin.pluginType(), SearchPluginIface::PluginType::SearchPlugin);
    QCOMPARE(filePlugin.getActioninfo(0).size(), 3);
    filePlugin.setEnable(false);
    QVERIFY(!filePlugin.isEnable());
    filePlugin.setEnable(true);
    QVERIFY(filePlugin.isEnable());

    SearchPluginIface::ResultInfo longFile;
    longFile.name = QString(260, QLatin1Char('f'));
    longFile.actionKey = longFilePath;
    longFile.description = QVector<SearchPluginIface::DescriptionInfo>()
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Path:"), longFile.actionKey}
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Modified time:"), QStringLiteral("2024/01/02 03:04:05")};
    QCOMPARE(filePlugin.detailPage(longFile), filePlugin.m_detailPage);
    QCOMPARE(filePlugin.m_pluginLabel->text(), QStringLiteral("File"));
    QVERIFY(!filePlugin.m_nameLabel->toolTip().isEmpty());
    QVERIFY(!filePlugin.m_timeLabel2->text().isEmpty());
    Q_EMIT GlobalSettings::getInstance().valueChanged(FONT_SIZE_KEY, 12);
    QCoreApplication::processEvents();
    QVERIFY(!filePlugin.m_nameLabel->toolTip().isEmpty());

    emitShortDateFormat(dateService, longDateFormat());
    QCOMPARE(filePlugin.detailPage(longFile), filePlugin.m_detailPage);
    QVERIFY(!filePlugin.m_timeLabel2->toolTip().isEmpty());

    SearchPluginIface::ResultInfo shortFile = longFile;
    shortFile.name = QStringLiteral("short.txt");
    shortFile.actionKey = shortFilePath;
    QCOMPARE(filePlugin.detailPage(shortFile), filePlugin.m_detailPage);
    QCOMPARE(filePlugin.m_nameLabel->toolTip(), QString());
    Q_EMIT GlobalSettings::getInstance().valueChanged(ICON_THEME_KEY, QStringLiteral("whitebox"));
    Q_EMIT GlobalSettings::getInstance().valueChanged(FONT_KEY, QStringLiteral("Sans"));
    QCoreApplication::processEvents();

    auto *config = FileIndexerConfig::getInstance();
    auto setFileIndexEnabled = [config](bool enabled) {
        if (config->m_gsettings && config->m_gsettings->keys().contains(QStringLiteral("fileIndexEnable"))) {
            config->m_gsettings->set(QStringLiteral("fileIndexEnable"), enabled);
        }
    };
    ScopedIndexableDirs scopedIndexableDirs(m_dirWatcherService, QStringList{tempDir.path()});

    DataQueue<SearchPluginIface::ResultInfo> directFileQueue;
    setFileIndexEnabled(false);
    const size_t directFileSearchId = filePlugin.KeywordSearch(QStringLiteral("short"), &directFileQueue);
    QTRY_VERIFY_WITH_TIMEOUT(filePlugin.m_pool.activeThreadCount() == 0, 5000);
    QCOMPARE(directFileSearchId, SearchManager::uniqueSymbolFile);
    QVERIFY(!directFileQueue.isEmpty());

    DataQueue<SearchPluginIface::ResultInfo> indexedFileQueue;
    setFileIndexEnabled(true);
    const size_t indexedFileSearchId = filePlugin.KeywordSearch(QStringLiteral("missing-index"), &indexedFileQueue);
    QTRY_VERIFY_WITH_TIMEOUT(filePlugin.m_pool.activeThreadCount() == 0, 5000);
    QCOMPARE(indexedFileSearchId, SearchManager::uniqueSymbolFile);

    filePlugin.openAction(999, shortFile.actionKey, 0);
    filePlugin.openAction(1, shortFile.actionKey, 0);
    filePlugin.openAction(2, shortFile.actionKey, 0);
    QCOMPARE(QApplication::clipboard()->text(), shortFile.actionKey);
    QCOMPARE(fileManager.lastItems, QStringList{shortFile.actionKey});
    QCOMPARE(fileManager.lastStartupId, QStringLiteral("ukui-search"));
    closeModalMessageBoxes();
    filePlugin.openAction(0, longFile.actionKey, 0);
    QCOMPARE(launcher.lastUrl, QUrl::fromLocalFile(longFile.actionKey).toString());
    closeModalMessageBoxes();

    SearchPluginIface::ResultInfo noDefaultFile = shortFile;
    noDefaultFile.actionKey = noDefaultFilePath;
    noDefaultFile.name = QStringLiteral("no-default.bin");
    noDefaultFile.description[0].value = noDefaultFile.actionKey;
    QCOMPARE(filePlugin.detailPage(noDefaultFile), filePlugin.m_detailPage);
    closeModalMessageBoxes();
    filePlugin.openAction(0, noDefaultFile.actionKey, 0);
    closeModalMessageBoxes();
    clickActionLabel(filePlugin.m_actionLabel1);

    QCOMPARE(filePlugin.detailPage(shortFile), filePlugin.m_detailPage);
    clickActionLabel(filePlugin.m_actionLabel1);
    clickActionLabel(filePlugin.m_actionLabel2);
    clickActionLabel(filePlugin.m_actionLabel3);
    QCOMPARE(QApplication::clipboard()->text(), shortFile.actionKey);
    QVERIFY(launcher.launchUrlCalls >= 2);
    filePlugin.stopSearch();

    DirSearchPlugin dirPlugin;
    QCOMPARE(dirPlugin.name(), QStringLiteral("Dir Search"));
    QCOMPARE(dirPlugin.description(), QStringLiteral("Dir search."));
    QCOMPARE(dirPlugin.getPluginName(), QStringLiteral("Dir Search"));
    QCOMPARE(dirPlugin.pluginType(), SearchPluginIface::PluginType::SearchPlugin);
    QCOMPARE(dirPlugin.getActioninfo(0).size(), 3);
    dirPlugin.setEnable(false);
    QVERIFY(!dirPlugin.isEnable());
    dirPlugin.setEnable(true);
    QVERIFY(dirPlugin.isEnable());

    SearchPluginIface::ResultInfo longDir = longFile;
    longDir.name = QString(260, QLatin1Char('d'));
    longDir.actionKey = longDirPath;
    QCOMPARE(dirPlugin.detailPage(longDir), dirPlugin.m_detailPage);
    QCOMPARE(dirPlugin.m_pluginLabel->text(), QStringLiteral("Directory"));
    QVERIFY(!dirPlugin.m_nameLabel->toolTip().isEmpty());
    Q_EMIT GlobalSettings::getInstance().valueChanged(FONT_SIZE_KEY, 12);
    QCoreApplication::processEvents();
    QVERIFY(!dirPlugin.m_nameLabel->toolTip().isEmpty());
    QCOMPARE(dirPlugin.detailPage(longDir), dirPlugin.m_detailPage);
    QVERIFY(!dirPlugin.m_timeLabel2->toolTip().isEmpty());
    emitShortDateFormat(dateService, QStringLiteral("yyyy/MM/dd"));

    SearchPluginIface::ResultInfo shortDir = longFile;
    shortDir.name = QStringLiteral("short-dir");
    shortDir.actionKey = shortDirPath;
    QCOMPARE(dirPlugin.detailPage(shortDir), dirPlugin.m_detailPage);
    QCOMPARE(dirPlugin.m_nameLabel->toolTip(), QString());
    Q_EMIT GlobalSettings::getInstance().valueChanged(ICON_THEME_KEY, QStringLiteral("whitebox"));
    QCoreApplication::processEvents();

    DataQueue<SearchPluginIface::ResultInfo> directDirQueue;
    setFileIndexEnabled(false);
    const size_t directDirSearchId = dirPlugin.KeywordSearch(QStringLiteral("short-dir"), &directDirQueue);
    QTRY_VERIFY_WITH_TIMEOUT(dirPlugin.m_pool.activeThreadCount() == 0, 5000);
    QCOMPARE(directDirSearchId, SearchManager::uniqueSymbolDir);
    QVERIFY(!directDirQueue.isEmpty());

    DataQueue<SearchPluginIface::ResultInfo> indexedDirQueue;
    setFileIndexEnabled(true);
    const size_t indexedDirSearchId = dirPlugin.KeywordSearch(QStringLiteral("missing-index"), &indexedDirQueue);
    QTRY_VERIFY_WITH_TIMEOUT(dirPlugin.m_pool.activeThreadCount() == 0, 5000);
    QCOMPARE(indexedDirSearchId, SearchManager::uniqueSymbolDir);

    dirPlugin.openAction(999, shortDir.actionKey, 0);
    dirPlugin.openAction(0, shortDir.actionKey, 0);
    dirPlugin.openAction(1, shortDir.actionKey, 0);
    dirPlugin.openAction(2, shortDir.actionKey, 0);
    QCOMPARE(QApplication::clipboard()->text(), shortDir.actionKey);
    QCOMPARE(fileManager.lastItems, QStringList{shortDir.actionKey});
    clickActionLabel(dirPlugin.m_actionLabel1);
    clickActionLabel(dirPlugin.m_actionLabel2);
    clickActionLabel(dirPlugin.m_actionLabel3);
    QCOMPARE(QApplication::clipboard()->text(), shortDir.actionKey);
    QVERIFY(launcher.launchUrlCalls >= 4);
    dirPlugin.stopSearch();
}

void PluginKeywordSearchWhiteboxTest::fileContentPluginCoversDetailActionsAndHelpers()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString textFilePath = tempDir.filePath(QStringLiteral("content.txt"));
    const QString noDefaultFilePath = tempDir.filePath(QStringLiteral("content-no-default.bin"));
    QVERIFY(writeFileText(textFilePath, QByteArray("content body")));
    QVERIFY(writeFileText(noDefaultFilePath, QByteArray::fromHex("00112233445566778899aabbccddeeff")));
    QVERIFY(prepareDefaultAppFixtures());

    FakeAppLauncher launcher;
    FakeFileManager1Service fileManager;
    FakeDateServerService dateService;
    ScopedDbusRegistration launcherRegistration(QStringLiteral("com.kylin.ProcessManager"),
                                                QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                                &launcher);
    ScopedDbusRegistration fileManagerRegistration(QStringLiteral("org.freedesktop.FileManager1"),
                                                   QStringLiteral("/org/freedesktop/FileManager1"),
                                                   &fileManager);
    ScopedDbusRegistration dateRegistration(QStringLiteral("com.kylin.kysdk.DateServer"),
                                            QStringLiteral("/com/kylin/kysdk/Date"),
                                            &dateService);
    QVERIFY(launcherRegistration.isRegistered());
    QVERIFY(fileManagerRegistration.isRegistered());
    QVERIFY(dateRegistration.isRegistered());

    FileContentSearchPlugin plugin;

    QCOMPARE(plugin.name(), QStringLiteral("File Content Search"));
    QCOMPARE(plugin.description(), QStringLiteral("File content search."));
    QCOMPARE(plugin.getPluginName(), QStringLiteral("File content search"));
    QCOMPARE(plugin.pluginType(), SearchPluginIface::PluginType::SearchPlugin);
    QCOMPARE(plugin.getActioninfo(0).size(), 3);
    plugin.setEnable(false);
    QVERIFY(!plugin.isEnable());
    plugin.setEnable(true);
    QVERIFY(plugin.isEnable());
    const QIcon pluginIcon = plugin.icon();
    QVERIFY(pluginIcon.isNull() || !pluginIcon.name().isEmpty() || !pluginIcon.availableSizes().isEmpty());

    auto *config = FileIndexerConfig::getInstance();
    auto setContentIndexEnabled = [config](bool enabled) {
        if (config->m_gsettings && config->m_gsettings->keys().contains(QStringLiteral("contentIndexEnable"))) {
            config->m_gsettings->set(QStringLiteral("contentIndexEnable"), enabled);
        }
    };
    if (config->m_gsettings && config->m_gsettings->keys().contains(QStringLiteral("contentFuzzySearch"))) {
        config->m_gsettings->set(QStringLiteral("contentFuzzySearch"), true);
    }
    DataQueue<SearchPluginIface::ResultInfo> disabledContentQueue;
    setContentIndexEnabled(false);
    const size_t disabledContentSearchId = plugin.KeywordSearch(QStringLiteral("disabled content"), &disabledContentQueue);
    QTRY_VERIFY_WITH_TIMEOUT(plugin.m_pool.activeThreadCount() == 0, 5000);
    QCOMPARE(disabledContentSearchId, SearchManager::uniqueSymbolContent);

    DataQueue<SearchPluginIface::ResultInfo> indexedContentQueue;
    setContentIndexEnabled(true);
    const size_t indexedContentSearchId = plugin.KeywordSearch(QStringLiteral("missing content index"), &indexedContentQueue);
    QTRY_VERIFY_WITH_TIMEOUT(plugin.m_pool.activeThreadCount() == 0, 5000);
    QCOMPARE(indexedContentSearchId, SearchManager::uniqueSymbolContent);

    SearchPluginIface::ResultInfo textResult;
    textResult.name = QString(260, QLatin1Char('c'));
    textResult.actionKey = textFilePath;
    textResult.type = 0;
    textResult.description = QVector<SearchPluginIface::DescriptionInfo>()
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Snippet"), QStringLiteral("keyword line one")}
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Path"), textResult.actionKey}
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Modified time"), QStringLiteral("2024/01/02 03:04:05")};

    QCOMPARE(plugin.detailPage(textResult), plugin.m_detailPage);
    QCOMPARE(plugin.m_pluginLabel->text(), QStringLiteral("File"));
    QVERIFY(plugin.m_snippetLabel->isVisible() || !plugin.m_snippetLabel->isHidden());
    QCOMPARE(plugin.m_snippetLabel->text(), QStringLiteral("keyword line one"));
    QVERIFY(!plugin.m_nameLabel->toolTip().isEmpty());
    QVERIFY(!plugin.m_timeLabel2->text().isEmpty());
    Q_EMIT GlobalSettings::getInstance().valueChanged(FONT_KEY, QStringLiteral("Sans"));
    QCoreApplication::processEvents();
    QVERIFY(!plugin.m_nameLabel->toolTip().isEmpty());
    emitShortDateFormat(dateService, longDateFormat());
    QCOMPARE(plugin.detailPage(textResult), plugin.m_detailPage);
    QVERIFY(!plugin.m_timeLabel2->toolTip().isEmpty());
    emitShortDateFormat(dateService, QStringLiteral("yyyy/MM/dd"));

    SearchPluginIface::ResultInfo sameKeyResult = textResult;
    sameKeyResult.name = QStringLiteral("content.txt");
    sameKeyResult.description[0].value = QStringLiteral("second snippet");
    QCOMPARE(plugin.detailPage(sameKeyResult), plugin.m_detailPage);
    QCOMPARE(plugin.m_nameLabel->toolTip(), QString());
    QCOMPARE(plugin.m_snippetLabel->text(), QStringLiteral("second snippet"));

    const QString html = plugin.getHtmlText(QStringLiteral("Ab<c>\nxy"), QStringLiteral("ac"));
    QVERIFY(html.contains(QStringLiteral("<b><font size=\"4\">")));
    QVERIFY(html.contains(QStringLiteral("<br />")));
    const QString noBold = plugin.getHtmlText(QStringLiteral("plain text"), QStringLiteral("zz"));
    QVERIFY(!noBold.contains(QStringLiteral("<b><font size=\"4\">")));

    const QString wrappedLong = plugin.wrapData(plugin.m_snippetLabel, QString(260, QLatin1Char('w')));
    QVERIFY(wrappedLong.contains(QLatin1Char('\n')) || wrappedLong.size() >= 260);
    QCOMPARE(plugin.wrapData(plugin.m_snippetLabel, QStringLiteral("short")), QStringLiteral("short"));

    Q_EMIT GlobalSettings::getInstance().valueChanged(ICON_THEME_KEY, QStringLiteral("whitebox"));
    Q_EMIT GlobalSettings::getInstance().valueChanged(FONT_SIZE_KEY, 13);
    QCoreApplication::processEvents();

    SearchPluginIface::ResultInfo ocrResult = textResult;
    ocrResult.actionKey = QStringLiteral("/tmp/ukui-search-whitebox-ocr-missing.png");
    ocrResult.type = 1;
    ocrResult.name = QStringLiteral("ocr.png");
    ocrResult.description[0].value = QStringLiteral("ocr snippet");
    QCOMPARE(plugin.detailPage(ocrResult), plugin.m_detailPage);
    plugin.m_thumbnailPool.waitForDone();
    QCoreApplication::processEvents();
    QCOMPARE(plugin.m_pluginLabel->text(), QStringLiteral("OCR"));
    QVERIFY(plugin.m_snippetLabel->isHidden());
    QVERIFY(!plugin.m_useImage);

    plugin.openAction(999, textResult.actionKey, 0);
    plugin.openAction(1, textResult.actionKey, 0);
    plugin.openAction(2, textResult.actionKey, 0);
    QCOMPARE(QApplication::clipboard()->text(), textResult.actionKey);
    QCOMPARE(fileManager.lastItems, QStringList{textResult.actionKey});
    closeModalMessageBoxes();
    plugin.openAction(0, textResult.actionKey, 0);
    QCOMPARE(launcher.lastUrl, QUrl::fromLocalFile(textResult.actionKey).toString());

    SearchPluginIface::ResultInfo noDefaultContent = textResult;
    noDefaultContent.actionKey = noDefaultFilePath;
    noDefaultContent.name = QStringLiteral("content-no-default.bin");
    noDefaultContent.description[1].value = noDefaultContent.actionKey;
    QCOMPARE(plugin.detailPage(noDefaultContent), plugin.m_detailPage);
    closeModalMessageBoxes();
    plugin.openAction(0, noDefaultContent.actionKey, 0);

    QCOMPARE(plugin.detailPage(textResult), plugin.m_detailPage);
    closeModalMessageBoxes();
    QCOMPARE(plugin.detailPage(noDefaultContent), plugin.m_detailPage);
    closeModalMessageBoxes();
    clickActionLabel(plugin.m_actionLabel1);
    QCOMPARE(plugin.detailPage(textResult), plugin.m_detailPage);
    clickActionLabel(plugin.m_actionLabel1);
    clickActionLabel(plugin.m_actionLabel2);
    clickActionLabel(plugin.m_actionLabel3);
    QCOMPARE(QApplication::clipboard()->text(), textResult.actionKey);
    QVERIFY(launcher.launchUrlCalls >= 2);
    plugin.stopSearch();

}

void PluginKeywordSearchWhiteboxTest::settingsMatchCoversXmlEnvironmentAndNonEmptyDelete()
{
    SettingsMatch *match = SettingsMatch::getInstance();
    match->m_dataMap.clear();
    match->m_searchMap.clear();

    qputenv("XDG_SESSION_TYPE", "wayland");
    QDomDocument doc;
    QVERIFY(doc.setContent(QStringLiteral(
        "<Root>"
        "<Item name=\"Display\">"
        "<Environment>v101</Environment>"
        "<EnglishFunc1>Screen</EnglishFunc1>"
        "<EnglishFunc2>Resolution</EnglishFunc2>"
        "<ChinesePlugin>显示</ChinesePlugin>"
        "<ChineseFunc1>屏幕</ChineseFunc1>"
        "<ChineseFunc2>分辨率</ChineseFunc2>"
        "</Item>"
        "</Root>")));
    QVERIFY(!match->matchCommonEnvironment(doc.elementsByTagName(QStringLiteral("Environment")).at(0)));
    match->matchNodes(doc.documentElement().firstChild());
    QVERIFY(match->m_dataMap.isEmpty());

    qputenv("XDG_SESSION_TYPE", "x11");
    QVERIFY(match->matchCommonEnvironment(doc.elementsByTagName(QStringLiteral("Environment")).at(0)));
    QDomDocument allowed;
    QVERIFY(allowed.setContent(QStringLiteral(
        "<Root>"
        "<Item name=\"Network\">"
        "<Environment>ordinary</Environment>"
        "<EnglishFunc1>Wifi</EnglishFunc1>"
        "<EnglishFunc2>Proxy</EnglishFunc2>"
        "<ChinesePlugin>网络</ChinesePlugin>"
        "<ChineseFunc1>无线</ChineseFunc1>"
        "<ChineseFunc2>代理</ChineseFunc2>"
        "</Item>"
        "</Root>")));
    match->matchNodes(allowed.documentElement().firstChild());
    QVERIFY(match->m_dataMap.contains(QStringLiteral("Network/Wifi/Proxy")));
    QVERIFY(match->m_searchMap.value(QStringLiteral("Network")).value(QStringLiteral("Wifi")).contains(QStringLiteral("Proxy")));

    QVariantMap first;
    first.insert(QStringLiteral("superordinate"), QStringLiteral("Audio"));
    first.insert(QStringLiteral("plugin"), QStringLiteral("Sound"));
    first.insert(QStringLiteral("subEnglish"), QStringLiteral("Input"));
    first.insert(QStringLiteral("plugin") + QLocale::system().name(), QStringLiteral("Sound Local"));
    first.insert(QStringLiteral("sub") + QLocale::system().name(), QStringLiteral("Input Local"));
    QVariantMap second = first;
    second.insert(QStringLiteral("subEnglish"), QStringLiteral("Output"));
    second.insert(QStringLiteral("sub") + QLocale::system().name(), QStringLiteral("Output Local"));

    match->handleData(first, SettingsMatch::HandleType::Add);
    match->handleData(second, SettingsMatch::HandleType::Add);
    QVERIFY(match->m_dataMap.contains(QStringLiteral("Audio/Sound/Input")));
    QVERIFY(match->m_dataMap.contains(QStringLiteral("Audio/Sound/Output")));
    match->handleData(first, SettingsMatch::HandleType::Delete);
    QVERIFY(!match->m_dataMap.contains(QStringLiteral("Audio/Sound/Input")));
    QVERIFY(match->m_dataMap.contains(QStringLiteral("Audio/Sound")));
    QVERIFY(match->m_searchMap.value(QStringLiteral("Audio")).value(QStringLiteral("Sound")).contains(QStringLiteral("Output")));

    FakeSettingsSearchService settingsService;
    ScopedDbusRegistration settingsRegistration(QStringLiteral("org.ukui.ukcc.search"),
                                                QStringLiteral("/"),
                                                &settingsService);
    if (!settingsRegistration.isRegistered()) {
        QSKIP("failed to register fake ukcc search service");
    }

    SettingsSearchPlugin plugin;
    QCOMPARE(plugin.name(), QStringLiteral("Settings Search"));
    QCOMPARE(plugin.description(), QStringLiteral("Settings search."));
    QCOMPARE(plugin.getPluginName(), QStringLiteral("Settings Search"));
    QCOMPARE(plugin.pluginType(), SearchPluginIface::PluginType::SearchPlugin);
    plugin.setEnable(false);
    QVERIFY(!plugin.isEnable());
    plugin.setEnable(true);
    QVERIFY(plugin.isEnable());
    plugin.openAction(999, QStringLiteral("ignored"), 0);

    SearchPluginIface::ResultInfo result;
    result.name = QString(260, QLatin1Char('s'));
    result.actionKey = QStringLiteral("Sound");
    QCOMPARE(plugin.detailPage(result), plugin.m_detailPage);
    QVERIFY(!plugin.m_nameLabel->toolTip().isEmpty());
    QTRY_VERIFY_WITH_TIMEOUT(!SettingsMatch::getInstance()->isRunning(), 10000);
}

void PluginKeywordSearchWhiteboxTest::noteSearchWorkerCoversInvalidInterfaceAndStoppedSymbol()
{
    DataQueue<SearchPluginIface::ResultInfo> queue;
    QDBusInterface invalidInterface(QStringLiteral("org.ukui.search.whitebox.missing"),
                                    QStringLiteral("/org/ukui/search/whitebox/missing"),
                                    QStringLiteral("org.ukui.search.whitebox.missing"),
                                    QDBusConnection::sessionBus());
    NoteSearch invalidSearch(&queue, QStringLiteral("note"), 1, &invalidInterface);
    invalidSearch.run();
    QVERIFY(queue.isEmpty());
    NoteSearchPlugin plugin;
    DataQueue<SearchPluginIface::ResultInfo> pluginQueue;
    const size_t id = plugin.KeywordSearch(QStringLiteral("note"), &pluginQueue);
    QVERIFY(id > 0);
    plugin.stopSearch();
    plugin.m_pool.waitForDone(3000);
    QVERIFY(pluginQueue.isEmpty());
}

void PluginKeywordSearchWhiteboxTest::noteSearchWorkerParsesSingleValueAndPrivateBusPaths()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("session D-Bus is unavailable in this environment");
    }

    FakeNoteService noteService;
    QVERIFY(QDBusConnection::sessionBus().isConnected());
    noteService.replyMap.insert(QStringLiteral("single-note-id"),
                                QVariantList{QStringLiteral("body only")});
    ScopedDbusRegistration noteRegistration(QStringLiteral("org.ukui.note"),
                                            QStringLiteral("/org/ukui/note"),
                                            &noteService);
    QVERIFY(noteRegistration.isRegistered());

    QDBusInterface interface(QStringLiteral("org.ukui.note"),
                             QStringLiteral("/org/ukui/note"),
                             QStringLiteral("org.ukui.note.interface"),
                             QDBusConnection::sessionBus());
    QVERIFY(interface.isValid());

    NoteSearchPlugin resetPlugin;
    DataQueue<SearchPluginIface::ResultInfo> queue;
    NoteSearch search(&queue, QStringLiteral("single note keyword"), 0, &interface);
    search.run();
    QVERIFY(!noteService.lastKeywords.isEmpty());
    QVERIFY(!queue.isEmpty());
    const SearchPluginIface::ResultInfo result = queue.tryDequeue();
    QCOMPARE(result.searchID, size_t(0));
    QCOMPARE(result.actionKey, QStringLiteral("single-note-id"));
    QCOMPARE(result.name, QString());
    QCOMPARE(result.toolTip, QString());
    QCOMPARE(result.description.size(), 1);
    QCOMPARE(result.description.first().value, QStringLiteral("body only"));

    noteService.lastKeywords.clear();
    noteService.replyMap.insert(QStringLiteral("skipped-note-id"),
                                QVariantList{QStringLiteral("skipped body"),
                                             QStringLiteral("skipped title")});
    NoteSearch skippedSearch(&queue, QStringLiteral("skipped keyword"), 1, &interface);
    skippedSearch.run();
    QVERIFY(!noteService.lastKeywords.isEmpty());
    QVERIFY(queue.isEmpty());
}

void PluginKeywordSearchWhiteboxTest::searchManagerCreatesResultInfoAndDirectMatches()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("matched-dir")));

    const QString filePath = dir.filePath(QStringLiteral("line\r\nmatch.txt"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write("match");
    file.close();

    SearchPluginIface::ResultInfo missingInfo;
    QVERIFY(!SearchManager::creatResultInfo(missingInfo, dir.filePath(QStringLiteral("missing.txt"))));
    SearchManager manager;

    SearchPluginIface::ResultInfo fileInfo;
    QVERIFY(SearchManager::creatResultInfo(fileInfo, filePath));
    QCOMPARE(fileInfo.resourceType, QStringLiteral("file"));
    QCOMPARE(fileInfo.actionKey, filePath);
    QVERIFY(!fileInfo.name.contains(QLatin1Char('\r')));
    QVERIFY(!fileInfo.name.contains(QLatin1Char('\n')));
    QCOMPARE(fileInfo.type, 0);
    QCOMPARE(fileInfo.description.size(), 2);

    SearchPluginIface::ResultInfo dirInfo;
    QVERIFY(SearchManager::creatResultInfo(dirInfo, dir.filePath(QStringLiteral("matched-dir"))));
    QCOMPARE(dirInfo.resourceType, QStringLiteral("file"));
    QCOMPARE(dirInfo.type, 0);

    DataQueue<SearchPluginIface::ResultInfo> fileQueue;
    SearchManager::uniqueSymbolFile++;
    DirectSearch fileDirect(QStringLiteral("match"), &fileQueue, FILE_SEARCH_VALUE, SearchManager::uniqueSymbolFile);
    fileDirect.match(QFileInfo(filePath));
    QVERIFY(!fileQueue.isEmpty());
    SearchPluginIface::ResultInfo queuedFile = fileQueue.tryDequeue();
    QCOMPARE(queuedFile.searchID, SearchManager::uniqueSymbolFile);
    QCOMPARE(queuedFile.actionKey, filePath);

    fileDirect.match(QFileInfo(dir.filePath(QStringLiteral("matched-dir"))));
    QVERIFY(fileQueue.isEmpty());

    DataQueue<SearchPluginIface::ResultInfo> dirQueue;
    SearchManager::uniqueSymbolDir++;
    DirectSearch dirDirect(QStringLiteral("matched"), &dirQueue, DIR_SEARCH_VALUE, SearchManager::uniqueSymbolDir);
    dirDirect.match(QFileInfo(dir.filePath(QStringLiteral("matched-dir"))));
    QVERIFY(!dirQueue.isEmpty());
    SearchPluginIface::ResultInfo queuedDir = dirQueue.tryDequeue();
    QCOMPARE(queuedDir.searchID, SearchManager::uniqueSymbolDir);
    QCOMPARE(queuedDir.actionKey, dir.filePath(QStringLiteral("matched-dir")));

    dirDirect.match(QFileInfo(filePath));
    QVERIFY(dirQueue.isEmpty());
}

void PluginKeywordSearchWhiteboxTest::searchManagerRunsAgainstRealDatabases()
{
    QTemporaryDir homeDir;
    QVERIFY(homeDir.isValid());
    ScopedEnvVar home("HOME", homeDir.path().toUtf8());
    ScopedEnvVar configHome("XDG_CONFIG_HOME", (homeDir.path() + QStringLiteral("/.config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);
    ScopedIndexableDirs scopedIndexableDirs(m_dirWatcherService, QStringList{homeDir.path()});
    ensureSearchConfigDirs();

    QCOMPARE(SearchManager::getCurrentIndexCount(), 0U);

    const QString keyword = QStringLiteral("Special");
    const QString textFilePath = homeDir.filePath(QStringLiteral("special-file.txt"));
    const QString imageFilePath = homeDir.filePath(QStringLiteral("special-image.jpg"));
    const QString dirPath = homeDir.filePath(QStringLiteral("special-dir"));
    QVERIFY(QDir(homeDir.path()).mkpath(QStringLiteral("special-dir")));

    QFile textFile(textFilePath);
    QVERIFY(textFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    textFile.write("special");
    textFile.close();

    QFile imageFile(imageFilePath);
    QVERIFY(imageFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    imageFile.write("jpeg");
    imageFile.close();

    writeFileSearchDatabase({{textFilePath, FILE_SEARCH_VALUE}, {imageFilePath, FILE_SEARCH_VALUE}, {dirPath, DIR_SEARCH_VALUE}}, keyword);
    QCOMPARE(SearchManager::getCurrentIndexCount(), 3U);

    DataQueue<SearchPluginIface::ResultInfo> fileQueue;
    SearchManager::uniqueSymbolFile++;
    FileSearch fileSearch(&fileQueue, SearchManager::uniqueSymbolFile, keyword, FILE_SEARCH_VALUE, 1, 0, 20);
    fileSearch.run();

    const QList<SearchPluginIface::ResultInfo> fileResults = drainResults(fileQueue);
    QCOMPARE(fileResults.size(), 2);
    QVERIFY(std::any_of(fileResults.cbegin(), fileResults.cend(), [&textFilePath](const SearchPluginIface::ResultInfo &result) {
        return result.actionKey == textFilePath && result.type == 0;
    }));
    QVERIFY(std::any_of(fileResults.cbegin(), fileResults.cend(), [&imageFilePath](const SearchPluginIface::ResultInfo &result) {
        return result.actionKey == imageFilePath && result.type == 1;
    }));

    SearchPluginIface::ResultInfo placeholder;
    placeholder.actionKey = QStringLiteral("stale");
    fileQueue.enqueue(placeholder);
    const size_t staleFileSymbol = SearchManager::uniqueSymbolFile;
    SearchManager::uniqueSymbolFile++;
    FileSearch staleFileSearch(&fileQueue, staleFileSymbol, keyword, FILE_SEARCH_VALUE, 1, 0, 20);
    staleFileSearch.run();
    QVERIFY(fileQueue.isEmpty());

    DataQueue<SearchPluginIface::ResultInfo> dirQueue;
    dirQueue.enqueue(placeholder);
    SearchManager::uniqueSymbolDir++;
    FileSearch dirSearch(&dirQueue, SearchManager::uniqueSymbolDir, keyword, DIR_SEARCH_VALUE, 1, 0, 20);
    dirSearch.run();
    const QList<SearchPluginIface::ResultInfo> dirResults = drainResults(dirQueue);
    QCOMPARE(dirResults.size(), 1);
    QCOMPARE(dirResults.first().actionKey, dirPath);
    QCOMPARE(dirResults.first().searchID, SearchManager::uniqueSymbolDir);

    dirQueue.enqueue(placeholder);
    const size_t staleDirSymbol = SearchManager::uniqueSymbolDir;
    SearchManager::uniqueSymbolDir++;
    FileSearch staleDirSearch(&dirQueue, staleDirSymbol, keyword, DIR_SEARCH_VALUE, 1, 0, 20);
    staleDirSearch.run();
    QVERIFY(dirQueue.isEmpty());

    DataQueue<SearchPluginIface::ResultInfo> unfilteredQueue;
    SearchManager::uniqueSymbolFile++;
    FileSearch unfilteredSearch(&unfilteredQueue, SearchManager::uniqueSymbolFile, keyword, QString(), 1, 0, 20);
    unfilteredSearch.run();
    QVERIFY(!unfilteredQueue.isEmpty());
    drainResults(unfilteredQueue);

    const QString contentFilePath = homeDir.filePath(QStringLiteral("zeta-file.txt"));
    QFile contentFile(contentFilePath);
    QVERIFY(contentFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    contentFile.write("indexed searchable content");
    contentFile.close();

    const QString contentTerm = QStringLiteral("indexed");
    QDir(QString::fromStdString(CONTENT_INDEX_PATH)).removeRecursively();
    QDir(QString::fromStdString(OCR_CONTENT_INDEX_PATH)).removeRecursively();
    fileContentIndexer contentIndexer(contentFilePath);
    QVERIFY(contentIndexer.index());
    Xapian::WritableDatabase contentDatabase(CONTENT_INDEX_PATH, Xapian::DB_CREATE_OR_OVERWRITE);
    contentDatabase.add_document(contentIndexer.document().getXapianDocument());
    Xapian::Document missingContentDoc = contentIndexer.document().getXapianDocument();
    missingContentDoc.add_value(CONTENT_DATABASE_PATH_SLOT, "/not/existing/content");
    contentDatabase.add_document(missingContentDoc);
    contentDatabase.commit();
    Xapian::WritableDatabase ocrDatabase(OCR_CONTENT_INDEX_PATH, Xapian::DB_CREATE_OR_OVERWRITE);
    ocrDatabase.commit();

    auto *config = FileIndexerConfig::getInstance();
    bool oldOcrEnable = false;
    bool restoreOcrEnable = false;
    if (config->m_gsettings && config->m_gsettings->keys().contains(QStringLiteral("contentIndexEnableOcr"))) {
        oldOcrEnable = config->m_gsettings->get(QStringLiteral("contentIndexEnableOcr")).toBool();
        config->m_gsettings->set(QStringLiteral("contentIndexEnableOcr"), true);
        restoreOcrEnable = true;
    }

    DataQueue<SearchPluginIface::ResultInfo> contentQueue;
    SearchManager::uniqueSymbolContent++;
    FileContentSearch contentSearch(&contentQueue, SearchManager::uniqueSymbolContent, contentTerm, false, 0, 20);
    contentSearch.run();

    const QList<SearchPluginIface::ResultInfo> contentResults = drainResults(contentQueue);
    if (restoreOcrEnable) {
        config->m_gsettings->set(QStringLiteral("contentIndexEnableOcr"), oldOcrEnable);
    }
    QCOMPARE(contentResults.size(), 1);
    QCOMPARE(contentResults.first().actionKey, contentFilePath);
    QCOMPARE(contentResults.first().searchID, SearchManager::uniqueSymbolContent);
    QVERIFY(contentResults.first().description.size() >= 3);
    QVERIFY(contentResults.first().description.first().value.contains(contentTerm, Qt::CaseInsensitive));

    contentQueue.enqueue(placeholder);
    const size_t staleContentSymbol = SearchManager::uniqueSymbolContent;
    SearchManager::uniqueSymbolContent++;
    FileContentSearch staleContentSearch(&contentQueue, staleContentSymbol, contentTerm, true, 0, 20);
    staleContentSearch.run();
    QVERIFY(contentQueue.isEmpty());
}

void PluginKeywordSearchWhiteboxTest::directSearchRunTraversesHomeAndHonorsMismatchedSymbols()
{
    QTemporaryDir homeDir;
    QVERIFY(homeDir.isValid());
    ScopedEnvVar home("HOME", homeDir.path().toUtf8());
    ScopedEnvVar configHome("XDG_CONFIG_HOME", (homeDir.path() + QStringLiteral("/.config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);
    ScopedIndexableDirs scopedIndexableDirs(m_dirWatcherService, QStringList{homeDir.path()});
    ensureSearchConfigDirs();

    const QString matchedFilePath = homeDir.filePath(QStringLiteral("direct-special-file.txt"));
    QFile matchedFile(matchedFilePath);
    QVERIFY(matchedFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    matchedFile.write("direct");
    matchedFile.close();
    QVERIFY(QDir(homeDir.path()).mkpath(QStringLiteral("direct-special-dir")));

    DataQueue<SearchPluginIface::ResultInfo> fileQueue;
    SearchManager::uniqueSymbolFile++;
    DirectSearch fileSearch(QStringLiteral("special"), &fileQueue, FILE_SEARCH_VALUE, SearchManager::uniqueSymbolFile);
    fileSearch.run();
    QVERIFY(!fileQueue.isEmpty());
    const QList<SearchPluginIface::ResultInfo> fileResults = drainResults(fileQueue);
    QVERIFY(std::any_of(fileResults.cbegin(), fileResults.cend(), [&matchedFilePath](const SearchPluginIface::ResultInfo &result) {
        return result.actionKey == matchedFilePath;
    }));

    DataQueue<SearchPluginIface::ResultInfo> dirQueue;
    SearchManager::uniqueSymbolDir++;
    DirectSearch dirSearch(QStringLiteral("special"), &dirQueue, DIR_SEARCH_VALUE, SearchManager::uniqueSymbolDir);
    dirSearch.run();
    QVERIFY(!dirQueue.isEmpty());
    const QList<SearchPluginIface::ResultInfo> dirResults = drainResults(dirQueue);
    QVERIFY(std::any_of(dirResults.cbegin(), dirResults.cend(), [&homeDir](const SearchPluginIface::ResultInfo &result) {
        return result.actionKey == homeDir.filePath(QStringLiteral("direct-special-dir"));
    }));

    const size_t staleFileSymbol = SearchManager::uniqueSymbolFile;
    SearchManager::uniqueSymbolFile++;
    DirectSearch staleFileSearch(QStringLiteral("special"), &fileQueue, FILE_SEARCH_VALUE, staleFileSymbol);
    staleFileSearch.run();
    QVERIFY(fileQueue.isEmpty());

    const size_t staleDirSymbol = SearchManager::uniqueSymbolDir;
    SearchManager::uniqueSymbolDir++;
    DirectSearch staleDirSearch(QStringLiteral("special"), &dirQueue, DIR_SEARCH_VALUE, staleDirSymbol);
    staleDirSearch.run();
    QVERIFY(dirQueue.isEmpty());

    QTemporaryDir policyDir;
    QVERIFY(policyDir.isValid());
    const QString blacklistedDir = policyDir.filePath(QStringLiteral("black-special-dir"));
    QVERIFY(QDir(policyDir.path()).mkpath(QStringLiteral("black-special-dir")));
    QVERIFY(writeFileText(policyDir.filePath(QStringLiteral("ordinary.txt")), QByteArray("ordinary")));

    {
        ScopedDirWatcherLists blockedRoot(m_dirWatcherService,
                                          QStringList{policyDir.path()},
                                          {},
                                          QStringList{policyDir.path()},
                                          {});
        DataQueue<SearchPluginIface::ResultInfo> blockedQueue;
        SearchManager::uniqueSymbolFile++;
        DirectSearch blockedSearch(QStringLiteral("ordinary"), &blockedQueue, FILE_SEARCH_VALUE, SearchManager::uniqueSymbolFile);
        blockedSearch.run();
        QVERIFY(blockedQueue.isEmpty());
        QVERIFY(SearchManager::isBlocked(policyDir.path()));
    }

    {
        ScopedDirWatcherLists blacklistedChild(m_dirWatcherService,
                                               QStringList{policyDir.path(), blacklistedDir},
                                               QStringList{blacklistedDir},
                                               {},
                                               {});
        DataQueue<SearchPluginIface::ResultInfo> blockedByBlacklistQueue;
        SearchManager::uniqueSymbolDir++;
        DirectSearch blacklistedSearch(QStringLiteral("black-special"), &blockedByBlacklistQueue, DIR_SEARCH_VALUE, SearchManager::uniqueSymbolDir);
        blacklistedSearch.run();
        QVERIFY(blockedByBlacklistQueue.isEmpty());
    }

    {
        ScopedDirWatcherLists blockedChild(m_dirWatcherService,
                                           QStringList{policyDir.path()},
                                           {},
                                           QStringList{blacklistedDir},
                                           {});
        DataQueue<SearchPluginIface::ResultInfo> blockedChildQueue;
        SearchManager::uniqueSymbolDir++;
        DirectSearch blockedChildSearch(QStringLiteral("black-special"), &blockedChildQueue, DIR_SEARCH_VALUE, SearchManager::uniqueSymbolDir);
        blockedChildSearch.run();
        QVERIFY(blockedChildQueue.isEmpty());

        FileMatchDecider fileDecider;
        Xapian::Document fileDoc;
        fileDoc.set_data(blacklistedDir.toStdString());
        QVERIFY(!fileDecider(fileDoc));

        FileContentMatchDecider contentDecider;
        Xapian::Document contentDoc;
        contentDoc.add_value(CONTENT_DATABASE_PATH_SLOT, blacklistedDir.toStdString());
        QVERIFY(!contentDecider(contentDoc));
    }

    {
        ScopedDirWatcherLists extraChild(m_dirWatcherService,
                                         QStringList{policyDir.path(), blacklistedDir},
                                         QStringList{blacklistedDir},
                                         {},
                                         QStringList{blacklistedDir});
        DataQueue<SearchPluginIface::ResultInfo> extraQueue;
        SearchManager::uniqueSymbolDir++;
        DirectSearch extraSearch(QStringLiteral("black-special"), &extraQueue, DIR_SEARCH_VALUE, SearchManager::uniqueSymbolDir);
        extraSearch.run();
        QVERIFY(!extraQueue.isEmpty());
        const QList<SearchPluginIface::ResultInfo> extraResults = drainResults(extraQueue);
        QVERIFY(std::any_of(extraResults.cbegin(), extraResults.cend(), [&blacklistedDir](const SearchPluginIface::ResultInfo &result) {
            return result.actionKey == blacklistedDir;
        }));
    }

    {
        ScopedDirWatcherLists defaultSwitch(m_dirWatcherService,
                                            QStringList{policyDir.path()},
                                            {},
                                            {},
                                            {});
        DataQueue<SearchPluginIface::ResultInfo> defaultQueue;
        DirectSearch defaultSearch(QStringLiteral("ordinary"), &defaultQueue, QStringLiteral("2"), SearchManager::uniqueSymbolFile);
        defaultSearch.run();
        QVERIFY(defaultQueue.isEmpty());
    }
}

void PluginKeywordSearchWhiteboxTest::webPluginCoversSchemaThemeAndPrivateBusBranches()
{
    GlobalSettings::getInstance().d->setValue(STYLE_NAME_KEY, QStringLiteral("ukui-dark"));
    WebSearchPlugin darkPlugin;
    QVERIFY(darkPlugin.m_iconLabel != nullptr);
    QVERIFY(darkPlugin.m_settings == nullptr || darkPlugin.m_webEngine == darkPlugin.m_settings->get(QStringLiteral("webEngine")).toString());

    if (darkPlugin.m_settings != nullptr) {
        darkPlugin.m_settings->set(QStringLiteral("webEngine"), QStringLiteral("sougou"));
        QTRY_COMPARE(darkPlugin.m_webEngine, QStringLiteral("sougou"));
    }

    GlobalSettings::getInstance().d->setValue(STYLE_NAME_KEY, QStringLiteral("ukui-light"));
    Q_EMIT GlobalSettings::getInstance().valueChanged(STYLE_NAME_KEY, QStringLiteral("ukui-light"));
    QCoreApplication::processEvents();
    QVERIFY(darkPlugin.m_iconLabel->pixmap() != nullptr);

    const QString defaultAppId = darkPlugin.getDefaultAppId("x-scheme-handler/http");
    QVERIFY(defaultAppId.isEmpty() || defaultAppId.endsWith(QStringLiteral(".desktop")));

    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("session D-Bus is unavailable in this environment");
    }

    DataQueue<SearchPluginIface::ResultInfo> queue;
    {
        FakeAppLauncher launcher;
        ScopedDbusRegistration registration(QStringLiteral("com.kylin.ProcessManager"),
                                            QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                            &launcher);
        QVERIFY(registration.isRegistered());

        darkPlugin.m_webEngine = QStringLiteral("360");
        darkPlugin.KeywordSearch(QStringLiteral("private bus keyword"), &queue);
        darkPlugin.openAction(0, QStringLiteral("ignored"), 0);
        QCOMPARE(launcher.launchUrlCalls, 1);
        QCOMPARE(launcher.lastUrl, QStringLiteral("https://www.so.com/s?q=private bus keyword"));
    }

    {
        BrokenAppLauncher brokenLauncher;
        ScopedDbusRegistration brokenRegistration(QStringLiteral("com.kylin.ProcessManager"),
                                                  QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                                  &brokenLauncher);
        QVERIFY(brokenRegistration.isRegistered());
        darkPlugin.m_webEngine = QStringLiteral("unknown");
        darkPlugin.KeywordSearch(QStringLiteral("fallback keyword"), &queue);
        darkPlugin.openAction(0, QStringLiteral("ignored"), 0);
    }
}

int main(int argc, char **argv)
{
    ScopedSessionBusDaemon sessionBus;
    if (!sessionBus.start()) {
        qCritical("failed to start private session bus daemon");
        return 1;
    }

    QApplication app(argc, argv);
    PluginKeywordSearchWhiteboxTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "plugin-keyword-search-whitebox-test.moc"
