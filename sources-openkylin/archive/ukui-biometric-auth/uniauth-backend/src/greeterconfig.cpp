/*
 * Copyright (C) 2024 Tianjin KYLIN Information Technology Co., Ltd.
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

#include "greeterconfig.h"
#include <QDir>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDebug>
#include <pwd.h>
#include "userconfigsettings.h"

#define GREETER_CONFIG_DEFAULT_FILE "/var/cache/ukui-greeter/default.conf"
#define GREETER_CONFIG_CUSTOM_FILE "/var/cache/ukui-greeter/custom.conf"
#define GREETER_CONFIG_ETC_DIR "/etc/ukui/ukui-greeter/ukui-greeter.conf.d/"
#define GREETER_CONFIG_ETC_FILE "/etc/ukui/ukui-greeter/ukui-greeter.conf"
#define GREETER_CONFIG_USER_FILE "%1/.config/ukui-greeter/ukui-greeter.conf"

GreeterConfig::GreeterConfig(QObject *parent) : QObject(parent), m_setttingsDefault(nullptr), m_settingsCustom(nullptr)
{
    initData();
}

void GreeterConfig::initData()
{
    m_strHomePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if (!m_setttingsDefault) {
        m_setttingsDefault = new QSettings(GREETER_CONFIG_DEFAULT_FILE, QSettings::IniFormat, this);
        m_setttingsDefault->clear();
    }
    // read config from conf.d directory
    loadConfigDirectory(GREETER_CONFIG_ETC_DIR);
    // read config from conf file
    loadConfigFromFile(GREETER_CONFIG_ETC_FILE);

    if (!m_settingsCustom) {
        m_settingsCustom = new QSettings(GREETER_CONFIG_CUSTOM_FILE, QSettings::IniFormat, this);
    }
}

void GreeterConfig::loadConfigDirectory(QString strPath)
{
    QDir confDir(strPath);

    QStringList filePathList = confDir.entryList(QDir::Files, QDir::Name);
    for (const QString &filePath : filePathList) {
        if (filePath.endsWith(".conf")) {
            loadConfigFromFile(strPath + filePath);
        }
    }
    if (m_setttingsDefault) {
        m_setttingsDefault->sync();
    }
}

void GreeterConfig::loadConfigFromFile(QString strPath)
{
    if (!m_setttingsDefault) {
        return;
    }
    qDebug() << "conf file:" << strPath;
    QSettings *settingsConf = new QSettings(strPath, QSettings::IniFormat, this);

    QStringList groups = settingsConf->childGroups();
    foreach (const QString &group, groups) {
        settingsConf->beginGroup(group);
        QStringList keys = settingsConf->childKeys();
        foreach (const QString &key, keys) {
            m_setttingsDefault->beginGroup(group);
            m_setttingsDefault->setValue(key, settingsConf->value(key));
            m_setttingsDefault->endGroup();
        }
        settingsConf->endGroup();
    }

    settingsConf->deleteLater();
    settingsConf = nullptr;
}

void GreeterConfig::setValue(
    const QString &group, const QString &key, const QVariant &value, QString strUserName /* = ""*/)
{
    if ((strUserName.isEmpty() && !m_settingsCustom) || group.isEmpty() || key.isEmpty()) {
        return;
    }
    if (!strUserName.isEmpty()) {
        QString strHomePath = getUserHomePath(strUserName);
        if (strHomePath.isEmpty()) {
            qDebug() << "GetUserHomePath failed!";
            return;
        }
        UserConfigSettings userSettings(
            QString(GREETER_CONFIG_USER_FILE).arg(strHomePath), QSettings::IniFormat, strUserName);
        userSettings.beginGroup(group);
        userSettings.setValue(key, value);
        userSettings.endGroup();
        userSettings.sync();
        Q_EMIT changedForUser(group, key, strUserName);
    } else if (m_settingsCustom) {
        m_settingsCustom->beginGroup(group);
        m_settingsCustom->setValue(key, value);
        m_settingsCustom->endGroup();
        m_settingsCustom->sync();
        Q_EMIT changed(group, key);
    }
}

