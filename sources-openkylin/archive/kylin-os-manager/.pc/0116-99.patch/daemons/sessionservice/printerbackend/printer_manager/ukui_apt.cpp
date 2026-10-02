#include "ukui_apt.h"
#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <QtDBus>
#include <QTimer>
#include "information_collector.h"
#include "common.h"

static const QString KUM_DEST = "com.kylin.systemupgrade";
static const QString KUM_PATH = "/com/kylin/systemupgrade";
static const QString KUM_IFACE = "com.kylin.systemupgrade.interface";

AptUtilHelper::AptUtilHelper(DeviceInformation device,QObject *parent) : QObject(parent),m_device(device)
{
    auto sysBus = QDBusConnection::systemBus();
    sysBus.connect(KUM_DEST, KUM_PATH, KUM_IFACE, QString("UpdateInstallFinished"),
        this, SLOT(onRecvApt(bool, QStringList, QString, QString)));
    sysBus.connect(KUM_DEST, KUM_PATH, KUM_IFACE, QString("InstalldebStatusChanged"),
                   this, SLOT(onInstalldebStatusChanged(int, QString, QString)));
    m_thread = new QThread;
    moveToThread(m_thread);
    connect(m_thread, &QThread::started, this, &AptUtilHelper::processPkg);
    connect(this, &AptUtilHelper::canceled, this, &AptUtilHelper::handleCancel);
    connect(this, &AptUtilHelper::finished, m_thread, &QThread::quit);
    connect(this, &AptUtilHelper::finished, this, &AptUtilHelper::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

AptUtilHelper::~AptUtilHelper()
{
    qDebug() << "~~~~~~~~destroy...";
}

void AptUtilHelper::processPkg()
{
    qDebug("Pkg Worker Started!");
    //Get Package...

    if (m_device.packageNameList.isEmpty()) {
        getPackagesNameFromHttp(m_device);
    }
    if(m_device.packageNameList.isEmpty()) {
        m_installStatus = ukuiInstallStatus::UKUI_INSTALL_FAIL;
        emit failed();
        return;
    }
    m_messages.insert("packagesName", m_device.packageNameList.join(" "));
    m_messages.insert("installType", "InstallPkgs");
    for (auto debName : m_device.debNameList) {
        QString verison = getDebVersion(debName);
        if (verison.isEmpty()) {
            m_packages.append(debName.left(debName.indexOf("_")));
        }
        else {
            m_packages.append(debName.left(debName.indexOf("_")) + "=" + getDebVersion(debName));
        }
    }
    qDebug() << m_packages;
    installPackage(m_packages);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, [=](){
        m_timer->stop();
        stopAptTask();
        m_messages.insert("result", "fail");
        m_messages.insert("resultString", "install package time out!");
        InformationCollector::getInstance().addMessageMap("install_package", m_messages);
        this->failed();
    });
    m_timer->start(APT_INSTALL_MAX_MILLISECOND);
    qDebug("Worker Ended!");
}

void AptUtilHelper::stopAptTask()
{
    if (m_installStatus != ukuiInstallStatus::UKUI_INSTALL_IN_PROGRESS) {
        return;
    }
    m_installStatus = ukuiInstallStatus::UKUI_INSTALL_STOP;
    qDebug() << "run CancelDownload method";
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface dbus_iface(KUM_DEST,KUM_PATH, KUM_IFACE, bus);
    QDBusMessage message = dbus_iface.call("CancelDownload");
    qDebug() << message;
}

void AptUtilHelper::handleCancel()
{
    if (m_timer) {
        m_timer->stop();
    }
    stopAptTask();
    this->failed();
}

void AptUtilHelper::onInstalldebStatusChanged(int progress , QString status, QString current_details)
{
    qDebug() << QString("InstalldebStatusChanged progress = %1 , status = %2 ,current_details = %3").arg(progress).arg(status).arg(current_details);
}

void AptUtilHelper::onRecvApt(bool success, QStringList updateGroup, QString errorString, QString errorDesc) {
    qDebug() << "onReceiveKumAptSignal" << success;
    if(success == false) {
        //Error
        m_messages.insert("result", "fail");
        m_messages.insert("resultString", "install package failed!");
        m_messages.insert("errorString", errorString);
        m_messages.insert("errorDesc", errorDesc);
        InformationCollector::getInstance().addMessageMap("install_package", m_messages);
        qDebug() << "Install package failed..."<< errorString << " " << errorDesc;
        m_packages.clear();
        m_installStatus = ukuiInstallStatus::UKUI_INSTALL_FAIL;
        emit failed();
    }else if(success == true) {
        m_messages.insert("result", "success");
        InformationCollector::getInstance().addMessageMap("install_package", m_messages);
        qDebug() << "Installed." << m_packages;
        m_installStatus = ukuiInstallStatus::UKUI_INSTALL_SUCCESS;
        emit succeed();
    } else {
        qDebug() << "installing...";
    }
}

bool checkDBusMessage(const QDBusMessage &message) {
    if (message.arguments().size()>1 && message.arguments().at(0).isValid()) {
        if(message.arguments().at(0).type() == QVariant::Int) {
            if (message.arguments().at(0).value<int>() == 0) {
                return true;
            }
        }
    }
    return false;
}

void AptUtilHelper::installPackage(QStringList packageName)
{
    qDebug() << "Package:" << packageName;
    qDebug() << "installPackage 1:";
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface dbus_iface(KUM_DEST,KUM_PATH, KUM_IFACE, bus);
    QDBusMessage message = dbus_iface.call("InstallPackages", packageName);
    qDebug() << message;
    if (!checkDBusMessage(message)) {
        m_installStatus = ukuiInstallStatus::UKUI_INSTALL_FAIL;
        emit failed();
    } else {
        m_installStatus = ukuiInstallStatus::UKUI_INSTALL_IN_PROGRESS;
    }
}

DebUtilHelper::DebUtilHelper(QString debName,QObject *parent) :
    QObject(parent),
    m_debName(debName)
{
    auto sysBus = QDBusConnection::systemBus();
    sysBus.connect(KUM_DEST, KUM_PATH, KUM_IFACE, QString("InstalldebFinished"),
        this, SLOT(onRecvApt(bool, QString, QString)));
    sysBus.connect(KUM_DEST, KUM_PATH, KUM_IFACE, QString("InstalldebStatusChanged"),
        this, SLOT(onInstalldebStatusChanged(int, QString, QString)));
    m_thread = new QThread;
    moveToThread(m_thread);
    connect(m_thread, &QThread::started, this, &DebUtilHelper::processDeb);
    connect(this, &DebUtilHelper::succeed, this, &DebUtilHelper::finished);
    connect(this, &DebUtilHelper::failed, this, &DebUtilHelper::finished);
    connect(this, &DebUtilHelper::finished, m_thread, &QThread::quit);
    connect(this, &DebUtilHelper::finished, this, &DebUtilHelper::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    //thread->start();
}

DebUtilHelper::~DebUtilHelper()
{
    qDebug() << "~~~~~~~~destroy...";
}

void DebUtilHelper::processDeb()
{
    m_messages.insert("packagesName", m_debName);
    m_messages.insert("installType", "InstallDebFile");
    qDebug("Deb Worker Started!");
    qDebug() << m_debName;
    installLocalDeb(m_debName);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, [=](){
        m_messages.insert("result", "fail");
        m_messages.insert("resultString", "install package time out!");
        InformationCollector::getInstance().addMessageMap("install_package", m_messages);
        this->failed();
    });
    m_timer->start(60000);
    qDebug("Deb Worker Ended!");
}

void DebUtilHelper::onInstalldebStatusChanged(int progress , QString status, QString current_details)
{
    qDebug() << QString("InstalldebStatusChanged progress = %1 , status = %2 ,current_details = %3").arg(progress).arg(status).arg(current_details);
}

void DebUtilHelper::onRecvApt(bool success, QString errorString, QString errorDesc) {
    qDebug() << "onReceiveKumAptSignal" << success;
    if(success == false) {
        //Error
        m_messages.insert("result", "fail");
        m_messages.insert("resultString", "install package failed!");
        m_messages.insert("errorString", errorString);
        m_messages.insert("errorDesc", errorDesc);
        InformationCollector::getInstance().addMessageMap("install_package", m_messages);
        qDebug() << "Install package failed..."<< errorString << " " << errorDesc;
        emit failed();
    }else if(success == true) {
        m_messages.insert("result", "success");
        InformationCollector::getInstance().addMessageMap("install_package", m_messages);
        qDebug() << "Installed." << m_debName;
        emit succeed();
    } else {
        qDebug() << "installing...";
    }
}

void DebUtilHelper::installLocalDeb(QString debFilePath)
{
    qDebug() << "Package:" << debFilePath;
    qDebug() << "installLocalDeb 2:";
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface dbus_iface(KUM_DEST, KUM_PATH, KUM_IFACE, bus);
    qDebug() << dbus_iface.call("InstallDebFile", "kylin-os-manager", debFilePath, true, true, QLocale::system().name());
}
