#include "hotplugdevdriverfinder.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QProcess>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QVersionNumber>
#include "kom-configure.h"

static QString getDebianArchitecture();

static QString getRetFromCommand(const QStringList &command);

static bool checkInstallable(const QString name, const QString version);

HotplugDevDriverFinder::HotplugDevDriverFinder(QObject *parent) : QObject(parent) {

}

HotplugDevDriverFinder::~HotplugDevDriverFinder() {

}

QString HotplugDevDriverFinder::apiServerAddressGet()
{
    kom::Configure conf;
    QString domain = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_DOMAIN).toString();
    if (domain == "") {
        return "api.kylinos.cn";
    }
    return domain;
}

void HotplugDevDriverFinder::dowork(HardWareInfo &hwInfo, QString vid, QString pid, QString vendor, QString model, QString devtype) {
    QStringList res;
    hwInfo.driverNameList.clear();
    hwInfo.driver_now = "";
    hwInfo.driver_version_now = "";
    hwInfo.isOnline = true;

    kom::Configure conf;
    QString scheme = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_PROTOCOL).toString();
    if (scheme == ""){
        scheme = "https";
    }

    QString httpRequest = QString("%1://%2/api/v1/getprintertype?systemVersion=%3&arch=%4%5%6%7%8")
            .arg(scheme)
            .arg(QString(apiServerAddressGet()))
            .arg(QString("V10SP1"))
            .arg(getDebianArchitecture())
            .arg(pid.size() ? QString("&pid=" + pid) : "")
            .arg(vid.size() ? QString("&vid=" + vid) : "")
            .arg(vendor.size() ? QString("&manufacter=" + vendor) : "")
            .arg(model.size() ? QString("&model=" + model) : "");
    if (httpRequest.contains("+")) {
        httpRequest.replace("+", "%2B");
    }

    QNetworkAccessManager manager;
    QNetworkRequest netRequest;
    QNetworkReply *netReply;
    QEventLoop loop;
    qDebug() << "http link is :" << httpRequest;
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    netRequest.setUrl(QUrl(httpRequest));
    netReply = manager.get(netRequest);

    QTimer timer;
    QObject::connect(netReply, SIGNAL(finished()), &loop, SLOT(quit()));
    QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    timer.start(10000);
    loop.exec();


    QByteArray strRateAll = netReply->readAll();
    qDebug() << "http result is" << strRateAll;
    if (strRateAll == "") {
        qDebug() << "http request error: cannot get info form: " << httpRequest;
        return;
    }

    hwInfo.vendor = vendor;
    hwInfo.model = model;
    hwInfo.PID = pid;
    hwInfo.VID = vid;

    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(strRateAll, &jsonParserError);
    qDebug() << strRateAll;
    if (jsonDocument.isNull() || jsonParserError.error != QJsonParseError::NoError) {
        qDebug() << "json解析失败";
        return;
    } else {
        qDebug() << "json解析成功!";
    }
    if (jsonDocument.isObject()) {
        QJsonObject jsonObject = jsonDocument.object();
        if (jsonObject.contains("data")
            && jsonObject.value("data").isArray()) {

            QJsonArray jsonArray = jsonObject.value("data").toArray();
            for (int i = 0; i < jsonArray.size(); i++) {
                if (jsonArray.at(i).isObject()) {
                    QJsonObject packages = jsonArray.at(i).toObject();
                    qDebug() << packages;
                    if (!packages.contains("type")
                        || !packages.contains("packageName")
                        || !packages.contains("others")
                            ) {
                        continue;
                    }
                    QString outputType = "";
                    if (!packages.value("type").isString()) {
                        continue;
                    } else {
                        outputType = packages.value("type").toString();
                    }

                    if (outputType == "scanner") {
                        hwInfo.type = DeviceType::SCANNER;
                    } else if (outputType == "printer") {
                        hwInfo.type = DeviceType::PRINTER;
                    } else if (devtype == "07") {
                        hwInfo.type = DeviceType::PRINTER;
                    } else if (devtype == "06" || devtype == "ff") {
                        hwInfo.type = DeviceType::SCANNER;
                    }

                    if (!packages.value("packageName").isString()) {
                        continue;
                    }

                    hwInfo.others += packages.value("others").toString();

                    res.append(packages.value("packageName").toString());
                }
            }
        }
    }
    QSet<QString> resSet = res.toSet();
    QStringList debPackages = resSet.toList();

    for (auto package: debPackages) {
        DriverInfo driverinfo;
        if (package.contains("\n")) {
            QStringList pkglist = package.split("\n");
            for (int i = 0; i < pkglist.length(); ++i) {
                driverinfo.driverName = QString(QString(pkglist[i]).left(QString(pkglist[i]).indexOf("_")));
                driverinfo.version = QString(QString(pkglist[i]).split("_")[1]);
                hwInfo.driverNameList.append(driverinfo);
            }
        } else {
            driverinfo.driverName = QString(package).left(QString(package).indexOf("_"));
            driverinfo.version = QString(package).split("_")[1];
            hwInfo.driverNameList.append(driverinfo);
        }
    }

    for (int i = 0; i < hwInfo.driverNameList.length(); ++i) {
        QString partDriver = hwInfo.driverNameList.at(i).driverName;
        res.append(partDriver);
    }

    checkDriverVersion(hwInfo);
    getDriverContent(hwInfo);
    upgradeableCheck(hwInfo);

}

