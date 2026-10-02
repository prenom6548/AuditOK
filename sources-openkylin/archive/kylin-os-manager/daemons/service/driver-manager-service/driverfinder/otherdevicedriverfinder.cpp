#include "otherdevicedriverfinder.h"
#include "../hardwarefinder/otherdevicefinder.h"
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

otherDeviceDriverFinder::otherDeviceDriverFinder(QObject *parent) : QObject(parent) {
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &otherDeviceDriverFinder::dowork);

    connect(this, &otherDeviceDriverFinder::finished, m_thread, &QThread::quit);
    connect(this, &otherDeviceDriverFinder::finished, this, &otherDeviceDriverFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

otherDeviceDriverFinder::~otherDeviceDriverFinder() {

}

QList<HardWareInfo> otherDeviceDriverFinder::getOtherDevInfoList() {
    return m_otherDevInfoList;
}

void otherDeviceDriverFinder::dowork() {
    m_usbInfoList.clear();
    m_otherDevInfoList.clear();

    if (tryConnectFromHost(apiServerAddressGet())) {
        m_isServerAddressConnected = false;
    } else {
        m_isServerAddressConnected = true;
    }

    OtherDeviceFinder otherDevFinder;
    otherDevFinder.dowork();
    m_usbInfoList = otherDevFinder.getUsbDeviceList();

    if (m_isServerAddressConnected) {
        for (int i = 0; i < m_usbInfoList.length(); ++i) {
            getPackagesNameFromHttp(m_usbInfoList[i]);
        }
    }

    checkDriverVersion();
    getDriverContent();
    upgradeableCheck();
    emit succeed();
}

QString otherDeviceDriverFinder::apiServerAddressGet()
{
    kom::Configure conf;
    QString domain = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_DOMAIN).toString();
    if (domain == "") {
        return "api.kylinos.cn";
    }
    return domain;
}

void otherDeviceDriverFinder::convertStrToDrinfo(QStringList drivers, DriverInfo &driverInfo) {
    for (int i = 0; i < drivers.length(); i++) {
        QStringList lines = QString(drivers[i]).split("\n");
        for (int j = 0; j < lines.length(); j++) {

            // 获取驱动包架构
            if (QString(lines[j]).contains("Architecture:", Qt::CaseInsensitive)) {
                driverInfo.arch = QString(lines[j]).split(": ")[1] + " ";
                continue;
            }

            // 获取驱动包版本
            // 必须行首匹配，避免误匹配 runtime_version 等字段
            if (QString(lines[j]).trimmed().startsWith("Version:", Qt::CaseInsensitive)) {
                driverInfo.version = QString(lines[j]).split(": ")[1] + " ";
                continue;
            }

            // 获取驱动包优先级
            if (QString(lines[j]).contains("Priority:", Qt::CaseInsensitive)) {
                driverInfo.priority = QString(lines[j]).split(": ")[1] + " ";
                continue;
            }

            // 获取驱动包所属组别
            if (QString(lines[j]).contains("Section:", Qt::CaseInsensitive)) {
                driverInfo.section = QString(lines[j]).split(": ")[1] + " ";
                continue;
            }

            // 获取驱动包源码包
            if (QString(lines[j]).contains("Source:", Qt::CaseInsensitive)) {
                driverInfo.source = QString(lines[j]).split(": ")[1] + " ";
                continue;
            }

            // 获取驱动包来源
            if (QString(lines[j]).contains("Origin:", Qt::CaseInsensitive)) {
                driverInfo.origin = QString(lines[j]).split(": ")[1] + " ";
                continue;
            }

            // 获取驱动包维护者信息
            if (QString(lines[j]).contains("Maintainer:", Qt::CaseInsensitive)) {
                driverInfo.maintainer = QString(lines[j]).split(": ")[1] + " ";
                continue;
            }

            // 获取驱动包bug报告信息
            if (QString(lines[j]).contains("Bugs:", Qt::CaseInsensitive)) {
                driverInfo.bugs = QString(lines[j]).split(": ")[1] + " ";
                continue;
            }

            // 获取驱动安装后占用空间信息
            if (QString(lines[j]).contains("Installed-Size:", Qt::CaseInsensitive)) {
                driverInfo.installedSize = QString(lines[j]).split(": ")[1] + " ";
                continue;
            }

            // 获取驱动包运行时依赖
            if (QString(lines[j]).contains("Depends:", Qt::CaseInsensitive)) {
                driverInfo.depends = QString(lines[j]).split(": ")[1] + " ";
                continue;
            }

            // 获取建议安装的相关软件包
            if (QString(lines[j]).contains("Recommends:", Qt::CaseInsensitive)) {
                driverInfo.recommends = QString(lines[j]).split(": ")[1] + " ";
                continue;
            }

            // 获取软件包的二进制文件存储在软件仓库中的路径
            if (QString(lines[j]).contains("Filename:", Qt::CaseInsensitive)) {
                driverInfo.filename = QString(lines[j]).split(": ")[1] + " ";
                continue;
            }

            // 获取软件包二进制文件在压缩状态下的大小
            if (QString(lines[j]).contains("Size:", Qt::CaseInsensitive)) {
                driverInfo.size = QString(lines[j]).split(": ")[1] + " ";
                continue;
            }
        }
    }
}

