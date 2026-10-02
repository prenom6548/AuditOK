/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "configuration.h"

Configuration *Configuration::m_oInstance=nullptr;
QMutex         Configuration::m_oMutex;

Configuration::Configuration(QObject *parent) : QObject(parent)
{
    m_oSetting = new QSettings(QSettings::NativeFormat,
                               QSettings::UserScope,
                               "ukui.kylinos.cn",
                               "kylin-burner",
                               this);
}

Configuration *Configuration::instance()
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);
        if (nullptr == m_oInstance)
            m_oInstance = new Configuration(nullptr);
    }
    return m_oInstance;
}

QVariant Configuration::get(QString s, // section
                            QString n, // name
                            QVariant d) // default value
{
    QVariant ret;

    ret = m_oSetting->value(QString("%1/%2").arg(s).arg(n), d);
    if (ret == d) set(s, n, d);
    return ret;
}

void Configuration::set(QString s, // section
                        QString n, // name
                        QVariant v) // value
{
    m_oSetting->setValue(QString("%1/%2").arg(s).arg(n), v);
}

QVariantList Configuration::gets(QString section, QString name)
{
    int  size;
    QVariantList ret;

    size = m_oSetting->beginReadArray(section);

    for (int i = 0; i < size; ++i)
    {
        m_oSetting->setArrayIndex(i);
        ret << m_oSetting->value(name);
    }
    m_oSetting->endArray();
    return ret;
}

void Configuration::sets(QString section, QString name, QVariantList values)
{
    m_oSetting->beginWriteArray(section);

    for (int i = 0; i < values.size(); ++i)
    {
        m_oSetting->setArrayIndex(i);
        m_oSetting->setValue(name, values.at(i));
    }
    m_oSetting->endArray();
}

Configuration::~Configuration()
{
}
