#include <QtTest>

#include <QDBusConnection>
#include <QDBusVariant>
#include <QDir>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUuid>

#define private public
#include "../appdata/app-info-table-private.h"
#include "../appdata/application-info.h"
#undef private

using namespace UkuiSearch;

namespace {

class FakeSignalTransformer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.search.signalTransformer")

Q_SIGNALS:
    void appDBItemsUpdate(UkuiSearch::ApplicationInfoMap results);
    void appDBItemsUpdateAll(QStringList desktopFilePaths);
    void appDBItemsAdd(QStringList desktopFilePaths);
    void appDBItemsDelete(QStringList desktopFilePaths);
};

class FakeAppDbManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.search.appDBManager")

public Q_SLOTS:
    void updateFavoritesState(const QString &)
    {
        ++favoritesNoOrderCalls;
    }

    void updateFavoritesState(const QString &, uint num)
    {
        lastFavoritesOrder = num;
    }

    void updateTopState(const QString &)
    {
        ++topNoOrderCalls;
    }

    void updateTopState(const QString &, uint num)
    {
        lastTopOrder = num;
    }

    void setValue(const QVariant &value)
    {
        lastSetValue = value;
    }

    QString tranPidToDesktopFp(uint pid) const
    {
        lastPid = pid;
        return pidDesktopFilePath;
    }

    QString tranWinIdToDesktopFilePath(const QDBusVariant &winId) const
    {
        lastWinId = winId.variant();
        return winDesktopFilePath;
    }

    QString desktopFilePathFromName(const QString &name) const
    {
        lastDesktopName = name;
        return nameDesktopFilePath;
    }

public:
    int favoritesNoOrderCalls = 0;
    int topNoOrderCalls = 0;
    uint lastFavoritesOrder = 0;
    uint lastTopOrder = 0;
    QVariant lastSetValue;
    mutable uint lastPid = 0;
    mutable QVariant lastWinId;
    mutable QString lastDesktopName;
    QString pidDesktopFilePath{QStringLiteral("/tmp/from-pid.desktop")};
    QString winDesktopFilePath{QStringLiteral("/tmp/from-win.desktop")};
    QString nameDesktopFilePath{QStringLiteral("/tmp/from-name.desktop")};
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

}

class AppInfoTableWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void querySingleAndAllRowsFromTemporaryDatabase();
    void queryWithRestrictionsKeywordsAndOrder();
    void queryEdgeCasesCoverFieldAndKeywordBranches();
    void privateDedupHelpersCoverPriorityBranches();
    void privateMutationAndSearchHelpersUseDatabase();
    void openDatabaseSqlFailureBranchesReturnFalse();
    void databaseFailureBranchesReturnFalse();
    void signalForwardingSlotsRelayPublicSignals();
    void applicationInfoFacadeForwardsQueriesAndMutations();

private:
    void createDatabase();
    void bindTableToTemporaryDatabase();
    void insertRow(const QString &desktopFilePath,
                   const QString &md5,
                   const QString &localName,
                   const QString &nameEn,
                   const QString &nameZh,
                   const QString &pinyin,
                   const QString &firstLetter,
                   int launchTimes,
                   int favorites,
                   int launched,
                   int lock,
                   const QString &keywords);
    QVariant valueFor(const QString &desktopFilePath, const QString &field);

    QString m_originalHome;
    QTemporaryDir m_homeDir;
    AppInfoTable *m_table = nullptr;
    QString m_databasePath;
    FakeSignalTransformer m_signalTransformer;
    FakeAppDbManager m_appDbManager;
    std::unique_ptr<ScopedDbusRegistration> m_signalTransformerRegistration;
    std::unique_ptr<ScopedDbusRegistration> m_appDbManagerRegistration;
};

