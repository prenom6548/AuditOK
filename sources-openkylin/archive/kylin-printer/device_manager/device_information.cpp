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

#include <QDBusMessage>
#include <QDBusConnection>


#include <QTimer>
#include <QSysInfo>
#include <QByteArray>
#include <QDataStream>
#include <QtMath>

#include "device_information.h"
#include "global_variable.h"
#include "gsettings_monitor.h"
#include "information_collector.h"
#include "base_info.h"
#include "common.h"

QString DeviceInformation::serialize() const
{
    QJsonObject jsonObject;
    jsonObject["displayName"]       = this->displayName;
    jsonObject["userChoosePpdName"] = this->userChoosePpdName;
    jsonObject["pid"]               = this->PID;
    jsonObject["vid"]               = this->VID;
    jsonObject["manufacter"]        = this->vendor;
    jsonObject["model"]             = this->model;
    jsonObject["ppdName"]           = this->ppdName;
    jsonObject["packageLevel"]      = this->packageLevel;

    // 将JSON对象转换为JSON文档
    QJsonDocument jsonDocument(jsonObject);
    QByteArray jsonData = jsonDocument.toJson();
    QString res = QString::fromLocal8Bit(jsonData);
    return res;
}

void DeviceInformation::unserialize(const QString &str)
{
    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(str.toLocal8Bit(), &jsonParserError);

    if (jsonDocument.isNull() || jsonParserError.error != QJsonParseError::NoError) {
        qDebug () << "json解析失败";
        return ;
    }
    if (!jsonDocument.isObject()) {
        return ;
    }
    QJsonObject jsonObject = jsonDocument.object();
    if (jsonObject.size() == 0) {
        qDebug() << "json is empty!";
    }

    for (auto it = jsonObject.begin(); it != jsonObject.end(); it++) {
        QString key = it.key();
        QJsonValue value = it.value();

        if (key.isEmpty() || value.isNull() || !value.isString()) {
            continue;
        }

        if (key == "displayName") {
            this->displayName = value.toString();
        } else if (key == "userChoosePpdName") {
            this->userChoosePpdName = value.toString();
        } else if (key == "pid") {
            this->PID = value.toString();
        } else if (key == "vid") {
            this->VID = value.toString();
        } else if (key == "manufacter") {
            this->vendor = value.toString();
        } else if (key == "model") {
            this->model = value.toString();
        } else if (key == "ppdName") {
            this->ppdName = value.toString();
        } else if (key == "packageLevel") {
            this->packageLevel = value.toString();
        }
    }
}

void DeviceInformation::importUri(const Uri &uri)
{
    this->protocolType = uri.protocolType;
    this->deviceType = uri.deviceType;
    this->driverlessDomain = uri.driverlessDomain;
    this->host = uri.ipAddress;
}

DeviceInformation unserializeDeviceInformation(const QString &str)
{
    DeviceInformation info;
    info.unserialize(str);
    return info;
}

