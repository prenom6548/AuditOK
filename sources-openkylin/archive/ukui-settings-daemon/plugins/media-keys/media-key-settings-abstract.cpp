/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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


#include "media-key-settings-abstract.h"

#define GSETTINGS_INIT_RESULT "gsettings-init-result"   //gsettings初始化结果

MediaKeyAbstractSettings::MediaKeyAbstractSettings(QString schema, QObject *parent) :
    QObject(parent)
{
    setGsettings(schema);
}

MediaKeyAbstractSettings::~MediaKeyAbstractSettings()
{
    clearMapData();
}

const QStringList MediaKeyAbstractSettings::getGsettingsKeys()
{
    return m_keysData.keys();
}

QVariant MediaKeyAbstractSettings::getGsettingsValue(const QString &key)
{
    if (m_keysData.contains(key)) {
        return m_keysData.value(key, QVariant(QVariant::Type::Invalid));
    } else {
        return QVariant(QVariant::Type::Invalid);
    }
}

bool MediaKeyAbstractSettings::resultInitSettings()
{
    //包含GSETTINGS_INIT_RESULT ，配置初始化失败
    return m_keysData.count() != 0;
}


QString MediaKeyAbstractSettings::getSummary(const QString &key) const
{
    if (m_settings.isNull()) {
        USD_LOG(LOG_DEBUG, "skip key:%s",key.toLatin1().data());
        return "";
    }

    if (m_settings->keys().contains(key)) {
        return m_settings->getSummary(key);
    }

    return "";
}

void MediaKeyAbstractSettings::resetSettings()
{
    m_settings.reset();
}

MediaKeyAbstractSettings::GsettingsPtr MediaKeyAbstractSettings::getGsettings()
{
    return m_settings;
}

void MediaKeyAbstractSettings::setGsettings(QString schema)
{
    m_settings = GsettingsPtr(new QGSettings(schema.toLatin1()));
}

void MediaKeyAbstractSettings::clearMapData()
{
    GsettingsMap().swap(m_keysData);
}

MediaKeyAbstractSettings::GsettingsMap MediaKeyAbstractSettings::getKeysData()
{
    return m_keysData;
}

void MediaKeyAbstractSettings::insertKeysData(QString key, QVariant qvar)
{
    m_keysData.insert(key, qvar);
}

//slots
void MediaKeyAbstractSettings::onKeyChanged(const QString &key)
{
    USD_LOG_SHOW_PARAMS(key.toLatin1().data());
    if (m_settings->keys().contains(key)) {
        QVariant value = m_settings->get(key);
        m_keysData.insert(key, value);
        Q_EMIT keyChanged(key, value);
    }
}