void AppInfoTableWhiteboxTest::initTestCase()
{
    QVERIFY(m_homeDir.isValid());
    m_originalHome = qEnvironmentVariable("HOME");
    qputenv("HOME", m_homeDir.path().toUtf8());
    qputenv("XDG_DATA_DIRS", (m_homeDir.path() + "/share:/usr/share").toUtf8());
    m_signalTransformerRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.ukui.search.appdb.service"),
                                                                     QStringLiteral("/org/ukui/search/appDataBase/signalTransformer"),
                                                                     &m_signalTransformer));
    m_appDbManagerRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.ukui.search.appdb.service"),
                                                                QStringLiteral("/org/ukui/search/appDataBase/dbManager"),
                                                                &m_appDbManager));
    createDatabase();
    m_table = new AppInfoTable;
    bindTableToTemporaryDatabase();
    QVERIFY(m_table->d->m_database->isOpen());
}

void AppInfoTableWhiteboxTest::cleanupTestCase()
{
    delete m_table;
    m_table = nullptr;
    m_appDbManagerRegistration.reset();
    m_signalTransformerRegistration.reset();
    qputenv("HOME", m_originalHome.toUtf8());
}

void AppInfoTableWhiteboxTest::createDatabase()
{
    const QString dbDir = QDir::homePath() + "/.config/org.ukui/ukui-search/appdata";
    QVERIFY(QDir().mkpath(dbDir));
    m_databasePath = dbDir + "/app-info.db";

    const QString connectionName = QUuid::createUuid().toString(QUuid::WithoutBraces);
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(m_databasePath);
        QVERIFY2(db.open(), qPrintable(db.lastError().text()));
        QSqlQuery query(db);
        QVERIFY2(query.exec("CREATE TABLE appInfo("
                            "DESKTOP_FILE_PATH TEXT PRIMARY KEY NOT NULL,"
                            "MODIFYED_TIME TEXT, INSERT_TIME TEXT, LOCAL_NAME TEXT,"
                            "NAME_EN TEXT, NAME_ZH TEXT, PINYIN_NAME TEXT,"
                            "FIRST_LETTER_OF_PINYIN TEXT, FIRST_LETTER_ALL TEXT,"
                            "ICON TEXT, TYPE TEXT, CATEGORY TEXT, EXEC TEXT, COMMENT TEXT,"
                            "MD5 TEXT, LAUNCH_TIMES INT, FAVORITES INT, LAUNCHED INT,"
                            "TOP INT, LOCK INT, DONT_DISPLAY INT, AUTO_START INT,"
                            "START_UP_WMCLASS TEXT, KEYWORDS TEXT, LOCAL_KEYWORDS TEXT)"),
                 qPrintable(query.lastError().text()));

        insertRow("/usr/share/applications/editor.desktop", "same-md5", "Editor", "Editor", "编辑器",
                  "bianjiqi", "bjq", 20, 2, 1, 0, "edit;write");
        insertRow(m_homeDir.path() + "/.local/share/applications/editor.desktop", "same-md5", "User Editor",
                  "User Editor", "用户编辑器", "yonghubianjiqi", "yhbjq", 5, 0, 0, 0, "custom;edit");
        insertRow("/opt/apps/tools/calc.desktop", "calc-md5", "Calculator", "Calculator", "计算器",
                  "jisuanqi", "jsq", 3, 0, 0, 1, "math;number");
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void AppInfoTableWhiteboxTest::bindTableToTemporaryDatabase()
{
    m_table->d->m_database->close();
    delete m_table->d->m_database;
    const QString connectionName = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_table->d->m_ConnectionName = connectionName;
    m_table->d->m_database = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", connectionName));
    m_table->d->m_database->setDatabaseName(m_databasePath);
    QVERIFY2(m_table->d->m_database->open(), qPrintable(m_table->d->m_database->lastError().text()));
}

void AppInfoTableWhiteboxTest::insertRow(const QString &desktopFilePath,
                                         const QString &md5,
                                         const QString &localName,
                                         const QString &nameEn,
                                         const QString &nameZh,
                                         const QString &pinyin,
                                         const QString &firstLetter,
                                         int launchTimes,
                                         int favorites,
                                         int launched,
                                         int lock,
                                         const QString &keywords)
{
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::connectionNames().last());
    QSqlQuery query(db);
    query.prepare("INSERT INTO appInfo("
                  "DESKTOP_FILE_PATH, MODIFYED_TIME, INSERT_TIME, LOCAL_NAME, NAME_EN, NAME_ZH,"
                  "PINYIN_NAME, FIRST_LETTER_OF_PINYIN, FIRST_LETTER_ALL, ICON, TYPE, CATEGORY,"
                  "EXEC, COMMENT, MD5, LAUNCH_TIMES, FAVORITES, LAUNCHED, TOP, LOCK,"
                  "DONT_DISPLAY, AUTO_START, START_UP_WMCLASS, KEYWORDS, LOCAL_KEYWORDS)"
                  " VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    const QVariantList values = {
        desktopFilePath, "2025-01-02 03:04:05", "2025-01-01 00:00:00", localName, nameEn, nameZh,
        pinyin, firstLetter, firstLetter, "application-x-executable", "Application", "Utility",
        "/bin/true", "comment", md5, launchTimes, favorites, launched, 0, lock, 0, 0, "wmclass",
        keywords, keywords
    };
    for (int i = 0; i < values.size(); ++i) {
        query.bindValue(i, values.at(i));
    }
    QVERIFY2(query.exec(), qPrintable(query.lastError().text()));
}

