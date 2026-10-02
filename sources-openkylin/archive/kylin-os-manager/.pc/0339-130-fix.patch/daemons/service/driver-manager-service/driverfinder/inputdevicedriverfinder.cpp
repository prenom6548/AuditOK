#include "inputdevicedriverfinder.h"
#include "../hardwarefinder/inputdevicefinder.h"
#include <QHostInfo>
#include <QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QTimer>
#include <QSysInfo>
#include <QByteArray>
#include <QDataStream>
#include <QProcess>
#include <QVersionNumber>
#include "kom-configure.h"
#include "../include/hostresolver.h"

static QString getDebianArchitecture();

static QString getRetFromCommand(const QStringList &command);

static bool checkInstallable(const QString name, const QString version);

inputdevicedriverfinder::inputdevicedriverfinder(QObject *parent) : QObject(parent) {
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &inputdevicedriverfinder::dowork);

    connect(this, &inputdevicedriverfinder::finished, m_thread, &QThread::quit);
    connect(this, &inputdevicedriverfinder::finished, this, &inputdevicedriverfinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

inputdevicedriverfinder::~inputdevicedriverfinder() {

}

QList<HardWareInfo> inputdevicedriverfinder::getInputDevInfoList() {
    return m_inputDevInfoList;
}

void inputdevicedriverfinder::dowork() {
    m_usbInfoList.clear();
    m_inputDevInfoList.clear();

    if (tryConnectFromHost(apiServerAddressGet())) {
        m_isServerAddressConnected = false;
    } else {
        m_isServerAddressConnected = true;
    }

    inputDeviceFinder inputDevFinder;
    inputDevFinder.dowork();
    m_usbInfoList = inputDevFinder.getUsbDeviceList();

    if(m_isServerAddressConnected){
        for (int i = 0; i < m_usbInfoList.length(); ++i) {
            getPackagesNameFromHttp(m_usbInfoList[i]);
        }
    }

    checkDriverVersion();
    getDriverContent();
    upgradeableCheck();
    emit succeed();
}

QString inputdevicedriverfinder::apiServerAddressGet()
{
    kom::Configure conf;
    QString domain = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_DOMAIN).toString();
    if (domain == "") {
        return "api.kylinos.cn";
    }
    return domain;
}

void inputdevicedriverfinder::convertStrToDrinfo(QStringList drivers, DriverInfo &driverInfo) {
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

int inputdevicedriverfinder::tryConnectFromHost(const QString &hostname) {
    int timeout = 2000; // 超时时间2秒
    int result = HostResolver::resolve(hostname, timeout);
    qDebug() << "解析结果：" << result;
    return result;
}

QStringList inputdevicedriverfinder::getPackagesNameFromHttp(HardWareInfo &hardwareinfo) {
    QStringList res;

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
            .arg(hardwareinfo.PID.size() ? QString("&pid=" + hardwareinfo.PID) : "")
            .arg(hardwareinfo.VID.size() ? QString("&vid=" + hardwareinfo.VID) : "")
            .arg(hardwareinfo.vendor.size() ? QString("&manufacter=" + hardwareinfo.vendor) : "")
            .arg(hardwareinfo.model.size() ? QString("&model=" + hardwareinfo.model) : "");
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

    if (netReply->error() != QNetworkReply::NoError) {
        return res;
    }

    QByteArray strRateAll = netReply->readAll();
    qDebug() << "http result is" << strRateAll;
    if (strRateAll == "") {
        qDebug() << "http request error: cannot get info form: " << httpRequest;
        return res;
    }
    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(strRateAll, &jsonParserError);
    qDebug() << strRateAll;
    if (jsonDocument.isNull() || jsonParserError.error != QJsonParseError::NoError) {
        qDebug() << "json解析失败";
        return res;
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

                    if (!packages.value("type").isString()) {
                        continue;
                    }

                    hardwareinfo.type = DeviceType::InputDev;

                    if (!packages.value("packageName").isString()) {
                        continue;
                    }

                    hardwareinfo.others += packages.value("others").toString();

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
                driverinfo.driverName += QString(QString(pkglist[i]).left(QString(pkglist[i]).indexOf("_")) + ";");
            }
        } else {
            driverinfo.driverName = QString(package).left(QString(package).indexOf("_")) + ";";
        }
        hardwareinfo.driverNameList.append(driverinfo);
    }

    for (int i = 0; i < hardwareinfo.driverNameList.length(); ++i) {
        QStringList partDriver = hardwareinfo.driverNameList.at(i).driverName.split(";");
        partDriver.removeLast();
        res.append(partDriver);
    }


    for (auto usbinfo: m_usbInfoList) {
        if (usbinfo.type == DeviceType::InputDev) {
            m_inputDevInfoList.append(usbinfo);
        }
    }
    return res;
}

