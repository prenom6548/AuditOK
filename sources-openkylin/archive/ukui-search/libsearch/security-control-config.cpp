/*
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 *
 * Authors: baijunjie <baijunjie@kylinos.cn>
 *
 */

#include "security-control-config.h"
#include <QVariant>
#include <QDBusReply>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>

using namespace UkuiSearch;

QMutex SecurityControlConfig::s_mutex;

static const QString APP_NAME = QStringLiteral("ukui-search");
static const QString BLACK_LIST_KEY = QStringLiteral("appBlackList");
static const QString WHITE_LIST_KEY = QStringLiteral("appWhiteList");

SecurityControlConfig &SecurityControlConfig::instance()
{
    static SecurityControlConfig instance;
    return instance;
}

bool SecurityControlConfig::isUnderSecurityControl() const
{
    QMutexLocker locker(&s_mutex);
    return !m_dataObject.isEmpty();
}

bool SecurityControlConfig::isBlackListEnable() const
{
    QMutexLocker locker(&s_mutex);
    return !m_dataObject.value(BLACK_LIST_KEY).toVariant().toStringList().isEmpty();
}

bool SecurityControlConfig::isWhiteListEnable() const
{
    QMutexLocker locker(&s_mutex);
    return !m_dataObject.value(WHITE_LIST_KEY).toVariant().toStringList().isEmpty();
}

QStringList SecurityControlConfig::appBlackList() const
{
    QMutexLocker locker(&s_mutex);
    return m_dataObject.value(BLACK_LIST_KEY).toVariant().toStringList();
}

QStringList SecurityControlConfig::appWhiteList() const
{
    QMutexLocker locker(&s_mutex);
    return m_dataObject.value(WHITE_LIST_KEY).toVariant().toStringList();
}

void SecurityControlConfig::onConfigFileChanged(QString userName,  QStringList appConfigNames)
{
    if (!appConfigNames.contains(APP_NAME)) {
        qWarning() << "The conf file is not changed";
        return;
    }
    if (userName != getenv("USER")) {
        qDebug() << "User name error,current user is" << userName;
        return;
    }
    if (m_configPath.isEmpty()) {
        getConfigPath();
    }
    parseData(m_configPath);
}

void SecurityControlConfig::onSecurityControlExited()
{
    QMutexLocker locker(&s_mutex);
    m_dataObject = QJsonObject();
}

SecurityControlConfig::SecurityControlConfig(QObject* parent) : QObject(parent)
{
    m_interface = new QDBusInterface(QStringLiteral("com.kylin.ukui.SettingsDaemon"),
                                     QStringLiteral("/securityConfig"),
                                     QStringLiteral("com.kylin.ukui.SettingsDaemon.interface"),
                                     QDBusConnection::systemBus(), parent);
    if (!m_interface->isValid()) {
        qWarning() << "Init security control interface error" << m_interface->lastError();
    } else {
        getConfigPath();
        parseData(m_configPath);

        connect(m_interface, SIGNAL(ConfigChanged(QString, QStringList)), this, SLOT(onConfigFileChanged(QString, QStringList)));
        connect(m_interface, SIGNAL(SecurityExit()), this, SLOT(onSecurityControlExited()));
    }
}

void SecurityControlConfig::parseData(const QString& path)
{
    QFile settingsFile(path);
    if (!settingsFile.exists()) {
        qWarning() << "security config file is not exists";
        return;
    }

    if (!settingsFile.open(QFile::ReadOnly)) {
        qWarning() << "SettingsManagerPrivate: configuration file " << settingsFile.fileName() << "open failed !";
        return;
    }

    QByteArray byteArray = settingsFile.readAll();
    settingsFile.close();
    QJsonParseError errRpt;
    QJsonDocument jsonDocument(QJsonDocument::fromJson(byteArray, &errRpt));
    if (errRpt.error != QJsonParseError::NoError) {
        qWarning() << "Incorrect security configuration file. JSON parse error";
        return;
    }
    QMutexLocker locker(&s_mutex);
    m_dataObject = jsonDocument.object().value(APP_NAME).toObject();
}

void SecurityControlConfig::getConfigPath() {
    QDBusReply<QString> reply = m_interface->call("GetConfigPath", const_cast<const char*>(getenv("USER")), APP_NAME);
    if (!reply.isValid()) {
        qWarning() << "get security config path error" << reply.error();
    } else {
        m_configPath = reply.value();
    }
}

// UkuiSearch