#ifndef TST_DBMANAGER_H
#define TST_DBMANAGER_H

#include <QObject>
#include <QtTest>
#include "dbManager.h"
#include "noteData.h"

class TestDBManager : public QObject
{
    Q_OBJECT

public:
    explicit TestDBManager(QObject *parent = nullptr) : QObject(parent) {}

private slots:
    void init();
    void cleanup();
    void testOpenAndCreateTables();
    void testAddNote();
    void testGetNote();
    void testGetAllNotes();
    void testUpdateNote();
    void testRemoveNote();
    void testPermanentlyRemoveAllNotes();
    void testForceLastRowIndexValue();
    void testMigrateNote();
    void testSignals();

private:
    DBManager* dbManager{nullptr};
    NoteData* testNote{nullptr};
};

#endif // TST_DBMANAGER_H
