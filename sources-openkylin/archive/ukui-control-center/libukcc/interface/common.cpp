/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "common.h"

#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QProcess>
#include <QFile>
#include <QRegularExpression>
#include <kysdk/kysdk-system/libkysysinfo.h>

#ifdef WITHKYSEC
#include <kysec/libkysec.h>
#include <kysec/status.h>
#endif

Common::Common()
{
}

Common::~Common()
{

}

void Common::centerToScreen(QWidget* widget) {
    if (!widget)
      return;
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = widget->width();
    int y = widget->height();
    widget->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
}

QVariantMap Common::getModuleHideStatus() {
    QDBusInterface m_interface( "org.ukui.ukcc.session",
                                "/",
                                "org.ukui.ukcc.session.interface",
                                QDBusConnection::sessionBus());

    QDBusReply<QVariantMap> obj_reply = m_interface.call("getModuleHideStatus");
    if (!obj_reply.isValid()) {
        qDebug()<<"execute dbus method getModuleHideStatus failed";
    }
    return obj_reply.value();
}

QString Common::getCpuInfo() {
    QFile file("/proc/cpuinfo");

    if (file.open(QIODevice::ReadOnly)) {
        QString buffer = file.readAll();
        QStringList modelLine = buffer.split('\n').filter(QRegularExpression("^model name"));
        QStringList modelLineWayland = buffer.split('\n').filter(QRegularExpression("^Hardware"));
        QStringList lines = buffer.split('\n');

        if (modelLine.isEmpty()) {
            if (modelLineWayland.isEmpty()) {
                return "Unknown";
            }
            modelLine = modelLineWayland;
        }


        int count = lines.filter(QRegularExpression("^processor")).count();

        QString result;
        result.append(modelLine.first().split(':').at(1));
        result = result.trimmed();

        return result;
    }

    return QString();
}


QString Common::getCpuArchitecture()
{
    QString cpuArchitecture;
    // 设置系统环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG","en_US");
    QProcess *process = new QProcess;
    process->setProcessEnvironment(env);
    process->start("lscpu");
    process->waitForFinished();

    QByteArray ba = process->readAllStandardOutput();

    delete process;
    QString cpuinfo = QString(ba.data());
    QStringList cpuinfo_list = cpuinfo.split("\n");
    for (int i = 0; i < cpuinfo_list.count(); i++) {
        QString mstring = cpuinfo_list.at(i);
        if (mstring.contains("Architecture")) {
            // 去除空格
            mstring = mstring.remove(QRegExp("\\s"));
            QStringList list = mstring.split(":");
            cpuArchitecture = list.at(1);
            break;
        }
    }
    return cpuArchitecture;
}

bool Common::isExistEffect() {
    QString filename = QDir::homePath() + "/.config/ukui-kwinrc";
    QSettings kwinSettings(filename, QSettings::IniFormat);

    QStringList keys = kwinSettings.childGroups();

    kwinSettings.beginGroup("Plugins");
    bool kwin = kwinSettings.value("blurEnabled", kwin).toBool();

    if (!kwinSettings.childKeys().contains("blurEnabled")) {
        kwin = true;
    }
    kwinSettings.endGroup();

    QFileInfo dir(filename);
    if (!dir.isFile()) {
        return true;
    }

    if (keys.contains("Compositing")) {
        kwinSettings.beginGroup("Compositing");
        QString xder;
        bool kwinOG = false;
        bool kwinEN = true;
        xder = kwinSettings.value("Backend", xder).toString();
        kwinOG = kwinSettings.value("OpenGLIsUnsafe", kwinOG).toBool();
        kwinEN = kwinSettings.value("Enabled", kwinEN).toBool();
        if ("XRender" == xder || kwinOG || !kwinEN) {
            return false;
        } else {
            return true;
        }
        kwinSettings.endGroup();
    }
    return true;
}

void Common::setKwinMouseSize(int size) {

    QString filename = QDir::homePath() + "/.config/kcminputrc";
    QSettings *mouseSettings = new QSettings(filename, QSettings::IniFormat);

    mouseSettings->beginGroup("Mouse");
    mouseSettings->setValue("cursorSize", size);
    mouseSettings->endGroup();

    delete mouseSettings;
    mouseSettings = nullptr;

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(5);
    args.append(0);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
}

bool Common::isWayland() {
    QString sessionType = getenv("XDG_SESSION_TYPE");

    if (!sessionType.compare("wayland", Qt::CaseSensitive)) {
       return true;
    } else {
        return false;
    }
}

