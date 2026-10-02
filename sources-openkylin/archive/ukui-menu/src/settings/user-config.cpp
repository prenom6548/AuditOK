/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

#include "user-config.h"

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#define PRE_INSTALLED_APPS_KEY "PreInstalledApps"

namespace UkuiMenu {

const QString UserConfig::configFilePath = QDir::homePath() + "/.config/ukui-menu/";
const QString UserConfig::configFileName = "config.json";

// GCOVR_EXCL_BR_START
UserConfig *UserConfig::instance()
{
    static UserConfig userConfig;
    return &userConfig;
}
// GCOVR_EXCL_BR_STOP

UserConfig::UserConfig(QObject *parent) : QObject(parent)
{
    init();
}

bool UserConfig::isFirstStartUp() const
{
    return m_isFirstStartUp;
}

void UserConfig::init()
{
    QFile configFile(configFilePath + configFileName);
    if ((m_isFirstStartUp = !configFile.exists())) {
        if (!QDir(configFilePath).exists() && !QDir().mkdir(configFilePath)) { // GCOVR_EXCL_LINE
            qWarning() << "Could not create the user profile directory"; // GCOVR_EXCL_LINE
            return; // GCOVR_EXCL_LINE
        } // GCOVR_EXCL_LINE

        initConfigFile();
        return;
    }

    // read
    readConfigFile();
}

void UserConfig::initConfigFile()
{
    QFile configFile(configFilePath + configFileName);
    configFile.open(QFile::WriteOnly);
    configFile.close();
}

void UserConfig::sync()
{
    writeConfigFile();
}

QSet<QString> UserConfig::preInstalledApps() const
{
    return m_preInstalledApps;
}

void UserConfig::addPreInstalledApp(const QString &appid)
{
    m_preInstalledApps.insert(appid);
}

bool UserConfig::isPreInstalledApps(const QString &appid) const
{
    return m_preInstalledApps.contains(appid);
}

void UserConfig::initPreInstalledLists(QStringList apps)
{
    for (const auto &app : apps) {
        m_preInstalledApps.insert(app);
    }
    sync();
}

void UserConfig::removePreInstalledApp(const QString &appid)
{
    m_preInstalledApps.remove(appid);
    sync();
}

void UserConfig::readConfigFile()
{
    m_preInstalledApps.clear();

    QFile file(configFilePath + configFileName);
    if (!file.open(QFile::ReadOnly)) { // GCOVR_EXCL_LINE
        return; // GCOVR_EXCL_LINE
    } // GCOVR_EXCL_LINE

    QByteArray byteArray = file.readAll();
    file.close();

    QJsonDocument jsonDocument(QJsonDocument::fromJson(byteArray));
    if (jsonDocument.isNull() || jsonDocument.isEmpty() || !jsonDocument.isArray()) {
        qWarning() << "UserConfig: Incorrect configuration files are ignored.";
        return;
    }

    QJsonArray jsonArray = jsonDocument.array();
    for (const auto &value : jsonArray) {
        QJsonObject object = value.toObject();
        // 预装app
        if (object.contains(PRE_INSTALLED_APPS_KEY)) {
            QJsonArray apps = object.value(QLatin1String(PRE_INSTALLED_APPS_KEY)).toArray();
            for (const auto &app : apps) {
                m_preInstalledApps.insert(app.toString());
            }
        }
    }
}

void UserConfig::writeConfigFile()
{
    QFile file(configFilePath + configFileName);
    if (!file.open(QFile::WriteOnly)) { // GCOVR_EXCL_LINE
        return; // GCOVR_EXCL_LINE
    } // GCOVR_EXCL_LINE

    QJsonDocument jsonDocument;
    QJsonArray configArray;

    // preInstalledApps
    QJsonObject preInstalledAppsObject;

    QJsonArray apps;
    for (const auto &appid : m_preInstalledApps) {
        apps.append(appid);
    }

    preInstalledAppsObject.insert(PRE_INSTALLED_APPS_KEY, apps);
    configArray.append(preInstalledAppsObject);

    jsonDocument.setArray(configArray);

    if (file.write(jsonDocument.toJson()) == -1) { // GCOVR_EXCL_LINE
        qWarning() << "Error saving configuration file."; // GCOVR_EXCL_LINE
    } // GCOVR_EXCL_LINE
    file.flush();
    file.close();
}

} // UkuiMenu
