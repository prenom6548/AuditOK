/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2022 KylinSoft Co., Ltd.
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
 *
 * Authors: sundagao <sundagao@kylinos.cn>
 */
#include "input-gsettings.h"
#include "clib-syslog.h"

InputGsettings::InputGsettings(QObject *parent) : QObject(parent)
{

}

InputGsettings::~InputGsettings()
{
    clearMapData();
}

InputGsettings *InputGsettings::instance()
{
    static InputGsettings inputGsettings;
    return &inputGsettings;
}

const QList<QString> InputGsettings::getGsettingsKeys(DeviceType type)
{
    switch (type) {
    case DeviceType::IN_MOUSE:
        return m_mouseData.keys();
        break;
    case DeviceType::IN_TOUCHPAD:
        return m_touchpadData.keys();
        break;
    case DeviceType::IN_KEYBOARD:
        return m_keyboardData.keys();
        break;
    case DeviceType::IN_TRACKPOINT:
        return m_trackpointData.keys();
        break;
    default:
        break;
    }
    return QList<QString>{};
}

QVariant InputGsettings::getGsettingsValue( const QString &key, DeviceType type)
{
    switch (type) {
    case DeviceType::IN_MOUSE:
        return m_mouseData.value(key, QVariant(QVariant::Type::Invalid));
        break;
    case DeviceType::IN_TOUCHPAD:
        return m_touchpadData.value(key, QVariant(QVariant::Type::Invalid));
        break;
    case DeviceType::IN_KEYBOARD:
        return m_keyboardData.value(key, QVariant(QVariant::Type::Invalid));
        break;
    case DeviceType::IN_TRACKPOINT:
        return m_trackpointData.value(key, QVariant(QVariant::Type::Invalid));
        break;
    default:
        break;
    }
    return QVariant(QVariant::Type::Invalid);
}

void InputGsettings::setGsettingsValue(const QString &key, const QVariant &value, DeviceType type)
{
    switch (type) {
    case DeviceType::IN_MOUSE:
        m_mouseSettings->set(key, value);
        break;
    case DeviceType::IN_TOUCHPAD:
        m_touchpadSettings->set(key, value);
        break;
    case DeviceType::IN_KEYBOARD:
        m_keyboardSettings->set(key, value);
        break;
    case DeviceType::IN_TRACKPOINT:
        m_trackpointSettings->set(key, value);
        break;
    default:
        break;
    }
}

void InputGsettings::initGsettings()
{
    initMouseGsettings();
    initTouchpadGsettings();
    initKeyboardGsettings();
    initTrackpointGsettings();
}

bool InputGsettings::resultInitGsettings()
{
    //包含GSETTINGS_INIT_RESULT ，配置初始化失败
    return !(m_mouseData.contains(GSETTINGS_INIT_RESULT) && m_touchpadData.contains(GSETTINGS_INIT_RESULT));
}

void InputGsettings::initMouseGsettings()
{
    if (QGSettings::isSchemaInstalled(UKUI_MOUSE_SCHEMA)) {
        m_mouseSettings = GsettingsPtr(new QGSettings(UKUI_MOUSE_SCHEMA));
        for (QString& key : m_mouseSettings->keys()) {
            m_mouseData.insert(key,m_mouseSettings->get(key));
        }
        connect(m_mouseSettings.data(),SIGNAL(changed(const QString&)),this,SLOT(onMouseChanged(const QString&)),Qt::DirectConnection);
    } else {
        m_mouseData.insert(GSETTINGS_INIT_RESULT,false);
    }
}

void InputGsettings::initTouchpadGsettings()
{
    if (QGSettings::isSchemaInstalled(UKUI_TOUCHPAD_SCHEMA)) {
        m_touchpadSettings = GsettingsPtr(new QGSettings(UKUI_TOUCHPAD_SCHEMA));

        for (QString& key : m_touchpadSettings->keys()) {
            m_touchpadData.insert(key,m_touchpadSettings->get(key));
        }
        connect(m_touchpadSettings.data(),SIGNAL(changed(const QString&)),this,SLOT(onTouchpadChanged(const QString&)),Qt::DirectConnection);
    } else {
        m_touchpadData.insert(GSETTINGS_INIT_RESULT,false);
    }
}

void InputGsettings::initKeyboardGsettings()
{
    if (QGSettings::isSchemaInstalled(USD_KEYBOARD_SCHEMA)) {
        m_keyboardSettings = GsettingsPtr(new QGSettings(USD_KEYBOARD_SCHEMA));

        for (QString& key : m_keyboardSettings->keys()) {
            m_keyboardData.insert(key,m_keyboardSettings->get(key));
        }
        connect(m_keyboardSettings.data(),SIGNAL(changed(const QString&)),this,SLOT(onKeyboardChanged(const QString&)),Qt::DirectConnection);
    } else {
        m_keyboardData.insert(GSETTINGS_INIT_RESULT,false);
    }
}

void InputGsettings::initTrackpointGsettings()
{
    if (QGSettings::isSchemaInstalled(USD_TRACKPOINT_SCHEMA)) {
        m_trackpointSettings = GsettingsPtr(new QGSettings(USD_TRACKPOINT_SCHEMA));

        for (QString& key : m_trackpointSettings->keys()) {
            m_trackpointData.insert(key,m_trackpointSettings->get(key));
        }
        connect(m_trackpointSettings.data(),SIGNAL(changed(const QString&)),this,SLOT(onTrackpointChanged(const QString&)),Qt::DirectConnection);
    } else {
        m_trackpointData.insert(GSETTINGS_INIT_RESULT,false);
    }
}

void InputGsettings::clearMapData()
{
    GsettingsMap().swap(m_mouseData);
    GsettingsMap().swap(m_touchpadData);
    GsettingsMap().swap(m_keyboardData);
    GsettingsMap().swap(m_trackpointData);
}

//slots
void InputGsettings::onMouseChanged(const QString &key)
{
    QVariant value = m_mouseSettings->get(key);
    m_mouseData.insert(key,value);
    Q_EMIT mouseChanged(key,value);
}

void InputGsettings::onTouchpadChanged(const QString &key)
{
    QVariant value = m_touchpadSettings->get(key);
    m_touchpadData.insert(key,value);
    Q_EMIT touchpadChanged(key,value);
}

void InputGsettings::onKeyboardChanged(const QString &key)
{
    QVariant value = m_keyboardSettings->get(key);
    m_keyboardData.insert(key,value);
    Q_EMIT keyboardChanged(key,value);
}

void InputGsettings::onTrackpointChanged(const QString &key)
{
    QVariant value = m_trackpointSettings->get(key);
    m_trackpointData.insert(key,value);
    Q_EMIT trackpointChanged(key,value);
}