bool Common::isOpenkylin()
{
    QString systemName = QString(QLatin1String(kdk_system_get_systemName()));
    if (systemName.compare("openkylin", Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

bool Common::isCommunity()
{
    QString filename = "/etc/os-release";
    QSettings osSettings(filename, QSettings::IniFormat);

    QString versionID = osSettings.value("VERSION_ID").toString();

    if (versionID.compare("22.04", Qt::CaseSensitive)) {
        return false;
    }
    return true;
}

QRect Common::sizeOnCursor()
{
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    return desk_rect;
}

bool Common::isTablet()
{
    QString projectName = QString(QLatin1String(kdk_system_get_projectSubName()));

    if (projectName.compare("mavis", Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

bool Common::isExitBattery()
{
    /* 默认机器没有电池 */
    bool hasBat = false;
    QDBusInterface *brightnessInterface = new QDBusInterface("org.freedesktop.UPower",
                                     "/org/freedesktop/UPower/devices/DisplayDevice",
                                     "org.freedesktop.DBus.Properties",
                                     QDBusConnection::systemBus());
    if (!brightnessInterface->isValid()) {
        qDebug() << "Create UPower Interface Failed : " << QDBusConnection::systemBus().lastError();
        return false;
    }

    QDBusReply<QVariant> briginfo;
    briginfo  = brightnessInterface ->call("Get", "org.freedesktop.UPower.Device", "PowerSupply");

    if (briginfo.value().toBool()) {
        hasBat = true ;
    }
    delete brightnessInterface;

    return hasBat;
}


QString Common::getHostName()
{
    QString hostname;
    // 设置系统环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG","en_US");
    QProcess *process = new QProcess;
    process->setProcessEnvironment(env);
    process->start("hostname");
    process->waitForFinished();

    QByteArray ba = process->readAllStandardOutput();

    delete process;
    hostname = ba.data();

    hostname.replace(QString("\n"),QString(""));
    return hostname;
}

bool Common::isZJY()
{
    QString filename = "/etc/os-release";
    QSettings osSettings(filename, QSettings::IniFormat);

    QString versionID = osSettings.value("PROJECT_CODENAME").toString();

    if (versionID.compare("v10sp1-zyj", Qt::CaseSensitive)) {
        return false;
    }
    return true;
}

bool Common::buriedSettings(QString pluginName, QString settingsName, QString action, QString value)
{
#ifdef SP2
    // 埋点数据
    char appName[] = "ukui-control-center";
    QByteArray actiontr = action.toLocal8Bit();  // toLocal8Bit 支持中文
    char *messageType = actiontr.data();

    KBuriedPoint pt[3];
    pt[0].key = "pluginName";
    std::string pluginStr = pluginName.toStdString();
    pt[0].value = pluginStr.c_str();

    pt[1].key = "settingsName";
    std::string settingStr = settingsName.toStdString();
    pt[1].value = settingStr.c_str();

    pt[2].key = "value";
    std::string valueStr = value.toStdString();
    pt[2].value = valueStr.c_str();

    if (kdk_buried_point(appName , messageType , pt , 3) == -1) {
        qDebug() << __FUNCTION__ << "messageType:" << action << "pluginName:" << pluginName
                 << "settingsName:" << settingsName << "value:" << value << "buried point fail !" << __LINE__;
        return false;
    }
#endif

    return true;
}

QString Common::boolToString(bool b)
{
    if (b) {
        return QString("true");
    } else {
        return QString("false");
    }
}

QString Common::getUkccVersion()
{
    FILE *pp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *q = NULL;
    QString version = "none";

    pp = popen("dpkg -l  ukui-control-center | grep  ukui-control-center", "r");
    if(NULL == pp)
        return version;

    while((read = getline(&line, &len, pp)) != -1){
        q = strrchr(line, '\n');
        *q = '\0';

        QString content = line;
        QStringList list = content.split(" ");

        list.removeAll("");

        if (list.size() >= 3)
            version = list.at(2);
    }

    free(line);
    line = NULL;
    pclose(pp);
    return version;
}

QString Common::getProductName()
{
    QDBusInterface ifc("com.control.center.qt.systemdbus",
                       "/",
                       "com.control.center.interface",
                       QDBusConnection::systemBus());
    QDBusReply<QString> res = ifc.call("getDmiDecodeRes", "-s system-product-name");
    return res.value().trimmed();
}
