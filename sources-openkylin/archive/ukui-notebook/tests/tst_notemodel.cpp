// tst_notemodel.cpp
#include "tst_notemodel.h"
#include <QSignalSpy>

void TestNoteModel::init()
{
    model = new NoteModel(this);
    // 创建一些测试用的笔记数据
    note1 = new NoteData(this);
    note1->setId(1);
    note1->setFullTitle("Test Note 1");
    note1->setContent("Content 1");
    note1->setCreationDateTime(QDateTime::fromString("2024-01-01 10:00:00", "yyyy-MM-dd hh:mm:ss"));

    note2 = new NoteData(this);
    note2->setId(2);
    note2->setFullTitle("Test Note 2");
    note2->setContent("Content 2");
    note2->setCreationDateTime(QDateTime::fromString("2024-01-02 10:00:00", "yyyy-MM-dd hh:mm:ss"));
}

void TestNoteModel::cleanup()
{
    delete model;
}

void TestNoteModel::testAddNote()
{
    QModelIndex index = model->addNote(note1);
    QVERIFY(index.isValid());
    QCOMPARE(model->rowCount(), 1);

    NoteData* retrievedNote = model->getNote(index);
    QVERIFY(retrievedNote != nullptr);
    QCOMPARE(retrievedNote->id(), note1->id());
    QCOMPARE(retrievedNote->fullTitle(), note1->fullTitle());
}

void TestNoteModel::testInsertNote()
{
    model->addNote(note1);
    QModelIndex index = model->insertNote(note2, 0);

    QVERIFY(index.isValid());
    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(model->getNote(index)->id(), note2->id());
}

void TestNoteModel::testAddListNote()
{
    QList<NoteData*> noteList;
    noteList << note1 << note2;

    model->addListNote(noteList);
    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(model->getNote(model->index(0, 0))->id(), note1->id());
    QCOMPARE(model->getNote(model->index(1, 0))->id(), note2->id());
}

void TestNoteModel::testRemoveNote()
{
    model->addNote(note1);
    QModelIndex index = model->index(0, 0);

    NoteData* removedNote = model->removeNote(index);
    QCOMPARE(removedNote->id(), note1->id());
    QCOMPARE(model->rowCount(), 0);
}

void TestNoteModel::testClearNotes()
{
    model->addNote(note1);
    model->addNote(note2);
    QCOMPARE(model->rowCount(), 2);

    model->clearNotes();
    QCOMPARE(model->rowCount(), 0);
}

void TestNoteModel::testData()
{
    model->addNote(note1);
    QModelIndex index = model->index(0, 0);

    QCOMPARE(model->data(index, NoteModel::NoteID).toInt(), note1->id());
    QCOMPARE(model->data(index, NoteModel::NoteFullTitle).toString(), note1->fullTitle());
    QCOMPARE(model->data(index, NoteModel::NoteContent).toString(), note1->content());
}

void TestNoteModel::testSetData()
{
    model->addNote(note1);
    QModelIndex index = model->index(0, 0);

    QSignalSpy spy(model, SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)));

    bool success = model->setData(index, "New Title", NoteModel::NoteFullTitle);
    QVERIFY(success);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(model->data(index, NoteModel::NoteFullTitle).toString(), QString("New Title"));
}

void TestNoteModel::testSort()
{
    model->addNote(note1);
    model->addNote(note2);

    // 测试按创建时间升序排序
    model->sort(0, Qt::AscendingOrder);
    QCOMPARE(model->getNote(model->index(0, 0))->id(), note2->id());
    QCOMPARE(model->getNote(model->index(1, 0))->id(), note1->id());

    // 测试按创建时间降序排序
    model->sort(0, Qt::DescendingOrder);
    QCOMPARE(model->getNote(model->index(0, 0))->id(), note1->id());
    QCOMPARE(model->getNote(model->index(1, 0))->id(), note2->id());
}

void TestNoteModel::testInvalidIndex()
{
    QModelIndex invalidIndex;
    QVERIFY(model->getNote(invalidIndex) == nullptr);
    QVERIFY(!model->setData(invalidIndex, "test", NoteModel::NoteFullTitle));
}

#include "moc_tst_notemodel.cpp"
