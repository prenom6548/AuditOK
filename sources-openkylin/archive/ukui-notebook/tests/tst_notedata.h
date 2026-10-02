// tst_notedata.h
#ifndef TST_NOTEDATA_H
#define TST_NOTEDATA_H

#include <QObject>
#include <QtTest>
#include "noteData.h"

class TestNoteData : public QObject
{
    Q_OBJECT

public:
    explicit TestNoteData(QObject *parent = nullptr) : QObject(parent) {}

private slots:
    void init();
    void cleanup();
    void testSetAndGetId();
    void testSetAndGetFullTitle();
    void testSetAndGetNoteColor();
    void testSetAndGetLastModificationDateTime();
    void testSetAndGetContent();
    void testSetAndGetMdContent();
    void testSetAndGetModified();
    void testSetAndGetSelected();
    void testSetAndGetScrollBarPosition();
    void testSetAndGetDeletionDateTime();
    void testSetAndGetCreationDateTime();
    void testStreamOperators();

private:
    NoteData* noteData;
};

#endif // TST_NOTEDATA_H
