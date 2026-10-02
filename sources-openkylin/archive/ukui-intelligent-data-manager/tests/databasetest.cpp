/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include <QtTest>
#include <QObject>

#include "databasemanager.h"

class DatabaseTest : public QObject
{
    Q_OBJECT
public:
    DatabaseTest() {}

private Q_SLOTS:
    void initTestCase();
    void test1InitState();
    void test2CreateSpace();
    void test3SetSpaceCondition();
    void test4GetSpaceCondition();
    void test5addFileToSpace();
    void test6removeFileFromSpace();
    void test7RenameSpace();
    void test8RemoveSpace();
    void test9getFilesFromSpaces();

    void cleanupTestCase();

private:
    DatabaseManager *manager;
};



void DatabaseTest::initTestCase()
{
    QDir d(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    d.remove("dbtest.db");
    manager = new DatabaseManager("dbtest.db");
}

void DatabaseTest::test1InitState()
{
    QString errMsg;
    QStringList spaces = manager->getAllSpaceNames(&errMsg);
    QCOMPARE(spaces.count() > 0, true);
    QCOMPARE(errMsg.isEmpty(), true);
}

void DatabaseTest::test2CreateSpace()
{
    QString errMsg;
    bool ok = manager->createSpace("TEST1", QStringList()<<"aaa"<<"bbb", &errMsg);
    QCOMPARE(ok, true);
    QCOMPARE(errMsg.isEmpty(), true);
    qDebug()<<manager->getAllSpaceNames();
}

void DatabaseTest::test3SetSpaceCondition()
{
    QString errMsg;
    bool ok = manager->changeSpaceConditions("TEST1", QStringList()<<"xxx"<<"yyy", &errMsg);
    QCOMPARE(ok, true);
    QCOMPARE(errMsg.isEmpty(), true);
}

void DatabaseTest::test4GetSpaceCondition()
{
    QString errMsg;
    QStringList conditions = manager->getSpaceCondition("TEST1", &errMsg);
    qDebug()<<conditions;
    QCOMPARE(errMsg.isEmpty(), true);
}

void DatabaseTest::test5addFileToSpace()
{
    QString errMsg;
    bool ok = manager->addFileToSpace("/home", "TEST1", true, &errMsg);
    QCOMPARE(ok, true);
    QCOMPARE(errMsg.isEmpty(), true);
    errMsg.clear();
    ok = manager->addFileToSpace("/home", "TEST1", false, &errMsg);
    QCOMPARE(ok, true);
    QCOMPARE(errMsg.isEmpty(), true);
}

void DatabaseTest::test6removeFileFromSpace()
{
    QString errMsg;
    bool ok = manager->removeFileFromSpace("/home", "TEST1", true, &errMsg);
    QCOMPARE(ok, true);
    QCOMPARE(errMsg.isEmpty(), true);
    errMsg.clear();
    ok = manager->removeFileFromSpace("/home", "TEST1", false, &errMsg);
    QCOMPARE(ok, true);
    QCOMPARE(errMsg.isEmpty(), true);
}

void DatabaseTest::test7RenameSpace()
{
    QString errMsg;
    bool ok = manager->renameSpace("TEST1", "TEST2", &errMsg);
    QCOMPARE(ok, true);
    QCOMPARE(errMsg.isEmpty(), true);
    qDebug()<<manager->getAllSpaceNames();
}

void DatabaseTest::test8RemoveSpace()
{
    QString errMsg;
    bool ok = manager->removeSpace("TEST2", &errMsg);
    QCOMPARE(ok, true);
    QCOMPARE(errMsg.isEmpty(), true);
    qDebug()<<manager->getAllSpaceNames();
}

void DatabaseTest::test9getFilesFromSpaces()
{
    QVERIFY(manager->createSpace("UserAddedSpace", QStringList()));
    QVERIFY(manager->createSpace("AutoAddedSpace", QStringList()));

    QCOMPARE(manager->addFileToSpace("/1", "UserAddedSpace", true), true);
    QCOMPARE(manager->addFileToSpace("/1", "AutoAddedSpace", false), true);
    QCOMPARE(manager->addFileToSpace("/2", "UserAddedSpace", true), true);
    QCOMPARE(manager->addFileToSpace("/2", "AutoAddedSpace", false), true);
    QCOMPARE(manager->addFileToSpace("/3", "UserAddedSpace", true), true);

    QVERIFY(manager->addFileToSpace("/4", "UserAddedSpace", true));
    QVERIFY(manager->addFileToSpace("/4", "AutoAddedSpace", true));
    QVERIFY(manager->addFileToSpace("/4", "UserAddedSpace", false));
    QVERIFY(manager->addFileToSpace("/4", "AutoAddedSpace", false));

    QString errMsg;
    QStringList allFiles = manager->getAllFilesFromSpaces(true, &errMsg);
    QCOMPARE(errMsg.isEmpty(), true);
    qDebug() << "user added files" << allFiles;
    errMsg.clear();
    allFiles = manager->getAllFilesFromSpaces(false, &errMsg);
    QCOMPARE(errMsg.isEmpty(), true);
    qDebug() << "auto added files" << allFiles;
    errMsg.clear();
    auto filesFromSpaces = manager->getAllFilesFromSpacesBySpaceIds(true, &errMsg);
    QCOMPARE(errMsg.isEmpty(), true);
    qDebug() << "user added files" <<filesFromSpaces;
    errMsg.clear();
    filesFromSpaces = manager->getAllFilesFromSpacesBySpaceIds(false, &errMsg);
    QCOMPARE(errMsg.isEmpty(), true);
    qDebug() << "auto added files" << filesFromSpaces;
    errMsg.clear();
    auto filesWithSpaceIds = manager->getAllFilesWithSpaceIds(true, &errMsg);
    QCOMPARE(errMsg.isEmpty(), true);
    qDebug() << "files with spaceids in user added files" << filesWithSpaceIds;
    errMsg.clear();
    filesWithSpaceIds = manager->getAllFilesWithSpaceIds(false, &errMsg);
    QCOMPARE(errMsg.isEmpty(), true);
    qDebug() << "files with spaceids in auto added files" << filesWithSpaceIds;

    QVERIFY(manager->removeSpace("UserAddedSpace"));
    QVERIFY(manager->removeSpace("AutoAddedSpace"));
}

void DatabaseTest::cleanupTestCase()
{
    delete manager;
    QDir d(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    QString dbPath = d.absoluteFilePath("dbtest.db");
    d.remove(dbPath);
}

QTEST_MAIN(DatabaseTest)
#include "databasetest.moc"