QString devInfoPrint(const DeviceInformation &debugInfo)
{
    QString info = QString  (
              QString("\n")
            + QString("+++++++++++++++++++++++++++++++++\n")
            + QString("protocolType is: ") + protocolMap.key(debugInfo.protocolType) + QString('\n')
            + QString("deviceType   is: ") + deviceTypeMap.key(debugInfo.deviceType) + QString('\n')
            + QString("name         is: ") + debugInfo.name         + QString('\n')
            + QString("vendor       is: ") + debugInfo.vendor       + QString("\n")
            + QString("model        is: ") + debugInfo.model        + QString("\n")
            + QString("serial       is: ") + debugInfo.serial       + QString("\n")
            + QString("uri          is: ") + debugInfo.uri          + QString("\n")
            + QString("packageName  is: ") + packageInfoListToString(debugInfo.packageInfoList) + QString("\n")
            + QString("makeAndModel is: ") + debugInfo.makeAndModel + QString("\n")
            + QString("uuid         is: ") + debugInfo.uuid         + QString("\n")
            + QString("ieee1284ID   is: ") + debugInfo.ieee1284ID   + QString("\n")
            + QString("info         is: ") + debugInfo.info         + QString("\n")
    );
    if (debugInfo.deviceType == DeviceType::USB) {
        info += QString (
              QString("sysPath      is: ") + debugInfo.sysPath      + QString('\n')
            + QString("devicePath   is: ") + debugInfo.devicePath   + QString("\n")
            + QString("usbClass     is: ") + debugInfo.usbClass     + QString("\n")
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

void getPackagesNameFromHttp(DeviceInformation &device)
{
    QStringList res;
    QMap<QString, QString> messages;

    if (device.model.isEmpty() && device.vendor.isEmpty() && device.PID.isEmpty() && device.VID.isEmpty()) {
        return ;
    }

    if (device.vendor.isEmpty()) {
        return ;
    }

    if (device.vendor.contains("Unknown", Qt::CaseInsensitive)) {
        return ;
    }
    QString scheme;
    if (GsettingsMonitor::getInstance()->apiServerAddressGet() == "172.20.184.183:8888") {
        scheme = "http";
    } else {
        scheme = "https";
    }
    // https://api.kylinos.cn/api/v1/getprinterdrive?systemVersion=V10SP1&product=Brother&model=HL-3190CDW&arch=mips64el&manufacter=HP
    // https://api.kylinos.cn/api/v1/getprinterdrive?systemVersion=V10Professional&arch=amd64&manufacter=HP&model=Color%20LaserJet%20MFP%20M277c6
    QString httpRequest = QString("%1://%2/api/v1/getprintertype?systemVersion=%3&arch=%4%5%6%7%8%9%10%11")
                                .arg(scheme)
                                .arg(GsettingsMonitor::getInstance()->apiServerAddressGet())
                                .arg(BaseInfo::instance().getSystemVersion())
                                .arg(BaseInfo::instance().getDebianArchitecture())
                                .arg(device.PID.size() ? QString("&pid=" + device.PID) : "")
                                .arg(device.VID.size() ? QString("&vid=" + device.VID) : "")
                                .arg(device.vendor.size() ? QString("&manufacter=" + device.vendor) : "")
                                .arg(device.model.size() ? QString("&model=" + device.model) : "")
                                .arg("&appName=kylin-printer")
                                .arg("&USBclass=07")
                                .arg(device.serial.size() ?QString("sn=" + device.serial) : "");
    // httpRequest = "http://172.20.184.183:8888/api/v1/getprintertype?systemVersion=V10SP1&arch=amd64&pid=2109&vid=0817&manufacter=HP&model=Color%20LaserJet%20Pro%20M254dn&appName=kylin-printer-test&usb_class=07&sn=00FDDD";
    // httpRequest = "http://172.20.184.183:8888/api/v1/getprintertype?systemVersion=V10SP1&arch=amd64&manufacter=Fuji-Xerox&model=ApeosPort-VII C3373&appName=kylin-printer&usb_class=07";
    
    if(httpRequest.contains("+")){
        httpRequest.replace("+", "%2B");
    }
    messages.insert("pid", device.PID);
    messages.insert("vid", device.VID);
    messages.insert("vendor", device.vendor);
    messages.insert("model", device.model);
    messages.insert("httpRequest", httpRequest);

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

    int statusCode = netReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    messages.insert("httpStatusCode", QString("%1").arg(statusCode));

    if (netReply->error() != QNetworkReply::NoError) {
        qDebug() << netReply->errorString();
        messages.insert("result", "fail");
        messages.insert("resultString", "netreply error!");
        messages.insert("networkErrorCode", QString("%1").arg(netReply->error()));
        messages.insert("networkErrorString", netReply->errorString());
        InformationCollector::getInstance().addMessageMap("get_package_list", messages);
        return ;
    }

    QByteArray strRateAll = netReply->readAll();
//    strRateAll = "{\"code\":200,\"message\":\"请求成功！\",\"data\":[{\"packageName\":\"hplip_3.21.4.3-1_amd64.deb\",\"type\":\"printer\",\"others\":null,\"ppdName\":\"uld-hp/HP_Color_Laser_15x_Series.ppd\",\"ppdScore\":65,\"packageLevel\":\"Level 1\"}],\"timestamp\":1710816773616,\"lastRequestTime\":\"2024-03-19 10:52:53\"}";
    messages.insert("httpReply", strRateAll);

    qDebug() << "http result is"<<strRateAll;
    if (strRateAll == "") {
        messages.insert("result", "fail");
        messages.insert("resultString", "netreply is empty!");
        messages.insert("networkErrorCode", QString("%1").arg(netReply->error()));
        messages.insert("networkErrorString", netReply->errorString());
        InformationCollector::getInstance().addMessageMap("get_package_list", messages);
        qDebug() << "http request error: cannot get info form: " << httpRequest;
        return ;
    }

    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(strRateAll, &jsonParserError );

    if ( jsonDocument.isNull() || jsonParserError.error != QJsonParseError::NoError ) {
        qDebug () << "json解析失败";
        messages.insert("result", "fail");
        messages.insert("resultString", "json parse error!");
        InformationCollector::getInstance().addMessageMap("get_package_list", messages);
        return ;
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
                QJsonObject packages = jsonArray.at(i).toObject();
                qDebug() << packages;
                if (   !packages.contains("type")
                    || !packages.contains("packageName")
                    || !packages.contains("others")
                    || !packages.contains("packageLevel")
                    || !packages.contains("ppdName")
                    || !packages.contains("ppdScore")
                    ) {
                    continue;
                }

                if(packages.value("packageLevel").isString()) {
                    QString packageLevel = packages.value("packageLevel").toString();
                    if (device.packageLevel.isEmpty() && !packageLevel.isEmpty())
                        device.packageLevel = packageLevel;
                }
                if (packages.value("ppdName").isString() && packages.value("ppdScore").isDouble()) {
                    int score = qFloor(packages.value("ppdScore").toDouble());
                    if (score > device.suggestPpdScore) {
                        device.suggestPpdScore = score;
                        device.suggestPpdName  = packages.value("ppdName").toString();
                    }
                }

                if (packages.value("packageName").isString()) {
                    res.append(packages.value("packageName").toString());
                }
            }
        }
    }
    QSet<QString> resSet = res.toSet();
    QStringList debPackages = resSet.toList();
    QStringList debNameList{QStringList()};
    for (auto package : debPackages) {
        if (package.contains("\n")) {
            auto pkglist = package.split("\n");
            debNameList.append(pkglist);
        } else {
            debNameList.append(package);
        }
    }

    for (int i = 0; i < debNameList.size(); i++) {
        PackageInfo info;
        QString name = debNameList.at(i);
        info.name = name.left(name.indexOf("_"));
        info.version = getDebVersion(name);
        device.packageInfoList.append(info);
    }

    messages.insert("packagesName", packageInfoListToString(device.packageInfoList));

    if (device.packageInfoList.isEmpty()) {
        messages.insert("result", "fail");
        messages.insert("resultString", "packagesName is empty!");
    } else {
        messages.insert("result", "success");
    }

    InformationCollector::getInstance().addMessageMap("get_package_list", messages);


    return ;
}

void getPackagesNameFromLocal(DeviceInformation &device)
{
    QStringList res;

    QDBusMessage msg = QDBusMessage::createMethodCall(
        "com.kylin-os-manager-driver-acquirer",
        "/driveracquirer",
        "driveracquirer.tool",
        "QueryDevices");

    // 设置参数（确保是 QList<QVariant>）
    QList<QVariant> args;
    QDBusConnection bus = QDBusConnection::systemBus();
    args << device.PID << device.VID << device.vendor << device.model << "printer";

    qDebug() << args;

    msg.setArguments(args);

    // 发送并检查返回值
    QString strRateAll;
    QDBusMessage response = bus.call(msg);
    if (response.type() == QDBusMessage::ErrorMessage) {
        qDebug() << "D-Bus 调用失败：" << response.errorMessage();
    } else {
        strRateAll = response.arguments().at(0).toString();
    }

    qDebug() << "search result: "<<strRateAll;
    // {"digit_id":"","driver":"cn.kylin.lanxum-gm6229=1.0.0,cn.kylin.lanxum-scan=2.1.0","others":null,"packageLevel":null,"packageName":"cn.kylin.lanxum-gm6229_x86_64_1.0.0_stable_binary_squashfks.ok\ncn.kylin.lanxum-scan_x86_64_2.1.0_stable_binary_squashfks.ok","ppdName":null,"type":"printer"}]
    // strRateAll = "{\"code\":200,\"message\":\"请求成功！\",\"data\":[{\"packageName\":\"hplip_3.21.4.3-1_amd64.deb\",\"type\":\"printer\",\"others\":null,\"ppdName\":\"uld-hp/HP_Color_Laser_15x_Series.ppd\",\"ppdScore\":65,\"digit_id\":\"666\",\"packageLevel\":\"1\"}],\"timestamp\":1710816773616,\"lastRequestTime\":\"2024-03-19 10:52:53\"}";
    if (strRateAll == "") {
        qDebug() << "get fail";
        return ;
    }
    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(strRateAll.toUtf8(), &jsonParserError );
    qDebug() << strRateAll;
    if ( jsonDocument.isNull() || jsonParserError.error != QJsonParseError::NoError ) {
        qDebug () << "json解析失败";
        return ;
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
                           || !packages.contains("digit_id")
                        //    || !packages.contains("packageLevel")
                           ) {
                        continue ;
                    }

                    if (!packages.value("type").isString()) {
                        continue ;
                    }

                    if(packages.value("type").toString() != "printer") {
                        continue;
                    }
                    
                    if (!packages.value("packageName").isString()) {
                        continue ;
                    }

                    device.digitId = packages.value("digit_id").toString();

                    if(packages.value("packageLevel").isString()) {
                        QString packageLevel = packages.value("packageLevel").toString();
                        if (device.packageLevel.isEmpty() && !packageLevel.isEmpty())
                            device.packageLevel = packageLevel;
                    }
                    if (packages.value("ppdName").isString() && packages.value("ppdScore").isDouble()) {
                        int score = qFloor(packages.value("ppdScore").toDouble());
                        if (score > device.suggestPpdScore) {
                            device.suggestPpdScore = score;
                            device.suggestPpdName  = packages.value("ppdName").toString();
                        }
                    }

                    if (packages.contains("driver") 
                     && packages.value("driver").isString()){
                        QStringList packageList = packages.value("driver").toString().split(",");
                        for (auto package : packageList) {
                            if (!package.contains("=")) {
                                continue;
                            }
                            PackageInfo info;
                            
                            info.name = package.split("=").at(0);
                            info.version = package.split("=").at(1);
                            device.packageInfoList.append(info);
                        }
                    } else {
                        res.append(packages.value("packageName").toString());
                        QSet<QString> resSet = res.toSet();
                        QStringList debPackages = resSet.toList();
                            QStringList debNameList{QStringList()};
                            for (auto package : debPackages) {
                                if (package.contains("\n")) {
                                    auto pkglist = package.split("\n");
                                    debNameList.append(pkglist);
                                } else {
                                    debNameList.append(package);
                                }
                            }

                            for (int i = 0; i < debNameList.size(); i++) {
                                PackageInfo info;
                                QString name = debNameList.at(i);
                                info.name = name.left(name.indexOf("_"));
                                info.version = getDebVersion(name);
                                device.packageInfoList.append(info);
                            }
                    }

                    // 默认仅处理第一个
                    break;
                }
            }
        }
    }

    return ;
}

