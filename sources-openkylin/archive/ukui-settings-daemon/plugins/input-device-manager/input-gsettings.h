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
#ifndef INPUTGSETTINGS_H
#define INPUTGSETTINGS_H

#include <QObject>
#include <QSharedPointer>
#include <QGSettings/qgsettings.h>
#include <QMap>
#include <QVariant>
#include "input-common.h"

#define GSETTINGS_INIT_RESULT "gsettings-init-result"   //gsettings初始化结果

class InputGsettings : public QObject
{
    Q_OBJECT
    typedef QSharedPointer<QGSettings> GsettingsPtr;
    typedef QMap<QString , QVariant> GsettingsMap;

public:
    ~InputGsettings();
    static InputGsettings* instance();
    const QList<QString> getGsettingsKeys(DeviceType type);
    QVariant getGsettingsValue(const QString& key, DeviceType type = DeviceType::IN_MOUSE);
    void setGsettingsValue(const QString& key, const QVariant& value, DeviceType type = DeviceType::IN_MOUSE);
    void initGsettings();
    bool resultInitGsettings();

private:
    explicit InputGsettings(QObject *parent = nullptr);
    InputGsettings(const InputGsettings&) = delete;
    InputGsettings& operator =(const InputGsettings&)=delete;
private:
    void initMouseGsettings();
    void initTouchpadGsettings();
    void initKeyboardGsettings();
    void initTrackpointGsettings();
    void clearMapData();
private:
    GsettingsPtr m_mouseSettings;
    GsettingsPtr m_touchpadSettings;
    GsettingsPtr m_keyboardSettings;
    GsettingsPtr m_trackpointSettings;

    GsettingsMap m_mouseData;
    GsettingsMap m_touchpadData;
    GsettingsMap m_keyboardData;
    GsettingsMap m_trackpointData;

public Q_SLOTS:
    void onMouseChanged(const QString&);
    void onTouchpadChanged(const QString&);
    void onKeyboardChanged(const QString&);
    void onTrackpointChanged(const QString&);
Q_SIGNALS:
    void mouseChanged(const QString&, QVariant);
    void touchpadChanged(const QString&, QVariant);
    void keyboardChanged(const QString&, QVariant);
    void trackpointChanged(const QString&, QVariant);
};

#endif // INPUTGSETTINGS_H
