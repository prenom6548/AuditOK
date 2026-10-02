#include "printerdriverfinder.h"
#include "../hardwarefinder/printerfinder.h"
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
#include <QtConcurrent/QtConcurrent>
#include "kom-configure.h"
#include <QFuture>
#include <QFutureWatcher>
#include "../include/hostresolver.h"

static QString getDebianArchitecture();

static QString getRetFromCommand(const QStringList &command);

static bool checkInstallable(const QString name, const QString version);

printerDriverFinder::printerDriverFinder(QObject *parent) : QObject(parent) {
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &printerDriverFinder::dowork);

    connect(this, &printerDriverFinder::finished, m_thread, &QThread::quit);
    connect(this, &printerDriverFinder::finished, this, &printerDriverFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

printerDriverFinder::~printerDriverFinder() {

}

QList<HardWareInfo> printerDriverFinder::getPrinterInfoList() {
    return m_printerInfoList;
}

void printerDriverFinder::dowork() {
    m_usbInfoList.clear();
    m_printerInfoList.clear();

    if (tryConnectFromHost(apiServerAddressGet())) {
        m_isServerAddressConnected = false;
        qDebug() << "域名解析失败";
    } else {
        m_isServerAddressConnected = true;
        qDebug() << "域名解析成功";
    }

    printerFinder printerfinder;
    printerfinder.dowork();
    m_usbInfoList = printerfinder.getUsbDeviceList();

    if (m_isServerAddressConnected) {
        // 使用 QtConcurrent 进行并行处理
        QFutureWatcher<void> watcher;
        QFuture<void> future = QtConcurrent::map(m_usbInfoList, [this](HardWareInfo &info) {
            getPackagesNameFromHttp(info);
        });

        connect(&watcher, &QFutureWatcher<void>::finished, this, [this]() {
            checkDriverVersion();
            getDriverContent();
            upgradeableCheck();
            emit succeed();
        });

        watcher.setFuture(future);

        // 确保事件循环在运行
        QEventLoop loop;
        connect(&watcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit);
        loop.exec();
    } else {
        m_printerInfoList.append(m_usbInfoList);
        checkDriverVersion();
        getDriverContent();
        upgradeableCheck();
        emit succeed();
    }
    qDebug() << "获取打印机驱动完成";
}

void printerDriverFinder::convertStrToDrinfo(QStringList drivers, DriverInfo &driverInfo) {
    for (int j = 0; j < drivers.length(); j++) {
        // 获取驱动包架构
        if (QString(drivers[j]).contains("Architecture:", Qt::CaseInsensitive)) {
            if(QString(drivers[j]).split(": ").length() >= 2){
                driverInfo.arch = QString(drivers[j]).split(": ")[1];
                continue;
            }
        }

        // 获取驱动包版本
        // 必须行首匹配，避免误匹配 runtime_version 等字段
        if (QString(drivers[j]).trimmed().startsWith("Version:", Qt::CaseInsensitive)) {
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

int printerDriverFinder::tryConnectFromHost(const QString &hostname) {
    int timeout = 2000; // 超时时间2秒
    int result = HostResolver::resolve(hostname, timeout);
    qDebug() << "解析结果：" << result;
    return result;
}

QString printerDriverFinder::apiServerAddressGet()
{
    kom::Configure conf;
    QString domain = conf.value(CONFIG_DEVICE_MANAGER, CONFIG_DEVICE_MANAGER_DOMAIN).toString();
    if (domain == "") {
        return "api.kylinos.cn";
    }
    return domain;
}

QStringList printerDriverFinder::getPackagesNameFromHttp(HardWareInfo &hardwareinfo) {
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

                    if (!packages.value("type").isString() || (packages.value("type").toString() != "printer" &&
                                                               packages.value("type").toString() != "scanner")) {
                        continue;
                    }


                    if (packages.value("type").toString() == "scanner") {
                        hardwareinfo.type = DeviceType::SCANNER;
                    }

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
                driverinfo.driverName = QString(QString(pkglist[i]).left(QString(pkglist[i]).indexOf("_")));
                driverinfo.version = QString(QString(pkglist[i]).split("_")[1]);
                hardwareinfo.driverNameList.append(driverinfo);
            }
        } else {
            driverinfo.driverName = QString(package).left(QString(package).indexOf("_"));
            driverinfo.version = QString(package).split("_")[1];
            hardwareinfo.driverNameList.append(driverinfo);
        }
    }

    for (int i = 0; i < hardwareinfo.driverNameList.length(); ++i) {
        QString partDriver = hardwareinfo.driverNameList.at(i).driverName;
        res.append(partDriver);
    }

    if (hardwareinfo.type == DeviceType::PRINTER) {
        m_printerInfoList.append(hardwareinfo);
    }

    return res;
}

void printerDriverFinder::checkDriverVersion() {
    for (int i = 0; i < m_printerInfoList.length(); ++i) {
        if (m_printerInfoList.at(i).driverNameList.isEmpty()) {
            continue;
        }
        QStringList driver;
        for (int k = 0; k < m_printerInfoList.at(i).driverNameList.length(); ++k) {
            driver.append(m_printerInfoList.at(i).driverNameList[k].driverName);
        }
        for (int j = 0; j < driver.length(); j++) {
            QString packageName = driver.at(j);
            QString res{QString()};
            QString cmd = QString("/usr/bin/dpkg-query -W -f='${Version}\\n' %1").arg(packageName);
            res = getRetFromCommand(cmd.split(" "));
            if (res != "") {
                m_printerInfoList[i].driver_now += packageName + ";";
                m_printerInfoList[i].driver_version_now += res + ";";
            }
        }
    }
}

void printerDriverFinder::getDriverContent() {
    for (int i = 0; i < m_printerInfoList.length(); ++i) {
        QStringList drivers;
        for (int j = 0; j < m_printerInfoList[i].driverNameList.length(); ++j) {
            drivers.append(m_printerInfoList[i].driverNameList[j].driverName);
        }
        for (int k = 0; k < drivers.length(); ++k) {
            QString commond = "apt-cache show " + drivers[k];
            QProcess proc;
            proc.start(commond);
            proc.waitForFinished();
            QStringList outPut = QString(proc.readAllStandardOutput()).split("\n\n");
            for (int m = 0; m < outPut.length(); ++m) {
                if (QString(outPut[m]).contains(m_printerInfoList[i].driverNameList[k].version.trimmed())) {
                    QStringList driver = QString(outPut[m]).split("\n");
                    convertStrToDrinfo(driver, m_printerInfoList[i].driverNameList[k]);
                }
            }
        }
    }
}

void printerDriverFinder::upgradeableCheck() {
    for (int i = 0; i < m_printerInfoList.length(); ++i) {
        if (m_printerInfoList.at(i).driverNameList.isEmpty()) {
            m_printerInfoList[i].upgradeable = false;
            continue;
        }

        // 判断本地 apt-cache 中是否有这个驱动版本
        for (int j = 0; j < m_printerInfoList.at(i).driverNameList.length(); ++j) {
            if (!checkInstallable(m_printerInfoList[i].driverNameList[j].driverName.trimmed(),
                                  m_printerInfoList[i].driverNameList[j].version.trimmed())) {
                m_printerInfoList[i].upgradeable = false;
                m_cacheExist= false;
                break;
            }
        }

        if (!m_printerInfoList.at(i).driverNameList.isEmpty() && m_printerInfoList.at(i).driver_now == "" && m_cacheExist) {
            m_printerInfoList[i].upgradeable = true;
            continue;
        }

        if(m_cacheExist){
            for (int j = 0; j < m_printerInfoList.at(i).driverNameList.length(); ++j) {
                QVersionNumber v1 = QVersionNumber::fromString(m_printerInfoList.at(i).driver_version_now.split(";")[j]);
                QVersionNumber v2 = QVersionNumber::fromString(m_printerInfoList.at(i).driverNameList[j].version);
                if (v2 > v1) {
                    m_printerInfoList[i].upgradeable = true;
                    break;
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
        if (QString(resList[i]).trimmed().startsWith("Version:")) {
            installabelVersionList.append(QString(QString(resList[i]).split(":")[1]).trimmed());
        }
    }
    if (installabelVersionList.contains(version)) {
        return true;
    }
    return false;
}
