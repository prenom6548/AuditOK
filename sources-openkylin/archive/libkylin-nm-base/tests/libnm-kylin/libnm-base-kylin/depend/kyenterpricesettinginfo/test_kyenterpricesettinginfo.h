#ifndef TEST_KYENTERPRICESETTINGINFO_H
#define TEST_KYENTERPRICESETTINGINFO_H

#include <QObject>
#include <QTest>
#include "kyenterpricesettinginfo.h"

class TestKyEnterpriceSettingInfo : public QObject
{
    Q_OBJECT
public:
    explicit TestKyEnterpriceSettingInfo(QObject *parent = nullptr);
    ~TestKyEnterpriceSettingInfo();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    // TLS methods
    void test_assembleEapMethodTlsSettings_basic();
    void test_assembleEapMethodTlsSettings_withDomain();
    void test_assembleEapMethodTlsSettings_withCaCert();
    void test_assembleEapMethodTlsSettings_withoutCaCert();
    void test_modifyEapMethodTlsSettings_basic();
    void test_modifyEapMethodTlsSettings_withDomain();
    void test_modifyEapMethodTlsSettings_withCaCert();
    void test_modifyEapMethodTlsSettings_withoutCaCert();
    void test_modifyEapMethodTlsSettings_withPasswordChanged();

    // PEAP methods
    void test_assembleEapMethodPeapSettings_basic();
    void test_assembleEapMethodPeapSettings_withPasswordChanged();
    void test_modifyEapMethodPeapSettings_basic();
    void test_modifyEapMethodPeapSettings_withPasswordChanged();

    // TTLS methods
    void test_assembleEapMethodTtlsSettings_authEap();
    void test_assembleEapMethodTtlsSettings_authNoEap();
    void test_modifyEapMethodTtlsSettings_authEap();
    void test_modifyEapMethodTtlsSettings_authNoEap();

    // LEAP methods
    void test_assembleEapMethodLeapSettings_basic();
    void test_assembleEapMethodLeapSettings_withPasswordChanged();
    void test_modifyEapMethodLeapSettings_basic();
    void test_modifyEapMethodLeapSettings_withPasswordChanged();

    // PWD methods
    void test_assembleEapMethodPwdSettings_basic();
    void test_assembleEapMethodPwdSettings_withPasswordChanged();
    void test_modifyEapMethodPwdSettings_basic();
    void test_modifyEapMethodPwdSettings_withPasswordChanged();

    // FAST methods
    void test_assembleEapMethodFastSettings_basic();
    void test_assembleEapMethodFastSettings_withAutoPac();
    void test_assembleEapMethodFastSettings_withoutAutoPac();
    void test_modifyEapMethodFastSettings_basic();
    void test_modifyEapMethodFastSettings_withAutoPac();
    void test_modifyEapMethodFastSettings_withoutAutoPac();
    void test_modifyEapMethodFastSettings_withPasswordChanged();

private:
    void prepareTlsInfo(KyEapMethodTlsInfo &info, bool withDomain = false, bool needCa = false);
    void preparePeapInfo(KyEapMethodPeapInfo &info, bool changed = false);
    void prepareTtlsInfo(KyEapMethodTtlsInfo &info, KyTtlsAuthMethod authType);
    void prepareLeapInfo(KyEapMethodLeapInfo &info, bool changed = false);
    void preparePwdInfo(KyEapMethodPwdInfo &info, bool changed = false);
    void prepareFastInfo(KyEapMethodFastInfo &info, bool allowAutoPac = false, bool changed = false);
};

#endif // TEST_KYENTERPRICESETTINGINFO_H