QVariant
GreeterConfig::getValue(const QString &group, const QString &key, QVariant varDefault, QString strUserName /*= ""*/)
{
    QVariant varValue;
    if (!strUserName.isEmpty()) {
        QString strHomePath = getUserHomePath(strUserName);
        if (!strHomePath.isEmpty()) {
            UserConfigSettings userSettings(
                QString(GREETER_CONFIG_USER_FILE).arg(strHomePath), QSettings::IniFormat, strUserName);
            if (userSettings.contains(group + "/" + key)) {
                userSettings.beginGroup(group);
                varValue = userSettings.value(key);
                userSettings.endGroup();
                return varValue;
            }
        }
    }
    if (m_settingsCustom) {
        if (m_settingsCustom->contains(group + "/" + key)) {
            m_settingsCustom->beginGroup(group);
            varValue = m_settingsCustom->value(key);
            m_settingsCustom->endGroup();
            return varValue;
        }
    }
    if (m_setttingsDefault) {
        if (m_setttingsDefault->contains(group + "/" + key)) {
            m_setttingsDefault->beginGroup(group);
            varValue = m_setttingsDefault->value(key);
            m_setttingsDefault->endGroup();
            return varValue;
        }
    }
    return varDefault;
}

bool GreeterConfig::resetValue(const QString &group, const QString &key, QString strUserName /*= ""*/)
{
    bool bRet = false;
    if (!strUserName.isEmpty()) {
        QString strHomePath = getUserHomePath(strUserName);
        if (!strHomePath.isEmpty()) {
            UserConfigSettings userSettings(
                QString(GREETER_CONFIG_USER_FILE).arg(strHomePath), QSettings::IniFormat, strUserName);
            if (userSettings.contains(group + "/" + key)) {
                userSettings.beginGroup(group);
                userSettings.remove(key);
                userSettings.endGroup();
                Q_EMIT changedForUser(group, key, strUserName);
                bRet = true;
            }
        }
    }
    if (m_settingsCustom) {
        if (m_settingsCustom->contains(group + "/" + key)) {
            m_settingsCustom->beginGroup(group);
            m_settingsCustom->remove(key);
            m_settingsCustom->endGroup();
            Q_EMIT changed(group, key);
            bRet = true;
        }
    }
    return bRet;
}

QString GreeterConfig::printConfig(int nType, QString strUserName /*= ""*/)
{
    // 创建一个空的QJsonObject
    QJsonObject jsonObject;
    switch (nType) {
        case 2: {
            if (m_settingsCustom) {
                QStringList keys = m_settingsCustom->allKeys();
                for (const QString &key : keys) {
                    jsonObject[key] = m_settingsCustom->value(key).toString();
                }
            }
            if (!strUserName.isEmpty()) {
                QString strHomePath = getUserHomePath(strUserName);
                if (!strHomePath.isEmpty()) {
                    UserConfigSettings userSettings(
                        QString(GREETER_CONFIG_USER_FILE).arg(strHomePath), QSettings::IniFormat, strUserName);
                    QStringList keys = userSettings.allKeys();
                    for (const QString &key : keys) {
                        jsonObject[key] = userSettings.value(key).toString();
                    }
                }
            }
        } break;
        case 1: {
            if (m_setttingsDefault) {
                QStringList keys = m_setttingsDefault->allKeys();
                for (const QString &key : keys) {
                    jsonObject[key] = m_setttingsDefault->value(key).toString();
                }
            }
        } break;
        case 0:
        default: {
            if (m_setttingsDefault) {
                QStringList keys = m_setttingsDefault->allKeys();
                for (const QString &key : keys) {
                    jsonObject[key] = m_setttingsDefault->value(key).toString();
                }
            }
            if (m_settingsCustom) {
                QStringList keys = m_settingsCustom->allKeys();
                for (const QString &key : keys) {
                    jsonObject[key] = m_settingsCustom->value(key).toString();
                }
            }
            if (!strUserName.isEmpty()) {
                QString strHomePath = getUserHomePath(strUserName);
                if (!strHomePath.isEmpty()) {
                    UserConfigSettings userSettings(
                        QString(GREETER_CONFIG_USER_FILE).arg(strHomePath), QSettings::IniFormat, strUserName);
                    QStringList keys = userSettings.allKeys();
                    for (const QString &key : keys) {
                        jsonObject[key] = userSettings.value(key).toString();
                    }
                }
            }
        } break;
    }
    // 创建QJsonDocument
    QJsonDocument jsonDoc(jsonObject);
    return jsonDoc.toJson();
}

QString GreeterConfig::getUserHomePath(const QString &strUserName)
{
    if (strUserName.isEmpty()) {
        return QString("");
    }
    struct passwd *pwdInfo = getpwnam(strUserName.toLatin1().data());
    if (pwdInfo) {
        return QString(pwdInfo->pw_dir);
    }
    return QString("");
}
