#include "pending-file-test.h"

#include <QTest>

#include "pending-file.h"

using namespace UkuiSearch;

void PendingFileTest::testDefaultStateAndSetters()
{
    PendingFile file(QStringLiteral("/tmp/demo"));

    QCOMPARE(file.path(), QStringLiteral("/tmp/demo"));
    QVERIFY(!file.shouldRemoveIndex());
    QVERIFY(!file.isDir());
    QVERIFY(!file.isCreated());
    QVERIFY(!file.isModified());
    QVERIFY(!file.isMoveTo());

    file.setPath(QStringLiteral("/tmp/changed"));
    file.setModified();
    file.setCreated();
    file.setMoveTo();
    file.setIsDir();

    QCOMPARE(file.path(), QStringLiteral("/tmp/changed"));
    QVERIFY(file.isModified());
    QVERIFY(file.isCreated());
    QVERIFY(file.isMoveTo());
    QVERIFY(file.isDir());

    file.setDeleted();
    QVERIFY(file.shouldRemoveIndex());
}

void PendingFileTest::testMergeKeepsLatestDeleteAndFlags()
{
    PendingFile base(QStringLiteral("/tmp/file.txt"));
    base.setCreated();
    base.setModified();

    PendingFile update(QStringLiteral("/tmp/file.txt"));
    update.setMoveTo();
    update.setDeleted();

    base.merge(update);

    QVERIFY(base.isCreated());
    QVERIFY(base.isModified());
    QVERIFY(base.isMoveTo());
    QVERIFY(base.shouldRemoveIndex());
}

void PendingFileTest::testTrailingSlashAndBooleanMergeBranches()
{
    PendingFile file(QStringLiteral("/tmp/original"));
    file.setPath(QStringLiteral("/tmp/folder/"));
    QVERIFY(file.path().startsWith(QStringLiteral("/tmp/folder")));

    file.setIsDir(false);
    QVERIFY(!file.isDir());
    file.setIsDir(true);
    QVERIFY(file.isDir());

    PendingFile base(QStringLiteral("/tmp/file.txt"));
    PendingFile emptyUpdate(QStringLiteral("/tmp/file.txt"));
    base.merge(emptyUpdate);
    QVERIFY(!base.isCreated());
    QVERIFY(!base.isModified());
    QVERIFY(!base.isMoveTo());
    QVERIFY(!base.shouldRemoveIndex());

    PendingFile createdUpdate(QStringLiteral("/tmp/file.txt"));
    createdUpdate.setCreated();
    createdUpdate.setModified();
    createdUpdate.setMoveTo();
    base.merge(createdUpdate);
    QVERIFY(base.isCreated());
    QVERIFY(base.isModified());
    QVERIFY(base.isMoveTo());
    QVERIFY(!base.shouldRemoveIndex());
}

void PendingFileTest::testEqualityUsesPathOnly()
{
    PendingFile a(QStringLiteral("/tmp/same"));
    a.setDeleted();

    PendingFile b(QStringLiteral("/tmp/same"));
    b.setCreated();
    b.setIsDir();

    PendingFile c(QStringLiteral("/tmp/other"));

    QVERIFY(a == b);
    QVERIFY(!(a == c));
}

QTEST_MAIN(PendingFileTest)