void HotplugDevDriverFinder::checkDriverVersion(HardWareInfo &hwInfo) {
        if (hwInfo.driverNameList.isEmpty()) {
            return;
        }
        QStringList driver;
        for (int k = 0; k < hwInfo.driverNameList.length(); ++k) {
            driver.append(hwInfo.driverNameList[k].driverName);
        }
        for (int j = 0; j < driver.length(); j++) {
            QString packageName = driver.at(j);
            QString res{QString()};
            QString cmd = QString("/usr/bin/dpkg-query -W -f='${Version}\\n' %1").arg(packageName);
            res = getRetFromCommand(cmd.split(" "));
            if (res != "") {
                hwInfo.driver_now += packageName + ";";
                hwInfo.driver_version_now += res + ";";
            }
        }

}

void HotplugDevDriverFinder::getDriverContent(HardWareInfo &hwInfo) {
    QStringList drivers;
    for (int j = 0; j < hwInfo.driverNameList.length(); ++j) {
        drivers.append(hwInfo.driverNameList[j].driverName);
    }
    for (int k = 0; k < drivers.length(); ++k) {
        QString commond = "apt-cache show " + drivers[k];
        QProcess proc;
        proc.start(commond);
        proc.waitForFinished();
        QStringList outPut = QString(proc.readAllStandardOutput()).split("\n\n");
        for (int m = 0; m < outPut.length(); ++m) {
            if (QString(outPut[m]).contains(hwInfo.driverNameList[k].version.trimmed())) {
                QStringList driver = QString(outPut[m]).split("\n");
                convertStrToDrinfo(driver, hwInfo.driverNameList[k]);
            }
        }
    }
}