QVariant AppInfoTableWhiteboxTest::valueFor(const QString &desktopFilePath, const QString &field)
{
    QSqlQuery query(*m_table->d->m_database);
    query.prepare(QString("SELECT %1 FROM appInfo WHERE DESKTOP_FILE_PATH=?").arg(field));
    query.bindValue(0, desktopFilePath);
    if (!query.exec() || !query.next()) {
        return {};
    }
    return query.value(0);
}

void AppInfoTableWhiteboxTest::querySingleAndAllRowsFromTemporaryDatabase()
{
    ApplicationPropertyMap map;
    QVERIFY(m_table->query(map, "/usr/share/applications/editor.desktop",
                           {ApplicationProperty::LocalName, ApplicationProperty::LaunchTimes}));
    QCOMPARE(map.value(ApplicationProperty::LocalName).toString(), QString("Editor"));
    QCOMPARE(map.value(ApplicationProperty::LaunchTimes).toInt(), 20);

    ApplicationInfoMap all;
    QVERIFY(m_table->query(all, {ApplicationProperty::DesktopFilePath, ApplicationProperty::LocalName}));
    QVERIFY(all.contains("/usr/share/applications/editor.desktop"));
    QVERIFY(all.contains("/opt/apps/tools/calc.desktop"));
    QCOMPARE(all.size(), 2);
}

void AppInfoTableWhiteboxTest::queryWithRestrictionsKeywordsAndOrder()
{
    ApplicationInfoMap restricted;
    ApplicationPropertyMap restrictions;
    restrictions.insert(ApplicationProperty::Lock, 1);
    QVERIFY(m_table->query(restricted, {ApplicationProperty::LocalName, ApplicationProperty::Lock}, restrictions));
    QCOMPARE(restricted.size(), 1);
    QCOMPARE(restricted.first().value(ApplicationProperty::LocalName).toString(), QString("Calculator"));

    ApplicationInfoMap keywordResult;
    QVERIFY(m_table->query(keywordResult, {ApplicationProperty::LocalName, ApplicationProperty::DesktopFilePath},
                           QStringList{"edit"}, {}));
    QCOMPARE(keywordResult.size(), 1);
    QVERIFY(keywordResult.contains("/usr/share/applications/editor.desktop"));

    QList<ApplicationInfoMap> ordered;
    QVERIFY(m_table->query(ordered, {ApplicationProperty::LocalName, ApplicationProperty::LaunchTimes},
                           QStringList{"edit"}, {}));
    QCOMPARE(ordered.size(), 1);
    QCOMPARE(ordered.first().first().value(ApplicationProperty::LaunchTimes).toInt(), 20);
}

