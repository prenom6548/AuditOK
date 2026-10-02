#include "apputil.h"

QString AppUtil::convertToIPv4Addr(const QHostAddress &addr)
{
    quint32 addrOrigin = addr.toIPv4Address();
    QHostAddress addrHost = QHostAddress(addrOrigin);
    QString addrStr = addrHost.toString();
    return addrStr;
}