QString getNameFromDeviceInformation(const DeviceInformation &info)
{
    QString name;
    if (info.vendor.size() && info.model.size()) {
        name = info.vendor + "-" + info.model;
        name.replace(" ", "-");
        name.replace("/", "-");
        name.replace("\\", "-");
        name.replace("'", "-");
        name.replace('"', "-");
        name.replace("?", "-");
        name.replace("#", "-");
        name.replace("<", "-");
        name.replace(">", "-");
    } else {
        name = info.info + " (" +info.uri + ")";
    }
    return name;
}

QString getPrinterNameFromDeviceInformation(const DeviceInformation &info)
{
    QString name;
    if (info.protocolType == ProtocolType::SMB
     || info.protocolType == ProtocolType::LPD
     || info.deviceType == DeviceType::IPP_REMOTE) {
        name = info.name;
    } else {
        if (info.vendor.isEmpty() || info.model.isEmpty()) {
            name = "printers";
        } else { 
            name = info.vendor + "-" + info.model;
        }
    }

    name.replace(" ", "-");
    name.replace("/", "-");
    name.replace("\\", "-");
    name.replace("'", "-");
    name.replace('"', "-");
    name.replace("?", "-");
    name.replace("#", "-");
    name.replace("<", "-");
    name.replace(">", "-");

    return name;
}

