/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "globalconfig.h"

GlobalConfig::GlobalConfig(QObject *parent):
    QObject(parent)
{
    initFlghtMode();
}

GlobalConfig::~GlobalConfig()
{

}

bool GlobalConfig::writeGlobalConfig(QString path, QString fileName, QByteArray info)
{
    QFile globalFile;
    QString configPath = QString("%1/%2").arg(GLOBAL_CONFIG_PATH).arg(path);
    QString filePath = QString("%1/%2/%3").arg(GLOBAL_CONFIG_PATH).arg(path).arg(fileName);
    if (filePath.contains("../")) {
        SYS_LOG(LOG_DEBUG,"%s is error path", path.toLatin1().data());
        return false;
    }

    ensureGlobalDirectory();
    ensureDirectoryExists(configPath);

    globalFile.setFileName(filePath);
    if (!globalFile.open(QIODevice::WriteOnly)) {
        SYS_LOG(LOG_WARNING,"%s open file cuz:",filePath.toLatin1().data(), globalFile.errorString().toLatin1().data());
        return false;
    }

    globalFile.setPermissions(QFileDevice::Permission(0x6444));
    globalFile.write(info);
    return true;
}

QByteArray GlobalConfig::readGlobalConfig(QString path)
{
    QFile globalFile;
    QString filePath = QString("%1/%2").arg(GLOBAL_CONFIG_PATH).arg(path);
    if (filePath.contains("../")) {
        SYS_LOG(LOG_DEBUG,"%s is error path", path.toLatin1().data());
        return QByteArray();
    }
    globalFile.setFileName(filePath);

    if (!QFile::exists(filePath)) {
        SYS_LOG(LOG_DEBUG,"read %s error",filePath.toLatin1().data());
        return QByteArray();
    }

    if (!globalFile.open(QIODevice::ReadOnly)) {
        SYS_LOG(LOG_DEBUG,"open %s error",filePath.toLatin1().data());
        return QByteArray();
    }

    return globalFile.readAll();
}

bool GlobalConfig::globalConfigExists(QString configPath)
{
    QString filePath = QString("%1/%2").arg(GLOBAL_CONFIG_PATH).arg(configPath);
    if (filePath.contains("../")) {
        SYS_LOG(LOG_DEBUG,"%s is error path", filePath.toLatin1().data());
        return false;
    }
    return QFile::exists(filePath);
}

void GlobalConfig::ensureDirectoryExists(QString dirPath)
{
    QDir dirCheck;
    if (dirPath.contains("../")) {
        return;
    }

    if (!dirCheck.exists(dirPath)) {
        if (!dirCheck.mkpath(dirPath)) {
            SYS_LOG(LOG_DEBUG, "Failed to create directory:%s",dirPath.toLatin1().data());
            return;
        }
    }

    QFile::setPermissions(dirPath,
                           QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |  // 7
                                  QFile::ReadGroup | QFile::ExeGroup |                      // 5
                                  QFile::ReadOther | QFile::ExeOther |                      // 5
                                  QFile::ExeOther | QFile::ExeGroup);                       // setuid位
}

void GlobalConfig::ensureGlobalDirectory()
{
    ensureDirectoryExists(GLOBAL_CONFIG_UKUI_PATH);
    ensureDirectoryExists(GLOBAL_CONFIG_USD_PATH);
    ensureDirectoryExists(GLOBAL_CONFIG_PATH);
}

void GlobalConfig::initFlghtMode()
{
    if (!IsSupportFlightMode()) {
        FlightModeState = -1;
        return;
    }
    QVariant variant = getGlobalConfig("rfkill", "rfkill");
    FlightModeState = variant.isValid()? variant.toInt() : 0;
}

