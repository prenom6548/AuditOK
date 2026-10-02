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

#ifndef JOYSTICKSCOMMON_H
#define JOYSTICKSCOMMON_H

#include <QString>
#include <QTimer>
#define GAMEKEY_PATH "/home/lzh/lzh/kmre-window-tianjin/JSON/"
#define JSON_NAME ".json"

#define BUTTON_A            0   //按键A
#define BUTTON_B            1   //按键B
#define BUTTON_X            2   //按键X
#define BUTTON_Y            3   //按键Y
#define BUTTON_LB           4   //按键LB
#define BUTTON_RB           5   //按键RB
#define BUTTON_BACK         6   //按键BACK
#define BUTTON_START        7   //按键START
#define BUTTON_HOME         8   //按键HOME
#define BUTTON_LA           9   //按键LA(左摇杆中键)
#define BUTTON_RA           10  //按键RA(右摇杆中键)

#define BUTTON_LT           11   //按键LA(左摇杆中键)
#define BUTTON_RT           12  //按键RA(右摇杆中键)

#define POV_RELEASE         0   //方向键释放
#define POV_UP              1   //方向键上
#define POV_RIGHT           2   //方向键右
#define POV_DOWN            4   //方向键下
#define POV_LEFT            8   //方向键左
#define POV_UPRIGHT         3   //方向键右上
#define POV_DOWNRIGHT       6   //方向键右下
#define POV_DOWNLEFT        12  //方向键左下
#define POV_UPLEFT          9   //方向键左上

#define AXIS_LA_RIGHTLEFT   0   //左摇杆左右
#define AXIS_LA_UPDOWN      1   //左摇杆上下
#define AXIS_RA_RIGHTLEFT   2   //右摇杆左右
#define AXIS_RA_UPDOWN      3   //右摇杆上下
#define AXIS_LT             4   //按键(摇杆)LT
#define AXIS_RT             5   //按键(摇杆)RT

#define AXIS_LA             6
#define AXIS_RA             7

#define AXIS_MAX            1
#define AXIS_MIN            -1

#define BUTTON_ORDER        20  //按钮基数
#define POV_ORDER           0   //方向键基数
#define AXIS_ORDER          10  //摇杆基数

#define LA_ORDER            8
#define RA_ORDER            9

#define POV_STATUS          1

#define LA_TIMER            20
#define RA_TIMER            40

#define JOYSTICKBRAINWIDTH  150
#define JOYSTICKBRAINHEIGHT 140

struct QJoystickDevice
{
   int id; //操纵杆的数字标识
   int instanceID; //操纵杆的SDL实例ID
   QString name; //操纵杆显示名称
   QList<int> povs; //操纵杆操作的方向键数量
   QList<double> axes; //操纵杆操作的轴的数量
   QList<bool> buttons; //操纵杆操作的按钮数量
};

struct QJoystickRumble
{
   uint length; //影响时间
   qreal strength; //影响范围
   QJoystickDevice *joystick; //操纵杆指针
};

struct QJoystickPOVEvent
{
   int pov; //方向键编号
   int angle; //方向键角度
   QJoystickDevice *joystick; //操纵杆指针
};

struct QJoystickAxisEvent
{
   int axis; //轴编号
   qreal value; //当前轴值
   QJoystickDevice *joystick; //操纵杆指针
};

struct QJoystickButtonEvent
{
   int button; //按钮的数字标识
   bool pressed; //按钮状态
   QJoystickDevice *joystick; //操纵杆指针
};

struct QJoystickButtonSetting
{
   QString btn_type; //按钮类型（包括common/intelligence）
   qreal pos_x; //按钮位置x轴坐标
   qreal pos_y; //按钮位置y轴坐标
   bool continuously; //连续点击标识
   int push_number; //每秒连击次数
};

struct QJoystickAxisSetting
{
   qreal pos_x; //中心位置x轴坐标
   qreal pos_y; //中心位置y轴坐标
   int wheelbase; //轴距
   int sensitivity; //灵敏度
};

struct AxisValue
{
    qreal mAxisUpDownValueLA = 0.0;
    qreal mAxisRightLeftValueLA = 0.0;

    qreal mAxisUpDownValueRA = 0.0;
    qreal mAxisRightLeftValueRA = 0.0;

    qreal mAxisUpDownValueLAPre = 0.0;
    qreal mAxisRightLeftValueLAPre = 0.0;

    qreal mAxisUpDownValueRAPre = 0.0;
    qreal mAxisRightLeftValueRAPre = 0.0;
};

struct AxisPos
{
    int mAxisLAPosX = 0;
    int mAxisLAPosY = 0;

    int mAxisRAPosX = 0;
    int mAxisRAPosY = 0;

};

struct AxisFrequency
{
    int mAxisRAUpDown = 0;
    int mAxisRARightLeft = 0;

};

struct PovStatus
{
    bool mPovUpStatus = false;
    bool mPovDownStatus = false;
    bool mPovRightStatus = false;
    bool mPovLeftStatus = false;
};

struct AxisStatus
{
    bool mAxisLTStatus = false;
    bool mAxisRTStatus = false;
};

struct JoystickBrainInfo
{
    int mBrainPosX = 0;
    int mBrainPosY = 0;
    int mBrainPosXPre = 0;
    int mBrainPosYPre = 0;
    int mBrainOrder = 0;
};
#endif // JOYSTICKSCOMMON_H
