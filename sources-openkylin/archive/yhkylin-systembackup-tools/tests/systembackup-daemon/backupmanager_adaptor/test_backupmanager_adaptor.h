#ifndef TEST_BACKUPMANAGER_ADAPTOR_H
#define TEST_BACKUPMANAGER_ADAPTOR_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../systembackup-daemon/backupmanager_adaptor.h"
#include "../../../systembackup-daemon/mybackupmanager.h"

class TestBackupManagerAdaptor : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testClassDefinition();
};

#endif // TEST_BACKUPMANAGER_ADAPTOR_H