#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#include "kyenterpricesettinginfo.h"




NetworkManager::Setting::SecretFlags gSecretFlags;
bool gPasswordIsNull;
bool gUserNameIsNull;

KyEapMethodAuth gAuthEapMethod;
KyNoEapMethodAuth gAuthNoEapMethod;

void prepareTlsInfo(KyEapMethodTlsInfo &info, bool withDomain, bool needCa)
{
    info.identity = gUserNameIsNull? "":"test_identity";
    if (withDomain) {
        info.domain = "test.domain.com";
    } else {
        info.domain = "";
    }
    info.devIfaceName = "wlan0";
    info.caCertPath = "/path/to/ca.crt";
    info.bNeedCa = needCa;
    info.clientCertPath = "/path/to/client.crt";
    info.clientPrivateKey = "/path/to/client.key";
    info.clientPrivateKeyPWD =gPasswordIsNull? "":"test_password";
    info.m_privateKeyPWDFlag = gSecretFlags;
    info.bChanged = false;
}

void preparePeapInfo(KyEapMethodPeapInfo &info, bool changed)
{
    info.phase2AuthMethod = KyAuthMethodMschapv2;
    info.userName = gUserNameIsNull? "":"test_user";
    info.userPWD = gPasswordIsNull? "":"test_password";;
    info.m_passwdFlag = gSecretFlags;
    info.bChanged = changed;
}

void prepareTtlsInfo(KyEapMethodTtlsInfo &info, KyTtlsAuthMethod authType)
{
    info.authType = authType;
    if (authType == AUTH_EAP) {
        info.authEapMethod = gAuthEapMethod;//KyAuthEapMethodMschapv2;
    } else {
        info.authNoEapMethod = gAuthNoEapMethod;//KyAuthMethodMschapv2;
    }
    info.userName = gUserNameIsNull? "":"test_user";
    info.userPWD = gPasswordIsNull? "":"test_password";
    info.m_passwdFlag = gSecretFlags;
    info.bChanged = false;
}

void prepareLeapInfo(KyEapMethodLeapInfo &info, bool changed)
{
    info.m_userName = gUserNameIsNull? "":"test_user";
    info.m_userPwd = gPasswordIsNull? "":"test_password";
    info.m_passwdFlag = gSecretFlags;
    info.bChanged = changed;
}

void preparePwdInfo(KyEapMethodPwdInfo &info, bool changed)
{
    info.m_userName = "test_user";
    info.m_userPwd = gPasswordIsNull? "":"test_password";
    info.m_passwdFlag = gSecretFlags;
    info.bChanged = changed;
}

void prepareFastInfo(KyEapMethodFastInfo &info, bool allowAutoPac, bool changed)
{
    info.m_anonIdentity = "anonymous";
    info.m_pacProvisioning = KyFastProvisioningAllowAuthenticated;
    info.m_allowAutoPacFlag = allowAutoPac;
    info.m_pacFilePath = "/path/to/pac file";
    info.m_authMethod = KyAuthMethodMschapv2;
    info.m_userName = gUserNameIsNull? "":"test_user";
    info.m_userPwd = gPasswordIsNull? "":"test_password";
    info.m_passwdFlag = gSecretFlags;
    info.bChanged = changed;
}

// TLS tests
void test_assembleEapMethodTlsSettings_basic()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, false, false);
    
    /*
    KyEapMethodTlsInfo teminfo;
    bool test=info==teminfo? true:false;
    test=info==info? true:false;
*/

    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodTlsSettings(connSettings, info);
}

void test_assembleEapMethodTlsSettings_withDomain()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, true, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodTlsSettings(connSettings, info);

}

void test_assembleEapMethodTlsSettings_withCaCert()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, false, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodTlsSettings(connSettings, info);

}

void test_assembleEapMethodTlsSettings_withoutCaCert()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, false, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodTlsSettings(connSettings, info);

}

void test_modifyEapMethodTlsSettings_basic()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, false, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodTlsSettings(connSettings, info);

}

void test_modifyEapMethodTlsSettings_withDomain()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, true, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodTlsSettings(connSettings, info);

}

void test_modifyEapMethodTlsSettings_withCaCert()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, false, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodTlsSettings(connSettings, info);

}

void test_modifyEapMethodTlsSettings_withoutCaCert()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, false, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodTlsSettings(connSettings, info);

}

void test_modifyEapMethodTlsSettings_withPasswordChanged()
{
    KyEapMethodTlsInfo info;
    prepareTlsInfo(info, false, false);
    info.bChanged = true;
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodTlsSettings(connSettings, info);

}

