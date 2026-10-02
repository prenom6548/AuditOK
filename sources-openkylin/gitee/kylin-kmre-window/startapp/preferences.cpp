/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "preferences.h"
#include "dbus_client.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDBusInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtDBus>
#include <QFile>
#include <QDir>
Preferences::Preferences()
{
    const QString &configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config";
    if (!QDir(configPath).exists("kmre")) {
        QDir(configPath).mkdir("kmre");
    }

    QString confName = QDir::homePath() + "/.config/kmre/kmre.ini";
    m_settings = new QSettings(confName, QSettings::IniFormat);
    m_settings->setIniCodec("UTF-8");

    initInfo();
    reset();
    load();
}

Preferences::~Preferences()
{
    save();

    if (m_settings) {
        delete m_settings;
        m_settings = nullptr;
    }
}

void Preferences::reset()
{
    if ((displayType == "unknown") || (displayType == "emugl")) {
        maxWindowNum = 5;
    }
    if (displayType == "drm") {
        maxWindowNum = 10;
    }
}

void Preferences::save()
{
    QSettings *set = m_settings;
//    if (set) {
//        set->beginGroup("settings");
//        set->setValue("max_window_number", maxWindowNum);
//        set->endGroup();
//        set->sync();
//    }
}

void Preferences::load()
{
    QSettings *set = m_settings;
    if (set) {
        set->beginGroup("settings");
        maxWindowNum = set->value("max_window_number", maxWindowNum).toInt();
        m_KmreAutoStart = set->value("kmre_auto_start", m_KmreAutoStart).toBool();
        m_AppNumLimit = set->value("appnum_limit", m_AppNumLimit).toBool();
        set->endGroup();
    }
}

void Preferences::getmaxWindowNum()
{
    QSettings *set = m_settings;
    if (set) {
        set->beginGroup("settings");
        maxWindowNum = set->value("max_window_number", maxWindowNum).toInt();
        set->endGroup();
    }
}

bool Preferences::getAppNumLimitConfig()
{
    QSettings *set = m_settings;
    if (set) {
        set->beginGroup("settings");
        m_AppNumLimit = set->value("appnum_limit", m_AppNumLimit).toBool();
        set->endGroup();
    }

    return m_AppNumLimit;
}

void Preferences::initInfo()
{
    QDBusInterface interface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus());
    QString value;
    QDBusMessage response = interface.call("getDisplayInformation");
    if (response.type() == QDBusMessage::ReplyMessage) {
        value = response.arguments().takeFirst().toString();
    }
    QString displayInfo = value;
    if (!displayInfo.isEmpty()) {
        QJsonDocument jsonDocument = QJsonDocument::fromJson(displayInfo.toLocal8Bit().data());
        if (!jsonDocument.isNull()) {
            QJsonObject jsonObject = jsonDocument.object();
            if (!jsonObject.isEmpty() && jsonObject.size() > 0) {
                if (jsonObject.contains("display_type")) {
                    displayType = jsonObject.value("display_type").toString();
                }
                if (jsonObject.contains("cpu_type")) {
                    cpuType = jsonObject.value("cpu_type").toString();
                }
                if (jsonObject.contains("gpu_vendor")) {
                    gpuVendor = jsonObject.value("gpu_vendor").toString();
                }
                if (jsonObject.contains("gpu_model")) {
                    gpuModel = jsonObject.value("gpu_model").toString();
                }
                if (displayType.isNull() || displayType.isEmpty()) {
                    displayType = "unknown";
                }
                if (gpuVendor.isNull() || gpuVendor.isEmpty()) {
                    gpuVendor = "unknown";
                }
            }
        }
    }
}

