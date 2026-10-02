/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "formatconversion.h"
#include <QFile>
#include <QDir>
#include <QtMath>
#include "gpsglobal.h"
#define a 6378245.0               // 长半轴
#define pi 3.14159265358979324    // π
#define ee 0.00669342162296594323 // e²
using namespace kmre_gps;
namespace kmre_dataformat
{
QString codeStr2 = "ZGQ3MjM0NmQwNQ==";

QString FormatConversion::toGPSPoint(QString locationData)
{
    QStringList list = locationData.split(",");
    QString latitude = dataCalculation(list[0]);
    QString longitude = dataCalculation(list[1]);
    QString data = latitude + "," + "N" + "," + longitude + "," + "E";
    return data;
}

QString FormatConversion::dataCalculation(QString data)
{
    QString converseData;
    QString converseData1;
    QString converseData2;
    double gpsData = data.toDouble();
    int temp = qFloor(gpsData);
    converseData.sprintf("%d", temp);

    double longitudeTmp1 = 60 * (gpsData - qFloor(gpsData));
    int integer = qFloor(longitudeTmp1);
    if (integer < 10) {
        converseData1.sprintf("%s%d%s", "0", integer, ".");
    } else {
        converseData1.sprintf("%d%s", integer, ".");
    }
    //小数部分
    double longitudeTmp2 = (longitudeTmp1 - integer) * 10000;
    int integer1 = qFloor(longitudeTmp2);
    if (integer1 < 1000) {
        converseData2.sprintf("%s%d", "0", integer1);
    } else if (integer1 < 100) {
        converseData2.sprintf("%s%d", "00", integer1);
    } else if (integer1 < 10) {
        converseData2.sprintf("%s%d", "000", integer1);
    } else {
        converseData2.sprintf("%d", integer1);
    }
    return converseData + converseData1 + converseData2;
}

QString FormatConversion::dataConversion(QString locationData)
{
    QStringList list = locationData.split(",");
    QString longi = list[0];
    QString lati = list[1];
    double longitude = longi.toDouble();
    double latitude = lati.toDouble();
    QString dev = calDev(latitude, longitude);
    if (dev == "") {
        return "";
    }
    list = dev.split(",");
    QString getLatitude = list[0];
    QString getLongitude = list[1];
    double retLat = latitude - getLatitude.toDouble();
    double retLon = longitude - getLongitude.toDouble();

    dev = calDev(retLat, retLon);
    if (dev == "") {
        return "";
    }
    list = dev.split(",");
    getLatitude = list[0];
    getLongitude = list[1];
    retLat = latitude - getLatitude.toDouble();
    retLon = longitude - getLongitude.toDouble();
    QString converseData;
    converseData.sprintf("%f%s%f", retLat, ",", retLon);
    return toGPSPoint(converseData);
}

// 计算偏差
QString FormatConversion::calDev(double wgLat, double wgLon)
{
    if (isOutOfChina(wgLat, wgLon)) {
        return "";
    }
    double dLat = calLat(wgLon - 105.0, wgLat - 35.0);
    double dLon = calLon(wgLon - 105.0, wgLat - 35.0);
    double radLat = wgLat / 180.0 * pi;
    double magic = sin(radLat);
    magic = 1 - ee * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
    dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);

    QString converseData;
    converseData.sprintf("%f%s%f", dLat, ",", dLon);
    return converseData;
}

// 判断坐标是否在国外
bool FormatConversion::isOutOfChina(double lat, double lon)
{
    if (lon < 72.004 || lon > 137.8347)
        return true;
    if (lat < 0.8293 || lat > 55.8271)
        return true;
    return false;
}

// 计算纬度
double FormatConversion::calLat(double x, double y)
{
    double resultLat = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(abs(x));
    resultLat += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
    resultLat += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
    resultLat += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi / 30.0)) * 2.0 / 3.0;
    return resultLat;
}

// 计算经度
double FormatConversion::calLon(double x, double y)
{
    double resultLon = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(abs(x));
    resultLon += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
    resultLon += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
    resultLon += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;
    return resultLon;
}

QString FormatConversion::decodeToOrigin()
{
    QString codeStr3 = "";
    QString m_confName = "/usr/share/kmre/gps.conf";
    if (QFile::exists(m_confName)) {

        QFile file(m_confName);
        if (!file.open(QIODevice::ReadOnly)) {
            return codeStr3;
        }
        codeStr3 = QString(file.readAll());
        file.close();
    }
    QString finalStr = "";
    QByteArray text1 = CODESTR.toLocal8Bit();
    QByteArray by1 = text1.fromBase64(text1);
    QString str1 = QString::fromLocal8Bit(by1);

    QByteArray text2 = codeStr2.toLocal8Bit();
    QByteArray by2 = text2.fromBase64(text2);
    QString str2 = QString::fromLocal8Bit(by2);

    QByteArray text3 = codeStr3.toLocal8Bit();
    QByteArray by3 = text3.fromBase64(text3);
    QString str3 = QString::fromLocal8Bit(by3);

    finalStr.append(str1);
    finalStr.append(str2);
    finalStr.append(str3);

    return finalStr;
}

} // namespace kmre_dataformat