// PEAP tests
void test_assembleEapMethodPeapSettings_basic()
{
    KyEapMethodPeapInfo info;
    preparePeapInfo(info, false);
    
	/*
    KyEapMethodPeapInfo teminfo;
    bool test=info==teminfo? true:false;
    test=info==info? true:false;
	*/
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodPeapSettings(connSettings, info);
}

void test_assembleEapMethodPeapSettings_withPasswordChanged()
{
    KyEapMethodPeapInfo info;
    preparePeapInfo(info, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodPeapSettings(connSettings, info);
}

void test_modifyEapMethodPeapSettings_basic()
{
    KyEapMethodPeapInfo info;
    preparePeapInfo(info, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodPeapSettings(connSettings, info);
}

void test_modifyEapMethodPeapSettings_withPasswordChanged()
{
    KyEapMethodPeapInfo info;
    preparePeapInfo(info, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodPeapSettings(connSettings, info);
}

// TTLS tests
void test_assembleEapMethodTtlsSettings_authEap()
{
    KyEapMethodTtlsInfo info;
    prepareTtlsInfo(info, AUTH_EAP);
    
    /*
    KyEapMethodTtlsInfo teminfo;
    bool test=info==teminfo? true:false;
    test=info==info? true:false;
    */

    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodTtlsSettings(connSettings, info);
}

void test_assembleEapMethodTtlsSettings_authNoEap()
{
    KyEapMethodTtlsInfo info;
    prepareTtlsInfo(info, AUTH_NO_EAP);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodTtlsSettings(connSettings, info);
}

void test_modifyEapMethodTtlsSettings_authEap()
{
    KyEapMethodTtlsInfo info;
    prepareTtlsInfo(info, AUTH_EAP);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodTtlsSettings(connSettings, info);
}

void test_modifyEapMethodTtlsSettings_authNoEap()
{
    KyEapMethodTtlsInfo info;
    prepareTtlsInfo(info, AUTH_NO_EAP);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodTtlsSettings(connSettings, info);
}

// LEAP tests
void test_assembleEapMethodLeapSettings_basic()
{
    KyEapMethodLeapInfo info;
    prepareLeapInfo(info, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodLeapSettings(connSettings, info);
}

void test_assembleEapMethodLeapSettings_withPasswordChanged()
{
    KyEapMethodLeapInfo info;
    prepareLeapInfo(info, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodLeapSettings(connSettings, info);
}

void test_modifyEapMethodLeapSettings_basic()
{
    KyEapMethodLeapInfo info;
    prepareLeapInfo(info, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodLeapSettings(connSettings, info);
}

void test_modifyEapMethodLeapSettings_withPasswordChanged()
{
    KyEapMethodLeapInfo info;
    prepareLeapInfo(info, true);
    
    KyEapMethodLeapInfo teminfo;
    bool test=info==teminfo? true:false;
    test=info==info? true:false;
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodLeapSettings(connSettings, info);
}

// PWD tests
void test_assembleEapMethodPwdSettings_basic()
{
    KyEapMethodPwdInfo info;
    preparePwdInfo(info, false);
    KyEapMethodPwdInfo teminfo;

    bool test=info==teminfo? true:false;
    test=info==info? true:false;
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodPwdSettings(connSettings, info);
}

void test_assembleEapMethodPwdSettings_withPasswordChanged()
{
    KyEapMethodPwdInfo info;
    preparePwdInfo(info, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodPwdSettings(connSettings, info);
}

void test_modifyEapMethodPwdSettings_basic()
{
    KyEapMethodPwdInfo info;
    preparePwdInfo(info, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodPwdSettings(connSettings, info);
}

void test_modifyEapMethodPwdSettings_withPasswordChanged()
{
    KyEapMethodPwdInfo info;
    preparePwdInfo(info, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodPwdSettings(connSettings, info);
}

// FAST tests
void test_assembleEapMethodFastSettings_basic()
{
    KyEapMethodFastInfo info;
    prepareFastInfo(info, false, false);
    
    KyEapMethodFastInfo teminfo;
    bool test=info==teminfo? true:false;
    test=info==info? true:false;

    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodFastSettings(connSettings, info);
}

void test_assembleEapMethodFastSettings_withAutoPac()
{
    KyEapMethodFastInfo info;
    prepareFastInfo(info, true, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodFastSettings(connSettings, info);
}

void test_assembleEapMethodFastSettings_withoutAutoPac()
{
    KyEapMethodFastInfo info;
    prepareFastInfo(info, false, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    assembleEapMethodFastSettings(connSettings, info);
}

void test_modifyEapMethodFastSettings_basic()
{
    KyEapMethodFastInfo info;
    prepareFastInfo(info, false, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodFastSettings(connSettings, info);
}

void test_modifyEapMethodFastSettings_withAutoPac()
{
    KyEapMethodFastInfo info;
    prepareFastInfo(info, true, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodFastSettings(connSettings, info);
}

void test_modifyEapMethodFastSettings_withoutAutoPac()
{
    KyEapMethodFastInfo info;
    prepareFastInfo(info, false, false);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodFastSettings(connSettings, info);
}

void test_modifyEapMethodFastSettings_withPasswordChanged()
{
    KyEapMethodFastInfo info;
    prepareFastInfo(info, false, true);
    
    NetworkManager::ConnectionSettings::Ptr connSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    modifyEapMethodFastSettings(connSettings, info);
}



extern "C" int LLVMFuzzerTestOneInput_Enterprise(const uint8_t *data, size_t size) {
    if (size < 5) {
        return 0;
    }


    uint8_t methodType = data[0] % 6;
    gSecretFlags=NetworkManager::Setting::SecretFlags(data[1] % 4);
    gPasswordIsNull=data[2]? true:false;
    gUserNameIsNull=data[3]? true:false;
    gAuthEapMethod=KyEapMethodAuth(data[4] % 6);
    //gAuthNoEapMethod=KyNoEapMethodAuth(data[5] % 9);
    #if 0
 if (size >= 1) {
        uint8_t methodType = data[0] % 6;

        switch (methodType) {
            case 0: {
                KyEapMethodTlsInfo tlsInfo;
                if (size > 1) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 1), static_cast<size_t>(128));
                    tlsInfo.identity = QString::fromLatin1(reinterpret_cast<const char*>(data + 1), copyLen);
                }
                if (size > 129) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 129), static_cast<size_t>(128));
                    tlsInfo.domain = QString::fromLatin1(reinterpret_cast<const char*>(data + 129), copyLen);
                }
                if (size > 257) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 257), static_cast<size_t>(64));
                    tlsInfo.devIfaceName = QString::fromLatin1(reinterpret_cast<const char*>(data + 257), copyLen);
                }
                tlsInfo.bNeedCa = (data[0] % 2 == 0);
                tlsInfo.m_privateKeyPWDFlag = NetworkManager::Setting::SecretFlags(data[0] % 4);
                tlsInfo.bChanged = false;
                break;
            }
            case 1: {
                KyEapMethodPeapInfo peapInfo;
                if (size > 1) {
                    peapInfo.phase2AuthMethod = static_cast<KyNoEapMethodAuth>(data[1] % 10);
                }
                if (size > 2) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 2), static_cast<size_t>(128));
                    peapInfo.userName = QString::fromLatin1(reinterpret_cast<const char*>(data + 2), copyLen);
                }
                if (size > 130) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 130), static_cast<size_t>(128));
                    peapInfo.userPWD = QString::fromLatin1(reinterpret_cast<const char*>(data + 130), copyLen);
                }
                if (size > 258) {
                    peapInfo.m_passwdFlag = NetworkManager::Setting::SecretFlags(data[258] % 4);
                }
                peapInfo.bChanged = false;
                break;
            }
            case 2: {
                KyEapMethodTtlsInfo ttlsInfo;
                if (size > 1) {
                    ttlsInfo.authType = static_cast<KyTtlsAuthMethod>(data[1] % 2);
                }
                if (size > 2) {
                    ttlsInfo.authEapMethod = static_cast<KyEapMethodAuth>(data[2] % 6);
                }
                if (size > 3) {
                    ttlsInfo.authNoEapMethod = static_cast<KyNoEapMethodAuth>(data[3] % 10);
                }
                if (size > 4) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 4), static_cast<size_t>(128));
                    ttlsInfo.userName = QString::fromLatin1(reinterpret_cast<const char*>(data + 4), copyLen);
                }
                if (size > 132) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 132), static_cast<size_t>(128));
                    ttlsInfo.userPWD = QString::fromLatin1(reinterpret_cast<const char*>(data + 132), copyLen);
                }
                if (size > 260) {
                    ttlsInfo.m_passwdFlag = NetworkManager::Setting::SecretFlags(data[260] % 4);
                }
                ttlsInfo.bChanged = false;
                break;
            }
            case 3: {
                KyEapMethodLeapInfo leapInfo;
                if (size > 1) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 1), static_cast<size_t>(128));
                    leapInfo.m_userName = QString::fromLatin1(reinterpret_cast<const char*>(data + 1), copyLen);
                }
                if (size > 129) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 129), static_cast<size_t>(128));
                    leapInfo.m_userPwd = QString::fromLatin1(reinterpret_cast<const char*>(data + 129), copyLen);
                }
                if (size > 257) {
                    leapInfo.m_passwdFlag = NetworkManager::Setting::SecretFlags(data[257] % 4);
                }
                leapInfo.bChanged = false;
                break;
            }
            case 4: {
                KyEapMethodPwdInfo pwdInfo;
                if (size > 1) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 1), static_cast<size_t>(128));
                    pwdInfo.m_userName = QString::fromLatin1(reinterpret_cast<const char*>(data + 1), copyLen);
                }
                if (size > 129) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 129), static_cast<size_t>(128));
                    pwdInfo.m_userPwd = QString::fromLatin1(reinterpret_cast<const char*>(data + 129), copyLen);
                }
                if (size > 257) {
                    pwdInfo.m_passwdFlag = NetworkManager::Setting::SecretFlags(data[257] % 4);
                }
                pwdInfo.bChanged = false;
                break;
            }
            case 5: {
                KyEapMethodFastInfo fastInfo;
                if (size > 1) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 1), static_cast<size_t>(128));
                    fastInfo.m_anonIdentity = QString::fromLatin1(reinterpret_cast<const char*>(data + 1), copyLen);
                }
                if (size > 129) {
                    fastInfo.m_pacProvisioning = static_cast<KyFastProvisioning>((data[129] % 5) - 1);
                }
                if (size > 130) {
                    fastInfo.m_allowAutoPacFlag = (data[130] % 2 == 0);
                }
                if (size > 131) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 131), static_cast<size_t>(128));
                    fastInfo.m_pacFilePath = QString::fromLatin1(reinterpret_cast<const char*>(data + 131), copyLen);
                }
                if (size > 259) {
                    fastInfo.m_authMethod = static_cast<KyNoEapMethodAuth>(data[259] % 10);
                }
                if (size > 260) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 260), static_cast<size_t>(128));
                    fastInfo.m_userName = QString::fromLatin1(reinterpret_cast<const char*>(data + 260), copyLen);
                }
                if (size > 388) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 388), static_cast<size_t>(128));
                    fastInfo.m_userPwd = QString::fromLatin1(reinterpret_cast<const char*>(data + 388), copyLen);
                }
                if (size > 516) {
                    fastInfo.m_passwdFlag = NetworkManager::Setting::SecretFlags(data[516] % 4);
                }
                fastInfo.bChanged = false;
                break;
            }
            default:
                break;
        }
    }
