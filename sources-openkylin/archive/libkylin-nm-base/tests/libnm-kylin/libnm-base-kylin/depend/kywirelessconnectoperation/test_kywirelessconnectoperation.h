#ifndef TEST_KYWIRELESSCONNECTOPERATION_H
#define TEST_KYWIRELESSCONNECTOPERATION_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QDebug>

#include "kywirelessconnectoperation.h"

class TestKyWirelessConnectOperation : public QObject
{
    Q_OBJECT
public:
    explicit TestKyWirelessConnectOperation(QObject *parent = nullptr);
    ~TestKyWirelessConnectOperation();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void test_constructor();
    void test_destructor();
    void test_requestWirelessScan();
    void test_getConnSecretFlags_notFound();
    void test_getPsk_notFound();
    void test_getPrivateKeyPassword_notFound();
    void test_get8021xPassword_notFound();
    void test_setWirelessAutoConnect_notFound();
    void test_updateIpv4AndIpv6SettingInfo_notFound();
    void test_updateWirelessPersonalConnect_notFound();
    void test_updateWirelessEnterPriseTlsConnect_notFound();
    void test_updateWirelessEnterPrisePeapConnect_notFound();
    void test_updateWirelessEnterPriseTtlsConnect_notFound();
    void test_updateWirelessEnterPriseLeapConnect_notFound();
    void test_updateWirelessEnterPrisePwdConnect_notFound();
    void test_updateWirelessEnterPriseFastConnect_notFound();
    void test_addConnect_notFound();
    void test_addTlsConnect_notFound();
    void test_addPeapConnect_notFound();
    void test_addTtlsConnect_notFound();
    void test_addLeapConnect_notFound();
    void test_addPwdConnect_notFound();
    void test_addFastConnect_notFound();
    void test_addAndActiveWirelessConnect_notFound();
    void test_addAndActiveWirelessEnterPriseTlsConnect_notFound();
    void test_addAndActiveWirelessEnterPrisePeapConnect_notFound();
    void test_addAndActiveWirelessEnterPriseTtlsConnect_notFound();
    void test_addAndActiveWirelessEnterPriseLeapConnect_notFound();
    void test_addAndActiveWirelessEnterPrisePwdConnect_notFound();
    void test_addAndActiveWirelessEnterPriseFastConnect_notFound();
    void test_activeWirelessAp_notFound();
    void test_deactiveWirelessAp_notFound();

private:
    KyWirelessConnectOperation *m_wirelessConnectOperation;
};

#endif // TEST_KYWIRELESSCONNECTOPERATION_H
