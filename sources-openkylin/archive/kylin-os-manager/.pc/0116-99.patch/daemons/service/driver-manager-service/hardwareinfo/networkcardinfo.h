#ifndef NETWORKCARDINFO_H
#define NETWORKCARDINFO_H

#include "hardwareinfo.h"
#include "../driverinfo/drivermodeinfo.h"

class NetworkCardInfo : public HardWareInfo
{
public:
    NetworkCardInfo();

    QString bus{QString()};
    QString MAC_addr{QString()};
    int isWireless = 0;      // 是否为无线网卡，是(1)，否(0)
    QString ip_addr{QString()};
    QString net_musk{QString{}};
    QString send_bytes{QString{}};
    QString recv_bytes{QString{}};
    QString MTU{QString{}};
    DriverModeInfo modeinfo;
};

#endif // NETWORKCARDINFO_H
