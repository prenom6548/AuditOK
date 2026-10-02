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

#include <string>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QTimer>
#include <QSysInfo>
#include <QByteArray>
#include <QDataStream>
#include <buriedpoint.hpp>

#include "device_information.h"
#include "base_info.h"
#include <libkydatacollect.h>
#include <QDBusConnection>
#include <QDBusInterface>

QString getVersionFromDeb(const QString &debName);
static bool checkInstallable(const QString name, const QString version);
static QString getRetFromCommand(const QStringList &command);

QString devInfoPrint(const DeviceInformation &debugInfo)
{
    QString info = QString  (
                QString("\n")
                + QString("+++++++++++++++++++++++++++++++++\n")
                + QString("connectType  is: ") + QString::number(int(debugInfo.connectType))  + QString('\n')
                + QString("protocolType is: ") + QString::number(int(debugInfo.protocolType)) + QString('\n')
                + QString("name         is: ") + debugInfo.name         + QString('\n')
                + QString("vendor       is: ") + debugInfo.vendor       + QString("\n")
                + QString("model        is: ") + debugInfo.model        + QString("\n")
                + QString("serial       is: ") + debugInfo.serial       + QString("\n")
                + QString("uri          is: ") + debugInfo.uri          + QString("\n")
                + QString("debNameList  is: ") + debugInfo.debNameList.join(",") + QString("\n")
                + QString("packageNameList  is: ") + debugInfo.packageNameList.join(",") + QString("\n")
                + QString("makeAndModel is: ") + debugInfo.makeAndModel + QString("\n")
                + QString("uuid         is: ") + debugInfo.uuid         + QString("\n")
                + QString("deviceId     is: ") + debugInfo.deviceId     + QString("\n")
                );
    if (debugInfo.connectType == ConnectType::InfoFrom_USB) {
        info += QString (
                    QString("sysPath      is: ") + debugInfo.sysPath      + QString('\n')
                    + QString("devicePath   is: ") + debugInfo.devicePath   + QString("\n")
                    + QString("deviceType   is: ") + debugInfo.deviceType   + QString("\n")
                    + QString("busNumber    is: ") + debugInfo.busNumber    + QString("\n")
                    + QString("deviceNumber is: ") + debugInfo.deviceNumber + QString("\n")
                    + QString("VID          is: ") + debugInfo.VID          + QString("\n")
                    + QString("PID          is: ") + debugInfo.PID          + QString("\n")
                    );
    }
    else {
        info += QString (
                    QString("networkNode  is: ") + debugInfo.networkNode  + QString('\n')
                    + QString("host         is: ") + debugInfo.host         + QString('\n')
                    );
    }
    info +=  QString("+++++++++++++++++++++++++++++++++\n");
    return info;
}

QDebug operator << (QDebug debug, const DeviceInformation &debugInfo)
{
    debug.noquote();
    debug << devInfoPrint(debugInfo);
    return debug;
}

