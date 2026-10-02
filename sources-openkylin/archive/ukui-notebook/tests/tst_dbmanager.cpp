// tst_dbmanager.cpp
#include "tst_dbmanager.h"
#include <QSignalSpy>

void TestDBManager::init()
{
    dbManager = new DBManager(this);
    // 使用内存数据库进行测试
    dbManager->open(":memory:", true);

    // 创建测试数据
    testNote = new NoteData(this);
    testNote->setId(1);
    testNote->setFullTitle("Test Note");
    testNote->setContent("Test Content");
    testNote->setCreationDateTime(QDateTime::currentDateTime());
    testNote->setLastModificationDateTime(QDateTime::currentDateTime());
}

void TestDBManager::cleanup()
{
    delete dbManager;
}

void TestDBManager::testOpenAndCreateTables()
{
    DBManager* db = new DBManager(this);
    db->open(":memory:", true);

    // 验证能获取最后一行ID（说明表创建成功）
    QCOMPARE(db->getLastRowID(), 0);

    delete db;
}

void TestDBManager::testAddNote()
{
    bool success = dbManager->addNote(testNote);
    QVERIFY(success);

    // 验证笔记确实被添加
    QVERIFY(dbManager->isNoteExist(testNote));
    QCOMPARE(dbManager->getLastRowID(), 1);
}

void TestDBManager::testGetNote()
{
    dbManager->addNote(testNote);

    NoteData* retrievedNote = dbManager->getNote("note_1");
    QVERIFY(retrievedNote != nullptr);
    QCOMPARE(retrievedNote->fullTitle(), testNote->fullTitle());
    QCOMPARE(retrievedNote->content(), testNote->content());
}

void TestDBManager::testGetAllNotes()
{
    dbManager->addNote(testNote);

    NoteData* note2 = new NoteData(this);
    note2->setId(2);
    note2->setFullTitle("Test Note 2");
    note2->setContent("Test Content 2");
    dbManager->addNote(note2);

    QList<NoteData*> notes = dbManager->getAllNotes();
    QCOMPARE(notes.size(), 2);
}

void TestDBManager::testUpdateNote()
{
    dbManager->addNote(testNote);

    testNote->setContent("Updated Content");
    bool success = dbManager->updateNote(testNote);
    QVERIFY(success);

    NoteData* updatedNote = dbManager->getNote("note_1");
    QCOMPARE(updatedNote->content(), QString("Updated Content"));
}

void TestDBManager::testRemoveNote()
{
    dbManager->addNote(testNote);
    bool success = dbManager->removeNote(testNote);
    QVERIFY(success);
    QVERIFY(!dbManager->isNoteExist(testNote));
}

void TestDBManager::testPermanentlyRemoveAllNotes()
{
    dbManager->addNote(testNote);
    NoteData* note2 = new NoteData(this);
    note2->setId(2);
    dbManager->addNote(note2);

    bool success = dbManager->permanantlyRemoveAllNotes();
    QVERIFY(success);
    QCOMPARE(dbManager->getAllNotes().size(), 0);
}

void TestDBManager::testForceLastRowIndexValue()
{
    bool success = dbManager->forceLastRowIndexValue(10);
    QVERIFY(success);
    QCOMPARE(dbManager->getLastRowID(), 10);
}

void TestDBManager::testMigrateNote()
{
    bool success = dbManager->migrateNote(testNote);
    QVERIFY(success);
    QVERIFY(dbManager->isNoteExist(testNote));
}

void TestDBManager::testSignals()
{
    QSignalSpy spy(dbManager, SIGNAL(notesReceived(QList<NoteData*>, int)));

    dbManager->addNote(testNote);
    dbManager->onNotesListRequested();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(1).toInt(), 1); // noteCounter
}

#include "moc_tst_dbmanager.cpp"
