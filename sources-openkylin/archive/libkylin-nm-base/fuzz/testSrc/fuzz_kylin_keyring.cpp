#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#include "agentutils.h"
#include "kylinagent.h"
#include "kylinsecretagent.h"


extern "C" int LLVMFuzzerTestOneInput_KyKeyring(const uint8_t *data, size_t size) {
    if (size < 1) {
        return 0;
    }

    uint8_t opType = data[0] % 5;

    switch (opType) {
        case 0: {
            if (size < 2) break;
            SecretPolicyFlag flag = static_cast<SecretPolicyFlag>(data[1] % 8);
            break;
        }
        case 1: {
            if (size < 2) break;
            SecretWepKeyId keyId = static_cast<SecretWepKeyId>(data[1] % 4);
            break;
        }
        case 2: {
            QString str1, str2, str3;
            
            if (size > 1) {
                size_t len1 = std::min(static_cast<size_t>(size - 1), static_cast<size_t>(64));
                str1 = QString::fromLatin1(reinterpret_cast<const char*>(data + 1), len1);
            }
            
            if (size > 65) {
                size_t len2 = std::min(static_cast<size_t>(size - 65), static_cast<size_t>(64));
                str2 = QString::fromLatin1(reinterpret_cast<const char*>(data + 65), len2);
            }
            
            if (size > 129) {
                size_t len3 = std::min(static_cast<size_t>(size - 129), static_cast<size_t>(64));
                str3 = QString::fromLatin1(reinterpret_cast<const char*>(data + 129), len3);
            }
            
            QMap<QString, QString> hintsMap;
            hintsMap.insert(VPN_TYPE, str1);
            hintsMap.insert(VPN_NAME, str2);
            hintsMap.insert(CONNECTION_KEY, str3);
            
            QMap<QString, QString> map;
            map.insert(SSID_KEY, str1);
            map.insert(UUID_KEY, str2);
            map.insert(ID_KEY, str3);
            
            break;
        }
        case 3: {
            if (size > 1) {
                size_t copyLen = std::min(static_cast<size_t>(size - 1), static_cast<size_t>(128));
                QString vpnName = QString::fromLatin1(reinterpret_cast<const char*>(data + 1), copyLen);
            }
            
            if (size > 129) {
                size_t copyLen = std::min(static_cast<size_t>(size - 129), static_cast<size_t>(256));
                QString uuid = QString::fromLatin1(reinterpret_cast<const char*>(data + 129), copyLen);
            }
            
            if (size > 385) {
                size_t copyLen = std::min(static_cast<size_t>(size - 385), static_cast<size_t>(256));
                QString connectionId = QString::fromLatin1(reinterpret_cast<const char*>(data + 385), copyLen);
            }
            break;
        }
        case 4: {
            if (size > 1) {
                uint8_t keyMgmtType = data[1] % 4;
                
                QString keyMgmt;
                switch (keyMgmtType) {
                    case 0:
                        keyMgmt = PSK_MGMT;
                        break;
                    case 1:
                        keyMgmt = SAE_MGMT;
                        break;
                    case 2:
                        keyMgmt = WEP_MGMT;
                        break;
                    case 3:
                        keyMgmt = LEAP_MGMT;
                        break;
                    default:
                        keyMgmt = "";
                        break;
                }
                
                if (size > 2) {
                    size_t copyLen = std::min(static_cast<size_t>(size - 2), static_cast<size_t>(128));
                    QString psk = QString::fromLatin1(reinterpret_cast<const char*>(data + 2), copyLen);
                }
                
                if (size > 130) {
                    uint8_t pskFlags = data[130] % 4;
                }
            }
            break;
        }
        default:
            break;
    }

    return 0;
}
