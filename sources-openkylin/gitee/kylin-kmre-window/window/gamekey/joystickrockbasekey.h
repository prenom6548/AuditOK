/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Zero Liu    liuzenghui1@kylinos.cn
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

#ifndef JOYSTICKROCKBASEKEY_H
#define JOYSTICKROCKBASEKEY_H

#include <QObject>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

#include "joystickbasekey.h"

const int DEFAULT_JOYSTICK_MARGIN = 18;
const int DEFAULT_JOYSTICK_ROCKER_SIZE = 94;
const int DEFAULT_JOYSTICK_ROCKER_ICON_SIZE = 80;
const int DEFAULT_JOYSTICK_ROCKER_FINISH_SIZE = 48;
const int DEFAULT_JOYSTICK_ROCKER_MARGIN =DEFAULT_JOYSTICK_ROCKER_SIZE -DEFAULT_JOYSTICK_ROCKER_ICON_SIZE  ;

class JoystickRockBaseKey: public JoystickBaseKey
{
    Q_OBJECT
public:
    JoystickRockBaseKey(KmreWindow* window, int type);

    void enableEdit(bool enable) Q_DECL_OVERRIDE;
    void updateSize(bool update_panel = true) Q_DECL_OVERRIDE;
    void updatePos() Q_DECL_OVERRIDE;  //这是一个重定义的方法

protected:
    void getTranslateCenterPos(QPoint pos) Q_DECL_OVERRIDE;

    QPushButton *mJoystickBtn = nullptr;
    QPushButton *mJoystickDeleteBtn = nullptr;
    QPushButton *mJoystickFinishBtn = nullptr;
    QLineEdit *mJoystickEdit = nullptr;

    double mFinishWidthRatio;   //编辑完成后的比例
    double mFInishHeightRatio;

    double mLittleType_x;   //修正误差值
    double mLittleType_y;
};

#endif // JOYSTICKROCKBASEKEY_H
