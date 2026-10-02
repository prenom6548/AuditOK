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

#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QDBusInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtDBus>
#include <QFile>

Preferences::Preferences()
    : mMinScrollSensitivity(0)
    , mMaxScrollSensitivity(100)
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
    //save();

    if (m_settings) {
        delete m_settings;
        m_settings = nullptr;
    }
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

void Preferences::reset()
{
    m_cameraDeviceName = "";
    m_KmreAutoStart = true;
    m_PhoneInfoCustom = false;
    PhonepresetbrandIndex = 0;
    PhonepresetmodelIndex = 0;
    m_defaultdns = true;
    m_dns = "";
    mScrollSensitivity = DEFAULT_SCROLL_SENSITIVITY;
}

void Preferences::save()
{
    QSettings *set = m_settings;
    if (set) {
        set->beginGroup("camera");
        set->setValue("device", m_cameraDeviceName);
        set->endGroup();
        set->sync();
        
        set->beginGroup("settings");
        set->setValue("kmre_auto_start", m_KmreAutoStart);
        set->endGroup();
        set->sync();

    }
}

void Preferences::load()
{
    QSettings *set = m_settings;
    if (set) {
        set->beginGroup("camera");
        m_cameraDeviceName = set->value("device", m_cameraDeviceName).toString();
        set->endGroup();
        
        set->beginGroup("settings");
        m_KmreAutoStart = set->value("kmre_auto_start", m_KmreAutoStart).toBool();
        mScrollSensitivity = set->value("scroll_sensitivity", DEFAULT_SCROLL_SENSITIVITY).toInt();
        set->endGroup();

        set->beginGroup("phoneinfo");
        m_PhoneInfoCustom = set->value("info_custom", m_PhoneInfoCustom).toBool();
        PhoneVendor = set->value("vendor", PhoneVendor).toString();
        PhoneBrand = set->value("brand", PhoneBrand).toString();
        PhoneName = set->value("name", PhoneName).toString();
        PhoneModel = set->value("model", PhoneModel).toString();
        PhoneEquip = set->value("equip", PhoneEquip).toString();
        PhoneSerialno = set->value("serialno", PhoneSerialno).toString();
        PhoneBoard = set->value("board", PhoneBoard).toString();
        PhoneIMEI = set->value("imei", PhoneIMEI).toString();     
        PhonepresetbrandIndex = set->value("preset_brand_index", PhonepresetbrandIndex).toInt();
        PhonepresetmodelIndex = set->value("preset_model_index", PhonepresetmodelIndex).toInt();
        set->endGroup();

        set->beginGroup("network");
        m_defaultdns = set->value("defaultdns", m_defaultdns).toBool();
        m_dns = set->value("dns", m_dns).toString();
        DockerNetworkMode = set->value("mode", "bridge").toString();
        DockerNetworkDevice = set->value("device", "").toString();
        set->endGroup();
    }
}

void Preferences::updateCameraConfig()
{
    QSettings *set = m_settings;
    if (set) {
        set->beginGroup("camera");
        set->setValue("device", m_cameraDeviceName);
        set->endGroup();
        set->sync();
    }
}

void Preferences::updateKmreAutoStartConfig()
{
    QSettings *set = m_settings;
    if (set) {
        set->beginGroup("settings");
        set->setValue("kmre_auto_start", m_KmreAutoStart);
        set->endGroup();
        set->sync();
    }
}

void Preferences::updatePhoneInfoConfig()
{
    QSettings *set = m_settings;
    if (set) {
        if (!PhoneName.isEmpty()) {
            set->beginGroup("phoneinfo");
            set->setValue("info_custom", m_PhoneInfoCustom);
            set->setValue("vendor", PhoneVendor);
            set->setValue("brand", PhoneBrand);
            set->setValue("name", PhoneName);
            set->setValue("model", PhoneModel);
            set->setValue("equip", PhoneEquip);
            set->setValue("serialno", PhoneSerialno);
            set->setValue("board", PhoneBoard);
            set->setValue("preset_model_index",PhonepresetmodelIndex);
            set->endGroup();
            set->sync();
        }
        else {
            set->beginGroup("phoneinfo");
            set->setValue("info_custom", m_PhoneInfoCustom);
            set->setValue("brand", PhoneBrand);
            set->setValue("model", PhoneModel);
            set->remove("vendor");
            set->remove("name");
            set->remove("equip");
            set->remove("serialno");
            set->remove("board");
            set->remove("preset_index");
            set->endGroup();
            set->sync();
        }
    }
}

void Preferences::updatecustomConfig()
{
    QSettings *set = m_settings;
    if (set) {
        set->beginGroup("phoneinfo");
        set->setValue("info_custom",m_PhoneInfoCustom);
        set->endGroup();
        set->sync();
    }
}

void Preferences::updatePhoneBrandIndex()
{
    QSettings *set = m_settings;
    if (set) {
        set->beginGroup("phoneinfo");
        set->setValue("preset_brand_index",PhonepresetbrandIndex);
        set->endGroup();
        set->sync();
    }
}

void Preferences::updatePhoneImei()
{
    QSettings *set = m_settings;
    if (set) {
        set->beginGroup("phoneinfo");
        set->setValue("imei",PhoneIMEI);
        set->endGroup();
        set->sync();
    }
}

void Preferences::updateDNS()
{
    QSettings *set = m_settings;
    if(set) {
        if (!m_defaultdns) {
            set->beginGroup("network");
            set->setValue("defaultdns",m_defaultdns);
            set->setValue("dns",m_dns);
            set->endGroup();
            set->sync();
        }
        else {
            set->beginGroup("network");
            set->setValue("defaultdns",m_defaultdns);
            set->remove("dns");
            set->endGroup();
            set->sync();
        }
    }
}

void Preferences::updateDockerNetwork(const QString &mode, const QString &device)
{
    DockerNetworkMode = mode;
    DockerNetworkDevice = device;
    
    if(m_settings) {
        m_settings->beginGroup("network");
        m_settings->setValue("mode",DockerNetworkMode);
        m_settings->setValue("device",DockerNetworkDevice);
        m_settings->endGroup();
        m_settings->sync();
    }
}

void Preferences::updateScrollSensitivity(int sensitivity)
{
    mScrollSensitivity = sensitivity;

    QSettings *set = m_settings;
    if (set) {
        set->beginGroup("settings");
        set->setValue("scroll_sensitivity", mScrollSensitivity);
        set->endGroup();
        set->sync();
    }
}
