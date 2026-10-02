/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "personalizeddata.h"
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDate>
#include <QTime>
#include <QDBusArgument>
#include <QDBusReply>

#define LIGHTDMDATA_ROOT_PATH "/var/lib/lightdm-data/"
#define ACT_DBUS_SERVICE "org.freedesktop.Accounts"
#define ACT_DBUS_PATH "/org/freedesktop/Accounts"
#define ACT_DBUS_INTERFACE "org.freedesktop.Accounts"
#define ACT_USER_INTERFACE "org.freedesktop.Accounts.User"
#define DBUS_PROP_INTERFACE "org.freedesktop.DBus.Properties"

enum enum_operation
{
    enum_operation_update = 0,
    enum_operation_add,
    enum_operation_del,
};

PersonalizedData::PersonalizedData(QString user)
{
    KyInfo() << user;
    m_user = user;
    m_usd_conf_path = QString("/var/lib/lightdm-data/%1/usd/config/ukui-settings-daemon.settings").arg(m_user);
    m_greeter_conf_path = QString("/var/lib/lightdm-data/%1/ukui-greeter.conf").arg(m_user);
    m_user_path = QString("/var/lib/lightdm-data/%1/").arg(m_user);
    m_file_watch = new QFileSystemWatcher();
    connect(m_file_watch, &QFileSystemWatcher::fileChanged, this, &PersonalizedData::fileChanged);
    connect(m_file_watch, &QFileSystemWatcher::directoryChanged, this, &PersonalizedData::directoryChanged);
    if (this->__load_conf_ukuigreeterconf() != 0) {
        if (QFileInfo(m_user_path).exists()) {
            if (!m_file_watch->directories().contains(m_user_path)) {
                m_file_watch->addPath(m_user_path);
            }
        } else if (QFileInfo(LIGHTDMDATA_ROOT_PATH).exists()) {
            if (!m_file_watch->directories().contains(LIGHTDMDATA_ROOT_PATH)) {
                m_file_watch->addPath(LIGHTDMDATA_ROOT_PATH);
            }
        }
        qDebug() << "init ukuigreeter conf failed:" << m_user << ",monitor:" << m_file_watch->directories();
    }
    // 无需读取usd配置
    //  this->__load_conf_usd_conf();
}

PersonalizedData::~PersonalizedData()
{
    KyInfo() << m_user;
    if (0 != m_conf_TimerID) {
        this->killTimer(m_conf_TimerID);
        m_conf_TimerID = 0;
    }
    if (nullptr != m_file_watch) {
        delete m_file_watch;
        m_file_watch = nullptr;
    }
}

void PersonalizedData::getJsonData(QJsonObject &json)
{
    json["user"] = m_user;
    {
        QJsonObject json_1;
        json_1["dateType"] = m_dateType;
        json_1["fontSize"] = m_fontSize;
        json_1["timeType"] = m_timeType;
        json_1["backgroundPath"] = m_backgroundPath;
        json_1["color"] = m_color;
        json["greeter"] = json_1;
    }
}

QString PersonalizedData::getJsonData()
{
    QJsonObject json;
    this->getJsonData(json);
    json["operation"] = enum_operation_update;
    QJsonDocument document;
    document.setObject(json);
    return document.toJson(QJsonDocument::Compact);
}

void PersonalizedData::fileChanged(const QString &path)
{
    KyInfo() << path;
    if (m_greeter_conf_path == path) {
        m_greeter_conf_path_changed = true;
    }
    if (m_usd_conf_path == path) {
        m_usd_conf_path_changed = true;
    }

    if (0 != m_conf_TimerID) {
        this->killTimer(m_conf_TimerID);
        m_conf_TimerID = 0;
    }
    m_conf_TimerID = this->startTimer(100);
}

