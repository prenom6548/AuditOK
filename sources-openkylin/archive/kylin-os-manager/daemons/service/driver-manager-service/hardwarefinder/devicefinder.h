#ifndef DEVICEFINDER_H
#define DEVICEFINDER_H

#include <QObject>
#include <QMap>

#include "../hardwareinfo/hardwareinfo.h"
#include "../hardwareinfo/graphiccardinfo.h"
#include "../hardwareinfo/bluetoothinfo.h"
#include "../hardwareinfo/camerainfo.h"
#include "../hardwareinfo/faninfo.h"
#include "../hardwareinfo/cdinfo.h"
#include "../hardwareinfo/batteryinfo.h"
#include "../hardwareinfo/soundcardinfo.h"
#include "../hardwareinfo/networkcardinfo.h"
#include "../driverinfo/drivermodeinfo.h"

char *get_ieee1284_id_from_child (struct udev *udev, struct udev_device *parent);
void parse_device_id (const char *device_id, struct device_id_s *id);
char* qstringTochar(const QString &qstr);
void devpath_from_usb_devaddr (const QString &devaddr, HardWareInfo &info);
void specialDeviceCheck(QString &vendor, QString &model);

struct device_id_s {
    QString full_device_id;
    QString mfg;
    QString mdl;
    QString sern;
    QString cls;
};
const QMap<QString, QString> mfgConvertMap = {
    std::map<QString, QString>::value_type("Lenovo Image", "Lenovo"),
    std::map<QString, QString>::value_type("Fuji Xerox", "Fuji-Xerox"),
    std::map<QString, QString>::value_type("INDEX BRAILLE", "INDEX-BRAILLE"),
    std::map<QString, QString>::value_type("KONICA MINOLTA", "KONICA-MINOLTA"),
    std::map<QString, QString>::value_type("Hewlett-Packard", "HP"),
    std::map<QString, QString>::value_type("Havon Pen", "Havon")
};

class deviceFinder : public QObject
{
    Q_OBJECT
public:
    deviceFinder(QObject *parent = nullptr);
    ~deviceFinder();
    virtual void dowork() = 0;

    void getDriverModeInfo(QString driverName, DriverModeInfo &modeinfo);
};

#endif // DEVICEFINDER_H
