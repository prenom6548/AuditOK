#ifndef LPINFO_BROWSER_H
#define LPINFO_BROWSER_H

#include <QObject>
#include <QString>
#include <QList>
#include <cups/http.h>
#include <cups/ipp.h>
#include "device_information.h"

extern QVariant getValueFromAttr(ipp_attribute_t *attr, int i);

class LpinfoBrowser: public QObject
{
    Q_OBJECT
private:
    LpinfoBrowser(/* args */);
public:
    static LpinfoBrowser *getInstance();
    static QStringList findRemotePrinter(const QString &host, const int port);
    static QMap<QString, DeviceInformation> findNetWorkPrinter();
    static QMap<QString, DeviceInformation> findDirectPrinter();
    static QMap<QString, DeviceInformation> findAllPrinter();
    static QMap<QString, DeviceInformation> findPrinter(const QStringList &include_schemes, const QStringList &exclude_schemes);
    static DeviceInformation getUsbPrinterDeviceFromDeviceId(const QString &deviceId);
    ~LpinfoBrowser();

};

#endif // LPINFO_BROWSER_H
