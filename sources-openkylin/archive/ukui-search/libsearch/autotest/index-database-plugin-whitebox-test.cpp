#include <cstdlib>

#include <QApplication>
#include <QDBusConnection>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QLabel>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <memory>

#include <xapian.h>

#define private public
#define protected public
#include "index/basic-indexer.h"
#include "index/writable-database.h"
#undef DATABASE_H
#include "index/database.h"
#include "index/index-status-recorder.h"
#include "index/file-search-plugin.h"
#undef protected
#undef private

using namespace UkuiSearch;

namespace {

QString configuredHome()
{
    const QByteArray value = qgetenv("UKUI_SEARCH_WHITEBOX_HOME");
    return value.isEmpty() ? QDir::homePath() : QString::fromLocal8Bit(value);
}

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

SearchPluginIface::ResultInfo fileLikeResult(const QString &path, const QString &name)
{
    SearchPluginIface::ResultInfo result;
    result.name = name;
    result.toolTip = name;
    result.actionKey = path;
    result.resourceType = QStringLiteral("file");
    result.description.append({QStringLiteral("Path"), path});
    result.description.append({QStringLiteral("Modified"), QStringLiteral("20240102030405000")});
    result.type = 0;
    return result;
}

}

class IndexDatabasePluginWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void writableDatabaseCoversAllDatabaseTypesAndMetadata();
    void writableDatabaseLifecycleAndStatusBranches();
    void databaseRefreshTracksExistingAndRemovedIndexes();
    void fileAndDirPluginDetailPagesAndSearchBranches();

private:
    QString m_home;
    FakePrivateDirWatcherService *m_dirWatcherService = nullptr;
    std::unique_ptr<ScopedDbusRegistration> m_dirWatcherRegistration;
};

void IndexDatabasePluginWhiteboxTest::initTestCase()
{
    m_home = configuredHome();
    QVERIFY2(!m_home.isEmpty(), "test home must be configured");
    QVERIFY(QDir().mkpath(m_home + QStringLiteral("/Desktop")));
    QVERIFY(QDir().mkpath(m_home + QStringLiteral("/.config/org.ukui/ukui-search")));
    m_dirWatcherService = new FakePrivateDirWatcherService;
    m_dirWatcherRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.ukui.search.fileindex.service"),
                                                              QStringLiteral("/org/ukui/search/privateDirWatcher"),
                                                              m_dirWatcherService));
}