void inputdevicedriverfinder::checkDriverVersion() {
    for (int i = 0; i < m_inputDevInfoList.length(); ++i) {
        if (m_inputDevInfoList.at(i).driverNameList.isEmpty()) {
            continue;
        }
        QStringList driver;
        for (int k = 0; k < m_inputDevInfoList.at(i).driverNameList.length(); ++k) {
            driver.append(m_inputDevInfoList.at(i).driverNameList[k].driverName);
        }
        for (int j = 0; j < driver.length(); j++) {
            QString packageName = driver.at(j);
            QString res{QString()};
            QString cmd = QString("/usr/bin/dpkg-query -W -f='${Version}\\n' %1").arg(packageName);
            res = getRetFromCommand(cmd.split(" "));
            if (res != "") {
                m_inputDevInfoList[i].driver_now += packageName + ";";
                m_inputDevInfoList[i].driver_version_now += res + ";";
            }
        }
    }
}

void inputdevicedriverfinder::upgradeableCheck() {
    for (int i = 0; i < m_inputDevInfoList.length(); ++i) {
        if (m_inputDevInfoList.at(i).driverNameList.isEmpty()) {
            m_inputDevInfoList[i].upgradeable = false;
            continue;
        }

        // 判断本地 apt-cache 中是否有这个驱动版本
        for (int j = 0; j < m_inputDevInfoList.at(i).driverNameList.length(); ++j) {
            if (!checkInstallable(m_inputDevInfoList[i].driverNameList[j].driverName.trimmed(),
                                  m_inputDevInfoList[i].driverNameList[j].version.trimmed())) {
                m_inputDevInfoList[i].upgradeable = false;
                continue;
            }
        }

        if ((!m_inputDevInfoList.at(i).driverNameList.isEmpty() && m_inputDevInfoList.at(i).driver_now == "")) {
            m_inputDevInfoList[i].upgradeable = true;
            continue;
        }

        for (int j = 0; j < m_inputDevInfoList.at(i).driverNameList.length(); ++j) {
            QVersionNumber v1 = QVersionNumber::fromString(m_inputDevInfoList.at(i).driver_version_now.split(";")[j]);
            QVersionNumber v2 = QVersionNumber::fromString(m_inputDevInfoList.at(i).driverNameList[j].version);
            if (v2 > v1) {
                m_inputDevInfoList[i].upgradeable = true;
                break;
            }
        }
    }
}

void inputdevicedriverfinder::getDriverContent() {
    for (int i = 0; i < m_inputDevInfoList.length(); ++i) {
        QStringList drivers;
        for (int j = 0; j < m_inputDevInfoList[i].driverNameList.length(); ++j) {
            drivers.append(m_inputDevInfoList[i].driverNameList[j].driverName);
        }
        for (int k = 0; k < drivers.length(); ++k) {
            QString commond = "apt-cache show " + drivers[k];
            QProcess proc;
            proc.start(commond);
            proc.waitForFinished();
            QStringList outPut = QString(proc.readAllStandardOutput()).split("\n\n");
            for (int m = 0; m < outPut.length(); ++m) {
                if (QString(outPut[m]).contains(m_inputDevInfoList[i].driverNameList[k].version.trimmed())) {
                    QStringList driver = QString(outPut[m]).split("\n");
                    convertStrToDrinfo(driver, m_inputDevInfoList[i].driverNameList[k]);
                }
            }
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