void AppInfoTableWhiteboxTest::queryEdgeCasesCoverFieldAndKeywordBranches()
{
    ApplicationPropertyMap emptyPropertiesResult;
    QVERIFY(m_table->query(emptyPropertiesResult, "/usr/share/applications/editor.desktop", {}));
    QVERIFY(emptyPropertiesResult.isEmpty());

    ApplicationInfoMap desktopPathRestricted;
    ApplicationPropertyMap unlockedRestriction;
    unlockedRestriction.insert(ApplicationProperty::Lock, 0);
    QVERIFY(m_table->query(desktopPathRestricted,
                           {ApplicationProperty::DesktopFilePath, ApplicationProperty::LocalName},
                           unlockedRestriction));
    QVERIFY(desktopPathRestricted.contains("/usr/share/applications/editor.desktop"));
    QCOMPARE(desktopPathRestricted.value("/usr/share/applications/editor.desktop")
                     .value(ApplicationProperty::DesktopFilePath)
                     .toString(),
             QString("/usr/share/applications/editor.desktop"));

    ApplicationInfoMap shortKeywordResult;
    QVERIFY(m_table->query(shortKeywordResult,
                           {ApplicationProperty::LocalName},
                           QStringList{"E"},
                           {}));
    QVERIFY(shortKeywordResult.contains("/usr/share/applications/editor.desktop"));

    ApplicationInfoMap restrictedKeywordResult;
    ApplicationPropertyMap launchedRestriction;
    launchedRestriction.insert(ApplicationProperty::Launched, 1);
    QVERIFY(m_table->query(restrictedKeywordResult,
                           {ApplicationProperty::DesktopFilePath, ApplicationProperty::LocalName},
                           QStringList{"edit"},
                           launchedRestriction));
    QCOMPARE(restrictedKeywordResult.size(), 1);
    QVERIFY(restrictedKeywordResult.contains("/usr/share/applications/editor.desktop"));

    QList<ApplicationInfoMap> orderedShortKeyword;
    QVERIFY(m_table->query(orderedShortKeyword,
                           {ApplicationProperty::DesktopFilePath, ApplicationProperty::LocalName},
                           QStringList{"E"},
                           {}));
    QVERIFY(!orderedShortKeyword.isEmpty());
    QCOMPARE(orderedShortKeyword.first().firstKey(), QString("/usr/share/applications/editor.desktop"));

    QList<ApplicationInfoMap> orderedRestrictedKeyword;
    QVERIFY(m_table->query(orderedRestrictedKeyword,
                           {ApplicationProperty::LocalName},
                           QStringList{"edit"},
                           launchedRestriction));
    QCOMPARE(orderedRestrictedKeyword.size(), 1);
}

void AppInfoTableWhiteboxTest::privateDedupHelpersCoverPriorityBranches()
{
    const QString configuredPath = m_homeDir.path() + "/share/applications/nested/demo.desktop";
    QCOMPARE(AppInfoTablePrivate::calcDesktopFileId(configuredPath), QString("nested-demo.desktop"));
    QCOMPARE(AppInfoTablePrivate::calcDesktopFileId("/opt/apps/tools/calc.desktop"), QString("calc.desktop"));
    QVERIFY(AppInfoTablePrivate::desktopPathPriority(configuredPath) >= 0);
    QCOMPARE(AppInfoTablePrivate::desktopPathPriority("/opt/apps/tools/calc.desktop"), -1);

    QHash<QString, QString> nameHash;
    ApplicationInfoMap infoMap;
    const QString md5 = QStringLiteral("duplicate-md5");
    const QString lowPriorityPath = QStringLiteral("/opt/apps/tools/duplicate.desktop");
    const QString highPriorityPath = m_homeDir.path() + "/share/applications/duplicate.desktop";
    const QString rejectedPath = QStringLiteral("/tmp/duplicate.desktop");

    QVERIFY(m_table->d->shouldInsert(nameHash, infoMap, md5, lowPriorityPath));
    infoMap.insert(lowPriorityPath, {});

    QVERIFY(m_table->d->shouldInsert(nameHash, infoMap, md5, highPriorityPath));
    QVERIFY(!infoMap.contains(lowPriorityPath));
    QCOMPARE(nameHash.value(md5), highPriorityPath);

    infoMap.insert(highPriorityPath, {});
    QVERIFY(!m_table->d->shouldInsert(nameHash, infoMap, md5, rejectedPath));
    QCOMPARE(nameHash.value(md5), highPriorityPath);
}

