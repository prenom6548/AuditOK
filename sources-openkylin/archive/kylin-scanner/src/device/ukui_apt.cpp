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

#include "ukui_apt.h"
#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <QtDBus>
#include <QTimer>

static const QString KUM_DEST = "com.kylin.UpgradeStrategies";
static const QString KUM_PATH = "/com/kylin/UpgradeStrategies";
static const QString KUM_IFACE = "com.kylin.UpgradeStrategies.interface";

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
    connect(this, &AptUtilHelper::succeed, this, &AptUtilHelper::finished);
    connect(this, &AptUtilHelper::failed, this, &AptUtilHelper::finished);
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
    if(m_device.packageNameList.isEmpty())
        {
            emit failed("PackageList is Empty");
            return;
        }


    if (m_device.packageNameList.isEmpty()){
        getPackagesNameFromHttp(m_device);
    }
    //安装对应版本
    if(m_device.packageNameList.length() == m_device.packageVersion.length()){
        for (int i = 0; i < m_device.packageNameList.length(); ++i) {
            QString package = m_device.packageNameList[i] + "=" + m_device.packageVersion[i];
            m_packages.append(package);
        }
    }else{
        m_packages = m_device.packageNameList;
    }
    qDebug() << m_packages;
    installPackage(m_packages);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, [=](){emit failed(QObject::tr("Install timeout."));});
    m_timer->start(600000);
    qDebug("Worker Ended!");
}

void AptUtilHelper::onInstalldebStatusChanged(int progress , QString status, QString current_details)
{
    qDebug() << QString("InstalldebStatusChanged progress = %1 , status = %2 ,current_details = %3").arg(progress).arg(status).arg(current_details);
}


void AptUtilHelper::onRecvApt(bool success, QStringList updateGroup, QString errorString, QString errorDesc) {
    qDebug() << "onReceiveKumAptSignal" << success;
    if(success == false) {
        //Error
        qDebug() << "Install package failed..."<< errorString << " " << errorDesc;
        m_packages.clear();
        emit failed(errorString);
    }else if(success == true) {
        qDebug() << "Installed." << m_packages;
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
    dbus_iface.setTimeout(600000);
    QDBusMessage message = dbus_iface.call("InstallPackages", packageName);
        qDebug() << message;
        if (!checkDBusMessage(message)) {
            emit failed("install failed");
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
    qDebug("Deb Worker Started!");
    qDebug() << m_debName;
    installLocalDeb(m_debName);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, [=](){emit failed(QObject::tr("Install timeout."));});
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
        qDebug() << "Install package failed..."<< errorString << " " << errorDesc;
        emit failed(errorString);
    }else if(success == true) {
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
    qDebug() << dbus_iface.call("InstallDebFile", "kylin-scanner", debFilePath, true, true, QLocale::system().name());
}
