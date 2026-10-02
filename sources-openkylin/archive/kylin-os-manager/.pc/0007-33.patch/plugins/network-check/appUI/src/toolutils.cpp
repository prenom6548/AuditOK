/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QFile>
#include <QList>
#include <QDebug>
#include <QVector>
#include <QStandardPaths>
#include <QDir>

#include "toolutils.h"

// static constexpr char webipConfigPath[] = "/etc/kylin-netcheck-tools/kylin-netcheck-tools.conf";

ToolUtils::ToolUtils() = default;

ToolUtils::~ToolUtils() = default;

//读取配置文件
QMap<QString, QVector<QString>> ToolUtils::getConfigFile()
{
    QMap<QString, QVector<QString>> ret;
    ret.clear();

    QFile file(getConfigPath());
    if (!file.exists()) {
        qCritical() << "config file is not exists !";
        return ret;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "get config file fail !";
        return ret;
    }
    QByteArray ba = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument jsonDocment = QJsonDocument::fromJson(ba, &err);
    if (err.error != QJsonParseError::NoError) {
        qCritical() << "get config file json err!" << getConfigPath();
        return ret;
    }
    if (jsonDocment.isNull() || jsonDocment.isEmpty()) {
        qCritical() << "get config file json null!" << getConfigPath();
        return ret;
    }
    QJsonObject jsonObj = jsonDocment.object();
    if (jsonObj.isEmpty() || jsonObj.size() == 0) {
        qCritical() << "get config file jsonobj null!";
        return ret;
    }

    if (jsonObj.contains("config-ip")) {
        QVector<QString> ipVector;
        ipVector.clear();
        QJsonValue arrayVaule = jsonObj.value("config-ip");
        if (arrayVaule.isArray()) {
            QJsonArray array = arrayVaule.toArray();
            for (int i = 0; i < array.size(); i++) {
                QJsonValue value = array.at(i);
                QString ip = value.toString();
                qDebug() << "Utils::getConfigFile ip:" << ip;
                ipVector.append(ip);
            }
        }
        ret.insert("config-ip", ipVector);
    } else {
        qCritical() << "config file don't have config-ip!";
    }
    if (jsonObj.contains("config-web")) {
        QVector<QString> webVector;
        webVector.clear();
        QJsonValue arrayVaule = jsonObj.value("config-web");
        if (arrayVaule.isArray()) {
            QJsonArray array = arrayVaule.toArray();
            for (int i = 0; i < array.size(); i++) {
                QJsonValue value = array.at(i);
                QString ip = value.toString();
                qDebug() << "Utils::getConfigFile web:" << ip;
                webVector.append(ip);
            }
        }
        ret.insert("config-web", webVector);
    } else {
        qCritical() << "config file don't have config-web!";
    }
    return ret;
}
//写入配置文件
bool ToolUtils::writeConfigFile(bool isOn, QMap<QString, QVector<QString>> configMap)
{

    QFile file(getConfigPath());
    //如果文件不存在，会自动创建
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "get config file fail !";
        return false;
    }

    file.resize(0);
    //=======================object部分的操作===============================
    QJsonObject usualObj;
    usualObj.insert("switch", isOn);

    QJsonArray ipArray;
    if (configMap.contains("config-ip")) {
        QVector<QString> ipVector = configMap.value("config-ip");
        if (!ipVector.isEmpty()) {
            QVector<QString>::iterator iter;
            for (iter = ipVector.begin(); iter != ipVector.end(); iter++) {
                QJsonValue value = *iter;
                ipArray.append(value);
            }
        }
    }

    QJsonArray webArray;
    if (configMap.contains("config-web")) {
        QVector<QString> webVector = configMap.value("config-web");
        if (!webVector.isEmpty()) {
            QVector<QString>::iterator iter;
            for (iter = webVector.begin(); iter != webVector.end(); iter++) {
                QJsonValue value = *iter;
                webArray.append(value);
            }
        }
    }

    QJsonObject rootObject;
    rootObject.insert("config-usual", usualObj);
    if (!ipArray.isEmpty()) {
        rootObject.insert("config-ip", ipArray);
    }
    if (!webArray.isEmpty()) {
        rootObject.insert("config-web", webArray);
    }

    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObject);
    file.write(jsonDoc.toJson());
    file.close();
    return true;
}
//读取开关
bool ToolUtils::getInnerCheckSetting()
{
    bool ret = false;

    QFile file(getConfigPath());
    if (!file.exists()) {
        qCritical() << "config file is not exists !";
        return ret;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "get config file fail !";
        return ret;
    }
    QByteArray ba = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument jsonDocment = QJsonDocument::fromJson(ba, &err);
    if (err.error != QJsonParseError::NoError) {
        qCritical() << "get config file json err!";
        return ret;
    }
    if (jsonDocment.isNull() || jsonDocment.isEmpty()) {
        qCritical() << "get config file json null!";
        return ret;
    }
    QJsonObject jsonObj = jsonDocment.object();
    if (jsonObj.isEmpty() || jsonObj.size() == 0) {
        qCritical() << "get config file jsonobj null!";
        return ret;
    }
    if (jsonObj.contains("config-usual")) {
        QJsonObject usualObj = jsonObj.value("config-usual").toObject();
        if (!usualObj.isEmpty() || usualObj.size() > 0) {
            if (usualObj.contains("switch")) {
                ret = usualObj.value("switch").toBool(false);
            }
        }
    } else {
        qCritical() << "config file don't have config-usual!";
    }
    return ret;
}

QString ToolUtils::getConfigPath()
{
    QString configPath = QString("%1/.config/kylin-os-manager/net-check").arg(QDir::homePath());

    /* dir exists ? */
    QDir dir(configPath);
    if (!dir.exists()) {
        if (!dir.mkpath(configPath)) {
            qCritical() << "create network check config path fail !";
            return "";
        }
    }

    /* file exists ? */
    QString configFile = configPath + QString("/") + QString("kylin-netcheck-tools.conf");
    QFileInfo fileInfo(configFile);
    if (!fileInfo.exists()) {
        if (!QFile::copy("/etc/kylin-os-manager/net-check/kylin-netcheck-tools.conf", configFile)) {
            qCritical() << "copy network check config file fail !";
            return "";
        }
    }

    return configFile;
}

bool ToolUtils::isIP(QString ip)
{
    QRegExp rxp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    if (!rxp.exactMatch(ip)) {
        return false;
    }
    return true;
}
bool ToolUtils::isWeb(QString web)
{
    if (web.contains(" ") || web.contains(";")) {
        return false;
    } else {
        return true;
    }
}