int otherDeviceDriverFinder::tryConnectFromHost(const QString &hostname) {
    int timeout = 2000; // 超时时间2秒
    int result = HostResolver::resolve(hostname, timeout);
    qDebug() << "解析结果：" << result;
    return result;
}

QStringList otherDeviceDriverFinder::getPackagesNameFromHttp(HardWareInfo &hardwareinfo) {
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

                    hardwareinfo.type = DeviceType::Other;

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
        if (usbinfo.type == DeviceType::Other) {
            m_otherDevInfoList.append(usbinfo);
        }
    }
    return res;
}

void otherDeviceDriverFinder::checkDriverVersion() {
    for (int i = 0; i < m_otherDevInfoList.length(); ++i) {
        if (m_otherDevInfoList.at(i).driverNameList.isEmpty()) {
            continue;
        }
        QStringList driver = m_otherDevInfoList.at(i).driverNameList[0].driverName.split(";");
        driver.removeLast();
        for (int j = 0; j < driver.length(); j++) {
            QString packageName = driver.at(j);
            QString res{QString()};
            QString cmd = QString("/usr/bin/dpkg-query -W -f='${Version}\\n' %1").arg(packageName);
            res = getRetFromCommand(cmd.split(" "));
            if (res != "") {
                m_otherDevInfoList[i].driver_now = packageName + ";";
                m_otherDevInfoList[i].driver_version_now = res + ";";
            }
        }
    }
}

void otherDeviceDriverFinder::getDriverContent() {
    for (int i = 0; i < m_otherDevInfoList.length(); ++i) {
        for (int j = 0; j < m_otherDevInfoList[i].driverNameList.length(); ++j) {
            if (m_otherDevInfoList[i].driverNameList[j].driverName.contains(";")) {
                QStringList drivers = m_otherDevInfoList[i].driverNameList[j].driverName.split(";");
                drivers.removeLast();
                for (int k = 0; k < drivers.length(); ++k) {
                    QString commond = "apt-cache show " + drivers[k];
                    QProcess proc;
                    proc.start(commond);
                    proc.waitForFinished();
                    QStringList outPut = QString(proc.readAllStandardOutput()).split("\n\n");
                    QStringList driver = QString(outPut[0]).split("\n");
                    convertStrToDrinfo(driver, m_otherDevInfoList[i].driverNameList[j]);
                }
            }
        }
    }
}

void otherDeviceDriverFinder::upgradeableCheck() {
    for (int i = 0; i < m_otherDevInfoList.length(); ++i) {
        if(m_otherDevInfoList.at(i).driverNameList.isEmpty()){
            m_otherDevInfoList[i].upgradeable = false;
            continue;
        }

        if ((!m_otherDevInfoList.at(i).driverNameList.isEmpty() && m_otherDevInfoList.at(i).driver_now == "")) {
            m_otherDevInfoList[i].upgradeable = true;
            continue;
        }

        if (m_otherDevInfoList.at(i).driver_version_now != m_otherDevInfoList.at(i).driverNameList[0].version) {
            QVersionNumber v1 = QVersionNumber::fromString(m_otherDevInfoList.at(i).driver_version_now);
            QVersionNumber v2 = QVersionNumber::fromString(m_otherDevInfoList.at(i).driverNameList[0].version);
            if (v2 > v1) {
                m_otherDevInfoList[i].upgradeable = true;
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