#endif

    //printf("LLVMFuzzerTestOneInput_Enterprise methodType:%d \n",methodType);
    switch (methodType) {
        case 0: {
                test_assembleEapMethodTlsSettings_basic();
                test_assembleEapMethodTlsSettings_withDomain();
                test_assembleEapMethodTlsSettings_withCaCert();
                test_assembleEapMethodTlsSettings_withoutCaCert();
                test_modifyEapMethodTlsSettings_basic();
                test_modifyEapMethodTlsSettings_withDomain();
                test_modifyEapMethodTlsSettings_withCaCert();
                test_modifyEapMethodTlsSettings_withoutCaCert();
                test_modifyEapMethodTlsSettings_withPasswordChanged();
            break;
        }
        case 1: {
                test_assembleEapMethodPeapSettings_basic();
                test_assembleEapMethodPeapSettings_withPasswordChanged();
                test_modifyEapMethodPeapSettings_basic();
                test_modifyEapMethodPeapSettings_withPasswordChanged();
            break;
        }
        case 2: {
            test_assembleEapMethodTtlsSettings_authEap();
            test_assembleEapMethodTtlsSettings_authNoEap();
            test_modifyEapMethodTtlsSettings_authEap();
            test_modifyEapMethodTtlsSettings_authNoEap();
            break;
        }
        case 3: {
            test_assembleEapMethodLeapSettings_basic();
            test_assembleEapMethodLeapSettings_withPasswordChanged();
            test_modifyEapMethodLeapSettings_basic();
            test_modifyEapMethodLeapSettings_withPasswordChanged();
            break;
        }
        case 4: {
            test_assembleEapMethodPwdSettings_basic();
            test_assembleEapMethodPwdSettings_withPasswordChanged();
            test_modifyEapMethodPwdSettings_basic();
            test_modifyEapMethodPwdSettings_withPasswordChanged();
        }
        case 5: {
                test_assembleEapMethodFastSettings_basic();
                test_assembleEapMethodFastSettings_withAutoPac();
                test_assembleEapMethodFastSettings_withoutAutoPac();
                test_modifyEapMethodFastSettings_basic();
                test_modifyEapMethodFastSettings_withAutoPac();
                test_modifyEapMethodFastSettings_withoutAutoPac();
                test_modifyEapMethodFastSettings_withPasswordChanged();
            break;
        }
        default:
            break;
    }
    

    return 0;
}




