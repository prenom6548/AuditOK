/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#ifndef SANEOBJECT_H
#define SANEOBJECT_H

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <QObject>
#include <QWidget>
#include <QString>
#include <QStringList>
#include <QtMath>
#include <QVector>
#include <QMap>
#include <QDir>
#include <QDateTime>
#include <QTimer>
#include <QFileInfo>
#include "utils/scannersettings.h"
#include <QList>
#include <boundscheck/securec.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "sane/sane.h"
#include "sane/saneopts.h"

#ifdef __cplusplus
}
#endif

#include "globalsignal.h"
#include <QMap>
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#define OUTPUT_UNKNOWN  0
#define OUTPUT_PNM      1
#define OUTPUT_TIFF     2
#define OUTPUT_PNG      3
#define OUTPUT_JPEG     4

/**
 * @brief The ScanDeviceInfo struct
 * Parameters getted from specific scanner device, which could set in scansetting page
 */
typedef struct  _ScanDeviceInfo {
    bool status;
    bool have_handle;
    bool haveOpenSaneDevice;
    QStringList name;
    QStringList type;
    QStringList color;
    QStringList resolution;//DPI
    QStringList size; //A3 A4 A5...
    QStringList tl;//tl_x,tl_y
    QStringList br;//br_x,br_y
}ScanDeviceInfo;

/**
 * @brief The UserSelectedInfo struct
 * Parameters chosen by user
 */
typedef struct _UserSelectedInfo {
    int deviceNameIndex;
    QString name;
    QString pageNumber;
    QString time;
    QString type;
    QString color;
    QString resolution;
    QString size;
    QString format;
    QString loadName = "";    // /tmp/kylin-scanner/image/ 目录中图片的加载名字
    QString saveName;
    QString saveDirectory;
}UserSelectedInfo ;


enum sizes_type {
    A2,
    A3,
    A4,
    A5,
    A6
};

class SaneObject: public QObject
{
    Q_OBJECT
public:
    static SaneObject *getInstance();

    bool detectSaneDevices();
    void openSaneDevice(int index);
    bool getSaneStatus();
    bool getSaneHaveOpenDevice();
    bool getSaneHaveHandle();
    QStringList getSaneNames();
    QStringList getSaneTypes();
    QStringList getSaneResolutions();
    int getNumbersFromQString(const QString &qstring);
    QStringList getSaneSizes();
    QStringList getSaneColors();

    void setSaneHaveOpenSaneDevice(bool haveOpenSaneDevice);
    void setSaneHaveHandle(bool have_handle);
    void setSaneStatus(bool status);
    void setSaneNames(QStringList name);
    void setSaneTypes(QStringList type);
    void setSaneResolutions(QStringList resolution);
    void setSaneSizes(QStringList size);
    void setSaneColors(QStringList color);
    void saneCancel(SANE_Handle sane_handle);
    void saneExit();
    void saneClose();

    void setSaneAllParametersByUser();
    void setSanePageNumberByUser();
    void setSaneTypeByUser();
    QString getSaneTypeByUser(QString type);
    void setSaneColorByUser();
    QString getSaneColorByUser(QString color);
    void setSaneResolutionByUser();
    int getSaneResolutionByUser(QString resolution);
    QString fileNameOperation();
    void setSaneSizeByUser();
    void setSaneFormatByUser();
    QString getSaneFormatByUser();
    QString getSaneSaveNameByUser();
    QString getSaneLoadNameByUser();
    QString getSaneSaveDirectoryByUser();
    QString getSaneTmpSaveDirectory();

    void openSaneDeviceForPage(int index);
    QString getFullScanFileNameExceptFormatForSave();
    QString getFullScanFileNameExceptFormatForPnmLoad();
    int startScanning(UserSelectedInfo info);
    bool testScannerIsAlive(QString deviceName);
    void hotPlugScanCompleteSlot();
    void saveScannerSettings();

    bool m_ParametersHaveSeted = false;

    const SANE_Device **g_deviceList;

    int hotplug_sock;
    SANE_Handle handle = nullptr;
    UserSelectedInfo userInfo;
    QMap<QString, QString> colorModesMap;
    QMap<QString, QString> sourceModesMap;
    bool usbAddSlotIsRunning = false;
    bool usbRemoveSlotIsRunning = false;
    int haveSourceFlag = 0;

    bool saneHaveStart = false;
    bool onDetection = false;
    QString openSaneName;
    int scanPageNumber = 0;
    int ocrFlag = 0;
    int cropFlag = 0;
    int openDeviceIndex = 0;

    int resolutionValue;
    QString devicemodel;
    QString nowSaveName;
    QString normalImagePath;
    QString saveFullScanFileName;
    QString loadFullScanFileName;
    QStringList saveFullScanFileNames;
    QStringList loadFullScanFileNames;

    bool haveScanSuccessImage = false;

    QString ocrOutputText;

    QString percentage = "100%";

    bool stopSaneReadFlag = false;

    bool isOnScanning = false;

    int output_format = OUTPUT_PNM;
    int resolution_value;
    QList<ScannerSettings> m_scannerSettingMap;    // 用于存储扫描仪参数

    bool convertAndSave();
    bool getSaneOptIndex(QString desc,int &index);
private:
    static SaneObject *instance;

    explicit SaneObject(QObject *parent = nullptr);
    ~SaneObject();
    SaneObject(const SaneObject &) = delete ;
    SaneObject &operator = (const SaneObject &) = delete ;

    ScanDeviceInfo devicesInfo;
    //QString : 属性描述， int:属性index
    QMap<QString,int> m_saneOptions;
    void dumpScannerOptions();

public slots:
    void stopSaneRead(bool isStoped);
    void restartSaneForException();
    void refreshListSlots();
    void getScannerSettings();

signals:
    void updatePageNum();

};

#define g_sane_object  SaneObject::getInstance()


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief detectScanDevices
 * detect scan devices to find available devices
 */

#ifdef __cplusplus
}
#endif

#endif // SANEOBJECT_H