void PersonalizedData::directoryChanged(const QString &path)
{
    qDebug() << "Dirctory Changed:" << path;
    if (LIGHTDMDATA_ROOT_PATH == path) {
        if (QFileInfo(m_user_path).exists()) {
            if (!m_file_watch->directories().contains(m_user_path)) {
                m_file_watch->addPath(m_user_path);
            }
        } else {
            QTimer::singleShot(100, this, [=]() {
                if (QFileInfo(m_user_path).exists()) {
                    if (!m_file_watch->directories().contains(m_user_path)) {
                        m_file_watch->addPath(m_user_path);
                    }
                }
            });
        }
    } else if (m_user_path == path) {
        if (QFileInfo(m_greeter_conf_path).exists()) {
            fileChanged(m_greeter_conf_path);
        } else {
            QTimer::singleShot(100, this, [=]() {
                if (QFileInfo(m_greeter_conf_path).exists()) {
                    fileChanged(m_greeter_conf_path);
                }
            });
        }
    }
}

void PersonalizedData::timerEvent(QTimerEvent *event)
{
    KyInfo() << m_file_watch->files();
    if (event->timerId() == m_conf_TimerID) {
        this->killTimer(m_conf_TimerID);
        m_conf_TimerID = 0;

        if (m_greeter_conf_path_changed) {
            m_greeter_conf_path_changed = false;
            this->__load_conf_ukuigreeterconf();
        }
        if (m_usd_conf_path_changed) {
            m_usd_conf_path_changed = false;
            this->__load_conf_usd_conf();
        }
        emit KYLINUSERDATAMNG::getInstance()->conf_changed(this->getJsonData());
    }
}

int PersonalizedData::__load_conf_ukuigreeterconf()
{
    if (!QFileInfo(m_greeter_conf_path).exists()) {
        KyWarning() << "config file not exist:" << m_greeter_conf_path;
        return -1;
    }
    QSettings conf(m_greeter_conf_path, QSettings::IniFormat);
    if (QSettings::NoError != conf.status()) {
        KyWarning() << m_greeter_conf_path << conf.status();
        return -1;
    }

    if (conf.childGroups().contains("Greeter")) {
        conf.beginGroup("Greeter");

        if (conf.contains("dateType"))
            m_dateType = conf.value("dateType", m_dateType).toString();

        if (conf.contains("fontSize"))
            m_fontSize = conf.value("fontSize", m_fontSize).toInt();

        if (conf.contains("timeType"))
            m_timeType = conf.value("timeType", m_timeType).toInt();

        conf.endGroup();
    }

    if (conf.childGroups().contains("greeter")) {
        conf.beginGroup("greeter");

        if (conf.contains("backgroundPath")) {
            m_backgroundPath = conf.value("backgroundPath", m_backgroundPath).toString();
            if (!m_backgroundPath.isEmpty()) {
                m_backgroundPath = this->__copy_file(m_backgroundPath);
            }
        }

        if (conf.contains("color"))
            m_color = conf.value("color", m_color).toString();

        conf.endGroup();
    }

    if (nullptr != m_file_watch && !m_file_watch->files().contains(m_greeter_conf_path)) {
        m_file_watch->addPath(m_greeter_conf_path);
    }
    return 0;
}

int PersonalizedData::__load_conf_usd_conf()
{
    KyInfo();
    QSettings conf(m_usd_conf_path, QSettings::IniFormat);
    if (QSettings::NoError != conf.status()) {
        KyWarning() << m_usd_conf_path << conf.status();
        return -1;
    }

    if (conf.childGroups().contains("xsettings")) {
        conf.beginGroup("xsettings");

        if (conf.contains("cursor-size"))
            m_cursor_size = conf.value("cursor-size", m_cursor_size).toInt();

        if (conf.contains("cursor-theme"))
            m_cursor_theme = conf.value("cursor-theme", m_cursor_theme).toString();

        if (conf.contains("scaling-factor"))
            m_scaling_factor = conf.value("scaling-factor", m_scaling_factor).toInt();

        conf.endGroup();
    }

    if (nullptr != m_file_watch && !m_file_watch->files().contains(m_usd_conf_path)) {
        m_file_watch->addPath(m_usd_conf_path);
    }
    return 0;
}

QString PersonalizedData::__copy_file(QString oldfile)
{
    if (QFile::exists(oldfile)) {
        QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        QString newfile = QString("%1/loginBackground-%2-%3-XXXXXX")
                              .arg(tempPath)
                              .arg(m_user)
                              .arg(QDate::currentDate().toString("yyyy-MM-dd"));

        if (QFile::exists(newfile)) {
            QFile::remove(newfile);
        }
        if (QFile::copy(oldfile, newfile)) {
            return newfile;
        }
    }
    return "";
}

