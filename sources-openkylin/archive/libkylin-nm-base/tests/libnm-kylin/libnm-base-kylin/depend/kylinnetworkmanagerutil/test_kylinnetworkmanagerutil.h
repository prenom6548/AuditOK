#ifndef TEST_KYLINNETWORKMANAGERUTIL_H
#define TEST_KYLINNETWORKMANAGERUTIL_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include "kylinnetworkmanagerutil.h"
#include "kylinnetworkresourcetype.h"
#include <NetworkManagerQt/AccessPoint>

class TestKylinNetworkManagerUtil : public QObject
{
    Q_OBJECT

public:
    explicit TestKylinNetworkManagerUtil(QObject *parent = nullptr);
    ~TestKylinNetworkManagerUtil();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void test_getConnectTypeByDbus_emptyPath();
    void test_getConnectTypeByDbus_validPath();
    void test_getSsidFromByteArray_empty();
    void test_getSsidFromByteArray_validUtf8();
    void test_getSsidFromByteArray_validGbk();
    void test_enumToQstring_noSecurity();
    void test_enumToQstring_wep();
    void test_enumToQstring_wpa1();
    void test_enumToQstring_wpa2();
    void test_enumToQstring_wpa3();
    void test_enumToQstring_8021x();
    void test_enumToQstring_mixed();
    void test_setWiredEnabledByGDbus_true();
    void test_setWiredEnabledByGDbus_false();
    void test_getWiredEnabledByGDbus();
    void test_setDeviceManagedByGDbus();

private:
};

#endif // TEST_KYLINNETWORKMANAGERUTIL_H