void postDeviceInfo(const DeviceInformation &device, int ppdRank)
{
    QJsonObject jsonObject;
    jsonObject["arch"] = BaseInfo::instance().getDebianArchitecture();
    jsonObject["pid"] = device.PID;
    jsonObject["vid"] = device.VID;
    jsonObject["manufacter"] = device.vendor;
    jsonObject["model"] = device.model;
    jsonObject["ppdName"] = device.userChoosePpdName;
    jsonObject["ppdRank"] = ppdRank;
    jsonObject["packageLevel"] = device.packageLevel;
    jsonObject["systemVersion"] = BaseInfo::instance().getSystemVersion();


    // 将JSON对象转换为JSON文档
    QJsonDocument jsonDocument(jsonObject);
    QByteArray jsonData = jsonDocument.toJson();

    // 创建网络请求并设置URL
    QNetworkAccessManager manager;
    QString scheme;
    if (GsettingsMonitor::getInstance()->apiServerAddressGet() == "172.20.184.183:8888") {
        scheme = "http";
    } else {
        scheme = "https";
    }
    QString httpRequest = QString("%1://%2/api/v1/addUserPpd").arg(scheme).arg(GsettingsMonitor::getInstance()->apiServerAddressGet());
    qDebug() << httpRequest;
    qDebug() << jsonData;
    QNetworkRequest request;

    // 设置请求头部信息
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setUrl(QUrl(httpRequest));

    // 发送POST请求
    QNetworkReply *reply;
    reply = manager.post(request, jsonData);
    QEventLoop loop;
    QTimer timer;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    timer.start(10000);

    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "Request succeeded, response: " << reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "HTTP Status Code: " << statusCode;
    } else {
        qDebug() << "Request failed, error: " << reply->errorString();
    }
    reply->deleteLater();

}
