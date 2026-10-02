#ifndef TEST_KYLINCONNECTSETTING_H
#define TEST_KYLINCONNECTSETTING_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHostAddress>
#include "kylinconnectsetting.h"

class TestKylinConnectSetting : public QObject
{
    Q_OBJECT

public:
    explicit TestKylinConnectSetting(QObject *parent = nullptr);
    ~TestKylinConnectSetting();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void test_constructor();
    void test_destructor();
    void test_setIfaceName();
    void test_setConnectName();
    void test_setIpConfigType_valid_v4();
    void test_setIpConfigType_valid_v6();
    void test_setIpConfigType_invalid_ipType();
    void test_setIpConfigType_invalid_configType();
    void test_ipv4AddressConstruct();
    void test_ipv6AddressConstruct();
    void test_dumpInfo();

private:
    KyConnectSetting *m_connectSetting = nullptr;
};

#endif // TEST_KYLINCONNECTSETTING_H
