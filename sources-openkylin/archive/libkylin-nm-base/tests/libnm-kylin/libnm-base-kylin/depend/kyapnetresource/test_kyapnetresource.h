#ifndef TEST_KYAPNETRESOURCE_H
#define TEST_KYAPNETRESOURCE_H

#include <QObject>
#include <QTest>
#include <QList>
#include "kyapnetresource.h"
#include "kylinapconnectitem.h"

class TestKyApNetResource : public QObject
{
    Q_OBJECT
public:
    TestKyApNetResource(QObject *parent = nullptr);
    ~TestKyApNetResource();

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Test constructor and destructor
    void test_constructor_basic();
    void test_destructor_basic();

    // Test getApConnections
    void test_getApConnections_emptyList();
    void test_getApConnections_normal();

    // Test signal
    void test_signal_wirelessApConnectStateChange();
};

#endif // TEST_KYAPNETRESOURCE_H