void AppInfoTableWhiteboxTest::privateMutationAndSearchHelpersUseDatabase()
{
    m_table->setAppFavoritesState(QStringLiteral("/opt/apps/tools/calc.desktop"), 4);
    m_table->setAppFavoritesState(QStringLiteral("/opt/apps/tools/calc.desktop"));
    m_table->setAppTopState(QStringLiteral("/opt/apps/tools/calc.desktop"), 3);
    m_table->setAppTopState(QStringLiteral("/opt/apps/tools/calc.desktop"));
    m_table->setAppLaunchedState(QStringLiteral("/opt/apps/tools/calc.desktop"), false);
    QCOMPARE(m_appDbManager.lastFavoritesOrder, 4U);
    QCOMPARE(m_appDbManager.favoritesNoOrderCalls, 1);
    QCOMPARE(m_appDbManager.lastTopOrder, 3U);
    QCOMPARE(m_appDbManager.topNoOrderCalls, 1);
    m_table->setAppLaunchedState(QStringLiteral("/opt/apps/tools/calc.desktop"), true);

    QString desktopFilePath;
    QVERIFY(m_table->tranPidToDesktopFp(123, desktopFilePath));
    QCOMPARE(desktopFilePath, QStringLiteral("/tmp/from-pid.desktop"));
    QCOMPARE(m_appDbManager.lastPid, 123U);
    QVERIFY(m_table->tranWinIdToDesktopFilePath(QVariant(456), desktopFilePath));
    QCOMPARE(desktopFilePath, QStringLiteral("/tmp/from-win.desktop"));
    QCOMPARE(m_appDbManager.lastWinId.toInt(), 456);
    QVERIFY(m_table->desktopFilePathFromName(QStringLiteral("calc"), desktopFilePath));
    QCOMPARE(desktopFilePath, QStringLiteral("/tmp/from-name.desktop"));
    QCOMPARE(m_appDbManager.lastDesktopName, QStringLiteral("calc"));

    QVERIFY(m_table->setAppLaunchTimes("/opt/apps/tools/calc.desktop", 7));
    QCOMPARE(valueFor("/opt/apps/tools/calc.desktop", "LAUNCH_TIMES").toInt(), 7);
    QCOMPARE(valueFor("/opt/apps/tools/calc.desktop", "LAUNCHED").toInt(), 1);

    QVERIFY(m_table->updateAppLaunchTimes("/opt/apps/tools/calc.desktop"));
    QCOMPARE(valueFor("/opt/apps/tools/calc.desktop", "LAUNCH_TIMES").toInt(), 8);

    QVERIFY(m_table->setAppLockState("/opt/apps/tools/calc.desktop", 0));
    QCOMPARE(valueFor("/opt/apps/tools/calc.desktop", "LOCK").toInt(), 0);

    QString keyword("bjq");
    QStringList flatResult;
    QVERIFY(m_table->searchInstallApp(keyword, flatResult));
    QVERIFY(flatResult.contains("/usr/share/applications/editor.desktop"));

    QStringList keywords({"Calc", "jsq"});
    QStringList multiResult;
    QVERIFY(m_table->searchInstallApp(keywords, multiResult));
    QVERIFY(multiResult.contains("/opt/apps/tools/calc.desktop"));

    QString shortKeyword("E");
    QStringList shortResult;
    QVERIFY(m_table->searchInstallApp(shortKeyword, shortResult));

    QStringList mixedKeywords({"E", "Calc", "jsq"});
    QStringList mixedResult;
    QVERIFY(m_table->searchInstallApp(mixedKeywords, mixedResult));
}