void HotplugDevDriverFinder::convertStrToDrinfo(QStringList drivers, DriverInfo &driverInfo) {
    for (int j = 0; j < drivers.length(); j++) {
        // 获取驱动包架构
        if (QString(drivers[j]).contains("Architecture:", Qt::CaseInsensitive)) {
            if(QString(drivers[j]).split(": ").length() >= 2){
                driverInfo.arch = QString(drivers[j]).split(": ")[1];
                continue;
            }
        }

        // 获取驱动包版本
        if (QString(drivers[j]).contains("Version:", Qt::CaseInsensitive)) {
            if(QString(drivers[j]).split(": ").length() >= 2){
                driverInfo.version = QString(drivers[j]).split(": ")[1];
                continue;
            }
        }

        // 获取驱动包优先级
        if (QString(drivers[j]).contains("Priority:", Qt::CaseInsensitive)) {
            if(QString(drivers[j]).split(": ").length() >= 2){
                driverInfo.priority = QString(drivers[j]).split(": ")[1];
                continue;
            }
        }

        // 获取驱动包所属组别
        if (QString(drivers[j]).contains("Section:", Qt::CaseInsensitive)) {
            if(QString(drivers[j]).split(": ").length() >= 2){
                driverInfo.section = QString(drivers[j]).split(": ")[1];
                continue;
            }
        }

        // 获取驱动包源码包
        if (QString(drivers[j]).contains("Source:", Qt::CaseInsensitive)) {
            if(QString(drivers[j]).split(": ").length() >= 2){
                driverInfo.source = QString(drivers[j]).split(": ")[1];
                continue;
            }
        }

        // 获取驱动包来源
        if (QString(drivers[j]).contains("Origin:", Qt::CaseInsensitive)) {
            if(QString(drivers[j]).split(": ").length() >= 2){
                driverInfo.origin = QString(drivers[j]).split(": ")[1];
                continue;
            }
        }

        // 获取驱动包维护者信息
        if (QString(drivers[j]).contains("Maintainer:", Qt::CaseInsensitive)) {
            if(QString(drivers[j]).split(": ").length() >= 2){
                driverInfo.maintainer = QString(drivers[j]).split(": ")[1];
                continue;
            }
        }

        // 获取驱动包bug报告信息
        if (QString(drivers[j]).contains("Bugs:", Qt::CaseInsensitive)) {
            if(QString(drivers[j]).split(": ").length() >= 2){
                driverInfo.bugs = QString(drivers[j]).split(": ")[1];
                continue;
            }
        }

        // 获取驱动安装后占用空间信息
        if (QString(drivers[j]).contains("Installed-Size:", Qt::CaseInsensitive)) {
            if(QString(drivers[j]).split(": ").length() >= 2){
                driverInfo.installedSize = QString(drivers[j]).split(": ")[1];
                continue;
            }
        }

        // 获取驱动包运行时依赖
        if (QString(drivers[j]).contains("Depends:", Qt::CaseInsensitive)) {
            if(QString(drivers[j]).split(": ").length() >= 2){
                driverInfo.depends = QString(drivers[j]).split(": ")[1];
                continue;
            }
        }

        // 获取建议安装的相关软件包
        if (QString(drivers[j]).contains("Recommends:", Qt::CaseInsensitive)) {
            if(QString(drivers[j]).split(": ").length() >= 2){
                driverInfo.recommends = QString(drivers[j]).split(": ")[1];
                continue;
            }
        }

        // 获取软件包的二进制文件存储在软件仓库中的路径
        if (QString(drivers[j]).contains("Filename:", Qt::CaseInsensitive)) {
            if(QString(drivers[j]).split(": ").length() >= 2){
                driverInfo.filename = QString(drivers[j]).split(": ")[1];
                continue;
            }
        }

        // 获取软件包二进制文件在压缩状态下的大小
        if (QString(drivers[j]).contains("Size:", Qt::CaseInsensitive)) {
            if(QString(drivers[j]).split(": ").length() >= 2){
                driverInfo.size = QString(drivers[j]).split(": ")[1];
                continue;
            }
        }
    }
}

void HotplugDevDriverFinder::upgradeableCheck(HardWareInfo &hwInfo) {
    if (hwInfo.driverNameList.isEmpty()) {
        hwInfo.upgradeable = false;
        return;
    }

    // 判断本地 apt-cache 中是否有这个驱动版本
    for (int j = 0; j < hwInfo.driverNameList.length(); ++j) {
        if (!checkInstallable(hwInfo.driverNameList[j].driverName.trimmed(),
                              hwInfo.driverNameList[j].version.trimmed())) {
            hwInfo.upgradeable = false;
            return;
        }
    }

    if ((!hwInfo.driverNameList.isEmpty() && hwInfo.driver_now == "")) {
        hwInfo.upgradeable = true;
        return;
    }

    for (int j = 0; j < hwInfo.driverNameList.length(); ++j) {
        QVersionNumber v1 = QVersionNumber::fromString(hwInfo.driver_version_now.split(";")[j]);
        QVersionNumber v2 = QVersionNumber::fromString(hwInfo.driverNameList[j].version);
        if (v2 > v1) {
            hwInfo.upgradeable = true;
            break;
        }
    }
}

static QString getDebianArchitecture() {
    QStringList command = QStringList{"dpkg", "--print-architecture"};
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