/////////////////////////PersonalizedDataMng//////////////////////////////////////

PersonalizedDataMng::PersonalizedDataMng()
{
    initData();
}

void PersonalizedDataMng::initData()
{
    if (m_isDataInited) {
        return;
    }
    if (!m_actService) {
        m_actService
            = new QDBusInterface(ACT_DBUS_SERVICE, ACT_DBUS_PATH, ACT_DBUS_INTERFACE, QDBusConnection::systemBus());

        connect(
            m_actService, SIGNAL(UserAdded(const QDBusObjectPath &)), this, SLOT(onUserAdded(const QDBusObjectPath &)));
        connect(
            m_actService,
            SIGNAL(UserDeleted(const QDBusObjectPath &)),
            this,
            SLOT(onUserDeleted(const QDBusObjectPath &)));
    }
    this->__updateUsersInfo();
    m_isDataInited = true;
}

QString PersonalizedDataMng::getUserName(const QString &userPath)
{
    QDBusInterface iface(ACT_DBUS_SERVICE, userPath, DBUS_PROP_INTERFACE, QDBusConnection::systemBus());
    QDBusReply<QDBusVariant> reply = iface.call("Get", ACT_USER_INTERFACE, "UserName");
    QString strUserName = "";
    if (reply.isValid()) {
        strUserName = reply.value().variant().toString();
    }
    return strUserName;
}

PersonalizedDataMng::~PersonalizedDataMng() {}

QString PersonalizedDataMng::GetConfInformation(QString name)
{
    QJsonObject json;
    if (m_userPersonalizedData.contains(name)) {
        QJsonObject json1;
        m_userPersonalizedData[name]->getJsonData(json1);
        json[name] = json1;
    } else if (name.isEmpty()) {
        for (auto &it : m_userPersonalizedData.toStdMap()) {
            QJsonObject json1;
            it.second->getJsonData(json1);
            json[it.first] = json1;
        }
    } else if ("lightdm" == name) {
        QString tmp(m_lightdm_str);
        m_lightdm_str = "";
        return tmp;
    }

    QJsonDocument document;
    document.setObject(json);
    QString strJson(document.toJson(QJsonDocument::Compact));
    KyDebug() << strJson;
    return strJson;
}

void PersonalizedDataMng::SetLoginSynInformation(QString val)
{
    m_lightdm_str = val;
}

void PersonalizedDataMng::onUserAdded(const QDBusObjectPath &path)
{
    QString strUserName = getUserName(path.path());
    if (!strUserName.isEmpty()) {
        m_userPersonalizedData[strUserName] = KylinUserDatePtr(new PersonalizedData(strUserName));
        m_mapUserPaths[path.path()] = strUserName;
        QJsonObject json;
        m_userPersonalizedData[strUserName]->getJsonData(json);
        json["operation"] = enum_operation_add;
        QJsonDocument document;
        document.setObject(json);
        emit conf_changed(document.toJson());
    }
}

void PersonalizedDataMng::onUserDeleted(const QDBusObjectPath &path)
{
    QString strUserName = m_mapUserPaths[path.path()];
    if (!strUserName.isEmpty()) {
        m_userPersonalizedData.remove(strUserName);
        m_mapUserPaths.remove(path.path());
        QJsonObject json;
        json["operation"] = enum_operation_del;
        json["user"] = strUserName;
        QJsonDocument document;
        document.setObject(json);
        emit conf_changed(document.toJson());
    }
}

void PersonalizedDataMng::__updateUsersInfo()
{
    if (m_actService) {
        QDBusMessage ret = m_actService->call("ListCachedUsers");
        QList<QVariant> outArgs = ret.arguments();
        QVariant first = outArgs.at(0);
        const QDBusArgument &dbusArgs = first.value<QDBusArgument>();
        QDBusObjectPath path;
        dbusArgs.beginArray();
        while (!dbusArgs.atEnd()) {
            dbusArgs >> path;
            QString strUserName = getUserName(path.path());
            if (!strUserName.isEmpty()) {
                m_userPersonalizedData[strUserName] = KylinUserDatePtr(new PersonalizedData(strUserName));
                m_mapUserPaths[path.path()] = strUserName;
            }
        }
        dbusArgs.endArray();
    }
}
