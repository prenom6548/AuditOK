/*
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QtTest>

#include "app-db-utils.h"
#include "application-property.h"
#include "sqlite-database-utils.h"

using namespace UkuiSearch;

namespace {

QSqlDatabase openTestDatabase(const QString &connectionName, const QString &databasePath)
{
    QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
    database.setDatabaseName(databasePath);
    configureSqliteDatabase(database, SqliteDatabaseMode::FileBacked);
    return database;
}

void createRepairTable(QSqlDatabase &database)
{
    QSqlQuery query(database);
    QVERIFY(query.exec(QStringLiteral(
        "CREATE TABLE appInfo("
        "DESKTOP_FILE_PATH TEXT PRIMARY KEY NOT NULL,"
        "LAUNCH_TIMES INT,"
        "FAVORITES INT,"
        "LAUNCHED INT,"
        "TOP INT,"
        "LOCK INT)")));
}

void createValueSetTable(QSqlDatabase &database)
{
    QSqlQuery query(database);
    QVERIFY(query.exec(QStringLiteral(
        "CREATE TABLE appInfo("
        "DESKTOP_FILE_PATH TEXT PRIMARY KEY NOT NULL,"
        "MODIFYED_TIME TEXT,"
        "LOCAL_NAME TEXT,"
        "LOCK INT)")));
}

void createTopTable(QSqlDatabase &database)
{
    QSqlQuery query(database);
    QVERIFY(query.exec(QStringLiteral(
        "CREATE TABLE appInfo("
        "DESKTOP_FILE_PATH TEXT PRIMARY KEY NOT NULL,"
        "MODIFYED_TIME TEXT,"
        "TOP INT)")));
}

} // namespace

class AppDbUtilsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void fileBackedConnectionsEnableWalAndBusyTimeout();
    void walKeepsWriterCommitsWorkingDuringActiveReads();
    void repairMalformedDesktopPathsMergesDuplicateRows();
    void repairMalformedDesktopPathsContinuesAfterSingleRowFailure();
    void topStateUpdateMovesExistingTopWithoutLeakingTransaction();
    void topStateUpdateInsertsNewTopWithoutLeakingTransaction();
    void topStateUpdateClearsTopWithoutLeakingTransaction();
    void valueSetFailuresRollbackSingleDesktopOnly();
};

void AppDbUtilsTest::fileBackedConnectionsEnableWalAndBusyTimeout()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString connectionName = QStringLiteral("app-db-config");
    const QString databasePath = QDir(tempDir.path()).filePath(QStringLiteral("app-info.db"));
    {
        auto database = openTestDatabase(connectionName, databasePath);
        QVERIFY(database.isOpen());

        QSqlQuery query(database);
        QVERIFY(query.exec(QStringLiteral("PRAGMA journal_mode")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString().toLower(), QStringLiteral("wal"));

        QVERIFY(query.exec(QStringLiteral("PRAGMA busy_timeout")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), defaultSqliteBusyTimeoutMs());

        database.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

void AppDbUtilsTest::walKeepsWriterCommitsWorkingDuringActiveReads()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString databasePath = QDir(tempDir.path()).filePath(QStringLiteral("app-info.db"));
    {
        auto writerDatabase = openTestDatabase(QStringLiteral("app-db-writer"), databasePath);
        auto readerDatabase = openTestDatabase(QStringLiteral("app-db-reader"), databasePath);
        QVERIFY(writerDatabase.isOpen());
        QVERIFY(readerDatabase.isOpen());

        QSqlQuery writerQuery(writerDatabase);
        QVERIFY(writerQuery.exec(QStringLiteral("CREATE TABLE appInfo(id INT PRIMARY KEY, value TEXT)")));
        QVERIFY(writerQuery.exec(QStringLiteral("INSERT INTO appInfo VALUES(1, 'before')")));

        QSqlQuery readerQuery(readerDatabase);
        readerQuery.setForwardOnly(true);
        QVERIFY(readerQuery.exec(QStringLiteral("SELECT value FROM appInfo WHERE id = 1")));
        QVERIFY(readerQuery.next());
        QCOMPARE(readerQuery.value(0).toString(), QStringLiteral("before"));

        QVERIFY(writerDatabase.transaction());
        writerQuery.prepare(QStringLiteral("UPDATE appInfo SET value = 'after' WHERE id = 1"));
        QVERIFY(writerQuery.exec());
        QVERIFY(writerDatabase.commit());

        QSqlQuery checkQuery(writerDatabase);
        QVERIFY(checkQuery.exec(QStringLiteral("SELECT value FROM appInfo WHERE id = 1")));
        QVERIFY(checkQuery.next());
        QCOMPARE(checkQuery.value(0).toString(), QStringLiteral("after"));

        writerDatabase.close();
        readerDatabase.close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("app-db-writer"));
    QSqlDatabase::removeDatabase(QStringLiteral("app-db-reader"));
}

void AppDbUtilsTest::repairMalformedDesktopPathsMergesDuplicateRows()
{
    {
        QSqlDatabase database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("app-db-repair"));
        database.setDatabaseName(QStringLiteral(":memory:"));
        configureSqliteDatabase(database, SqliteDatabaseMode::InMemory);
        QVERIFY(database.isOpen());
        createRepairTable(database);

        QSqlQuery query(database);
        QVERIFY(query.exec(QStringLiteral(
            "INSERT INTO appInfo VALUES"
            "('/usr/share//applications/demo.desktop', 2, 3, 0, 4, 0),"
            "('/usr/share/applications/demo.desktop', 5, 1, 1, 2, 1)")));

        bool repaired = false;
        QVERIFY(repairMalformedDesktopPaths(database, &repaired));
        QVERIFY(repaired);

        QVERIFY(query.exec(QStringLiteral(
            "SELECT DESKTOP_FILE_PATH, LAUNCH_TIMES, FAVORITES, LAUNCHED, TOP, LOCK "
            "FROM appInfo")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/usr/share/applications/demo.desktop"));
        QCOMPARE(query.value(1).toInt(), 7);
        QCOMPARE(query.value(2).toInt(), 1);
        QCOMPARE(query.value(3).toInt(), 1);
        QCOMPARE(query.value(4).toInt(), 2);
        QCOMPARE(query.value(5).toInt(), 1);
        QVERIFY(!query.next());

        database.close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("app-db-repair"));
}

void AppDbUtilsTest::repairMalformedDesktopPathsContinuesAfterSingleRowFailure()
{
    {
        QSqlDatabase database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("app-db-repair-continue"));
        database.setDatabaseName(QStringLiteral(":memory:"));
        configureSqliteDatabase(database, SqliteDatabaseMode::InMemory);
        QVERIFY(database.isOpen());
        createRepairTable(database);

        QSqlQuery query(database);
        QVERIFY(query.exec(QStringLiteral(
            "INSERT INTO appInfo VALUES"
            "('/broken//path.desktop', 1, 0, 0, 0, 0),"
            "('/keep//path.desktop', 2, 0, 0, 0, 0)")));
        QVERIFY(query.exec(QStringLiteral(
            "CREATE TRIGGER break_broken_path_update "
            "BEFORE UPDATE ON appInfo "
            "WHEN OLD.DESKTOP_FILE_PATH = '/broken//path.desktop' "
            "BEGIN "
            "SELECT RAISE(FAIL, 'broken row'); "
            "END")));

        bool repaired = false;
        QVERIFY(!repairMalformedDesktopPaths(database, &repaired));
        QVERIFY(repaired);

        QVERIFY(query.exec(QStringLiteral(
            "SELECT DESKTOP_FILE_PATH FROM appInfo ORDER BY DESKTOP_FILE_PATH")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/broken//path.desktop"));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/keep/path.desktop"));
        QVERIFY(!query.next());

        database.close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("app-db-repair-continue"));
}

void AppDbUtilsTest::topStateUpdateMovesExistingTopWithoutLeakingTransaction()
{
    {
        QSqlDatabase database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("app-db-top-move"));
        database.setDatabaseName(QStringLiteral(":memory:"));
        configureSqliteDatabase(database, SqliteDatabaseMode::InMemory);
        QVERIFY(database.isOpen());
        createTopTable(database);

        QSqlQuery query(database);
        QVERIFY(query.exec(QStringLiteral(
            "INSERT INTO appInfo VALUES"
            "('/apps/alpha.desktop', '', 1),"
            "('/apps/beta.desktop', '', 2),"
            "('/apps/gamma.desktop', '', 3)")));

        ApplicationInfoMap updatedInfo;
        QVERIFY(applyTopStateUpdate(database, QStringLiteral("/apps/alpha.desktop"), 3, &updatedInfo));
        QCOMPARE(updatedInfo[QStringLiteral("/apps/alpha.desktop")]
                     .value(ApplicationProperty::Top)
                     .toUInt(),
                 3U);
        QCOMPARE(updatedInfo[QStringLiteral("/apps/beta.desktop")]
                     .value(ApplicationProperty::Top)
                     .toUInt(),
                 1U);
        QCOMPARE(updatedInfo[QStringLiteral("/apps/gamma.desktop")]
                     .value(ApplicationProperty::Top)
                     .toUInt(),
                 2U);

        QVERIFY(query.exec(QStringLiteral(
            "SELECT DESKTOP_FILE_PATH, TOP FROM appInfo ORDER BY TOP")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/apps/beta.desktop"));
        QCOMPARE(query.value(1).toUInt(), 1U);
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/apps/gamma.desktop"));
        QCOMPARE(query.value(1).toUInt(), 2U);
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/apps/alpha.desktop"));
        QCOMPARE(query.value(1).toUInt(), 3U);
        QVERIFY(!query.next());

        QVERIFY(database.transaction());
        database.rollback();
        database.close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("app-db-top-move"));
}

void AppDbUtilsTest::topStateUpdateInsertsNewTopWithoutLeakingTransaction()
{
    {
        QSqlDatabase database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("app-db-top-insert"));
        database.setDatabaseName(QStringLiteral(":memory:"));
        configureSqliteDatabase(database, SqliteDatabaseMode::InMemory);
        QVERIFY(database.isOpen());
        createTopTable(database);

        QSqlQuery query(database);
        QVERIFY(query.exec(QStringLiteral(
            "INSERT INTO appInfo VALUES"
            "('/apps/alpha.desktop', '', 1),"
            "('/apps/beta.desktop', '', 2),"
            "('/apps/gamma.desktop', '', 0)")));

        ApplicationInfoMap updatedInfo;
        QVERIFY(applyTopStateUpdate(database, QStringLiteral("/apps/gamma.desktop"), 1, &updatedInfo));
        QCOMPARE(updatedInfo[QStringLiteral("/apps/gamma.desktop")]
                     .value(ApplicationProperty::Top)
                     .toUInt(),
                 1U);
        QCOMPARE(updatedInfo[QStringLiteral("/apps/alpha.desktop")]
                     .value(ApplicationProperty::Top)
                     .toUInt(),
                 2U);
        QCOMPARE(updatedInfo[QStringLiteral("/apps/beta.desktop")]
                     .value(ApplicationProperty::Top)
                     .toUInt(),
                 3U);

        QVERIFY(query.exec(QStringLiteral(
            "SELECT DESKTOP_FILE_PATH, TOP FROM appInfo ORDER BY TOP, DESKTOP_FILE_PATH")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/apps/gamma.desktop"));
        QCOMPARE(query.value(1).toUInt(), 1U);
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/apps/alpha.desktop"));
        QCOMPARE(query.value(1).toUInt(), 2U);
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/apps/beta.desktop"));
        QCOMPARE(query.value(1).toUInt(), 3U);
        QVERIFY(!query.next());

        QVERIFY(database.transaction());
        database.rollback();
        database.close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("app-db-top-insert"));
}

void AppDbUtilsTest::topStateUpdateClearsTopWithoutLeakingTransaction()
{
    {
        QSqlDatabase database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("app-db-top-clear"));
        database.setDatabaseName(QStringLiteral(":memory:"));
        configureSqliteDatabase(database, SqliteDatabaseMode::InMemory);
        QVERIFY(database.isOpen());
        createTopTable(database);

        QSqlQuery query(database);
        QVERIFY(query.exec(QStringLiteral(
            "INSERT INTO appInfo VALUES"
            "('/apps/alpha.desktop', '', 1),"
            "('/apps/beta.desktop', '', 2),"
            "('/apps/gamma.desktop', '', 3)")));

        ApplicationInfoMap updatedInfo;
        QVERIFY(applyTopStateUpdate(database, QStringLiteral("/apps/beta.desktop"), 0, &updatedInfo));
        QCOMPARE(updatedInfo[QStringLiteral("/apps/beta.desktop")]
                     .value(ApplicationProperty::Top)
                     .toUInt(),
                 0U);
        QCOMPARE(updatedInfo[QStringLiteral("/apps/gamma.desktop")]
                     .value(ApplicationProperty::Top)
                     .toUInt(),
                 2U);

        QVERIFY(query.exec(QStringLiteral(
            "SELECT DESKTOP_FILE_PATH, TOP FROM appInfo ORDER BY TOP DESC, DESKTOP_FILE_PATH")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/apps/gamma.desktop"));
        QCOMPARE(query.value(1).toUInt(), 2U);
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/apps/alpha.desktop"));
        QCOMPARE(query.value(1).toUInt(), 1U);
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/apps/beta.desktop"));
        QCOMPARE(query.value(1).toUInt(), 0U);
        QVERIFY(!query.next());

        QVERIFY(database.transaction());
        database.rollback();
        database.close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("app-db-top-clear"));
}

void AppDbUtilsTest::valueSetFailuresRollbackSingleDesktopOnly()
{
    {
        QSqlDatabase database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("app-db-values"));
        database.setDatabaseName(QStringLiteral(":memory:"));
        configureSqliteDatabase(database, SqliteDatabaseMode::InMemory);
        QVERIFY(database.isOpen());
        createValueSetTable(database);

        QSqlQuery query(database);
        QVERIFY(query.exec(QStringLiteral(
            "INSERT INTO appInfo VALUES"
            "('/apps/alpha.desktop', '', 'alpha', 0),"
            "('/apps/beta.desktop', '', 'beta', 0)")));

        ApplicationInfoMap updates;
        updates[QStringLiteral("/apps/alpha.desktop")]
            .insert(ApplicationProperty::LocalName, QStringLiteral("alpha-updated"));
        updates[QStringLiteral("/apps/beta.desktop")]
            .insert(ApplicationProperty::Invalid, 1);

        ApplicationInfoMap appliedUpdates;
        QVERIFY(!applyValueSetUpdates(database, updates, &appliedUpdates));
        QCOMPARE(appliedUpdates.size(), 1);
        QCOMPARE(appliedUpdates[QStringLiteral("/apps/alpha.desktop")]
                     .value(ApplicationProperty::LocalName)
                     .toString(),
                 QStringLiteral("alpha-updated"));

        QVERIFY(query.exec(QStringLiteral(
            "SELECT DESKTOP_FILE_PATH, LOCAL_NAME FROM appInfo ORDER BY DESKTOP_FILE_PATH")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/apps/alpha.desktop"));
        QCOMPARE(query.value(1).toString(), QStringLiteral("alpha-updated"));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/apps/beta.desktop"));
        QCOMPARE(query.value(1).toString(), QStringLiteral("beta"));
        QVERIFY(!query.next());

        database.close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("app-db-values"));
}

QTEST_GUILESS_MAIN(AppDbUtilsTest)

#include "app-db-utils-test.moc"
