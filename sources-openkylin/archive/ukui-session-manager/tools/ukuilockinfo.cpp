/*
* Copyright (C) Copyright 2021 KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/
#include "ukuilockinfo.h"
#include "../ukui-session/xdgdesktopfile.h"

#include <QDBusInterface>
#include <QSettings>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <glib.h>
#include <QDBusMetaType>

QDBusArgument &InhibitInfo::operator<<(QDBusArgument &argument, const InhibitInfo::InhibitorInfo &mystruct)
{
    argument.beginStructure();
    argument << mystruct.name << mystruct.icon << mystruct.uid;
    argument.endStructure();
    return argument;
}

const QDBusArgument &InhibitInfo::operator>>(const QDBusArgument &argument, InhibitInfo::InhibitorInfo &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.name >> mystruct.icon >> mystruct.uid;
    argument.endStructure();
    return argument;
}

Ukuilockinfo::Ukuilockinfo()
{
}

QVector<InhibitInfo::InhibitorInfo> Ukuilockinfo::listInhibitorInfo(Ukuilockinfo::InhibitorType type)
{
    QVector<InhibitInfo::InhibitorInfo> result;
    switch (type) {
    case InhibitorType::logout: {
        getLogoutInhibitor(result);
        break;
    }
    case InhibitorType::suspend: {
        getSystemdInhibitor(QString("sleep"), result);
        break;
    }
    case InhibitorType::shutdown: {
        getSystemdInhibitor(QString("shutdown"), result);
        break;
    }
    }

    return result;
}

bool Ukuilockinfo::getCfgValue(Ukuilockinfo::buttonType button)
{
    bool buttonActive = false;
    bool newIniFile = false;//ini文件是否为新建文件
    QString iniDir = "/usr/share/ukui/ukui-session-manager/config";
    if (!QFile::exists(iniDir + "/btnconfig.ini")) {
        qDebug() << "btnconfig.ini file is not exists!!!";

        QDir dir(iniDir);
        if (!dir.exists(iniDir)) {
            if (dir.mkdir(iniDir)) {//目前创建不成功  没有权限
                QFile iniFile(iniDir + "/btnconfig.ini");
                if (iniFile.open(QIODevice::WriteOnly)) {
                    newIniFile = true;
                    iniFile.close();
                }
                qDebug() << "inifile open faile!";
            } else {
                qDebug() << "create inidir faile!";
            }

        }
    }
    QSettings *cfgSettings = new QSettings("/usr/share/ukui/ukui-session-manager/config/btnconfig.ini", QSettings::IniFormat);

    if (newIniFile) {//貌似路径下文件只可读不可写
        cfgSettings->setValue("btn/SwitchUserBtnHide", false);
        cfgSettings->setValue("btn/HibernateBtnHide", false);
        cfgSettings->setValue("btn/LockScreenBtnHide", false);
        cfgSettings->setValue("btn/LogoutBtnHide", false);
        cfgSettings->setValue("btn/RebootBtnHide", false);
        cfgSettings->setValue("btn/ShutDownBtnHide", false);
        cfgSettings->setValue("btn/SuspendBtnHide", false);
    }

    switch (button) {
    case switchuserBtn:
        buttonActive = !(cfgSettings->value("btn/SwitchUserBtnHide").toBool());
        break;
    case hibernateBtn:
        buttonActive = !(cfgSettings->value("btn/HibernateBtnHide").toBool());
        break;
    case suspendBtn:
        buttonActive = !(cfgSettings->value("btn/SuspendBtnHide").toBool());
        break;
    case lockscreenBtn:
        buttonActive = !(cfgSettings->value("btn/LockScreenBtnHide").toBool());
        break;
    case logoutBtn:
        buttonActive = !(cfgSettings->value("btn/LogoutBtnHide").toBool());
        break;
    case rebootBtn:
        buttonActive = !(cfgSettings->value("btn/RebootBtnHide").toBool());
        break;
    case shutdownBtn:
        buttonActive = !(cfgSettings->value("btn/ShutDownBtnHide").toBool());
        break;
    default:
        break;
    }

    return buttonActive;
}

void Ukuilockinfo::getSystemdInhibitor(QString type, QVector<InhibitInfo::InhibitorInfo> &inhibitorVec)
{
    qDBusRegisterMetaType<SystemdInhibitor::Inhibitor>();

    QDBusInterface loginInterface("org.freedesktop.login1", "/org/freedesktop/login1",
                                  "org.freedesktop.login1.Manager", QDBusConnection::systemBus());

    if (loginInterface.isValid()) {
        qDebug() << "create interface success";
    }

    QDBusMessage result = loginInterface.call("ListInhibitors");
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        SystemdInhibitor::Inhibitor inhibitor;
        dbusArgs >> inhibitor;
        if (inhibitor.mode == QString("block") && inhibitor.action.contains(type)) {
                InhibitInfo::InhibitorInfo inhibitInfo;
                findNameAndIcon(inhibitor.name, inhibitInfo);

                if (inhibitInfo.name.isEmpty()) {
                    inhibitInfo.name = inhibitor.name;
                }
                inhibitInfo.uid = inhibitor.uid;
                inhibitorVec.push_back(inhibitInfo);
        }
    }

    dbusArgs.endArray();
}

void Ukuilockinfo::findNameAndIcon(QString &inhibitorName, InhibitInfo::InhibitorInfo &inhibitor)
{
    QString icon;
    QString name;
    QStringList desktop_paths = { "/usr/share/applications", "/etc/xdg/autostart" };

    //逻辑问题，规避文件管理器这种多个desktop中exec字段均为peony的情况
    if ("peony" == inhibitorName) {
        XdgDesktopFile desktopFile;
        desktopFile.load("/usr/share/applications/peony.desktop");
        icon = desktopFile.value("Icon", QVariant("application-x-desktop")).toString();
//                name = getAppLocalName(fi.absoluteFilePath());
        //根据系统的本地语言设置获取对应的名称,改为直接使用xdg的接口
        name = desktopFile.localizedValue("Name").toString();
        inhibitor.name = name;
        inhibitor.icon = icon;
        return;
    }

    for (const QString &dirName : const_cast<const QStringList&>(desktop_paths)) {
        QDir dir(dirName);
        if (!dir.exists()) {
            continue;
        }

        const QFileInfoList files = dir.entryInfoList(QStringList(QLatin1String("*.desktop")), QDir::Files | QDir::Readable);
        for (const QFileInfo &fi : files) {
            XdgDesktopFile desktopFile;
            desktopFile.load(fi.absoluteFilePath());
            //不再使用basename判断，使用命令判断
            //QString base = fi.baseName();
            if (desktopFile.expandExecString().isEmpty()) continue;
            QString command = desktopFile.expandExecString().takeFirst();
            QStringList list = command.split("/");
            QString exec = list.last();
            if (exec == inhibitorName) {
                icon = desktopFile.value("Icon", QVariant("application-x-desktop")).toString();
//                name = getAppLocalName(fi.absoluteFilePath());
                //根据系统的本地语言设置获取对应的名称,改为直接使用xdg的接口
                name = desktopFile.localizedValue("Name").toString();

                inhibitor.name = name;
                inhibitor.icon = icon;

                break;
            }
        }
    }
}

//QString Ukuilockinfo::getAppLocalName(QString desktopfp)
//{
//    GError **error = nullptr;
//    GKeyFileFlags flags = G_KEY_FILE_NONE;
//    GKeyFile *keyfile = g_key_file_new();

//    QByteArray fpbyte = desktopfp.toLocal8Bit();
//    char *filepath = fpbyte.data();
//    g_key_file_load_from_file(keyfile, filepath, flags,error);

//    char *name = g_key_file_get_locale_string(keyfile, "Desktop Entry", "Name", nullptr, nullptr);
//    QString namestr = QString::fromLocal8Bit(name);

//    g_key_file_free(keyfile);
//    return namestr;
//}

void Ukuilockinfo::getLogoutInhibitor(QVector<InhibitInfo::InhibitorInfo> &inhibitorVec)
{
    QDBusInterface sessionInterface("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                    "org.gnome.SessionManager", QDBusConnection::sessionBus());

    if (!sessionInterface.isValid()) {
        qDebug() << "interface not usable";
        return;
    }

    QDBusMessage result = sessionInterface.call("GetInhibitors");
    auto res = result.arguments().toVector();
    for (auto it = res.begin(); it != res.end(); ++it) {
        QString info = it->toString();
        if (info.isEmpty()) {
            continue;
        }
        QStringList infolist = info.split("/");
        InhibitInfo::InhibitorInfo inhibitorInfo;
        inhibitorInfo.name = infolist[0];
        inhibitorInfo.icon = infolist[2];
        inhibitorVec.push_back(inhibitorInfo);
    }
}

QDBusArgument &SystemdInhibitor::operator<<(QDBusArgument &argument, const SystemdInhibitor::Inhibitor &mystruct)
{
    argument.beginStructure();
    argument << mystruct.action << mystruct.name << mystruct.reason << mystruct.mode << mystruct.uid << mystruct.pid;
    argument.endStructure();
    return argument;
}

const QDBusArgument &SystemdInhibitor::operator>>(const QDBusArgument &argument, SystemdInhibitor::Inhibitor &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.action >> mystruct.name >> mystruct.reason >> mystruct.mode >> mystruct.uid >> mystruct.pid;
    argument.endStructure();
    return argument;
}
