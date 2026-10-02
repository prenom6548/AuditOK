#include "aptinstaller.h"
#include <QDebug>
#include <QtDBus/QtDBus>
#include <QVersionNumber>

AptInstaller::AptInstaller(QStringList driver, QObject *parent) : QObject(parent), m_driverInfo(driver)
{
    auto sysBus = QDBusConnection::systemBus();
    sysBus.connect(KUM_DEST, KUM_PATH, KUM_IFACE, QString("UpdateInstallFinished"),
                   this, SLOT(onRecvApt(bool, QStringList, QString, QString)));
    sysBus.connect(KUM_DEST, KUM_PATH, KUM_IFACE, QString("InstalldebStatusChanged"),
                   this, SLOT(onInstalldebStatusChanged(int, QString, QString)));

    m_thread = new QThread;
    moveToThread(m_thread);
    connect(m_thread, &QThread::started, this, &AptInstaller::processPkg);
    connect(this, &AptInstaller::succeed, this, &AptInstaller::finished);
    connect(this, &AptInstaller::failed, this, &AptInstaller::finished);
    connect(this, &AptInstaller::finished, m_thread, &QThread::quit);
    connect(this, &AptInstaller::finished, this, &AptInstaller::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

AptInstaller::~AptInstaller()
{

}

void AptInstaller::onInstalldebStatusChanged(int progress , QString status, QString current_details)
{
    qDebug() << QString("InstalldebStatusChanged progress = %1 , status = %2 ,current_details = %3").arg(progress).arg(status).arg(current_details);
    emit progressChanged(progress);
}

void AptInstaller::onRecvApt(bool success, QStringList updateGroup, QString errorString, QString errorDesc)
{
    Q_UNUSED(updateGroup);
    qDebug() << "onReceiveKumAptSignal" << success;
    if(success == false) {
        //Error
        qDebug() << "Install package failed..."<< errorString << " " << errorDesc;
        m_driverInfo.clear();
        emit failed(errorString);
    }else if(success == true) {
        qDebug() << "Installed." << m_driverInfo;
        emit succeed();
    } else {
        qDebug() << "installing...";
    }
}

void AptInstaller::processPkg()
{
    qDebug() << "start apt install pkg";
    qDebug() << "Waitting for install pkg: " << m_driverInfo;
    installPackage(m_driverInfo);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, [=](){emit failed(QObject::tr("Install timeout."));});
    m_timer->start(60000000);
    qDebug("Worker Ended!");
}

void AptInstaller::installPackage(QStringList packageName)
{
    qDebug() << "Package:" << packageName;
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface dbus_iface(KUM_DEST, KUM_PATH, KUM_IFACE, bus);
    dbus_iface.setTimeout(600000);
    qDebug() << dbus_iface.call("InstallPackages", packageName);
}

QStringList AptInstaller::getDriverInfo() {
    return m_driverInfo;
}


