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
#include "ukcccommon.h"

#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QProcess>
#include <QFile>
#include <QRegularExpression>
#include <kysdk/kysdk-system/libkysysinfo.h>

#define KWIN_SERVICE "org.ukui.KWin"
#define KWIN_PATH "/Compositor"
#define KWIN_INTERFACE "org.ukui.kwin.Compositing"

using namespace ukcc;

UkccCommon::UkccCommon()
{
}

UkccCommon::~UkccCommon()
{

}

void UkccCommon::centerToScreen(QWidget* widget) {
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

QVariantMap UkccCommon::getModuleHideStatus() {
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

QString UkccCommon::getCpuInfo() {
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


QString UkccCommon::getCpuArchitecture()
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

bool UkccCommon::isSupportsEffect()
{
    QDBusInterface kwin("org.ukui.KWin",
                       "/Effects",
                       "org.ukui.kwin.Effects",
                       QDBusConnection::sessionBus());
    QDBusReply<bool> res = kwin.call("isEffectSupported", "blur");
    return res;
}

void UkccCommon::setKwinMouseSize(int size) {

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

bool UkccCommon::isWayland() {
    QString sessionType = getenv("XDG_SESSION_TYPE");

    if (!sessionType.compare("wayland", Qt::CaseSensitive)) {
       return true;
    } else {
        return false;
    }
}

bool UkccCommon::isOpenkylin()
{
    QString systemName = QString(QLatin1String(kdk_system_get_systemName()));
    if (systemName.compare("openkylin", Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

bool UkccCommon::isCommunity()
{
    QString filename = "/etc/os-release";
    QSettings osSettings(filename, QSettings::IniFormat);

    QString versionID = osSettings.value("VERSION_ID").toString();

    if (versionID.compare("22.04", Qt::CaseSensitive)) {
        return false;
    }
    return true;
}

QRect UkccCommon::sizeOnCursor()
{
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    return desk_rect;
}

bool UkccCommon::isTablet()
{
    QString projectName = QString(QLatin1String(kdk_system_get_projectSubName()));

    if (projectName.compare("mavis", Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

bool UkccCommon::isExitBattery()
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


QString UkccCommon::getHostName()
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

bool UkccCommon::isZJY()
{
    QString filename = "/etc/os-release";
    QSettings osSettings(filename, QSettings::IniFormat);

    QString versionID = osSettings.value("PROJECT_CODENAME").toString();

    if (versionID.compare("v10sp1-zyj", Qt::CaseSensitive)) {
        return false;
    }
    return true;
}

bool UkccCommon::buriedSettings(QString pluginName, QString settingsName, QString action, QString value)
{
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

    return true;
}

QString UkccCommon::boolToString(bool b)
{
    if (b) {
        return QString("true");
    } else {
        return QString("false");
    }
}

QString UkccCommon::getUkccVersion()
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

bool UkccCommon::checkCompositorRunning()
{
    QDBusInterface dbus_iface (KWIN_SERVICE, KWIN_PATH, KWIN_INTERFACE, QDBusConnection::sessionBus());
    QVariant reply = dbus_iface.property ("active");
    bool isfoundCompositingManger = false;
    if (reply.toBool()) {
        isfoundCompositingManger = true;
    }

    if(!isfoundCompositingManger)
    {
        if(QGSettings::isSchemaInstalled("org.gnome.metacity"))
        {
            QProcess process;
            process.start("sh -c \"ps -e |grep metacity\"");
            if(process.waitForStarted(100)&&process.waitForFinished(100) && process.readAllStandardOutput().contains("metacity"))
            {
                QGSettings metacityGSettings("org.gnome.metacity", "/org/gnome/metacity/");
                isfoundCompositingManger = metacityGSettings.get("compositing-manager").toBool();
            }
        }
    }

    if(!isfoundCompositingManger)
    {
        if(QGSettings::isSchemaInstalled("org.mate.Marco.general"))
        {
            QProcess process;
            process.start("sh -c \"ps -e |grep marco\"");
            if(process.waitForStarted(100)&&process.waitForFinished(100) && process.readAllStandardOutput().contains("marco"))
            {
                QGSettings marcoGSettings("org.mate.Marco.general", "/org/mate/marco/general/");
                isfoundCompositingManger = marcoGSettings.get("compositing-manager").toBool();
            }
        }
    }

    return isfoundCompositingManger;
}


QString UkccCommon::getProductName()
{
    QDBusInterface ifc("com.control.center.qt.systemdbus",
                       "/",
                       "com.control.center.interface",
                       QDBusConnection::systemBus());
    QDBusReply<QString> res = ifc.call("getDmiDecodeRes", "-s system-product-name");
    return res.value().trimmed();
}

QString UkccCommon::getGpuName()
{
    QProcess process;
    QString gpuName;
    QString cmd = QString("lshw -C display");

    process.start(cmd);
    process.waitForFinished();

    QByteArray ba = process.readAllStandardOutput();
    QStringList result = QString::fromStdString(ba.toStdString()).split("\n");
    for (QString line : result) {
        if (line.contains("product")) {
            gpuName = line.split(":").at(1);
        }
    }
    qDebug() << Q_FUNC_INFO << gpuName.trimmed();
    return gpuName.trimmed();
}

bool UkccCommon::isHuawei()
{
    return (getCpuInfo().contains("HUAWEI", Qt::CaseInsensitive) || getCpuInfo().contains("PANGU", Qt::CaseInsensitive)) && isWayland();
}

/*
* 判断用户是否为域用户
* 和/etc/passwd文件中用户做对比
* 1：域用户，0：非域用户
*/
bool UkccCommon::isDomainUser(const char* username)
{
    FILE *fp;
    fp=fopen("/etc/passwd","r");
    if(fp == NULL)
    {
        return true;
    }
    char buf[1024], name[128];
    while(!feof(fp))
    {
        if(fgets(buf,sizeof (buf),fp) == NULL)
        {
            break;
        }
        sscanf(buf,"%[^:]",name);
        if(strcmp(name,username) == 0)
        {
            fclose(fp);
            return false;
        }
    }
    fclose(fp);
    return true;
}
