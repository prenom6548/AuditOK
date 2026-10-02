#ifndef DEVICE_MAP_H
#define DEVICE_MAP_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QObject>

// old json format:
// {
//     "HP-LaserJet-M506": {
//         "deviceId": "MFG:Hewlett-Packard;CMD:PJL,PCLXL,PCL,PDF,POSTSCRIPT;CID:HPLJPDLV1;MDL:HP LaserJet M506;CLS:PRINTER;DES:HP LaserJet M506;MCT:PR;MCL:EN;MCV:2.3;",
//         "path": "/devices/pci0000:00/0000:00:14.0/usb1/1-6",
//         "uri": "usb://HP/LaserJet%20M506?serial=PHC8LC5BFW"
//     }
// }


// new json format:
// {
//     "version": "0.0.1",
//     "uris": {
//         "usb://HP/LaserJet%20M506?serial=PHC8LC5BFW": {
//             "deviceId": "MFG:Hewlett-Packard;CMD:PJL,PCLXL,PCL,PDF,POSTSCRIPT;CID:HPLJPDLV1;MDL:HP LaserJet M506;CLS:PRINTER;DES:HP LaserJet M506;MCT:PR;MCL:EN;MCV:2.3;",
//             "path": "/devices/pci0000:00/0000:00:14.0/usb1/1-6",
//             "printers": {
//                 "HP-LaserJet-M506",
//             }
//         }
//     }
// }

class DeviceMap : public QObject
{
    Q_OBJECT
private:
    struct DevInfo
    {
        QStringList printerNames;
        QString path;
        QString uri;
        QString deviceId;
    };

    QString m_jsonVersion{QString()};
    QMap<QString, DevInfo> m_devMp;
    bool generateMap();
    void getJsonVerison(const QJsonObject &jsonObject);
    bool jsonRead();
    bool jsonReadFromOldGsettings(const QJsonObject &jsonObject);
    void updateDevMpFromJson(const QJsonObject &jsonObject);
    QStringList getPrinterNameListFromJson(const QJsonObject &jsonObject);
    int jsonWrite();
    QJsonObject writeDevInfoToJson(const DevInfo &info);
    bool updateDevMpFromPrinterName(const QString &printerName = QString());
    DeviceMap(/* args */);
    
public:
    static DeviceMap *getInstance();
    bool checkPathAdd(const QString &path);
    bool checkPathRemove(const QString &path);
    bool checkPath(const QString &handle, const QString &path);
    ~DeviceMap();
public slots:
    void doPrinterRemove(const QString &prtName);
    void doPrinterAdd(const QString &prtName);
};

#endif // DEVICE_MAP_H
