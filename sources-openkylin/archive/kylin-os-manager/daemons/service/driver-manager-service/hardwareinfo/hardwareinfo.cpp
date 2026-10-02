#include "hardwareinfo.h"
#include <QDebug>

HardWareInfo::HardWareInfo()
{

}

QDebug operator<<(QDebug debug, const HardWareInfo &debugInfo)
{
    debug.noquote();
    debug << hardWareInfoPrint(debugInfo);
    return debug;
}

QString hardWareInfoPrint(const HardWareInfo &debugInfo)
{

    QString info = QString(
              QString("\n")
            + QString("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
            + QString("DeviceType   is: ") + QString::number(int(debugInfo.type))  + QString('\n')
            + QString("name         is: ") + debugInfo.name         + QString('\n')
            + QString("vendor       is: ") + debugInfo.vendor       + QString("\n")
            + QString("model        is: ") + debugInfo.model        + QString("\n")
            + QString("serial       is: ") + debugInfo.serial       + QString("\n")
            + QString("deviceId     is: ") + debugInfo.deviceId     + QString("\n")
            );
    if(debugInfo.driverNameList.length() != 0){
        for (int var = 0; var < debugInfo.driverNameList.length(); ++var) {
            info = QString(info + QString("deb  ") + QString::number(var) + QString("  Name is: ") + debugInfo.driverNameList[0].driverName + QString("\n"));
        }
    }
    info = QString(info
            + QString("driver_now    is: ") + debugInfo.driver_now + QString("\n")
            + QString("driver_version_now is: ") + debugInfo.driver_version_now + QString("\n")
            + QString("sysPath       is: ") + debugInfo.sysPath + QString("\n")
            + QString("devicePath    is: ") + debugInfo.devicePath + QString("\n")
            + QString("busNumber     is: ") + debugInfo.busNumber + QString("\n")
            + QString("deviceNumber  is: ") + debugInfo.deviceNumber + QString("\n")
            + QString("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")
           );
    return info;
}
