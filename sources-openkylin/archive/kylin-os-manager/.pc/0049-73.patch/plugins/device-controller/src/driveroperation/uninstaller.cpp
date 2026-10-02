#include "uninstaller.h"
#include <QDebug>
#include <QtDBus>
#include <QVersionNumber>
#include <QString>
#include <QProcess>
#include <unistd.h>
#include <pwd.h>

#include "../include/common.h"

static QString getUserName();

Uninstaller::Uninstaller(QString driver, QObject *parent) : QObject(parent), m_driverInfo(driver) {
    auto sysBus = QDBusConnection::systemBus();
    sysBus.connect(KUM_DEST, KUM_PATH, KUM_IFACE, QString("PurgePackagesFinished"),
                   this, SLOT(onRecvPurge(bool, QString, QString)));
    sysBus.connect(KUM_DEST, KUM_PATH, KUM_IFACE, QString("PurgePkgStatusChanged"),
                   this, SLOT(onUninstalldebStatusChanged(int, QString, QString)));
    m_thread = new QThread;
    moveToThread(m_thread);
    connect(m_thread, &QThread::started, this, &Uninstaller::processPkg);
    connect(this, &Uninstaller::succeed, this, &Uninstaller::finished);
    connect(this, &Uninstaller::failed, this, &Uninstaller::finished);
    connect(this, &Uninstaller::finished, m_thread, &QThread::quit);
    connect(this, &Uninstaller::finished, this, &Uninstaller::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}


Uninstaller::~Uninstaller() {

}

void Uninstaller::processPkg() {
    qDebug() << "start uninstall pkg";

    // 输出需安装的pkg
    QStringList pkgName;
    QStringList pkgtmp = m_driverInfo.split(" ");
    for (int i = 0; i < pkgtmp.length(); i=i+2) {
        if(pkgtmp[i] != ""){
            pkgName.append(pkgtmp[i]);
        }
    }

    qDebug() << "Waitting for uninstall pkg: " << pkgName;
    uninstallPackage(pkgName);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, [=](){emit failed("Uninstall timeout.");});
    m_timer->start(180000);
    qDebug("Worker Ended!");
}

void Uninstaller::uninstallPackage(QStringList packageName) {
    qDebug() << "Package:" << packageName;
    QString username = getUserName();
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface dbus_iface(KUM_DEST, KUM_PATH, KUM_IFACE, bus);
    qDebug() << dbus_iface.call("PurgePackages", packageName, username);
}

void Uninstaller::onUninstalldebStatusChanged(int progress , QString status, QString current_details) {
    qDebug() << QString("UninstalldebStatusChanged progress = %1 , status = %2 ,current_details = %3").arg(progress).arg(status).arg(current_details);
    emit progressChanged(progress);
}

void Uninstaller::onRecvPurge(bool success, QString errorString, QString errorDesc) {
    qDebug() << "onReceivePurgeSignal" << success;
    if(success == false) {
        //Error
        qDebug() << "Uninstall package failed..."<< errorString << " " << errorDesc;
        m_driverInfo="";
        emit failed(errorString);
    }else if(success == true) {
        qDebug() << "Uninstalled." << m_driverInfo;
        emit succeed();
    } else {
        qDebug() << "Uninstalling...";
    }
}

static QString getUserName()
{
    uid_t uid;
    uid_t NO_UID = -1;
    struct passwd *pw;

    errno = 0;
    uid = geteuid ();
    if (uid == -1 && errno) {
        qDebug () << "get uid error";
        return "";
    }
    pw = getpwuid (uid);
    if (!pw) {
        qDebug ("cannot find name for user ID %lu", (unsigned long int) uid);
        return "";
    }
    return QString(pw->pw_name);
}