void AppInfoTableWhiteboxTest::openDatabaseSqlFailureBranchesReturnFalse()
{
    ApplicationInfoMap infoMap;
    ApplicationPropertyMap emptyRestrictions;
    QVERIFY(!m_table->query(infoMap, {ApplicationProperty::LocalName}, emptyRestrictions));
    QVERIFY(!m_table->query(infoMap, {ApplicationProperty::LocalName}, QStringList{}, emptyRestrictions));

    QList<ApplicationInfoMap> ordered;
    QVERIFY(!m_table->query(ordered, {ApplicationProperty::LocalName}, QStringList{}, emptyRestrictions));

    QVERIFY(!m_table->updateAppLaunchTimes(QStringLiteral("/tmp/missing.desktop")));
}

void AppInfoTableWhiteboxTest::databaseFailureBranchesReturnFalse()
{
    m_table->d->m_database->close();

    ApplicationPropertyMap propertyMap;
    QVERIFY(!m_table->query(propertyMap, QStringLiteral("/opt/apps/tools/calc.desktop"),
                            {ApplicationProperty::LocalName}));

    ApplicationInfoMap infoMap;
    QVERIFY(!m_table->query(infoMap, {ApplicationProperty::LocalName}));

    ApplicationPropertyMap restrictions;
    restrictions.insert(ApplicationProperty::Lock, 0);
    QVERIFY(!m_table->query(infoMap, {ApplicationProperty::LocalName}, restrictions));
    QVERIFY(!m_table->query(infoMap, {ApplicationProperty::LocalName}, QStringList{"Calc"}, restrictions));

    QList<ApplicationInfoMap> ordered;
    QVERIFY(!m_table->query(ordered, {ApplicationProperty::LocalName}, QStringList{"Calc"}, restrictions));

    QVERIFY(!m_table->setAppLaunchTimes(QStringLiteral("/opt/apps/tools/calc.desktop"), 1));
    QVERIFY(!m_table->updateAppLaunchTimes(QStringLiteral("/opt/apps/tools/calc.desktop")));
    QVERIFY(!m_table->setAppLockState(QStringLiteral("/opt/apps/tools/calc.desktop"), 1));

    QString keyword("Calc");
    QStringList installAppInfo;
    QVERIFY(!m_table->searchInstallApp(keyword, installAppInfo));

    QStringList keywords({"Calc"});
    QVERIFY(!m_table->searchInstallApp(keywords, installAppInfo));

    bindTableToTemporaryDatabase();
}

void AppInfoTableWhiteboxTest::signalForwardingSlotsRelayPublicSignals()
{
    QSignalSpy updateSpy(m_table, &AppInfoTable::appDBItems2BUpdate);
    QSignalSpy updateAllSpy(m_table, &AppInfoTable::appDBItems2BUpdateAll);
    QSignalSpy addSpy(m_table, &AppInfoTable::appDBItems2BAdd);
    QSignalSpy deleteSpy(m_table, &AppInfoTable::appDBItems2BDelete);
    QVERIFY(updateSpy.isValid());
    QVERIFY(updateAllSpy.isValid());
    QVERIFY(addSpy.isValid());
    QVERIFY(deleteSpy.isValid());

    ApplicationInfoMap info;
    info["/tmp/app.desktop"].insert(ApplicationProperty::LocalName, "Tmp");
    m_table->d->sendAppDBItemsUpdate(info);
    m_table->d->sendAppDBItemsUpdateAll({"/tmp/all.desktop"});
    m_table->d->sendAppDBItemsAdd({"/tmp/add.desktop"});
    m_table->d->sendAppDBItemsDelete({"/tmp/delete.desktop"});

    QCOMPARE(updateSpy.size(), 1);
    QCOMPARE(updateAllSpy.size(), 1);
    QCOMPARE(addSpy.size(), 1);
    QCOMPARE(deleteSpy.size(), 1);
}

