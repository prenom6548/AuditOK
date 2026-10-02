/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef DEVICE_MAP_H
#define DEVICE_MAP_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QObject>

#include "single_thread_worker.h"

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

struct DevInfo
{
    QStringList printerNames;
    QString path;
    QString uri;
    QString deviceId;
};

class DeviceMapUtils {
public:
    static QJsonArray qStringListToQJsonArray(const QStringList &list);
    static QStringList qJsonArrayToQStringList(const QJsonArray &array);
    static bool generateMap(QMap<QString, DevInfo> &devmap);
    static QString getJsonVerison(const QJsonObject &jsonObject);
    static bool jsonRead(QMap<QString, DevInfo> &devmap);
    static bool jsonReadFromOldGsettings(const QJsonObject &jsonObject, QMap<QString, DevInfo> &devmap);
    static void updateDevMpFromJson(const QJsonObject &jsonObject, QMap<QString, DevInfo> &devmap);
    static int jsonWrite(const QMap<QString, DevInfo> &devmap);
    static QJsonObject writeDevInfoToJson(const DevInfo &info);
};

class DeviceMapOnce : public SingleThreadWorker
{
    Q_OBJECT
public:
    DeviceMapOnce();
    ~DeviceMapOnce();
private:
    QMap<QString, DevInfo> m_devMp;
    void dowork() override;

signals:
    void done();
};

class DeviceMap : public QObject
{
    Q_OBJECT
private:
    QMap<QString, DevInfo> m_devMp;
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
