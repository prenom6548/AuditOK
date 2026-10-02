/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDBusInterface>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QJsonArray>

enum enum_operation
{
    enum_operation_update = 0,
    enum_operation_add,
    enum_operation_del,
};

static const char *static_dbus_service_name = "org.ukui.UniauthBackend";
static const char *static_dbus_path = "/org/ukui/UniauthBackend";
static const char *static_dbus_interface = "org.ukui.UniauthBackend";

PersonalizedData::PersonalizedData(QString user)
{
    KyInfo() << user;
    m_user = user;
}

PersonalizedData::~PersonalizedData()
{
    KyInfo() << m_user;
}

void PersonalizedData::getJsonData(QJsonObject &json)
{
    json.insert("user", m_user);
    {
        QJsonObject json_1;
        json_1["dateType"] = m_dateType;
        json_1["fontSize"] = m_fontSize;
        json_1["timeType"] = m_timeType;
        json_1["backgroundPath"] = m_backgroundPath;
        json_1["color"] = m_color;
        json["greeter"] = json_1;
    }

    /*
    {
        QJsonObject json_1;
        json_1["cursor-size"] = m_cursor_size;
        json_1["cursor-theme"] = m_cursor_theme;
        json_1["scaling-factor"] = m_scaling_factor;
        json["usd"] = json_1;
    }
    */
    // KyInfo() << json;
}

void PersonalizedData::setJson(const QJsonObject &json)
{
    if (json.contains("user") && json.value("user").toString() != m_user) {
        KyWarning() << m_user;
        return;
    }
    if (json.contains("greeter") && json.value("greeter").isObject()) {
        QJsonObject json_1 = json.value("greeter").toObject();
        if (json_1.contains("dateType"))
            m_dateType = json_1["dateType"].toString();

        if (json_1.contains("fontSize"))
            m_fontSize = json_1["fontSize"].toInt();

        if (json_1.contains("timeType"))
            m_timeType = json_1["timeType"].toInt();

        if (json_1.contains("backgroundPath"))
            m_backgroundPath = json_1["backgroundPath"].toString();

        if (json_1.contains("color"))
            m_color = json_1["color"].toString();
    }
}

/////////////////////////PersonalizedDataMng//////////////////////////////////////

PersonalizedDataMng::PersonalizedDataMng(void)
{
    //后续移动到公共接口
    this->__getUserInfomation();

    //后续移动到公共监控dbus接口
    if (QDBusConnection::systemBus().connect(
            static_dbus_service_name,
            static_dbus_path,
            static_dbus_interface,
            "updateUserInformation",
            this,
            SLOT(updateUserInformation(QString)))) {
        KyDebug() << "updateUserInformation signal connected successfully to slot";
    } else {
        KyDebug() << "updateUserInformation signal connection was not successful";
    }
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
    }

    QJsonDocument document;
    document.setObject(json);
    QString strJson(document.toJson(QJsonDocument::Compact));
    KyInfo() << strJson;
    return strJson;
}

KylinUserDatePtr PersonalizedDataMng::GetUserPtr(QString user)
{
    if (m_userPersonalizedData.contains(user)) {
        return m_userPersonalizedData[user];
    }
    return nullptr;
}

void PersonalizedDataMng::updateUserInformation(QString jsonstring)
{
    KyDebug() << jsonstring;
    QJsonParseError jsonParseError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonstring.toUtf8(), &jsonParseError);

    if (jsonParseError.error != QJsonParseError::NoError) {
        KyInfo() << "getAgreementInfo Parse json failed!!";
    } else {
        QJsonObject objJson = jsonDoc.object();
        if (objJson.isEmpty()) {
            KyInfo() << "getAgreementInfo Json is null!!";
        } else {
            if (objJson.contains("user") && objJson["user"].isString() && objJson.contains("operation")) {
                QString key = objJson["user"].toString();
                int opt = objJson["operation"].toInt();
                if (enum_operation_update == opt || enum_operation_add == opt) {
                    if (m_userPersonalizedData.contains(key)) {
                        m_userPersonalizedData[key]->setJson(objJson);
                    } else {
                        m_userPersonalizedData[key] = KylinUserDatePtr(new PersonalizedData(key));
                        m_userPersonalizedData[key]->setJson(objJson);
                    }
                } else if (enum_operation_del) {
                    m_userPersonalizedData.remove(key);
                }
            }
        }
    }
}

void PersonalizedDataMng::__getUserInfomation()
{
    QDBusInterface iface(
        static_dbus_service_name, static_dbus_path, static_dbus_interface, QDBusConnection::systemBus());
    QString param1 = "";
    QDBusPendingCall pcall = iface.asyncCall("GetUserInformation", param1);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
    this->wait_for_finish(watcher);

    QDBusPendingReply<QString> reply = pcall;
    if (reply.isError()) {
        KyWarning() << "getAgreementInfo error: " << reply.error().message();
    } else {
        KyInfo() << reply.value();
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);

        if (jsonParseError.error != QJsonParseError::NoError) {
            KyInfo() << "getAgreementInfo Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                KyInfo() << "getAgreementInfo Json is null!!";
            } else {
                for (auto &key : objRes.keys()) {
                    QJsonValue value = objRes.value(key);
                    if (value.isObject()) {
                        KyInfo() << key;
                        if (m_userPersonalizedData.contains(key)) {
                            m_userPersonalizedData[key]->setJson(value.toObject());
                        } else {
                            m_userPersonalizedData[key] = KylinUserDatePtr(new PersonalizedData(key));
                            m_userPersonalizedData[key]->setJson(value.toObject());
                        }
                    }
                }
            }
        }
    }
}

void PersonalizedDataMng::wait_for_finish(QDBusPendingCallWatcher *call)
{
    QEventLoop eventloop;
    connect(call, &QDBusPendingCallWatcher::finished, this, [&](QDBusPendingCallWatcher *) { eventloop.exit(); });
    eventloop.exec();
}