void AppInfoTableWhiteboxTest::applicationInfoFacadeForwardsQueriesAndMutations()
{
    AppInfoTable &singletonTable = AppInfoTable::instance();
    singletonTable.d->m_database->close();
    delete singletonTable.d->m_database;
    const QString connectionName = QUuid::createUuid().toString(QUuid::WithoutBraces);
    singletonTable.d->m_ConnectionName = connectionName;
    singletonTable.d->m_database = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", connectionName));
    singletonTable.d->m_database->setDatabaseName(m_databasePath);
    QVERIFY2(singletonTable.d->m_database->open(), qPrintable(singletonTable.d->m_database->lastError().text()));

    ApplicationInfo info;
    QCOMPARE(info.getInfo("/usr/share/applications/editor.desktop", ApplicationProperty::LocalName).toString(),
             QString("Editor"));

    ApplicationPropertyMap oneApp = info.getInfo("/opt/apps/tools/calc.desktop",
                                                 {ApplicationProperty::DesktopFilePath, ApplicationProperty::Lock});
    QCOMPARE(oneApp.value(ApplicationProperty::Lock).toInt(), 0);

    ApplicationInfoMap all = info.getInfo({ApplicationProperty::LocalName});
    QVERIFY(all.contains("/usr/share/applications/editor.desktop"));

    ApplicationPropertyMap restrictions;
    restrictions.insert(ApplicationProperty::Launched, 1);
    ApplicationInfoMap restricted = info.getInfo({ApplicationProperty::LocalName, ApplicationProperty::Launched}, restrictions);
    QVERIFY(restricted.contains("/usr/share/applications/editor.desktop"));

    ApplicationInfoMap singleKeyword = info.searchApp({ApplicationProperty::LocalName}, "edit", {});
    QVERIFY(singleKeyword.contains("/usr/share/applications/editor.desktop"));

    ApplicationInfoMap multiKeyword = info.searchApp({ApplicationProperty::LocalName}, QStringList{"Calc", "jsq"}, {});
    QVERIFY(multiKeyword.contains("/opt/apps/tools/calc.desktop"));

    QList<ApplicationInfoMap> ordered = info.searchAppWithOrder({ApplicationProperty::LocalName, ApplicationProperty::LaunchTimes},
                                                                QStringList{"edit"}, {});
    QCOMPARE(ordered.size(), 1);

    info.setFavoritesOfApp("/opt/apps/tools/calc.desktop", 4);
    info.setAppToFavorites("/opt/apps/tools/calc.desktop");
    info.removeAppFromFavorites("/opt/apps/tools/calc.desktop");
    info.setTopOfApp("/opt/apps/tools/calc.desktop", 3);
    info.setAppToTop("/opt/apps/tools/calc.desktop");
    info.setAppLaunchedState("/opt/apps/tools/calc.desktop", false);

    QString desktopFilePath;
    const bool pidLookupResult = info.tranPidToDesktopFp(-1, desktopFilePath);
    const bool zeroPidLookupResult = info.tranPidToDesktopFp(static_cast<uint>(0), desktopFilePath);
    const bool nameLookupResult = info.desktopFilePathFromName("calc", desktopFilePath);
    const bool winIdLookupResult = info.tranWinIdToDesktopFilePath(QVariant(), desktopFilePath);
    Q_UNUSED(pidLookupResult)
    Q_UNUSED(zeroPidLookupResult)
    Q_UNUSED(nameLookupResult)
    Q_UNUSED(winIdLookupResult)
}

QTEST_MAIN(AppInfoTableWhiteboxTest)
#include "app-info-table-whitebox-test.moc"
