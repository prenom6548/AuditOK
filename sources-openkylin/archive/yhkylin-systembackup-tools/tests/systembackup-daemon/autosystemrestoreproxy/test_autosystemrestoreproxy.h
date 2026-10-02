#ifndef TEST_AUTOSYSTEMRESTOREPROXY_H
#define TEST_AUTOSYSTEMRESTOREPROXY_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../systembackup-daemon/autosystemrestoreproxy.h"

class TestAutoSystemRestoreProxy : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testConstructor();
    void testCheckEnvEx_data();
    void testCheckEnvEx();
};

#endif // TEST_AUTOSYSTEMRESTOREPROXY_H