// tst_notemodel.h
#ifndef TST_NOTEMODEL_H
#define TST_NOTEMODEL_H

#include <QObject>
#include <QtTest/QtTest>
#include "noteModel.h"
#include "noteData.h"

class TestNoteModel : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void testAddNote();
    void testInsertNote();
    void testAddListNote();
    void testRemoveNote();
    void testClearNotes();
    void testData();
    void testSetData();
    void testSort();
    void testInvalidIndex();

private:
    NoteModel* model;
    NoteData* note1;
    NoteData* note2;
};

#endif // TST_NOTEMODEL_H
