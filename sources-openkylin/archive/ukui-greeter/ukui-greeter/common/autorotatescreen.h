/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#ifndef AUTOROTATESCREEN_H
#define AUTOROTATESCREEN_H

/*!
 *@brief 根据重力传感器设置屏幕的方向
 * .pro Qt += sensor
 */

#include <QObject>
#include <QMutex>
#include <QOrientationSensor>
#include <QOrientationReading>
#include <QProcess>
#include "monitorwatcher.h"

class AutoRotateScreen : public QObject
{
    Q_OBJECT
public:
    enum RotateDir{
        Normal = 0,         // 顺时针旋转 0
        Inverted,           // 顺时针旋转 180
        Left,               // 顺时针旋转 270
        Right,              // 顺时针旋转 90
    };
    static AutoRotateScreen* instance(QObject *parent = nullptr);
    void start();
    void stop();

signals:
    void screenRotate(RotateDir dir);

private:
    AutoRotateScreen(QObject *parent = nullptr);
    RotateDir getScreenDirection();
    void init();
    void setScreenDirection(const RotateDir dir);

private slots:
    void onReadingChanged();
    void onActiveChanged();

private:
    QOrientationSensor       *m_pSensor;
    RotateDir                m_curScreenDirection;  // 当前屏幕的旋转方向
    MonitorWatcher           *monitorWatcher = nullptr;
};

#endif // AUTOROTATESCREEN_H