void IndexDatabasePluginWhiteboxTest::writableDatabaseCoversAllDatabaseTypesAndMetadata()
{
    QTemporaryDir sourceDir(m_home + QStringLiteral("/writable-db-source-XXXXXX"));
    QVERIFY(sourceDir.isValid());

    QFile file(sourceDir.path() + QStringLiteral("/whitebox-index-file.txt"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("whitebox xapian document");
    file.close();

    BasicIndexer indexer(file.fileName());
    QVERIFY(indexer.index());
    const Document document = indexer.document();

    {
        WritableDatabase database(DataBaseType::Basic);
        QVERIFY(database.open());
        QVERIFY(database.open());
        database.beginTransation();
        database.addDocument(document);
        database.setMetaData(QStringLiteral("qt-key"), QStringLiteral("qt-value"));
        database.setMetaData(std::string("std-key"), std::string("std-value"));
        database.endTransation();
        database.commit();
        QCOMPARE(QString::fromStdString(database.getMetaData("qt-key")), QStringLiteral("qt-value"));
        QCOMPARE(QString::fromStdString(database.getMetaData("std-key")), QStringLiteral("std-value"));
        QVERIFY(database.getIndexTimes().contains(document.getUniqueTerm()));
        database.removeDocument(file.fileName());
        database.removeDocument(document.getUniqueTerm());
        database.removeChildrenDocument(sourceDir.path());
        database.commit();
        database.rebuild();
        QVERIFY(!QDir(INDEX_PATH).exists());
    }

    {
        WritableDatabase database(DataBaseType::Content);
        QVERIFY(database.open());
        database.beginTransation();
        database.cancelTransation();
        database.rebuild();
        QVERIFY(!QDir(CONTENT_INDEX_PATH).exists());
    }

    {
        WritableDatabase database(DataBaseType::OcrContent);
        QVERIFY(database.open());
        database.rebuild();
        QVERIFY(!QDir(OCR_CONTENT_INDEX_PATH).exists());
    }
}

void IndexDatabasePluginWhiteboxTest::writableDatabaseLifecycleAndStatusBranches()
{
    auto *recorder = IndexStatusRecorder::getInstance();

    QDir(INDEX_PATH).removeRecursively();
    {
        WritableDatabase database(DataBaseType::Basic);
        database.rebuild();
        QVERIFY(QDir(INDEX_PATH).exists());
        QVERIFY(database.open());
        database.rebuild();
        QVERIFY(!QDir(INDEX_PATH).exists());
        database.errorRecord();
    }
    QCOMPARE(recorder->getStatus(INDEX_DATABASE_STATE_KEY).toInt(), int(IndexStatusRecorder::State::Error));

    QDir(CONTENT_INDEX_PATH).removeRecursively();
    {
        WritableDatabase database(DataBaseType::Content);
        database.rebuild();
        QVERIFY(QDir(CONTENT_INDEX_PATH).exists());
        QVERIFY(database.open());
        database.rebuild();
        QVERIFY(!QDir(CONTENT_INDEX_PATH).exists());
        database.errorRecord();
    }
    QCOMPARE(recorder->getStatus(CONTENT_INDEX_DATABASE_STATE_KEY).toInt(), int(IndexStatusRecorder::State::Error));

    QDir(OCR_CONTENT_INDEX_PATH).removeRecursively();
    {
        WritableDatabase database(DataBaseType::OcrContent);
        database.rebuild();
        QVERIFY(QDir(OCR_CONTENT_INDEX_PATH).exists());
        QVERIFY(database.open());
        database.rebuild();
        QVERIFY(!QDir(OCR_CONTENT_INDEX_PATH).exists());
        database.errorRecord();
    }
    QCOMPARE(recorder->getStatus(OCR_CONTENT_INDEX_DATABASE_STATE_KEY).toInt(), int(IndexStatusRecorder::State::Error));
}

void IndexDatabasePluginWhiteboxTest::databaseRefreshTracksExistingAndRemovedIndexes()
{
    QTemporaryDir sourceDir(m_home + QStringLiteral("/database-refresh-XXXXXX"));
    QVERIFY(sourceDir.isValid());

    const QString textPath = sourceDir.path() + QStringLiteral("/database-refresh.txt");
    QFile file(textPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("database refresh");
    file.close();

    BasicIndexer indexer(textPath);
    QVERIFY(indexer.index());

    {
        WritableDatabase database(DataBaseType::Basic);
        database.rebuild();
        QVERIFY(database.open());
        database.addDocument(indexer.document());
        database.commit();
    }
    {
        Database database(DataBaseType::Basic);
        QVERIFY(database.valid());
        QCOMPARE(database.getIndexDocCount(), uint(1));
    }
    QVERIFY(QDir(INDEX_PATH).removeRecursively());
    {
        Database database(DataBaseType::Basic);
        QVERIFY(!database.valid());
        QCOMPARE(database.getIndexDocCount(), uint(0));
    }

    {
        WritableDatabase database(DataBaseType::Content);
        database.rebuild();
        QVERIFY(database.open());
        database.commit();
    }
    {
        Database database(DataBaseType::Content);
        QVERIFY(database.valid());
        QCOMPARE(database.getIndexDocCount(), uint(0));
    }
    QVERIFY(QDir(CONTENT_INDEX_PATH).removeRecursively());
    {
        Database database(DataBaseType::Content);
        QVERIFY(!database.valid());
        QCOMPARE(database.getIndexDocCount(), uint(0));
    }

    {
        WritableDatabase database(DataBaseType::OcrContent);
        database.rebuild();
        QVERIFY(database.open());
        database.commit();
    }
    {
        Database database(DataBaseType::OcrContent);
        QVERIFY(database.valid());
        QCOMPARE(database.getIndexDocCount(), uint(0));
    }
    QVERIFY(QDir(OCR_CONTENT_INDEX_PATH).removeRecursively());
    {
        Database database(DataBaseType::OcrContent);
        QVERIFY(!database.valid());
        QCOMPARE(database.getIndexDocCount(), uint(0));
    }
}

void IndexDatabasePluginWhiteboxTest::fileAndDirPluginDetailPagesAndSearchBranches()
{
    QTemporaryDir dir(m_home + QStringLiteral("/file-plugin-XXXXXX"));
    QVERIFY(dir.isValid());

    QFile file(dir.path() + QStringLiteral("/whitebox-result.txt"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("data");
    file.close();

    DataQueue<SearchPluginIface::ResultInfo> fileQueue;
    FileSearchPlugin filePlugin;
    QCOMPARE(filePlugin.name(), QStringLiteral("File Search"));
    QVERIFY(filePlugin.description().contains(QStringLiteral("File")));
    QCOMPARE(filePlugin.getActioninfo(0).size(), 3);
    filePlugin.setEnable(false);
    QVERIFY(!filePlugin.isEnable());
    filePlugin.setEnable(true);
    QVERIFY(filePlugin.isEnable());

    SearchPluginIface::ResultInfo fileResult = fileLikeResult(file.fileName(), QStringLiteral("whitebox-result.txt"));
    QWidget *filePage = filePlugin.detailPage(fileResult);
    QVERIFY(filePage);
    QCOMPARE(filePlugin.m_currentActionKey, file.fileName());
    QCOMPARE(filePlugin.m_pluginLabel->text(), QStringLiteral("File"));
    QCOMPARE(filePlugin.m_pathLabel2->toolTip(), file.fileName());
    QVERIFY(!filePlugin.m_timeLabel2->text().isEmpty());
    filePlugin.stopSearch();

    DataQueue<SearchPluginIface::ResultInfo> dirQueue;
    DirSearchPlugin dirPlugin;
    QCOMPARE(dirPlugin.name(), QStringLiteral("Dir Search"));
    QVERIFY(dirPlugin.description().contains(QStringLiteral("Dir")));
    QCOMPARE(dirPlugin.getActioninfo(0).size(), 3);
    dirPlugin.setEnable(false);
    QVERIFY(!dirPlugin.isEnable());
    dirPlugin.setEnable(true);
    QVERIFY(dirPlugin.isEnable());

    SearchPluginIface::ResultInfo dirResult = fileLikeResult(dir.path(), QStringLiteral("file-plugin-dir"));
    QWidget *dirPage = dirPlugin.detailPage(dirResult);
    QVERIFY(dirPage);
    QCOMPARE(dirPlugin.m_currentActionKey, dir.path());
    QCOMPARE(dirPlugin.m_pluginLabel->text(), QStringLiteral("Directory"));
    QCOMPARE(dirPlugin.m_pathLabel2->toolTip(), dir.path());
    dirPlugin.stopSearch();

    QCOMPARE(fileQueue.length(), 0);
    QCOMPARE(dirQueue.length(), 0);
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
    IndexDatabasePluginWhiteboxTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "index-database-plugin-whitebox-test.moc"
