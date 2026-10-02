/*
 * Copyright (C) 2024 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QtDBus/QtDBus>

#include "configmonitor.h"

static const char* usdServiceName = "com.kylin.ukui.SettingsDaemon";
static const char* usdSecurityConfigPath = "/securityConfig";
static const char* usdServiceInterface = "com.kylin.ukui.SettingsDaemon.interface";

static QJsonObject readJsonFile(const QString& filePath)
{
    if (filePath.isEmpty() || !QFile(filePath).exists()) {
        qWarning() << "The file path: " << filePath << " may be empty or the file does not exist!";

        return QJsonObject();
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Can not open the file at the path: " << filePath << " to read its contents."
            << "The error string is: " << file.errorString();

        return QJsonObject();
    }

    QByteArray json = file.readAll();
    file.close();

    qInfo() << "Reading the file at the file path: " << filePath << " finished."
        << "The json string read from the file: " << json;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    if (doc.isEmpty() || error.error != QJsonParseError::NoError) {
        qWarning() << "The json string is empty or there is an error."
            << " json = " << json;

        return QJsonObject();
    }

    qInfo() << "Creating the json object from the json string from the file: " << filePath << " finished."
        << "The json string from the document object: " << doc.toJson();

    return doc.object();
}

ConfigMonitor::ConfigMonitor(const QString& configFileBaseName)
    : QObject(),
      m_configFileBaseName(configFileBaseName)
{
    loadModuleConfig();
    connectUsdDBusSignal();
}

QString ConfigMonitor::getUserName() {
    return QString::fromLocal8Bit(qgetenv("USER"));
}

bool ConfigMonitor::shouldUpdateModuleConfig(const QString& user, const QStringList& fileBaseNameList) const {
    if (fileBaseNameList.isEmpty()) {
        return false;
    }

    // 用户名为空，则表示所有用户都应该响应更新
    if (user.isEmpty()) {
        return true;
    }

    // 用户名不为空，则只有是当前用户的时候，才应该更新
    return user == getUserName();
}

void ConfigMonitor::onConfigFilesChanged(const QString& user, const QStringList& fileBaseNameList) {
    qInfo() << "The signal: ConfigChanged is received. user = " << user << " fileBaseNameList" << fileBaseNameList;

    if (!shouldUpdateModuleConfig(user, fileBaseNameList)) {
        return;
    }

    if (fileBaseNameList.contains(m_configFileBaseName)) {
        updateModuleConfig();
    }
}

void ConfigMonitor::clearModuleConfig() {
    qInfo() << "The signal: SecurityExit is received.";

    m_moduleConfigObject = QJsonObject();
    Q_EMIT moduleConfigChanged();
}

void ConfigMonitor::connectUsdDBusSignal()
{
    QDBusConnection::systemBus().connect(usdServiceName,
                                         usdSecurityConfigPath,
                                         usdServiceInterface,
                                         "ConfigChanged",
                                         this,
                                         SLOT(onConfigFilesChanged(const QString&, const QStringList&)));

    QDBusConnection::systemBus().connect(usdServiceName,
                                         usdSecurityConfigPath,
                                         usdServiceInterface,
                                         "SecurityExit",
                                         this,
                                         SLOT(clearModuleConfig()));
}

bool ConfigMonitor::getModuleControlledByName(const QString& moduleName) const {
    return m_moduleConfigObject.contains(moduleName);
}

QJsonValue ConfigMonitor::getModuleConfigByName(const QString& moduleName) const {
    if (!m_moduleConfigObject.contains(moduleName)) {
        return QJsonValue();
    }

    return m_moduleConfigObject.value(moduleName);
}

QString ConfigMonitor::getModuleJsonFilePath() const
{
    QDBusInterface securityConfig(usdServiceName,
                                  usdSecurityConfigPath,
                                  usdServiceInterface,
                                  QDBusConnection::systemBus());

    QDBusReply<QString> reply = securityConfig.call("GetConfigPath", getUserName(), m_configFileBaseName);
    if (!reply.isValid()) {
        return "";
    }
    return reply.value();
}

void ConfigMonitor::loadModuleConfig()
{
    QJsonObject jsonObject = readJsonFile(getModuleJsonFilePath());
    if (jsonObject.isEmpty() || jsonObject.keys().isEmpty()) {
        qWarning() << "read config json file path error";
        m_moduleConfigObject = QJsonObject();
        return;
    }

    m_moduleConfigObject = jsonObject.value(jsonObject.keys().constFirst()).toObject();

    qInfo() << "Loading config for " << m_configFileBaseName << " finished."
            << "json: " << QJsonDocument(m_moduleConfigObject).toJson();
}

void ConfigMonitor::updateModuleConfig()
{
    loadModuleConfig();

    Q_EMIT moduleConfigChanged();
}
