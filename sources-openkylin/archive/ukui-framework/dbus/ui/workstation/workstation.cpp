/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
* -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "workstation.h"
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <qlockfile.h>

static const QByteArray WORKSTATION_GSETTINGS_ID = "org.kylin.workstation.settings";
static const QString DEFAULT_APPS_KEY = "default-apps";
static const QString CONFIG_PATH = QDir::homePath() + "/.config/kylin-workstation/default-apps.json";
static const QString DEFAULT_APPS_GROUP = "DefaultApps";

using namespace UKUI;

ConfigWorkstation::ConfigWorkstation(QObject* parent)
{
    if (QGSettings::isSchemaInstalled(WORKSTATION_GSETTINGS_ID)) {
        m_gSettings = new QGSettings(WORKSTATION_GSETTINGS_ID, QByteArray(), this);
        if (!getConfigApps()) {
            qWarning() << "Fail to get default apps of the config file.";
        }
    }
}

ConfigWorkstation::~ConfigWorkstation()
{
    if (m_gSettings) {
        delete m_gSettings;
        m_gSettings = nullptr;
    }
}

QStringList ConfigWorkstation::getDefaultApps()
{
    if (!getConfigApps()) {
        qWarning() << "Fail to get default apps of the config file.";
    }
    return m_defaultApps;
}

bool ConfigWorkstation::setDefaultApps(const QStringList& defaultApps) const
{
    bool res(false);

    if (defaultApps.isEmpty()) {
        qWarning() << "There is no apps need to set as default";
    }

    if (defaultApps == m_defaultApps) {
        qWarning() << "Default apps has not been changed.";
        return res;
    }

    if (m_gSettings && m_gSettings->keys().contains(DEFAULT_APPS_KEY)) {
        m_gSettings->set(DEFAULT_APPS_KEY, defaultApps);
        res = true;
    }
    return res;
}

bool ConfigWorkstation::getConfigApps()
{
    if (m_gSettings && m_gSettings->keys().contains(DEFAULT_APPS_KEY)) {
        m_defaultApps = m_gSettings->get(DEFAULT_APPS_KEY).toStringList();
        return true;
    }

    QFile configFile(CONFIG_PATH);

    if (!configFile.exists()) {
        return false;
    }

    QLockFile lockFile(CONFIG_PATH + ".lock");
    if (!lockFile.lock()) {
        qWarning() << "Fail to lock" << CONFIG_PATH << lockFile.error();
    }

    if (!configFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Fail to open file " << CONFIG_PATH;
        lockFile.unlock();
        return false;
    }

    QByteArray byteArray = configFile.readAll();
    configFile.close();
    lockFile.unlock();

    QJsonParseError errRpt{};
    QJsonDocument jsonDocument(QJsonDocument::fromJson(byteArray, &errRpt));
    if (errRpt.error != QJsonParseError::NoError) {
        qWarning() << "Incorrect configuration files. JSON parse error";
        return false;
    }

    // 读取配置文件
    QJsonObject settingsData = jsonDocument.object();
    m_defaultApps = settingsData.value(DEFAULT_APPS_GROUP).toVariant().toStringList();
    return true;
}
