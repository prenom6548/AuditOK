// tst_notedata.cpp
#include "tst_notedata.h"
#include <QSignalSpy>
#include <QBuffer>

void TestNoteData::init()
{
    noteData = new NoteData(this);
}

void TestNoteData::cleanup()
{
    delete noteData;
}

void TestNoteData::testSetAndGetId()
{
    const int testId = 1;
    noteData->setId(testId);
    QCOMPARE(noteData->id(), testId);
}

void TestNoteData::testSetAndGetFullTitle()
{
    const QString testTitle = "Test Title";
    noteData->setFullTitle(testTitle);
    QCOMPARE(noteData->fullTitle(), testTitle);
}

void TestNoteData::testSetAndGetNoteColor()
{
    const int testColor = 2;
    noteData->setNoteColor(testColor);
    QCOMPARE(noteData->notecolor(), testColor);
}

void TestNoteData::testSetAndGetLastModificationDateTime()
{
    const QDateTime testDateTime = QDateTime::currentDateTime();
    noteData->setLastModificationDateTime(testDateTime);
    QCOMPARE(noteData->lastModificationdateTime(), testDateTime);
}

void TestNoteData::testSetAndGetContent()
{
    const QString testContent = "Test Content";
    noteData->setContent(testContent);
    QCOMPARE(noteData->content(), testContent);
}

void TestNoteData::testSetAndGetMdContent()
{
    const QString testMdContent = "# Test Markdown Content";
    noteData->setMdContent(testMdContent);
    QCOMPARE(noteData->mdContent(), testMdContent);
}

void TestNoteData::testSetAndGetModified()
{
    noteData->setModified(true);
    QVERIFY(noteData->isModified());
    
    noteData->setModified(false);
    QVERIFY(!noteData->isModified());
}

void TestNoteData::testSetAndGetSelected()
{
    noteData->setSelected(true);
    QVERIFY(noteData->isSelected());
    
    noteData->setSelected(false);
    QVERIFY(!noteData->isSelected());
}

void TestNoteData::testSetAndGetScrollBarPosition()
{
    const int testPosition = 100;
    noteData->setScrollBarPosition(testPosition);
    QCOMPARE(noteData->scrollBarPosition(), testPosition);
}

void TestNoteData::testSetAndGetDeletionDateTime()
{
    const QDateTime testDateTime = QDateTime::currentDateTime();
    noteData->setDeletionDateTime(testDateTime);
    QCOMPARE(noteData->deletionDateTime(), testDateTime);
}

void TestNoteData::testSetAndGetCreationDateTime()
{
    const QDateTime testDateTime = QDateTime::currentDateTime();
    noteData->setCreationDateTime(testDateTime);
    QCOMPARE(noteData->creationDateTime(), testDateTime);
}

void TestNoteData::testStreamOperators()
{
    // 准备测试数据
    const int testId = 1;
    const QString testTitle = "Test Title";
    const QString testContent = "Test Content";
    const QString testMdContent = "# Test MD Content";
    const int testColor = 2;
    const QDateTime testCreationDateTime = QDateTime::currentDateTime();
    const QDateTime testModificationDateTime = QDateTime::currentDateTime();

    noteData->setId(testId);
    noteData->setFullTitle(testTitle);
    noteData->setContent(testContent);
    noteData->setMdContent(testMdContent);
    noteData->setNoteColor(testColor);
    noteData->setCreationDateTime(testCreationDateTime);
    noteData->setLastModificationDateTime(testModificationDateTime);

    // 测试写入
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << noteData;

    // 测试读取
    QDataStream readStream(&data, QIODevice::ReadOnly);
    NoteData* readNoteData = nullptr;
    readStream >> readNoteData;

    // 验证结果
    QVERIFY(readNoteData != nullptr);
    QCOMPARE(readNoteData->id(), testId);
    QCOMPARE(readNoteData->fullTitle(), testTitle);
    QCOMPARE(readNoteData->content(), testContent);
    QCOMPARE(readNoteData->mdContent(), testMdContent);
    QCOMPARE(readNoteData->notecolor(), testColor);
    QCOMPARE(readNoteData->creationDateTime(), testCreationDateTime);
    QCOMPARE(readNoteData->lastModificationdateTime(), testModificationDateTime);

    delete readNoteData;
}

#include "moc_tst_notedata.cpp"
