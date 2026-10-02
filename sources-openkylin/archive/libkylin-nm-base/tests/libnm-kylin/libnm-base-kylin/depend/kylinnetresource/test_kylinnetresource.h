#ifndef TEST_KYLINNETRESOURCE_H
#define TEST_KYLINNETRESOURCE_H

#include <QObject>
#include <QString>
#include "kylinnetresource.h"

class TestKyNetResource : public QObject
{
    Q_OBJECT

public:
    explicit TestKyNetResource(QObject *parent = nullptr);
    ~TestKyNetResource();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_constructor();
    void test_destructor();
    void test_getConnectionSetting_withInvalidUuid();
    void test_getConnectionSetting_withValidUuid();
    void test_getConnectionSetting_ipv4DhcpMethod();
    void test_getConnectionSetting_ipv4ManualMethod();
    void test_getConnectionSetting_ipv6DhcpMethod();
    void test_getConnectionSetting_ipv6ManualMethod();

private:
    KyNetResource *m_netResource = nullptr;
};

#endif // TEST_KYLINNETRESOURCE_H
