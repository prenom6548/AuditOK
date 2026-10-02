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
/*!
 *@brief 设置登录界面触摸屏和鼠标点对应
 *
 */

#ifndef TOUCHSCREENSET_H
#define TOUCHSCREENSET_H

#include <QObject>
#include <QDebug>

#include "monitorwatcher.h"


class TouchScreenSet : public QObject
{
    Q_OBJECT
public:
    static TouchScreenSet *instance(QObject* parent = nullptr);
    void init();

public slots:
    void setTouchMapScreen();

private:
    TouchScreenSet(QObject *parent = nullptr);

    MonitorWatcher *m_monitorWatcher;
};

#endif // TOUCHSCREENSET_H
