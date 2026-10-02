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

#ifndef INPUTCOMMON_H
#define INPUTCOMMON_H

/* Keys with same names for both touchpad and mouse */
#define KEY_LEFT_HANDED                  "left-handed"          /*  a boolean for mouse, an enum for touchpad */
#define KEY_MOTION_ACCELERATION          "motion-acceleration"
#define KEY_MOTION_THRESHOLD             "motion-threshold"

/* Mouse settings */
#define UKUI_MOUSE_SCHEMA                "org.ukui.peripherals-mouse"
#define KEY_MOUSE_LOCATE_POINTER         "locate-pointer"
#define KEY_MIDDLE_BUTTON_EMULATION      "middle-button-enabled"
#define KEY_MOUSE_WHEEL_SPEED            "wheel-speed"
#define KEY_MOUSE_ACCEL                  "mouse-accel"
#define KEY_MOUSE_NATRUAL_SCROLLING      "natural-scroll"
#define KEY_MOUSE_DOUBLE_CLICK           "double-click"
#define CURSOR_THEME                     "cursor-theme"
#define CURSOR_SIZE                      "cursor-size"


/* Touchpad settings */
#define UKUI_TOUCHPAD_SCHEMA             "org.ukui.peripherals-touchpad"
#define KEY_TOUCHPAD_DISABLE_W_TYPING    "disable-while-typing"                 //打字时禁用触摸板
#define KEY_TOUCHPAD_TWO_FINGER_CLICK    "two-finger-click"                     //双指点击
#define KEY_TOUCHPAD_THREE_FINGER_CLICK  "three-finger-click"                   //三指点击
#define KEY_TOUCHPAD_NATURAL_SCROLL      "natural-scroll"                       //自然滚动
#define KEY_TOUCHPAD_TAP_TO_CLICK        "tap-to-click"                         //触摸点击
#define KEY_TOUCHPAD_ONE_FINGER_TAP      "tap-button-one-finger"                //单指轻触
#define KEY_TOUCHPAD_TWO_FINGER_TAP      "tap-button-two-finger"                //双指轻触
#define KEY_TOUCHPAD_THREE_FINGER_TAP    "tap-button-three-finger"              //三指轻触
#define KEY_VERT_EDGE_SCROLL             "vertical-edge-scrolling"              //垂直边缘滚动
#define KEY_HORIZ_EDGE_SCROLL            "horizontal-edge-scrolling"            //水平边缘滚动
#define KEY_VERT_TWO_FINGER_SCROLL       "vertical-two-finger-scrolling"        //垂直双指滚动
#define KEY_HORIZ_TWO_FINGER_SCROLL      "horizontal-two-finger-scrolling"      //水平双指滚动
#define KEY_TOUCHPAD_ENABLED             "touchpad-enabled"                     //触摸板开关
#define KEY_ZONE_GESTURE_LEFT_EDGE_ENABLED  "zone-gesture-left-edge-enabled"    //左侧边缘手势开关
#define KEY_ZONE_GESTURE_RIGHT_EDGE_ENABLED "zone-gesture-right-edge-enabled"   //右侧边缘手势开关
#define KEY_ZONE_GESTURE_TOP_EDGE_ENABLED   "zone-gesture-top-edge-enabled"     //顶部边缘手势开关
#define KEY_IS_ZONE_GESTURE_AVAILABLE       "is-zone-gesture-available"         //触控板软件边缘手势是否可用
#define KEY_TOUCHPAD_TWO_FINGER_PINCH       "two-finger-pinch"                   //双指捏合缩放

#define KEY_TOUCHPAD_DISBLE_O_E_MOUSE    "disable-on-external-mouse"        //插入鼠标，禁用触摸板  true/false
#define KEY_TOUCHPAD_DOUBLE_CLICK_DRAG   "double-click-drag"                //点击两次拖动		true/false
#define KEY_TOUCHPAD_BOTTOM_R_C_CLICK_M  "bottom-right-corner-click-menu"   //右下角点击菜单	true/false
#define KEY_TOUCHPAD_MOUSE_SENSITVITY    "mouse-sensitivity"                //鼠标敏感度	1-4  四个档位  低中高最高

/************keyboard settings************/
#define USD_KEYBOARD_SCHEMA  "org.ukui.peripherals-keyboard"
#define KEY_REPEAT           "repeat"
#define KEY_RATE             "rate"
#define KEY_DELAY            "delay"

#define USD_TRACKPOINT_SCHEMA             "org.ukui.peripherals-trackpoint"
#define KEY_TRACKPOINT_ENABLED            "trackpoint-enabled"

enum class DeviceType{
    IN_NONE,            //无
    IN_MOUSE,           //鼠标
    IN_TOUCHPAD,        //触控板
    IN_TOUCHSCREEN,     //触控屏
    IN_TABLET,          //触控笔
    IN_JOYSTICK,        //摇杆设备
    IN_KEYBOARD,        //键盘

    //trackpoint 设备
    IN_TRACKPOINT= 100  //联想红点设备
};

#endif // INPUTCOMMON_H
