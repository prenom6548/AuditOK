#ifndef TEST_KYLINSECRETAGENT_H
#define TEST_KYLINSECRETAGENT_H

#include <QString>
#include <QVariantMap>
#include <QDBusObjectPath>
#include "kylinsecretagent.h"

class TestKylinSecretAgent : public ::QObject
{
    Q_OBJECT
public:
    explicit TestKylinSecretAgent(QObject *parent = nullptr);
    ~TestKylinSecretAgent();

private:

    KylinSecretAgent *m_agent;

 private Q_SLOTS:

    void SetUp();
    void GetSecretsWithEmptyConnection();
    void GetSecretsWithVpnConnection();
    void CancelGetSecretsTest();
    void SaveSecretsTest();

    void DeleteSecretsTest();
    void GetSecretsWithWirelessConnection();
    void GetSecretsWithEnterpriseConnection();

    
    // Added ask* function tests
    void AskSecretForPasswordTest();
    void AskSecretForWepTest();
    void AskSecretForLeapTest();
    void AskSecretForWirelessTest();
    void AskSecretWithIdentityAndPasswordTest();
    void AskSecretForTlsTest();
    void AskSecretForEnterpriceWirelessTest();
    void AskSecretForVpnTest();
    void AskForSecretTest();
    void GetWepKeyFromIdTest();
    void SaveOneSecretTest();
    
    void TearDown();//一定要放在最后面，否则可能有内存问题

};



#endif // TEST_KYLINSECRETAGENT_H