QStringList getPackagesNameFromHttp(DeviceInformation &device)
{
    QStringList res;

    QString httpRequest = QString("https://%1/api/v1/getprintertype?systemVersion=%2&arch=%3%4%5%6%7%8%9%10")
            .arg(QString(BaseInfo::instance().getServerAddress()))
            .arg(QString("V10SP1"))
            .arg(BaseInfo::instance().getDebianArchitecture())
            .arg(device.PID.size() ? QString("&pid=" + device.PID) : "")
            .arg(device.VID.size() ? QString("&vid=" + device.VID) : "")
            .arg(device.vendor.size() ? QString("&manufacter=" + device.vendor) : "")
            .arg(device.model.size() ? QString("&model=" + device.model) : "")
            .arg("&appName=kylin-scanner")
            .arg("&USBclass=" + device.deviceType)
            .arg(device.serial.size() ?QString("sn=" + device.serial) : "") ;
    if(httpRequest.contains("+")){
        httpRequest.replace("+", "%2B");
    }
    // qDebug() << httpRequest;
    QNetworkAccessManager manager;
    QNetworkRequest netRequest;
    QNetworkReply *netReply;
    QEventLoop loop;
    qDebug () << "http link is :" << httpRequest;
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    netRequest.setUrl(QUrl(httpRequest));
    netReply = manager.get(netRequest);

    QTimer timer;
    QObject::connect(netReply, SIGNAL(finished()), &loop, SLOT(quit()));
    QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    timer.start(10000);
    loop.exec();

    if (netReply->error() != QNetworkReply::NoError) {
        qDebug() << netReply->errorString();
        return res;
    }

    QByteArray strRateAll = netReply->readAll();
    qDebug() << "http result is"<<strRateAll;
    if (strRateAll == "") {
        qDebug() << "http request error: cannot get info form: " << httpRequest;
        return res;
    }
    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(strRateAll, &jsonParserError );
    qDebug() << strRateAll;
    if ( jsonDocument.isNull() || jsonParserError.error != QJsonParseError::NoError ) {
        qDebug () << "json解析失败";
        return res;
    }
    else {
        qDebug() << "json解析成功!";
    }
    if (jsonDocument.isObject()) {
        QJsonObject jsonObject = jsonDocument.object();
        if ( jsonObject.contains("data")
             && jsonObject.value("data").isArray() ) {

            QJsonArray jsonArray = jsonObject.value("data").toArray();
            for ( int i = 0; i < jsonArray.size(); i++) {
                if (jsonArray.at(i).isObject()) {
                    QJsonObject packages = jsonArray.at(i).toObject();
                    qDebug() << packages;
                    if (   !packages.contains("type")
                           || !packages.contains("packageName")
                           || !packages.contains("others")
                           //                           || !packages.contains("packageLevel")
                           ) {
                        continue ;
                    }

                    if (!packages.value("type").isString()) {
                        continue ;
                    }
                    if(packages.value("digit_id") != ""){
                        device.digitId = packages.value("digit_id").toString();
                    }
                    if(packages.value("packageLevel") != -1) {
                        QString packageLevel = packages.value("packageLevel").toString();
                        device.packageLevel = packageLevel.toInt();
                    }
                    if(packages.value("type") == "scanner"){
                        device.type = DeviceType::SCANNER;
                    }else if(packages.value("type") == "printer"){
                        device.type = DeviceType::PRINTER;
                    }

                    if (!packages.value("packageName").isString()) {
                        buriedNoDriverDevice(device);
                        continue ;
                    }

                    device.others += packages.value("others").toString();

                    if (packages.value("packageName").isString()) {
                        res.append(packages.value("packageName").toString());
                    }

                }
            }
        }
    }
    QSet<QString> resSet = res.toSet();
    QStringList debPackages = resSet.toList();
    for (auto package : debPackages) {
        if (package.contains("\n")) {
            auto pkglist = package.split("\n");
            device.debNameList.append(pkglist);
        } else {
            device.debNameList.append(package);
        }
    }

    for (int i = 0; i < device.debNameList.size(); i++) {
        QString name = device.debNameList.at(i);
        device.packageNameList.append(name.left(name.indexOf("_")));
        device.packageVersion.append(getVersionFromDeb(name));
    }

    return device.packageNameList;
}

DeviceInformation::DeviceInformation()
{

}

QString getVersionFromDeb(const QString &debName)
{
    QString deb(debName);
    if (!deb.contains("_")) {
        return "";
    }

    deb.remove(0, deb.indexOf("_") + 1);

    if (!deb.contains("_")) {
        return "";
    }

    QString debVersion = deb.left(deb.indexOf("_"));
    return debVersion;

}

void buriedNoDriverDevice(DeviceInformation device)
{
    QMap<QString, QString> buried_data;
    buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::kylinScannerDeviceNoDriver)));
    buried_data.insert("deviceVendor", device.vendor);
    buried_data.insert("deviceName", device.name);
    buried_data.insert("deviceModel", device.model);
    buried_data.insert("deviceSerial", device.serial);
    buried_data.insert("deviceVid", device.VID);
    buried_data.insert("devicePid", device.PID);

    int cursor{0};
    KCustomProperty property[buried_data.size()];
    for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
        property[cursor].key = strdup(iter.key().toLocal8Bit().data());
        property[cursor].value = strdup(iter.value().toLocal8Bit().data());
        cursor++;
    }
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node, property, buried_data.size());
    kdk_dia_upload_default(node, "device_no_driver", const_cast<char *>("mainPage"));
    kdk_dia_data_free(node);

}

static QString getRetFromCommand(const QStringList &command) {
    QProcess proc;
    QStringList options;
    options << "-c" << command.join(" ");
    proc.closeWriteChannel();
    proc.start("bash", options);
    if (!proc.waitForFinished())
        return "";
    QString res = QString(proc.readAll());
    proc.close();
    if (res.right(1) == "\n")
        res.chop(1);
    return res;
}


static bool checkInstallable(const QString name, const QString version) {
    QStringList pingCommand = QStringList{"ping", "-c", "1", "kom.kylinos.cn"};
    QString pingResult = getRetFromCommand(pingCommand);
    if (pingResult.isEmpty() || pingResult.contains("100% packet loss")) {
        return false; // Website is not accessible
    }


    QStringList command = QStringList{"apt-cache", "show", name};
    QString res = getRetFromCommand(command);
    if (res.isEmpty()) {
        return false;
    }
    QStringList installabelVersionList;
    QStringList resList = res.split("\n");

    for (int i = 0; i < resList.size(); i++) {
        if (QString(resList[i]).contains("Version:")) {
            installabelVersionList.append(QString(QString(resList[i]).split(":")[1]).trimmed());
        }
    }
    if (installabelVersionList.contains(version)) {
        return true;
    }
    return false;
}