QVariant GlobalConfig::getGlobalConfig(QString schema, QString key)
{
    QString configFile = QString("/etc/ukui/usd/globalconf.ini");
    QFile conf(configFile);
    if(!conf.exists()) {
        return QVariant();
    }
    QSettings usdSettings(configFile, QSettings::IniFormat);
    usdSettings.beginGroup(schema);
    QVariant value = usdSettings.value(key);
    usdSettings.endGroup();
    return value;

}

//__import__('gi.repository.GLib', globals(), locals(), ['Variant']).Variant("s","aaa")
bool GlobalConfig::setGlobalConf(QString schema, QString key, QDBusVariant value)
{
    ensureGlobalDirectory();
    QString configFile = QString("/etc/ukui/usd/globalconf.ini");
    QSettings *usdSettings = new QSettings(configFile, QSettings::IniFormat);
    usdSettings->beginGroup(schema);
    usdSettings->setValue(key, value.variant());
    usdSettings->endGroup();
    usdSettings->sync();
    usdSettings->deleteLater();

    QFile::setPermissions(configFile,QFileDevice::Permission(0x6444));

    return true;
}


QDBusVariant GlobalConfig::getGlobalConf(QString schema, QString key)
{
    QVariant value = getGlobalConfig(schema, key);
    if (!value.isValid()) {
        sendErrorReply(QDBusError::ErrorType::Failed, QString("can't find the key.%1").arg(key));
    }
    return (QDBusVariant)value;
}


bool GlobalConfig::setSchemaGlobalConf(QString schema, QVariantMap &map)
{
    QDir dirCheck;

    QString usdDir = QString("/etc/ukui");
    QString configDir = QString("/etc/ukui/usd");
    QString configFile = QString("/etc/ukui/usd/globalconf.ini");
    if (!dirCheck.exists(usdDir)) {
        dirCheck.mkdir(usdDir);

        QFile file(usdDir);
        file.setPermissions(QFileDevice::Permission(0x6000));
        file.close();
    }

    if (!dirCheck.exists(configDir)) {
        dirCheck.mkdir(configDir);
    }

    QFile file(configDir);
    file.setPermissions(QFileDevice::Permission(0x6000));
    file.close();

    QSettings *usdSettings = new QSettings(configFile, QSettings::IniFormat);
    usdSettings->beginGroup(schema);
    QVariantMap::const_iterator it;
    for(it = map.constBegin();it!=map.constEnd();++it)
    {
        usdSettings->setValue(it.key(), it.value());
    }
    usdSettings->endGroup();
    usdSettings->sync();
    usdSettings->deleteLater();

    QFile::setPermissions(configFile,QFileDevice::Permission(0x6000));
    return true;
}

bool GlobalConfig::IsSupportFlightMode()
{
    return UsdBaseClass::isNotebook();
}

void GlobalConfig::SetFlightModeState(int state)
{
    if (state == FlightModeState) {
        return;
    }

    setGlobalConf("rfkill", "rfkill", QDBusVariant(state));
    FlightModeState = state;
    Q_EMIT FlightModeChanged(FlightModeState);
}

int GlobalConfig::GetFlightModeState()
{
    return FlightModeState;
}

QVariantMap GlobalConfig::getSchemaGlobalConf(QString schema)
{
    QVariantMap map;
    QVariant value;
    QString configFile = QString("/etc/ukui/usd/globalconf.ini");
    QFile conf(configFile);

    if(!conf.exists()) {
        sendErrorReply(QDBusError::ErrorType::Failed,QString("configFile is't exist!"));
    }

    QSettings *usdSettings = new QSettings(configFile, QSettings::IniFormat);
    usdSettings->beginGroup(schema);
    QStringList keyList = usdSettings->allKeys();
    for(QString key:keyList)
    {
        value = usdSettings->value(key);
        map.insert(key, value);
    }

    usdSettings->endGroup();
    usdSettings->sync();
    usdSettings->deleteLater();

    if (map.size()<=0) {
        sendErrorReply(QDBusError::ErrorType::Failed,QString("config have no schem %1 configure").arg(schema));
    }
    return map;

}
