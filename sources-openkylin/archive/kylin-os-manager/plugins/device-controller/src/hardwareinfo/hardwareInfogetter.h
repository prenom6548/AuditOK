#ifndef HARDWAREINFOGETTER_H
#define HARDWAREINFOGETTER_H

#include <QObject>
#include "hardwareinfo.h"
#include "bluetoothinfo.h"
#include "graphiccardinfo.h"
#include "networkcardinfo.h"
#include "pcinfo.h"
#include "soundcardinfo.h"
#include "../driverinfo/driverinfo.h"
#include "../driverinfo/drivermodeinfo.h"
#include <QtDBus>

class HardwareInfoGetter : public QObject {
Q_OBJECT
public:
    HardwareInfoGetter(QObject *parent = nullptr);

    PcInfo getPcInfo();

    QList<BluetoothInfo> getBluetoothHardwares();

    QList<NetworkCardInfo> getNetworkHardwares();

    QList<SoundCardInfo> getSoundcardHardwares();

    QList<GraphicCardInfo> getGraphicHardwares();

    QList<HardWareInfo> getScannerHardwares();

    QList<HardWareInfo> getPrinterHardwares();

    QList<HardWareInfo> getInputHardwares();

    QList<HardWareInfo> getVideoHardwares();

    QList<HardWareInfo> getOtherHardwares();

signals:
    void getHwSucceed();

private:
    QString getInfo(DeviceType type);

    QList<HardWareInfo> convertJsonToHardWareList(const QString &jsonString, DeviceType type);


};

#endif //HARDWAREINFOGETTER_H
